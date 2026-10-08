use super::Shared;
use crate::reconcile::{ReconcileHandler, ReconcileKind, pipeline, pool};
use crate::{Result, ResultExt, error::ensure};
use crossbeam_channel as channel;
use std::{collections::BTreeMap, sync, time};

type Paths = Vec<(String, pipeline::local::Snapshot)>;
type PathsResponse = channel::Sender<Result<Option<Paths>>>;

pub(in crate::reconcile) enum Message {
    Request(Box<super::ingress::Frozen>),
    Paths(Vec<u8>, PathsResponse),
    Snapshot(
        Vec<u8>,
        channel::Sender<Result<Option<pipeline::local::Snapshot>>>,
    ),
    Output(crate::Record),
    Local(pipeline::Local),
    ScanFinished,
    Completed(Box<pool::Completed>),
    Stop,
}

pub(super) struct Fetch {
    pools: Option<[pool::Pool; 3]>,
    ingress: channel::Receiver<Message>,
    replies: channel::Sender<Vec<pool::Completed>>,
    pipeline: Option<pipeline::Pipeline>,
    handler: sync::Arc<dyn ReconcileHandler>,
    shared: sync::Arc<Shared>,
    completed: BTreeMap<u64, pool::Completed>,
    next: u64,
    last_progress: time::Instant,
    paths: Option<(Vec<u8>, PathsResponse)>,
}

impl Fetch {
    // Validate limits before starting any command-owned threads.
    pub(super) fn validate(handler: &dyn ReconcileHandler) -> Result<()> {
        for kind in [
            ReconcileKind::Directory,
            ReconcileKind::TrackedFile,
            ReconcileKind::Move,
        ] {
            ensure!(
                (1..=32).contains(&handler.workers(kind)),
                "Failed to configure reconcile workers: expected 1..=32"
            );
        }
        for kind in [ReconcileKind::UntrackedFile, ReconcileKind::ExactMatch] {
            ensure!(
                handler.workers(kind) == handler.workers(ReconcileKind::TrackedFile),
                "Failed to configure reconcile workers: digest limits must match"
            );
        }
        ensure!(
            (1..=65536).contains(&handler.queue_capacity()),
            "Failed to configure reconcile queue capacity"
        );
        ensure!(
            (1048576..=268435456).contains(&handler.queue_bytes()),
            "Failed to configure reconcile byte budget"
        );
        Ok(())
    }

    // Construct isolated pools and transfer all scheduler state to one fetch owner.
    #[allow(clippy::too_many_arguments)]
    pub(super) fn new(
        handler: sync::Arc<dyn ReconcileHandler>,
        shared: sync::Arc<Shared>,
        config: &crate::Config,
        args: &[&str],
        events: channel::Sender<Message>,
        ingress: channel::Receiver<Message>,
        replies: channel::Sender<Vec<pool::Completed>>,
    ) -> Result<Self> {
        let metadata = pool::Pool::new(
            sync::Arc::clone(&handler),
            ReconcileKind::Directory,
            events.clone(),
        )
        .context("Failed to create fetch metadata pool")?;
        let digest = pool::Pool::new(
            sync::Arc::clone(&handler),
            ReconcileKind::TrackedFile,
            events.clone(),
        )
        .context("Failed to create fetch digest pool")?;
        let moves = pool::Pool::new(
            sync::Arc::clone(&handler),
            ReconcileKind::Move,
            events.clone(),
        )
        .context("Failed to create fetch move pool")?;
        let pipeline = if handler.pipeline() {
            pipeline::Pipeline::start(
                config,
                args,
                handler.workers(ReconcileKind::Directory),
                events,
                shared.control.clone(),
            )
            .context("Failed to start fetch local scanning")?
        } else {
            None
        };
        Ok(Self {
            pools: Some([metadata, digest, moves]),
            ingress,
            replies,
            pipeline,
            handler,
            shared,
            completed: BTreeMap::new(),
            next: 0,
            last_progress: time::Instant::now(),
            paths: None,
        })
    }

