pub(super) mod shared;
use super::{
    Statistics,
    local::Snapshot,
    path::{Path, PathRef},
    results::Results,
};
use crate::reconcile::pool::Work;
use crate::{ReconcileKind, ReconcileReply, ReconcileRequest, Result, ResultExt, error::ensure};
use crossbeam_channel as channel;
use shared::{Local, ScanShared};
use std::{
    collections::{HashMap, HashSet, VecDeque},
    sync::atomic,
    time,
};

enum Side {
    Local,
    Server(Box<Work>),
}
pub(super) struct Tables {
    paths: HashMap<Path, Side>,
    results: Results,
    ready: VecDeque<Work>,
    probes: channel::Sender<String>,
    probed: HashSet<Path>,
    pub shared: ScanShared,
    finalized: bool,
    local: u64,
    server: u64,
    compared: u64,
    cached: u64,
    timestamps: u64,
    comparison_hashed: u64,
    compare_nanos: u64,
    started: time::Instant,
}

impl Tables {
    // Keep every A/B/C mutation on the fetch thread, with bounded local batches.
    pub(super) fn new(events: channel::Sender<crate::reconcile::runtime::fetch::Message>) -> Self {
        let (probes, receiver) = channel::unbounded();
        Self {
            paths: HashMap::new(),
            results: Results::default(),
            ready: VecDeque::new(),
            probes,
            probed: HashSet::new(),
            shared: std::sync::Arc::new(shared::Shared::new(events, receiver)),
            finalized: false,
            local: 0,
            server: 0,
            compared: 0,
            cached: 0,
            timestamps: 0,
            comparison_hashed: 0,
            compare_nanos: 0,
            started: time::Instant::now(),
        }
    }

    // Reuse exactly one metadata snapshot when a local marker has already arrived.
    fn send(&mut self, mut work: Work) -> Result<()> {
        if work.snapshot.is_none()
            && !self.shared.fallback.load(atomic::Ordering::Acquire)
            && let Ok(path) = std::str::from_utf8(
                work.request
                    .path_bytes()
                    .context("Failed to read paired path")?,
            )
        {
            work.snapshot = Some(
                self.shared
                    .metadata
                    .snapshot_for(path)
                    .context("Failed to reuse paired metadata")?,
            );
        }
        self.ready.push_back(work);
        Ok(())
    }

    // Pair one local record without taking a shared table lock.
    fn local(&mut self, path: &str, snapshot: Snapshot, probe: bool) -> Result<()> {
        reconcile_span!("pair_local");
        let key = Path::new(path);
        if !probe {
            self.local += 1;
            if self.probed.remove(&key) {
                return Ok(());
            }
        }
        match self.paths.remove(&key) {
            Some(Side::Server(mut work)) => {
                work.snapshot = Some(snapshot);
                if probe {
                    self.probed.insert(key);
                }
                self.send(*work)
                    .context("Failed to deliver local-first pair")?;
            }
            Some(side) if probe => {
                self.paths.insert(key, side);
            }
            Some(Side::Local) => {
                return Err(crate::Error::new(
                    "Failed to enumerate duplicate local path",
                ));
            }
            None if !probe => {
                self.paths.insert(key, Side::Local);
            }
            None => {}
        }
        Ok(())
    }

    // Freeze a server slot or pair it with a previously received local marker.
    pub(super) fn server(&mut self, work: Work) -> Result<()> {
        reconcile_span!("pair_server");
        self.server += 1;
        if self.shared.fallback.load(atomic::Ordering::Acquire) {
            return self.send(work);
        }
        let path = work
            .request
            .path_bytes()
            .context("Failed to get server record path")?;
        let Ok(original) = std::str::from_utf8(path) else {
            self.shared.use_fallback();
            return self.send(work).context("Failed to compare opaque SDK path");
        };
        match self.paths.remove(PathRef::new(original)) {
            Some(Side::Local) => self
                .send(work)
                .context("Failed to deliver server-first pair")?,
            Some(Side::Server(previous)) => {
                self.send(*previous)
                    .context("Failed to compare repeated server path")?;
                self.send(work)
                    .context("Failed to compare duplicate server path")?;
            }
            None if self.finalized => self
                .send(work)
                .context("Failed to dispatch late server path")?,
            None => {
                self.probes.send(original.to_owned()).map_err(|error| {
                    crate::Error::new(format!("Failed to queue targeted probe: {error}"))
                })?;
                self.paths
                    .insert(Path::new(original), Side::Server(Box::new(work)));
            }
        }
        if self.finalized {
            self.compact();
        }
        Ok(())
    }

    // Pair each arriving scan event immediately on the fetch owner.
    pub(super) fn consume(&mut self, local: Local) -> Result<()> {
        reconcile_span!("fetch_local_batches");
        match local {
            Local::Files(files) => {
                for (path, snapshot) in files {
                    self.local(&path, snapshot, false)
                        .context("Failed to pair scanned batch")?;
                }
            }
            Local::Probe(path, snapshot) => self
                .local(&path, snapshot, true)
                .context("Failed to pair targeted probe")?,
        }
        Ok(())
    }

