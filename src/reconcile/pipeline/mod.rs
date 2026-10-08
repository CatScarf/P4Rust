pub(crate) mod local;
mod metadata;
mod names;
mod path;
mod radix;
mod results;
mod scanner;
mod tables;
use super::pool::Work;
use crate::{Config, Result, ResultExt, control::Control};
use std::{path::PathBuf, sync, thread, time};
use tables::Tables;
pub(super) use tables::shared::Local;

/// A snapshot of concurrent reconcile enumeration, pairing, and comparison work.
#[derive(Clone, Debug, Default)]
pub struct Statistics {
    pub local_files: u64,
    pub local_directories: u64,
    pub server_files: u64,
    pub compared_files: u64,
    pub timestamp_matches: u64,
    pub cached_digests: u64,
    /// Unique initialized path snapshots, including directories and missing paths.
    pub metadata_files: u64,
    /// Unique directory listings initialized by enumeration or targeted probes.
    pub metadata_directories: u64,
    /// Reads served by command-owned metadata instead of another filesystem query.
    pub metadata_reuses: u64,
    /// Physical payload sizes hashed by the local scanner, excluding classification probes.
    pub scan_hashed_bytes: u64,
    /// Physical payload sizes passed through SDK comparison digests, excluding move diffs.
    pub comparison_hashed_bytes: u64,
    pub unmatched_paths: usize,
    pub digest_candidates: usize,
    pub digest_pairs: usize,
    pub results: usize,
    pub scan_seconds: f64,
    /// Cumulative comparison worker time, rather than elapsed command time.
    pub compare_seconds: f64,
}

pub(super) struct Pipeline {
    tables: Tables,
    scanner: Option<thread::JoinHandle<Result<()>>>,
    failure: sync::mpsc::Receiver<crate::Error>,
}

impl Pipeline {
    // Start local workers before the command opens its single server connection.
    #[allow(clippy::too_many_arguments)]
    pub(super) fn start(
        config: &Config,
        args: &[&str],
        workers: usize,
        events: crossbeam_channel::Sender<super::runtime::fetch::Message>,
        control: Control,
    ) -> Result<Option<Self>> {
        if !matches!(config.charset.as_str(), "utf8" | "utf8unchecked")
            || args.iter().any(|argument| {
                argument.starts_with('-')
                    && !matches!(
                        *argument,
                        "-n" | "-m" | "-M" | "-I" | "-a" | "-e" | "-d" | "-t" | "-f" | "-l" | "-A"
                    )
            })
        {
            return Ok(None);
        }
        let paths: Vec<_> = args.iter().filter(|arg| !arg.starts_with('-')).collect();
        let scope = match paths.as_slice() {
            [scope] => scope
                .strip_suffix("/...")
                .or_else(|| scope.strip_suffix("\\...")),
            [] => Some(config.cwd.as_str()),
            _ => None,
        };
        let Some(scope) = scope.filter(|scope| !scope.contains(['*', '#', '@', '%'])) else {
            return Ok(None);
        };
        let root = PathBuf::from(scope);
        if !root.is_absolute()
            || !root.is_dir()
            || root
                .symlink_metadata()
                .context("Failed to inspect reconcile scope")?
                .file_type()
                .is_symlink()
        {
            return Ok(None);
        }
        let (errors, failure) = sync::mpsc::channel();
        let tables = Tables::new(events);
        let shared = sync::Arc::clone(&tables.shared);
        let config = config.clone();
        let hashes = !args.contains(&"-m");
        let ignore = !args.contains(&"-I");
        #[cfg(feature = "reconcile-trace")]
        let trace = super::trace::ReconcileTrace::current();
        let scanner = thread::Builder::new()
            .name("p4rust-local-enumerator".into())
            .spawn(move || {
                #[cfg(feature = "reconcile-trace")]
                let _attachment = super::trace::ReconcileTrace::attach(trace)
                    .context("Failed to attach local enumerator trace")?;
                reconcile_span!("local_scan_lifetime");
                let started = time::Instant::now();
                let result =
                    scanner::Scanner::run(root, config, workers, hashes, ignore, &shared, &control)
                        .context("Failed to enumerate local reconcile files");
                if let Err(error) = result {
                    errors.send(error).map_err(|error| {
                        crate::Error::new(format!("Failed to report local scan error: {error}"))
                    })?;
                    control.cancel();
                }
                shared
                    .finish_scan(started.elapsed(), &control)
                    .context("Failed to finish local path enumeration")
            })
            .context("Failed to spawn local reconcile enumeration")?;
        Ok(Some(Self {
            tables,
            scanner: Some(scanner),
            failure,
        }))
    }

