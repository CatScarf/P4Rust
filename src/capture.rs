mod message;
use crate::{CommandStatus, Event, Progress, Record, Result};
use crate::{Message, ProgressCallback, ResultExt};
use crate::{control::Control, error::ensure, ffi};
use std::{
    ffi::c_void,
    sync::{Mutex, mpsc},
    thread,
    time::Duration,
};

#[derive(Default)]
struct State {
    record: Option<Vec<(Vec<u8>, Vec<u8>)>>,
    record_bytes: usize,
    partial: bool,
    error_count: Option<i32>,
    failure: Option<crate::Error>,
}

pub(crate) struct Capture {
    sender: mpsc::SyncSender<Result<Event>>,
    control: Control,
    state: Mutex<State>,
}

impl Capture {
    // Coordinate concurrent SDK callbacks through command-local state and a bounded channel.
    pub(crate) fn new(sender: mpsc::SyncSender<Result<Event>>, control: Control) -> Self {
        Self {
            sender,
            control,
            state: Mutex::new(State::default()),
        }
    }

    // Apply backpressure while allowing cancellation and receiver closure to stop delivery.
    fn send(&self, mut event: Result<Event>) -> Result<()> {
        loop {
            if let Some(error) = self.control.error() {
                return Err(error).context("Failed to deliver P4 event");
            }
            match self.sender.try_send(event) {
                Ok(()) => return Ok(()),
                Err(mpsc::TrySendError::Full(pending)) => {
                    event = pending;
                    thread::sleep(Duration::from_millis(10));
                }
                Err(mpsc::TrySendError::Disconnected(_)) => {
                    self.control.cancel();
                    return Err(crate::Error::new(
                        "Failed to deliver P4 event: stream was dropped",
                    ));
                }
            }
        }
    }

    // Preserve each SDK callback boundary while assembling atomic tagged records.
    fn receive(&self, event: u32, data: &[u8], value: &[u8], state: &mut State) -> Result<()> {
        match event {
            ffi::TEXT => {
                ensure!(
                    data.len() <= 1_048_576,
                    "Failed to bound text callback size"
                );
                self.send(Ok(Event::Text { raw: data.to_vec() }))
                    .context("Failed to emit text callback")?;
            }
            ffi::BINARY => {
                ensure!(
                    data.len() <= 1_048_576,
                    "Failed to bound binary callback size"
                );
                self.send(Ok(Event::Binary(data.to_vec())))
                    .context("Failed to emit binary callback")?;
            }
            ffi::RECORD | ffi::RECORD_PARTIAL => {
                ensure!(
                    state.record.is_none(),
                    "Failed to begin record: previous record is incomplete"
                );
                state.record = Some(Vec::new());
                state.record_bytes = 0;
                state.partial = event == ffi::RECORD_PARTIAL;
            }
            ffi::FIELD => {
                ensure!(
                    state
                        .record
                        .as_ref()
                        .is_some_and(|record| record.len() < 16_384),
                    "Failed to bound tagged field count"
                );
                ensure!(
                    data.len() <= 1_048_576 && value.len() <= 1_048_576 - data.len(),
                    "Failed to bound tagged field size"
                );
                state.record_bytes += data.len() + value.len();
                ensure!(
                    state.record_bytes <= 1_048_576,
                    "Failed to bound tagged record size: exceeds 1 MiB"
                );
                state
                    .record
                    .as_mut()
                    .context("Failed to collect field: missing record")?
                    .push((data.to_vec(), value.to_vec()));
            }
            ffi::RECORD_END => self
                .record(state)
                .context("Failed to emit complete record")?,
            ffi::PROGRESS => self
                .progress(data, value)
                .context("Failed to emit SDK progress")?,
            _ => self
                .other(event, data, value, state)
                .context("Failed to emit SDK callback")?,
        }
        Ok(())
    }

    // Forward native message, information, completion, and bridge failure callbacks.
    fn other(&self, event: u32, data: &[u8], value: &[u8], state: &mut State) -> Result<()> {
        ensure!(data.len() <= 1_048_576, "Failed to bound SDK callback size");
        let output = match event {
            ffi::INFO => {
                ensure!(value.len() == 1, "Failed to read native information level");
                Event::Info {
                    level: value[0],
                    text: String::from_utf8_lossy(data).into_owned(),
                    raw: data.to_vec(),
                }
            }
            ffi::MESSAGE | ffi::HANDLE_ERROR => {
                let message =
                    Message::decode(data, value).context("Failed to decode SDK Error object")?;
                if event == ffi::MESSAGE {
                    Event::Message(message)
                } else {
                    Event::HandleError(message)
                }
            }
            ffi::OUTPUT_ERROR => Event::OutputError {
                text: String::from_utf8_lossy(data).into_owned(),
                raw: data.to_vec(),
            },
            ffi::ERROR => {
                let text = String::from_utf8_lossy(data).into_owned();
                self.send(Ok(Event::NativeError {
                    text: text.clone(),
                    raw: data.to_vec(),
                }))
                .context("Failed to emit bridge error")?;
                return Err(crate::Error::new(format!(
                    "Failed to execute native command: {text}"
                )));
            }
            ffi::FINISHED => Event::Finished,
            ffi::STATUS => {
                state.error_count = Some(i32::from_ne_bytes(
                    data.try_into()
                        .context("Failed to decode SDK error count")?,
                ));
                return Ok(());
            }
            _ => {
                return Err(crate::Error::new(format!(
                    "Failed to collect unknown native event: {event}"
                )));
            }
        };
        self.send(Ok(output))
            .context("Failed to forward native callback")
    }

