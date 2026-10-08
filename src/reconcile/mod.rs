pub(crate) mod pipeline;
mod pool;
pub(crate) mod runtime;
mod task;
use crate::{Result, ResultExt};
pub use pipeline::Statistics as ReconcileStatistics;
pub use task::{ReconcileKind, ReconcileReply, ReconcileRequest, ReconcileStatus};

/// An owned strategy for local SDK work; networking stays on the command thread.
pub trait ReconcileHandler: Send + Sync {
    /// Enable concurrent local enumeration and server-to-local path pairing.
    fn pipeline(&self) -> bool {
        false
    }
    /// Observe command-local pipeline counters without retaining borrowed native data.
    fn progress(&self, _: &ReconcileStatistics) -> Result<()> {
        Ok(())
    }
    /// Select the number of independent workers for a local work category.
    fn workers(&self, kind: ReconcileKind) -> usize;
    /// Bound the number of queued, running, and completed requests together.
    fn queue_capacity(&self) -> usize {
        128
    }
    /// Bound retained request metadata independently of the task count.
    fn queue_bytes(&self) -> usize {
        16 * 1024 * 1024
    }
    /// Process scoped local work without retaining its native objects after the callback.
    fn handle(&self, request: &mut ReconcileRequest) -> Result<ReconcileReply>;
}

/// Parallel SDK comparisons and traversal with bounded command-local queues.
#[derive(Clone, Debug)]
pub struct FastReconcile {
    metadata: usize,
    digest: usize,
    moves: usize,
    capacity: usize,
    bytes: usize,
}

impl FastReconcile {
    /// Configure conservative disk parallelism without inspecting global environment state.
    pub fn new() -> Self {
        Self {
            metadata: 8,
            digest: 4,
            moves: 4,
            capacity: 128,
            bytes: 16 * 1024 * 1024,
        }
    }
    /// Set all three local worker limits to the same value.
    pub fn workers(mut self, count: usize) -> Self {
        self.metadata = count;
        self.digest = count;
        self.moves = count;
        self
    }
    /// Limit concurrent directory enumeration and stat requests.
    pub fn metadata_workers(mut self, count: usize) -> Self {
        self.metadata = count;
        self
    }
    /// Limit concurrent file classification and canonical digest requests.
    pub fn digest_workers(mut self, count: usize) -> Self {
        self.digest = count;
        self
    }
    /// Limit concurrent SDK move content comparisons.
    pub fn move_workers(mut self, count: usize) -> Self {
        self.moves = count;
        self
    }
    /// Bound retained work across all stages of this command.
    pub fn queue_capacity(mut self, count: usize) -> Self {
        self.capacity = count;
        self
    }
    /// Bound the byte budget used by owned request metadata.
    pub fn queue_bytes(mut self, bytes: usize) -> Self {
        self.bytes = bytes;
        self
    }
}

impl Default for FastReconcile {
    /// Construct the default bounded parallel engine.
    fn default() -> Self {
        Self::new()
    }
}

impl ReconcileHandler for FastReconcile {
    /// Pair the single server enumerator with parallel local scanning and comparisons.
    fn pipeline(&self) -> bool {
        true
    }
    /// Route directory, digest, and move work to separate worker limits.
    fn workers(&self, kind: ReconcileKind) -> usize {
        match kind {
            ReconcileKind::Directory => self.metadata,
            ReconcileKind::Move => self.moves,
            ReconcileKind::TrackedFile
            | ReconcileKind::ExactMatch
            | ReconcileKind::UntrackedFile => self.digest,
        }
    }
    /// Return the configured total request capacity.
    fn queue_capacity(&self) -> usize {
        self.capacity
    }
    /// Return the configured metadata byte budget.
    fn queue_bytes(&self) -> usize {
        self.bytes
    }
    /// Reuse the SDK's file conversion, hashing, and matching behavior.
    fn handle(&self, request: &mut ReconcileRequest) -> Result<ReconcileReply> {
        request
            .execute()
            .context("Failed to execute fast reconcile request")
    }
}
