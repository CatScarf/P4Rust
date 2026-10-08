use super::super::{local::Snapshot, metadata::Metadata};
use crate::reconcile::runtime::fetch::Message;
use crate::{Result, ResultExt, control::Control};
use crossbeam_channel as channel;
use std::sync::{self, atomic};

pub(in crate::reconcile) enum Local {
    Files(Vec<(String, Snapshot)>),
    Probe(String, Snapshot),
}

pub(in crate::reconcile) struct Shared {
    pub metadata: Metadata,
    pub stop: atomic::AtomicBool,
    pub fallback: atomic::AtomicBool,
    pub directories: atomic::AtomicU64,
    pub scan_nanos: atomic::AtomicU64,
    events: channel::Sender<Message>,
    pub probes: channel::Receiver<String>,
}

impl Shared {
    // Share only metadata and scan control while the fetch thread owns A/B/C.
    pub(super) fn new(events: channel::Sender<Message>, probes: channel::Receiver<String>) -> Self {
        Self {
            metadata: Metadata::new(),
            stop: atomic::AtomicBool::new(false),
            fallback: atomic::AtomicBool::new(false),
            directories: atomic::AtomicU64::new(0),
            scan_nanos: atomic::AtomicU64::new(0),
            events,
            probes,
        }
    }

    // Apply bounded scan backpressure with prompt cancellation and shutdown checks.
    fn send(&self, mut event: Message, control: &Control) -> Result<()> {
        reconcile_span!("scan_publish_wait");
        loop {
            if self.stop.load(atomic::Ordering::Acquire) {
                return Ok(());
            }
            if let Some(error) = control.error() {
                return Err(error).context("Failed to publish local scan batch");
            }
            match self
                .events
                .send_timeout(event, std::time::Duration::from_millis(25))
            {
                Ok(()) => return Ok(()),
                Err(channel::SendTimeoutError::Timeout(returned)) => event = returned,
                Err(channel::SendTimeoutError::Disconnected(_)) => {
                    if self.stop.load(atomic::Ordering::Acquire) {
                        return Ok(());
                    }
                    return Err(crate::Error::new(
                        "Failed to publish local scan batch: fetch disconnected",
                    ));
                }
            }
        }
    }

    // Transfer a directory batch without sharing mutable path tables.
    pub(in crate::reconcile) fn publish(
        &self,
        files: &mut Vec<(String, Snapshot)>,
        control: &Control,
    ) -> Result<()> {
        if files.is_empty() {
            return Ok(());
        }
        self.send(Message::Local(Local::Files(std::mem::take(files))), control)
            .context("Failed to transfer local directory batch")
    }

    // Transfer a targeted result independently of a partially filled scan batch.
    pub(in crate::reconcile) fn probed(
        &self,
        path: String,
        snapshot: Snapshot,
        control: &Control,
    ) -> Result<()> {
        self.send(Message::Local(Local::Probe(path, snapshot)), control)
            .context("Failed to transfer targeted local metadata")
    }

    // Receive one priority path without taking a path-table or probe-queue mutex.
    pub(in crate::reconcile) fn probe(&self) -> Result<Option<String>> {
        match self.probes.try_recv() {
            Ok(path) => Ok(Some(path)),
            Err(channel::TryRecvError::Empty | channel::TryRecvError::Disconnected) => Ok(None),
        }
    }

    // Preserve canonical traversal for filenames that Rust cannot represent.
    pub(in crate::reconcile) fn use_fallback(&self) {
        self.fallback.store(true, atomic::Ordering::Release);
        self.stop.store(true, atomic::Ordering::Release);
    }

    // Count accepted directories independently of the fetch-owned path table.
    pub(in crate::reconcile) fn directory(&self) {
        self.directories.fetch_add(1, atomic::Ordering::Relaxed);
    }

    // Publish scanner completion only after all producer threads have finished.
    pub(in crate::reconcile) fn finish_scan(
        &self,
        duration: std::time::Duration,
        control: &Control,
    ) -> Result<()> {
        reconcile_span!("scan_finalize");
        self.metadata
            .compact()
            .context("Failed to compact scanned metadata")?;
        self.scan_nanos.store(
            u64::try_from(duration.as_nanos()).context("Failed to measure scan duration")?,
            atomic::Ordering::Relaxed,
        );
        self.send(Message::ScanFinished, control)
            .context("Failed to publish scan completion")
    }
}

pub(in crate::reconcile) type ScanShared = sync::Arc<Shared>;
