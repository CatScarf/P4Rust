mod dispatch;
use super::{ReconcileHandler, ReconcileKind, ReconcileRequest, pool};
use crate::{Result, ResultExt, control::Control, error::ensure};
use std::{collections::BTreeMap, ffi::c_void, sync, time};

#[derive(Default)]
struct State {
    next: u64,
    count: usize,
    bytes: usize,
    completed: BTreeMap<u64, pool::Completed>,
    failure: Option<crate::Error>,
    last_progress: Option<time::Instant>,
}

pub(crate) struct Runtime {
    pools: sync::Mutex<Option<[pool::Pool; 3]>>,
    receiver: sync::Mutex<sync::mpsc::Receiver<pool::Completed>>,
    state: sync::Mutex<State>,
    control: Control,
    capacity: usize,
    budget: usize,
    sequence: sync::atomic::AtomicU64,
    pipeline: sync::Mutex<Option<super::pipeline::Pipeline>>,
    handler: sync::Arc<dyn ReconcileHandler>,
}

impl Runtime {
    // Validate limits and initialize all categories before the native command starts.
    pub(crate) fn new(
        handler: sync::Arc<dyn ReconcileHandler>,
        control: Control,
        config: &crate::Config,
        args: &[&str],
    ) -> Result<Self> {
        let kinds = [
            ReconcileKind::Directory,
            ReconcileKind::TrackedFile,
            ReconcileKind::Move,
        ];
        for kind in kinds {
            ensure!(
                (1..=32).contains(&handler.workers(kind)),
                "Failed to configure reconcile workers: expected 1..=32"
            );
        }
        ensure!(
            handler.workers(ReconcileKind::UntrackedFile)
                == handler.workers(ReconcileKind::TrackedFile),
            "Failed to configure reconcile workers: tracked and untracked digest limits must match"
        );
        ensure!(
            handler.workers(ReconcileKind::ExactMatch)
                == handler.workers(ReconcileKind::TrackedFile),
            "Failed to configure reconcile workers: exact and tracked digest limits must match"
        );
        ensure!(
            (1..=65536).contains(&handler.queue_capacity()),
            "Failed to configure reconcile queue capacity"
        );
        ensure!(
            (1048576..=268435456).contains(&handler.queue_bytes()),
            "Failed to configure reconcile byte budget"
        );
        let (sender, receiver) = sync::mpsc::channel();
        let metadata = pool::Pool::new(sync::Arc::clone(&handler), kinds[0], sender.clone())
            .context("Failed to initialize metadata workers")?;
        let digest = pool::Pool::new(sync::Arc::clone(&handler), kinds[1], sender.clone())
            .context("Failed to initialize digest workers")?;
        let moves = pool::Pool::new(sync::Arc::clone(&handler), kinds[2], sender)
            .context("Failed to initialize move workers")?;
        let pipeline = if handler.pipeline() {
            super::pipeline::Pipeline::start(
                config,
                args,
                handler.workers(kinds[0]),
                control.clone(),
            )
            .context("Failed to start reconcile path pipeline")?
        } else {
            None
        };
        Ok(Self {
            pools: sync::Mutex::new(Some([metadata, digest, moves])),
            receiver: sync::Mutex::new(receiver),
            state: sync::Mutex::new(State::default()),
            control,
            capacity: handler.queue_capacity(),
            budget: handler.queue_bytes(),
            sequence: sync::atomic::AtomicU64::new(0),
            pipeline: sync::Mutex::new(pipeline),
            handler,
        })
    }

    // Reserve retained work without holding any Rust lock across native reply dispatch.
    fn submit(&self, request: ReconcileRequest) -> Result<()> {
        let bytes = request.metadata.bytes.len();
        ensure!(
            bytes <= self.budget,
            "Failed to enqueue reconcile request: exceeds byte budget"
        );
        loop {
            self.check().context("Failed to submit reconcile request")?;
            let mut state = self
                .state
                .lock()
                .map_err(|_| crate::Error::new("Failed to lock reconcile admission"))?;
            if state.count < self.capacity && state.bytes <= self.budget - bytes {
                state.count += 1;
                state.bytes += bytes;
                break;
            }
            drop(state);
            self.poll(true)
                .context("Failed to wait for reconcile queue capacity")?;
        }
        let id = self.sequence.fetch_add(1, sync::atomic::Ordering::Relaxed);
        let work = pool::Work {
            id,
            request,
            snapshot: None,
        };
        if work.request.kind == ReconcileKind::TrackedFile {
            let pipeline = self
                .pipeline
                .lock()
                .map_err(|_| crate::Error::new("Failed to lock reconcile path pipeline"))?;
            if let Some(pipeline) = pipeline.as_ref() {
                return pipeline
                    .server(work)
                    .context("Failed to submit server path slot");
            }
        }
        self.enqueue(work)
            .context("Failed to enqueue ready reconcile work")
    }

