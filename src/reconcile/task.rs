use crate::{Record, Result, ResultExt, error::ensure, ffi};
use std::{ffi::c_void, ptr::NonNull};

/// The local operation requested by the SDK during reconciliation.
#[derive(Clone, Copy, Debug, Eq, PartialEq)]
pub enum ReconcileKind {
    TrackedFile,
    ExactMatch,
    Directory,
    UntrackedFile,
    Move,
}

/// A tracked-file response in the original reconcile protocol.
#[derive(Clone, Copy, Debug, Eq, PartialEq)]
pub enum ReconcileStatus {
    Same,
    Missing,
    Exists,
}

pub(crate) struct Task(NonNull<c_void>);
// Each task owns its SDK file objects; only its final reply accesses the connection.
unsafe impl Send for Task {}

impl Task {
    // Adopt one native task transferred by a synchronous scheduling callback.
    pub(crate) unsafe fn adopt(pointer: *mut c_void) -> Result<Self> {
        NonNull::new(pointer)
            .map(Self)
            .context("Failed to adopt null reconcile task")
    }
    // Expose the private handle only while its owned task remains alive.
    pub(crate) fn pointer(&self) -> *mut c_void {
        self.0.as_ptr()
    }
    // Read native failure text while the task owns its diagnostic buffer.
    pub(crate) fn error(&self) -> crate::Error {
        let mut length = 0;
        // The native task owns this byte span until it is dropped.
        let pointer = unsafe { ffi::p4rust_reconcile_error_v3(self.pointer(), &mut length) };
        let bytes = if length == 0 {
            &[]
        } else {
            // The ABI returns a valid diagnostic span for a live task.
            unsafe { std::slice::from_raw_parts(pointer, length) }
        };
        crate::Error::new(format!(
            "Failed to process reconcile task: {}",
            String::from_utf8_lossy(bytes)
        ))
    }
}

impl Drop for Task {
    // Release isolated native file objects after execution or command cancellation.
    fn drop(&mut self) {
        // Ownership is unique and no SDK worker borrows this task after completion.
        unsafe { ffi::p4rust_reconcile_drop_v3(self.pointer()) };
    }
}

/// An owned local request with byte-preserving RPC metadata and SDK comparison support.
pub struct ReconcileRequest {
    pub(crate) task: Task,
    pub kind: ReconcileKind,
    pub metadata: Record,
    pub(crate) executed: bool,
}

impl ReconcileRequest {
    /// Borrow the native local path without assuming that it is UTF-8.
    pub fn path_bytes(&self) -> Option<&[u8]> {
        self.metadata.get_raw(b"localPath")
    }
    /// Execute this request with independent SDK objects on the calling worker.
    pub fn execute(&mut self) -> Result<ReconcileReply> {
        ensure!(
            !self.executed,
            "Failed to execute reconcile request: already executed"
        );
        self.executed = true;
        // This worker owns the task and the SDK initializes thread-local state for the call.
        let status = unsafe { ffi::p4rust_reconcile_execute_v3(self.task.pointer()) };
        ensure!(status == 0, "{}", self.task.error());
        let mut result = None;
        // The callback borrows a bounded dictionary owned by this live task.
        let status = unsafe {
            ffi::p4rust_reconcile_result_v3(
                self.task.pointer(),
                Self::result,
                std::ptr::from_mut(&mut result).cast::<c_void>(),
            )
        };
        ensure!(status == 0, "Failed to read reconcile result");
        let result = result
            .context("Failed to receive reconcile result")?
            .context("Failed to decode reconcile result")?;
        Ok(ReconcileReply {
            identity: self.task.pointer() as usize,
            kind: self.kind,
            result,
            status: None,
        })
    }
    // Copy a native result before its descriptor and byte spans expire.
    unsafe extern "C" fn result(
        context: *mut c_void,
        _: u32,
        data: *const u8,
        length: usize,
        _: *const u8,
        _: usize,
    ) -> i32 {
        let copied = std::panic::catch_unwind(|| {
            // The native callback owns this descriptor span for the duration of this call.
            let frame = unsafe { std::slice::from_raw_parts(data, length) };
            unsafe { Record::copy(frame) }.context("Failed to copy reconcile result fields")
        });
        // Execute supplied this exclusive result slot until the callback returns.
        let slot = unsafe { &mut *context.cast::<Option<Result<Record>>>() };
        *slot = Some(match copied {
            Ok(result) => result,
            Err(_) => Err(crate::Error::new(
                "Failed to copy reconcile result: callback panicked",
            )),
        });
        0
    }
}

/// A completed local request whose reply is sent by its connection's owning thread.
pub struct ReconcileReply {
    pub(crate) identity: usize,
    kind: ReconcileKind,
    pub result: Record,
    status: Option<ReconcileStatus>,
}

impl ReconcileReply {
    /// Override a tracked-file decision while retaining the SDK confirmation context.
    pub fn with_status(mut self, status: ReconcileStatus) -> Result<Self> {
        ensure!(
            self.kind == ReconcileKind::TrackedFile,
            "Failed to override reconcile status: request is not a tracked-file comparison"
        );
        self.status = Some(status);
        Ok(self)
    }
    // Commit only on the SDK connection thread after the worker has returned ownership.
    pub(crate) fn commit(self, request: ReconcileRequest) -> Result<()> {
        let status = match self.status {
            None => -1,
            Some(ReconcileStatus::Same) => 0,
            Some(ReconcileStatus::Missing) => 1,
            Some(ReconcileStatus::Exists) => 2,
        };
        // The runtime calls commit exclusively on the native connection thread.
        let result = unsafe { ffi::p4rust_reconcile_commit_v3(request.task.pointer(), status) };
        ensure!(result == 0, "{}", request.task.error());
        Ok(())
    }
}
