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
mod config;
mod control;
mod error;
mod ffi;
mod native;
mod output;
mod reconcile;
mod session;
mod stream;

pub use command::Command;
pub use config::Config;
pub use control::CancellationToken;
pub use error::{CommandStatus, Error, Result, ResultExt};
pub use output::{Output, RawOutput};
pub use reconcile::ReconcileStatistics;
#[cfg(feature = "reconcile-trace")]
pub use reconcile::trace::ReconcileTrace;
pub use reconcile::{FastReconcile, ReconcileHandler, ReconcileKind};
pub use reconcile::{ReconcileReply, ReconcileRequest, ReconcileStatus};
pub use session::Session;
pub use stream::{CommandStream, Event, Progress, Record};
pub use stream::{Message, MessageId, ProgressCallback};
pub use stream::{ProgressKind, ProgressUnit};

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

    /// Create a thread-owned connection for sequential commands.
    pub fn session(&self) -> Result<Session> {
        Session::new(self.config.clone()).context("Failed to create persistent P4 session")
    }

    /// Query this client's SDK ignore rules without opening a server connection.
    pub fn is_ignored(&self, path: impl Into<String>) -> Result<bool> {
        self.config
            .is_ignored(path)
            .context("Failed to query client ignore policy")
    }
}
