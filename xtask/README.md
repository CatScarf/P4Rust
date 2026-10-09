# Maintainer tasks

- `cargo xtask prepare`: verify and copy maintained Perforce and Jam sources.
- `cargo xtask build [Cargo options]`: build native dependencies and Rust.
- `cargo xtask check`: build native dependencies and run strict workspace Clippy.
- `cargo xtask package`: build dependencies, verify workspace packages offline, and stage the common Rust crate and selected resource crate.
- `cargo xtask ci`: install runner dependencies, build, run Clippy and strict Python typing checks, and package crates and two Python stable ABI wheels.
- `cargo xtask ci-cache`: prepare runner tools and export platform-specific OpenSSL and SDK keys for GitHub Actions.
- `cargo xtask publish-plan`: compare downloaded CI artifacts with crates.io and print the versions requiring publication.
- `cargo xtask publish`: publish missing resource versions followed by the public crate, then create or replace the versioned GitHub release with separate Rust and Python ZIPs.
- `cargo xtask native [--openssl-lib-dir <cache>]`: produce native resources, optionally reusing a compatible OpenSSL source and static-library cache.

Sources and their SHA-256 inventory live under `sdk`. Build copies of sources, compiler outputs, and dependency caches live under ignored `temp`. Generated libraries and licenses are installed under `resources/<target>` and included only in distributable resource crates. Consumer build scripts emit linker metadata and never invoke native tools.

Set `P4RUST_TARGET` to a supported resource directory name; the default is the host. CI uses native runners for six targets. Versions come from each package's `Cargo.toml`; public and resource versions are independent. Verification runs through GitHub Actions, with `cargo clippy --workspace --all-targets --all-features -- -D warnings` and package builds; no tests are included.

Windows x64 and ARM64 Rust builds use the toolchain's bundled `rust-lld` through `.cargo/config.toml`, including xtask and CI. Consumers should set `linker = "rust-lld"` in their own matching `[target.x86_64-pc-windows-msvc]` or `[target.aarch64-pc-windows-msvc]` table; Cargo does not inherit dependency linker configuration.

Production uses the vendor Jam rules with extensions disabled, a target-compatible compiler, Perl, LLVM tools, and NASM on x64. `P4RUST_OBJCOPY`, `P4RUST_STRIP`, and `P4RUST_AR` override archive tools; `P4RUST_JOM` enables parallel MSVC OpenSSL builds. Native pruning retains the complete transitive archive-member closure of all ABI exports.

All external commands use one executor and print `> <command>` before execution. Release publishing requires `GH_TOKEN`; new crates.io versions also require the repository's `CARGO_REGISTRY_TOKEN` Actions secret. Authorization is passed through stdin, never command arguments or files. Uploads reuse the checked CI artifacts and honor registry throttling through curl's `Retry-After` handling.

Each resource includes a generated `NATIVE-INPUTS.sha256` for native sources, build policy, locked native tool dependencies, and its Rust linker code. CI reuses the published resource when these inputs match and requires a resource version bump when they change. Publication compares extracted contents, excluding Cargo provenance and generated lock files; already published identical versions are skipped. The entire seven-package plan is validated before uploads. Resources are published serially and must appear in the sparse index before publishing the public crate. Reruns resume partial publication and keep ZIP entries identical to the crates.io downloads.

GitHub Actions stores OpenSSL libraries and headers separately from the three Perforce SDK libraries. `xtask` keys include the actual compiler and system SDK, native build policy, and pinned source identities; SDK keys also include the OpenSSL policy. Every cached file is checked against its SHA-256 before reuse. Missing or invalid payloads rebuild; Rust and bridge changes preserve these keys. No build objects or complete source trees are cached, and Rust compilation, Clippy, pruning, and packaging still run after a cache hit.

Linux production runs inside native-architecture manylinux_2_28 containers, including native dependencies and crate validation. Python wheels use PyO3 and pinned maturin through managed uv environments; ordinary CPython targets `cp39-abi3`, and free-threaded CPython targets the 3.15 stable ABI. The release validates twelve wheels with typing markers and license notices. Pyright runs in strict mode with Python 3.9 semantics and verifies the packaged distribution's public type completeness. No PyPI upload is performed.
