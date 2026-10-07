use crate::error::{Result, ResultExt, ensure};
use sha2::Digest;
use std::{
    fs,
    path::{Component, Path, PathBuf},
};

pub(super) struct Sources;

impl Sources {
    // Read checksums and reconstruction metadata for the maintained SDK sources.
    fn inventory(root: &Path) -> Result<serde_json::Value> {
        serde_json::from_slice(
            &fs::read(root.join("sdk/sources.json"))
                .context("Failed to read SDK source inventory")?,
        )
        .context("Failed to decode SDK source inventory")
    }

    // Resolve the isolated build copy of a pinned vendor source tree.
    pub(super) fn directory(root: &Path, name: &str) -> Result<PathBuf> {
        let inventory =
            Self::inventory(root).context("Failed to read source directory inventory")?;
        let directory = inventory[name]["directory"]
            .as_str()
            .context("Failed to read source directory")?;
        Self::validate(directory).context("Failed to validate source directory")?;
        Ok(root.join("temp/sdk-source").join(directory))
    }

    // Reject absolute paths and parent traversal in the checked source inventory.
    fn validate(path: &str) -> Result<()> {
        ensure!(
            !path.is_empty()
                && Path::new(path)
                    .components()
                    .all(|part| matches!(part, Component::Normal(_))),
            "Failed to validate SDK source path: {path}"
        );
        Ok(())
    }

    // Verify maintained source bytes before copying them into an isolated build tree.
    pub(super) fn prepare(root: &Path) -> Result<()> {
        let inventory = Self::inventory(root).context("Failed to read SDK sources")?;
        for name in ["perforce", "jam"] {
            let record = &inventory[name];
            let directory = record["directory"]
                .as_str()
                .context("Failed to identify vendor directory")?;
            Self::validate(directory).context("Failed to validate vendor directory")?;
            let source = root.join("sdk/sources").join(directory);
            let destination =
                Self::directory(root, name).context("Failed to locate source cache")?;
            for (path, checksum) in record["files"]
                .as_object()
                .context("Failed to read source checksums")?
            {
                Self::validate(path).context("Failed to validate vendor source file")?;
                let bytes = fs::read(source.join(path))
                    .with_context(|| format!("Failed to read vendor source {path}"))?;
                let hash = format!("{:x}", sha2::Sha256::digest(&bytes));
                ensure!(
                    checksum.as_str() == Some(hash.as_str()),
                    "Failed to verify vendor source: {path}"
                );
                if !path.contains(".part") {
                    Self::install(&destination.join(path), &bytes)
                        .context("Failed to copy vendor source")?;
                }
            }
            Self::assemble(record, &source, &destination)
                .context("Failed to reconstruct vendor source")?;
        }
        Ok(())
    }

    // Restore oversized source files from plain byte fragments below the Git file limit.
    fn assemble(record: &serde_json::Value, source: &Path, destination: &Path) -> Result<()> {
        for (path, parts) in record["split_files"]
            .as_object()
            .context("Failed to read source fragments")?
        {
            Self::validate(path).context("Failed to validate reconstructed source path")?;
            let mut bytes = Vec::new();
            for part in parts
                .as_array()
                .context("Failed to list source fragments")?
            {
                let part = part
                    .as_str()
                    .context("Failed to identify source fragment")?;
                Self::validate(part).context("Failed to validate source fragment path")?;
                bytes
                    .extend(fs::read(source.join(part)).context("Failed to read source fragment")?);
            }
            Self::install(&destination.join(path), &bytes)
                .context("Failed to install reconstructed source")?;
        }
        Ok(())
    }

    // Reuse unchanged source files without invalidating the vendor's build cache.
    fn install(path: &Path, bytes: &[u8]) -> Result<()> {
        if path.is_file() && fs::read(path).context("Failed to inspect cached source")? == bytes {
            return Ok(());
        }
        fs::create_dir_all(path.parent().context("Failed to locate source parent")?)
            .context("Failed to create source directory")?;
        fs::write(path, bytes).context("Failed to install source bytes")
    }
}
