use super::command::Runner;
use crate::{
    error::{Result, ResultExt},
    platform::Platform,
};
use std::{env, path::Path, process::Command};

pub(crate) struct Dependencies;

impl Dependencies {
    // Locate installed tools without launching unlogged discovery processes.
    fn present(name: &str) -> bool {
        env::var_os("PATH").is_some_and(|path| {
            env::split_paths(&path).any(|directory| {
                directory.join(name).is_file() || directory.join(format!("{name}.exe")).is_file()
            })
        })
    }

    // Install native build dependencies for the selected runner and compiler.
    pub(crate) fn install(platform: &Platform) -> Result<()> {
        if !platform.windows() {
            if std::env::var_os("P4RUST_MANYLINUX").is_some() {
                Runner::run(
                    Command::new("git").args([
                        "config",
                        "--global",
                        "--add",
                        "safe.directory",
                        "/io",
                    ]),
                    false,
                )
                .context("Failed to register container checkout ownership")?;
                for tool in ["c++", "perl", "llvm-strip", "llvm-ar"] {
                    Runner::run(Command::new(tool).arg("--version"), true)
                        .with_context(|| format!("Failed to locate manylinux tool {tool}"))?;
                }
            } else if platform.apple() {
                Runner::run(
                    Command::new("brew").args(["install", "llvm", "nasm"]),
                    false,
                )
                .context("Failed to install Apple build tools")?;
            } else {
                Runner::run(Command::new("sudo").args(["apt-get", "update"]), false)
                    .context("Failed to refresh Linux packages")?;
                Runner::run(
                    Command::new("sudo").args([
                        "apt-get",
                        "install",
                        "-y",
                        "build-essential",
                        "perl",
                        "nasm",
                        "llvm",
                    ]),
                    false,
                )
                .context("Failed to install Linux build tools")?;
            }
        } else {
            for (tool, package, installed) in [
                ("perl", "strawberryperl", "C:/Strawberry/perl/bin/perl.exe"),
                (
                    "llvm-objcopy",
                    "llvm",
                    "C:/Program Files/LLVM/bin/llvm-objcopy.exe",
                ),
                ("nasm", "nasm", "C:/Program Files/NASM/nasm.exe"),
            ] {
                if (tool != "nasm" || platform.target.starts_with("x86_64"))
                    && !Self::present(tool)
                    && !Path::new(installed).is_file()
                {
                    Runner::run(
                        Command::new("choco").args(["install", package, "--yes", "--no-progress"]),
                        false,
                    )
                    .with_context(|| format!("Failed to install {package}"))?;
                }
            }
        }
        Runner::run(
            Command::new("rustup").args(["target", "add", &platform.target]),
            false,
        )
        .context("Failed to install Rust target")?;
        Ok(())
    }
}