    // Process producer events in arrival order and block only when the pipe is empty.
    pub(super) fn run(&mut self) -> Result<()> {
        loop {
            let message = {
                reconcile_span!("fetch_event_wait");
                self.ingress.recv().map_err(|error| {
                    crate::Error::new(format!("Failed to receive fetch event: {error}"))
                })?
            };
            if matches!(message, Message::Stop) {
                return Ok(());
            }
            self.process(message)
                .context("Failed to process fetch event")?;
            // Coalesce ready events and replies without a timer or table polling.
            for _ in 0..63 {
                let Ok(message) = self.ingress.try_recv() else {
                    break;
                };
                if matches!(message, Message::Stop) {
                    return Ok(());
                }
                self.process(message)
                    .context("Failed to process queued fetch event")?;
            }
            self.flush()
                .context("Failed to publish ordered fetch replies")?;
            self.paths()
                .context("Failed to service candidate request")?;
            self.progress(false)
                .context("Failed to report fetch progress")?;
        }
    }

    // Check failures and dispatch only pairs made ready by the arriving event.
    fn process(&mut self, message: Message) -> Result<()> {
        if let Some(pipeline) = &self.pipeline {
            pipeline
                .check()
                .context("Failed to inspect scanner failure")?;
        }
        self.shared
            .check()
            .context("Failed to continue reconcile fetch")?;
        self.message(message)
            .context("Failed to handle producer event")?;
        loop {
            let work = self.pipeline.as_mut().and_then(pipeline::Pipeline::ready);
            let Some(work) = work else { break };
            self.enqueue(work)
                .context("Failed to dispatch paired fetch task")?;
        }
        Ok(())
    }
    // Preserve SDK comparisons while avoiding worker scheduling for cheap timestamp matches.
    fn enqueue(&mut self, mut work: pool::Work) -> Result<()> {
        if work.snapshot.is_none()
            && let Some(pipeline) = self.pipeline.as_ref().filter(|pipeline| pipeline.enabled())
            && let Some(path) = work.request.path_bytes()
            && let Ok(path) = std::str::from_utf8(path)
        {
            work.snapshot = Some(
                pipeline
                    .snapshot(path)
                    .context("Failed to prepare isolated SDK task metadata")?,
            );
        }
        if self.handler.inline_timestamp() && Self::timestamp(&work) {
            reconcile_span!("fetch_timestamp");
            let completed = pool::Pool::execute(work, self.handler.as_ref());
            self.completed.insert(completed.id, completed);
            return Ok(());
        }
        let index = match work.request.kind {
            ReconcileKind::Directory => 0,
            ReconcileKind::Move => 2,
            _ => 1,
        };
        self.pools
            .as_ref()
            .context("Failed to enqueue after fetch shutdown")?[index]
            .submit(work)
            .context("Failed to send fetch comparison task")
    }

    // Inline only the branch that cannot enter a canonical content digest.
    pub(super) fn timestamp(work: &pool::Work) -> bool {
        if work.request.kind != ReconcileKind::TrackedFile
            || work.request.metadata.get_raw(b"digestType").is_some()
        {
            return false;
        }
        let Some(snapshot) = work.snapshot else {
            return false;
        };
        work.request
            .metadata
            .get_raw(b"time")
            .and_then(|time| std::str::from_utf8(time).ok())
            .and_then(|time| time.parse::<i32>().ok())
            .is_some_and(|time| i64::from(time) == snapshot.time)
    }

    // Route owned ingress without consulting shared scheduler mutexes.
    fn message(&mut self, message: Message) -> Result<()> {
        reconcile_span!("fetch_request");
        match message {
            Message::Request(request) => {
                let work = request
                    .decode()
                    .context("Failed to decode frozen SDK ingress")?;
                if work.request.kind == ReconcileKind::TrackedFile
                    && let Some(pipeline) = &mut self.pipeline
                {
                    pipeline
                        .server(work)
                        .context("Failed to pair fetched server path")?;
                } else {
                    self.enqueue(work)
                        .context("Failed to enqueue fetched task")?;
                }
            }
            Message::Local(local) => {
                if let Some(pipeline) = &mut self.pipeline {
                    pipeline
                        .consume(local)
                        .context("Failed to consume local event")?;
                }
            }
            Message::ScanFinished => {
                if let Some(pipeline) = &mut self.pipeline {
                    pipeline
                        .finish_scan()
                        .context("Failed to consume scan completion")?;
                }
            }
            Message::Completed(completed) => {
                self.completed.insert(completed.id, *completed);
            }
            Message::Stop => {}
            Message::Output(record) => {
                if let Some(pipeline) = &mut self.pipeline {
                    pipeline
                        .output(record)
                        .context("Failed to retain fetched SDK output")?;
                }
            }
            Message::Snapshot(path, sender) => {
                let result = self
                    .snapshot(&path)
                    .context("Failed to resolve SDK metadata helper");
                sender.send(result).map_err(|error| {
                    crate::Error::new(format!("Failed to return metadata helper: {error}"))
                })?;
            }
            Message::Paths(directory, sender) => {
                ensure!(
                    self.paths.is_none(),
                    "Failed to retain duplicate candidate request"
                );
                self.paths = Some((directory, sender));
            }
        }
        Ok(())
    }

