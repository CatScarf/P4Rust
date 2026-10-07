use super::{ReconcileHandler, ReconcileKind, ReconcileRequest, pool, task::Task};
use crate::{Record, Result, ResultExt, control::Control, error::ensure};
use std::{collections::BTreeMap, ffi::c_void, sync, time};

#[derive(Default)]
struct State {
    next: u64,
    count: usize,
    bytes: usize,
    completed: BTreeMap<u64, pool::Completed>,
    failure: Option<crate::Error>,
}

pub(crate) struct Runtime {
    pools: sync::Mutex<Option<[pool::Pool; 3]>>,
    receiver: sync::Mutex<sync::mpsc::Receiver<pool::Completed>>,
    state: sync::Mutex<State>,
    control: Control,
    capacity: usize,
    budget: usize,
    sequence: sync::atomic::AtomicU64,
}

impl Runtime {
    // Validate limits and initialize all categories before the native command starts.
    pub(crate) fn new(handler: sync::Arc<dyn ReconcileHandler>, control: Control) -> Result<Self> {
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
        Ok(Self {
            pools: sync::Mutex::new(Some([metadata, digest, moves])),
            receiver: sync::Mutex::new(receiver),
            state: sync::Mutex::new(State::default()),
            control,
            capacity: handler.queue_capacity(),
            budget: handler.queue_bytes(),
            sequence: sync::atomic::AtomicU64::new(0),
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
        let index = match request.kind {
            ReconcileKind::Directory => 0,
            ReconcileKind::Move => 2,
            _ => 1,
        };
        self.pools
            .lock()
            .map_err(|_| crate::Error::new("Failed to lock reconcile pools"))?
            .as_ref()
            .context("Failed to submit to stopped reconcile runtime")?[index]
            .submit(pool::Work { id, request })
            .context("Failed to dispatch reconcile request")
    }

    // Receive completions and commit them in request order without retaining a lock.
    fn poll(&self, wait: bool) -> Result<()> {
        self.check()
            .context("Failed to poll reconcile completion")?;
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
            reply
                .commit(request)
                .context("Failed to commit reconcile reply")?;
        }
        Ok(())
    }

    // Stop admitting work immediately when cancellation or a callback failure occurs.
    fn check(&self) -> Result<()> {
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
        let pools = self
            .pools
            .lock()
            .map_err(|_| crate::Error::new("Failed to close reconcile pools"))?
            .take();
        let mut failure = None;
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

    // Translate private scheduling operations into ownership-safe Rust requests and replies.
    unsafe fn dispatch(
        &self,
        operation: u32,
        pointer: *mut c_void,
        data: *const u8,
        length: usize,
    ) -> Result<i32> {
        match operation {
            1 => {
                // Submission always transfers the task, including failure paths.
                let task =
                    unsafe { Task::adopt(pointer) }.context("Failed to own reconcile request")?;
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
                .context("Failed to schedule local reconcile work")?;
            }
            2 => self
                .poll(false)
                .context("Failed to pump completed reconcile replies")?,
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
            _ => {
                return Err(crate::Error::new(
                    "Failed to decode reconcile scheduling operation",
                ));
            }
        }
        Ok(0)
    }
}
