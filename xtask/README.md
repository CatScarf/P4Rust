# Maintainer tasks

- `cargo xtask prepare`: verify and extract Windows x64 SDK libraries.
- `cargo xtask prepare --all`: extract all retained 64-bit SDK targets.
- `cargo xtask build [Cargo options]`: prepare the SDK, compile native libraries, and build Rust.
- `cargo xtask check`: prepare libraries and run strict workspace Clippy.
- `cargo xtask package`: prepare libraries and verify the public crate offline.
- `cargo xtask verify`: test the public crate and run the offline external-consumer packaging checks.
- `cargo xtask native [--openssl-lib-dir <cache>]`: rebuild the native release with the maintainer toolchain, without tracking generated libraries.
- `cargo xtask archive`: refresh archives after intentional SDK changes; extract all targets before editing inputs.

Only SDK `.tar.zst` library archives and their SHA-256 inventory are tracked. Extracted SDKs and generated native libraries under `sdk/lib` and `native/lib` are ignored. Archive checksums and every extracted file are checked before use; unchanged valid files are reused. Native libraries are built locally and included only in packaged crates. Headers and vendor notices remain readable in Git.

The public package contains extracted native libraries so downstream builds need no xtask, decompression tools, or C++ compilation. Its Rust build script only emits linker metadata. SDK production supports Windows x64 MSVC; other retained 64-bit SDKs are inputs for future ports.

Native production requires MSVC, Perl, NASM, PowerShell 7, and LLVM objcopy. `P4RUST_OBJCOPY` selects objcopy; `P4RUST_JOM` enables parallel OpenSSL builds. OpenSSL production is cached locally; `native --openssl-lib-dir <cache>` accepts an explicit static-library cache.
