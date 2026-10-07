use crate::error::{Result, ResultExt, ensure};
use std::{io::Write, process};

pub(crate) struct Runner;

impl Runner {
    // Print and execute every explicit child command with consistent failure context.
    pub(crate) fn run(command: &mut process::Command, capture: bool) -> Result<process::Output> {
        Self::environment(command).context("Failed to prepare command environment")?;
        let display = std::iter::once(command.get_program())
            .chain(command.get_args())
            .map(|value| {
                let value = value.to_string_lossy();
                if value.contains(char::is_whitespace) {
                    format!("\"{value}\"")
                } else {
                    value.into_owned()
                }
            })
            .collect::<Vec<_>>()
            .join(" ");
        println!("> {display}");
        std::io::stdout()
            .flush()
            .context("Failed to flush command preview")?;
        if !capture {
            command
                .stdout(process::Stdio::inherit())
                .stderr(process::Stdio::inherit());
        }
        let output = command
            .output()
            .with_context(|| format!("Failed to launch {display}"))?;
        ensure!(
            output.status.success(),
            "Failed to execute {display}: {}\n{}",
            output.status,
            String::from_utf8_lossy(&output.stderr)
        );
        Ok(output)
    }

    // Supply runner tool paths without changing the producer process's environment.
    fn environment(command: &mut process::Command) -> Result<()> {
        let platform =
            crate::platform::Platform::selected().context("Failed to select command platform")?;
        let mut paths = Vec::new();
        if platform.windows() {
            paths.extend([
                "C:/Strawberry/perl/bin",
                "C:/Program Files/NASM",
                "C:/Program Files/LLVM/bin",
            ]);
        } else if platform.apple() {
            paths.extend(["/opt/homebrew/opt/llvm/bin", "/usr/local/opt/llvm/bin"]);
        }
        let mut paths: Vec<_> = paths.into_iter().map(std::path::PathBuf::from).collect();
        let inherited = command
            .get_envs()
            .find(|(name, _)| *name == "PATH")
            .and_then(|(_, value)| value.map(ToOwned::to_owned))
            .or_else(|| std::env::var_os("PATH"));
        if let Some(inherited) = inherited {
            paths.extend(std::env::split_paths(&inherited));
        }
        command.env(
            "PATH",
            std::env::join_paths(paths).context("Failed to join command tool paths")?,
        );
        Ok(())
    }
}
