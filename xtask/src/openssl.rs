use crate::automation::command::Runner;
use crate::cache::Cache;
use crate::error::{Result, ResultExt};
use crate::platform::Platform;
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
        Runner::run(
            Command::new(program)
                .args(args)
                .current_dir(source)
                .envs(tool.env().iter().cloned()),
            false,
        )
        .with_context(|| format!("Failed to start OpenSSL production command {program}"))?;
        Ok(())
    }

    // Disable unrelated protocols while preserving the complete modern TLS algorithm set.
    fn configure(source: &Path, platform: &Platform) -> Result<()> {
        let mut options = Self::OPTIONS.to_vec();
        options[1] = match platform.target.as_str() {
            "x86_64-pc-windows-msvc" => "VC-WIN64A-P4RUST",
            "aarch64-pc-windows-msvc" => "VC-WIN64-ARM-P4RUST",
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
    pub(crate) fn build(root: &Path, platform: &Platform) -> Result<std::path::PathBuf> {
        let cache = Cache::openssl(root, platform).context("Failed to select OpenSSL cache")?;
        let names = platform.ssl_names();
        if let Some(path) = cache
            .restore(&[names[0], names[1], "include/openssl/ssl.h"])
            .context("Failed to inspect OpenSSL cache")?
        {
            return Ok(path);
        }
        let source = root
            .join("temp/native-production/openssl-compact")
            .join(&platform.target);
        if source.exists() {
            fs::remove_dir_all(&source)
                .context("Failed to clear incompatible OpenSSL build tree")?;
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
        cache
            .save(&source, &[names[0], names[1], "include"])
            .context("Failed to save compact OpenSSL cache")
    }
}
