use super::{Statistics, local::Snapshot, path::Path, results::Results};
use crate::reconcile::pool::Work;
use crate::{ReconcileKind, ReconcileReply, ReconcileRequest, Result, ResultExt, error::ensure};
use std::{
    collections::{HashMap, HashSet, VecDeque},
    sync::{self, atomic},
    time,
};

enum Side {
    Local(Snapshot),
    Server(Box<Work>),
}
pub(super) struct Tables {
    paths: [sync::Mutex<HashMap<Path, Side>>; 32],
    results: sync::Mutex<Results>,
    ready: sync::mpsc::Sender<Work>,
    pub done: atomic::AtomicBool,
    pub stop: atomic::AtomicBool,
    pub fallback: atomic::AtomicBool,
    local: atomic::AtomicU64,
    directories: atomic::AtomicU64,
    server: atomic::AtomicU64,
    compared: atomic::AtomicU64,
    cached: atomic::AtomicU64,
    timestamps: atomic::AtomicU64,
    comparison_hashed: atomic::AtomicU64,
    scan_nanos: atomic::AtomicU64,
    compare_nanos: atomic::AtomicU64,
    probes: sync::Mutex<VecDeque<String>>,
    probed: sync::Mutex<HashSet<Path>>,
    started: time::Instant,
    pub metadata: super::metadata::Metadata,
}

impl Tables {
    // Allocate independent shards for path pairing and separate B/C result ownership.
    pub(super) fn new(ready: sync::mpsc::Sender<Work>) -> Self {
        Self {
            paths: std::array::from_fn(|_| sync::Mutex::new(HashMap::new())),
            results: sync::Mutex::new(Results::default()),
            ready,
            done: atomic::AtomicBool::new(false),
            stop: atomic::AtomicBool::new(false),
            fallback: atomic::AtomicBool::new(false),
            local: atomic::AtomicU64::new(0),
            directories: atomic::AtomicU64::new(0),
            server: atomic::AtomicU64::new(0),
            compared: atomic::AtomicU64::new(0),
            cached: atomic::AtomicU64::new(0),
            timestamps: atomic::AtomicU64::new(0),
            comparison_hashed: atomic::AtomicU64::new(0),
            scan_nanos: atomic::AtomicU64::new(0),
            compare_nanos: atomic::AtomicU64::new(0),
            probes: sync::Mutex::new(VecDeque::new()),
            probed: sync::Mutex::new(HashSet::new()),
            started: time::Instant::now(),
            metadata: super::metadata::Metadata::new(),
        }
    }

    // Select a small command-local shard without adding a dependency for hashing.
    fn shard(&self, path: &Path) -> &sync::Mutex<HashMap<Path, Side>> {
        &self.paths[path.shard(self.paths.len())]
    }

    // Move a complete pair to scheduler admission after releasing its path shard.
    fn send(&self, work: Work) -> Result<()> {
        self.ready.send(work).map_err(|error| {
            crate::Error::new(format!("Failed to send complete path pair: {error}"))
        })
    }

    // Retain one local slot or atomically take its existing server counterpart.
    pub(super) fn local(&self, path: &str, snapshot: Snapshot) -> Result<()> {
        self.local.fetch_add(1, atomic::Ordering::Relaxed);
        let key = Path::new(path);
        let work = {
            let mut table = self
                .shard(&key)
                .lock()
                .map_err(|_| crate::Error::new("Failed to lock local path shard"))?;
            if self
                .probed
                .lock()
                .map_err(|_| crate::Error::new("Failed to inspect targeted probe deduplication"))?
                .remove(&key)
            {
                return Ok(());
            }
            match table.remove(&key) {
                Some(Side::Server(mut work)) => {
                    work.snapshot = Some(snapshot);
                    Some(*work)
                }
                Some(Side::Local(_)) => {
                    return Err(crate::Error::new(
                        "Failed to enumerate duplicate local path",
                    ));
                }
                None => {
                    table.insert(key, Side::Local(snapshot));
                    None
                }
            }
        };
        if let Some(work) = work {
            self.send(work)
                .context("Failed to deliver local-first pair")?;
        }
        Ok(())
    }

