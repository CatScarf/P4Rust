# P4Rust

Safe Rust bindings to the Perforce 2026.1 C++ API. Six resource crates supply precompiled libraries for Windows MSVC, Linux GNU, and macOS on x64 and ARM64. Consumers compile only Rust.

```toml
[dependencies]
p4rust = "0.1"
```

```rust
use p4rust::{Client, Config, Result, ResultExt};

// Query server information with explicit connection settings.
fn main() -> Result<()> {
    let client = Client::new(Config::new("localhost:1666", "user", "workspace"))
        .context("Failed to create client")?;
    let output = client.command("info").run().context("Failed to start query")?.collect_output().context("Failed to query server")?;
    println!("{}", output.text);
    Ok(())
}
```

`client.command(name).args(args).input(form).timeout(duration).cancellation(&token).run()` is the execution path for queries and parallel transfers. Options are optional. `run()` starts an owned background worker and returns `CommandStream`, an `Iterator<Item = Result<Event>>`. `collect_output()` collects the remaining stream into `Output`.

SDK callbacks remain distinct: `Text`, `Info`, `Binary`, `Record`, `RecordPartial`, `Message`, `HandleError`, `OutputError`, and `Finished`. `Info.level` is the original level byte. Structured messages retain severity, generic code, every error ID and format, parameters, and exact SDK `Marshall2` bytes. `Progress` identifies `Description`, `Total`, `Update`, or `Done` and preserves signed counters and failure values.

`Completed(CommandStatus)` follows native cleanup on success or failure. Its `exit_code` is the bridge return code (0 for success, 1 for failure), and `error_count` is the SDK's server error count when available. The SDK does not launch a process or provide a separate process exit code. A failed completion is followed by `Err`; `Error::command_status()` retains the status through contextual wrapping. SDK `Finished` alone does not indicate success.

Clients and callbacks are coordinated internally. The queue holds at most 64 events with cancellable backpressure. Callback payloads and tagged records are limited to 1 MiB; progress descriptions to 16 KiB. Text events preserve original bytes and callback boundaries; `collect_output()` decodes the concatenated stream, handling split UTF-8 characters and retaining non-UTF-8 bytes. Dropping a stream cancels its command without cancelling other commands sharing the external token.

Timeouts cover setup, output delivery, and execution; errors retain `TimedOut` or `Interrupted` causes. SDK operations that cannot poll finish cleanup in the owned worker. Commands cancelled during setup are never dispatched.

Windows requires the shared MSVC runtime; `crt-static` is unsupported. Version 0.1.0 has not been published to crates.io.

## Maintainers

`xtask` verifies the pinned expanded Perforce and Jam sources, builds native libraries, and packages the Rust crate with a separate resource crate for each platform. Generated libraries remain outside Git. See [maintainer tasks](xtask/README.md).

GitHub Actions builds all six platforms and runs strict Clippy. Each `v<version>` release contains one `p4rust-<version>.zip` with the common Rust crate and six resource crates, each below 10 MB. The version comes from `Cargo.toml`; rebuilding it replaces the ZIP and updates the tag.
