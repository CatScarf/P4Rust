use crate::{
    Producer,
    error::{Error, Result, ResultExt},
    sdk::Sdk,
};
mod bundle;
mod cache;
pub(crate) mod command;
mod dependencies;
mod github;
mod python;
mod registry;
use command::Runner;
use std::{
    path::{Path, PathBuf},
    process::Command,
};

pub(crate) struct Task;

impl Task {
    // Prepare runner dependencies once and produce a checked distributable crate.
    fn ci(root: &Path) -> Result<()> {
        if python::Container::dispatch(root, "ci")? {
            return Ok(());
        }
        let platform =
            crate::platform::Platform::selected().context("Failed to select CI target")?;
        cache::Preparation::dependencies(root, &platform)
            .context("Failed to prepare CI dependencies")?;
        Sdk::prepare(root).context("Failed to prepare CI SDK")?;
        if !registry::Registry::prepare(root, &platform)
            .context("Failed to select reusable CI native libraries")?
        {
            Producer::run().context("Failed to build CI native libraries")?;
        }
        python::Python::prepare(root).context("Failed to prepare Python packaging")?;
        Self::cargo(root, "build", &["--release".into()])
            .context("Failed to build CI Rust library")?;
        Self::cargo(root, "check", &[]).context("Failed CI Clippy")?;
        Self::cargo(root, "package", &[]).context("Failed to package CI crate")?;
        python::Python::build(root).context("Failed to build typed Python wheels")
    }
    // Find the repository without depending on the invocation directory.
    fn root() -> Result<PathBuf> {
        Path::new(env!("CARGO_MANIFEST_DIR"))
            .parent()
            .context("Failed to locate xtask repository")
            .map(Path::to_path_buf)
    }

    // Run a child process and propagate both launch and exit failures.
    fn command(root: &Path, program: &str, args: &[String]) -> Result<()> {
        Runner::run(Command::new(program).args(args).current_dir(root), false)
            .with_context(|| format!("Failed to run {program}"))?;
        Ok(())
    }

    // Dispatch explicit maintainer operations without invoking native compilers implicitly.
    pub(crate) fn run() -> Result<()> {
        let root = Self::root().context("Failed to initialize xtask")?;
        let args: Vec<String> = std::env::args().skip(1).collect();
        let command = args.first().map(String::as_str).unwrap_or("help");
        match command {
            "ci-cache" if args.len() == 1 => {
                if python::Container::dispatch(&root, "ci-cache")? {
                    return Ok(());
                }
                cache::Preparation::run(&root).context("Failed to prepare CI cache keys")
            }
            "ci" if args.len() == 1 => Self::ci(&root).context("Failed to build CI package"),
            "publish" if args.len() == 1 => {
                github::GitHub::publish(&root).context("Failed to publish release")
            }
            "publish-python" if args.len() == 1 => {
                python::publish::Publisher::run(&root).context("Failed to publish Python wheels")
            }
            "publish-plan" if args.len() == 1 => {
                registry::Registry::preview(&root).context("Failed to preview registry publication")
            }
            "prepare"
                if args.len() == 1
                    || args.get(1).is_some_and(|arg| arg == "--all") && args.len() == 2 =>
            {
                Sdk::prepare(&root).context("Failed to prepare build inputs")
            }
            "native" => {
                Sdk::prepare(&root).context("Failed to prepare native production inputs")?;
                Producer::run().context("Failed to rebuild native libraries")?;
                Ok(())
            }
            "build" | "check" | "package" => {
                Sdk::prepare(&root).context("Failed to prepare Cargo inputs")?;
                Producer::run().context("Failed to prepare native release libraries")?;
                Self::cargo(&root, command, &args[1..]).context("Failed to execute Cargo workflow")
            }
            "help" if args.len() <= 1 => {
                println!(
                    "cargo xtask <prepare|build [Cargo options]|check|package|ci-cache|ci|publish|publish-python|publish-plan|native [--openssl-lib-dir <cache>]>"
                );
                Ok(())
            }
            _ => Err(Error::new(
                "Failed to select xtask command; run cargo xtask help",
            )),
        }
    }

