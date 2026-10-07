use crate::{
    Producer,
    archive::Archives,
    error::{Error, Result, ResultExt},
};
pub(crate) mod command;
mod dependencies;
mod github;
use command::Runner;
use std::{
    path::{Path, PathBuf},
    process::Command,
};

pub(crate) struct Task;

impl Task {
    // Prepare runner dependencies once and produce a checked distributable crate.
    fn ci(root: &Path) -> Result<()> {
        let platform =
            crate::platform::Platform::selected().context("Failed to select CI target")?;
        dependencies::Dependencies::install(&platform)
            .context("Failed to prepare CI dependencies")?;
        Archives::prepare(root, false).context("Failed to prepare CI SDK")?;
        Producer::run().context("Failed to build CI native libraries")?;
        Self::cargo(root, "build", &["--release".into()])
            .context("Failed to build CI Rust library")?;
        Self::cargo(root, "check", &[]).context("Failed CI Clippy")?;
        Self::cargo(root, "package", &[]).context("Failed to package CI crate")
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
            "ci" if args.len() == 1 => Self::ci(&root).context("Failed to build CI package"),
            "publish" if args.len() == 1 => {
                github::GitHub::publish(&root).context("Failed to publish release")
            }
            "archive" => Archives::refresh(&root).context("Failed to refresh compressed libraries"),
            "prepare"
                if args.len() == 1
                    || args.get(1).is_some_and(|arg| arg == "--all") && args.len() == 2 =>
            {
                Archives::prepare(&root, args.len() == 2).context("Failed to prepare build inputs")
            }
            "native" => {
                Archives::prepare(&root, false)
                    .context("Failed to prepare native production inputs")?;
                Producer::run().context("Failed to rebuild native libraries")?;
                Ok(())
            }
            "build" | "check" | "package" => {
                Archives::prepare(&root, false).context("Failed to prepare Cargo inputs")?;
                Producer::run().context("Failed to prepare native release libraries")?;
                Self::cargo(&root, command, &args[1..]).context("Failed to execute Cargo workflow")
            }
            "help" if args.len() <= 1 => {
                println!(
                    "cargo xtask <prepare [--all]|build [Cargo options]|check|package|ci|publish|native [--openssl-lib-dir <cache>]|archive>"
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
            "package" => vec!["package", "-p", "p4rust", "--offline", "--allow-dirty"],
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
        Self::command(root, "cargo", &args).context("Failed to run Cargo")?;
        if command == "package" {
            Self::package_limit(root).context("Failed to validate package size")?;
        }
        Ok(())
    }

    // Reject oversized distributable crates using Cargo's actual package directory.
    fn package_limit(root: &Path) -> Result<()> {
        let metadata = Self::metadata(root).context("Failed to read package metadata")?;
        let package = metadata["packages"]
            .as_array()
            .context("Failed to read Cargo packages")?
            .iter()
            .find(|package| package["name"].as_str() == Some("p4rust"))
            .context("Failed to locate public crate")?;
        let version = package["version"]
            .as_str()
            .context("Failed to read public crate version")?;
        let directory = metadata["target_directory"]
            .as_str()
            .context("Failed to locate Cargo output directory")?;
        let archive = Path::new(directory).join(format!("package/p4rust-{version}.crate"));
        let bytes = std::fs::metadata(archive)
            .context("Failed to inspect packaged crate")?
            .len();
        if bytes >= 10_000_000 {
            return Err(Error::new(format!(
                "Failed to meet package size limit: {bytes} bytes"
            )));
        }
        println!("Public crate: {bytes} bytes (limit: 10000000)");
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
