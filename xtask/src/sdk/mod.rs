mod jam;
mod reconcile;
mod source;

use crate::{
    automation::command::Runner,
    cache::Cache,
    error::{Result, ResultExt, ensure},
    platform::Platform,
};
use std::{
    fs,
    path::{Path, PathBuf},
    process::Command,
};

pub(crate) struct Sdk;

impl Sdk {
    // Verify and copy the pinned Perforce and Jam source trees.
    pub(crate) fn prepare(root: &Path) -> Result<()> {
        source::Sources::prepare(root).context("Failed to prepare SDK sources")
    }

    // Locate the checked source tree without retaining precompiled SDK inputs.
    pub(crate) fn source(root: &Path) -> Result<PathBuf> {
        source::Sources::directory(root, "perforce").context("Failed to locate Perforce sources")
    }

    // Build the three required static API libraries using the vendor's dependency rules.
    pub(crate) fn build(root: &Path, ssl: &Path, platform: &Platform) -> Result<PathBuf> {
        let cache = Cache::sdk(root, platform).context("Failed to select SDK cache")?;
        let suffix = if platform.msvc() { "lib" } else { "a" };
        let libraries: Vec<_> = ["client", "rpc", "supp"]
            .iter()
            .map(|name| format!("lib{name}.{suffix}"))
            .collect();
        let names: Vec<_> = libraries.iter().map(String::as_str).collect();
        let reusable = ssl == root.join("temp/native-cache/openssl");
        if reusable
            && let Some(path) = cache
                .restore(&names)
                .context("Failed to inspect SDK cache")?
        {
            return Ok(path);
        }
        let source = Self::source(root).context("Failed to find SDK build inputs")?;
        Self::patch(root, &source).context("Failed to patch SDK production rules")?;
        let output = root.join("temp/sdk-build").join(&platform.target);
        if output.exists() {
            fs::remove_dir_all(&output).context("Failed to clear incompatible SDK build tree")?;
        }
        fs::create_dir_all(&output).context("Failed to create SDK build directory")?;
        let executable = jam::Jam::build(root, platform).context("Failed to build Jam")?;
        let tool = platform
            .compiler()
            .try_get_compiler()
            .context("Failed to locate SDK compiler")?;
        let jobs = std::thread::available_parallelism()
            .context("Failed to select SDK parallelism")?
            .get()
            .to_string();
        let mut command = Command::new(executable);
        command
            .current_dir(&source)
            .envs(tool.env().iter().cloned())
            .args([
                "-q",
                "-j",
                &jobs,
                "-sSMARTHEAP=0",
                "-sMALLOC_OVERRIDE=no",
                "-sUSE_EXTENSIONS=0",
                "-sUSE_WILDARGS=no",
                "-sBUILD_P4D=false",
                "-sWARNINGS_AS_ERRORS=0",
                "-sSSL=yes",
            ])
            .arg(format!("-sEXEC={}", Self::path(&output, platform)))
            .arg(format!(
                "-sSSLINCDIR={}",
                Self::path(&ssl.join("include"), platform)
            ))
            .arg(format!("-sSSLLIBDIR={}", Self::path(ssl, platform)));
        Self::platform(&mut command, platform).context("Failed to configure SDK platform")?;
        command.args(&libraries);
        Runner::run(&mut command, false).context("Failed to compile Perforce SDK sources")?;
        for name in &libraries {
            ensure!(
                output.join(name).is_file(),
                "Failed to find produced SDK library: {name}"
            );
        }
        if reusable {
            cache
                .save(&output, &names)
                .context("Failed to save compact SDK cache")
        } else {
            Ok(output)
        }
    }

    // Keep Windows shell paths free of slash characters interpreted as command switches.
    fn path(path: &Path, platform: &Platform) -> String {
        let text = path.to_string_lossy();
        if platform.windows() {
            text.replace('/', "\\")
        } else {
            text.into_owned()
        }
    }

    // Apply marked compatibility fixes only to the ignored build copy.
    fn patch(root: &Path, source: &Path) -> Result<()> {
        let rules = source.join("Jamrules");
        let text = fs::read_to_string(&rules).context("Failed to read SDK production rules")?;
        fs::write(
            rules,
            text.replace(
                "local _Z = /Zi ;",
                "# // PR_002 Start\nlocal _Z = ;\n# // PR_002 End",
            ),
        )
        .context("Failed to configure SDK debug policy")?;
        let header = source.join("zlib/zutil.h");
        let text = fs::read_to_string(&header).context("Failed to read vendor zlib header")?;
        fs::write(
            header,
            text.replace(
                "#if defined(MACOS) || defined(TARGET_OS_MAC)",
                "// PR_003 Start\n#if !defined(__APPLE__) && (defined(MACOS) || defined(TARGET_OS_MAC))\n// PR_003 End",
            ),
        )
        .context("Failed to correct vendor Apple fdopen detection")?;
        reconcile::Hooks::install(root, source).context("Failed to install SDK reconcile hooks")
    }

    // Select native compiler settings and a valid Apple SDK explicitly.
    fn platform(command: &mut Command, platform: &Platform) -> Result<()> {
        let arm = platform.target.starts_with("aarch64");
        if platform.msvc() {
            command
                .args(["-sOS=NT", "-sMSVSVER=17", "-sCRT=dyn", "-sTYPE=dyn"])
                .arg(if arm {
                    "-sOSPLAT=ARM64"
                } else {
                    "-sOSPLAT=X64"
                });
        } else if platform.apple() {
            let output = Runner::run(Command::new("xcrun").arg("--show-sdk-path"), true)
                .context("Failed to locate Apple SDK")?;
            let sdk =
                String::from_utf8(output.stdout).context("Failed to decode Apple SDK path")?;
            command
                .args([
                    "-sOS=MACOSX",
                    "-sOSVER=120",
                    "-sOSCOMP=clang",
                    "-sCLANGVER=17",
                    "-sTYPE=pic",
                ])
                .arg(if arm {
                    "-sOSPLAT=ARM64"
                } else {
                    "-sOSPLAT=X86_64"
                })
                .arg(format!("-sMACOSX_SDK={}", sdk.trim()));
        } else {
            command
                .args([
                    "-sOS=LINUX",
                    "-sOSVER=26",
                    "-sOSCOMP=gcc",
                    "-sGCCVER=14",
                    "-sTYPE=pic",
                ])
                .arg(if arm {
                    "-sOSPLAT=AARCH64"
                } else {
                    "-sOSPLAT=X86_64"
                });
        }
        Ok(())
    }
}
