use crate::error::{Result, ResultExt};
mod automation;
#[path = "../../src/error.rs"]
// Native command status helpers are shared but unused by the maintainer binary.
#[allow(dead_code)]
mod error;
mod openssl;
mod platform;
mod producer;
mod prune;
mod release;
mod sdk;
use producer::Producer;

// Execute the explicit maintainer workflow for the selected SDK platform.
fn main() -> Result<()> {
    automation::Task::run().context("Failed to execute xtask")
}
