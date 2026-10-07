mod identity;
use crate::{
    error::{Result, ResultExt},
    platform::Platform,
};
use sha2::Digest;
use std::{
    fs,
    path::{Component, Path, PathBuf},
};

pub(crate) struct Cache {
    pub(crate) key: String,
    pub(crate) directory: PathBuf,
}

impl Cache {
    // Identify TLS archives independently of Rust sources and the bridge implementation.
    pub(crate) fn openssl(root: &Path, platform: &Platform) -> Result<Self> {
        let mut identity = identity::Identity::new(root, platform)
            .context("Failed to identify OpenSSL toolchain")?;
        identity
            .files(root, &["xtask/src/openssl.rs"])
            .context("Failed to identify OpenSSL build policy")?;
        let source = openssl_src::source_dir();
        identity
            .files(&source, &["VERSION.dat", "Configure"])
            .context("Failed to identify pinned OpenSSL source")?;
        identity
            .files(
                source
                    .parent()
                    .context("Failed to locate OpenSSL source package")?,
                &["Cargo.toml"],
            )
            .context("Failed to identify pinned OpenSSL source package")?;
        Ok(Self::new(root, "openssl", platform, identity.finish()))
    }

    // Include vendor sources, compatibility fixes, and the TLS policy in the SDK key.
    pub(crate) fn sdk(root: &Path, platform: &Platform) -> Result<Self> {
        let mut identity =
            identity::Identity::new(root, platform).context("Failed to identify SDK toolchain")?;
        identity
            .files(
                root,
                &[
                    "sdk/sources.json",
                    "xtask/src/sdk/mod.rs",
                    "xtask/src/sdk/source.rs",
                    "xtask/src/sdk/jam.rs",
                    "native/sdk_hooks.h",
                ],
            )
            .context("Failed to identify SDK source and build policy")?;
        identity.add(
            Self::openssl(root, platform)
                .context("Failed to identify SDK TLS dependency")?
                .key
                .as_bytes(),
        );
        Ok(Self::new(root, "sdk", platform, identity.finish()))
    }

    // Keep only portable production payloads in a separate ignored cache directory.
    fn new(root: &Path, name: &str, platform: &Platform, hash: String) -> Self {
        Self {
            key: format!("p4rust-native-v1-{name}-{}-{hash}", platform.target),
            directory: root.join("temp/native-cache").join(name),
        }
    }

    // Reuse complete payloads only after checking policy identity and every saved checksum.
    pub(crate) fn restore(&self, required: &[&str]) -> Result<Option<PathBuf>> {
        let manifest = self.directory.join("cache.json");
        if !manifest.is_file() {
            println!("Native cache miss: {}", self.key);
            return Ok(None);
        }
        let bytes = fs::read(manifest).context("Failed to read native cache manifest")?;
        let record: serde_json::Value = match serde_json::from_slice(&bytes) {
            Ok(record) => record,
            Err(error) => {
                println!("Rebuilding malformed native cache: {error}");
                return Ok(None);
            }
        };
        let Some(checksums) = record["files"].as_object() else {
            return Ok(None);
        };
        if record["key"] != self.key || required.iter().any(|name| !checksums.contains_key(*name)) {
            return Ok(None);
        }
        for (name, expected) in checksums {
            if !Path::new(name)
                .components()
                .all(|part| matches!(part, Component::Normal(_)))
            {
                return Ok(None);
            }
            let path = self.directory.join(name);
            if !path.is_file() {
                return Ok(None);
            }
            let hash = Self::hash(&path).context("Failed to validate cached native file")?;
            if expected.as_str() != Some(hash.as_str()) {
                println!("Rebuilding native cache with invalid checksum: {name}");
                return Ok(None);
            }
        }
        println!("Reusing verified native cache: {}", self.key);
        Ok(Some(self.directory.clone()))
    }

    // Save only libraries and required headers, never compiler objects or build trees.
    pub(crate) fn save(&self, source: &Path, names: &[&str]) -> Result<PathBuf> {
        if self.directory.exists() {
            fs::remove_dir_all(&self.directory)
                .context("Failed to replace native cache payload")?;
        }
        fs::create_dir_all(&self.directory).context("Failed to create native cache payload")?;
        let mut checksums = serde_json::Map::new();
        for name in names {
            self.copy(source, Path::new(name), &mut checksums)
                .context("Failed to save native cache file")?;
        }
        let record = serde_json::json!({"key": self.key, "files": checksums});
        fs::write(
            self.directory.join("cache.json"),
            serde_json::to_vec(&record).context("Failed to encode native cache manifest")?,
        )
        .context("Failed to finish native cache manifest")?;
        println!("Saved native cache payload: {}", self.key);
        Ok(self.directory.clone())
    }

    // Copy selected payload trees and record their relative paths and content hashes.
    fn copy(
        &self,
        source: &Path,
        relative: &Path,
        hashes: &mut serde_json::Map<String, serde_json::Value>,
    ) -> Result<()> {
        let path = source.join(relative);
        let target = self.directory.join(relative);
        if path.is_dir() {
            fs::create_dir_all(&target).context("Failed to create cached header directory")?;
            for entry in fs::read_dir(path).context("Failed to enumerate native cache headers")? {
                let entry = entry.context("Failed to read native cache header entry")?;
                self.copy(source, &relative.join(entry.file_name()), hashes)
                    .context("Failed to copy native cache header tree")?;
            }
        } else {
            fs::copy(&path, target).context("Failed to copy native cache payload")?;
            hashes.insert(
                relative.to_string_lossy().replace('\\', "/"),
                Self::hash(&path)
                    .context("Failed to checksum native cache payload")?
                    .into(),
            );
        }
        Ok(())
    }

    // Hash exact file bytes instead of trusting modification times or archive sizes.
    fn hash(path: &Path) -> Result<String> {
        Ok(format!(
            "{:x}",
            sha2::Sha256::digest(
                fs::read(path).context("Failed to read native cache checksum input")?
            )
        ))
    }
}
