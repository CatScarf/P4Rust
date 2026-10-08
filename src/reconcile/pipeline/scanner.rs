use super::{local::Agent, tables::Tables};
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
        tables: &sync::Arc<Tables>,
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
            for _ in 0..workers {
                let scanner = &scanner;
                let config = &config;
                jobs.push(scope.spawn(move || {
                    let result = scanner
                        .work(config, hashes, ignore, tables, control)
                        .context("Failed local scan worker");
                    if result.is_err() {
                        control.cancel();
                        scanner.changed.notify_all();
                    }
                    result
                }));
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
        tables: &Tables,
        control: &Control,
    ) -> Result<()> {
        let agent = Agent::new(config, control, ignore, &tables.stop)
            .context("Failed to create local scan agent")?;
        while !tables.stop.load(sync::atomic::Ordering::Acquire)
            && let Some(path) = self.next(control).context("Failed to get scan directory")?
        {
            Self::probes(&agent, hashes, tables)
                .context("Failed to process targeted local probes")?;
            let result = self
                .directory(&path, &agent, hashes, tables)
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
    fn probes(agent: &Agent<'_>, hashes: bool, tables: &Tables) -> Result<()> {
        while let Some(path) = tables
            .probe()
            .context("Failed to select targeted local path")?
        {
            let snapshot = tables
                .metadata
                .inspect(&path, agent, hashes)
                .context("Failed to probe server-listed file")?;
            tables
                .probed(&path, snapshot)
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
        tables: &Tables,
    ) -> Result<Vec<PathBuf>> {
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
        for path in entries
            .entries()
            .context("Failed to consume shared directory entries")?
        {
            if tables.stop.load(sync::atomic::Ordering::Acquire) {
                break;
            }
            Self::probes(agent, hashes, tables)
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
                tables
                    .local(path, snapshot)
                    .context("Failed to pair scanned local record")?;
            }
        }
        Ok(children)
    }
}
