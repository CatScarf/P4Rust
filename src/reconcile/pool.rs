use super::runtime::fetch::Message;
use super::{ReconcileHandler, ReconcileKind, ReconcileReply, ReconcileRequest};
use crate::{Result, ResultExt};
use crossbeam_channel as channel;
use std::{sync, thread};

pub(super) struct Work {
    pub id: u64,
    pub request: ReconcileRequest,
    pub snapshot: Option<super::pipeline::local::Snapshot>,
}
pub(super) struct Completed {
    pub id: u64,
    pub bytes: usize,
    pub duration: std::time::Duration,
    pub reply: Result<(ReconcileRequest, ReconcileReply)>,
}
pub(super) struct Pool {
    sender: Option<channel::Sender<Work>>,
    workers: Vec<thread::JoinHandle<Result<()>>>,
}

impl Pool {
    // Spawn independent SDK workers while keeping all queue ownership within this command.
    pub(super) fn new(
        handler: sync::Arc<dyn ReconcileHandler>,
        kind: ReconcileKind,
        results: channel::Sender<Message>,
    ) -> Result<Self> {
        let (sender, receiver) = channel::bounded::<Work>(handler.queue_capacity());
        let mut pool = Self {
            sender: Some(sender),
            workers: Vec::new(),
        };
        for index in 0..handler.workers(kind) {
            #[cfg(feature = "reconcile-trace")]
            let trace = super::trace::ReconcileTrace::current();
            let receiver = receiver.clone();
            let handler = sync::Arc::clone(&handler);
            let results = results.clone();
            let worker = thread::Builder::new()
                .name(format!("p4rust-reconcile-{kind:?}-{index}"))
                .spawn(move || {
                    #[cfg(feature = "reconcile-trace")]
                    let _attachment = super::trace::ReconcileTrace::attach(trace)
                        .context("Failed to attach reconcile worker trace")?;
                    Self::work(receiver, handler, results)
                })
                .context("Failed to spawn reconcile worker");
            match worker {
                Ok(worker) => pool.workers.push(worker),
                Err(error) => {
                    pool.close()
                        .context("Failed to clean up partially initialized reconcile pool")?;
                    return Err(error);
                }
            }
        }
        Ok(pool)
    }

    // Receive owned work directly without a shared receiver mutex.
    fn work(
        receiver: channel::Receiver<Work>,
        handler: sync::Arc<dyn ReconcileHandler>,
        results: channel::Sender<Message>,
    ) -> Result<()> {
        loop {
            #[cfg(feature = "reconcile-trace")]
            let waiting = super::trace::ReconcileTrace::span("worker_queue_wait");
            let work = receiver.recv();
            #[cfg(feature = "reconcile-trace")]
            drop(waiting);
            let Ok(work) = work else { break };
            if results
                .send(Message::Completed(Box::new(Self::execute(
                    work,
                    handler.as_ref(),
                ))))
                .is_err()
            {
                // Fetch disconnects before joining workers during shutdown.
                break;
            }
        }
        Ok(())
    }

    // Run one isolated task on either a digest worker or the fetch owner.
    pub(super) fn execute(mut work: Work, handler: &dyn ReconcileHandler) -> Completed {
        let bytes = work.request.metadata.bytes.len();
        let pointer = work.request.task.pointer() as usize;
        let started = std::time::Instant::now();
        #[cfg(feature = "reconcile-trace")]
        let execution = super::trace::ReconcileTrace::span(match work.request.kind {
            ReconcileKind::TrackedFile => "sdk_tracked_other",
            ReconcileKind::ExactMatch => "sdk_exact",
            ReconcileKind::Directory => "sdk_directory",
            ReconcileKind::UntrackedFile => "sdk_untracked",
            ReconcileKind::Move => "sdk_move",
        });
        let reply = match std::panic::catch_unwind(std::panic::AssertUnwindSafe(|| {
            if let Some(snapshot) = &work.snapshot {
                // The worker owns both the native task and its copied local snapshot.
                let status = unsafe {
                    crate::ffi::p4rust_reconcile_snapshot_v5(work.request.task.pointer(), snapshot)
                };
                crate::error::ensure!(status == 0, "Failed to attach local reconcile snapshot");
            }
            handler.handle(&mut work.request)
        })) {
            Ok(result) => result.context("Failed to handle reconcile request"),
            Err(_) => Err(crate::Error::new(
                "Failed to handle reconcile request: worker panicked",
            )),
        }
        .and_then(|reply| {
            crate::error::ensure!(
                reply.identity == pointer && work.request.executed,
                "Failed to handle reconcile request: reply belongs to a different request"
            );
            Ok((work.request, reply))
        });
        #[cfg(feature = "reconcile-trace")]
        {
            if let Ok((request, reply)) = &reply
                && request.kind == ReconcileKind::TrackedFile
            {
                if reply.result.get_raw(b"timestampMatch") == Some(b"1") {
                    execution.classify("sdk_tracked_timestamp");
                } else if reply.result.get_raw(b"hashedBytes").is_some() {
                    execution.classify("sdk_tracked_hash");
                }
            }
            drop(execution);
        }
        Completed {
            id: work.id,
            bytes,
            duration: started.elapsed(),
            reply,
        }
    }

    // Enqueue only after the command runtime has reserved its total capacity.
    pub(super) fn submit(&self, work: Work) -> Result<()> {
        self.sender
            .as_ref()
            .context("Failed to submit to closed reconcile pool")?
            .send(work)
            .map_err(|error| {
                crate::Error::new(format!("Failed to enqueue reconcile request: {error}"))
            })
    }

    // Close the queue and join every worker before native connection state is released.
    pub(super) fn close(&mut self) -> Result<()> {
        self.sender.take();
        let mut failure = None;
        for worker in self.workers.drain(..) {
            match worker.join() {
                Ok(Ok(())) => {}
                Ok(Err(error)) => failure = Some(error),
                Err(_) => {
                    failure = Some(crate::Error::new(
                        "Failed to join reconcile worker: worker panicked",
                    ))
                }
            }
        }
        match failure {
            Some(error) => Err(error),
            None => Ok(()),
        }
    }
}

impl Drop for Pool {
    // Join remaining workers even when construction or native execution fails.
    fn drop(&mut self) {
        if let Err(error) = self.close() {
            eprintln!("Failed to clean up reconcile pool: {error}");
        }
    }
}
