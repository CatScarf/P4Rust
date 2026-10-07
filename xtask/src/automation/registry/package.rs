use super::{super::command::Runner, Http};
use crate::error::{Result, ResultExt, ensure};
use sha2::Digest;
use std::{
    collections::BTreeMap,
    fs,
    path::{Path, PathBuf},
    process::Command,
};

pub(super) struct Package {
    pub(super) name: String,
    pub(super) version: String,
    metadata: serde_json::Value,
}

impl Package {
    // Select each package's independent version and Cargo publishing metadata.
    pub(super) fn new(metadata: &serde_json::Value, name: &str) -> Result<Self> {
        let package = metadata["packages"]
            .as_array()
            .context("Failed to read workspace packages")?
            .iter()
            .find(|package| package["name"] == name)
            .context("Failed to find publish package")?;
        Ok(Self {
            name: name.to_owned(),
            version: package["version"]
                .as_str()
                .context("Failed to read package version")?
                .to_owned(),
            metadata: package.clone(),
        })
    }

    // Locate a checked crate within its platform's downloaded CI artifact.
    pub(super) fn artifact(&self, root: &Path, target: &str) -> PathBuf {
        root.join("temp/release-inputs")
            .join(format!("p4rust-{target}"))
            .join(format!("{}-{}.crate", self.name, self.version))
    }

    // Query the sparse index without treating network failures as an absent version.
    pub(super) fn index(&self, root: &Path) -> Result<Option<serde_json::Value>> {
        let name = &self.name;
        let url = format!(
            "https://index.crates.io/{}/{}/{name}",
            &name[..2],
            &name[2..4]
        );
        let (status, bytes) =
            Http::get(root, &url, None).context("Failed to query registry index")?;
        if status == 404 {
            return Ok(None);
        }
        ensure!(
            status == 200,
            "Failed to read registry index: HTTP {status}"
        );
        let text = std::str::from_utf8(&bytes).context("Failed to decode registry index")?;
        for line in text.lines().filter(|line| !line.is_empty()) {
            let record: serde_json::Value =
                serde_json::from_str(line).context("Failed to parse registry index entry")?;
            if record["vers"] == self.version {
                ensure!(
                    record["yanked"] == false,
                    "Failed to reuse yanked package {} {}",
                    self.name,
                    self.version
                );
                return Ok(Some(record));
            }
        }
        Ok(None)
    }

    // Download the published artifact and verify the registry's SHA-256 checksum.
    pub(super) fn download(&self, root: &Path, record: &serde_json::Value) -> Result<PathBuf> {
        let directory = root.join("temp/registry/downloads");
        fs::create_dir_all(&directory).context("Failed to create registry download directory")?;
        let path = directory.join(format!("{}-{}.crate", self.name, self.version));
        let url = format!(
            "https://static.crates.io/crates/{0}/{0}-{1}.crate",
            self.name, self.version
        );
        let (status, _) =
            Http::get(root, &url, Some(&path)).context("Failed to download published package")?;
        ensure!(
            status == 200,
            "Failed to download published package: HTTP {status}"
        );
        self.verify(&path, record)
            .context("Failed to verify published package")?;
        Ok(path)
    }

    // Reject a registry version whose payload differs from the expected checked artifact.
    pub(super) fn verify(&self, path: &Path, record: &serde_json::Value) -> Result<()> {
        let bytes = fs::read(path).context("Failed to read package checksum input")?;
        let checksum = format!("{:x}", sha2::Sha256::digest(&bytes));
        ensure!(
            record["cksum"] == checksum,
            "Failed to verify checksum for {} {}",
            self.name,
            self.version
        );
        Ok(())
    }

    // Extract checked crate entries into an isolated directory keyed by archive checksum.
    pub(super) fn unpack(&self, root: &Path, archive: &Path) -> Result<PathBuf> {
        let bytes = fs::read(archive).context("Failed to read package archive")?;
        let directory = root
            .join("temp/registry/unpacked")
            .join(format!("{:x}", sha2::Sha256::digest(bytes)));
        let prefix = format!("{}-{}/", self.name, self.version);
        let entries = Runner::run(Command::new("tar").arg("-tzf").arg(archive), true)
            .context("Failed to list registry package entries")?;
        let entries =
            String::from_utf8(entries.stdout).context("Failed to decode package entries")?;
        ensure!(
            entries.lines().all(|entry| entry.starts_with(&prefix)
                && !entry.contains("../")
                && !entry.contains('\\')),
            "Failed to validate package entry paths"
        );
        fs::create_dir_all(&directory).context("Failed to create package extraction directory")?;
        Runner::run(
            Command::new("tar")
                .arg("-xzf")
                .arg(archive)
                .arg("-C")
                .arg(&directory),
            false,
        )
        .context("Failed to extract registry package")?;
        Ok(directory.join(prefix))
    }

