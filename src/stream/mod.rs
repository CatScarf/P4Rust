use crate::{Command, CommandStatus, Output, Result, ResultExt};
use crate::{capture::Capture, control::Control, native::Native};
use std::{sync::mpsc, thread, time::Duration};
mod event;
pub use event::{Event, Message, MessageId};
pub use event::{Progress, ProgressCallback, Record};

/// A bounded event iterator that requests cancellation when dropped.
#[must_use]
pub struct CommandStream {
    receiver: mpsc::Receiver<Result<Event>>,
    control: Control,
    worker: Option<thread::JoinHandle<()>>,
    finished: bool,
}

impl CommandStream {
    // Start an owned worker so blocked SDK cleanup cannot outlive borrowed caller data.
    pub(crate) fn start(command: Command, control: Control) -> Result<Self> {
        let (sender, receiver) = mpsc::sync_channel(64);
        let worker_control = control.clone();
        let worker = thread::Builder::new()
            .name("p4rust-command".into())
            .spawn(move || {
                let capture = Capture::new(sender, worker_control.clone());
                let result = std::panic::catch_unwind(std::panic::AssertUnwindSafe(|| {
                    let args: Vec<&str> = command.arguments.iter().map(String::as_str).collect();
                    Native::execute(
                        &command.config,
                        &command.name,
                        &args,
                        &command.form,
                        &worker_control,
                        &capture,
                    )
                    .with_context(|| format!("Failed to execute P4 command '{}'", command.name))
                }));
                let result = match result {
                    Ok(result) => result,
                    Err(_) => Err(crate::Error::new(
                        "Failed to execute P4 command: worker panicked",
                    )),
                };
                let result = match worker_control.error() {
                    Some(error) => Err(error),
                    None => result,
                };
                capture.complete(result);
            })
            .context("Failed to spawn P4 stream worker")?;
        Ok(Self {
            receiver,
            control,
            worker: Some(worker),
            finished: false,
        })
    }

    /// Collect remaining events into a complete output, propagating command failures.
    pub fn collect_output(mut self) -> Result<Output> {
        let mut output = Output::default();
        let mut completed = false;
        for event in &mut self {
            match event.context("Failed to collect P4 event stream")? {
                Event::Text { raw } => output.raw.text.extend(raw),
                Event::Info { level, raw, .. } => {
                    output.raw.text.extend_from_slice(&raw);
                    output.raw.text.push(b'\n');
                    output.info.push((level, raw));
                }
                Event::Binary(bytes) => output.binary.extend(bytes),
                Event::Record(record) => output.raw.records.push(record.raw),
                Event::RecordPartial(record) => output.partial_records.push(record),
                Event::Message(message) | Event::HandleError(message) => {
                    if message.severity == 2 {
                        output.raw.warnings.push(message.raw.clone());
                    } else if message.severity == 1 {
                        output.raw.text.extend_from_slice(&message.raw);
                    }
                    output.messages.push(message);
                }
                Event::OutputError { raw, .. } | Event::NativeError { raw, .. } => {
                    output.errors.push(raw);
                }
                Event::Progress(_) => {}
                Event::Finished => output.finished_callbacks += 1,
                Event::Completed(status) => {
                    output.status = Some(status);
                    completed = true;
                }
            }
        }
        crate::error::ensure!(
            completed,
            "Failed to collect command output: missing completion event"
        );
        output.decode();
        Ok(output)
    }

    // Join a completed worker and preserve a final panic as an explicit stream error.
    fn join(&mut self) -> Result<()> {
        if let Some(worker) = self.worker.take() {
            worker.join().map_err(|_| {
                crate::Error::new("Failed to join P4 command worker: worker panicked")
            })?;
        }
        Ok(())
    }
}

impl Iterator for CommandStream {
    type Item = Result<Event>;

    /// Wait for the next event while checking the caller's deadline and cancellation.
    fn next(&mut self) -> Option<Self::Item> {
        if self.finished {
            return None;
        }
        loop {
            if let Some(error) = self.control.error() {
                self.control.cancel();
                self.finished = true;
                return Some(Err(error).context("Failed to wait for P4 command event"));
            }
            match self.receiver.recv_timeout(Duration::from_millis(10)) {
                Ok(event) => {
                    if event.is_err()
                        || matches!(
                            event,
                            Ok(Event::Completed(CommandStatus { success: true, .. }))
                        )
                    {
                        self.finished = true;
                        if let Err(error) = self.join() {
                            return Some(Err(error));
                        }
                    }
                    return Some(event);
                }
                Err(mpsc::RecvTimeoutError::Timeout) => {}
                Err(error) => {
                    self.finished = true;
                    return Some(Err(error).context("Failed to receive P4 command event"));
                }
            }
        }
    }
}

impl Drop for CommandStream {
    // Request cancellation and let the owned worker finish any uninterruptible cleanup.
    fn drop(&mut self) {
        self.control.cancel();
        if self
            .worker
            .as_ref()
            .is_some_and(thread::JoinHandle::is_finished)
            && let Err(error) = self.join()
        {
            eprintln!("{error}");
        }
    }
}
