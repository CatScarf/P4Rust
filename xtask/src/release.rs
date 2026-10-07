use crate::error::{Result, ResultExt, ensure};
use std::{fs, path::Path, process::Command};

pub(crate) struct Release;

impl Release {
    // Strip debug records and remove SDK objects outside the complete C ABI closure.
    fn prune(root: &Path, staging: &Path) -> Result<()> {
        let tool = cc::windows_registry::find_tool("x86_64-pc-windows-msvc", "cl.exe")
            .context("Failed to locate native pruning toolchain")?;
        let objcopy = std::env::var("P4RUST_OBJCOPY").unwrap_or_else(|_| {
            let installed = Path::new("C:/Program Files/LLVM/bin/llvm-objcopy.exe");
            if installed.is_file() {
                installed.to_string_lossy().into_owned()
            } else {
                "llvm-objcopy".to_owned()
            }
        });
        let status = Command::new("pwsh")
            .args(["-NoProfile", "-File"])
            .arg(root.join("xtask/scripts/prune-native.ps1"))
            .arg("-LibraryDirectory")
            .arg(staging)
            .arg("-Objcopy")
            .arg(objcopy)
            .envs(tool.env().iter().cloned())
            .status()
            .context("Failed to run native archive pruning")?;
        ensure!(status.success(), "Failed to prune native release archives");
        Ok(())
    }

    // Prepare one target's complete precompiled dependencies inside the public crate.
    pub(crate) fn install(root: &Path, output: &Path, bridge: &Path, ssl: &Path) -> Result<()> {
        let target = "x86_64-pc-windows-msvc";
        let staging = output.join("release-libraries");
        let destination = root.join("native/lib").join(target);
        fs::create_dir_all(&staging).context("Failed to create release staging directory")?;
        fs::create_dir_all(&destination).context("Failed to create release library directory")?;
        let mut paths = vec![
            bridge.to_path_buf(),
            ssl.join("libssl.lib"),
            ssl.join("libcrypto.lib"),
        ];
        for name in ["libclient", "libp4script_cstub", "librpc", "libsupp"] {
            paths.push(
                root.join("sdk/lib")
                    .join(target)
                    .join(format!("{name}.lib")),
            );
        }
        for path in &paths {
            fs::copy(
                path,
                staging.join(
                    path.file_name()
                        .context("Failed to identify source archive")?,
                ),
            )
            .with_context(|| format!("Failed to stage archive {}", path.display()))?;
        }
        Self::prune(root, &staging).context("Failed to compact native release")?;
        for path in &paths {
            let name = path
                .file_name()
                .context("Failed to identify release archive")?;
            fs::copy(staging.join(name), destination.join(name))
                .context("Failed to install compacted native library")?;
        }
        Ok(())
    }
}
