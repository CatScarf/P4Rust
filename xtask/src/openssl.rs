use crate::error::{Result, ResultExt, ensure};
use sha2::Digest;
use std::{fs, path::Path, process::Command};

pub(crate) struct OpenSsl;

impl OpenSsl {
    const OPTIONS: &[&str] = &[
        "Configure",
        "VC-WIN64A-P4RUST",
        "--release",
        "no-shared",
        "no-module",
        "no-tests",
        "no-comp",
        "no-zlib",
        "no-legacy",
        "no-ssl3",
        "no-md2",
        "no-rc5",
        "no-weak-ssl-ciphers",
        "no-idea",
        "no-seed",
        "no-capieng",
        "no-engine",
        "no-dso",
        "no-cmp",
        "no-cms",
        "no-ts",
        "no-quic",
        "no-dtls",
    ];

    // Copy the pinned Cargo source without modifying the shared registry cache.
    fn copy(source: &Path, destination: &Path) -> Result<()> {
        fs::create_dir_all(destination).context("Failed to create OpenSSL source directory")?;
        for entry in fs::read_dir(source).context("Failed to enumerate OpenSSL sources")? {
            let entry = entry.context("Failed to read OpenSSL source entry")?;
            let target = destination.join(entry.file_name());
            if entry
                .file_type()
                .context("Failed to inspect OpenSSL source")?
                .is_dir()
            {
                Self::copy(&entry.path(), &target)
                    .context("Failed to copy OpenSSL source subtree")?;
            } else {
                fs::copy(entry.path(), target).context("Failed to copy OpenSSL source file")?;
            }
        }
        Ok(())
    }

    // Run a production command with the selected Visual Studio environment.
    fn command(source: &Path, program: &str, args: &[&str]) -> Result<()> {
        let tool = cc::windows_registry::find_tool("x86_64-pc-windows-msvc", "cl.exe")
            .context("Failed to locate MSVC for OpenSSL production")?;
        let status = Command::new(program)
            .args(args)
            .current_dir(source)
            .envs(tool.env().iter().cloned())
            .status()
            .with_context(|| format!("Failed to start OpenSSL production command {program}"))?;
        ensure!(
            status.success(),
            "Failed to run OpenSSL production command {program}"
        );
        Ok(())
    }

    // Disable unrelated protocols while preserving the complete modern TLS algorithm set.
    fn configure(source: &Path) -> Result<()> {
        let configuration = source.join("Configurations/99-p4rust.conf");
        fs::write(configuration,
            "( 'VC-WIN64A-P4RUST' => { inherit_from => ['VC-WIN64A'], lib_cflags => '/MD', bin_cflags => '/MD', dso_cflags => '/MD', ASFLAGS => '' } );\n")
            .context("Failed to write release-only OpenSSL target")?;
        Self::command(source, "perl", Self::OPTIONS)
            .context("Failed to configure compact OpenSSL release")?;
        Ok(())
    }

    // Produce TLS archives without debug records or unrelated application protocols.
    pub(crate) fn build(output: &Path) -> Result<std::path::PathBuf> {
        let compiler = cc::windows_registry::find_tool("x86_64-pc-windows-msvc", "cl.exe")
            .context("Failed to identify OpenSSL cache toolchain")?;
        let policy = format!(
            "OpenSSL-3.6.3-MD-ASFLAGS-empty:{}:{}",
            compiler.path().display(),
            Self::OPTIONS.join(" ")
        );
        let key = format!("{:x}", sha2::Sha256::digest(policy));
        let source = output.join(key);
        if Self::cached(&source).context("Failed to inspect OpenSSL cache")? {
            return Ok(source);
        }
        Self::copy(&openssl_src::source_dir(), &source)
            .context("Failed to prepare pinned OpenSSL source")?;
        Self::configure(&source).context("Failed to prepare OpenSSL release configuration")?;
        if let Ok(jom) = std::env::var("P4RUST_JOM") {
            Self::command(&source, &jom, &["-j", "16", "build_libs"])
                .context("Failed to compile compact OpenSSL archives in parallel")?;
        } else {
            Self::command(&source, "nmake", &["/NOLOGO", "build_libs"])
                .context("Failed to compile compact OpenSSL archives")?;
        }
        let mut checksums = serde_json::Map::new();
        for name in ["libssl.lib", "libcrypto.lib"] {
            let bytes =
                fs::read(source.join(name)).context("Failed to read produced OpenSSL archive")?;
            checksums.insert(
                name.into(),
                format!("{:x}", sha2::Sha256::digest(bytes)).into(),
            );
        }
        fs::write(
            source.join("p4rust-cache.json"),
            serde_json::to_vec(&checksums).context("Failed to encode OpenSSL cache")?,
        )
        .context("Failed to save OpenSSL cache")?;
        Ok(source)
    }

    // Reuse only complete archives matching the current production policy and hashes.
    fn cached(source: &Path) -> Result<bool> {
        let inventory = source.join("p4rust-cache.json");
        if !inventory.is_file() {
            return Ok(false);
        }
        let checksums: serde_json::Value =
            serde_json::from_slice(&fs::read(inventory).context("Failed to read OpenSSL cache")?)
                .context("Failed to decode OpenSSL cache")?;
        for name in ["libssl.lib", "libcrypto.lib"] {
            let path = source.join(name);
            if !path.is_file() {
                return Ok(false);
            }
            let hash = format!(
                "{:x}",
                sha2::Sha256::digest(
                    fs::read(path).context("Failed to hash cached OpenSSL library")?
                )
            );
            if checksums[name].as_str() != Some(hash.as_str()) {
                return Ok(false);
            }
        }
        println!("Reusing verified OpenSSL production cache");
        Ok(true)
    }
}
