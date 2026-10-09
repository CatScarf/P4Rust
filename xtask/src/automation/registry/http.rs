use super::super::command::Runner;
use crate::error::{Result, ResultExt, ensure};
use std::{
    fs,
    path::{Path, PathBuf},
    process::Command,
};

pub(crate) struct Http;

impl Http {
    // Fetch public registry data with bounded retries and an identifiable user agent.
    pub(crate) fn get(
        root: &Path,
        url: &str,
        destination: Option<&Path>,
    ) -> Result<(u16, Vec<u8>)> {
        let output = match destination {
            Some(path) => path.to_path_buf(),
            None => Self::response(root).context("Failed to stage registry response")?,
        };
        let mut command = Self::command(root, url, &output);
        command.arg("--location");
        let result = Runner::run(&mut command, true).context("Failed to download registry data")?;
        let status =
            Self::status(&result.stdout).context("Failed to decode registry download status")?;
        let bytes = if destination.is_none() {
            fs::read(output).context("Failed to read registry response")?
        } else {
            Vec::new()
        };
        Ok((status, bytes))
    }

    // Upload the exact checked crate while keeping authorization out of argv and files.
    pub(super) fn upload(root: &Path, body: &Path, token: &str) -> Result<(u16, Vec<u8>)> {
        ensure!(
            !token.is_empty()
                && token
                    .bytes()
                    .all(|byte| byte.is_ascii_alphanumeric() || b"_-.".contains(&byte)),
            "Failed to validate registry token"
        );
        let response = Self::response(root).context("Failed to stage publish response")?;
        let mut command = Self::command(root, "https://crates.io/api/v1/crates/new", &response);
        command
            .args([
                "--request",
                "PUT",
                "--header",
                "Content-Type: application/octet-stream",
                "--header",
                "Accept: application/json",
                "--config",
                "-",
                "--data-binary",
            ])
            .arg(format!("@{}", body.display()));
        let input = format!("header = \"Authorization: {token}\"\n");
        let output = Runner::input(&mut command, input.as_bytes())
            .context("Failed to upload registry package")?;
        let status =
            Self::status(&output.stdout).context("Failed to decode registry upload status")?;
        Ok((
            status,
            fs::read(response).context("Failed to read publish response")?,
        ))
    }

    // Let curl honor Retry-After for throttling and retry transient HTTP failures.
    fn command(root: &Path, url: &str, output: &Path) -> Command {
        let mut command = Command::new("curl");
        command.current_dir(root).args([
            "--silent",
            "--show-error",
            "--retry",
            "8",
            "--retry-max-time",
            "1200",
            "--connect-timeout",
            "30",
            "--max-time",
            "120",
            "--user-agent",
            "P4Rust-xtask (https://github.com/CatScarf/P4Rust)",
            "--write-out",
            "%{http_code}",
            "--header",
            "Cache-Control: no-cache",
            "--max-filesize",
            "10000000",
            url,
        ]);
        command.arg("--output").arg(output);
        command
    }

    // Parse curl's HTTP status without mixing response bodies from retried requests.
    fn status(bytes: &[u8]) -> Result<u16> {
        std::str::from_utf8(bytes)
            .context("Failed to decode HTTP status")?
            .trim()
            .parse()
            .context("Failed to parse HTTP status")
    }

    // Reuse an ignored response file for sequential registry requests.
    fn response(root: &Path) -> Result<PathBuf> {
        let directory = root.join("temp/registry");
        fs::create_dir_all(&directory).context("Failed to create registry request directory")?;
        Ok(directory.join("http-response"))
    }
}
