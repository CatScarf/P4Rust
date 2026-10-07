mod decode;
mod message;
mod record;
use crate::{CommandStatus, Event, Result, ResultExt};
use crate::{control::Control, error::ensure, stream::queue::Queue};
use std::{ffi::c_void, sync};

#[derive(Default)]
struct State {
    next_ticket: u64,
    error_count: Option<i32>,
    failure: Option<crate::Error>,
}

impl State {
    // Reserve an ordering ticket while callback state is exclusively borrowed.
    fn ticket(&mut self) -> Result<u64> {
        let ticket = self.next_ticket;
        self.next_ticket = ticket
            .checked_add(1)
            .context("Failed to reserve event ticket")?;
        Ok(ticket)
    }
}

pub(crate) struct Capture {
    queue: sync::Arc<Queue>,
    control: Control,
    state: sync::Mutex<State>,
}

impl Capture {
    // Coordinate callback decoding separately from ordered bounded event delivery.
    pub(crate) fn new(queue: sync::Arc<Queue>, control: Control) -> Self {
        Self {
            queue,
            control,
            state: sync::Mutex::new(State::default()),
        }
    }

    // Reserve completion ordering without holding callback state during queue backpressure.
    fn send(&self, event: Result<Event>) -> Result<()> {
        let ticket = self
            .state
            .lock()
            .map_err(|_| crate::Error::new("Failed to lock event ordering: poisoned mutex"))?
            .ticket()
            .context("Failed to reserve completion ticket")?;
        self.queue
            .push(ticket, event, &self.control)
            .context("Failed to deliver completion event")
    }

    // Borrow validated native spans only until their synchronous callback returns.
    unsafe fn bytes<'a>(pointer: *const u8, length: usize) -> Result<&'a [u8]> {
        if length == 0 {
            return Ok(&[]);
        }
        ensure!(
            !pointer.is_null() && length <= isize::MAX as usize,
            "Failed to validate native byte span"
        );
        // The callback contract keeps this memory valid until callback return.
        Ok(unsafe { std::slice::from_raw_parts(pointer, length) })
    }

    // Decode and order callbacks under the state lock, then release it before delivery.
    unsafe fn dispatch(
        &self,
        kind: u32,
        data: *const u8,
        length: usize,
        value: *const u8,
        value_length: usize,
    ) -> Result<bool> {
        let prepared = {
            let mut state = self.state.lock().map_err(|_| {
                crate::Error::new("Failed to lock native callback state: poisoned mutex")
            })?;
            if state.failure.is_some() {
                return Ok(false);
            }
            // Native spans and descriptors remain borrowed only during this SDK callback.
            let result = unsafe { Self::bytes(data, length) }
                .context("Failed to borrow event data")
                .and_then(|data| {
                    unsafe { Self::bytes(value, value_length) }
                        .context("Failed to borrow event value")
                        .and_then(|value| unsafe { Self::prepare(kind, data, value, &mut state) })
                });
            match result {
                Ok(Some(event)) => Some((
                    state
                        .ticket()
                        .context("Failed to reserve callback ticket")?,
                    event,
                    state.failure.is_none(),
                )),
                Ok(None) => None,
                Err(error) => {
                    state.failure = Some(error);
                    return Ok(false);
                }
            }
        };
        if let Some((ticket, event, alive)) = prepared {
            if let Err(error) = self.queue.push(ticket, Ok(event), &self.control) {
                let mut state = self.state.lock().map_err(|_| {
                    crate::Error::new("Failed to record delivery failure: poisoned mutex")
                })?;
                if state.failure.is_none() {
                    state.failure = Some(error);
                }
                return Ok(false);
            }
            return Ok(alive);
        }
        Ok(true)
    }

    // Contain Rust panics while parallel callbacks coordinate ordering and cancellation.
    pub(crate) unsafe extern "C" fn callback(
        context: *mut c_void,
        event: u32,
        data: *const u8,
        length: usize,
        value: *const u8,
        value_length: usize,
    ) -> i32 {
        // The command worker owns this context through every callback and SDK cleanup.
        let capture = unsafe { &*context.cast::<Self>() };
        let result = std::panic::catch_unwind(std::panic::AssertUnwindSafe(|| {
            unsafe { capture.dispatch(event, data, length, value, value_length) }
                .context("Failed to dispatch native callback")
        }));
        match result {
            Ok(Ok(alive)) => i32::from(!alive),
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

    // Preserve the bridge return code after all callbacks and native cleanup finish.
    pub(crate) fn finish(&self, exit_code: i32) -> Result<CommandStatus> {
        let mut state = self
            .state
            .lock()
            .map_err(|_| crate::Error::new("Failed to finish callbacks: poisoned mutex"))?;
        let status = CommandStatus {
            exit_code,
            error_count: state.error_count,
            success: exit_code == 0 && state.failure.is_none(),
        };
        if let Some(error) = state.failure.take() {
            return Err(error.with_status(status)).context("Failed to capture P4 command output");
        }
        Ok(status)
    }

    // Deliver post-cleanup status and failure before closing production without discarding events.
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
        if let Err(error) = self.queue.close(false) {
            eprintln!("Failed to finish event production: {error}");
        }
    }
}
