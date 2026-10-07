use super::{ReconcileHandler, ReconcileKind, ReconcileReply, ReconcileRequest};
use crate::{Result, ResultExt};
use std::{sync, thread};

pub(super) struct Work {
    pub id: u64,
    pub request: ReconcileRequest,
}
pub(super) struct Completed {
    pub id: u64,
    pub bytes: usize,
    pub reply: Result<ReconcileReply>,
}
pub(super) struct Pool {
    sender: Option<sync::mpsc::SyncSender<Work>>,
    workers: Vec<thread::JoinHandle<Result<()>>>,
}

impl Pool {
    // Spawn independent SDK workers while keeping all queue ownership within this command.
    pub(super) fn new(
        handler: sync::Arc<dyn ReconcileHandler>,
        kind: ReconcileKind,
        results: sync::mpsc::Sender<Completed>,
    ) -> Result<Self> {
        let (sender, receiver) = sync::mpsc::sync_channel::<Work>(handler.queue_capacity());
        let receiver = sync::Arc::new(sync::Mutex::new(receiver));
        let mut pool = Self {
            sender: Some(sender),
            workers: Vec::new(),
        };
        for index in 0..handler.workers(kind) {
            let receiver = sync::Arc::clone(&receiver);
            let handler = sync::Arc::clone(&handler);
            let results = results.clone();
            let worker = thread::Builder::new()
                .name(format!("p4rust-reconcile-{kind:?}-{index}"))
                .spawn(move || Self::work(receiver, handler, results))
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

    // Receive ownership briefly under the queue lock and run callbacks outside it.
    fn work(
        receiver: sync::Arc<sync::Mutex<sync::mpsc::Receiver<Work>>>,
        handler: sync::Arc<dyn ReconcileHandler>,
        results: sync::mpsc::Sender<Completed>,
    ) -> Result<()> {
        loop {
            let work = receiver
                .lock()
                .map_err(|_| {
                    crate::Error::new("Failed to receive reconcile work: queue lock poisoned")
                })?
                .recv();
            let Ok(work) = work else { break };
            let bytes = work.request.metadata.bytes.len();
            let pointer = work.request.task.pointer() as usize;
            let reply = match std::panic::catch_unwind(std::panic::AssertUnwindSafe(|| {
                handler.handle(work.request)
            })) {
                Ok(result) => result.context("Failed to handle reconcile request"),
                Err(_) => Err(crate::Error::new(
                    "Failed to handle reconcile request: worker panicked",
                )),
            }
            .and_then(|reply| {
                crate::error::ensure!(
                    reply.request.task.pointer() as usize == pointer,
                    "Failed to handle reconcile request: reply belongs to a different request"
                );
                Ok(reply)
            });
            results
                .send(Completed {
                    id: work.id,
                    bytes,
                    reply,
                })
                .map_err(|error| {
                    crate::Error::new(format!("Failed to send reconcile completion: {error}"))
                })?;
        }
        Ok(())
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
