use crate::{
    error::{Result, ResultExt, ensure},
    platform::Platform,
};
use std::{
    collections::HashSet,
    fs,
    path::{Path, PathBuf},
    process::Command,
};

pub(crate) struct Prune;

impl Prune {
    // Run a native archive tool and retain its diagnostics on failure.
    fn command(command: &mut Command) -> Result<String> {
        let output = command
            .output()
            .context("Failed to launch native archive tool")?;
        ensure!(
            output.status.success(),
            "Failed to run native archive tool: {}",
            String::from_utf8_lossy(&output.stderr)
        );
        String::from_utf8(output.stdout).context("Failed to decode archive tool output")
    }

    // Preserve complete archive-member dependency closures without pruning cold code.
    pub(crate) fn compact(
        root: &Path,
        staging: &Path,
        libraries: &[PathBuf],
        platform: &Platform,
    ) -> Result<()> {
        if platform.msvc() {
            return Self::msvc(root, staging, platform).context("Failed to prune MSVC libraries");
        }
        let strip = Platform::utility("P4RUST_STRIP", "llvm-strip");
        for path in libraries {
            Self::command(Command::new(&strip).arg("--strip-debug").arg(path))
                .context("Failed to strip native archive debug records")?;
        }
        let trace = Self::probe(root, staging, libraries, platform)
            .context("Failed to trace native library dependencies")?;
        fs::write(staging.join("dependency-trace.txt"), &trace)
            .context("Failed to save native dependency trace")?;
        let ar = Platform::utility("P4RUST_AR", "llvm-ar");
        for path in libraries {
            let name = path
                .file_name()
                .and_then(|name| name.to_str())
                .context("Failed to identify archive")?;
            if matches!(
                name,
                "libssl.a"
                    | "libcrypto.a"
                    | "libstdc++.a"
                    | "libwinpthread.a"
                    | "libgcc.a"
                    | "libgcc_eh.a"
            ) {
                continue;
            }
            let required = Self::members(&trace, name);
            if required.is_empty() && matches!(name, "libgcc.a" | "libgcc_eh.a" | "libwinpthread.a")
            {
                continue;
            }
            ensure!(
                !required.is_empty() || name == "libp4script_cstub.a",
                "Failed to trace archive members: {name}"
            );
            let entries = Self::command(Command::new(&ar).arg("t").arg(path))
                .context("Failed to enumerate archive members")?;
            let unused: Vec<&str> = entries
                .lines()
                .filter(|entry| !required.contains(Self::basename(entry)))
                .collect();
            if !unused.is_empty() {
                Self::command(Command::new(&ar).arg("d").arg(path).args(unused))
                    .context("Failed to remove unreachable archive members")?;
            }
        }
        Self::probe(root, staging, libraries, platform)
            .context("Failed to relink compact native libraries")?;
        Ok(())
    }

    // Use the existing COFF dependency probe under the selected MSVC environment.
    fn msvc(root: &Path, staging: &Path, platform: &Platform) -> Result<()> {
        let tool = platform
            .compiler()
            .try_get_compiler()
            .context("Failed to locate MSVC pruning toolchain")?;
        let status = Command::new("pwsh")
            .args(["-NoProfile", "-File"])
            .arg(root.join("xtask/scripts/prune-native.ps1"))
            .arg("-LibraryDirectory")
            .arg(staging)
            .arg("-Objcopy")
            .arg(Platform::utility("P4RUST_OBJCOPY", "llvm-objcopy"))
            .envs(tool.env().iter().cloned())
            .status()
            .context("Failed to start MSVC pruning")?;
        ensure!(status.success(), "Failed to prune MSVC libraries: {status}");
        Ok(())
    }

    // Link both ABI exports with ordinary archive extraction and no section garbage collection.
    fn probe(
        root: &Path,
        staging: &Path,
        libraries: &[PathBuf],
        platform: &Platform,
    ) -> Result<String> {
        let source = staging.join("dependency-probe.cc");
        fs::write(&source, "#include <client.h>\n// Exercise every public ABI export without connecting to a server.\nint main() { return p4rust_abi_version() == 1 && p4rust_execute_v1(nullptr, nullptr, 0, nullptr, nullptr, nullptr) != 0 && p4rust_execute_controlled_v1(nullptr, nullptr, 0, nullptr, nullptr, nullptr, nullptr, nullptr) != 0 ? 0 : 1; }\n")
            .context("Failed to write ABI dependency probe")?;
        let map = staging.join("dependency-probe.map");
        let executable = staging.join(if platform.windows() {
            "dependency-probe.exe"
        } else {
            "dependency-probe"
        });
        let compiler = platform
            .compiler()
            .try_get_compiler()
            .context("Failed to locate ABI probe compiler")?;
        let mut command = compiler.to_command();
        command
            .arg(&source)
            .arg("-I")
            .arg(root.join("native"))
            .arg("-o")
            .arg(&executable);
        if platform.apple() {
            command
                .arg("-Wl,-why_load")
                .arg(format!("-Wl,-map,{}", map.display()))
                .args(libraries)
                .args([
                    "-framework",
                    "CoreFoundation",
                    "-framework",
                    "CoreGraphics",
                    "-framework",
                    "Security",
                    "-framework",
                    "Foundation",
                ]);
        } else {
            command
                .arg("-Wl,--no-gc-sections")
                .arg(format!("-Wl,-Map,{}", map.display()))
                .arg("-Wl,--start-group")
                .args(libraries)
                .arg("-Wl,--end-group");
            if platform.windows() {
                command.args([
                    "-static-libstdc++",
                    "-static-libgcc",
                    "-lws2_32",
                    "-ladvapi32",
                    "-lcrypt32",
                    "-luser32",
                    "-lshell32",
                    "-lole32",
                    "-lgdi32",
                ]);
            } else {
                command.args(["-pthread", "-ldl", "-lresolv"]);
            }
        }
        let output = command
            .output()
            .context("Failed to link ABI dependency probe")?;
        ensure!(
            output.status.success(),
            "Failed to link ABI dependency probe: {}",
            String::from_utf8_lossy(&output.stderr)
        );
        let status = Command::new(executable)
            .status()
            .context("Failed to execute ABI dependency probe")?;
        ensure!(
            status.success(),
            "Failed to verify ABI dependency probe: {status}"
        );
        let trace = if platform.apple() {
            String::from_utf8(output.stderr).context("Failed to decode Apple archive trace")?
        } else {
            fs::read_to_string(map).context("Failed to read GNU archive dependency map")?
        };
        Ok(trace)
    }

    // Normalize archive member paths across GNU, COFF, and Apple tools.
    fn basename(name: &str) -> &str {
        name.rsplit(['/', '\\']).next().unwrap_or(name).trim()
    }

    // Extract member names from linker maps and Apple's archive-load diagnostics.
    fn members(trace: &str, archive: &str) -> HashSet<String> {
        trace
            .lines()
            .filter_map(|line| {
                let (_, suffix) = line.split_once(archive)?;
                let suffix = if suffix.starts_with('[') {
                    suffix.split_once(']')?.1
                } else {
                    suffix
                };
                let member = suffix.strip_prefix('(')?;
                let (member, _) = member.split_once(')')?;
                Some(Self::basename(member).to_owned())
            })
            .collect()
    }
}
