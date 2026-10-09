mod container;
pub(super) mod publish;
pub(super) mod wheels;
use super::{Task, command::Runner};
use crate::{
    error::{Result, ResultExt, ensure},
    platform::Platform,
};
pub(crate) use container::Container;
use std::{
    fs,
    path::{Path, PathBuf},
    process::Command,
};

pub(crate) struct Python;

impl Python {
    const UV: &'static str = "0.12.24";
    const MATURIN: &'static str = "maturin==1.15.0";
    const PYRIGHT: &'static str = "pyright==1.1.414";

    // Install pinned packaging tools and managed interpreters without modifying system Python.
    pub(crate) fn prepare(root: &Path) -> Result<()> {
        let platform = Platform::selected().context("Failed to select Python platform")?;
        Self::tools(root).context("Failed to prepare Python tools")?;
        if std::env::var_os("P4RUST_MANYLINUX").is_none() {
            Self::run(
                root,
                &["python", "install", "--no-bin", "3.14", "3.15t"],
                false,
            )
            .context("Failed to install managed Python interpreters")?;
        }
        Self::licenses(root, &platform).context("Failed to stage Python native licenses")?;
        publish::Fingerprint::stamp(root).context("Failed to stamp Python build inputs")?;
        Ok(())
    }

    // Download only the publishing executable when an interpreter is unnecessary.
    fn tools(root: &Path) -> Result<()> {
        let platform = Platform::selected().context("Failed to select Python tools platform")?;
        let directory = root.join("temp/python-tools");
        fs::create_dir_all(&directory).context("Failed to create Python tools directory")?;
        let uv = Self::uv(root)?;
        if !uv.is_file() {
            let suffix = if platform.windows() { "zip" } else { "tar.gz" };
            let archive = directory.join(format!("uv.{suffix}"));
            let url = format!(
                "https://github.com/astral-sh/uv/releases/download/{}/uv-{}.{suffix}",
                Self::UV,
                Self::distribution(&platform)
            );
            Runner::run(
                Command::new("curl")
                    .args(["--fail", "--location", "--retry", "3", "--output"])
                    .arg(&archive)
                    .arg(url),
                false,
            )
            .context("Failed to download uv")?;
            Runner::run(
                Command::new("tar")
                    .arg("-xf")
                    .arg(&archive)
                    .arg("-C")
                    .arg(&directory),
                false,
            )
            .context("Failed to extract uv")?;
        }
        Ok(())
    }

    // Locate the uv binary in its upstream platform archive layout.
    fn uv(root: &Path) -> Result<PathBuf> {
        let platform = Platform::selected().context("Failed to select uv binary")?;
        let directory = root.join("temp/python-tools");
        Ok(if platform.windows() {
            directory.join("uv.exe")
        } else {
            directory.join(format!("uv-{}/uv", Self::distribution(&platform)))
        })
    }

    // Use a static uv executable inside either Linux runtime baseline.
    fn distribution(platform: &Platform) -> String {
        if platform.host.ends_with("linux-gnu") {
            platform.host.replace("linux-gnu", "linux-musl")
        } else {
            platform.host.clone()
        }
    }

    // Configure isolated tool environments without changing the machine's Python installation.
    fn command(root: &Path, args: &[&str]) -> Result<Command> {
        let mut command = Command::new(Self::uv(root)?);
        command
            .args(args)
            .current_dir(root)
            .env("UV_PYTHON_INSTALL_DIR", root.join("temp/python-runtime"))
            .env("UV_CACHE_DIR", root.join("temp/python-cache"))
            .env("PYTHONPATH", root.join("temp/python-types"));
        Ok(command)
    }

    // Execute all Python tooling through the shared command logger.
    fn run(root: &Path, args: &[&str], capture: bool) -> Result<std::process::Output> {
        Runner::run(&mut Self::command(root, args)?, capture)
            .context("Failed to run Python tooling")
    }

    // Select native interpreters from manylinux or the managed uv installation.
    pub(crate) fn interpreter(root: &Path, free_threaded: bool) -> Result<PathBuf> {
        if std::env::var_os("P4RUST_MANYLINUX").is_some() {
            let path = PathBuf::from(if free_threaded {
                "/opt/python/cp315-cp315t/bin/python"
            } else {
                "/opt/python/cp314-cp314/bin/python"
            });
            ensure!(
                path.is_file(),
                "Failed to locate manylinux interpreter: {}",
                path.display()
            );
            return Ok(path);
        }
        let request = if free_threaded { "3.15t" } else { "3.14" };
        let output = Self::run(root, &["python", "find", "--managed-python", request], true)?;
        Ok(PathBuf::from(
            String::from_utf8(output.stdout)
                .context("Failed to decode managed Python path")?
                .trim(),
        ))
    }

