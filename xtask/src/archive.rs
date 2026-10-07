use crate::error::{Result, ResultExt, ensure};
use serde::{Deserialize, Serialize};
use sha2::Digest;
use std::{
    fs,
    path::{Path, PathBuf},
};

#[derive(Serialize, Deserialize)]
struct File {
    name: String,
    bytes: u64,
    sha256: String,
}

#[derive(Serialize, Deserialize)]
struct Bundle {
    directory: String,
    archive: String,
    sha256: String,
    bytes: u64,
    files: Vec<File>,
}

pub(crate) struct Archives;

impl Archives {
    const TARGET: &str = "x86_64-pc-windows-msvc";
    const TARGETS: &[&str] = &[
        "x86_64-pc-windows-msvc",
        "aarch64-pc-windows-msvc",
        "x86_64-pc-windows-gnu",
        "x86_64-unknown-linux-gnu",
        "aarch64-unknown-linux-gnu",
        "x86_64-apple-darwin",
        "aarch64-apple-darwin",
    ];

    // Calculate the checksum of the exact file bytes.
    fn hash(path: &Path) -> Result<String> {
        let bytes = fs::read(path).with_context(|| format!("Failed to read {}", path.display()))?;
        Ok(format!("{:x}", sha2::Sha256::digest(bytes)))
    }

    // Admit only known library roots and flat library filenames.
    fn validate(bundle: &Bundle) -> Result<()> {
        let valid = Self::TARGETS.iter().any(|target| {
            bundle.directory == format!("sdk/lib/{target}")
                && bundle.archive == format!("sdk/archives/{target}.tar.zst")
        });
        ensure!(
            valid,
            "Failed to validate archive destination: {}",
            bundle.directory
        );
        ensure!(
            !bundle.files.is_empty(),
            "Failed to validate empty archive inventory"
        );
        for file in &bundle.files {
            ensure!(
                Self::filename(&file.name),
                "Failed to validate archive filename: {}",
                file.name
            );
        }
        Ok(())
    }

    // Reject paths, special entries, and extensions unrelated to native libraries.
    fn filename(name: &str) -> bool {
        !name.contains(['/', '\\', ':'])
            && (name.ends_with(".lib") || name.ends_with(".a"))
            && !name.starts_with('.')
    }

    // Reuse a library directory only if all recorded bytes still match.
    fn intact(root: &Path, bundle: &Bundle) -> Result<bool> {
        for file in &bundle.files {
            let path = root.join(&bundle.directory).join(&file.name);
            if !path.is_file() {
                return Ok(false);
            }
            if fs::metadata(&path)
                .context("Failed to inspect extracted library")?
                .len()
                != file.bytes
                || Self::hash(&path).context("Failed to verify extracted library")? != file.sha256
            {
                return Ok(false);
            }
        }
        Ok(true)
    }

    // Extract a checked flat archive and reject unlisted or nonregular entries.
    fn extract(root: &Path, bundle: &Bundle) -> Result<()> {
        let input =
            fs::File::open(root.join(&bundle.archive)).context("Failed to open SDK archive")?;
        let decoder = zstd::Decoder::new(input).context("Failed to decode SDK archive")?;
        let mut archive = tar::Archive::new(decoder);
        let destination = root.join(&bundle.directory);
        fs::create_dir_all(&destination).context("Failed to create extraction directory")?;
        let mut seen = std::collections::HashSet::new();
        for entry in archive
            .entries()
            .context("Failed to enumerate SDK archive")?
        {
            let mut entry = entry.context("Failed to read SDK archive entry")?;
            let path = entry
                .path()
                .context("Failed to read archived path")?
                .into_owned();
            let name = path
                .to_str()
                .context("Failed to decode archived filename")?;
            ensure!(
                Self::filename(name) && entry.header().entry_type().is_file(),
                "Failed to validate archived entry"
            );
            let file = bundle
                .files
                .iter()
                .find(|file| file.name == name)
                .context("Failed to find archived file in inventory")?;
            ensure!(
                entry.size() == file.bytes && seen.insert(name.to_owned()),
                "Failed to validate archived size or duplicate"
            );
            entry
                .unpack(destination.join(name))
                .context("Failed to extract native library")?;
        }
        ensure!(
            seen.len() == bundle.files.len(),
            "Failed to extract complete library inventory"
        );
        ensure!(
            Self::intact(root, bundle).context("Failed to verify extracted archive")?,
            "Failed to verify extracted library checksums"
        );
        Ok(())
    }