    // Emit ordered display fields and raw bytes only after the native record boundary.
    fn record(&self, state: &mut State) -> Result<()> {
        let raw = state
            .record
            .take()
            .context("Failed to finish tagged record: missing record")?;
        let fields = raw
            .iter()
            .map(|(key, value)| {
                (
                    String::from_utf8_lossy(key).into_owned(),
                    String::from_utf8_lossy(value).into_owned(),
                )
            })
            .collect();
        let record = Record { fields, raw };
        let event = if state.partial {
            Event::RecordPartial(record)
        } else {
            Event::Record(record)
        };
        self.send(Ok(event))
            .context("Failed to deliver tagged record")
    }

    // Decode the fixed-width progress metadata without borrowing native memory afterward.
    fn progress(&self, description: &[u8], metadata: &[u8]) -> Result<()> {
        ensure!(
            metadata.len() == 56 && description.len() <= 16_384,
            "Failed to validate SDK progress frame"
        );
        let mut values = [0_i64; 7];
        for (index, bytes) in metadata.as_chunks::<8>().0.iter().enumerate() {
            values[index] = i64::from_ne_bytes(*bytes);
        }
        self.send(Ok(Event::Progress(Progress {
            id: values[0] as u64,
            kind: values[1] as i32,
            units: values[2] as i32,
            description: String::from_utf8_lossy(description).into_owned(),
            raw_description: description.to_vec(),
            total: values[3],
            current: values[4],
            callback: match values[5] {
                1 => ProgressCallback::Description,
                2 => ProgressCallback::Total,
                3 => ProgressCallback::Update,
                4 => ProgressCallback::Done,
                _ => {
                    return Err(crate::Error::new(
                        "Failed to identify native progress callback",
                    ));
                }
            },
            failure: values[6] as i32,
        })))
        .context("Failed to deliver progress event")
    }

    // Borrow validated native spans only until their callback returns.
    unsafe fn bytes<'a>(pointer: *const u8, length: usize) -> Result<&'a [u8]> {
        if length == 0 {
            return Ok(&[]);
        }
        ensure!(
            !pointer.is_null() && length <= isize::MAX as usize,
            "Failed to validate native byte span"
        );
        // The synchronous callback contract keeps this span valid until callback return.
        Ok(unsafe { std::slice::from_raw_parts(pointer, length) })
    }

    // Contain Rust panics and synchronize callbacks from all parallel SDK workers.
    pub(crate) unsafe extern "C" fn callback(
        context: *mut c_void,
        event: u32,
        data: *const u8,
        length: usize,
        value: *const u8,
        value_length: usize,
    ) -> i32 {
        // The owning command worker keeps this immutable context alive through SDK cleanup.
        let capture = unsafe { &*context.cast::<Self>() };
        let result = std::panic::catch_unwind(std::panic::AssertUnwindSafe(|| -> Result<bool> {
            let mut state = capture.state.lock().map_err(|_| {
                crate::Error::new("Failed to lock native callback state: poisoned mutex")
            })?;
            if state.failure.is_some() {
                return Ok(false);
            }
            // Native bytes are borrowed only while this callback holds the command-local lock.
            let result = unsafe { Self::bytes(data, length) }
                .context("Failed to borrow event data")
                .and_then(|bytes| {
                    unsafe { Self::bytes(value, value_length) }
                        .context("Failed to borrow event value")
                        .and_then(|value| capture.receive(event, bytes, value, &mut state))
                });
            match result {
                Ok(()) => Ok(true),
                Err(error) => {
                    state.failure = Some(error);
                    Ok(false)
                }
            }
        }));
        match result {
            Ok(Ok(true)) => 0,
            Ok(Ok(false)) => 1,
            Ok(Err(error)) => {
                eprintln!("{error}");
                capture.control.cancel();
                1
            }
            Err(_) => {
                eprintln!("Failed to process native callback: Rust callback panicked");
                capture.control.cancel();
                1
            }
        }
    }

    // Preserve the bridge return code after all native callbacks and cleanup finish.
    pub(crate) fn finish(&self, exit_code: i32) -> Result<CommandStatus> {
        let mut state = self
            .state
            .lock()
            .map_err(|_| crate::Error::new("Failed to finish callbacks: poisoned mutex"))?;
        let status = CommandStatus {
            exit_code,
            error_count: state.error_count,
            success: exit_code == 0 && state.failure.is_none() && state.record.is_none(),
        };
        if let Some(error) = state.failure.take() {
            return Err(error.with_status(status)).context("Failed to capture P4 command output");
        }
        if state.record.is_some() {
            return Err(
                crate::Error::new("Failed to collect command output: incomplete record")
                    .with_status(status),
            );
        }
        Ok(status)
    }

    // Deliver exactly one completion or failure after the native session is cleaned up.
    pub(crate) fn complete(&self, result: Result<CommandStatus>) {
        let status = match &result {
            Ok(status) => Some(*status),
            Err(error) => error.command_status(),
        };
        let result = (|| -> Result<()> {
            if let Some(status) = status {
                self.send(Ok(Event::Completed(status)))
                    .context("Failed to emit command status")?;
            }
            let result = result.and_then(|status| {
                if status.success {
                    Ok(())
                } else {
                    Err(crate::Error::new(format!(
                        "Failed to execute P4 command: native exit code {}",
                        status.exit_code
                    ))
                    .with_status(status))
                }
            });
            if let Err(error) = result {
                self.send(Err(error))
                    .context("Failed to emit command failure")?;
            }
            Ok(())
        })();
        if let Err(error) = result
            && self.control.error().is_none()
        {
            eprintln!("{error}");
        }
    }
}
