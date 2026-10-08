mod local;
mod output;

use crate::{Result, ResultExt};
use local::Local;
use std::{cell::RefCell, sync, time};

thread_local! {
    static CURRENT: RefCell<Option<sync::Arc<sync::Mutex<Local>>>> = const { RefCell::new(None) };
}

/// Optional per-command timeline with exclusive activity bins and bounded slow spans.
#[derive(Clone)]
pub struct ReconcileTrace {
    inner: sync::Arc<Inner>,
}

impl std::fmt::Debug for ReconcileTrace {
    // Describe capture configuration without locking or copying recorded timing buffers.
    fn fmt(&self, formatter: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        formatter
            .debug_struct("ReconcileTrace")
            .field("resolution_ns", &self.inner.resolution)
            .finish_non_exhaustive()
    }
}

struct Inner {
    started: time::Instant,
    resolution: u64,
    threads: sync::Mutex<Vec<sync::Arc<sync::Mutex<Local>>>>,
    failed: sync::atomic::AtomicBool,
}

pub(crate) struct Attachment {
    previous: Option<sync::Arc<sync::Mutex<Local>>>,
    _trace: Option<ReconcileTrace>,
}

pub(crate) struct Span {
    local: Option<sync::Arc<sync::Mutex<Local>>>,
}

impl ReconcileTrace {
    /// Create an isolated capture with a 10 ms to 10 s activity resolution.
    pub fn new(resolution: time::Duration) -> Result<Self> {
        let resolution = u64::try_from(resolution.as_nanos())
            .context("Failed to encode reconcile trace resolution")?;
        crate::error::ensure!(
            (10_000_000..=10_000_000_000).contains(&resolution),
            "Failed to configure reconcile trace: expected resolution between 10 ms and 10 s"
        );
        Ok(Self {
            inner: sync::Arc::new(Inner {
                started: time::Instant::now(),
                resolution,
                threads: sync::Mutex::new(Vec::new()),
                failed: sync::atomic::AtomicBool::new(false),
            }),
        })
    }

    // Register one worker buffer without a shared lock on subsequent timing operations.
    pub(crate) fn attach(trace: Option<Self>) -> Result<Attachment> {
        let local = trace
            .as_ref()
            .map(|trace| -> Result<_> {
                let name = std::thread::current()
                    .name()
                    .unwrap_or("unnamed")
                    .to_owned();
                let local = sync::Arc::new(sync::Mutex::new(Local::new(name, trace.inner.clone())));
                trace
                    .inner
                    .threads
                    .lock()
                    .map_err(|_| {
                        crate::Error::new(
                            "Failed to register reconcile trace worker: poisoned mutex",
                        )
                    })?
                    .push(local.clone());
                Ok(local)
            })
            .transpose()
            .context("Failed to attach reconcile trace")?;
        let previous = CURRENT.with(|current| current.replace(local));
        Ok(Attachment {
            previous,
            _trace: trace,
        })
    }

    // Obtain the active command capture before spawning a child worker.
    pub(crate) fn current() -> Option<Self> {
        CURRENT.with(|current| {
            current
                .borrow()
                .as_ref()
                .and_then(|local| match local.lock() {
                    Ok(local) => local.inner.upgrade().map(|inner| Self { inner }),
                    Err(error) => {
                        error.into_inner().fail();
                        None
                    }
                })
        })
    }

    // Begin one nested interval and attribute parent time exclusively around child spans.
    pub(crate) fn span(stage: &'static str) -> Span {
        let local = CURRENT.with(|current| current.borrow().clone());
        if let Some(local) = &local {
            match local.lock() {
                Ok(mut buffer) => buffer.begin(stage),
                Err(error) => error.into_inner().fail(),
            }
        }
        Span { local }
    }

    /// Write a completed capture with nanosecond bounds, exclusive bins, and span totals.
    pub fn write_json(&self, path: impl AsRef<std::path::Path>) -> Result<()> {
        output::Output::write(self, path.as_ref()).context("Failed to export reconcile timeline")
    }
}

impl Drop for Attachment {
    // Restore any outer command capture when this worker finishes.
    fn drop(&mut self) {
        CURRENT.with(|current| current.replace(self.previous.take()));
    }
}

impl Span {
    // Refine a completed SDK branch before closing its timed interval.
    pub(crate) fn classify(&self, stage: &'static str) {
        if let Some(local) = &self.local {
            match local.lock() {
                Ok(mut buffer) => buffer.classify(stage),
                Err(error) => error.into_inner().fail(),
            }
        }
    }
}

impl Drop for Span {
    // Close the interval and preserve diagnostic failure for the fallible exporter.
    fn drop(&mut self) {
        if let Some(local) = &self.local {
            match local.lock() {
                Ok(mut buffer) => buffer.end(),
                Err(error) => error.into_inner().fail(),
            }
        }
    }
}
