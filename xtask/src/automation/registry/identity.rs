use crate::error::{Result, ResultExt};
use sha2::Digest;
use std::{fs, path::Path};

pub(super) struct Identity;

impl Identity {
    // Fingerprint native source inputs independently of public Rust code and package versions.
    pub(super) fn resource(root: &Path, target: &str) -> Result<String> {
        let mut digest = sha2::Sha256::new();
        Self::add(&mut digest, target, target.as_bytes());
        if target.ends_with("linux-gnu") {
            Self::tree(
                &mut digest,
                root,
                &root.join("xtask/src/automation/python/container.rs"),
            )
            .context("Failed to fingerprint manylinux production policy")?;
        }
        for relative in [
            "native",
            "xtask/src/sdk",
            "sdk/sources.json",
            "sdk/LICENSE",
            "xtask/src/producer.rs",
            "xtask/src/platform.rs",
            "xtask/src/prune.rs",
            "xtask/src/openssl.rs",
            "xtask/src/release.rs",
        ] {
            Self::tree(&mut digest, root, &root.join(relative))
                .with_context(|| format!("Failed to fingerprint {relative}"))?;
        }
        let resource = root.join("resources").join(target);
        for relative in ["src", "build.rs", "LICENSE", "Cargo.toml"] {
            Self::tree(&mut digest, root, &resource.join(relative))
                .with_context(|| format!("Failed to fingerprint resource {relative}"))?;
        }
        let lock = fs::read_to_string(root.join("Cargo.lock"))
            .context("Failed to read native dependency lock")?;
        for block in lock.split("[[package]]").filter(|block| {
            ["cc", "openssl-src", "find-msvc-tools", "shlex"]
                .iter()
                .any(|name| {
                    block
                        .trim_start()
                        .starts_with(&format!("name = \"{name}\"\n"))
                        || block
                            .trim_start()
                            .starts_with(&format!("name = \"{name}\"\r\n"))
                })
        }) {
            Self::add(
                &mut digest,
                "native-dependency",
                block.replace("\r\n", "\n").trim().as_bytes(),
            );
        }
        Ok(format!("{:x}", digest.finalize()))
    }

    // Traverse maintained text inputs in stable order and ignore generated native archives.
    fn tree(digest: &mut sha2::Sha256, root: &Path, path: &Path) -> Result<()> {
        if path.is_dir() {
            let mut paths = fs::read_dir(path)
                .context("Failed to enumerate native inputs")?
                .map(|entry| entry.map(|entry| entry.path()))
                .collect::<std::io::Result<Vec<_>>>()
                .context("Failed to read native input paths")?;
            paths.sort();
            for path in paths {
                if path
                    .file_name()
                    .is_some_and(|name| name == "lib" || name == "archives")
                {
                    continue;
                }
                Self::tree(digest, root, &path).context("Failed to fingerprint native input")?;
            }
        } else {
            let name = path
                .strip_prefix(root)
                .context("Failed to identify native input")?
                .to_string_lossy()
                .replace('\\', "/");
            let text = fs::read_to_string(path)
                .context("Failed to read native input")?
                .replace("\r\n", "\n");
            let text = if path.file_name().is_some_and(|name| name == "Cargo.toml") {
                text.lines()
                    .filter(|line| !line.starts_with("version = "))
                    .collect::<Vec<_>>()
                    .join("\n")
            } else {
                text
            };
            Self::add(digest, &name, text.as_bytes());
        }
        Ok(())
    }

    // Frame paths and payload lengths to keep distinct input sequences unambiguous.
    fn add(digest: &mut sha2::Sha256, name: &str, bytes: &[u8]) {
        digest.update(name.len().to_le_bytes());
        digest.update(name.as_bytes());
        digest.update(bytes.len().to_le_bytes());
        digest.update(bytes);
    }
}
