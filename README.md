# P4Rust

Safe Rust bindings to the official Perforce C++ API. Precompiled native libraries and no Rust dependencies. Release packages cover Windows x64 (MSVC), Windows ARM64, Linux x64/ARM64, and macOS x64/ARM64.

## Usage

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
    let output = client.run("info", &[]).context("Failed to query server")?;
    for record in output.records {
        for (key, value) in record {
            println!("{key}: {value}");
        }
    }
    Ok(())
}
```

`Client` can be shared across threads without an application lock; each command opens an independent connection. `run_with_input` accepts form or password input. Output contains text, binary bytes, ordered tagged records, and warnings. `Output.raw` preserves the original text, field, and warning bytes; display strings replace invalid UTF-8 after text fragments have been combined. Errors retain operation context and their original cause.

`run_with_options` and `run_with_input_and_options` accept `RunOptions { timeout, cancellation }`. Timeouts use `Duration`; cancelling a cloned `CancellationToken` interrupts its commands. The error chain contains `std::io::ErrorKind::TimedOut` or `Interrupted`.

Caller deadlines include connection setup. Native commands stop through the SDK's cooperative interrupt hook. DNS, connection setup, or local SDK work that cannot poll may finish cleanup in an owned background worker; a command cancelled during setup is never dispatched.

Windows MSVC packages require the shared MSVC runtime; `crt-static` is unsupported on those targets. Version 0.1.0 has not yet been published to crates.io.

## Development

```text
cargo xtask prepare
cargo xtask build
cargo xtask package
```

SDK libraries are stored as `.tar.zst` archives. `xtask` verifies and extracts them, builds the native release locally, and bundles it into the distributable crate. See [xtask](xtask/README.md) for maintainer commands.

## Releases

GitHub Actions builds and packages all six targets on pushes to `main` or manual runs. Pull requests build the same packages without publishing. The version comes from `Cargo.toml`.

Each release `v<version>` contains one `p4rust-<version>.zip`, with a target directory containing its precompiled `.crate`. All six builds must pass, and each crate must stay below 10 MB. Rebuilding the same version replaces the ZIP and updates its tag to the current commit; increasing the version creates a new release.
