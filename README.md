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

Binding code is licensed under MIT; resource crates retain the Perforce and OpenSSL licenses.

## Maintainers

[xtask](xtask/README.md) builds native resources and packages all six targets. GitHub Actions runs builds and Clippy, then releases one ZIP containing the Rust crate and six resource crates.
