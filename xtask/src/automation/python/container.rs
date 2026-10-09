use crate::{
    automation::command::Runner,
    error::{Result, ResultExt},
    platform::Platform,
};
use std::{env, path::Path, process::Command};

pub(crate) struct Container;

impl Container {
    // Execute Linux production inside the same manylinux baseline for crates and wheels.
    pub(crate) fn dispatch(root: &Path, task: &str) -> Result<bool> {
        let platform = Platform::selected().context("Failed to select container platform")?;
        if !platform.target.ends_with("linux-gnu") || env::var_os("P4RUST_MANYLINUX").is_some() {
            return Ok(false);
        }
        let home = env::var_os("HOME").context("Failed to locate runner home")?;
        let cargo = env::var_os("CARGO_HOME")
            .map(std::path::PathBuf::from)
            .unwrap_or_else(|| Path::new(&home).join(".cargo"));
        let rustup = env::var_os("RUSTUP_HOME")
            .map(std::path::PathBuf::from)
            .unwrap_or_else(|| Path::new(&home).join(".rustup"));
        let architecture = platform
            .target
            .split('-')
            .next()
            .context("Failed to select container architecture")?;
        let image = format!("quay.io/pypa/manylinux_2_28_{architecture}");
        let mut command = Command::new("docker");
        command.args(["run", "--rm", "--volume"]).arg(format!("{}:/io", root.display()))
            .arg("--volume").arg(format!("{}:/cargo", cargo.display()))
            .arg("--volume").arg(format!("{}:/rustup", rustup.display()))
            .args(["--workdir", "/io", "--env", "CARGO_HOME=/cargo", "--env", "RUSTUP_HOME=/rustup",
                "--env", "CARGO_TARGET_DIR=/io/temp/manylinux-target",
                "--env", "P4RUST_MANYLINUX=2_28", "--env", "P4RUST_TARGET",
                "--env", "PATH=/cargo/bin:/opt/clang/bin:/usr/local/bin:/opt/rh/gcc-toolset-14/root/usr/bin:/opt/rh/gcc-toolset-13/root/usr/bin:/usr/bin:/bin"]);
        if let Some(output) = env::var_os("GITHUB_OUTPUT") {
            command
                .arg("--volume")
                .arg(format!("{}:/github-output", Path::new(&output).display()))
                .args(["--env", "GITHUB_OUTPUT=/github-output"]);
        }
        Runner::run(command.arg(image).args(["cargo", "xtask", task]), false)
            .context("Failed to execute manylinux production")?;
        Ok(true)
    }
}
