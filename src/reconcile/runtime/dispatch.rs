use super::{Runtime, fetch::Message};
use crate::{Record, Result, ResultExt, error::ensure};
use crossbeam_channel as channel;
use std::{ffi::c_void, sync::atomic};

impl Runtime {
    // Translate SDK callbacks into owned Rust ingress or connection-only reply pumping.
    pub(super) unsafe fn dispatch(
        &self,
        operation: u32,
        pointer: *mut c_void,
        data: *const u8,
        length: usize,
    ) -> Result<i32> {
        ensure!(
            std::thread::current().id() == self.owner,
            "Failed to dispatch SDK helper outside its connection thread"
        );
        reconcile_span!(match operation {
            1 => "rpc_request",
            2 => "rpc_pump",
            3 => "rpc_wait",
            4 => "rpc_barrier",
            5 | 7 => "rpc_query",
            6 => "rpc_close",
            8 => "rpc_candidates",
            9 => "rpc_output",
            10 => "rpc_metadata",
            _ => "rpc_unknown",
        });
        match operation {
            1 => unsafe { self.request(pointer, data, length) }
                .context("Failed to queue SDK reconcile request")?,
            2 => self.poll(false).context("Failed to pump SDK replies")?,
            3 => self.poll(true).context("Failed to wait for SDK replies")?,
            4 => {
                while self.pending() {
                    self.poll(true).context("Failed to drain SDK stage")?;
                }
            }
            5 => return Ok(i32::from(self.pending())),
            6 => self
                .close()
                .context("Failed to close reconcile fetch owner")?,
            7 => {
                return Ok(i32::from(
                    self.shared.count.load(atomic::Ordering::Acquire) < self.capacity,
                ));
            }
            8 => {
                return unsafe { self.candidates(pointer, data, length) }
                    .context("Failed to request unmatched SDK candidates");
            }
            9 => unsafe { self.output(data, length) }
                .context("Failed to queue authoritative SDK output")?,
            10 => {
                return unsafe { self.snapshot(pointer, data, length) }
                    .context("Failed to request exceptional SDK metadata");
            }
            _ => {
                return Err(crate::Error::new(
                    "Failed to decode SDK scheduling operation",
                ));
            }
        }
        Ok(0)
    }

    // Adopt the task and enqueue stable descriptors before returning to the SDK.
    unsafe fn request(&self, pointer: *mut c_void, data: *const u8, length: usize) -> Result<()> {
        let request = unsafe { super::ingress::Frozen::adopt(pointer, data, length) }
            .context("Failed to freeze SDK ingress")?;
        self.submit(request)
            .context("Failed to admit frozen SDK task")
    }
    // Service exceptional metadata helpers without processing paths on the connection thread.
    unsafe fn snapshot(&self, pointer: *mut c_void, data: *const u8, length: usize) -> Result<i32> {
        ensure!(
            !pointer.is_null() && !data.is_null(),
            "Failed to read SDK metadata helper"
        );
        let (sender, receiver) = channel::bounded(1);
        let path = unsafe { std::slice::from_raw_parts(data, length) }.to_vec();
        self.send(Message::Snapshot(path, sender))
            .context("Failed to queue metadata helper")?;
        let Some(snapshot) = self
            .response(receiver)
            .context("Failed to collect metadata helper")?
        else {
            return Ok(0);
        };
        // The SDK owns this synchronous output slot until the callback returns.
        unsafe {
            pointer
                .cast::<crate::reconcile::pipeline::local::Snapshot>()
                .write(snapshot)
        };
        Ok(1)
    }

    // Copy fetch-owned candidates into the SDK sink only on its connection thread.
    unsafe fn candidates(
        &self,
        pointer: *mut c_void,
        data: *const u8,
        length: usize,
    ) -> Result<i32> {
        reconcile_span!("candidate_wait_scan");
        ensure!(
            !pointer.is_null() && !data.is_null(),
            "Failed to read SDK candidate request"
        );
        let (sender, receiver) = channel::bounded(1);
        let directory = unsafe { std::slice::from_raw_parts(data, length) }.to_vec();
        self.send(Message::Paths(directory, sender))
            .context("Failed to queue candidate helper")?;
        let Some(paths) = self
            .response(receiver)
            .context("Failed to collect candidate helper")?
        else {
            return Ok(0);
        };
        for (path, snapshot) in paths {
            let path = std::ffi::CString::new(path).context("Failed to encode candidate path")?;
            // Both the SDK sink and copied snapshot remain live for this synchronous call.
            ensure!(
                unsafe { crate::ffi::p4rust_reconcile_path_v5(pointer, path.as_ptr(), &snapshot) }
                    == 0,
                "Failed to copy SDK candidate snapshot"
            );
        }
        Ok(1)
    }

    // Transfer final SDK classifications to fetch-owned table C.
    unsafe fn output(&self, data: *const u8, length: usize) -> Result<()> {
        ensure!(!data.is_null(), "Failed to read SDK output descriptors");
        let record = unsafe { Record::copy(std::slice::from_raw_parts(data, length)) }
            .context("Failed to freeze final SDK output")?;
        self.send(Message::Output(record))
            .context("Failed to enqueue SDK output")
    }
}