    // Pair a server request exclusively on the Rust fetch owner.
    pub(super) fn server(&mut self, work: Work) -> Result<()> {
        self.tables
            .server(work)
            .context("Failed to pair server reconcile record")
    }
    // Pair an arriving local batch without polling scanner state.
    pub(super) fn consume(&mut self, local: Local) -> Result<()> {
        self.tables
            .consume(local)
            .context("Failed to consume scanner event")
    }
    // Finalize unmatched paths when the last scanner producer completes.
    pub(super) fn finish_scan(&mut self) -> Result<()> {
        self.tables
            .finish_scan()
            .context("Failed to finalize scanner event")
    }
    // Transfer one matched pair without allocating a poll result vector.
    pub(super) fn ready(&mut self) -> Option<Work> {
        self.tables.ready()
    }
    // Observe local completion without delaying native reply flushing.
    pub(super) fn scanning(&self) -> bool {
        self.tables.scanning()
    }

    // Preserve canonical traversal when filenames cannot be represented by Rust strings.
    pub(super) fn enabled(&self) -> bool {
        !self
            .tables
            .shared
            .fallback
            .load(sync::atomic::Ordering::Acquire)
    }
    // Retain canonical traversal when an SDK callback uses opaque filenames.
    pub(super) fn use_fallback(&self) {
        self.tables.shared.use_fallback();
    }

    // Surface enumeration errors before generic cancellation hides their origin.
    pub(super) fn check(&self) -> Result<()> {
        match self.failure.try_recv() {
            Ok(error) => Err(error).context("Failed local reconcile pipeline"),
            Err(sync::mpsc::TryRecvError::Empty | sync::mpsc::TryRecvError::Disconnected) => Ok(()),
        }
    }

    // Feed canonical results into digest table B and operation table C.
    pub(super) fn completed(
        &mut self,
        request: &super::super::ReconcileRequest,
        reply: &super::super::ReconcileReply,
        duration: time::Duration,
    ) -> Result<()> {
        self.tables
            .completed(request, reply, duration)
            .context("Failed to retain reconcile comparison")
    }

    // Remove unmatched local paths only after local enumeration and tracked comparisons finish.
    pub(super) fn paths(&mut self, directory: &[u8]) -> Result<Vec<(String, local::Snapshot)>> {
        self.tables
            .paths(directory)
            .context("Failed to collect unmatched local files")
    }

    // Snapshot fetch-owned counters without copying path or result tables.
    pub(super) fn statistics(&self) -> Result<Statistics> {
        Ok(self.tables.statistics())
    }
    // Reuse the shared directory and path registry for native task construction.
    pub(super) fn snapshot(&self, path: &str) -> Result<local::Snapshot> {
        self.tables
            .shared
            .metadata
            .snapshot_for(path)
            .context("Failed to obtain native metadata snapshot")
    }

    // Stop publishers before disconnecting their shared event pipe.
    pub(super) fn stop(&self) {
        self.tables
            .shared
            .stop
            .store(true, sync::atomic::Ordering::Release);
    }
    // Join all scanner threads before releasing requests that borrow native command state.
    pub(super) fn close(&mut self) -> Result<()> {
        self.tables
            .shared
            .stop
            .store(true, sync::atomic::Ordering::Release);
        if let Some(scanner) = self.scanner.take() {
            scanner
                .join()
                .map_err(|_| crate::Error::new("Failed to join local scan: thread panicked"))?
                .context("Failed to finish local enumeration")?;
        }
        self.tables.finish_results();
        self.check()
            .context("Failed to close local reconcile pipeline")
    }
    // Retain final SDK records in result table C after protocol classification.
    pub(super) fn output(&mut self, record: crate::Record) -> Result<()> {
        if !self.enabled() {
            return Ok(());
        }
        self.tables.output(record);
        Ok(())
    }
}
