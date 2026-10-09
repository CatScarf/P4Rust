use crate::error::{Result, ResultExt, ensure};
use std::{
    fs,
    path::{Path, PathBuf},
};

pub(crate) struct Wheels;

impl Wheels {
    // Inspect packaged typing without executing a foreign-architecture extension.
    pub(crate) fn extract(path: &Path, destination: &Path) -> Result<()> {
        let file = fs::File::open(path).context("Failed to open typed Python wheel")?;
        let mut zip = zip::ZipArchive::new(file).context("Failed to inspect typed Python wheel")?;
        zip.extract(destination)
            .context("Failed to extract typed Python wheel")
    }

    // Find the ordinary stable ABI wheel without assuming a platform tag spelling.
    pub(crate) fn regular(root: &Path) -> Result<PathBuf> {
        for entry in fs::read_dir(root.join("temp/ci-artifacts"))
            .context("Failed to list Python artifacts")?
        {
            let entry = entry.context("Failed to read Python artifact")?;
            if entry.file_name().to_string_lossy().contains("-cp39-abi3-") {
                return Ok(entry.path());
            }
        }
        Err(crate::error::Error::new("Failed to find cp39-abi3 wheel"))
    }

    // Check native wheel tags and mandatory typing before adding release artifacts.
    pub(crate) fn validate(path: &Path, version: &str, free_threaded: bool) -> Result<()> {
        let name = path
            .file_name()
            .and_then(|name| name.to_str())
            .context("Failed to read wheel name")?;
        ensure!(
            name.starts_with(&format!("p4rust-{version}-")),
            "Failed to match Python wheel version: {name}"
        );
        ensure!(
            if free_threaded {
                name.contains("-cp315-abi3t-") || name.contains("-cp315-abi3.abi3t-")
            } else {
                name.contains("-cp39-abi3-")
            },
            "Failed to validate stable ABI wheel: {name}"
        );
        let file = fs::File::open(path).context("Failed to open wheel")?;
        let mut zip = zip::ZipArchive::new(file).context("Failed to inspect wheel")?;
        for required in [
            "p4rust/py.typed",
            "p4rust/_native.pyi",
            "p4rust/_events.py",
            "p4rust/licenses/Perforce.txt",
        ] {
            zip.by_name(required)
                .with_context(|| format!("Failed to find required wheel file {required}"))?;
        }
        Ok(())
    }
}
