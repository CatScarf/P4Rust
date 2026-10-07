use crate::{
    error::{Result, ResultExt, ensure},
    openssl,
    platform::Platform,
    release,
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
    fn bridge(&self) -> Result<PathBuf> {
        let mut compiler = self.platform.compiler();
        compiler
            .out_dir(&self.output)
            .include(self.root.join("sdk/include/p4"))
            .file(self.root.join("native/client.cc"));
        compiler
            .try_compile("p4rust_bridge")
            .context("Failed to precompile C ABI bridge")?;
        Ok(self.output.join(self.platform.bridge()))
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
        openssl::OpenSsl::build(
            &self.root.join("temp/native-production/openssl-compact"),
            &self.platform,
        )
        .context("Failed to precompile compact OpenSSL")
    }

    // Install compact archives into the single public release package.
    fn install_release(&self, bridge: &std::path::Path, ssl: &std::path::Path) -> Result<()> {
        release::Release::install(&self.root, &self.output, bridge, ssl, &self.platform)
            .context("Failed to generate native release layout")
    }
    // Generate native dependencies only when a maintainer explicitly runs this tool.
    pub(crate) fn run() -> Result<()> {
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
