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
            output: Output {
                text: String::new(),
                binary: Vec::new(),
                records: Vec::new(),
                warnings: Vec::new(),
            },
            errors: Vec::new(),
            failure: None,
        }
    }

    // Decode UTF-8 explicitly rather than exposing unchecked native strings.
    fn text(bytes: &[u8]) -> Result<String> {
        Ok(std::str::from_utf8(bytes)
            .context("Failed to decode native UTF-8 output")?
            .to_owned())
    }

    // Collect a synchronous ABI event while preserving tagged record boundaries.
    fn receive(&mut self, event: u32, data: &[u8], value: &[u8]) -> Result<()> {
        match event {
            ffi::TEXT => self
                .output
                .text
                .push_str(&Self::text(data).context("Failed to collect text")?),
            ffi::BINARY => self.output.binary.extend_from_slice(data),
            ffi::RECORD => self.output.records.push(Vec::new()),
            ffi::FIELD => {
                let record = self
                    .output
                    .records
                    .last_mut()
                    .context("Failed to collect field: missing record")?;
                record.push((
                    Self::text(data).context("Failed to decode field key")?,
                    Self::text(value).context("Failed to decode field value")?,
                ));
            }
            ffi::WARNING => self
                .output
                .warnings
                .push(Self::text(data).context("Failed to collect warning")?),
            ffi::ERROR => self
                .errors
                .push(Self::text(data).context("Failed to collect server error")?),
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
    pub(crate) fn finish(self, status: i32) -> Result<Output> {
        if let Some(error) = self.failure {
            return Err(error).context("Failed to capture P4 command output");
        }
        ensure!(
            status == 0 && self.errors.is_empty(),
            "Failed to execute P4 command: {}",
            self.errors.join("; ")
        );
        Ok(self.output)
    }
}

#[cfg(test)]
mod tests {
    use super::Capture;
    use crate::ffi;
    use crate::{Result, ResultExt};

    // Preserve callback decoding errors without unwinding through native frames.
    #[test]
    fn rejects_invalid_callback_utf8() {
        let mut capture = Capture::new();
        let invalid = [0xff];
        // The context and byte span remain live through this synchronous callback.
        let status = unsafe {
            Capture::callback(
                (&mut capture as *mut Capture).cast(),
                ffi::TEXT,
                invalid.as_ptr(),
                invalid.len(),
                std::ptr::null(),
                0,
            )
        };
        assert_eq!(status, 1);
        let error = capture.finish(status).err();
        assert!(error.is_some_and(|error| format!("{error:#}").contains("UTF-8")));
    }

    // Reject fields without record boundaries and unsupported ABI event kinds.
    #[test]
    fn rejects_invalid_record_events() {
        let mut capture = Capture::new();
        assert!(capture.receive(ffi::FIELD, b"key", b"value").is_err());
        assert!(capture.receive(99, &[], &[]).is_err());
    }

    // Copy arbitrary binary output without UTF-8 decoding or native ownership.
    #[test]
    fn preserves_binary_bytes() -> Result<()> {
        let mut capture = Capture::new();
        capture
            .receive(ffi::BINARY, &[0, 255, 128], &[])
            .context("Failed to collect binary fixture")?;
        let output = capture
            .finish(0)
            .context("Failed to finish binary fixture")?;
        assert_eq!(output.binary, [0, 255, 128]);
        Ok(())
    }
}
