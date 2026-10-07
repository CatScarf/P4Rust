use std::{error, fmt};

/// A contextual failure with an inspectable underlying cause.
#[derive(Debug)]
pub struct Error {
    message: String,
    source: Option<Box<dyn error::Error + Send + Sync>>,
    status: Option<crate::CommandStatus>,
}

/// A result returned by the safe Perforce API.
pub type Result<T> = std::result::Result<T, Error>;

impl Error {
    /// Create a failure described by an operation-specific message.
    pub fn new(message: impl Into<String>) -> Self {
        Self {
            message: message.into(),
            source: None,
            status: None,
        }
    }

    // Preserve the original error while describing the failed operation.
    fn caused_by(
        message: impl Into<String>,
        source: impl error::Error + Send + Sync + 'static,
    ) -> Self {
        let status = (&source as &dyn error::Error)
            .downcast_ref::<Self>()
            .and_then(Self::command_status);
        Self {
            message: message.into(),
            source: Some(Box::new(source)),
            status,
        }
    }

    /// Inspect native completion even when a failure has additional operation context.
    pub fn command_status(&self) -> Option<crate::CommandStatus> {
        self.status
    }

    // Preserve a bridge return code on native and SDK command failures.
    pub(crate) fn with_status(mut self, status: crate::CommandStatus) -> Self {
        self.status = Some(status);
        self
    }
}

impl fmt::Display for Error {
    // Render operation context followed by the underlying failure.
    fn fmt(&self, formatter: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(formatter, "{}", self.message)?;
        if let Some(source) = &self.source {
            write!(formatter, ": {source}")?;
        }
        Ok(())
    }
}

impl error::Error for Error {
    // Expose the original cause for standard error inspection.
    fn source(&self) -> Option<&(dyn error::Error + 'static)> {
        self.source
            .as_deref()
            .map(|source| source as &dyn error::Error)
    }
}

/// Attach operation context without discarding the original error.
pub trait ResultExt<T> {
    /// Add a fixed description of the failed operation.
    fn context(self, message: impl Into<String>) -> Result<T>;
    /// Build a description only when the operation fails.
    fn with_context(self, message: impl FnOnce() -> String) -> Result<T>;
}

impl<T, E: error::Error + Send + Sync + 'static> ResultExt<T> for std::result::Result<T, E> {
    // Wrap the original failure with a fixed operation description.
    fn context(self, message: impl Into<String>) -> Result<T> {
        self.map_err(|source| Error::caused_by(message, source))
    }
    // Wrap the original failure with a lazily generated description.
    fn with_context(self, message: impl FnOnce() -> String) -> Result<T> {
        self.map_err(|source| Error::caused_by(message(), source))
    }
}

impl<T> ResultExt<T> for Option<T> {
    // Describe an unexpectedly missing value.
    fn context(self, message: impl Into<String>) -> Result<T> {
        self.ok_or_else(|| Error::new(message))
    }
    // Describe a missing value without allocating on success.
    fn with_context(self, message: impl FnOnce() -> String) -> Result<T> {
        self.ok_or_else(|| Error::new(message()))
    }
}

// Return a contextual validation error when a required condition fails.
macro_rules! ensure {
    ($condition:expr, $($message:tt)*) => {
        if !$condition { return Err($crate::error::Error::new(format!($($message)*))); }
    };
}
pub(crate) use ensure;