    // Verify compressed and extracted checksums before exposing local build inputs.
    pub(crate) fn prepare(root: &Path, all: bool) -> Result<()> {
        let manifest = fs::read(root.join("sdk/archives.json"))
            .context("Failed to read compressed SDK inventory")?;
        let bundles: Vec<Bundle> = serde_json::from_slice(&manifest)
            .context("Failed to decode compressed SDK inventory")?;
        let mut seen = std::collections::HashSet::new();
        let mut prepared = false;
        for bundle in bundles {
            Self::validate(&bundle).context("Failed to validate compressed SDK inventory")?;
            ensure!(
                seen.insert(bundle.directory.clone()),
                "Failed to validate duplicate SDK inventory"
            );
            if !all && !bundle.directory.ends_with(Self::TARGET) {
                continue;
            }
            prepared = true;
            Self::provenance(root, &bundle).context("Failed to verify SDK provenance")?;
            ensure!(
                Self::hash(&root.join(&bundle.archive)).context("Failed to hash compressed SDK")?
                    == bundle.sha256,
                "Failed to verify compressed SDK checksum: {}",
                bundle.archive
            );
            if !Self::intact(root, &bundle).context("Failed to check local library cache")? {
                Self::extract(root, &bundle).context("Failed to prepare local native libraries")?;
            }
        }
        ensure!(
            prepared,
            "Failed to locate selected SDK in compressed inventory"
        );
        Ok(())
    }

    // Verify SDK inputs against the retained vendor provenance inventory.
    fn provenance(root: &Path, bundle: &Bundle) -> Result<()> {
        let document: serde_json::Value = serde_json::from_slice(
            &fs::read(root.join("sdk/manifest.json")).context("Failed to read SDK provenance")?,
        )
        .context("Failed to decode SDK provenance")?;
        let target = bundle.directory.trim_start_matches("sdk/lib/");
        let records = document["platforms"]
            .as_array()
            .context("Failed to read SDK targets")?
            .iter()
            .find(|record| record["target"].as_str() == Some(target))
            .context("Failed to find vendor SDK target")?["libraries"]
            .as_array()
            .context("Failed to read SDK libraries")?;
        ensure!(
            records.len() == bundle.files.len(),
            "Failed to validate library inventory count"
        );
        for file in &bundle.files {
            let record = records
                .iter()
                .find(|record| {
                    record["file"].as_str().is_some_and(|name| {
                        name == file.name
                            || name
                                == format!(
                                    "lib/{}/{}",
                                    bundle.directory.trim_start_matches("sdk/lib/"),
                                    file.name
                                )
                    })
                })
                .context("Failed to find library provenance")?;
            ensure!(
                record["sha256"].as_str() == Some(file.sha256.as_str())
                    && record["bytes"].as_u64() == Some(file.bytes),
                "Failed to validate library provenance: {}",
                file.name
            );
        }
        Ok(())
    }

    // Compress flat libraries reproducibly with Zstandard and preserve their inventory.
    fn pack(root: &Path, directory: &str, archive: &str) -> Result<Bundle> {
        let mut paths: Vec<PathBuf> = fs::read_dir(root.join(directory))
            .context("Failed to enumerate archive inputs")?
            .map(|entry| entry.map(|entry| entry.path()))
            .collect::<std::io::Result<_>>()
            .context("Failed to read archive input paths")?;
        paths.sort();
        let mut bundle = Bundle {
            directory: directory.into(),
            archive: archive.into(),
            sha256: String::new(),
            bytes: 0,
            files: Vec::new(),
        };
        for path in &paths {
            let name = path
                .file_name()
                .and_then(|name| name.to_str())
                .context("Failed to decode library filename")?;
            ensure!(
                path.is_file() && Self::filename(name),
                "Failed to select regular library input"
            );
            bundle.files.push(File {
                name: name.into(),
                bytes: fs::metadata(path)
                    .context("Failed to inspect library")?
                    .len(),
                sha256: Self::hash(path).context("Failed to hash library input")?,
            });
        }
        Self::provenance(root, &bundle).context("Failed to verify archive input provenance")?;
        let output = root.join(archive);
        fs::create_dir_all(
            output
                .parent()
                .context("Failed to locate archive directory")?,
        )
        .context("Failed to create archive directory")?;
        let file =
            fs::File::create(&output).context("Failed to create compressed library archive")?;
        let mut encoder =
            zstd::Encoder::new(file, 19).context("Failed to initialize Zstandard encoder")?;
        encoder
            .include_checksum(true)
            .context("Failed to enable Zstandard checksum")?;
        let mut builder = tar::Builder::new(encoder);
        builder.mode(tar::HeaderMode::Deterministic);
        for (path, file) in paths.iter().zip(&bundle.files) {
            builder
                .append_path_with_name(path, &file.name)
                .context("Failed to archive native library")?;
        }
        builder
            .into_inner()
            .context("Failed to finalize tar archive")?
            .finish()
            .context("Failed to finalize Zstandard archive")?;
        bundle.sha256 = Self::hash(&output).context("Failed to hash compressed library archive")?;
        bundle.bytes = fs::metadata(&output)
            .context("Failed to inspect compressed library archive")?
            .len();
        println!(
            "{}: {} -> {} bytes",
            bundle.archive,
            bundle.files.iter().map(|file| file.bytes).sum::<u64>(),
            bundle.bytes
        );
        Ok(bundle)
    }