    // Compare package contents while excluding Cargo's provenance and generated lock files.
    pub(super) fn same(&self, root: &Path, candidate: &Path, published: &Path) -> Result<bool> {
        let candidate = self
            .unpack(root, candidate)
            .context("Failed to unpack candidate package")?;
        let published = self
            .unpack(root, published)
            .context("Failed to unpack published package")?;
        let mut current = BTreeMap::new();
        let mut existing = BTreeMap::new();
        Self::contents(&candidate, &candidate, &mut current)
            .context("Failed to fingerprint candidate contents")?;
        Self::contents(&published, &published, &mut existing)
            .context("Failed to fingerprint published contents")?;
        Ok(current == existing)
    }

    // Hash extracted file contents independently of tar timestamps and directory ordering.
    fn contents(
        base: &Path,
        directory: &Path,
        output: &mut BTreeMap<String, String>,
    ) -> Result<()> {
        for entry in fs::read_dir(directory).context("Failed to enumerate package contents")? {
            let entry = entry.context("Failed to read package entry")?;
            let path = entry.path();
            ensure!(
                !entry
                    .file_type()
                    .context("Failed to inspect package entry")?
                    .is_symlink(),
                "Failed to fingerprint package symlink"
            );
            if path.is_dir() {
                Self::contents(base, &path, output)
                    .context("Failed to fingerprint package directory")?;
            } else {
                let name = path
                    .strip_prefix(base)
                    .context("Failed to identify package file")?
                    .to_string_lossy()
                    .replace('\\', "/");
                if [".cargo_vcs_info.json", "Cargo.toml.orig", "Cargo.lock"]
                    .contains(&name.as_str())
                {
                    continue;
                }
                let bytes = fs::read(path).context("Failed to hash package file")?;
                output.insert(name, format!("{:x}", sha2::Sha256::digest(bytes)));
            }
        }
        Ok(())
    }

    // Encode Cargo's upload metadata for the exact precompiled CI artifact.
    pub(super) fn upload_body(&self, root: &Path, archive: &Path) -> Result<PathBuf> {
        let metadata = self
            .publish_metadata()
            .context("Failed to prepare registry metadata")?;
        let metadata =
            serde_json::to_vec(&metadata).context("Failed to encode registry metadata")?;
        let bytes = fs::read(archive).context("Failed to read registry upload archive")?;
        ensure!(
            bytes.len() < 10_000_000,
            "Failed to meet registry package size limit"
        );
        let mut body = Vec::new();
        body.extend(
            u32::try_from(metadata.len())
                .context("Failed to encode metadata length")?
                .to_le_bytes(),
        );
        body.extend(metadata);
        body.extend(
            u32::try_from(bytes.len())
                .context("Failed to encode archive length")?
                .to_le_bytes(),
        );
        body.extend(bytes);
        let directory = root.join("temp/registry/uploads");
        fs::create_dir_all(&directory).context("Failed to create registry upload directory")?;
        let path = directory.join(format!("{}-{}.body", self.name, self.version));
        fs::write(&path, body).context("Failed to stage registry upload")?;
        Ok(path)
    }

    // Translate Cargo metadata fields and dependency requirements to the registry schema.
    fn publish_metadata(&self) -> Result<serde_json::Value> {
        let source = &self.metadata;
        let mut result = serde_json::json!({"name": self.name, "vers": self.version,
            "features": source["features"], "badges": {}, "readme": null, "readme_file": source["readme"]});
        for field in [
            "authors",
            "description",
            "documentation",
            "homepage",
            "keywords",
            "categories",
            "license",
            "license_file",
            "repository",
            "links",
            "rust_version",
        ] {
            result[field] = source[field].clone();
        }
        if let Some(readme) = source["readme"].as_str() {
            let manifest = Path::new(
                source["manifest_path"]
                    .as_str()
                    .context("Failed to locate publish manifest")?,
            );
            let path = manifest
                .parent()
                .context("Failed to locate readme directory")?
                .join(readme);
            result["readme"] = fs::read_to_string(path)
                .context("Failed to read published readme")?
                .into();
        }
        result["deps"] = source["dependencies"].as_array().context("Failed to read publish dependencies")?
            .iter().map(|dependency| serde_json::json!({"name": dependency["name"],
                "version_req": dependency["req"], "features": dependency["features"],
                "optional": dependency["optional"], "default_features": dependency["uses_default_features"],
                "target": dependency["target"], "kind": dependency["kind"].as_str().unwrap_or("normal"),
                "registry": dependency["registry"], "explicit_name_in_toml": dependency["rename"]}))
            .collect::<Vec<_>>().into();
        Ok(result)
    }
}