    // Hold frozen server metadata until its local slot exists or enumeration is complete.
    pub(super) fn server(&self, mut work: Work) -> Result<()> {
        self.server.fetch_add(1, atomic::Ordering::Relaxed);
        if self.fallback.load(atomic::Ordering::Acquire) {
            return self
                .send(work)
                .context("Failed to dispatch canonical fallback request");
        }
        let path = work
            .request
            .path_bytes()
            .context("Failed to get server record path")?;
        let Ok(original) = std::str::from_utf8(path) else {
            self.use_fallback();
            return self.send(work).context("Failed to compare opaque SDK path");
        };
        let key = Path::new(original);
        let original = original.to_owned();
        let mut duplicate = None;
        let ready = {
            let mut table = self
                .shard(&key)
                .lock()
                .map_err(|_| crate::Error::new("Failed to lock server path shard"))?;
            match table.remove(&key) {
                Some(Side::Local(snapshot)) => {
                    work.snapshot = Some(snapshot);
                    Some(work)
                }
                Some(Side::Server(previous)) => {
                    duplicate = Some(*previous);
                    Some(work)
                }
                None if self.done.load(atomic::Ordering::Acquire) => Some(work),
                None => {
                    table.insert(key, Side::Server(Box::new(work)));
                    self.probes
                        .lock()
                        .map_err(|_| crate::Error::new("Failed to queue targeted local probe"))?
                        .push_back(original);
                    None
                }
            }
        };
        if let Some(work) = duplicate {
            self.send(work)
                .context("Failed to compare repeated server path")?;
        }
        if let Some(work) = ready {
            self.send(work)
                .context("Failed to deliver server-first pair")?;
        }
        Ok(())
    }

    // Prioritize server-requested paths without waiting for an unrelated directory subtree.
    pub(super) fn probe(&self) -> Result<Option<String>> {
        self.probes
            .lock()
            .map_err(|_| crate::Error::new("Failed to receive targeted local probe"))
            .map(|mut probes| probes.pop_front())
    }
    // Stop only speculative scanning while keeping SDK execution and cleanup active.
    pub(super) fn use_fallback(&self) {
        self.fallback.store(true, atomic::Ordering::Release);
        self.stop.store(true, atomic::Ordering::Release);
    }

    // Pair targeted metadata atomically and prevent the ordinary scanner from inserting it twice.
    pub(super) fn probed(&self, path: &str, snapshot: Snapshot) -> Result<()> {
        let key = Path::new(path);
        let ready = {
            let mut table = self
                .shard(&key)
                .lock()
                .map_err(|_| crate::Error::new("Failed to pair targeted probe"))?;
            match table.remove(&key) {
                Some(Side::Server(mut work)) => {
                    work.snapshot = Some(snapshot);
                    self.probed
                        .lock()
                        .map_err(|_| crate::Error::new("Failed to deduplicate targeted probe"))?
                        .insert(key);
                    Some(*work)
                }
                Some(local) => {
                    table.insert(key, local);
                    None
                }
                None => None,
            }
        };
        if let Some(work) = ready {
            self.send(work)
                .context("Failed to dispatch targeted path pair")?;
        }
        Ok(())
    }

    // Count visited directories without serializing scanner workers.
    pub(super) fn directory(&self) {
        self.directories.fetch_add(1, atomic::Ordering::Relaxed);
    }

    // Release remaining server records for canonical missing-file checks after every scanner joins.
    pub(super) fn finish_scan(&self, duration: time::Duration) -> Result<()> {
        self.scan_nanos.store(
            u64::try_from(duration.as_nanos()).context("Failed to measure scan duration")?,
            atomic::Ordering::Relaxed,
        );
        self.done.store(true, atomic::Ordering::Release);
        for shard in &self.paths {
            let requests = {
                let mut table = shard
                    .lock()
                    .map_err(|_| crate::Error::new("Failed to drain server path shard"))?;
                let keys: Vec<_> = table
                    .iter()
                    .filter_map(|(key, side)| {
                        matches!(side, Side::Server(_)).then_some(key.clone())
                    })
                    .collect();
                keys.into_iter()
                    .filter_map(|key| match table.remove(&key) {
                        Some(Side::Server(work)) => Some(*work),
                        _ => None,
                    })
                    .collect::<Vec<_>>()
            };
            for work in requests {
                self.send(work)
                    .context("Failed to dispatch unmatched server path")?;
            }
        }
        Ok(())
    }

    // Yield single-sided local candidates without revisiting the filesystem tree.
    pub(super) fn paths(&self, directory: &[u8]) -> Result<Vec<(String, Snapshot)>> {
        ensure!(
            self.done.load(atomic::Ordering::Acquire),
            "Failed to read candidates before scan completion"
        );
        let directory = Path::normalized(
            std::str::from_utf8(directory).context("Failed to decode candidate directory")?,
        );
        let directory = directory.trim_end_matches('/');
        let mut paths = Vec::new();
        for shard in &self.paths {
            let mut table = shard
                .lock()
                .map_err(|_| crate::Error::new("Failed to collect local path shard"))?;
            let keys: Vec<_> = table
                .keys()
                .filter(|path| path.within(directory))
                .cloned()
                .collect();
            for key in keys {
                if let Some(Side::Local(snapshot)) = table.remove(&key) {
                    paths.push((key.original().to_owned(), snapshot));
                }
            }
        }
        paths.sort_by(|left, right| left.0.cmp(&right.0));
        Ok(paths)
    }