    // Refresh all compressed 64-bit inputs after an intentional maintainer update.
    pub(crate) fn refresh(root: &Path) -> Result<()> {
        let mut bundles = Vec::new();
        for target in Self::TARGETS {
            bundles.push(
                Self::pack(
                    root,
                    &format!("sdk/lib/{target}"),
                    &format!("sdk/archives/{target}.tar.zst"),
                )
                .context("Failed to compress vendor SDK")?,
            );
        }
        fs::write(
            root.join("sdk/archives.json"),
            serde_json::to_vec_pretty(&bundles).context("Failed to encode archive inventory")?,
        )
        .context("Failed to save archive inventory")?;
        Ok(())
    }
}

#[cfg(test)]
mod tests {
    use super::{Archives, Bundle, File};
    use crate::error::{Result, ResultExt};
    use std::fs;

    // Reject traversal paths and targets outside the maintained 64-bit inventory.
    #[test]
    fn rejects_unsafe_inventory() {
        let mut bundle = Bundle {
            directory: "sdk/lib/i686-pc-windows-msvc".into(),
            archive: "sdk/archives/i686-pc-windows-msvc.tar.zst".into(),
            sha256: String::new(),
            bytes: 0,
            files: vec![File {
                name: "libclient.lib".into(),
                bytes: 0,
                sha256: String::new(),
            }],
        };
        assert!(Archives::validate(&bundle).is_err());
        bundle.directory = "sdk/lib/x86_64-pc-windows-msvc".into();
        bundle.archive = "sdk/archives/x86_64-pc-windows-msvc.tar.zst".into();
        assert!(Archives::validate(&bundle).is_ok());
        for name in [
            "../outside.lib",
            "C:outside.lib",
            "nested/libclient.lib",
            "link",
            "..\\outside.lib",
        ] {
            bundle.files[0].name = name.into();
            assert!(Archives::validate(&bundle).is_err());
        }
    }

    // Restore missing and damaged libraries and reject corrupted compressed inputs.
    #[test]
    fn round_trip_and_corruption() -> Result<()> {
        let timestamp = std::time::SystemTime::now()
            .duration_since(std::time::UNIX_EPOCH)
            .context("Failed to timestamp archive fixture")?
            .as_nanos();
        let root = super::Path::new(env!("CARGO_MANIFEST_DIR"))
            .join(format!("../temp/archive-test-{timestamp}"));
        let directory = "sdk/lib/x86_64-pc-windows-msvc";
        fs::create_dir_all(root.join(directory)).context("Failed to create archive fixture")?;
        let library = root.join(directory).join("test.lib");
        fs::write(&library, b"fixture library").context("Failed to write library fixture")?;
        let hash = Archives::hash(&library).context("Failed to hash library fixture")?;
        let inventory = serde_json::json!({"platforms": [{"target": "x86_64-pc-windows-msvc", "libraries": [{"file": "lib/x86_64-pc-windows-msvc/test.lib", "bytes": 15, "sha256": hash}]}]});
        fs::write(root.join("sdk/manifest.json"), inventory.to_string())
            .context("Failed to write fixture provenance")?;
        let bundle = Archives::pack(
            &root,
            directory,
            "sdk/archives/x86_64-pc-windows-msvc.tar.zst",
        )
        .context("Failed to pack fixture")?;
        fs::write(
            root.join("sdk/archives.json"),
            serde_json::to_vec(&[bundle]).context("Failed to encode fixture inventory")?,
        )
        .context("Failed to save fixture inventory")?;
        fs::remove_file(&library).context("Failed to remove fixture extraction")?;
        Archives::prepare(&root, false).context("Failed to restore missing library")?;
        assert_eq!(
            fs::read(&library).context("Failed to read restored library")?,
            b"fixture library"
        );
        fs::write(&library, b"damaged").context("Failed to damage extracted fixture")?;
        Archives::prepare(&root, false).context("Failed to restore damaged library")?;
        assert_eq!(
            fs::read(&library).context("Failed to read repaired library")?,
            b"fixture library"
        );
        fs::write(
            root.join("sdk/archives/x86_64-pc-windows-msvc.tar.zst"),
            b"damaged",
        )
        .context("Failed to corrupt archive fixture")?;
        assert!(Archives::prepare(&root, false).is_err());
        Ok(())
    }
}
