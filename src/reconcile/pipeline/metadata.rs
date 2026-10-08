use super::{
    local::{Agent, Snapshot},
    path::Path,
};
use crate::{Result, ResultExt};
use std::{
    collections::{HashMap, HashSet},
    fs,
    path::PathBuf,
    sync::{self, atomic},
    time,
};

type Stored<T> = std::result::Result<T, sync::Arc<crate::Error>>;
type Listing = sync::Arc<sync::OnceLock<Stored<sync::Arc<Directory>>>>;

struct File {
    entry: sync::Mutex<Option<Box<walkdir::DirEntry>>>,
    metadata: sync::OnceLock<Stored<Box<Snapshot>>>,
    contents: sync::OnceLock<Stored<Box<Snapshot>>>,
}
pub(super) struct Directory {
    files: sync::Mutex<HashMap<Path, sync::Arc<File>>>,
    entries: sync::Mutex<Vec<PathBuf>>,
}
pub(super) struct Metadata {
    directories: [sync::Mutex<HashMap<Path, Listing>>; 32],
    pub files: atomic::AtomicU64,
    pub listings: atomic::AtomicU64,
    pub reuses: atomic::AtomicU64,
    pub hashed: atomic::AtomicU64,
}

impl Metadata {
    // Allocate one command-wide registry shared by enumeration and targeted probes.
    pub(super) fn new() -> Self {
        Self {
            directories: std::array::from_fn(|_| sync::Mutex::new(HashMap::new())),
            files: atomic::AtomicU64::new(0),
            listings: atomic::AtomicU64::new(0),
            reuses: atomic::AtomicU64::new(0),
            hashed: atomic::AtomicU64::new(0),
        }
    }
    // Enumerate each directory once while publishing its result without holding a registry lock.
    pub(super) fn directory(&self, path: &std::path::Path) -> Result<sync::Arc<Directory>> {
        let spelling = path
            .to_str()
            .context("Failed to encode metadata directory")?;
        let key = Path::new(spelling);
        let slot = self.directories[key.shard(self.directories.len())]
            .lock()
            .map_err(|_| crate::Error::new("Failed to lock directory registry"))?
            .entry(key)
            .or_insert_with(|| sync::Arc::new(sync::OnceLock::new()))
            .clone();
        slot.get_or_init(|| {
            self.listings.fetch_add(1, atomic::Ordering::Relaxed);
            Self::read_directory(path)
                .map(sync::Arc::new)
                .map_err(sync::Arc::new)
        })
        .clone()
        .context("Failed to load shared directory metadata")
    }
    // Preserve names, errors, and symlink boundaries through the portable third-party walker.
    fn read_directory(path: &std::path::Path) -> Result<Directory> {
        let mut files = HashMap::new();
        let mut entries = Vec::new();
        for entry in walkdir::WalkDir::new(path)
            .follow_links(false)
            .follow_root_links(false)
            .max_depth(1)
        {
            let entry = match entry {
                Ok(entry) => entry,
                Err(error)
                    if error.depth() == 0
                        && error
                            .io_error()
                            .is_some_and(|error| error.kind() == std::io::ErrorKind::NotFound) =>
                {
                    continue;
                }
                Err(error) => {
                    return Err(error)
                        .with_context(|| format!("Failed to enumerate {}", path.display()));
                }
            };
            if entry.depth() == 0 {
                continue;
            }
            let Some(spelling) = entry.file_name().to_str() else {
                entries.push(entry.into_path());
                continue;
            };
            files.insert(
                Path::new(spelling),
                sync::Arc::new(File {
                    entry: sync::Mutex::new(Some(Box::new(entry.clone()))),
                    metadata: sync::OnceLock::new(),
                    contents: sync::OnceLock::new(),
                }),
            );
            entries.push(entry.into_path());
        }
        Ok(Directory {
            files: sync::Mutex::new(files),
            entries: sync::Mutex::new(entries),
        })
    }
    // Share one slot for an existing or missing path, including repeat server records.
    fn file(&self, path: &std::path::Path) -> Result<sync::Arc<File>> {
        let parent = path.parent().context("Failed to find metadata parent")?;
        let directory = self
            .directory(parent)
            .context("Failed to obtain file directory")?;
        let key = Path::new(
            path.file_name()
                .and_then(std::ffi::OsStr::to_str)
                .context("Failed to encode metadata filename")?,
        );
        let slot = directory
            .files
            .lock()
            .map_err(|_| crate::Error::new("Failed to lock shared file slots"))?
            .entry(key)
            .or_insert_with(|| {
                sync::Arc::new(File {
                    entry: sync::Mutex::new(None),
                    metadata: sync::OnceLock::new(),
                    contents: sync::OnceLock::new(),
                })
            })
            .clone();
        Ok(slot)
    }
    // Fetch each path's metadata once; Windows entries reuse enumeration-provided metadata.
    fn raw(&self, file: &File) -> Result<Snapshot> {
        let reused = file.metadata.get().is_some();
        let result = file.metadata.get_or_init(|| {
            self.files.fetch_add(1, atomic::Ordering::Relaxed);
            (|| {
                let entry = file
                    .entry
                    .lock()
                    .map_err(|_| crate::Error::new("Failed to lock walker metadata"))?
                    .take();
                entry.map_or(Ok(Snapshot::default()), |entry| {
                    let metadata = entry.metadata().context("Failed to read walker metadata")?;
                    Self::snapshot(entry.path(), &metadata)
                })
            })()
            .map(Box::new)
            .map_err(sync::Arc::new)
        });
        if reused {
            self.reuses.fetch_add(1, atomic::Ordering::Relaxed);
        }
        result
            .as_ref()
            .map(|snapshot| **snapshot)
            .map_err(sync::Arc::clone)
            .context("Failed to reuse file metadata")
    }
    // Match SDK existence, size, timestamps, permissions, and special-file flags.
    fn snapshot(path: &std::path::Path, metadata: &fs::Metadata) -> Result<Snapshot> {
        let mut stat = 1 | (i32::from(!metadata.permissions().readonly()) * 2);
        if metadata.is_dir() {
            stat |= 4 | 16;
        }
        if metadata.is_symlink() {
            stat |= 8;
        }
        if !metadata.is_dir() && !metadata.is_file() && !metadata.is_symlink() {
            stat |= 16;
        }
        if metadata.len() == 0 {
            stat |= 64;
        }
        #[cfg(unix)]
        {
            use std::os::unix::fs::PermissionsExt;
            stat &= !2;
            if metadata.permissions().mode() & 0o200 != 0 {
                stat |= 2;
            }
            if metadata.permissions().mode() & 0o100 != 0 {
                stat |= 32;
            }
        }
        #[cfg(windows)]
        if metadata.is_dir()
            || path
                .extension()
                .and_then(std::ffi::OsStr::to_str)
                .is_some_and(|extension| {
                    ["exe", "cmd", "bat", "com"]
                        .iter()
                        .any(|candidate| extension.eq_ignore_ascii_case(candidate))
                })
        {
            stat |= 32;
        }
        #[cfg(target_os = "macos")]
        {
            use std::os::darwin::fs::MetadataExt;
            if metadata.st_flags() & 2 != 0 {
                stat &= !2;
            }
        }
        #[cfg(not(windows))]
        let _ = path;
        let modified = metadata
            .modified()
            .context("Failed to read cached modification time")?;
        let time = match modified.duration_since(time::UNIX_EPOCH) {
            Ok(duration) => {
                i64::try_from(duration.as_secs()).context("Failed to encode cached timestamp")?
            }
            Err(error) => {
                -i64::try_from(error.duration().as_secs())
                    .context("Failed to encode historical timestamp")?
                    - i64::from(error.duration().subsec_nanos() != 0)
            }
        };
        Ok(Snapshot {
            size: metadata.len(),
            time,
            link_time: time,
            stat,
            ..Snapshot::default()
        })
    }
    // Resolve link targets through the same registry without fetching any path twice.
    fn resolved(&self, path: &std::path::Path, file: &File) -> Result<Snapshot> {
        let mut snapshot = self
            .raw(file)
            .context("Failed to read initial path metadata")?;
        if snapshot.stat & 8 == 0 {
            return Ok(snapshot);
        }
        let link_time = snapshot.time;
        let mut visited = HashSet::new();
        let mut current = path.to_path_buf();
        while snapshot.stat & 8 != 0 {
            let key = Path::new(current.to_str().context("Failed to encode link path")?);
            if !visited.insert(key) {
                return Err(crate::Error::new(
                    "Failed to resolve cyclic filesystem link",
                ));
            }
            let target = fs::read_link(&current)
                .with_context(|| format!("Failed to read link {}", current.display()))?;
            current = if target.is_absolute() {
                target
            } else {
                current
                    .parent()
                    .context("Failed to resolve link parent")?
                    .join(target)
            };
            current = Self::lexical(&current);
            let target = self
                .file(&current)
                .context("Failed to obtain link target slot")?;
            snapshot = self
                .raw(&target)
                .context("Failed to reuse target metadata")?;
        }
        snapshot.stat |= 8;
        snapshot.link_time = link_time;
        Ok(snapshot)
    }
    // Normalize lexical parent segments without resolving links or querying the filesystem.
    fn lexical(path: &std::path::Path) -> PathBuf {
        let mut result = PathBuf::new();
        for component in path.components() {
            if matches!(component, std::path::Component::ParentDir) {
                result.pop();
            } else if !matches!(component, std::path::Component::CurDir) {
                result.push(component.as_os_str());
            }
        }
        result
    }
    // Compute canonical content at most once on the metadata slot's elected worker.
    pub(super) fn inspect(&self, path: &str, agent: &Agent<'_>, hashes: bool) -> Result<Snapshot> {
        let path = Self::lexical(std::path::Path::new(path));
        let file = self
            .file(&path)
            .context("Failed to obtain canonical file slot")?;
        if !hashes {
            return self
                .resolved(&path, &file)
                .context("Failed to reuse timestamp metadata");
        }
        file.contents
            .get_or_init(|| {
                (|| {
                    let snapshot = self
                        .resolved(&path, &file)
                        .context("Failed to resolve canonical snapshot")?;
                    let snapshot = agent
                        .cached(
                            path.to_str().context("Failed to encode canonical path")?,
                            snapshot,
                            hashes,
                        )
                        .context("Failed to compute canonical snapshot")?;
                    if snapshot.hashed != 0 {
                        self.hashed
                            .fetch_add(snapshot.size, atomic::Ordering::Relaxed);
                    }
                    Ok(Box::new(snapshot))
                })()
                .map_err(sync::Arc::new)
            })
            .as_ref()
            .map(|snapshot| **snapshot)
            .map_err(sync::Arc::clone)
            .context("Failed to reuse canonical snapshot")
    }
    // Read the shared entry classification without computing contents or following links.
    pub(super) fn basic(&self, path: &str) -> Result<Snapshot> {
        let file = self
            .file(std::path::Path::new(path))
            .context("Failed to obtain basic metadata slot")?;
        self.raw(&file)
            .context("Failed to reuse entry classification")
    }
    // Supply native candidates with the same immutable snapshot as local enumeration.
    pub(super) fn snapshot_for(&self, path: &str) -> Result<Snapshot> {
        let path = Self::lexical(std::path::Path::new(path));
        let file = self
            .file(&path)
            .context("Failed to obtain native snapshot slot")?;
        if let Some(snapshot) = file.contents.get() {
            return snapshot
                .as_ref()
                .map(|snapshot| **snapshot)
                .map_err(sync::Arc::clone)
                .context("Failed to reuse native content snapshot");
        }
        self.resolved(&path, &file)
            .context("Failed to resolve native metadata")
    }
}

impl Directory {
    // Release traversal names after their single scanner consumes the cached listing.
    pub(super) fn entries(&self) -> Result<Vec<PathBuf>> {
        let mut entries = self
            .entries
            .lock()
            .map_err(|_| crate::Error::new("Failed to consume directory listing"))?;
        Ok(std::mem::take(&mut *entries))
    }
}
