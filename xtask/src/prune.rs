use crate::{
    automation::command::Runner,
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
        let output = Runner::run(command, true).context("Failed to launch native archive tool")?;
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
            if matches!(name, "libssl.a" | "libcrypto.a") {
                continue;
            }
            let required = Self::members(&trace, name);
            ensure!(
                !required.is_empty(),
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
        let _ = root;
        let names = [
            "p4rust_bridge",
            "libclient",
            "librpc",
            "libsupp",
            "libssl",
            "libcrypto",
        ];
        for name in names {
            let source = staging.join(format!("{name}.lib"));
            let stripped = staging.join(format!("{name}-stripped.lib"));
            Self::command(
                Command::new(Platform::utility("P4RUST_OBJCOPY", "llvm-objcopy"))
                    .arg("--strip-debug")
                    .arg(&source)
                    .arg(&stripped),
            )
            .context("Failed to strip COFF debug records")?;
            fs::copy(stripped, source).context("Failed to install stripped COFF library")?;
        }
        let trace =
            Self::coff_probe(staging, &tool).context("Failed to trace COFF dependencies")?;
        fs::write(staging.join("dependency-trace.txt"), &trace)
            .context("Failed to preserve COFF trace")?;
        for name in ["libclient", "librpc", "libsupp"] {
            Self::coff_compact(staging, name, &trace, &tool)
                .context("Failed to compact COFF library")?;
        }
        Self::coff_probe(staging, &tool).context("Failed to relink compact COFF libraries")?;
        Ok(())
    }

    // Link every ABI export without dropping cold paths or starting a native executable.
    fn coff_probe(staging: &Path, tool: &cc::Tool) -> Result<String> {
        let mut command = Command::new("link.exe");
        command
            .envs(tool.env().iter().cloned())
            .env("VSLANG", "1033")
            .args([
                "/NOLOGO",
                "/DLL",
                "/INCREMENTAL:NO",
                "/OPT:NOREF",
                "/VERBOSE",
                "/EXPORT:p4rust_abi_version",
                "/EXPORT:p4rust_execute_v1",
                "/EXPORT:p4rust_execute_controlled_v1",
                "/EXPORT:p4rust_execute_reconcile_v3",
                "/EXPORT:p4rust_reconcile_execute_v3",
                "/EXPORT:p4rust_reconcile_result_v3",
                "/EXPORT:p4rust_reconcile_commit_v3",
                "/EXPORT:p4rust_reconcile_error_v3",
                "/EXPORT:p4rust_reconcile_drop_v3",
                "/EXPORT:p4rust_scan_open_v5",
                "/EXPORT:p4rust_scan_file_v5",
                "/EXPORT:p4rust_scan_close_v5",
                "/EXPORT:p4rust_reconcile_snapshot_v5",
                "/EXPORT:p4rust_reconcile_path_v5",
            ])
            .arg(format!(
                "/OUT:{}",
                staging.join("dependency-probe.dll").display()
            ))
            .arg(format!("/LIBPATH:{}", staging.display()));
        for name in [
            "p4rust_bridge",
            "libclient",
            "librpc",
            "libsupp",
            "libssl",
            "libcrypto",
            "ws2_32",
            "advapi32",
            "crypt32",
            "user32",
            "shell32",
            "ole32",
            "oleaut32",
            "gdi32",
            "bcrypt",
            "iphlpapi",
        ] {
            command.arg(format!("{name}.lib"));
        }
        Self::command(&mut command).context("Failed to link COFF dependency probe")
    }

    // Remove unreachable COFF members using a response file for long archive paths.
    fn coff_compact(staging: &Path, name: &str, trace: &str, tool: &cc::Tool) -> Result<()> {
        let source = staging.join(format!("{name}.lib"));
        let output = staging.join(format!("{name}-pruned.lib"));
        let required = Self::members(trace, &format!("{name}.lib"));
        ensure!(
            !required.is_empty(),
            "Failed to find reachable COFF members: {name}"
        );
        let entries = Self::command(
            Command::new("lib.exe")
                .args(["/NOLOGO", "/LIST"])
                .arg(&source)
                .envs(tool.env().iter().cloned()),
        )
        .context("Failed to enumerate COFF library")?;
        let mut args = vec![
            "/NOLOGO".to_owned(),
            "/BREPRO".to_owned(),
            format!("\"/OUT:{}\"", output.display()),
            format!("\"{}\"", source.display()),
        ];
        for member in entries
            .lines()
            .filter(|member| !required.contains(Self::basename(member)))
        {
            args.push(format!("\"/REMOVE:{}\"", member.trim()));
        }
        let response = staging.join(format!("{name}-prune.rsp"));
        fs::write(&response, args.join("\n")).context("Failed to write COFF response file")?;
        Self::command(
            Command::new("lib.exe")
                .arg(format!("@{}", response.display()))
                .envs(tool.env().iter().cloned()),
        )
        .context("Failed to prune COFF library")?;
        fs::copy(output, source).context("Failed to install compact COFF library")?;
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
        fs::write(&source, "#include <client.h>\n// Retain every public ABI symbol for archive dependency analysis.\nint main() { auto volatile version = &p4rust_abi_version; auto volatile execute = &p4rust_execute_v1; auto volatile controlled = &p4rust_execute_controlled_v1; (void)version; (void)execute; (void)controlled; return 0; }\n")
            .context("Failed to write ABI dependency probe")?;
        let map = staging.join("dependency-probe.map");
        let executable = staging.join("dependency-probe");
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
                    "CoreServices",
                    "-framework",
                    "ApplicationServices",
                    "-framework",
                    "CoreGraphics",
                    "-framework",
                    "Security",
                    "-framework",
                    "Foundation",
                    "-framework",
                    "SystemConfiguration",
                ]);
        } else {
            command
                .arg("-Wl,--no-gc-sections")
                .arg(format!("-Wl,-Map,{}", map.display()))
                .arg("-Wl,--start-group")
                .args(libraries)
                .arg("-Wl,--end-group")
                .args(["-pthread", "-ldl", "-lresolv", "-lrt", "-lm"]);
        }
        let output =
            Runner::run(&mut command, true).context("Failed to link ABI dependency probe")?;
        ensure!(
            output.status.success(),
            "Failed to link ABI dependency probe: {}",
            String::from_utf8_lossy(&output.stderr)
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
