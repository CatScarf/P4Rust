mod dispatch;
pub(super) mod fetch;
pub(super) mod ingress;
use super::{ReconcileHandler, pool};
use crate::{Result, ResultExt, control::Control, error::ensure};
use crossbeam_channel as channel;
use std::{cell::RefCell, collections::VecDeque};
use std::{ffi::c_void, sync, sync::atomic, thread, time};

struct Shared {
    control: Control,
    count: atomic::AtomicUsize,
    bytes: atomic::AtomicUsize,
    sequence: atomic::AtomicU64,
    failed: atomic::AtomicBool,
    failure: sync::Mutex<Option<crate::Error>>,
}

pub(crate) struct Runtime {
    ingress: channel::Sender<fetch::Message>,
    replies: channel::Receiver<Vec<pool::Completed>>,
    shared: sync::Arc<Shared>,
    worker: sync::Mutex<Option<thread::JoinHandle<()>>>,
    capacity: usize,
    budget: usize,
    commits: RefCell<VecDeque<pool::Completed>>,
    owner: thread::ThreadId,
}

impl Shared {
    // Publish a contextual failure once without locking successful callbacks.
    fn fail(&self, error: crate::Error) {
        self.failed.store(true, atomic::Ordering::Release);
        match self.failure.lock() {
            Ok(mut failure) => {
                failure.get_or_insert(error);
            }
            Err(_) => eprintln!("Failed to retain reconcile fetch failure: {error}"),
        }
        self.control.cancel();
    }

    // Inspect command cancellation and cold-path failure flags.
    fn check(&self) -> Result<()> {
        ensure!(
            !self.failed.load(atomic::Ordering::Acquire),
            "Failed to continue reconcile after fetch failure"
        );
        if let Some(error) = self.control.error() {
            return Err(error).context("Failed to continue reconcile fetch");
        }
        Ok(())
    }
}

impl Runtime {
    // Spawn a single Rust fetch owner before the SDK opens its server connection.
    pub(crate) fn new(
        handler: sync::Arc<dyn ReconcileHandler>,
        control: Control,
        config: &crate::Config,
        args: &[&str],
    ) -> Result<Self> {
        fetch::Fetch::validate(handler.as_ref())
            .context("Failed to validate reconcile fetch configuration")?;
        let capacity = handler.queue_capacity();
        let budget = handler.queue_bytes();
        let shared = sync::Arc::new(Shared {
            control,
            count: atomic::AtomicUsize::new(0),
            bytes: atomic::AtomicUsize::new(0),
            sequence: atomic::AtomicU64::new(0),
            failed: atomic::AtomicBool::new(false),
            failure: sync::Mutex::new(None),
        });
        let (ingress, receiver) = channel::bounded(capacity);
        let (sender, replies) = channel::unbounded();
        let mut fetch = fetch::Fetch::new(
            handler,
            sync::Arc::clone(&shared),
            config,
            args,
            ingress.clone(),
            receiver,
            sender,
        )
        .context("Failed to initialize reconcile fetch owner")?;
        #[cfg(feature = "reconcile-trace")]
        let trace = super::trace::ReconcileTrace::current();
        let state = sync::Arc::clone(&shared);
        let worker = thread::Builder::new()
            .name("p4rust-reconcile-fetch".into())
            .spawn(move || {
                let result = std::panic::catch_unwind(std::panic::AssertUnwindSafe(|| {
                    #[cfg(feature = "reconcile-trace")]
                    let _attachment = super::trace::ReconcileTrace::attach(trace)
                        .context("Failed to attach fetch trace")?;
                    reconcile_span!("fetch_lifetime");
                    if let Err(error) = fetch
                        .run()
                        .context("Failed to process reconcile fetch pipeline")
                    {
                        state.fail(error);
                    }
                    fetch
                        .close()
                        .context("Failed to clean up reconcile fetch pipeline")
                }))
                .unwrap_or_else(|_| {
                    Err(crate::Error::new(
                        "Failed to process reconcile fetch: thread panicked",
                    ))
                });
                if let Err(error) = result {
                    state.fail(error);
                }
                if let Err(error) = fetch.close() {
                    state.fail(error);
                }
            })
            .context("Failed to spawn reconcile fetch thread")?;
        Ok(Self {
            ingress,
            replies,
            shared,
            worker: sync::Mutex::new(Some(worker)),
            capacity,
            budget,
            commits: RefCell::new(VecDeque::new()),
            owner: thread::current().id(),
        })
    }
    // Reserve total retained task and byte credits before transferring ownership.
    fn submit(&self, mut request: ingress::Frozen) -> Result<()> {
        reconcile_span!("request_admission");
        let bytes = request.bytes;
        ensure!(
            bytes <= self.budget,
            "Failed to enqueue reconcile request: exceeds byte budget"
        );
        while self.shared.count.load(atomic::Ordering::Acquire) >= self.capacity
            || self.shared.bytes.load(atomic::Ordering::Acquire) > self.budget - bytes
        {
            reconcile_span!("admission_backpressure");
            self.poll(true)
                .context("Failed to wait for reconcile admission credits")?;
        }
        self.shared
            .check()
            .context("Failed to admit reconcile request")?;
        self.shared.count.fetch_add(1, atomic::Ordering::Release);
        self.shared
            .bytes
            .fetch_add(bytes, atomic::Ordering::Release);
        request.id = self.shared.sequence.fetch_add(1, atomic::Ordering::Relaxed);
        self.send(fetch::Message::Request(Box::new(request)))
            .context("Failed to queue frozen SDK request")
    }

