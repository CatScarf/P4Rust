use super::Inner;
use std::{collections::BTreeMap, sync};

pub(super) struct Local {
    pub name: String,
    pub inner: sync::Weak<Inner>,
    pub stages: BTreeMap<&'static str, Stage>,
    stack: Vec<Frame>,
    started: std::time::Instant,
    resolution: u64,
}

#[derive(Default)]
pub(super) struct Stage {
    pub count: u64,
    pub first: u64,
    pub last: u64,
    pub inclusive: u64,
    pub exclusive: u64,
    pub bins: Vec<u64>,
    pub slow: Vec<(u64, u64)>,
}

struct Frame {
    stage: &'static str,
    started: u64,
    resumed: u64,
}

impl Local {
    // Allocate a thread-owned activity buffer sharing only the command clock.
    pub fn new(name: String, inner: sync::Arc<Inner>) -> Self {
        Self {
            name,
            started: inner.started,
            resolution: inner.resolution,
            inner: sync::Arc::downgrade(&inner),
            stages: BTreeMap::new(),
            stack: Vec::new(),
        }
    }

    // Read elapsed time without truncating a representable command duration.
    fn now(&self) -> u64 {
        match u64::try_from(self.started.elapsed().as_nanos()) {
            Ok(nanos) => nanos,
            Err(_) => {
                self.fail();
                u64::MAX
            }
        }
    }

    // Pause the parent before recording a nested timing frame.
    pub fn begin(&mut self, stage: &'static str) {
        let now = self.now();
        if let Some(parent) = self.stack.last() {
            self.account(parent.stage, parent.resumed, now);
        }
        self.stack.push(Frame {
            stage,
            started: now,
            resumed: now,
        });
    }

    // Replace only a leaf label after its native result identifies the executed branch.
    pub fn classify(&mut self, stage: &'static str) {
        if let Some(frame) = self
            .stack
            .last_mut()
            .filter(|frame| frame.started == frame.resumed)
        {
            frame.stage = stage;
        }
    }

    // Record exclusive segments and full span bounds without synchronous file output.
    pub fn end(&mut self) {
        let now = self.now();
        let Some(frame) = self.stack.pop() else {
            self.fail();
            return;
        };
        self.account(frame.stage, frame.resumed, now);
        let stage = self.stages.entry(frame.stage).or_default();
        if stage.count == 0 {
            stage.first = frame.started;
        }
        stage.first = stage.first.min(frame.started);
        stage.last = stage.last.max(now);
        stage.count += 1;
        stage.inclusive += now - frame.started;
        if now - frame.started >= 10_000_000 {
            stage.slow.push((frame.started, now));
            if stage.slow.len() > 64 {
                stage
                    .slow
                    .sort_unstable_by_key(|(start, end)| std::cmp::Reverse(end - start));
                stage.slow.truncate(64);
            }
        }
        if let Some(parent) = self.stack.last_mut() {
            parent.resumed = now;
        }
    }

    // Split actual exclusive execution across fixed clock bins for faithful occupancy plots.
    fn account(&mut self, stage: &'static str, mut start: u64, end: u64) {
        let resolution = self.resolution;
        let stage = self.stages.entry(stage).or_default();
        stage.exclusive += end - start;
        while start < end {
            let index = (start / resolution) as usize;
            if index >= 360_000 {
                if let Some(inner) = self.inner.upgrade() {
                    inner.failed.store(true, sync::atomic::Ordering::Relaxed);
                }
                return;
            }
            if stage.bins.len() <= index {
                stage.bins.resize(index + 1, 0);
            }
            let stop = end.min((index as u64 + 1) * resolution);
            stage.bins[index] += stop - start;
            start = stop;
        }
    }

    // Reject exports taken before all of this thread's scopes have closed.
    pub fn complete(&self) -> bool {
        self.stack.is_empty()
    }

    // Preserve capture errors without creating a reference cycle through worker buffers.
    pub fn fail(&self) {
        if let Some(inner) = self.inner.upgrade() {
            inner.failed.store(true, sync::atomic::Ordering::Relaxed);
        }
    }
}