    // Pair only complementary additions and deletions while retaining worker duration.
    pub(super) fn completed(
        &self,
        request: &ReconcileRequest,
        reply: &ReconcileReply,
        duration: time::Duration,
    ) -> Result<()> {
        self.compare_nanos.fetch_add(
            u64::try_from(duration.as_nanos()).context("Failed to measure comparison duration")?,
            atomic::Ordering::Relaxed,
        );
        if let Some(bytes) = reply.result.get_raw(b"hashedBytes") {
            let bytes = std::str::from_utf8(bytes)
                .context("Failed to decode digest payload size")?
                .parse::<i64>()
                .context("Failed to parse digest payload size")?;
            self.comparison_hashed.fetch_add(
                u64::try_from(bytes.max(0)).context("Failed to count digest payload size")?,
                atomic::Ordering::Relaxed,
            );
        }
        if reply.result.get_raw(b"cachedDigest") == Some(b"1") {
            self.cached.fetch_add(1, atomic::Ordering::Relaxed);
        }
        if request.kind == ReconcileKind::TrackedFile {
            self.compared.fetch_add(1, atomic::Ordering::Relaxed);
            if reply.result.get_raw(b"timestampMatch") == Some(b"1") {
                self.timestamps.fetch_add(1, atomic::Ordering::Relaxed);
            }
        }
        if self.fallback.load(atomic::Ordering::Acquire) {
            return Ok(());
        }
        self.results
            .lock()
            .map_err(|_| crate::Error::new("Failed to lock digest and result tables"))?
            .completed(request, reply)
            .context("Failed to classify local comparison")
    }

    // Retain complete authoritative SDK records in C after move classification.
    pub(super) fn output(&self, record: crate::Record) -> Result<()> {
        self.results
            .lock()
            .map_err(|_| crate::Error::new("Failed to finalize operation table C"))?
            .output(record);
        Ok(())
    }

    // Release unmatched digest candidates after their server classifications arrive.
    pub(super) fn finish_results(&self) -> Result<()> {
        self.results
            .lock()
            .map_err(|_| crate::Error::new("Failed to drain digest table B"))?
            .finish();
        Ok(())
    }
    // Read bounded aggregate counters instead of copying every result or unmatched path.
    pub(super) fn statistics(&self) -> Result<Statistics> {
        let mut unmatched_paths = 0;
        for shard in &self.paths {
            unmatched_paths += shard
                .lock()
                .map_err(|_| crate::Error::new("Failed to count path table A"))?
                .len();
        }
        let results = self
            .results
            .lock()
            .map_err(|_| crate::Error::new("Failed to count tables B and C"))?;
        let (digest_candidates, digest_pairs, final_results) = results.counts();
        Ok(Statistics {
            local_files: self.local.load(atomic::Ordering::Relaxed),
            local_directories: self.directories.load(atomic::Ordering::Relaxed),
            server_files: self.server.load(atomic::Ordering::Relaxed),
            compared_files: self.compared.load(atomic::Ordering::Relaxed),
            cached_digests: self.cached.load(atomic::Ordering::Relaxed),
            metadata_files: self.metadata.files.load(atomic::Ordering::Relaxed),
            metadata_directories: self.metadata.listings.load(atomic::Ordering::Relaxed),
            metadata_reuses: self.metadata.reuses.load(atomic::Ordering::Relaxed),
            timestamp_matches: self.timestamps.load(atomic::Ordering::Relaxed),
            scan_hashed_bytes: self.metadata.hashed.load(atomic::Ordering::Relaxed),
            comparison_hashed_bytes: self.comparison_hashed.load(atomic::Ordering::Relaxed),
            unmatched_paths,
            digest_candidates,
            digest_pairs,
            results: final_results,
            scan_seconds: if self.done.load(atomic::Ordering::Acquire) {
                self.scan_nanos.load(atomic::Ordering::Relaxed) as f64 / 1e9
            } else {
                self.started.elapsed().as_secs_f64()
            },
            compare_seconds: self.compare_nanos.load(atomic::Ordering::Relaxed) as f64 / 1e9,
        })
    }
}
