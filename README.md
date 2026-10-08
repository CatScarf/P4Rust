# P4Rust

Safe Rust bindings to the Perforce 2026.1 C++ API. Precompiled resources support Windows MSVC, Linux GNU, and macOS on x64 and ARM64. Consumers compile only Rust.

## crates.io

| Crate | Link |
| --- | --- |
| p4rust | [crates.io/crates/p4rust](https://crates.io/crates/p4rust) |

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

Use `FastReconcile` for one server connection with parallel local scanning, canonical comparisons, and move matching. Set `Config.charset` to `utf8` for the concurrent path pipeline on Unicode servers; other encodings retain SDK traversal.

The concurrent pipeline uses the cross-platform `walkdir` walker and shares one metadata snapshot per path for each command, including native comparisons.

```rust
let events = client
    .command("reconcile")
    .args(["-n", "-m", "D:/workspace/..."])
    .reconcile_handler(
        p4rust::FastReconcile::new()
            .metadata_workers(8)
            .digest_workers(4)
            .move_workers(4),
    )
    .run()
    .context("Failed to start reconcile")?;

for event in events {
    println!("{:#?}", event.context("Failed to reconcile")?);
}
```

`-n` previews changes; remove it to apply them. `-m` uses the SDK timestamp shortcut; omit it to compare contents. Add `-M` for SDK move detection. A sharded path table pairs server and local records immediately; a digest table matches Add/Delete candidates, and the result table retains final SDK classifications. SDK mappings, ignore rules, file types, text conversions, and similarity matching still apply. `queue_capacity()` and `queue_bytes()` bound pending RPC tasks and metadata. Custom `ReconcileHandler` implementations inspect owned requests and use `request.execute()` for SDK computation; `progress()` receives pipeline counters.

Binding code is licensed under MIT; resource crates retain the Perforce and OpenSSL licenses.

## Maintainers

[xtask](xtask/README.md) builds native resources and packages all six targets. GitHub Actions runs builds and Clippy, publishes changed crates.io versions, then releases one ZIP containing the Rust crate and six resource crates.
