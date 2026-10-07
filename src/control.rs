use crate::{Config, Output, Result, ResultExt, native::Native};
use std::{io, sync, thread, time};

/// A clonable, one-way cancellation signal that can be shared across commands.
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

/// Per-command deadline and cancellation settings, including connection setup.
#[derive(Clone, Debug, Default)]
pub struct RunOptions {
    pub timeout: Option<time::Duration>,
    pub cancellation: CancellationToken,
}

#[derive(Clone)]
pub(crate) struct Control {
    token: CancellationToken,
    deadline: Option<time::Instant>,
}

impl Control {
    // Preserve a precise monotonic deadline without rounding subsecond timeouts.
    fn new(options: &RunOptions) -> Result<Self> {
        let deadline = match options.timeout {
            Some(timeout) => Some(
                time::Instant::now()
                    .checked_add(timeout)
                    .context("Failed to represent command deadline")?,
            ),
            None => None,
        };
        Ok(Self {
            token: options.cancellation.clone(),
            deadline,
        })
    }

    // Identify interruption using standard I/O error kinds instead of message parsing.
    fn reason(&self) -> Option<(io::ErrorKind, &'static str)> {
        if self.token.is_cancelled() {
            Some((io::ErrorKind::Interrupted, "P4 command was cancelled"))
        } else if self
            .deadline
            .is_some_and(|deadline| time::Instant::now() >= deadline)
        {
            Some((io::ErrorKind::TimedOut, "P4 command timed out"))
        } else {
            None
        }
    }

    // Preserve native failures when an interruption terminates the same operation.
    fn finish(&self, result: Result<Output>) -> Result<Output> {
        match self.reason() {
            Some((kind, message)) => {
                let error = match result {
                    Err(error) => io::Error::new(kind, error),
                    Ok(_) => io::Error::new(kind, message),
                };
                Err(error)
                    .context(message)
                    .context("Failed to complete controlled P4 command")
            }
            None => result,
        }
    }

    // Check cancellation from the owning native thread without panicking or mutating state.
    pub(crate) unsafe extern "C" fn alive(context: *mut std::ffi::c_void) -> i32 {
        // The worker owns this control until all native callbacks and cleanup have finished.
        let control = unsafe { &*context.cast::<Self>() };
        i32::from(control.reason().is_none())
    }

    // Wait with bounded cancellation latency while native execution owns every borrowed buffer.
    fn wait(&self, receiver: sync::mpsc::Receiver<Result<Output>>) -> Result<Output> {
        loop {
            if let Some((kind, message)) = self.reason() {
                return Err(io::Error::new(kind, message)).context("Failed to wait for P4 command");
            }
            let interval = self
                .deadline
                .map(|deadline| deadline.saturating_duration_since(time::Instant::now()))
                .unwrap_or(time::Duration::from_millis(10))
                .min(time::Duration::from_millis(10));
            match receiver.recv_timeout(interval) {
                Ok(result) => {
                    return self
                        .finish(result)
                        .context("Failed to finish P4 command worker");
                }
                Err(sync::mpsc::RecvTimeoutError::Timeout) => continue,
                Err(error) => {
                    return Err(error).context("Failed to receive P4 command worker result");
                }
            }
        }
    }

    // Run owned native state so a deadline can return during blocked DNS or connection setup.
    pub(crate) fn execute(
        config: &Config,
        command: &str,
        args: &[&str],
        input: &str,
        options: &RunOptions,
    ) -> Result<Output> {
        let control = Self::new(options).context("Failed to prepare command control")?;
        if let Some((kind, message)) = control.reason() {
            return Err(io::Error::new(kind, message))
                .context("Failed to start controlled P4 command");
        }
        let config = config.clone();
        let command = command.to_owned();
        let args: Vec<String> = args.iter().map(|arg| (*arg).to_owned()).collect();
        let input = input.to_owned();
        let worker_control = control.clone();
        let (sender, receiver) = sync::mpsc::channel();
        let _worker = thread::Builder::new()
            .name("p4rust-command".into())
            .spawn(move || {
                let args: Vec<&str> = args.iter().map(String::as_str).collect();
                let result =
                    Native::execute(&config, &command, &args, &input, Some(&worker_control))
                        .context("Failed to execute controlled native command");
                // A closed receiver is expected after the caller has returned an interruption.
                let _delivery = sender.send(worker_control.finish(result));
            })
            .context("Failed to spawn P4 command worker")?;
        control
            .wait(receiver)
            .context("Failed to wait for controlled native command")
    }
}
