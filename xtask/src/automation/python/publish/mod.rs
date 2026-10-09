mod fingerprint;
mod index;
use super::{Python, wheels::Wheels};
use crate::{
    automation::{Task, command::Runner},
    error::{Result, ResultExt, ensure},
};
pub(super) use fingerprint::Fingerprint;
use index::Index;
use std::io::Read;
use std::{collections::BTreeSet, fs, io};
use std::{
    path::{Path, PathBuf},
    process::Command,
};

pub(crate) struct Publisher;

impl Publisher {
    // Publish missing wheels through GitHub OIDC and retain canonical registry bytes in Release.
    pub(crate) fn run(root: &Path) -> Result<()> {
        let version = Self::version(root).context("Failed to select Python publication version")?;
        let bundle = Self::download(root, &version).context("Failed to download Python release")?;
        let paths = Self::unpack(&bundle, &version).context("Failed to stage release wheels")?;
        let fingerprint =
            Fingerprint::current(root).context("Failed to identify publication inputs")?;
        for path in &paths {
            Self::validate(root, path, &version, &fingerprint)
                .context("Failed to validate candidate wheel")?;
        }
        let pending = Self::plan(root, &version, &fingerprint, &paths)
            .context("Failed to validate complete PyPI publication plan")?;
        println!(
            "PyPI publication plan: {} new files, {} unchanged",
            pending.len(),
            paths.len() - pending.len()
        );
        if !pending.is_empty() {
            Python::tools(root).context("Failed to prepare PyPI publishing tool")?;
            let mut command = Python::command(
                root,
                &["publish", "--trusted-publishing", "always", "--no-config"],
            )
            .context("Failed to configure trusted publishing")?;
            command.args(&pending);
            Runner::run(&mut command, false).context("Failed to publish Python wheels")?;
        }
        Self::verify(root, &version, &paths).context("Failed to confirm PyPI publication")?;
        Self::bundle(&bundle, &paths).context("Failed to bundle canonical PyPI wheels")?;
        Runner::run(
            Command::new("gh")
                .current_dir(root)
                .args([
                    "release",
                    "upload",
                    &format!("v{version}"),
                    "--repo",
                    "CatScarf/P4Rust",
                    "--clobber",
                ])
                .arg(&bundle),
            false,
        )
        .context("Failed to upload canonical Python release ZIP")?;
        println!(
            "Published https://pypi.org/project/p4rust/{version}/ with {} verified wheels",
            paths.len()
        );
        Ok(())
    }

    // Require matching public Rust and Python binding versions before uploading.
    fn version(root: &Path) -> Result<String> {
        let metadata =
            Task::metadata(root).context("Failed to read Python publication metadata")?;
        let packages = metadata["packages"]
            .as_array()
            .context("Failed to enumerate publication packages")?;
        let mut versions = Vec::new();
        for name in ["p4rust", "p4rust-python"] {
            let package = packages
                .iter()
                .find(|package| package["name"] == name)
                .with_context(|| format!("Failed to find publication package {name}"))?;
            versions.push(
                package["version"]
                    .as_str()
                    .context("Failed to read publication version")?,
            );
        }
        ensure!(
            versions[0] == versions[1],
            "Failed to match Rust and Python publication versions"
        );
        Ok(versions[0].to_owned())
    }

    // Fetch the single Python ZIP produced by the successful six-platform release job.
    fn download(root: &Path, version: &str) -> Result<PathBuf> {
        let directory = root.join("temp/python-publish");
        fs::create_dir_all(&directory).context("Failed to create PyPI staging directory")?;
        let name = format!("p4rust-python-{version}.zip");
        Runner::run(
            Command::new("gh")
                .current_dir(root)
                .args([
                    "release",
                    "download",
                    &format!("v{version}"),
                    "--repo",
                    "CatScarf/P4Rust",
                    "--pattern",
                    &name,
                    "--clobber",
                    "--dir",
                ])
                .arg(&directory),
            false,
        )
        .context("Failed to download GitHub Python release asset")?;
        Ok(directory.join(name))
    }

