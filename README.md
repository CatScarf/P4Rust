# P4Rust

Safe Rust bindings to the official Perforce C++ API. Supports Windows x64 MSVC with precompiled native libraries and no Rust dependencies.

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

`run_with_input` accepts form or password input. Output contains text, binary bytes, ordered tagged records, and warnings. Errors retain operation context and their original cause.

The package requires the standard MSVC runtime; `crt-static` is unsupported. Version 0.1.0 has not yet been published to crates.io.

## Development

```text
cargo xtask prepare
cargo xtask build
cargo xtask package
```

SDK libraries are stored as `.tar.zst` archives. `xtask` verifies and extracts them, builds the native release locally, and bundles it into the distributable crate. See [xtask](xtask/README.md) for maintainer commands.
