# Perforce SDK

Perforce 2025.2, build 3086021, with OpenSSL 3 compatibility. The SDK is a maintainer input; published P4Rust crates contain the native libraries they need.

- `include/p4/`: 15 shared headers.
- `archives/<target>.tar.zst`: four SDK libraries per target.
- `manifest.json`: upstream download provenance and original library checksums.
- `archives.json`: compressed archive and extracted-file checksums.
- `NOTICE.md`: vendor notices.

`cargo xtask prepare` extracts the current host target, or the target selected by `P4RUST_TARGET`. Use `prepare --all` to extract all retained targets. Extracted files under `lib/` are ignored by Git.

| Target | Vendor platform |
| --- | --- |
| `x86_64-pc-windows-msvc` | `bin.ntx64` |
| `aarch64-pc-windows-msvc` | `bin.ntarm64` |
| `x86_64-pc-windows-gnu` | `bin.mingw64x64` |
| `x86_64-unknown-linux-gnu` | `bin.linux26x86_64` |
| `aarch64-unknown-linux-gnu` | `bin.linux26aarch64` |
| `x86_64-apple-darwin` | `bin.macosx12x86_64` |
| `aarch64-apple-darwin` | `bin.macosx12arm64` |

GitHub Actions builds all seven targets. No 32-bit targets are retained. The SDK libraries are `client`, `p4script_cstub`, `rpc`, and `supp`; scripting runtimes and aggregate archives are excluded.
