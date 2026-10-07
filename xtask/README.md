# Maintainer tasks

- `cargo xtask prepare`: verify and extract the selected SDK target.
- `cargo xtask prepare --all`: extract all retained 64-bit SDK targets.
- `cargo xtask build [Cargo options]`: prepare the SDK, compile native libraries, and build Rust.
- `cargo xtask check`: prepare libraries and run strict workspace Clippy.
- `cargo xtask package`: prepare libraries and verify the public crate offline.
- `cargo xtask native [--openssl-lib-dir <cache>]`: rebuild the native release with the maintainer toolchain, without tracking generated libraries.
- `cargo xtask archive`: refresh archives after intentional SDK changes; extract all targets before editing inputs.

Only SDK `.tar.zst` library archives and their SHA-256 inventory are tracked. Extracted SDKs and generated native libraries under `sdk/lib` and `native/lib` are ignored. Archive checksums and every extracted file are checked before use; unchanged valid files are reused. Native libraries are built locally and included only in packaged crates. Headers and vendor notices remain readable in Git.

The public package contains extracted native libraries so downstream builds need no xtask, decompression tools, or C++ compilation. Its Rust build script only emits linker metadata. Set `P4RUST_TARGET` to one of the seven targets in [the SDK table](../sdk/README.md); the default is the current host. CI uses a native runner for each target.

Native production requires a target-compatible C++ compiler, Perl, PowerShell 7, LLVM archive tools, and NASM on x64. MSVC uses nmake; Linux, macOS, and MinGW use make. Windows GNU uses MSYS2 MINGW64 (MSVCRT) and `P4RUST_RUNTIME_LICENSE_DIR` pointing to its `share/licenses` directory. `P4RUST_OBJCOPY`, `P4RUST_STRIP`, and `P4RUST_AR` override LLVM tools; `P4RUST_JOM` enables parallel MSVC OpenSSL builds. OpenSSL production is cached locally; `native --openssl-lib-dir <cache>` accepts an explicit static-library cache.

`bundle-release.ps1` merges seven target crates into the versioned release ZIP and rejects missing, oversized, or mismatched packages. The release workflow reads the public crate version automatically and replaces same-version assets. GitHub Releases must remain mutable to allow replacements.
