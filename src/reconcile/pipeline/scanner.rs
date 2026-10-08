use super::{local::Agent, tables::shared::Shared};
use crate::{Config, Result, ResultExt, control::Control};
use std::{collections::VecDeque, path::PathBuf, sync, thread};

struct State {
    pending: VecDeque<PathBuf>,
    active: usize,
    failed: bool,
}
pub(super) struct Scanner {
    state: sync::Mutex<State>,
    changed: sync::Condvar,
}

impl Scanner {
    // Run a work-stealing directory queue with no lock held during disk access.
    pub(super) fn run(
        root: PathBuf,
        config: Config,
        workers: usize,
        hashes: bool,
        ignore: bool,
        tables: &sync::Arc<Shared>,
        control: &Control,
    ) -> Result<()> {
        let scanner = Self {
            state: sync::Mutex::new(State {
                pending: VecDeque::from([root]),
                active: 0,
                failed: false,
            }),
            changed: sync::Condvar::new(),
        };
        thread::scope(|scope| {
            let mut jobs = Vec::new();
            for index in 0..workers {
                #[cfg(feature = "reconcile-trace")]
                let trace = super::super::trace::ReconcileTrace::current();
                let scanner = &scanner;
                let config = &config;
                jobs.push(
                    thread::Builder::new()
                        .name(format!("p4rust-local-scan-{index}"))
                        .spawn_scoped(scope, move || {
                            #[cfg(feature = "reconcile-trace")]
                            let _attachment = super::super::trace::ReconcileTrace::attach(trace)
                                .context("Failed to attach local scanner trace")?;
                            reconcile_span!("scan_worker");
                            let result = scanner
                                .work(config, hashes, ignore, tables, control)
                                .context("Failed local scan worker");
                            if result.is_err() {
                                control.cancel();
                                scanner.changed.notify_all();
                            }
                            result
                        })
                        .context("Failed to spawn named local scan worker")?,
                );
            }
            let mut failure = None;
            for job in jobs {
                let result = job
                    .join()
                    .map_err(|_| crate::Error::new("Failed to join local scanner: panicked"))
                    .and_then(|result| result.context("Failed to finish local scan worker"));
                if let Err(error) = result {
                    failure.get_or_insert(error);
                }
            }
            failure.map_or(Ok(()), Err)
        })
    }

    // Take one directory or wait until active workers discover additional children.
    fn next(&self, control: &Control) -> Result<Option<PathBuf>> {
        reconcile_span!("scan_queue_wait");
        let mut state = self
            .state
            .lock()
            .map_err(|_| crate::Error::new("Failed to lock scan queue"))?;
        loop {
            if state.failed {
                return Ok(None);
            }
            if let Some(error) = control.error() {
                return Err(error).context("Failed to advance directory scan");
            }
            if let Some(path) = state.pending.pop_front() {
                state.active += 1;
                return Ok(Some(path));
            }
            if state.active == 0 {
                return Ok(None);
            }
            state = self
                .changed
                .wait_timeout(state, std::time::Duration::from_millis(50))
                .map_err(|_| crate::Error::new("Failed to wait for scan queue"))?
                .0;
        }
    }

    // Scan files on this worker and publish children after the directory lock is released.
    fn work(
        &self,
        config: &Config,
        hashes: bool,
        ignore: bool,
        tables: &Shared,
        control: &Control,
    ) -> Result<()> {
        let agent = Agent::new(config, control, ignore, &tables.stop)
            .context("Failed to create local scan agent")?;
        while !tables.stop.load(sync::atomic::Ordering::Acquire)
            && let Some(path) = self.next(control).context("Failed to get scan directory")?
        {
            Self::probes(&agent, hashes, tables, control)
                .context("Failed to process targeted local probes")?;
            let result = self
                .directory(&path, &agent, hashes, tables, control)
                .context("Failed to inspect local directory");
            let mut state = self
                .state
                .lock()
                .map_err(|_| crate::Error::new("Failed to finish scan directory"))?;
            state.active -= 1;
            match result {
                Ok(children) => state.pending.extend(children),
                Err(error) => {
                    state.failed = true;
                    self.changed.notify_all();
                    return Err(error);
                }
            }
            self.changed.notify_all();
        }
        Ok(())
    }

    // Service SDK flow-control requests ahead of unrelated local enumeration work.
    fn probes(agent: &Agent<'_>, hashes: bool, tables: &Shared, control: &Control) -> Result<()> {
        reconcile_span!("targeted_probes");
        while let Some(path) = tables
            .probe()
            .context("Failed to select targeted local path")?
        {
            let snapshot = tables
                .metadata
                .inspect(&path, agent, hashes)
                .context("Failed to probe server-listed file")?;
            tables
                .probed(path, snapshot, control)
                .context("Failed to publish targeted local record")?;
        }
        Ok(())
    }

    // Keep directory symlinks as files and retain only paths accepted by the SDK ignore policy.
    fn directory(
        &self,
        directory: &std::path::Path,
        agent: &Agent,
        hashes: bool,
        tables: &Shared,
        control: &Control,
    ) -> Result<Vec<PathBuf>> {
        reconcile_span!("scan_directory");
        let Some(path) = directory.to_str() else {
            tables.use_fallback();
            return Ok(Vec::new());
        };
        if agent
            .inspect(path, true, false)
            .context("Failed to check directory ignore rules")?
            .is_none()
        {
            return Ok(Vec::new());
        }
        tables.directory();
        let entries = tables
            .metadata
            .directory(directory)
            .with_context(|| format!("Failed to enumerate {}", directory.display()))?;
        let mut children = Vec::new();
        let mut batch = Vec::with_capacity(64);
        for path in entries
            .entries()
            .context("Failed to consume shared directory entries")?
        {
            if tables.stop.load(sync::atomic::Ordering::Acquire) {
                break;
            }
            Self::probes(agent, hashes, tables, control)
                .context("Failed to advance server-prioritized probes")?;
            let Some(spelling) = path.to_str() else {
                tables.use_fallback();
                break;
            };
            let stat = tables
                .metadata
                .basic(spelling)
                .context("Failed to classify cached entry")?
                .stat;
            if stat & 4 != 0 && stat & 8 == 0 {
                children.push(path.clone());
                continue;
            }
            if stat & 16 != 0 && stat & 8 == 0 {
                continue;
            }
            let Some(path) = path.to_str() else {
                tables.use_fallback();
                break;
            };
            if agent
                .accepted(path, false)
                .context("Failed to filter cached file")?
            {
                let snapshot = tables
                    .metadata
                    .inspect(path, agent, hashes)
                    .context("Failed to reuse scanned snapshot")?;
                batch.push((path.to_owned(), snapshot));
                if batch.len() == 64 {
                    tables
                        .publish(&mut batch, control)
                        .context("Failed to publish scanned local records")?;
                }
            }
        }
        tables
            .publish(&mut batch, control)
            .context("Failed to flush directory batch")?;
        Ok(children)
    }
}