    // Dispatch only complete pairs while preserving connection-thread ownership of replies.
    fn enqueue(&self, work: pool::Work) -> Result<()> {
        let index = match work.request.kind {
            ReconcileKind::Directory => 0,
            ReconcileKind::Move => 2,
            _ => 1,
        };
        self.pools
            .lock()
            .map_err(|_| crate::Error::new("Failed to lock reconcile pools"))?
            .as_ref()
            .context("Failed to submit to stopped reconcile runtime")?[index]
            .submit(work)
            .context("Failed to dispatch reconcile request")
    }

    // Receive completions and commit them in request order without retaining a lock.
    fn poll(&self, wait: bool) -> Result<()> {
        self.check()
            .context("Failed to poll reconcile completion")?;
        let ready = self
            .pipeline
            .lock()
            .map_err(|_| crate::Error::new("Failed to drain paired reconcile paths"))?
            .as_ref()
            .map(super::pipeline::Pipeline::ready)
            .unwrap_or_default();
        for work in ready {
            self.enqueue(work)
                .context("Failed to dispatch paired comparison")?;
        }
        self.progress(false)
            .context("Failed to report reconcile progress")?;
        let result = {
            let receiver = self
                .receiver
                .lock()
                .map_err(|_| crate::Error::new("Failed to lock reconcile completions"))?;
            if wait {
                match receiver.recv_timeout(time::Duration::from_millis(25)) {
                    Ok(result) => Some(result),
                    Err(sync::mpsc::RecvTimeoutError::Timeout) => None,
                    Err(sync::mpsc::RecvTimeoutError::Disconnected) => {
                        return Err(crate::Error::new(
                            "Failed to receive reconcile completion: workers disconnected",
                        ));
                    }
                }
            } else {
                match receiver.try_recv() {
                    Ok(result) => Some(result),
                    Err(sync::mpsc::TryRecvError::Empty) => None,
                    Err(sync::mpsc::TryRecvError::Disconnected) => None,
                }
            }
        };
        if let Some(result) = result {
            self.state
                .lock()
                .map_err(|_| crate::Error::new("Failed to retain reconcile completion"))?
                .completed
                .insert(result.id, result);
        }
        loop {
            let completed = {
                let mut state = self
                    .state
                    .lock()
                    .map_err(|_| crate::Error::new("Failed to order reconcile completion"))?;
                let next = state.next;
                let completed = state.completed.remove(&next);
                if let Some(completed) = &completed {
                    state.next += 1;
                    state.count -= 1;
                    state.bytes -= completed.bytes;
                }
                completed
            };
            let Some(completed) = completed else { break };
            let (request, reply) = completed
                .reply
                .context("Failed to complete reconcile worker")?;
            if let Some(pipeline) = self
                .pipeline
                .lock()
                .map_err(|_| crate::Error::new("Failed to record reconcile comparison"))?
                .as_ref()
            {
                pipeline
                    .completed(&request, &reply, completed.duration)
                    .context("Failed to record canonical comparison")?;
            }
            reply
                .commit(request)
                .context("Failed to commit reconcile reply")?;
        }
        Ok(())
    }

    // Stop admitting work immediately when cancellation or a callback failure occurs.
    fn check(&self) -> Result<()> {
        if let Some(pipeline) = self
            .pipeline
            .lock()
            .map_err(|_| crate::Error::new("Failed to check path pipeline"))?
            .as_ref()
        {
            pipeline
                .check()
                .context("Failed to inspect local enumeration failure")?;
        }
        if let Some(error) = self.control.error() {
            return Err(error);
        }
        ensure!(
            self.state
                .lock()
                .map_err(|_| crate::Error::new("Failed to inspect reconcile failure"))?
                .failure
                .is_none(),
            "Failed to continue reconcile after callback failure"
        );
        Ok(())
    }