    // Extract exactly one wheel per platform and ABI without accepting nested paths.
    fn unpack(bundle: &Path, version: &str) -> Result<Vec<PathBuf>> {
        let directory = bundle
            .parent()
            .context("Failed to locate wheel staging parent")?
            .join("wheels");
        fs::create_dir_all(&directory).context("Failed to create wheel staging directory")?;
        let file = fs::File::open(bundle).context("Failed to open Python ZIP")?;
        let mut archive = zip::ZipArchive::new(file).context("Failed to inspect Python ZIP")?;
        ensure!(archive.len() == 12, "Failed to find twelve release wheels");
        let mut slots = BTreeSet::new();
        let mut paths = Vec::new();
        for index in 0..archive.len() {
            let mut entry = archive
                .by_index(index)
                .context("Failed to read wheel ZIP entry")?;
            let name = entry
                .enclosed_name()
                .context("Failed to validate wheel ZIP path")?;
            ensure!(
                name.components().count() == 1
                    && name.extension().is_some_and(|extension| extension == "whl"),
                "Failed to validate flat wheel ZIP entry"
            );
            let path = directory.join(name);
            ensure!(
                slots.insert(Self::slot(&path)?),
                "Failed to reject duplicate platform/ABI wheel"
            );
            let mut file = fs::File::create(&path).context("Failed to create staged wheel")?;
            io::copy(&mut entry, &mut file).context("Failed to unpack wheel")?;
            Wheels::validate(&path, version, path.to_string_lossy().contains("-cp315-"))
                .context("Failed to validate staged wheel")?;
            paths.push(path);
        }
        paths.sort();
        Ok(paths)
    }

    // Identify the supported platform and stable ABI without relying on ZIP ordering.
    fn slot(path: &Path) -> Result<(usize, bool)> {
        let name = path
            .file_name()
            .and_then(|name| name.to_str())
            .context("Failed to read wheel platform tag")?;
        let tags = [
            "win_amd64.whl",
            "win_arm64.whl",
            "manylinux_2_28_x86_64.whl",
            "manylinux_2_28_aarch64.whl",
            "macosx_12_0_x86_64.whl",
            "macosx_12_0_arm64.whl",
        ];
        let platform = tags
            .iter()
            .position(|tag| name.ends_with(tag))
            .context("Failed to match supported Python platform")?;
        Ok((platform, name.contains("-cp315-")))
    }

    // Require the maintained input identity as well as the wheel's public packaging contract.
    fn validate(root: &Path, path: &Path, version: &str, fingerprint: &str) -> Result<()> {
        Wheels::validate(path, version, Self::slot(path)?.1)
            .context("Failed to inspect publication wheel")?;
        Self::description(root, path).context("Failed to verify wheel project description")?;
        ensure!(
            Fingerprint::read(path)? == fingerprint,
            "Failed to match maintained wheel inputs; bump the public and Python versions before publishing changed inputs"
        );
        Ok(())
    }

    // Require the shared README in every wheel before immutable registry publication.
    fn description(root: &Path, path: &Path) -> Result<()> {
        let file = fs::File::open(path).context("Failed to open described wheel")?;
        let mut archive = zip::ZipArchive::new(file).context("Failed to inspect wheel metadata")?;
        let name = archive
            .file_names()
            .find(|name| name.ends_with(".dist-info/METADATA"))
            .context("Failed to locate wheel metadata")?
            .to_owned();
        let mut text = String::new();
        archive
            .by_name(&name)
            .context("Failed to open wheel metadata")?
            .read_to_string(&mut text)
            .context("Failed to read wheel metadata")?;
        let text = text.replace("\r\n", "\n");
        let (headers, body) = text
            .split_once("\n\n")
            .context("Failed to find wheel description")?;
        ensure!(
            headers
                .lines()
                .any(|line| line.starts_with("Description-Content-Type: text/markdown")),
            "Failed to identify Markdown wheel description"
        );
        let readme = fs::read_to_string(root.join("README.md"))
            .context("Failed to read shared project description")?
            .replace("\r\n", "\n");
        ensure!(
            body.trim_end() == readme.trim_end(),
            "Failed to match wheel description with README.md"
        );
        Ok(())
    }

