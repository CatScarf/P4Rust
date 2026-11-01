use super::Capture;
use crate::{Record, Result, ResultExt, error::ensure, ffi};

impl Record {
    // Copy a borrowed SDK dictionary into one byte buffer and one field-offset vector.
    pub(crate) unsafe fn copy(frame: &[u8]) -> Result<Self> {
        let size = std::mem::size_of::<ffi::Field>();
        ensure!(
            frame.len().is_multiple_of(size),
            "Failed to validate native tagged descriptor count"
        );
        let pointer = frame.as_ptr().cast::<ffi::Field>();
        ensure!(
            frame.is_empty() || pointer.is_aligned(),
            "Failed to validate native tagged descriptor alignment"
        );
        // The native callback owns this aligned descriptor array and all referenced bytes.
        let fields = if frame.is_empty() {
            &[]
        } else {
            unsafe { std::slice::from_raw_parts(pointer, frame.len() / size) }
        };
        let mut length: usize = 0;
        for field in fields {
            length = length
                .checked_add(field.key_length)
                .and_then(|length| length.checked_add(field.value_length))
                .context("Failed to represent tagged record byte length")?;
        }
        let mut record = Self {
            bytes: Vec::with_capacity(length),
            spans: Vec::with_capacity(fields.len()),
        };
        for field in fields {
            // Every field span stays valid until the synchronous SDK callback returns.
            let key = unsafe { Capture::bytes(field.key, field.key_length) }
                .context("Failed to borrow tagged field name")?;
            let value = unsafe { Capture::bytes(field.value, field.value_length) }
                .context("Failed to borrow tagged field value")?;
            let start = record.bytes.len();
            record.bytes.extend_from_slice(key);
            let middle = record.bytes.len();
            record.bytes.extend_from_slice(value);
            record
                .spans
                .push((start..middle, middle..record.bytes.len()));
        }
        Ok(record)
    }
}
