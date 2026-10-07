use std::{env, path::PathBuf};

struct Link;

impl Link {
    // Select packaged archives for TARGET, independently of the build-script host.
    fn run() -> Result<(), Box<dyn std::error::Error>> {
        let target =
            env::var("TARGET").map_err(|error| format!("Failed to read target: {error}"))?;
        if target != "x86_64-pc-windows-msvc" {
            return Err(format!("Precompiled P4Rust ABI v1 is unavailable for {target}").into());
        }
        let features = env::var("CARGO_CFG_TARGET_FEATURE")
            .map_err(|error| format!("Failed to read target features: {error}"))?;
        if features.split(',').any(|feature| feature == "crt-static") {
            return Err("Precompiled P4Rust requires the shared MSVC CRT (/MD)".into());
        }
        let root = env::var_os("CARGO_MANIFEST_DIR")
            .ok_or("Failed to locate packaged native libraries")?;
        let directory = PathBuf::from(root).join("native/lib").join(&target);
        if !directory.join("p4rust_bridge.lib").is_file() {
            return Err(
                "Precompiled libraries are missing; run cargo xtask build in a source checkout"
                    .into(),
            );
        }
        println!("cargo:rustc-link-search=native={}", directory.display());
        println!("cargo:rerun-if-changed=native/lib");
        Self::libraries();
        println!("cargo:rerun-if-changed=build.rs");
        Ok(())
    }

    // Link the supplied static archives and standard Windows system libraries.
    fn libraries() {
        for library in [
            "p4rust_bridge",
            "libclient",
            "libp4script_cstub",
            "librpc",
            "libsupp",
            "libssl",
            "libcrypto",
        ] {
            println!("cargo:rustc-link-lib=static={library}");
        }
        for library in [
            "ws2_32", "advapi32", "crypt32", "user32", "shell32", "ole32", "gdi32",
        ] {
            println!("cargo:rustc-link-lib={library}");
        }
    }
}

// Emit local linker metadata without compiling native code or accessing the network.
fn main() -> Result<(), Box<dyn std::error::Error>> {
    Link::run().map_err(|error| format!("Failed to link precompiled P4Rust: {error}").into())
}