    // Pump replies while ingress is full so neither direction can deadlock.
    fn send(&self, mut message: fetch::Message) -> Result<()> {
        loop {
            self.shared
                .check()
                .context("Failed to transfer fetch message")?;
            match self.ingress.try_send(message) {
                Ok(()) => return Ok(()),
                Err(channel::TrySendError::Full(returned)) => {
                    reconcile_span!("ingress_backpressure");
                    message = returned;
                    let mut selection = channel::Select::new();
                    let send = selection.send(&self.ingress);
                    let receive = selection.recv(&self.replies);
                    if let Ok(operation) = selection.select_timeout(time::Duration::from_millis(25))
                    {
                        if operation.index() == send {
                            operation.send(&self.ingress, message).map_err(|error| {
                                crate::Error::new(format!("Failed to send fetch message: {error}"))
                            })?;
                            return Ok(());
                        }
                        ensure!(
                            operation.index() == receive,
                            "Failed to select fetch channel"
                        );
                        let batch = operation.recv(&self.replies).map_err(|error| {
                            crate::Error::new(format!("Failed to receive fetch replies: {error}"))
                        })?;
                        self.commits.borrow_mut().extend(batch);
                        self.poll(false)
                            .context("Failed to commit fetch backpressure replies")?;
                    }
                }
                Err(channel::TrySendError::Disconnected(_)) => {
                    return Err(crate::Error::new(
                        "Failed to transfer fetch message: disconnected",
                    ));
                }
            }
        }
    }

    // Wait only for retained native work and commit replies on the connection thread.
    fn poll(&self, wait: bool) -> Result<()> {
        self.shared
            .check()
            .context("Failed to poll reconcile completion")?;
        if wait && self.pending() && self.commits.borrow().is_empty() {
            reconcile_span!("completion_wait");
            match self.replies.recv_timeout(time::Duration::from_millis(25)) {
                Ok(batch) => self.commits.borrow_mut().extend(batch),
                Err(channel::RecvTimeoutError::Timeout) => {}
                Err(channel::RecvTimeoutError::Disconnected) => {
                    return Err(crate::Error::new(
                        "Failed to receive reconcile completion: fetch disconnected",
                    ));
                }
            }
        }
        for batch in self.replies.try_iter().take(32) {
            self.commits.borrow_mut().extend(batch);
        }
        loop {
            let completed = self.commits.borrow_mut().pop_front();
            let Some(completed) = completed else { break };
            self.shared.count.fetch_sub(1, atomic::Ordering::Release);
            self.shared
                .bytes
                .fetch_sub(completed.bytes, atomic::Ordering::Release);
            let (request, reply) = completed
                .reply
                .context("Failed to complete reconcile worker")?;
            reconcile_span!("sdk_commit");
            reply
                .commit(request)
                .context("Failed to commit reconcile reply")?;
        }
        Ok(())
    }

