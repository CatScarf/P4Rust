//! Safe Rust bindings for the Perforce C++ API.

// Compile timing scopes out unless diagnostics are explicitly enabled.
macro_rules! reconcile_span {
    ($stage:expr) => {
        #[cfg(feature = "reconcile-trace")]
        let _trace_span = crate::reconcile::trace::ReconcileTrace::span($stage);
    };
}

mod capture;
mod command;
mod control;
mod error;
mod ffi;
mod native;
mod output;
mod reconcile;
mod stream;

pub use command::Command;
pub use control::CancellationToken;
use error::ensure;
pub use error::{CommandStatus, Error, Result, ResultExt};
pub use output::{Output, RawOutput};
pub use reconcile::ReconcileStatistics;
#[cfg(feature = "reconcile-trace")]
pub use reconcile::trace::ReconcileTrace;
pub use reconcile::{FastReconcile, ReconcileHandler, ReconcileKind};
pub use reconcile::{ReconcileReply, ReconcileRequest, ReconcileStatus};
pub use stream::{CommandStream, Event, Progress, Record};
pub use stream::{Message, MessageId, ProgressCallback};
pub use stream::{ProgressKind, ProgressUnit};

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

    /// Configure a Perforce command using one execution path for queries and transfers.
    pub fn command(&self, name: impl Into<String>) -> Command {
        Command::new(&self.config, name)
    }
}
