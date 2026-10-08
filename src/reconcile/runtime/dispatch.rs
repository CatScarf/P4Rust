use super::Runtime;
use crate::reconcile::{ReconcileKind, ReconcileRequest, task::Task};
use crate::{Record, Result, ResultExt, error::ensure};
use std::ffi::c_void;

impl Runtime {
    // Translate private scheduling operations without transferring connection ownership.
    pub(super) unsafe fn dispatch(
        &self,
        operation: u32,
        pointer: *mut c_void,
        data: *const u8,
        length: usize,
    ) -> Result<i32> {
        match operation {
            1 => unsafe { self.request(pointer, data, length) }
                .context("Failed to dispatch reconcile request")?,
            2 => self
                .poll(false)
                .context("Failed to pump reconcile replies")?,
            3 => self
                .poll(true)
                .context("Failed to wait for reconcile reply")?,
            4 => {
                while self
                    .pending()
                    .context("Failed to inspect reconcile barrier")?
                {
                    self.poll(true).context("Failed to drain reconcile stage")?;
                }
            }
            5 => {
                return Ok(i32::from(
                    self.pending()
                        .context("Failed to inspect reconcile queue")?,
                ));
            }
            6 => self.close().context("Failed to stop reconcile workers")?,
            7 => {
                return Ok(i32::from(
                    self.state
                        .lock()
                        .map_err(|_| {
                            crate::Error::new("Failed to inspect reconcile admission slots")
                        })?
                        .count
                        < self.capacity,
                ));
            }
            8 => {
                return unsafe { self.candidates(pointer, data, length) }
                    .context("Failed to dispatch unmatched candidates");
            }
            9 => unsafe { self.output(data, length) }
                .context("Failed to dispatch final reconcile record")?,
            10 => {
                return unsafe { self.snapshot(pointer, data, length) }
                    .context("Failed to dispatch shared metadata");
            }
            _ => {
                return Err(crate::Error::new(
                    "Failed to decode reconcile scheduling operation",
                ));
            }
        }
        Ok(0)
    }
    // Copy cached metadata into the SDK caller's synchronous snapshot buffer.
    unsafe fn snapshot(&self, pointer: *mut c_void, data: *const u8, length: usize) -> Result<i32> {
        ensure!(
            !pointer.is_null() && !data.is_null(),
            "Failed to read metadata request"
        );
        let pipeline = self
            .pipeline
            .lock()
            .map_err(|_| crate::Error::new("Failed to lock metadata pipeline"))?;
        let Some(pipeline) = pipeline.as_ref().filter(|pipeline| pipeline.enabled()) else {
            return Ok(0);
        };
        let Ok(path) = std::str::from_utf8(unsafe { std::slice::from_raw_parts(data, length) })
        else {
            pipeline.use_fallback();
            return Ok(0);
        };
        let snapshot = pipeline
            .snapshot(path)
            .context("Failed to load native snapshot")?;
        unsafe {
            pointer
                .cast::<crate::reconcile::pipeline::local::Snapshot>()
                .write(snapshot);
        }
        Ok(1)
    }

    // Adopt every submitted task before copying and decoding its frozen metadata.
    unsafe fn request(&self, pointer: *mut c_void, data: *const u8, length: usize) -> Result<()> {
        let task = unsafe { Task::adopt(pointer) }.context("Failed to own reconcile request")?;
        ensure!(
            !data.is_null() && length > 0,
            "Failed to borrow reconcile request descriptors"
        );
        let metadata = unsafe { Record::copy(std::slice::from_raw_parts(data, length)) }
            .context("Failed to copy reconcile request metadata")?;
        let kind = match metadata.get_raw(b"kind") {
            Some(b"tracked") => ReconcileKind::TrackedFile,
            Some(b"exact") => ReconcileKind::ExactMatch,
            Some(b"directory") => ReconcileKind::Directory,
            Some(b"untracked") => ReconcileKind::UntrackedFile,
            Some(b"move") => ReconcileKind::Move,
            _ => return Err(crate::Error::new("Failed to decode reconcile request kind")),
        };
        self.submit(ReconcileRequest {
            task,
            kind,
            metadata,
            executed: false,
        })
        .context("Failed to schedule local reconcile work")
    }

    // Wait for enumeration only at the SDK's untracked-file phase, allowing earlier RPC flushes.
    fn wait_scan(&self) -> Result<()> {
        loop {
            let scanning = self
                .pipeline
                .lock()
                .map_err(|_| crate::Error::new("Failed to inspect candidate scan completion"))?
                .as_ref()
                .is_some_and(crate::reconcile::pipeline::Pipeline::scanning);
            if !scanning {
                return Ok(());
            }
            self.poll(true)
                .context("Failed to wait for local candidate enumeration")?;
        }
    }

    // Copy single-sided paths and reusable canonical snapshots into the SDK-owned candidate sink.
    unsafe fn candidates(
        &self,
        pointer: *mut c_void,
        data: *const u8,
        length: usize,
    ) -> Result<i32> {
        self.wait_scan()
            .context("Failed to finish local enumeration")?;
        let paths = {
            let pipeline = self
                .pipeline
                .lock()
                .map_err(|_| crate::Error::new("Failed to read candidate pipeline"))?;
            let Some(pipeline) = pipeline.as_ref() else {
                return Ok(0);
            };
            if !pipeline.enabled() {
                return Ok(0);
            }
            ensure!(
                !data.is_null() && !pointer.is_null(),
                "Failed to read candidate directory or sink"
            );
            pipeline
                .paths(unsafe { std::slice::from_raw_parts(data, length) })
                .context("Failed to collect single-sided local candidates")?
        };
        for (path, snapshot) in paths {
            let path = std::ffi::CString::new(path).context("Failed to encode candidate path")?;
            ensure!(
                unsafe { crate::ffi::p4rust_reconcile_path_v5(pointer, path.as_ptr(), &snapshot) }
                    == 0,
                "Failed to copy local candidate snapshot"
            );
        }
        Ok(1)
    }

    // Retain complete SDK classifications in table C before their streaming delivery.
    unsafe fn output(&self, data: *const u8, length: usize) -> Result<()> {
        ensure!(!data.is_null(), "Failed to read final reconcile record");
        let record = unsafe { Record::copy(std::slice::from_raw_parts(data, length)) }
            .context("Failed to copy final reconcile record")?;
        if let Some(pipeline) = self
            .pipeline
            .lock()
            .map_err(|_| crate::Error::new("Failed to lock result table C"))?
            .as_ref()
        {
            pipeline
                .output(record)
                .context("Failed to finalize reconcile record")?;
        }
        Ok(())
    }
}
