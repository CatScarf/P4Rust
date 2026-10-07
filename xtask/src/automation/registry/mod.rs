use super::Task;
use crate::{
    error::{Result, ResultExt, ensure},
    platform::Platform,
};
use std::{env, fs, path, thread, time};
mod http;
mod identity;
mod package;
use http::Http;
use package::Package;

pub(crate) struct Registry;

struct Publication {
    package: Package,
    target: &'static str,
    candidate: path::PathBuf,
    published: Option<path::PathBuf>,
}

impl Registry {
    // Print a read-only publication plan without requesting registry credentials.
    pub(crate) fn preview(root: &path::Path) -> Result<()> {
        for item in Self::plan(root).context("Failed to prepare publication preview")? {
            println!(
                "{} {} {}",
                if item.published.is_some() {
                    "Skip"
                } else {
                    "Publish"
                },
                item.package.name,
                item.package.version
            );
        }
        Ok(())
    }

    // Reuse an unchanged published resource or record the inputs for a new native build.
    pub(crate) fn prepare(root: &path::Path, platform: &Platform) -> Result<bool> {
        let metadata = Task::metadata(root).context("Failed to read native resource metadata")?;
        let package = Package::new(&metadata, &format!("p4rust-resources-{}", platform.target))
            .context("Failed to select native resource")?;
        let identity = identity::Identity::resource(root, &platform.target)
            .context("Failed to identify native resource inputs")?;
        let destination = root.join("resources").join(&platform.target);
        if let Some(record) = package
            .index(root)
            .context("Failed to find published resource")?
        {
            let archive = package
                .download(root, &record)
                .context("Failed to download reusable resource")?;
            let source = package
                .unpack(root, &archive)
                .context("Failed to unpack reusable resource")?;
            let previous = fs::read_to_string(source.join("NATIVE-INPUTS.sha256"))
                .context("Failed to read published native inputs; bump the resource version")?;
            ensure!(
                previous.trim() == identity,
                "Failed to reuse {} {}: native inputs changed; bump its version",
                package.name,
                package.version
            );
            Self::restore(&source, &destination).context("Failed to install reusable resource")?;
            println!(
                "Reusing published resource: {} {}",
                package.name, package.version
            );
            return Ok(true);
        }
        fs::write(
            destination.join("NATIVE-INPUTS.sha256"),
            format!("{identity}\n"),
        )
        .context("Failed to record native resource inputs")?;
        Ok(false)
    }

    // Restore only generated payloads without changing tracked manifests or Rust sources.
    fn restore(source: &path::Path, destination: &path::Path) -> Result<()> {
        fs::create_dir_all(destination.join("native"))
            .context("Failed to create resource library directory")?;
        for entry in
            fs::read_dir(source.join("native")).context("Failed to enumerate reusable libraries")?
        {
            let entry = entry.context("Failed to read reusable library")?;
            fs::copy(
                entry.path(),
                destination.join("native").join(entry.file_name()),
            )
            .context("Failed to restore reusable native library")?;
        }
        for name in [
            "NATIVE-INPUTS.sha256",
            "PERFORCE-LICENSE.txt",
            "OPENSSL-LICENSE.txt",
        ] {
            fs::copy(source.join(name), destination.join(name))
                .context("Failed to restore resource metadata")?;
        }
        Ok(())
    }

    // Validate the whole publication plan before uploading resources and then the public crate.
    pub(crate) fn publish(root: &path::Path) -> Result<()> {
        let publications =
            Self::plan(root).context("Failed to prepare crates.io publication plan")?;
        let needed = publications
            .iter()
            .filter(|item| item.published.is_none())
            .count();
        println!(
            "crates.io publication plan: {needed} new versions, {} unchanged",
            publications.len() - needed
        );
        let token = if needed == 0 {
            String::new()
        } else {
            env::var("CARGO_REGISTRY_TOKEN")
                .context("Failed to read CARGO_REGISTRY_TOKEN for new versions")?
        };
        for item in publications {
            let published = match item.published.as_ref() {
                Some(path) => {
                    println!(
                        "Skipping published version: {} {}",
                        item.package.name, item.package.version
                    );
                    path.clone()
                }
                None => {
                    thread::sleep(time::Duration::from_secs(1));
                    Self::upload(root, &item.package, &item.candidate, &token).with_context(
                        || {
                            format!(
                                "Failed to publish {} {}",
                                item.package.name, item.package.version
                            )
                        },
                    )?;
                    item.candidate.clone()
                }
            };
            Self::canonical(root, &item, &published)
                .context("Failed to retain canonical registry artifact")?;
        }
        Ok(())
    }

