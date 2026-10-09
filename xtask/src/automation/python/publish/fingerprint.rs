use crate::{
    automation::registry::identity::Identity,
    error::{Result, ResultExt},
    platform::Platform,
};
use sha2::Digest;
use std::{fs, io::Read, path::Path};

pub(crate) struct Fingerprint;

impl Fingerprint {
    // Identify maintained bindings, locked dependencies, and all native resource inputs.
    pub(super) fn current(root: &Path) -> Result<String> {
        let mut digest = sha2::Sha256::new();
        for relative in [
            "Cargo.toml",
            "Cargo.lock",
            "LICENSE",
            "README.md",
            "src",
            "python/Cargo.toml",
            "python/pyproject.toml",
            "python/src",
            "python/python/p4rust",
        ] {
            Self::tree(&mut digest, root, &root.join(relative))
                .with_context(|| format!("Failed to fingerprint Python input {relative}"))?;
        }
        Self::add(
            &mut digest,
            "wheel-policy",
            super::super::Python::MATURIN.as_bytes(),
        );
        Self::add(
            &mut digest,
            "wheel-policy",
            b"cp39-abi3;cp315-abi3t;release;strip;manylinux_2_28;macos_12",
        );
        for target in Platform::TARGETS {
            let native = Identity::resource(root, target)
                .context("Failed to fingerprint native Python input")?;
            Self::add(&mut digest, target, native.as_bytes());
        }
        Ok(format!("{:x}", digest.finalize()))
    }

    // Embed a source fingerprint into both wheels without recording compiler output in Git.
    pub(crate) fn stamp(root: &Path) -> Result<()> {
        fs::write(
            root.join("python/python/p4rust/_inputs.sha256"),
            Self::current(root)?,
        )
        .context("Failed to write Python input fingerprint")
    }

    // Read the recorded input identity from a published or candidate wheel.
    pub(super) fn read(path: &Path) -> Result<String> {
        let file = fs::File::open(path).context("Failed to open fingerprinted wheel")?;
        let mut archive =
            zip::ZipArchive::new(file).context("Failed to inspect wheel fingerprint")?;
        let mut value = String::new();
        archive
            .by_name("p4rust/_inputs.sha256")
            .context("Failed to find wheel input fingerprint")?
            .read_to_string(&mut value)
            .context("Failed to read wheel input fingerprint")?;
        Ok(value)
    }

    // Traverse maintained UTF-8 files in stable order across native runner platforms.
    fn tree(digest: &mut sha2::Sha256, root: &Path, path: &Path) -> Result<()> {
        if path.is_dir() {
            let mut paths = fs::read_dir(path)
                .context("Failed to enumerate Python inputs")?
                .map(|entry| entry.map(|entry| entry.path()))
                .collect::<std::io::Result<Vec<_>>>()
                .context("Failed to read Python input paths")?;
            paths.sort();
            for path in paths {
                if path
                    .file_name()
                    .is_some_and(|name| name == "licenses" || name == "__pycache__")
                    || path.extension().is_some_and(|extension| {
                        ["so", "pyd", "pyc", "sha256"]
                            .contains(&extension.to_string_lossy().as_ref())
                    })
                {
                    continue;
                }
                Self::tree(digest, root, &path)
                    .context("Failed to fingerprint maintained Python source")?;
            }
        } else {
            let name = path
                .strip_prefix(root)
                .context("Failed to identify Python input path")?
                .to_string_lossy()
                .replace('\\', "/");
            let text = fs::read_to_string(path)
                .context("Failed to read Python input")?
                .replace("\r\n", "\n");
            Self::add(digest, &name, text.as_bytes());
        }
        Ok(())
    }

    // Frame paths and contents independently of operating system line endings.
    fn add(digest: &mut sha2::Sha256, name: &str, bytes: &[u8]) {
        digest.update((name.len() as u64).to_le_bytes());
        digest.update(name.as_bytes());
        digest.update((bytes.len() as u64).to_le_bytes());
        digest.update(bytes);
    }
}
