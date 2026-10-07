# SDK sources

The repository stores expanded Perforce 2026.1.3062361 and Jam 2.6 sources. Maintained changes live directly in these sources inside paired `// PR_XXX Start` and `// PR_XXX End` markers. `sources.json` records upstream archive locations and SHA-256 checksums of the maintained source files. `xtask` verifies these bytes, copies them under `temp`, and builds the three API libraries using the vendor Jam rules without rewriting source code.

Supported targets are Windows MSVC, Linux GNU, and macOS, each on x64 and ARM64. Precompiled SDK and bridge libraries are generated only for resource crate packaging.

See [vendor notices](NOTICE.md) and [the Perforce source license](LICENSE).

Two oversized vendor files are stored as plain fragments below 1 MiB each; xtask restores their exact bytes in the build copy. No source compression is used.