    // Inspect retained work independently of worker completion delivery.
    fn pending(&self) -> Result<bool> {
        Ok(self
            .state
            .lock()
            .map_err(|_| crate::Error::new("Failed to inspect pending reconcile work"))?
            .count
            != 0)
    }

    // Join workers before dropping their native tasks or callback contexts.
    pub(crate) fn close(&self) -> Result<()> {
        let pipeline = self
            .pipeline
            .lock()
            .map_err(|_| crate::Error::new("Failed to stop local pipeline"))?
            .take();
        let mut pipeline_failure = None;
        if let Some(mut pipeline) = pipeline {
            if let Err(error) = pipeline.close() {
                pipeline_failure = Some(error);
            }
            let report = if pipeline.enabled() {
                pipeline
                    .statistics()
                    .context("Failed to read final pipeline counters")
                    .and_then(|statistics| {
                        self.handler
                            .progress(&statistics)
                            .context("Failed to deliver final pipeline counters")
                    })
            } else {
                Ok(())
            };
            if let Err(error) = report {
                pipeline_failure.get_or_insert(error);
            }
        }
        let pools = self
            .pools
            .lock()
            .map_err(|_| crate::Error::new("Failed to close reconcile pools"))?
            .take();
        let mut failure = pipeline_failure;
        if let Some(mut pools) = pools {
            for pool in &mut pools {
                if let Err(error) = pool.close() {
                    failure = Some(error);
                }
            }
        }
        self.receiver
            .lock()
            .map_err(|_| crate::Error::new("Failed to release queued reconcile completions"))?
            .try_iter()
            .for_each(drop);
        let mut state = self
            .state
            .lock()
            .map_err(|_| crate::Error::new("Failed to release ordered reconcile completions"))?;
        state.completed.clear();
        state.count = 0;
        state.bytes = 0;
        match failure {
            Some(error) => Err(error),
            None => Ok(()),
        }
    }

    // Report snapshots periodically without invoking user callbacks under scheduler locks.
    fn progress(&self, force: bool) -> Result<()> {
        {
            let mut state = self
                .state
                .lock()
                .map_err(|_| crate::Error::new("Failed to inspect pipeline progress interval"))?;
            if !force
                && state
                    .last_progress
                    .is_some_and(|last| last.elapsed() < time::Duration::from_secs(15))
            {
                return Ok(());
            }
            state.last_progress = Some(time::Instant::now());
        }
        let statistics = self
            .pipeline
            .lock()
            .map_err(|_| crate::Error::new("Failed to snapshot path pipeline"))?
            .as_ref()
            .filter(|pipeline| pipeline.enabled())
            .map(super::pipeline::Pipeline::statistics)
            .transpose()
            .context("Failed to read pipeline counters")?;
        if let Some(statistics) = statistics {
            self.handler
                .progress(&statistics)
                .context("Failed to report pipeline counters")?;
        }
        Ok(())
    }

    // Recover the original Rust failure after native cleanup has stopped all workers.
    pub(crate) fn finish(&self) -> Result<()> {
        self.close().context("Failed to join reconcile pools")?;
        let failure = self
            .state
            .lock()
            .map_err(|_| crate::Error::new("Failed to read reconcile callback failure"))?
            .failure
            .take();
        match failure {
            Some(error) => Err(error),
            None => Ok(()),
        }
    }

    // Contain panics at the C boundary and record the first contextual failure.
    pub(crate) unsafe extern "C" fn callback(
        context: *mut c_void,
        operation: u32,
        task: *mut c_void,
        data: *const u8,
        length: usize,
    ) -> i32 {
        // Native execution keeps this shared runtime alive through worker cleanup.
        let runtime = unsafe { &*context.cast::<Self>() };
        let result = match std::panic::catch_unwind(std::panic::AssertUnwindSafe(|| {
            // The operation owns a task only for submission and borrows its descriptor array.
            unsafe { runtime.dispatch(operation, task, data, length) }
        })) {
            Ok(result) => result,
            Err(_) => Err(crate::Error::new(
                "Failed to dispatch reconcile callback: panicked",
            )),
        };
        match result {
            Ok(value) => value,
            Err(error) => {
                runtime.control.cancel();
                match runtime.state.lock() {
                    Ok(mut state) => {
                        if state.failure.is_none() {
                            state.failure = Some(error);
                        }
                    }
                    Err(_) => eprintln!("Failed to retain reconcile callback error: {error}"),
                }
                -1
            }
        }
    }
}
