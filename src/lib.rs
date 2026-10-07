//! Safe Rust bindings for the Perforce C++ API.

mod capture;
mod control;
mod error;
mod ffi;
mod native;
mod output;

pub use control::{CancellationToken, RunOptions};
use error::ensure;
pub use error::{Error, Result, ResultExt};
pub use output::{Output, RawOutput};

/// Explicit settings for a Perforce connection.
#[derive(Clone, Debug)]
pub struct Config {
    pub port: String,
    pub user: String,
    pub client: String,
    pub cwd: String,
    pub charset: String,
}

impl Config {
    /// Create explicit connection settings without changing environment variables.
    pub fn new(
        port: impl Into<String>,
        user: impl Into<String>,
        client: impl Into<String>,
    ) -> Self {
        Self {
            port: port.into(),
            user: user.into(),
            client: client.into(),
            cwd: String::new(),
            charset: String::new(),
        }
    }

    /// Validate connection settings before crossing the native boundary.
    fn validate(&self) -> Result<()> {
        for (name, value) in [
            ("port", &self.port),
            ("user", &self.user),
            ("client", &self.client),
            ("cwd", &self.cwd),
            ("charset", &self.charset),
        ] {
            ensure!(
                !value.contains('\0'),
                "Failed to validate {name}: embedded NUL"
            );
            if matches!(name, "port" | "user" | "client") {
                ensure!(
                    !value.is_empty(),
                    "Failed to validate {name}: value is empty"
                );
            }
        }
        Ok(())
    }
}

/// A thread-safe configured client that opens an independent connection per command.
pub struct Client {
    config: Config,
}

impl Client {
    /// Construct a client after validating explicit connection settings.
    pub fn new(config: Config) -> Result<Self> {
        config.validate().context("Failed to construct P4 client")?;
        Ok(Self { config })
    }

    /// Run a Perforce command with noninteractive form input disabled.
    pub fn run(&self, command: &str, args: &[&str]) -> Result<Output> {
        self.run_with_input(command, args, "")
            .context("Failed to run P4 command")
    }

    /// Run a Perforce command with explicit noninteractive form input.
    pub fn run_with_input(&self, command: &str, args: &[&str], input: &str) -> Result<Output> {
        self.execute(command, args, input, None)
            .context("Failed to run P4 command with input")
    }

    /// Run a command with a deadline and cancellation signal.
    pub fn run_with_options(
        &self,
        command: &str,
        args: &[&str],
        options: &RunOptions,
    ) -> Result<Output> {
        self.run_with_input_and_options(command, args, "", options)
            .context("Failed to run controlled P4 command")
    }

    /// Run a command with explicit input, a deadline, and cancellation.
    pub fn run_with_input_and_options(
        &self,
        command: &str,
        args: &[&str],
        input: &str,
        options: &RunOptions,
    ) -> Result<Output> {
        self.execute(command, args, input, Some(options))
            .context("Failed to run controlled P4 command with input")
    }

    // Validate command data before choosing synchronous or controlled execution.
    fn execute(
        &self,
        command: &str,
        args: &[&str],
        input: &str,
        options: Option<&RunOptions>,
    ) -> Result<Output> {
        ensure!(
            !command.is_empty() && !command.contains('\0'),
            "Failed to validate command name"
        );
        ensure!(
            args.len() <= i32::MAX as usize,
            "Failed to validate arguments: too many arguments"
        );
        ensure!(
            !input.contains('\0'),
            "Failed to validate input: embedded NUL"
        );
        for arg in args {
            ensure!(
                !arg.contains('\0'),
                "Failed to validate argument: embedded NUL"
            );
        }
        let result = match options {
            Some(options) => control::Control::execute(&self.config, command, args, input, options),
            None => native::Native::execute(&self.config, command, args, input, None),
        };
        result.with_context(|| format!("Failed to execute native P4 command '{command}'"))
    }
}

#[cfg(test)]
mod client_tests;
#[cfg(test)]
mod concurrency_tests;
