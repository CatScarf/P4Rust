# SDK sources

The repository stores expanded Perforce 2026.1.3062361 and Jam 2.6 sources. `sources.json` records upstream locations and SHA-256 checksums for vendor source files. `xtask` verifies the source bytes, copies them under `temp`, and builds the four API libraries using the vendor Jam rules.

Supported targets are Windows MSVC, Linux GNU, and macOS, each on x64 and ARM64. Precompiled SDK and bridge libraries are generated only for resource crate packaging.

See [vendor notices](NOTICE.md) and [the Perforce source license](LICENSE).

Two oversized vendor files are stored as plain fragments below 1 MiB each; xtask restores their exact bytes in the build copy. No source compression is used.