    // Check every existing file before publishing any missing ABI or platform wheels.
    fn plan(
        root: &Path,
        version: &str,
        fingerprint: &str,
        paths: &[PathBuf],
    ) -> Result<Vec<PathBuf>> {
        let published =
            Index::files(root, version).context("Failed to read existing PyPI wheels")?;
        let mut pending = Vec::new();
        for path in paths {
            let name = path
                .file_name()
                .and_then(|name| name.to_str())
                .context("Failed to name candidate wheel")?;
            if let Some(file) = published.get(name) {
                if Index::digest(path)? != file.digest {
                    Index::download(root, file, path)
                        .context("Failed to reuse published Python wheel")?;
                }
                Self::validate(root, path, version, fingerprint)
                    .context("Failed to verify unchanged PyPI version")?;
            } else {
                pending.push(path.clone());
            }
        }
        ensure!(
            published.keys().all(|name| paths
                .iter()
                .any(|path| path.file_name().is_some_and(|file| file == name.as_str()))),
            "Failed to match existing PyPI wheel inventory"
        );
        Ok(pending)
    }

    // Confirm uploaded filenames and checksums before reporting publication success.
    fn verify(root: &Path, version: &str, paths: &[PathBuf]) -> Result<()> {
        for attempt in 0..16 {
            let published = Index::files(root, version)
                .context("Failed to confirm published wheel inventory")?;
            if published.len() == paths.len() {
                return Self::checksums(paths, &published)
                    .context("Failed to verify published wheel bytes");
            }
            ensure!(
                published.len() < paths.len(),
                "Failed to match PyPI wheel inventory"
            );
            ensure!(
                attempt < 15,
                "Failed to confirm twelve published wheels after registry propagation"
            );
            println!(
                "Waiting for PyPI file visibility: {}/{}",
                published.len(),
                paths.len()
            );
            std::thread::sleep(std::time::Duration::from_secs(2));
        }
        Err(crate::error::Error::new(
            "Failed to complete PyPI visibility checks",
        ))
    }

    // Check every immutable registry checksum once all uploads are visible.
    fn checksums(
        paths: &[PathBuf],
        published: &std::collections::BTreeMap<String, index::Published>,
    ) -> Result<()> {
        for path in paths {
            let name = path
                .file_name()
                .and_then(|name| name.to_str())
                .context("Failed to identify uploaded wheel")?;
            let file = published
                .get(name)
                .context("Failed to find uploaded wheel on PyPI")?;
            ensure!(
                Index::digest(path)? == file.digest,
                "Failed to confirm uploaded wheel checksum: {name}"
            );
        }
        Ok(())
    }

    // Make the GitHub distribution identical to the immutable wheels available through pip.
    fn bundle(bundle: &Path, paths: &[PathBuf]) -> Result<()> {
        let file = fs::File::create(bundle).context("Failed to create canonical Python ZIP")?;
        let mut zip = zip::ZipWriter::new(file);
        for path in paths {
            let name = path
                .file_name()
                .and_then(|name| name.to_str())
                .context("Failed to name canonical wheel")?;
            zip.start_file(
                name,
                zip::write::SimpleFileOptions::default()
                    .compression_method(zip::CompressionMethod::Stored),
            )
            .context("Failed to create canonical wheel entry")?;
            let mut file = fs::File::open(path).context("Failed to open canonical wheel")?;
            io::copy(&mut file, &mut zip).context("Failed to bundle canonical wheel")?;
        }
        zip.finish()
            .context("Failed to finish canonical Python ZIP")?;
        Ok(())
    }
}
