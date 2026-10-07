use crate::{Result, ResultExt};
use std::{io, sync, time};

/// A clonable, one-way cancellation signal shared across selected commands.
#[derive(Clone, Debug, Default)]
pub struct CancellationToken(sync::Arc<sync::atomic::AtomicBool>);

impl CancellationToken {
    /// Request cancellation without blocking or acquiring an application mutex.
    pub fn cancel(&self) {
        self.0.store(true, sync::atomic::Ordering::Release);
    }

    /// Report whether cancellation has been requested.
    pub fn is_cancelled(&self) -> bool {
        self.0.load(sync::atomic::Ordering::Acquire)
    }
}

#[derive(Clone)]
pub(crate) struct Control {
    local: CancellationToken,
    external: Option<CancellationToken>,
    deadline: Option<time::Instant>,
}

impl Control {
    // Create an owned cancellation signal and monotonic command deadline.
    pub(crate) fn new(
        timeout: Option<time::Duration>,
        external: Option<CancellationToken>,
    ) -> Result<Self> {
        let deadline = timeout
            .map(|duration| {
                time::Instant::now()
                    .checked_add(duration)
                    .context("Failed to represent command deadline")
            })
            .transpose()
            .context("Failed to configure command deadline")?;
        Ok(Self {
            local: CancellationToken::default(),
            external,
            deadline,
        })
    }

    // Cancel this command without cancelling other streams sharing the external token.
    pub(crate) fn cancel(&self) {
        self.local.cancel();
    }

    // Preserve standard I/O interruption kinds for caller error inspection.
    pub(crate) fn error(&self) -> Option<crate::Error> {
        let reason = if self.local.is_cancelled()
            || self
                .external
                .as_ref()
                .is_some_and(CancellationToken::is_cancelled)
        {
            Some((io::ErrorKind::Interrupted, "P4 command was cancelled"))
        } else if self
            .deadline
            .is_some_and(|deadline| time::Instant::now() >= deadline)
        {
            Some((io::ErrorKind::TimedOut, "P4 command timed out"))
        } else {
            None
        };
        reason.and_then(|(kind, message)| {
            Err::<(), _>(io::Error::new(kind, message))
                .context("Failed to execute controlled P4 command")
                .err()
        })
    }

    // Check interruption from any SDK thread while the owned worker remains alive.
    pub(crate) unsafe extern "C" fn alive(context: *mut std::ffi::c_void) -> i32 {
        // The native worker owns this immutable control until all SDK threads finish.
        let control = unsafe { &*context.cast::<Self>() };
        i32::from(control.error().is_none())
    }
}