    // Classify completed work and publish ordered reply batches without connection access.
    fn flush(&mut self) -> Result<()> {
        reconcile_span!("ordered_reply");
        let mut batch = Vec::new();
        while let Some(completed) = self.completed.remove(&self.next) {
            self.next += 1;
            let (request, reply) = completed.reply.as_ref().map_err(|error| {
                crate::Error::new(format!("Failed to complete fetch comparison: {error}"))
            })?;
            if let Some(pipeline) = &mut self.pipeline {
                pipeline
                    .completed(request, reply, completed.duration)
                    .context("Failed to classify fetch comparison")?;
            }
            batch.push(completed);
            if batch.len() == 64 {
                self.replies
                    .send(std::mem::take(&mut batch))
                    .map_err(|error| {
                        crate::Error::new(format!("Failed to publish ordered reply batch: {error}"))
                    })?;
            }
        }
        if !batch.is_empty() {
            self.replies.send(batch).map_err(|error| {
                crate::Error::new(format!("Failed to publish ordered reply batch: {error}"))
            })?;
        }
        Ok(())
    }

    // Defer candidate responses until every published scanner batch is consumed.
    fn paths(&mut self) -> Result<()> {
        if self
            .pipeline
            .as_ref()
            .is_some_and(|pipeline| pipeline.enabled() && pipeline.scanning())
        {
            return Ok(());
        }
        let Some((directory, sender)) = self.paths.take() else {
            return Ok(());
        };
        let result = self
            .pipeline
            .as_mut()
            .filter(|pipeline| pipeline.enabled())
            .map(|pipeline| pipeline.paths(&directory))
            .transpose()
            .context("Failed to collect candidate response");
        sender.send(result).map_err(|error| {
            crate::Error::new(format!("Failed to return candidate response: {error}"))
        })
    }

    // Keep exceptional SDK metadata requests on the same fetch-owned registry path.
    fn snapshot(&self, path: &[u8]) -> Result<Option<pipeline::local::Snapshot>> {
        let Some(pipeline) = self.pipeline.as_ref().filter(|pipeline| pipeline.enabled()) else {
            return Ok(None);
        };
        let Ok(path) = std::str::from_utf8(path) else {
            pipeline.use_fallback();
            return Ok(None);
        };
        pipeline
            .snapshot(path)
            .map(Some)
            .context("Failed to read SDK helper snapshot")
    }

    // Report counters outside all metadata locks at a bounded interval.
    fn progress(&mut self, force: bool) -> Result<()> {
        if !force && self.last_progress.elapsed() < time::Duration::from_secs(15) {
            return Ok(());
        }
        self.last_progress = time::Instant::now();
        if let Some(pipeline) = self.pipeline.as_ref().filter(|pipeline| pipeline.enabled()) {
            self.handler
                .progress(
                    &pipeline
                        .statistics()
                        .context("Failed to read fetch counters")?,
                )
                .context("Failed to deliver fetch counters")?;
        }
        Ok(())
    }

    // Stop scanner publishers and join every native task user before scope destruction.
    pub(super) fn close(&mut self) -> Result<()> {
        reconcile_span!("fetch_cleanup");
        let mut failure = None;
        if let Some(pipeline) = &self.pipeline {
            pipeline.stop();
        }
        // Disconnect every producer before joining threads that can be blocked on send.
        drop(std::mem::replace(&mut self.ingress, channel::never()));
        if let Some(pipeline) = &mut self.pipeline
            && let Err(error) = pipeline.close()
        {
            failure = Some(error);
        }
        if let Some(mut pools) = self.pools.take() {
            for pool in &mut pools {
                if let Err(error) = pool.close() {
                    failure.get_or_insert(error);
                }
            }
        }
        if let Err(error) = self.progress(true) {
            failure.get_or_insert(error);
        }
        self.pipeline.take();
        self.completed.clear();
        match failure {
            Some(error) => Err(error),
            None => Ok(()),
        }
    }
}

impl Drop for Fetch {
    // Release local and comparison workers when spawning or execution fails.
    fn drop(&mut self) {
        if let Err(error) = self.close() {
            eprintln!("Failed to drop fetch owner: {error}");
        }
    }
}