    // Keep packaging and validation independent of the native producer toolchain.
    fn cargo(root: &Path, command: &str, extra: &[String]) -> Result<()> {
        let mut args: Vec<String> = match command {
            "build" => vec!["build", "-p", "p4rust"],
            "check" => vec![
                "clippy",
                "--workspace",
                "--all-targets",
                "--all-features",
                "--",
                "-D",
                "warnings",
            ],
            "package" => vec![
                "package",
                "--workspace",
                "--exclude",
                "xtask",
                "--exclude",
                "p4rust-python",
                "--offline",
                "--allow-dirty",
                "--exclude-lockfile",
            ],
            _ => return Err(Error::new("Failed to select Cargo workflow")),
        }
        .into_iter()
        .map(str::to_owned)
        .collect();
        if command != "build" && !extra.is_empty() {
            return Err(Error::new("Failed to validate xtask options"));
        }
        let platform =
            crate::platform::Platform::selected().context("Failed to select Cargo target")?;
        if command == "check" {
            let separator = args
                .iter()
                .position(|arg| arg == "--")
                .context("Failed to locate Clippy option separator")?;
            args.splice(
                separator..separator,
                ["--target".into(), platform.target.clone()],
            );
        } else {
            args.extend(["--target".into(), platform.target.clone()]);
        }
        args.extend_from_slice(extra);
        if command == "check" {
            let python = python::Python::interpreter(root, true)
                .context("Failed to select Clippy Python interpreter")?;
            Runner::run(
                Command::new("cargo")
                    .args(&args)
                    .current_dir(root)
                    .env("PYO3_PYTHON", python),
                false,
            )
            .context("Failed to run workspace Clippy")?;
        } else {
            Self::command(root, "cargo", &args).context("Failed to run Cargo")?;
        }
        if command == "package" {
            Self::package_limit(root, &platform).context("Failed to validate package size")?;
        }
        Ok(())
    }

    // Reject oversized distributable crates using Cargo's actual package directory.
    fn package_limit(root: &Path, platform: &crate::platform::Platform) -> Result<()> {
        let metadata = Self::metadata(root).context("Failed to read package metadata")?;
        let directory = metadata["target_directory"]
            .as_str()
            .context("Failed to locate Cargo output directory")?;
        let packages = metadata["packages"]
            .as_array()
            .context("Failed to inspect resource versions")?;
        let destination = root.join("temp/ci-artifacts");
        std::fs::create_dir_all(&destination).context("Failed to create CI artifact directory")?;
        for name in [
            "p4rust".to_owned(),
            format!("p4rust-resources-{}", platform.target),
        ] {
            let version = packages
                .iter()
                .find(|package| package["name"] == name)
                .context("Failed to locate independently versioned package")?["version"]
                .as_str()
                .context("Failed to read independent package version")?;
            let archive = Path::new(directory).join(format!("package/{name}-{version}.crate"));
            Self::stage_package(&archive, &destination)
                .context("Failed to stage release package")?;
        }
        Ok(())
    }

    // Validate each compressed crate and stage only the current platform's resources.
    fn stage_package(archive: &Path, destination: &Path) -> Result<()> {
        let bytes = std::fs::metadata(archive)
            .context("Failed to inspect packaged crate")?
            .len();
        if bytes >= 10_000_000 {
            return Err(Error::new(format!(
                "Failed to meet package size limit: {bytes} bytes"
            )));
        }
        std::fs::copy(
            archive,
            destination.join(
                archive
                    .file_name()
                    .context("Failed to identify packaged crate")?,
            ),
        )
        .context("Failed to copy CI package")?;
        println!("{}: {bytes} bytes (limit: 10000000)", archive.display());
        Ok(())
    }

    // Read Cargo's package version and output paths without compiling native code.
    fn metadata(root: &Path) -> Result<serde_json::Value> {
        let output = Runner::run(
            Command::new("cargo")
                .args(["metadata", "--no-deps", "--format-version", "1"])
                .current_dir(root),
            true,
        )
        .context("Failed to query Cargo metadata")?;
        serde_json::from_slice(&output.stdout).context("Failed to decode Cargo metadata")
    }
}