    // Reject changed contents under an existing version instead of silently skipping changes.
    fn plan(root: &path::Path) -> Result<Vec<Publication>> {
        let metadata = Task::metadata(root).context("Failed to read publish package versions")?;
        let mut selections: Vec<_> = Platform::TARGETS
            .iter()
            .map(|target| (format!("p4rust-resources-{target}"), *target))
            .collect();
        selections.push(("p4rust".to_owned(), Platform::TARGETS[0]));
        let mut publications = Vec::new();
        for (name, target) in selections {
            let package =
                Package::new(&metadata, &name).context("Failed to select publish package")?;
            let candidate = package.artifact(root, target);
            ensure!(
                candidate.is_file(),
                "Failed to find checked package: {}",
                candidate.display()
            );
            let published = if let Some(record) = package
                .index(root)
                .context("Failed to check published version")?
            {
                let archive = package
                    .download(root, &record)
                    .context("Failed to read published artifact")?;
                ensure!(
                    package
                        .same(root, &candidate, &archive)
                        .context("Failed to compare published contents")?,
                    "Failed to publish {name} {}: contents changed; bump its version",
                    package.version
                );
                Some(archive)
            } else {
                None
            };
            publications.push(Publication {
                package,
                target,
                candidate,
                published,
            });
        }
        Ok(publications)
    }

    // Upload the exact CI package and require its checksum to become visible in the index.
    fn upload(
        root: &path::Path,
        package: &Package,
        archive: &path::Path,
        token: &str,
    ) -> Result<()> {
        let body = package
            .upload_body(root, archive)
            .context("Failed to create registry upload body")?;
        let (status, response) =
            Http::upload(root, &body, token).context("Failed to send registry upload")?;
        if !(200..300).contains(&status) {
            if let Some(record) = package
                .index(root)
                .context("Failed to resolve ambiguous upload result")?
            {
                return package
                    .verify(archive, &record)
                    .context("Failed to verify completed upload");
            }
            return Err(crate::error::Error::new(format!(
                "Failed to upload package: HTTP {status}: {}",
                String::from_utf8_lossy(&response)
            )));
        }
        let response: serde_json::Value =
            serde_json::from_slice(&response).context("Failed to decode publish response")?;
        ensure!(
            response["errors"].is_null(),
            "Failed to publish package: {}",
            response["errors"]
        );
        for _ in 0..60 {
            if let Some(record) = package
                .index(root)
                .context("Failed to wait for published index entry")?
            {
                package
                    .verify(archive, &record)
                    .context("Failed to verify published payload")?;
                println!(
                    "Published and indexed: {} {}",
                    package.name, package.version
                );
                return Ok(());
            }
            thread::sleep(time::Duration::from_secs(5));
        }
        Err(crate::error::Error::new(
            "Failed to observe published package in index within five minutes",
        ))
    }

    // Bundle the same bytes consumers download, including on idempotent workflow reruns.
    fn canonical(root: &path::Path, publication: &Publication, archive: &path::Path) -> Result<()> {
        let targets: &[&str] = if publication.package.name == "p4rust" {
            Platform::TARGETS
        } else {
            std::slice::from_ref(&publication.target)
        };
        for target in targets {
            let destination = publication.package.artifact(root, target);
            if archive != destination {
                fs::copy(archive, destination)
                    .context("Failed to stage canonical published crate")?;
            }
        }
        Ok(())
    }
}
