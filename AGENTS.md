# Project Rules

## Language

- All comments, documentation, and project rules throughout this project must be written in English.

## Rust

- Maintain only 64-bit targets. Do not add 32-bit SDKs or native release packages.
- Build optimized native static libraries, strip debug symbols, disable unused OpenSSL features, and retain only archive members in the bridge's transitive symbol dependency closure.
- Preserve supported behavior through regression tests and keep every compressed distributable `.crate` strictly below 10 MB (10,000,000 bytes).
- After modifying Rust code, run `cargo clippy --workspace --all-targets --all-features -- -D warnings` and fix every reported issue.
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
- Commit the bundled Perforce C++ SDK under `sdk/`, including its headers, required libraries, and license notices. SDK `.tar.zst` archives may exceed 1 MiB; extracted `sdk/lib/` files must remain ignored.
- Do not track precompiled bridge or native release libraries. `xtask` generates `native/lib/` locally and includes them only in the distributable crate.
- Do not commit other files larger than 1 MiB, build outputs, caches, or temporary files.
