# Maintainer tasks

- `cargo xtask prepare`: verify and extract pinned expanded Perforce and Jam sources.
- `cargo xtask build [Cargo options]`: build native dependencies and Rust.
- `cargo xtask check`: build native dependencies and run strict workspace Clippy.
- `cargo xtask package`: build dependencies, verify workspace packages offline, and stage the common Rust crate and selected resource crate.
- `cargo xtask ci`: install runner dependencies, build, run Clippy, and package.
- `cargo xtask publish`: combine six platform artifacts into one ZIP and create or replace the versioned GitHub release.
- `cargo xtask native [--openssl-lib-dir <cache>]`: produce native resources, optionally reusing a compatible OpenSSL source and static-library cache.

Sources and their SHA-256 inventory live under `sdk`. Build copies of sources, compiler outputs, and dependency caches live under ignored `temp`. Generated libraries and licenses are installed under `resources/<target>` and included only in distributable resource crates. Consumer build scripts emit linker metadata and never invoke native tools.

Set `P4RUST_TARGET` to a supported resource directory name; the default is the host. CI uses native runners for six targets. Keep the public and resource crate versions synchronized. Verification runs through GitHub Actions, with `cargo clippy --workspace --all-targets --all-features -- -D warnings` and package builds; no tests are included.

Production uses the vendor Jam rules with extensions disabled, a target-compatible compiler, Perl, LLVM tools, and NASM on x64. `P4RUST_OBJCOPY`, `P4RUST_STRIP`, and `P4RUST_AR` override archive tools; `P4RUST_JOM` enables parallel MSVC OpenSSL builds. Native pruning retains the complete transitive archive-member closure of all ABI exports.

All external commands use one executor and print `> <command>` before execution. Release publishing requires `GH_TOKEN` and mutable GitHub Releases.