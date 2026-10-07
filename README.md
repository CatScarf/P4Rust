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

`client.command(name).args(args).input(form).timeout(duration).cancellation(&token).run()` is the execution path for queries and parallel transfers. Options are optional. `run()` starts an owned background worker and returns `CommandStream`, an `Iterator<Item = Result<Event>>` delivering text, binary data, complete tagged records, warnings, SDK progress, and `Completed`. Failures arrive as stream errors. `collect_output()` collects the remaining stream into `Output`.

Clients and callbacks are coordinated internally. The queue holds at most 64 events and applies cancellable backpressure. Text and binary chunks are bounded; tagged records and warnings are limited to 1 MiB. Text events combine split UTF-8 sequences and retain original bytes. Dropping a stream cancels its command without cancelling other commands sharing the external token.

Timeouts cover setup, output delivery, and execution; errors retain `TimedOut` or `Interrupted` causes. SDK operations that cannot poll finish cleanup in the owned worker. Commands cancelled during setup are never dispatched.

Windows requires the shared MSVC runtime; `crt-static` is unsupported. Version 0.1.0 has not been published to crates.io.

## Maintainers

`xtask` verifies the pinned expanded Perforce and Jam sources, builds native libraries, and packages the Rust crate with a separate resource crate for each platform. Generated libraries remain outside Git. See [maintainer tasks](xtask/README.md).

GitHub Actions builds all six platforms and runs strict Clippy. Each `v<version>` release contains one `p4rust-<version>.zip` with the common Rust crate and six resource crates, each below 10 MB. The version comes from `Cargo.toml`; rebuilding it replaces the ZIP and updates the tag.