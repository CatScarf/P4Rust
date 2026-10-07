# P4Rust

Safe Rust bindings to the Perforce 2026.1 C++ API. Precompiled resources support Windows MSVC, Linux GNU, and macOS on x64 and ARM64. Consumers compile only Rust.

```toml
[dependencies]
p4rust = "0.1"
```

```rust
use p4rust::{Client, Config, Event, Result, ResultExt};

// Stream file records, progress, and command completion.
fn main() -> Result<()> {
    let client = Client::new(Config::new("localhost:1666", "user", "workspace"))
        .context("Failed to create client")?;
    let events = client
        .command("sync")
        .args(["D:/workspace/file.txt"])
        .run()
        .context("Failed to start sync")?;

    for event in events {
        match event.context("Failed to sync file")? {
            Event::Record(record) => {
                for (key, value) in record.fields() {
                    println!("{key}: {value}");
                }
            }
            Event::Progress(progress) => {
                println!(
                    "{:?}: {}/{}",
                    progress.kind, progress.current, progress.total
                );
            }
            Event::Message(message) | Event::HandleError(message) => {
                println!("{}", message.text);
            }
            event => println!("{event:#?}"),
        }
    }
    Ok(())
}
```

`run()` returns `CommandStream`, an `Iterator<Item = Result<Event>>`. Add `.input(form)`, `.timeout(duration)`, or `.cancellation(&token)` before `run()` as needed. Use `collect_output()` to gather the stream into one `Output`.

Use `FastReconcile` to schedule SDK directory scans, canonical file comparisons, and move matching in separate Rust worker pools:

```rust
let events = client
    .command("reconcile")
    .args(["-n", "-m", "D:/workspace/..."])
    .reconcile_handler(p4rust::FastReconcile::new()
        .metadata_workers(8)
        .digest_workers(4)
        .move_workers(4))
    .run()
    .context("Failed to start reconcile")?;

for event in events {
    println!("{:#?}", event.context("Failed to reconcile")?);
}
```

`-n` previews changes; remove it to apply them. `-m` retains the SDK timestamp shortcut; omit it to compare contents. SDK mappings, ignore rules, file types, and text conversions still apply. `queue_capacity()` bounds pending tasks and `queue_bytes()` bounds copied request metadata. Custom `ReconcileHandler` implementations inspect scoped requests with owned RPC metadata and use `request.execute()` for SDK computation, optionally overriding a tracked-file decision with `reply.with_status(...)`. Summary output (`status -s`) uses the SDK's short traversal.

Binding code is licensed under MIT; resource crates retain the Perforce and OpenSSL licenses.

## Maintainers

[xtask](xtask/README.md) builds native resources and packages all six targets. GitHub Actions runs builds and Clippy, publishes changed crates.io versions, then releases one ZIP containing the Rust crate and six resource crates.
