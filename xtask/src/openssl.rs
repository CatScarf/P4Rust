use crate::error::{Result, ResultExt, ensure};
use crate::platform::Platform;
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

    // Run a production command with the selected production compiler environment.
    fn command(source: &Path, program: &str, args: &[&str], platform: &Platform) -> Result<()> {
        let tool = platform
            .compiler()
            .try_get_compiler()
            .context("Failed to locate OpenSSL production compiler")?;
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
    fn configure(source: &Path, platform: &Platform) -> Result<()> {
        let mut options = Self::OPTIONS.to_vec();
        options[1] = match platform.target.as_str() {
            "x86_64-pc-windows-msvc" => "VC-WIN64A-P4RUST",
            "aarch64-pc-windows-msvc" => "VC-WIN64-ARM-P4RUST",
            "x86_64-pc-windows-gnu" => "mingw64",
            "x86_64-unknown-linux-gnu" => "linux-x86_64",
            "aarch64-unknown-linux-gnu" => "linux-aarch64",
            "x86_64-apple-darwin" => "darwin64-x86_64-cc",
            "aarch64-apple-darwin" => "darwin64-arm64-cc",
            _ => return Err(crate::error::Error::new("Failed to select OpenSSL target")),
        };
        if platform.msvc() {
            let parent = options[1].trim_end_matches("-P4RUST");
            fs::write(source.join("Configurations/99-p4rust.conf"), format!(
                "( '{}' => {{ inherit_from => ['{parent}'], lib_cflags => '/MD', bin_cflags => '/MD', dso_cflags => '/MD', ASFLAGS => '' }} );\n", options[1]))
                .context("Failed to write release-only OpenSSL target")?;
        } else {
            options.extend(["-O2", "-fPIC"]);
        }
        Self::command(source, "perl", &options, platform)
            .context("Failed to configure compact OpenSSL release")?;
        Ok(())
    }

    // Produce TLS archives without debug records or unrelated application protocols.
    pub(crate) fn build(output: &Path, platform: &Platform) -> Result<std::path::PathBuf> {
        let compiler = platform
            .compiler()
            .try_get_compiler()
            .context("Failed to identify OpenSSL cache toolchain")?;
        let policy = if platform.target == "x86_64-pc-windows-msvc" {
            format!(
                "OpenSSL-3.6.3-MD-ASFLAGS-empty:{}:{}",
                compiler.path().display(),
                Self::OPTIONS.join(" ")
            )
        } else {
            format!(
                "OpenSSL-3.6.3-release:{}:{}:{}",
                platform.target,
                compiler.path().display(),
                Self::OPTIONS.join(" ")
            )
        };
        let key = format!("{:x}", sha2::Sha256::digest(policy));
        let source = output.join(key);
        if Self::cached(&source, platform).context("Failed to inspect OpenSSL cache")? {
            return Ok(source);
        }
        Self::copy(&openssl_src::source_dir(), &source)
            .context("Failed to prepare pinned OpenSSL source")?;
        Self::configure(&source, platform)
            .context("Failed to prepare OpenSSL release configuration")?;
        if platform.msvc() {
            if let Ok(jom) = std::env::var("P4RUST_JOM") {
                Self::command(&source, &jom, &["-j", "4", "build_libs"], platform)
                    .context("Failed to compile OpenSSL in parallel")?;
            } else {
                Self::command(&source, "nmake", &["/NOLOGO", "build_libs"], platform)
                    .context("Failed to compile OpenSSL")?;
            }
        } else {
            let jobs = std::thread::available_parallelism()
                .context("Failed to select OpenSSL parallelism")?
                .get()
                .to_string();
            Self::command(&source, "make", &["-j", &jobs, "build_libs"], platform)
                .context("Failed to compile OpenSSL")?;
        }
        let mut checksums = serde_json::Map::new();
        for name in platform.ssl_names() {
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
    fn cached(source: &Path, platform: &Platform) -> Result<bool> {
        let inventory = source.join("p4rust-cache.json");
        if !inventory.is_file() {
            return Ok(false);
        }
        let checksums: serde_json::Value =
            serde_json::from_slice(&fs::read(inventory).context("Failed to read OpenSSL cache")?)
                .context("Failed to decode OpenSSL cache")?;
        for name in platform.ssl_names() {
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
