use crate::error::{Result, ResultExt, ensure};

use std::{fs, path::PathBuf};
mod archive;
#[path = "../../src/error.rs"]
mod error;
mod openssl;
mod release;
mod task;

struct Producer {
    root: PathBuf,
    output: PathBuf,
}

impl Producer {
    // Locate the repository and isolate production build intermediates.
    fn new() -> Result<Self> {
        ensure!(
            cfg!(all(
                target_os = "windows",
                target_arch = "x86_64",
                target_env = "msvc"
            )),
            "Native production currently requires Windows x64 MSVC"
        );
        let manifest = PathBuf::from(env!("CARGO_MANIFEST_DIR"));
        let root = manifest
            .parent()
            .context("Failed to locate native production repository")?
            .to_path_buf();
        let output = root.join("temp/native-production");
        fs::create_dir_all(&output).context("Failed to create native production directory")?;
        Ok(Self { root, output })
    }

    // Build a standalone static bridge with no Rust or cxx symbols.
    fn bridge(&self) -> Result<PathBuf> {
        let mut compiler = cc::Build::new();
        compiler
            .cpp(true)
            .std("c++17")
            .flag("/EHsc")
            .define("OS_NT", None)
            .define("NOMINMAX", None)
            .opt_level(2)
            .debug(false)
            .static_crt(false)
            .target("x86_64-pc-windows-msvc")
            .host("x86_64-pc-windows-msvc")
            .cargo_metadata(false)
            .out_dir(&self.output)
            .include(self.root.join("sdk/include/p4"))
            .file(self.root.join("native/client.cc"));
        compiler
            .try_compile("p4rust_bridge")
            .context("Failed to precompile C ABI bridge")?;
        Ok(self.output.join("p4rust_bridge.lib"))
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
                path.join("libssl.lib").is_file() && path.join("libcrypto.lib").is_file(),
                "Supplied directory must contain static libssl.lib and libcrypto.lib"
            );
            return Ok(path);
        }
        ensure!(
            args.is_empty(),
            "Usage: cargo xtask native [--openssl-lib-dir <producer-cache>]"
        );
        openssl::OpenSsl::build(&self.output.join("openssl-compact"))
            .context("Failed to precompile compact OpenSSL")
    }

    // Install compact archives into the single public release package.
    fn install_release(&self, bridge: &std::path::Path, ssl: &std::path::Path) -> Result<()> {
        release::Release::install(&self.root, &self.output, bridge, ssl)
            .context("Failed to generate native release layout")
    }
    // Generate native dependencies only when a maintainer explicitly runs this tool.
    fn run() -> Result<()> {
        let producer = Self::new().context("Failed to initialize native producer")?;
        let bridge = producer
            .bridge()
            .context("Failed to produce native bridge")?;
        let ssl = producer
            .openssl()
            .context("Failed to produce OpenSSL libraries")?;
        producer
            .install_release(&bridge, &ssl)
            .context("Failed to install native release dependencies")
    }
}

// Execute the isolated maintainer-side build workflow.
fn main() -> Result<()> {
    task::Task::run().context("Failed to execute xtask")
}
