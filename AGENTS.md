# Project Rules

## Language

- All comments, documentation, and project rules throughout this project must be written in English.

## Rust

- Maintain six 64-bit targets: Windows MSVC, Linux GNU, and macOS, each on x64 and ARM64. Do not add 32-bit SDKs or native release packages.
- Build optimized native static libraries, strip debug symbols, disable unused OpenSSL features, and retain only archive members in the bridge's transitive symbol dependency closure.
- Preserve supported behavior and keep every compressed distributable `.crate` strictly below 10 MB (10,000,000 bytes).
- Validate compilation and `cargo clippy --workspace --all-targets --all-features -- -D warnings` through GitHub Actions. Do not build locally. Keep all test code and test steps absent.
- Version the public crate and each resource crate independently. Bump resource versions when their maintained native inputs change; unchanged resources reuse checksum-verified crates.io artifacts. Publish resources before the public crate, and reject changed contents under an already published version.
- Never use `unwrap`. Propagate unexpected errors and never discard them. Add operation-specific context at every propagation boundary, for example `Failed to connect: <inner error>`, so the error chain identifies each failed operation. Use `unwrap_or` only for expected fallback cases.
- When a single `use` imports more than five functions, variables, or types, import their parent module instead and access items through `::`.

## Code Structure

- Keep comments concise and necessary.
- Give every function a single-line English comment describing its purpose; do not add Args or Returns sections.
- Prefer functions of at most 50 lines and strongly avoid exceeding 100 lines.
- Prefer source files of at most 500 lines and strongly avoid exceeding 1,000 lines.
- Keep each module below ten files by splitting larger modules.
- Prefer methods and associated items; keep free functions and variables below 10% where practical. Required entry points and FFI declarations are exceptions.

## Git

- Commit only when explicitly requested. Review diffs, staged files, and untracked files before committing.
- Use a single-line English commit message. Push after a requested commit if a remote exists.
- Keep expanded Perforce and Jam sources, checksums, and license notices under `sdk/`. Split vendor files larger than 1 MiB into plain fragments and reconstruct them in the ignored build tree. Preserve vendor code and license text; project formatting and comment rules apply to maintained binding code.
- Do not track precompiled native libraries. `xtask` builds the SDK and bridge, then installs libraries into six platform resource crates. Consumers compile only Rust. Keep YAML thin and log every external command through the shared executor.
- Do not commit other files larger than 1 MiB, build outputs, caches, or temporary files.

- Do not track PowerShell, Python, shell, or other executable scripts. Implement build, dependency, packaging, and release operations in Rust xtask.
