use super::command::Runner;
use crate::{
    error::{Result, ResultExt, ensure},
    platform::Platform,
};
use sha2::Digest;
use std::{fs, io::Write, path::Path, process::Command};

pub(super) struct Bundle;

impl Bundle {
    // Combine one common Rust package and six platform resources into a single ZIP.
    pub(super) fn create(root: &Path, version: &str) -> Result<()> {
        let output = root.join("temp/release-output");
        fs::create_dir_all(&output).context("Failed to create release output directory")?;
        let file = fs::File::create(output.join(format!("p4rust-{version}.zip")))
            .context("Failed to create release ZIP")?;
        let mut zip = zip::ZipWriter::new(file);
        let mut public_hash = None;
        for target in Platform::TARGETS {
            let input = root
                .join("temp/release-inputs")
                .join(format!("p4rust-{target}"));
            let public = input.join(format!("p4rust-{version}.crate"));
            let bytes = Self::read(&public).context("Failed to read public package")?;
            let hash = sha2::Sha256::digest(&bytes);
            if let Some(expected) = public_hash {
                ensure!(
                    hash == expected,
                    "Failed to match common Rust package across platforms: {target}"
                );
            } else {
                public_hash = Some(hash);
                Self::append(&mut zip, &public, &bytes).context("Failed to bundle Rust package")?;
            }
            let resource = input.join(format!("p4rust-resources-{target}-{version}.crate"));
            let bytes = Self::read(&resource).context("Failed to read resource package")?;
            Self::validate(&resource, target, version)
                .context("Failed to verify resource package")?;
            Self::append(&mut zip, &resource, &bytes)
                .context("Failed to bundle resource package")?;
        }
        zip.finish().context("Failed to finish release ZIP")?;
        Ok(())
    }

    // Enforce the compressed package size limit before allocating ZIP entries.
    fn read(path: &Path) -> Result<Vec<u8>> {
        ensure!(
            fs::metadata(path)
                .context("Failed to inspect release package")?
                .len()
                < 10_000_000,
            "Failed to meet package size limit: {}",
            path.display()
        );
        fs::read(path).context("Failed to read release package bytes")
    }

    // Verify the resource package contains exactly the maintained native archive set.
    fn validate(path: &Path, target: &str, version: &str) -> Result<()> {
        let output = Runner::run(Command::new("tar").arg("-tzf").arg(path), true)
            .context("Failed to list packaged resource files")?;
        let entries =
            String::from_utf8(output.stdout).context("Failed to decode package file list")?;
        let prefix = format!("p4rust-resources-{target}-{version}/native/");
        let suffix = if target.ends_with("windows-msvc") {
            ".lib"
        } else {
            ".a"
        };
        ensure!(
            entries
                .lines()
                .filter(|name| name.starts_with(&prefix) && name.ends_with(suffix))
                .count()
                == 6,
            "Failed to verify six native libraries for {target}"
        );
        Ok(())
    }

    // Store already compressed crates without a second compression pass.
    fn append(zip: &mut zip::ZipWriter<fs::File>, path: &Path, bytes: &[u8]) -> Result<()> {
        let name = path
            .file_name()
            .and_then(|name| name.to_str())
            .context("Failed to name ZIP entry")?;
        zip.start_file(
            name,
            zip::write::SimpleFileOptions::default()
                .compression_method(zip::CompressionMethod::Stored),
        )
        .context("Failed to create ZIP entry")?;
        zip.write_all(bytes).context("Failed to write ZIP entry")
    }
}
