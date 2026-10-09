use super::dependencies::Dependencies;
use crate::{
    cache::Cache,
    error::{Result, ResultExt},
    platform::Platform,
};
use std::{env, fs, io::Write, path::Path};

pub(super) struct Preparation;

impl Preparation {
    // Prepare tools once per job before reading their versions into native cache keys.
    pub(super) fn dependencies(root: &Path, platform: &Platform) -> Result<()> {
        let marker = root.join("temp/ci-prepared-target");
        if marker.is_file()
            && std::env::var_os("P4RUST_MANYLINUX").is_none()
            && fs::read_to_string(&marker).context("Failed to read CI preparation marker")?
                == platform.target
        {
            return Ok(());
        }
        Dependencies::install(platform).context("Failed to install CI toolchain")?;
        fs::create_dir_all(root.join("temp"))
            .context("Failed to create CI preparation directory")?;
        fs::write(marker, &platform.target).context("Failed to record prepared CI target")
    }

    // Export exact cache keys while leaving restore and save transport to GitHub Actions.
    pub(super) fn run(root: &Path) -> Result<()> {
        let platform = Platform::selected().context("Failed to select cached CI target")?;
        Self::dependencies(root, &platform).context("Failed to prepare cached CI tools")?;
        let ssl = Cache::openssl(root, &platform).context("Failed to compute CI OpenSSL key")?;
        let sdk = Cache::sdk(root, &platform).context("Failed to compute CI SDK key")?;
        let output = env::var_os("GITHUB_OUTPUT").context("Failed to locate CI step outputs")?;
        let mut output = fs::OpenOptions::new()
            .append(true)
            .open(output)
            .context("Failed to open CI step outputs")?;
        writeln!(output, "openssl={}\nsdk={}", ssl.key, sdk.key)
            .context("Failed to export native cache keys")?;
        println!("Native cache keys: {} / {}", ssl.key, sdk.key);
        Ok(())
    }
}
