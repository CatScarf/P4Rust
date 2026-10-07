use crate::Output;
use crate::error::{Result, ResultExt, ensure};
use crate::ffi;
use std::ffi::c_void;

pub(crate) struct Capture {
    output: Output,
    errors: Vec<String>,
    failure: Option<crate::Error>,
}

impl Capture {
    // Initialize buffers owned entirely by the Rust caller.
    pub(crate) fn new() -> Self {
        Self {
            output: Output::default(),
            errors: Vec::new(),
            failure: None,
        }
    }

    // Collect a synchronous ABI event while preserving tagged record boundaries.
    fn receive(&mut self, event: u32, data: &[u8], value: &[u8]) -> Result<()> {
        match event {
            ffi::TEXT => self.output.raw.text.extend_from_slice(data),
            ffi::BINARY => self.output.binary.extend_from_slice(data),
            ffi::RECORD => self.output.raw.records.push(Vec::new()),
            ffi::FIELD => {
                let record = self
                    .output
                    .raw
                    .records
                    .last_mut()
                    .context("Failed to collect field: missing record")?;
                record.push((data.to_vec(), value.to_vec()));
            }
            ffi::WARNING => self.output.raw.warnings.push(data.to_vec()),
            ffi::ERROR => self.errors.push(String::from_utf8_lossy(data).into_owned()),
            _ => {
                return Err(crate::Error::new(format!(
                    "Failed to collect native event: unknown kind {event}"
                )));
            }
        }
        Ok(())
    }

    // Borrow bytes only during a callback, accepting NULL solely for empty spans.
    unsafe fn bytes<'a>(pointer: *const u8, length: usize) -> Result<&'a [u8]> {
        if length == 0 {
            return Ok(&[]);
        }
        ensure!(
            !pointer.is_null() && length <= isize::MAX as usize,
            "Failed to validate native byte span"
        );
        // ABI v1 guarantees these bytes remain valid until this callback returns.
        Ok(unsafe { std::slice::from_raw_parts(pointer, length) })
    }

    // Contain Rust panics and return callback failures through an explicit status.
    pub(crate) unsafe extern "C" fn callback(
        context: *mut c_void,
        event: u32,
        data: *const u8,
        length: usize,
        value: *const u8,
        value_length: usize,
    ) -> i32 {
        // Native execution is synchronous and never shares this context across threads.
        let capture = unsafe { &mut *context.cast::<Self>() };
        if capture.failure.is_some() {
            return 1;
        }
        let result = std::panic::catch_unwind(std::panic::AssertUnwindSafe(|| {
            // Both spans are borrowed from the validated native callback contract.
            let bytes = unsafe { Self::bytes(data, length) }
                .context("Failed to borrow native event data")?;
            let value = unsafe { Self::bytes(value, value_length) }
                .context("Failed to borrow native event value")?;
            capture
                .receive(event, bytes, value)
                .context("Failed to process native output event")
        }));
        match result {
            Ok(Ok(())) => 0,
            Ok(Err(error)) => {
                capture.failure = Some(error);
                1
            }
            Err(_) => {
                capture.failure = Some(crate::Error::new(
                    "Failed to capture native output: Rust callback panicked",
                ));
                1
            }
        }
    }

    // Return structured output only after checking both sides of the ABI boundary.
    pub(crate) fn finish(mut self, status: i32) -> Result<Output> {
        if let Some(error) = self.failure {
            return Err(error).context("Failed to capture P4 command output");
        }
        ensure!(
            status == 0 && self.errors.is_empty(),
            "Failed to execute P4 command: {}",
            self.errors.join("; ")
        );
        self.output.decode();
        Ok(self.output)
    }
}
