use super::{Task, command::Runner};
use crate::error::{Result, ResultExt};
use std::{env, path::Path, process::Command};

pub(crate) struct GitHub;

impl GitHub {
    // Run GitHub CLI requests through the same visible command executor as builds.
    fn gh(root: &Path, args: &[&str], capture: bool) -> Result<String> {
        let output = Runner::run(Command::new("gh").args(args).current_dir(root), capture)
            .context("Failed to run GitHub CLI")?;
        String::from_utf8(output.stdout).context("Failed to decode GitHub CLI output")
    }

    // Publish independent Rust and Python ZIP assets under the public Cargo version.
    pub(crate) fn publish(root: &Path) -> Result<()> {
        let metadata = Task::metadata(root).context("Failed to read release metadata")?;
        let version = metadata["packages"]
            .as_array()
            .context("Failed to read release packages")?
            .iter()
            .find(|package| package["name"] == "p4rust")
            .context("Failed to find release crate")?["version"]
            .as_str()
            .context("Failed to read release version")?;
        super::bundle::Bundle::create(root, version).context("Failed to merge release ZIP")?;
        super::registry::Registry::publish(root)
            .context("Failed to publish checked crates.io packages")?;
        super::bundle::Bundle::create(root, version)
            .context("Failed to bundle canonical published packages")?;
        let repository =
            env::var("GITHUB_REPOSITORY").context("Failed to read release repository")?;
        let sha = env::var("GITHUB_SHA").context("Failed to read release commit")?;
        let tag = format!("v{version}");
        let assets = [
            format!("p4rust-rust-{version}.zip"),
            format!("p4rust-python-{version}.zip"),
        ];
        let archives: Vec<_> = assets
            .iter()
            .map(|asset| format!("temp/release-output/{asset}"))
            .collect();
        let title = format!("P4Rust {version}");
        let notes = format!(
            "Rust crates and typed Python wheels for six supported targets. Python: ordinary CPython 3.9+ and free-threaded CPython 3.15+. Source commit: {sha}."
        );
        let exists =
            Self::exists(root, &repository, &tag).context("Failed to locate versioned release")?;
        let mut args = vec!["release"];
        if exists {
            Self::replace(root, &repository, &sha, &tag, &assets, &archives)
                .context("Failed to replace same-version release")?;
            args.extend([
                "edit",
                &tag,
                "--draft=false",
                "--prerelease=false",
                "--target",
                &sha,
            ]);
        } else {
            args.extend(["create", &tag]);
            args.extend(archives.iter().map(String::as_str));
            args.extend(["--target", &sha]);
        }
        args.extend([
            "--repo",
            &repository,
            "--title",
            &title,
            "--notes",
            &notes,
            "--latest",
        ]);
        Self::gh(root, &args, false).context("Failed to publish release metadata")?;
        Ok(())
    }

    // Find existing version tags without treating API or authentication errors as absence.
    fn exists(root: &Path, repository: &str, tag: &str) -> Result<bool> {
        let releases: serde_json::Value = serde_json::from_str(
            &Self::gh(
                root,
                &[
                    "release", "list", "--repo", repository, "--limit", "100000", "--json",
                    "tagName",
                ],
                true,
            )
            .context("Failed to list versioned releases")?,
        )
        .context("Failed to decode release list")?;
        Ok(releases
            .as_array()
            .context("Failed to read release list")?
            .iter()
            .any(|release| release["tagName"] == tag))
    }

    // Replace both current ZIPs and remove obsolete release assets after successful uploads.
    fn replace(
        root: &Path,
        repository: &str,
        sha: &str,
        tag: &str,
        assets: &[String],
        archives: &[String],
    ) -> Result<()> {
        Self::gh(
            root,
            &[
                "api",
                "--method",
                "PATCH",
                &format!("repos/{repository}/git/refs/tags/{tag}"),
                "-f",
                &format!("sha={sha}"),
                "-F",
                "force=true",
            ],
            false,
        )
        .context("Failed to update release tag")?;
        let mut upload = vec!["release", "upload", tag];
        upload.extend(archives.iter().map(String::as_str));
        upload.extend(["--repo", repository, "--clobber"]);
        Self::gh(root, &upload, false).context("Failed to replace release ZIP")?;
        let release: serde_json::Value = serde_json::from_str(
            &Self::gh(
                root,
                &["api", &format!("repos/{repository}/releases/tags/{tag}")],
                true,
            )
            .context("Failed to read release assets")?,
        )
        .context("Failed to decode release assets")?;
        for item in release["assets"]
            .as_array()
            .context("Failed to enumerate release assets")?
        {
            if !assets.iter().any(|asset| item["name"] == asset.as_str()) {
                let id = item["id"]
                    .as_u64()
                    .context("Failed to read obsolete asset identifier")?;
                Self::gh(
                    root,
                    &[
                        "api",
                        "--method",
                        "DELETE",
                        &format!("repos/{repository}/releases/assets/{id}"),
                    ],
                    false,
                )
                .context("Failed to remove obsolete release asset")?;
            }
        }
        Ok(())
    }
}
