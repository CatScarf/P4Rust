use crate::reconcile::{ReconcileKind, ReconcileRequest, pool::Work, task::Task};
use crate::{Record, Result, ResultExt, error::ensure, ffi};
use std::ffi::c_void;

pub(in crate::reconcile) struct Frozen {
    task: Task,
    fields: Vec<ffi::Field>,
    pub bytes: usize,
    pub id: u64,
}

// Task owns every referenced byte; no native execution starts before fetch decoding.
unsafe impl Send for Frozen {}

impl Frozen {
    // Own the native task and copy descriptors while leaving its frozen bytes in place.
    pub(super) unsafe fn adopt(
        pointer: *mut c_void,
        data: *const u8,
        length: usize,
    ) -> Result<Self> {
        let task = unsafe { Task::adopt(pointer) }.context("Failed to own frozen SDK task")?;
        let size = std::mem::size_of::<ffi::Field>();
        ensure!(
            !data.is_null()
                && length > 0
                && length.is_multiple_of(size)
                && length / size <= 16384
                && data.cast::<ffi::Field>().is_aligned(),
            "Failed to validate frozen SDK descriptors"
        );
        // Scope::Submit creates these descriptors from this task's owned request strings.
        let borrowed =
            unsafe { std::slice::from_raw_parts(data.cast::<ffi::Field>(), length / size) };
        let mut fields = Vec::with_capacity(borrowed.len());
        let mut bytes = 0;
        for field in borrowed {
            ensure!(
                field.key_length <= 1048576 - bytes
                    && field.value_length <= 1048576 - bytes - field.key_length,
                "Failed to bound frozen SDK bytes: exceeds 1 MiB"
            );
            ensure!(
                (field.key_length == 0 || !field.key.is_null())
                    && (field.value_length == 0 || !field.value.is_null()),
                "Failed to borrow frozen SDK field bytes"
            );
            bytes += field.key_length + field.value_length;
            fields.push(ffi::Field {
                key: field.key,
                key_length: field.key_length,
                value: field.value,
                value_length: field.value_length,
            });
        }
        Ok(Self {
            task,
            fields,
            bytes,
            id: 0,
        })
    }

    // Decode native-owned fields on fetch before releasing their copied descriptors.
    pub(super) fn decode(self) -> Result<Work> {
        reconcile_span!("fetch_record_decode");
        // The aligned descriptor allocation and its unexecuted task retain all fields.
        let frame = unsafe {
            std::slice::from_raw_parts(
                self.fields.as_ptr().cast::<u8>(),
                self.fields.len() * std::mem::size_of::<ffi::Field>(),
            )
        };
        let metadata =
            unsafe { Record::copy(frame) }.context("Failed to copy fetched SDK fields")?;
        let kind = match metadata.get_raw(b"kind") {
            Some(b"tracked") => ReconcileKind::TrackedFile,
            Some(b"exact") => ReconcileKind::ExactMatch,
            Some(b"directory") => ReconcileKind::Directory,
            Some(b"untracked") => ReconcileKind::UntrackedFile,
            Some(b"move") => ReconcileKind::Move,
            _ => {
                return Err(crate::Error::new(
                    "Failed to decode fetched SDK request kind",
                ));
            }
        };
        Ok(Work {
            id: self.id,
            request: ReconcileRequest {
                task: self.task,
                kind,
                metadata,
                executed: false,
            },
            snapshot: None,
        })
    }
}
