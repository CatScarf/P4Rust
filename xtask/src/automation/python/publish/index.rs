use crate::{
    automation::registry::http::Http,
    error::{Result, ResultExt, ensure},
};
use sha2::Digest;
use std::{collections::BTreeMap, fs, path::Path};

pub(super) struct Published {
    pub(super) url: String,
    pub(super) digest: String,
}

pub(super) struct Index;

impl Index {
    // Read the immutable files already published for this package version.
    pub(super) fn files(root: &Path, version: &str) -> Result<BTreeMap<String, Published>> {
        let (status, bytes) = Http::get(
            root,
            &format!("https://pypi.org/pypi/p4rust/{version}/json"),
            None,
        )
        .context("Failed to query PyPI version")?;
        if status == 404 {
            return Ok(BTreeMap::new());
        }
        ensure!(status == 200, "Failed to query PyPI: HTTP {status}");
        let value: serde_json::Value =
            serde_json::from_slice(&bytes).context("Failed to parse PyPI response")?;
        Self::description(root, &value).context("Failed to confirm PyPI project description")?;
        let files = value["urls"]
            .as_array()
            .context("Failed to read published wheel list")?;
        let mut result = BTreeMap::new();
        for file in files {
            let name = file["filename"]
                .as_str()
                .context("Failed to name published wheel")?;
            let url = file["url"]
                .as_str()
                .context("Failed to locate published wheel")?;
            ensure!(
                url.starts_with("https://files.pythonhosted.org/"),
                "Failed to validate PyPI file host"
            );
            let digest = file["digests"]["sha256"]
                .as_str()
                .context("Failed to read published wheel checksum")?;
            result.insert(
                name.to_owned(),
                Published {
                    url: url.to_owned(),
                    digest: digest.to_owned(),
                },
            );
        }
        Ok(result)
    }

    // Confirm PyPI rendered metadata comes from the maintained Markdown README.
    fn description(root: &Path, value: &serde_json::Value) -> Result<()> {
        let body = value["info"]["description"]
            .as_str()
            .context("Failed to read PyPI project description")?
            .replace("\r\n", "\n");
        let content_type = value["info"]["description_content_type"]
            .as_str()
            .context("Failed to read PyPI description content type")?;
        ensure!(
            content_type.starts_with("text/markdown"),
            "Failed to identify PyPI Markdown description"
        );
        let readme = fs::read_to_string(root.join("README.md"))
            .context("Failed to read maintained project description")?
            .replace("\r\n", "\n");
        ensure!(
            body.trim_end() == readme.trim_end(),
            "Failed to match PyPI description with README.md"
        );
        Ok(())
    }

    // Download and verify the canonical published bytes before replacing a rebuilt wheel.
    pub(super) fn download(root: &Path, published: &Published, path: &Path) -> Result<()> {
        let destination = path.with_extension("download");
        let (status, _) = Http::get(root, &published.url, Some(&destination))
            .context("Failed to download canonical PyPI wheel")?;
        ensure!(
            status == 200,
            "Failed to download PyPI wheel: HTTP {status}"
        );
        ensure!(
            Self::digest(&destination)? == published.digest,
            "Failed to verify PyPI wheel checksum"
        );
        fs::rename(destination, path).context("Failed to stage canonical PyPI wheel")
    }

    // Hash exact wheel bytes for registry verification and canonical release bundling.
    pub(super) fn digest(path: &Path) -> Result<String> {
        let bytes = fs::read(path).context("Failed to read wheel checksum input")?;
        Ok(format!("{:x}", sha2::Sha256::digest(bytes)))
    }
}
