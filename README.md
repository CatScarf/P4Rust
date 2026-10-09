# P4Rust

Safe Rust bindings to the Perforce 2026.1 C++ API. Precompiled resources support Windows MSVC, Linux GNU, and macOS on x64 and ARM64. Consumers compile only Rust.

## Published Packages

| Language | Link | Compatibility |
| --- | --- | --- |
| Rust | [crates.io](https://crates.io/crates/p4rust) | Windows MSVC, Linux GNU (glibc 2.28+), macOS 12+; x64 and ARM64. Consumers compile only Rust. |
| Python | [PyPI](https://pypi.org/project/p4rust/) | CPython 3.9+ (`cp39-abi3`), including free-threaded CPython 3.15+ (`cp315-abi3.abi3t`); Windows, Linux (glibc 2.28+), macOS 12+; x64 and ARM64. |

## Rust

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

`run()` streams events; `collect_output()` collects results. Configure input, timeout, and cancellation on the builder.

Use `FastReconcile` for parallel local processing.

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

`-n` previews changes; `-m` checks timestamps before contents; `-M` enables move detection.

## Python

Install [p4rust from PyPI](https://pypi.org/project/p4rust/) with `pip install p4rust`. Wheels support ordinary CPython 3.9+ and free-threaded CPython 3.15+ on the same six platforms. Public APIs include complete typing and preserve native message codes, bytes, progress, and completion status. The Python ZIP is also available from [GitHub Releases](https://github.com/CatScarf/P4Rust/releases).

```python
from p4rust import Client, Config, RecordEvent

client = Client(Config("localhost:1666", "user", "workspace"))
events = (
    client
    .command("files")
    .args(["D:/workspace/..."])
    .timeout(30.0)
    .run()
)
with events:
    for event in events:
        if isinstance(event, RecordEvent):
            print(event.record.fields())
```

Use `.input(form)`, `.cancellation(token)`, or `.reconcile_handler(FastReconcile())` before `.run()`. `collect_output()` collects the remaining stream; `close()` cancels a command.

## Maintainers

[xtask](xtask/README.md) builds native resources and packages all six targets. GitHub Actions runs builds, Clippy, and Python typing checks, publishes changed crates.io versions and Python wheels to PyPI, and releases separate Rust and Python ZIPs.
