use crate::error::{Result, ResultExt, ensure};
use std::{env, path::Path};

pub(crate) struct Platform {
    pub(crate) target: String,
    pub(crate) host: String,
}

impl Platform {
    pub(crate) const TARGETS: &[&str] = &[
        "x86_64-pc-windows-msvc",
        "aarch64-pc-windows-msvc",
        "x86_64-pc-windows-gnu",
        "x86_64-unknown-linux-gnu",
        "aarch64-unknown-linux-gnu",
        "x86_64-apple-darwin",
        "aarch64-apple-darwin",
    ];

    // Select a maintained SDK target independently of the producer's Rust host.
    pub(crate) fn selected() -> Result<Self> {
        let host = match (env::consts::OS, env::consts::ARCH) {
            ("windows", "x86_64") if cfg!(target_env = "gnu") => "x86_64-pc-windows-gnu",
            ("windows", "x86_64") => "x86_64-pc-windows-msvc",
            ("windows", "aarch64") => "aarch64-pc-windows-msvc",
            ("linux", "x86_64") => "x86_64-unknown-linux-gnu",
            ("linux", "aarch64") => "aarch64-unknown-linux-gnu",
            ("macos", "x86_64") => "x86_64-apple-darwin",
            ("macos", "aarch64") => "aarch64-apple-darwin",
            _ => {
                return Err(crate::error::Error::new(
                    "Failed to select supported producer host",
                ));
            }
        }
        .to_owned();
        let target = match env::var("P4RUST_TARGET") {
            Ok(value) => value,
            Err(env::VarError::NotPresent) => host.clone(),
            Err(error) => return Err(error).context("Failed to read native target"),
        };
        ensure!(
            Self::TARGETS.contains(&target.as_str()),
            "Failed to select supported SDK target: {target}"
        );
        Ok(Self { target, host })
    }

    // Identify targets that use the Microsoft archive and CRT conventions.
    pub(crate) fn msvc(&self) -> bool {
        self.target.ends_with("windows-msvc")
    }
    // Identify targets that require Windows system libraries.
    pub(crate) fn windows(&self) -> bool {
        self.target.contains("windows")
    }
    // Identify targets that use Apple's C++ runtime and Mach-O archives.
    pub(crate) fn apple(&self) -> bool {
        self.target.ends_with("apple-darwin")
    }
    // Select the producer archive name without changing the public ABI.
    pub(crate) fn bridge(&self) -> &'static str {
        if self.msvc() {
            "p4rust_bridge.lib"
        } else {
            "libp4rust_bridge.a"
        }
    }
    // Select the static TLS archive convention for the selected compiler.
    pub(crate) fn ssl_names(&self) -> [&'static str; 2] {
        if self.msvc() {
            ["libssl.lib", "libcrypto.lib"]
        } else {
            ["libssl.a", "libcrypto.a"]
        }
    }
    // Configure a compiler with target-specific SDK definitions and release policy.
    pub(crate) fn compiler(&self) -> cc::Build {
        let mut compiler = cc::Build::new();
        compiler
            .cpp(true)
            .std("c++17")
            .opt_level(2)
            .debug(false)
            .static_crt(false)
            .target(&self.target)
            .host(&self.host)
            .cargo_metadata(false);
        if self.msvc() {
            compiler.flag("/EHsc");
        }
        if self.windows() {
            compiler.define("OS_NT", None).define("NOMINMAX", None);
        } else {
            compiler.pic(true).define(
                if self.apple() {
                    "OS_MACOSX"
                } else {
                    "OS_LINUX"
                },
                None,
            );
        }
        compiler
    }
    // Locate a named compiler utility while allowing explicit maintainer overrides.
    pub(crate) fn utility(variable: &str, fallback: &str) -> String {
        env::var(variable).unwrap_or_else(|_| {
            let installed = Path::new("C:/Program Files/LLVM/bin").join(format!("{fallback}.exe"));
            if cfg!(windows) && installed.is_file() {
                installed.to_string_lossy().into_owned()
            } else {
                fallback.into()
            }
        })
    }
}
