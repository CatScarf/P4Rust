//! Safe Rust bindings for the Perforce C++ API.

mod capture;
mod error;
mod ffi;
mod native;

use error::ensure;
pub use error::{Error, Result, ResultExt};

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

/// Captured text, binary bytes, tagged records, and warnings.
#[derive(Debug)]
pub struct Output {
    pub text: String,
    pub binary: Vec<u8>,
    pub records: Vec<Vec<(String, String)>>,
    pub warnings: Vec<String>,
}

/// A configured client that opens a scoped native connection per command.
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
        native::Native::execute(&self.config, command, args, input)
            .with_context(|| format!("Failed to execute native P4 command '{command}'"))
    }
}

#[cfg(test)]
mod client_tests;
