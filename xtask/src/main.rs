use crate::error::{Result, ResultExt};
mod archive;
#[path = "../../src/error.rs"]
mod error;
mod openssl;
mod platform;
mod producer;
mod prune;
mod release;
mod task;
use producer::Producer;

// Execute the explicit maintainer workflow for the selected SDK platform.
fn main() -> Result<()> {
    task::Task::run().context("Failed to execute xtask")
}