    // Include all native third-party notices in both Python wheels.
    fn licenses(root: &Path, platform: &Platform) -> Result<()> {
        let destination = root.join("python/python/p4rust/licenses");
        fs::create_dir_all(&destination).context("Failed to create Python license directory")?;
        fs::copy(root.join("LICENSE"), destination.join("MIT.txt"))
            .context("Failed to copy binding license")?;
        fs::copy(root.join("sdk/LICENSE"), destination.join("Perforce.txt"))
            .context("Failed to copy SDK license")?;
        for entry in fs::read_dir(root.join("resources").join(&platform.target))
            .context("Failed to list resource notices")?
        {
            let entry = entry.context("Failed to inspect resource notice")?;
            if entry
                .file_name()
                .to_string_lossy()
                .ends_with("-LICENSE.txt")
            {
                fs::copy(entry.path(), destination.join(entry.file_name()))
                    .context("Failed to copy native license")?;
            }
        }
        Ok(())
    }

    // Produce two stable ABI wheels and statically validate complete public typing.
    pub(crate) fn build(root: &Path) -> Result<()> {
        let metadata = Task::metadata(root).context("Failed to inspect Python versions")?;
        let packages = metadata["packages"]
            .as_array()
            .context("Failed to read package versions")?;
        let public = packages
            .iter()
            .find(|p| p["name"] == "p4rust")
            .context("Failed to find Rust version")?;
        let binding = packages
            .iter()
            .find(|p| p["name"] == "p4rust-python")
            .context("Failed to find Python version")?;
        ensure!(
            public["version"] == binding["version"],
            "Failed to match Rust and Python versions"
        );
        for (free_threaded, feature) in [(false, "abi3"), (true, "abi3t")] {
            Self::wheel(root, free_threaded, feature).context("Failed to compile Python wheel")?;
        }
        Self::typing(root).context("Failed to validate Python typing")
    }

    // Build one actual stable ABI binary with a compatible native interpreter.
    fn wheel(root: &Path, free_threaded: bool, feature: &str) -> Result<()> {
        let platform = Platform::selected().context("Failed to select wheel target")?;
        let interpreter = Self::interpreter(root, free_threaded)?;
        let interpreter = interpreter
            .to_str()
            .context("Failed to encode Python interpreter")?;
        let mut args = vec![
            "run",
            "--no-project",
            "--python",
            interpreter,
            "--with",
            Self::MATURIN,
            "maturin",
            "build",
            "--release",
            "--target",
            &platform.target,
            "--strip",
            "--manifest-path",
            "python/Cargo.toml",
            "--no-default-features",
            "--features",
            feature,
            "--interpreter",
            interpreter,
            "--out",
            "temp/ci-artifacts",
        ];
        if std::env::var_os("P4RUST_MANYLINUX").is_some() {
            args.extend(["--manylinux", "2_28"]);
        }
        let mut command = Self::command(root, &args)?;
        if let Some(config) = wheels::Wheels::configuration(root, &platform.target, free_threaded)?
        {
            command.env("PYO3_CONFIG_FILE", config);
        }
        Runner::run(&mut command, false).context("Failed to compile Python wheel")?;
        Ok(())
    }

    // Require strict source typing and complete types in the packaged distribution.
    fn typing(root: &Path) -> Result<()> {
        let interpreter = Self::interpreter(root, false)?;
        let interpreter = interpreter
            .to_str()
            .context("Failed to encode typing interpreter")?;
        Self::run(
            root,
            &[
                "run",
                "--no-project",
                "--python",
                interpreter,
                "--with",
                Self::PYRIGHT,
                "pyright",
                "--project",
                "python/pyproject.toml",
            ],
            false,
        )
        .context("Failed strict Python type checking")?;
        let wheel = wheels::Wheels::regular(root).context("Failed to locate typed wheel")?;
        wheels::Wheels::extract(&wheel, &root.join("temp/python-types"))
            .context("Failed to prepare packaged typing")?;
        Self::run(
            root,
            &[
                "run",
                "--no-project",
                "--python",
                interpreter,
                "--with",
                Self::PYRIGHT,
                "pyright",
                "--pythonpath",
                interpreter,
                "--verifytypes",
                "p4rust",
                "--ignoreexternal",
            ],
            false,
        )
        .context("Failed Python public type completeness")?;
        Ok(())
    }
}
