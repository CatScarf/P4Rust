use crate::{CancellationToken, Config, Result, ResultExt};
use crate::{control::Control, error::ensure, stream::CommandStream};
use std::sync::Arc;
use std::time::Duration;

/// Owned configuration for one background Perforce command.
#[must_use]
pub struct Command {
    pub(crate) config: Config,
    pub(crate) name: String,
    pub(crate) arguments: Vec<String>,
    pub(crate) form: String,
    timeout: Option<Duration>,
    cancellation: Option<CancellationToken>,
    pub(crate) reconcile: Option<Arc<dyn crate::ReconcileHandler>>,
}

impl Command {
    // Capture connection settings without borrowing the client during execution.
    pub(crate) fn new(config: &Config, name: impl Into<String>) -> Self {
        Self {
            config: config.clone(),
            name: name.into(),
            arguments: Vec::new(),
            form: String::new(),
            timeout: None,
            cancellation: None,
            reconcile: None,
        }
    }

    /// Append command arguments in their supplied order.
    pub fn args(mut self, args: impl IntoIterator<Item = impl Into<String>>) -> Self {
        self.arguments.extend(args.into_iter().map(Into::into));
        self
    }

    /// Supply noninteractive form or password input.
    pub fn input(mut self, input: impl Into<String>) -> Self {
        self.form = input.into();
        self
    }

    /// Set a deadline covering connection setup, output delivery, and execution.
    pub fn timeout(mut self, timeout: Duration) -> Self {
        self.timeout = Some(timeout);
        self
    }

    /// Share a caller-owned cancellation token without borrowing it for execution.
    pub fn cancellation(mut self, token: &CancellationToken) -> Self {
        self.cancellation = Some(token.clone());
        self
    }

    /// Delegate local reconcile requests to an owned parallel Rust handler.
    pub fn reconcile_handler(mut self, handler: impl crate::ReconcileHandler + 'static) -> Self {
        self.reconcile = Some(Arc::new(handler));
        self
    }

    /// Start background execution and return a bounded stream of command events.
    pub fn run(self) -> Result<CommandStream> {
        self.validate()
            .context("Failed to validate P4 command builder")?;
        let control = Control::new(self.timeout, self.cancellation.clone())
            .context("Failed to prepare stream control")?;
        CommandStream::start(self, control).context("Failed to start P4 command stream")
    }

    // Validate owned command data before starting the native worker.
    fn validate(&self) -> Result<()> {
        ensure!(
            self.reconcile.is_none()
                || matches!(self.name.as_str(), "reconcile" | "rec" | "status"),
            "Failed to configure reconcile handler: unsupported command"
        );
        ensure!(
            !self.name.is_empty() && !self.name.contains('\0'),
            "Failed to validate command name"
        );
        ensure!(
            self.arguments.len() <= i32::MAX as usize,
            "Failed to validate argument count"
        );
        ensure!(
            !self.form.contains('\0'),
            "Failed to validate form input: embedded NUL"
        );
        for arg in &self.arguments {
            ensure!(
                !arg.contains('\0'),
                "Failed to validate command argument: embedded NUL"
            );
        }
        Ok(())
    }
}