    // Pump native completions while collecting a synchronous fetch helper response.
    fn response<T>(&self, receiver: channel::Receiver<Result<T>>) -> Result<T> {
        loop {
            self.shared
                .check()
                .context("Failed to wait for fetch helper")?;
            match receiver.recv_timeout(time::Duration::from_millis(1)) {
                Ok(result) => return result.context("Failed to complete fetch helper"),
                Err(channel::RecvTimeoutError::Timeout) => self
                    .poll(false)
                    .context("Failed to progress fetch helper replies")?,
                Err(channel::RecvTimeoutError::Disconnected) => {
                    return Err(crate::Error::new(
                        "Failed to receive fetch helper: disconnected",
                    ));
                }
            }
        }
    }

    // Include every ingress, running, ordered and queued reply in stage barriers.
    fn pending(&self) -> bool {
        self.shared.count.load(atomic::Ordering::Acquire) != 0
    }

    // Join the fetch owner and all native-data users before closing the SDK session.
    pub(crate) fn close(&self) -> Result<()> {
        reconcile_span!("cleanup");
        if let Some(worker) = self
            .worker
            .lock()
            .map_err(|_| crate::Error::new("Failed to lock fetch lifecycle"))?
            .take()
        {
            // A disconnected fetch has already entered cleanup.
            let _disconnected = self.ingress.send(fetch::Message::Stop).is_err();
            worker
                .join()
                .map_err(|_| crate::Error::new("Failed to join reconcile fetch thread"))?;
        }
        self.replies.try_iter().for_each(drop);
        self.commits.borrow_mut().clear();
        self.shared.count.store(0, atomic::Ordering::Release);
        self.shared.bytes.store(0, atomic::Ordering::Release);
        Ok(())
    }

    // Recover the original fetch failure after all borrowed SDK resources are released.
    pub(crate) fn finish(&self) -> Result<()> {
        self.close().context("Failed to finish reconcile fetch")?;
        match self
            .shared
            .failure
            .lock()
            .map_err(|_| crate::Error::new("Failed to read fetch failure"))?
            .take()
        {
            Some(error) => Err(error),
            None => Ok(()),
        }
    }

    // Contain SDK callback panics and retain the first contextual failure.
    pub(crate) unsafe extern "C" fn callback(
        context: *mut c_void,
        operation: u32,
        task: *mut c_void,
        data: *const u8,
        length: usize,
    ) -> i32 {
        // Native execution retains this runtime through synchronous cleanup.
        let runtime = unsafe { &*context.cast::<Self>() };
        let result = std::panic::catch_unwind(std::panic::AssertUnwindSafe(|| {
            // Submitted task ownership is adopted before metadata validation.
            unsafe { runtime.dispatch(operation, task, data, length) }
        }))
        .unwrap_or_else(|_| {
            Err(crate::Error::new(
                "Failed to dispatch reconcile callback: panicked",
            ))
        });
        match result {
            Ok(value) => value,
            Err(error) => {
                runtime.shared.fail(error);
                -1
            }
        }
    }
}

impl Drop for Runtime {
    // Join the fetch thread even when native command initialization fails.
    fn drop(&mut self) {
        if let Err(error) = self.close() {
            eprintln!("Failed to drop reconcile fetch: {error}");
        }
    }
}
