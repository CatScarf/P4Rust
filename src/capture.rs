use crate::{Event, Progress, Record, Result, ResultExt};
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
    text: Vec<u8>,
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

    // Process a callback without allowing parallel records or UTF-8 fragments to interleave.
    fn receive(&self, event: u32, data: &[u8], value: &[u8], state: &mut State) -> Result<()> {
        match event {
            ffi::TEXT => {
                for chunk in data.chunks(16_384) {
                    state.text.extend_from_slice(chunk);
                    self.text(state, false)
                        .context("Failed to emit text event")?;
                }
            }
            ffi::BINARY => {
                for chunk in data.chunks(16_384) {
                    self.send(Ok(Event::Binary(chunk.to_vec())))
                        .context("Failed to emit binary event")?;
                }
            }
            ffi::RECORD => {
                ensure!(
                    state.record.is_none(),
                    "Failed to begin record: previous record is incomplete"
                );
                state.record = Some(Vec::new());
                state.record_bytes = 0;
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
            ffi::WARNING => {
                ensure!(data.len() <= 1_048_576, "Failed to bound warning size");
                self.send(Ok(Event::Warning {
                    message: String::from_utf8_lossy(data).into_owned(),
                    raw: data.to_vec(),
                }))
                .context("Failed to emit warning")?;
            }
            ffi::ERROR => {
                let message = String::from_utf8_lossy(&data[..data.len().min(1_048_576)]);
                return Err(crate::Error::new(format!(
                    "Failed to execute P4 command: {message}"
                )));
            }
            ffi::PROGRESS => self
                .progress(data, value)
                .context("Failed to emit SDK progress")?,
            _ => {
                return Err(crate::Error::new(format!(
                    "Failed to collect unknown native event: {event}"
                )));
            }
        }
        Ok(())
    }

    // Preserve incomplete UTF-8 tails until a following callback completes the sequence.
    fn text(&self, state: &mut State, final_chunk: bool) -> Result<()> {
        let mut end = state.text.len();
        let mut position = 0;
        while !final_chunk && position < state.text.len() {
            match std::str::from_utf8(&state.text[position..]) {
                Ok(_) => break,
                Err(error) => {
                    position += error.valid_up_to();
                    if let Some(length) = error.error_len() {
                        position += length;
                    } else {
                        end = position;
                        break;
                    }
                }
            }
        }
        if end != 0 {
            let raw: Vec<_> = state.text.drain(..end).collect();
            let text = String::from_utf8_lossy(&raw).into_owned();
            self.send(Ok(Event::Text { text, raw }))
                .context("Failed to deliver decoded text")?;
        }
        Ok(())
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
        self.send(Ok(Event::Record(Record { fields, raw })))
            .context("Failed to deliver tagged record")
    }

    // Decode the fixed-width progress metadata without borrowing native memory afterward.
    fn progress(&self, description: &[u8], metadata: &[u8]) -> Result<()> {
        ensure!(
            metadata.len() == 56 && description.len() <= 16_384,
            "Failed to validate SDK progress frame"
        );
        let mut values = [0_i64; 7];
        for (index, bytes) in metadata.chunks_exact(8).enumerate() {
            values[index] = i64::from_ne_bytes(
                bytes
                    .try_into()
                    .context("Failed to decode SDK progress value")?,
            );
        }
        self.send(Ok(Event::Progress(Progress {
            id: values[0] as u64,
            kind: values[1] as u32,
            units: values[2] as u32,
            description: String::from_utf8_lossy(description).into_owned(),
            total: u64::try_from(values[3]).ok(),
            current: values[4].max(0) as u64,
            finished: values[5] != 0,
            failed: values[6] != 0,
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

    // Validate native completion and flush the final incomplete text sequence.
    pub(crate) fn finish(&self, status: i32) -> Result<()> {
        let mut state = self
            .state
            .lock()
            .map_err(|_| crate::Error::new("Failed to finish callbacks: poisoned mutex"))?;
        if let Some(error) = state.failure.take() {
            return Err(error).context("Failed to capture P4 command output");
        }
        ensure!(
            status == 0,
            "Failed to execute native P4 command: native status {status}"
        );
        ensure!(
            state.record.is_none(),
            "Failed to collect command output: incomplete record"
        );
        self.text(&mut state, true)
            .context("Failed to flush final text event")
    }

    // Deliver exactly one completion or failure after the native session is cleaned up.
    pub(crate) fn complete(&self, result: Result<()>) {
        if let Err(error) = self.send(result.map(|()| Event::Completed)) {
            if self.control.error().is_none() {
                eprintln!("{error}");
            }
        }
    }
}