    // Finalize unmatched server slots only after the scanner completion event.
    pub(super) fn finish_scan(&mut self) -> Result<()> {
        self.finalized = true;
        self.probed = HashSet::new();
        while self.shared.probes.try_recv().is_ok() {}
        let keys: Vec<_> = self
            .paths
            .iter()
            .filter_map(|(key, side)| matches!(side, Side::Server(_)).then_some(key.clone()))
            .collect();
        for key in keys {
            if let Some(Side::Server(work)) = self.paths.remove(&key) {
                self.send(*work)
                    .context("Failed to dispatch unmatched server path")?;
            }
        }
        self.compact();
        Ok(())
    }
    // Transfer available pairs without allocating a per-poll result vector.
    pub(super) fn ready(&mut self) -> Option<Work> {
        self.ready.pop_front()
    }

    // Include undelivered scanner batches when reporting enumeration completion.
    pub(super) fn scanning(&self) -> bool {
        !self.finalized
    }

    // Yield remaining additions after every scanner batch has been consumed.
    pub(super) fn paths(&mut self, directory: &[u8]) -> Result<Vec<(String, Snapshot)>> {
        reconcile_span!("candidate_collect");
        ensure!(
            self.finalized,
            "Failed to read candidates before scan completion"
        );
        let directory = Path::normalized(
            std::str::from_utf8(directory).context("Failed to decode candidate directory")?,
        );
        let directory = directory.trim_end_matches('/');
        let keys: Vec<_> = self
            .paths
            .keys()
            .filter(|key| key.within(directory))
            .cloned()
            .collect();
        let mut paths = Vec::new();
        for key in keys {
            if let Some(Side::Local) = self.paths.remove(&key) {
                paths.push(key.original().to_owned());
            }
        }
        self.compact();
        paths.sort();
        paths
            .into_iter()
            .map(|path| {
                let snapshot = self
                    .shared
                    .metadata
                    .snapshot_for(&path)
                    .context("Failed to reuse unmatched metadata")?;
                Ok((path, snapshot))
            })
            .collect()
    }

    // Shrink depleted path storage only at geometric thresholds.
    fn compact(&mut self) {
        if self.paths.is_empty() {
            self.paths = HashMap::new();
        } else if self.paths.capacity() > 1024 && self.paths.len() < self.paths.capacity() / 4 {
            self.paths.shrink_to(self.paths.len() * 2);
        }
    }

    // Classify comparisons and update counters with exclusive fetch ownership.
    pub(super) fn completed(
        &mut self,
        request: &ReconcileRequest,
        reply: &ReconcileReply,
        duration: time::Duration,
    ) -> Result<()> {
        reconcile_span!("comparison_result");
        self.compare_nanos +=
            u64::try_from(duration.as_nanos()).context("Failed to measure comparison duration")?;
        if let Some(bytes) = reply.result.get_raw(b"hashedBytes") {
            let bytes = std::str::from_utf8(bytes)
                .context("Failed to decode digest payload size")?
                .parse::<i64>()
                .context("Failed to parse digest payload size")?;
            self.comparison_hashed +=
                u64::try_from(bytes.max(0)).context("Failed to count digest payload size")?;
        }
        self.cached += u64::from(reply.result.get_raw(b"cachedDigest") == Some(b"1"));
        if request.kind == ReconcileKind::TrackedFile {
            self.compared += 1;
            self.timestamps += u64::from(reply.result.get_raw(b"timestampMatch") == Some(b"1"));
        }
        if self.shared.fallback.load(atomic::Ordering::Acquire) {
            return Ok(());
        }
        self.results
            .completed(request, reply)
            .context("Failed to classify local comparison")
    }

    // Retain authoritative SDK output after move classification.
    pub(super) fn output(&mut self, record: crate::Record) {
        if !self.shared.fallback.load(atomic::Ordering::Acquire) {
            self.results.output(record);
        }
    }
    // Release unmatched digest candidates after SDK output completes.
    pub(super) fn finish_results(&mut self) {
        self.results.finish();
    }
    // Snapshot fetch-owned counters and shared metadata statistics.
    pub(super) fn statistics(&self) -> Statistics {
        let (digest_candidates, digest_pairs, results) = self.results.counts();
        Statistics {
            local_files: self.local,
            local_directories: self.shared.directories.load(atomic::Ordering::Relaxed),
            server_files: self.server,
            compared_files: self.compared,
            cached_digests: self.cached,
            metadata_files: self.shared.metadata.files.load(atomic::Ordering::Relaxed),
            metadata_directories: self
                .shared
                .metadata
                .listings
                .load(atomic::Ordering::Relaxed),
            metadata_reuses: self.shared.metadata.reuses.load(atomic::Ordering::Relaxed),
            timestamp_matches: self.timestamps,
            scan_hashed_bytes: self.shared.metadata.hashed.load(atomic::Ordering::Relaxed),
            comparison_hashed_bytes: self.comparison_hashed,
            unmatched_paths: self.paths.len(),
            digest_candidates,
            digest_pairs,
            results,
            scan_seconds: if self.finalized {
                self.shared.scan_nanos.load(atomic::Ordering::Relaxed) as f64 / 1e9
            } else {
                self.started.elapsed().as_secs_f64()
            },
            compare_seconds: self.compare_nanos as f64 / 1e9,
        }
    }
}
