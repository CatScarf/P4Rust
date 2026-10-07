use crate::{
    automation::command::Runner,
    error::{Result, ResultExt},
    platform::Platform,
};
use sha2::Digest;
use std::{env, fs, path::Path, process::Command};

pub(super) struct Identity(sha2::Sha256);

impl Identity {
    // Fingerprint the actual compiler, target, native options, and selected system SDK.
    pub(super) fn new(root: &Path, platform: &Platform) -> Result<Self> {
        let mut identity = Self(sha2::Sha256::new());
        identity.add(platform.target.as_bytes());
        identity
            .compiler(platform)
            .context("Failed to fingerprint native compiler")?;
        identity
            .environment(platform)
            .context("Failed to fingerprint native environment")?;
        identity
            .files(
                root,
                &[
                    "xtask/src/platform.rs",
                    "xtask/src/cache/mod.rs",
                    "xtask/src/cache/identity.rs",
                ],
            )
            .context("Failed to identify native cache policy")?;
        Ok(identity)
    }

    // Include compiler banners, configured flags, and system header and library paths.
    fn compiler(&mut self, platform: &Platform) -> Result<()> {
        let tool = platform
            .compiler()
            .try_get_compiler()
            .context("Failed to find cache compiler")?;
        self.add(tool.path().to_string_lossy().as_bytes());
        let output = Runner::run(
            Command::new(tool.path())
                .envs(tool.env().iter().cloned())
                .arg(if platform.msvc() { "/?" } else { "--version" }),
            true,
        )
        .context("Failed to read native compiler version")?;
        self.add(&output.stdout);
        self.add(&output.stderr);
        for argument in tool.args() {
            self.add(argument.to_string_lossy().as_bytes());
        }
        for (name, value) in tool.env() {
            if matches!(name.to_str(), Some("INCLUDE" | "LIB" | "LIBPATH")) {
                self.add(name.to_string_lossy().as_bytes());
                self.add(value.to_string_lossy().as_bytes());
            }
        }
        Ok(())
    }

    // Track native environment overrides and Apple's selected SDK without temporary PATH entries.
    fn environment(&mut self, platform: &Platform) -> Result<()> {
        for name in [
            "CC",
            "CXX",
            "CFLAGS",
            "CXXFLAGS",
            "SDKROOT",
            "MACOSX_DEPLOYMENT_TARGET",
        ] {
            self.add(name.as_bytes());
            self.add(
                env::var_os(name)
                    .unwrap_or_default()
                    .to_string_lossy()
                    .as_bytes(),
            );
        }
        if platform.apple() {
            let output = Runner::run(Command::new("xcrun").arg("--show-sdk-version"), true)
                .context("Failed to identify cached Apple SDK")?;
            self.add(&output.stdout);
        }
        Ok(())
    }

    // Separate identity components with their lengths to avoid ambiguous concatenation.
    pub(super) fn add(&mut self, bytes: &[u8]) {
        self.0.update((bytes.len() as u64).to_le_bytes());
        self.0.update(bytes);
    }

    // Include maintained build definitions without depending on unrelated Rust API edits.
    pub(super) fn files(&mut self, root: &Path, paths: &[&str]) -> Result<()> {
        for path in paths {
            self.add(path.as_bytes());
            self.add(
                &fs::read(root.join(path))
                    .with_context(|| format!("Failed to fingerprint native build input {path}"))?,
            );
        }
        Ok(())
    }

    // Produce a stable key suitable for GitHub's platform-specific cache service.
    pub(super) fn finish(self) -> String {
        format!("{:x}", self.0.finalize())
    }
}
