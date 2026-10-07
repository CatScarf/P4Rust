use crate::error::{Result, ResultExt};
mod archive;
mod automation;
#[path = "../../src/error.rs"]
mod error;
mod openssl;
mod platform;
mod producer;
mod prune;
mod release;
use producer::Producer;

// Execute the explicit maintainer workflow for the selected SDK platform.
fn main() -> Result<()> {
    automation::Task::run().context("Failed to execute xtask")
}
