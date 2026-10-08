mod records;

use super::{
    local::{Agent, Snapshot},
    names::Names,
    path::Path,
    radix::Radix,
};
use crate::{Result, ResultExt};
use std::{
    collections::HashSet,
    fs,
    path::PathBuf,
    sync::{self, atomic},
    time,
};

type Stored<T> = std::result::Result<T, sync::Arc<crate::Error>>;
type Listing = sync::Arc<sync::OnceLock<Stored<sync::Arc<Directory>>>>;
type Contents = sync::Arc<sync::OnceLock<Stored<Box<Snapshot>>>>;

#[derive(Clone, Copy, Default)]
struct Stat {
    size: u64,
    time: i64,
    flags: i32,
}
struct ContentIndex {
    files: sync::Mutex<Names<Contents>>,
}
pub(super) struct Directory {
    files: sync::Mutex<Names<Stored<Stat>, records::Records>>,
    entries: sync::Mutex<Vec<PathBuf>>,
    contents: sync::OnceLock<Box<ContentIndex>>,
}
pub(super) struct Metadata {
    directories: [sync::Mutex<Radix<Listing>>; 32],
    pub files: atomic::AtomicU64,
    pub listings: atomic::AtomicU64,
    pub reuses: atomic::AtomicU64,
    pub hashed: atomic::AtomicU64,
}

impl Metadata {
    // Compact each registry shard after the local enumeration workers have joined.
    pub(super) fn compact(&self) -> Result<()> {
        for shard in &self.directories {
            shard
                .lock()
                .map_err(|_| crate::Error::new("Failed to compact directory registry"))?
                .compact();
        }
        Ok(())
    }

    // Allocate one command-wide registry shared by enumeration and targeted probes.
    pub(super) fn new() -> Self {
        Self {
            directories: std::array::from_fn(|_| sync::Mutex::new(Radix::new())),
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
        let hash = Path::hash_of(spelling);
        let mut directories = self.directories[hash as usize % self.directories.len()]
            .lock()
            .map_err(|_| crate::Error::new("Failed to lock directory registry"))?;
        let slot = sync::Arc::clone(
            directories
                .get_or_insert_with(spelling, || sync::Arc::new(sync::OnceLock::new()))
                .context("Failed to retain shared directory slot")?,
        );
        drop(directories);
        slot.get_or_init(|| {
            self.listings.fetch_add(1, atomic::Ordering::Relaxed);
            self.read_directory(path)
                .map(sync::Arc::new)
                .map_err(sync::Arc::new)
        })
        .clone()
        .context("Failed to load shared directory metadata")
    }
    // Preserve names, errors, and symlink boundaries through the portable third-party walker.
    fn read_directory(&self, path: &std::path::Path) -> Result<Directory> {
        let mut files = Names::new();
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
            let snapshot = entry
                .metadata()
                .context("Failed to read walker metadata")
                .and_then(|metadata| Self::snapshot(entry.path(), &metadata))
                .map(Stat::from)
                .map_err(sync::Arc::new);
            files
                .push(spelling, snapshot)
                .context("Failed to retain enumerated metadata")?;
            self.files.fetch_add(1, atomic::Ordering::Relaxed);
            entries.push(entry.into_path());
        }
        files
            .finish()
            .context("Failed to finish compact directory names")?;
        Ok(Directory {
            files: sync::Mutex::new(files),
            entries: sync::Mutex::new(entries),
            contents: sync::OnceLock::new(),
        })
    }
    // Retain one compact metadata value for each existing or missing path.
    fn raw(&self, path: &std::path::Path) -> Result<Snapshot> {
        let parent = path.parent().context("Failed to find metadata parent")?;
        let directory = self
            .directory(parent)
            .context("Failed to obtain file directory")?;
        let name = path
            .file_name()
            .and_then(std::ffi::OsStr::to_str)
            .context("Failed to encode metadata filename")?;
        let snapshot = directory
            .files
            .lock()
            .map_err(|_| crate::Error::new("Failed to lock shared file slots"))?
            .get_or_insert_with(name, || {
                self.files.fetch_add(1, atomic::Ordering::Relaxed);
                Ok(Stat::default())
            })
            .context("Failed to cache path classification")?;
        self.reuses.fetch_add(1, atomic::Ordering::Relaxed);
        snapshot
            .map(Snapshot::from)
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
    fn resolved(&self, path: &std::path::Path) -> Result<Snapshot> {
        let mut snapshot = self
            .raw(path)
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
            snapshot = self
                .raw(&current)
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
        if !hashes {
            return self
                .resolved(&path)
                .context("Failed to reuse timestamp metadata");
        }
        let contents = self
            .contents(&path, true)
            .context("Failed to obtain canonical content slot")?
            .context("Failed to initialize content slot")?;
        contents
            .get_or_init(|| {
                (|| {
                    let snapshot = self
                        .resolved(&path)
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
        self.raw(std::path::Path::new(path))
            .context("Failed to reuse entry classification")
    }
    // Supply native candidates with the same immutable snapshot as local enumeration.
    pub(super) fn snapshot_for(&self, path: &str) -> Result<Snapshot> {
        let path = Self::lexical(std::path::Path::new(path));
        let contents = self
            .contents(&path, false)
            .context("Failed to obtain native content cache")?;
        if let Some(snapshot) = contents.as_ref().and_then(|slot| slot.get()) {
            return snapshot
                .as_ref()
                .map(|snapshot| **snapshot)
                .map_err(sync::Arc::clone)
                .context("Failed to reuse native content snapshot");
        }
        self.resolved(&path)
            .context("Failed to resolve native metadata")
    }
    // Allocate digest synchronization only for paths that actually request canonical contents.
    fn contents(&self, path: &std::path::Path, create: bool) -> Result<Option<Contents>> {
        let directory = self
            .directory(path.parent().context("Failed to find content parent")?)
            .context("Failed to obtain content directory")?;
        let index = if create {
            Some(directory.contents.get_or_init(|| {
                Box::new(ContentIndex {
                    files: sync::Mutex::new(Names::new()),
                })
            }))
        } else {
            directory.contents.get()
        };
        let Some(index) = index else {
            return Ok(None);
        };
        let mut contents = index
            .files
            .lock()
            .map_err(|_| crate::Error::new("Failed to lock canonical content slots"))?;
        let name = path
            .file_name()
            .and_then(std::ffi::OsStr::to_str)
            .context("Failed to encode content filename")?;
        Ok(if create {
            Some(
                contents
                    .get_or_insert_with(name, || sync::Arc::new(sync::OnceLock::new()))
                    .context("Failed to cache content synchronization")?,
            )
        } else {
            contents.get(name)
        })
    }
}

impl From<Snapshot> for Stat {
    // Retain only immutable filesystem fields in the command-wide metadata index.
    fn from(snapshot: Snapshot) -> Self {
        Self {
            size: snapshot.size,
            time: snapshot.time,
            flags: snapshot.stat,
        }
    }
}
impl From<Stat> for Snapshot {
    // Materialize the native ABI snapshot only when a comparison needs it.
    fn from(stat: Stat) -> Self {
        Self {
            size: stat.size,
            time: stat.time,
            link_time: stat.time,
            stat: stat.flags,
            ..Self::default()
        }
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
