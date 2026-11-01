use crate::{
    automation::command::Runner,
    error::{Result, ResultExt, ensure},
    openssl,
    platform::Platform,
    release,
    sdk::Sdk,
};
use std::{fs, path::PathBuf};

pub(crate) struct Producer {
    root: PathBuf,
    output: PathBuf,
    platform: Platform,
}

impl Producer {
    // Locate the repository and isolate production build intermediates.
    fn new() -> Result<Self> {
        let platform = Platform::selected().context("Failed to select native platform")?;
        let manifest = PathBuf::from(env!("CARGO_MANIFEST_DIR"));
        let root = manifest
            .parent()
            .context("Failed to locate native production repository")?
            .to_path_buf();
        let output = root.join("temp/native-production").join(&platform.target);
        fs::create_dir_all(&output).context("Failed to create native production directory")?;
        Ok(Self {
            root,
            output,
            platform,
        })
    }

    // Build a standalone static bridge with no Rust or cxx symbols.
    fn bridge(&self, source: &std::path::Path) -> Result<PathBuf> {
        let mut compiler = self.platform.compiler();
        compiler.out_dir(&self.output);
        let includes: Vec<_> = [
            "client", "diff", "dme", "dmec", "i18n", "map", "net", "rpc", "script", "support",
            "sys", "msgs", "web",
        ]
        .iter()
        .map(|name| source.join(name))
        .collect();
        for directory in &includes {
            compiler.include(directory);
        }
        let tool = compiler
            .try_get_compiler()
            .context("Failed to locate C ABI compiler")?;
        let object = self.output.join(if self.platform.msvc() {
            "client.obj"
        } else {
            "client.o"
        });
        let mut command = tool.to_command();
        if self.platform.msvc() {
            command
                .args([
                    "/c",
                    "/std:c++17",
                    "/O2",
                    "/MD",
                    "/EHsc",
                    "/DOS_NT",
                    "/DNOMINMAX",
                ])
                .arg(format!("/Fo{}", object.display()));
        } else {
            command
                .args(["-c", "-std=c++17", "-O2", "-fPIC"])
                .arg("-o")
                .arg(&object)
                .arg(if self.platform.apple() {
                    "-DOS_MACOSX"
                } else {
                    "-DOS_LINUX"
                });
        }
        command.arg(self.root.join("native/client.cc"));
        Runner::run(&mut command, false).context("Failed to precompile C ABI bridge")?;
        let archive = self.output.join(self.platform.bridge());
        let mut command = if self.platform.msvc() {
            let mut command = std::process::Command::new("lib.exe");
            command
                .args(["/NOLOGO", "/BREPRO"])
                .arg(format!("/OUT:{}", archive.display()));
            command.envs(tool.env().iter().cloned());
            command
        } else {
            let mut command = std::process::Command::new(Platform::utility("P4RUST_AR", "llvm-ar"));
            command.arg("rcs").arg(&archive);
            command
        };
        Runner::run(command.arg(&object), false).context("Failed to archive C ABI bridge")?;
        self.runtime(&archive)
            .context("Failed to include C++ runtime compatibility")?;
        Ok(archive)
    }

    // Bundle manylinux's nonshared GCC helpers before the common dependency pruning pass.
    fn runtime(&self, archive: &std::path::Path) -> Result<()> {
        if !self.platform.target.ends_with("linux-gnu")
            || std::env::var_os("P4RUST_MANYLINUX").is_none()
        {
            return Ok(());
        }
        let compiler = self
            .platform
            .compiler()
            .try_get_compiler()
            .context("Failed to locate C++ runtime compiler")?;
        let output = Runner::run(
            compiler
                .to_command()
                .arg("-print-file-name=libstdc++_nonshared.a"),
            true,
        )
        .context("Failed to locate nonshared C++ runtime")?;
        let name = String::from_utf8(output.stdout)
            .context("Failed to decode nonshared C++ runtime path")?;
        let runtime = PathBuf::from(name.trim());
        ensure!(
            runtime.is_file(),
            "Failed to find nonshared C++ runtime: {}",
            runtime.display()
        );
        Runner::run(
            std::process::Command::new(Platform::utility("P4RUST_AR", "llvm-ar"))
                .arg("qLs")
                .arg(archive)
                .arg(runtime),
            false,
        )
        .context("Failed to merge nonshared C++ runtime members")?;
        Ok(())
    }

    // Build OpenSSL on the producer or reuse explicitly supplied static libraries.
    fn openssl(&self) -> Result<PathBuf> {
        let args: Vec<String> = if std::env::args().nth(1).as_deref() == Some("native") {
            std::env::args().skip(2).collect()
        } else {
            Vec::new()
        };
        if args.len() == 2 && args[0] == "--openssl-lib-dir" {
            let path = PathBuf::from(&args[1])
                .canonicalize()
                .context("Failed to locate supplied OpenSSL libraries")?;
            ensure!(
                self.platform
                    .ssl_names()
                    .iter()
                    .all(|name| path.join(name).is_file()),
                "Supplied directory must contain target-compatible static OpenSSL archives"
            );
            return Ok(path);
        }
        ensure!(
            args.is_empty(),
            "Usage: cargo xtask native [--openssl-lib-dir <producer-cache>]"
        );
        openssl::OpenSsl::build(&self.root, &self.platform)
            .context("Failed to precompile compact OpenSSL")
    }

    // Install compact archives into the single public release package.
    fn install_release(
        &self,
        bridge: &std::path::Path,
        ssl: &std::path::Path,
        sdk: &std::path::Path,
    ) -> Result<()> {
        release::Release::install(&self.root, &self.output, bridge, ssl, sdk, &self.platform)
            .context("Failed to generate native release layout")
    }
    // Generate native dependencies only when a maintainer explicitly runs this tool.
    pub(crate) fn run() -> Result<()> {
        let producer = Self::new().context("Failed to initialize native producer")?;
        let ssl = producer
            .openssl()
            .context("Failed to produce OpenSSL libraries")?;
        let sdk = Sdk::build(&producer.root, &ssl, &producer.platform)
            .context("Failed to produce Perforce SDK")?;
        let source = Sdk::source(&producer.root).context("Failed to locate bridge SDK headers")?;
        let bridge = producer
            .bridge(&source)
            .context("Failed to produce native bridge")?;
        producer
            .install_release(&bridge, &ssl, &sdk)
            .context("Failed to install native release dependencies")
    }
}
