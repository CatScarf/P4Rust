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

#[cfg(test)]
mod tests {
    use super::Capture;
    use crate::ffi;
    use crate::{Result, ResultExt};

    // Preserve non-UTF-8 callback bytes without rejecting the command.
    #[test]
    fn preserves_invalid_callback_utf8() -> Result<()> {
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
        assert_eq!(status, 0);
        let output = capture
            .finish(status)
            .context("Failed to finish legacy text fixture")?;
        assert_eq!(output.text, "\u{fffd}");
        assert_eq!(output.raw.text, invalid);
        Ok(())
    }

    // Reassemble UTF-8 characters split at every possible byte boundary.
    #[test]
    fn decodes_split_multibyte_text() -> Result<()> {
        let bytes = "中文 🐯 café".as_bytes();
        for split in 0..=bytes.len() {
            let mut capture = Capture::new();
            capture
                .receive(ffi::TEXT, &bytes[..split], &[])
                .context("Failed to collect first text fragment")?;
            capture
                .receive(ffi::TEXT, &bytes[split..], &[])
                .context("Failed to collect second text fragment")?;
            let output = capture
                .finish(0)
                .context("Failed to decode fragmented UTF-8")?;
            assert_eq!(output.text, "中文 🐯 café");
            assert_eq!(output.raw.text, bytes);
        }
        Ok(())
    }

    // Preserve legacy field and warning bytes while producing readable display strings.
    #[test]
    fn preserves_legacy_records_and_warnings() -> Result<()> {
        let mut capture = Capture::new();
        capture
            .receive(ffi::RECORD, &[], &[])
            .context("Failed to begin legacy record")?;
        capture
            .receive(ffi::FIELD, &[0xff], &[0xd6, 0xd0])
            .context("Failed to capture legacy field")?;
        capture
            .receive(ffi::WARNING, &[0x80], &[])
            .context("Failed to capture legacy warning")?;
        let output = capture
            .finish(0)
            .context("Failed to finish legacy record")?;
        assert_eq!(
            output.raw.records,
            vec![vec![(vec![0xff], vec![0xd6, 0xd0])]]
        );
        assert_eq!(output.raw.warnings, vec![vec![0x80]]);
        assert_eq!(output.records[0][0].0, "\u{fffd}");
        assert_eq!(output.warnings[0], "\u{fffd}");
        Ok(())
    }

    // Report legacy server errors as command failures rather than decoding failures.
    #[test]
    fn reports_non_utf8_server_errors() -> Result<()> {
        let mut capture = Capture::new();
        capture
            .receive(ffi::ERROR, b"server: \xff", &[])
            .context("Failed to capture legacy server error")?;
        let error = capture
            .finish(1)
            .err()
            .context("Failed to reject server error")?;
        assert!(error.to_string().contains("server:"));
        assert!(!error.to_string().contains("decode"));
        Ok(())
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
