use std::{env, path::PathBuf};

struct Link;

impl Link {
    // Link packaged libraries only when this resource crate matches the Rust target.
    fn run() -> Result<(), Box<dyn std::error::Error>> {
        let target = env::var("TARGET")
            .map_err(|error| format!("Failed to read resource target: {error}"))?;
        let package = env::var("CARGO_PKG_NAME")
            .map_err(|error| format!("Failed to read resource package: {error}"))?;
        let expected = package
            .strip_prefix("p4rust-resources-")
            .ok_or("Failed to identify resource platform")?;
        if target != expected {
            return Ok(());
        }
        if target.ends_with("windows-msvc") {
            let features = env::var("CARGO_CFG_TARGET_FEATURE")
                .map_err(|error| format!("Failed to read resource target features: {error}"))?;
            if features.split(',').any(|feature| feature == "crt-static") {
                return Err("P4Rust resources require the shared MSVC CRT (/MD)".into());
            }
        }
        let root = env::var_os("CARGO_MANIFEST_DIR").ok_or("Failed to locate resource package")?;
        let directory = PathBuf::from(root).join("native");
        let bridge = if target.ends_with("windows-msvc") {
            "p4rust_bridge.lib"
        } else {
            "libp4rust_bridge.a"
        };
        if !directory.join(bridge).is_file() {
            return Err(
                "Precompiled resources are missing; run cargo xtask build in a source checkout"
                    .into(),
            );
        }
        println!("cargo:rustc-link-search=native={}", directory.display());
        println!("cargo:rerun-if-changed=native");
        Self::libraries(&target);
        Ok(())
    }

    // Emit archive and system-library metadata without launching native tools.
    fn libraries(target: &str) {
        let msvc = target.ends_with("windows-msvc");
        for library in [
            "p4rust_bridge",
            "client",
            "p4script_cstub",
            "rpc",
            "supp",
            "ssl",
            "crypto",
        ] {
            let name = if msvc && library != "p4rust_bridge" {
                format!("lib{library}")
            } else {
                library.into()
            };
            println!("cargo:rustc-link-lib=static={name}");
        }
        if msvc {
            for library in [
                "ws2_32", "advapi32", "crypt32", "user32", "shell32", "ole32", "oleaut32", "gdi32",
                "bcrypt", "iphlpapi",
            ] {
                println!("cargo:rustc-link-lib={library}");
            }
        } else if target.ends_with("apple-darwin") {
            println!("cargo:rustc-link-lib=c++");
            for framework in [
                "CoreFoundation",
                "CoreServices",
                "ApplicationServices",
                "CoreGraphics",
                "Security",
                "Foundation",
                "SystemConfiguration",
            ] {
                println!("cargo:rustc-link-lib=framework={framework}");
            }
        } else {
            println!("cargo:rustc-link-lib=dylib:+verbatim=libstdc++.so.6");
            for library in ["pthread", "dl", "resolv", "rt", "m"] {
                println!("cargo:rustc-link-lib={library}");
            }
        }
    }
}

// Provide static linker metadata while compiling only Rust on the consumer side.
fn main() -> Result<(), Box<dyn std::error::Error>> {
    Link::run().map_err(|error| format!("Failed to link P4Rust resources: {error}").into())
}
