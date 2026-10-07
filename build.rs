use std::{env, path::PathBuf};

struct Link;

impl Link {
    // Select packaged archives for TARGET, independently of the build-script host.
    fn run() -> Result<(), Box<dyn std::error::Error>> {
        let target =
            env::var("TARGET").map_err(|error| format!("Failed to read target: {error}"))?;
        if ![
            "x86_64-pc-windows-msvc",
            "aarch64-pc-windows-msvc",
            "x86_64-pc-windows-gnu",
            "x86_64-unknown-linux-gnu",
            "aarch64-unknown-linux-gnu",
            "x86_64-apple-darwin",
            "aarch64-apple-darwin",
        ]
        .contains(&target.as_str())
        {
            return Err(format!("Precompiled P4Rust ABI v1 is unavailable for {target}").into());
        }
        let features = env::var("CARGO_CFG_TARGET_FEATURE")
            .map_err(|error| format!("Failed to read target features: {error}"))?;
        if target.ends_with("windows-msvc")
            && features.split(',').any(|feature| feature == "crt-static")
        {
            return Err("Precompiled P4Rust requires the shared MSVC CRT (/MD)".into());
        }
        let root = env::var_os("CARGO_MANIFEST_DIR")
            .ok_or("Failed to locate packaged native libraries")?;
        let directory = PathBuf::from(root).join("native/lib").join(&target);
        let bridge = if target.ends_with("windows-msvc") {
            "p4rust_bridge.lib"
        } else {
            "libp4rust_bridge.a"
        };
        if !directory.join(bridge).is_file() {
            return Err(
                "Precompiled libraries are missing; run cargo xtask build in a source checkout"
                    .into(),
            );
        }
        println!("cargo:rustc-link-search=native={}", directory.display());
        println!("cargo:rerun-if-changed=native/lib");
        Self::libraries(&target);
        println!("cargo:rerun-if-changed=build.rs");
        Ok(())
    }

    // Link the supplied static archives and the selected platform's system libraries.
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
        if target.contains("windows") {
            if !msvc {
                for library in ["stdc++", "winpthread", "gcc_eh", "gcc"] {
                    println!("cargo:rustc-link-lib=static={library}");
                }
            }
            for library in [
                "ws2_32", "advapi32", "crypt32", "user32", "shell32", "ole32", "gdi32",
            ] {
                println!("cargo:rustc-link-lib={library}");
            }
        } else if target.ends_with("apple-darwin") {
            println!("cargo:rustc-link-lib=c++");
            for framework in ["CoreFoundation", "Security", "Foundation"] {
                println!("cargo:rustc-link-lib=framework={framework}");
            }
        } else {
            println!("cargo:rustc-link-lib=dylib:+verbatim=libstdc++.so.6");
            for library in ["pthread", "dl", "resolv"] {
                println!("cargo:rustc-link-lib={library}");
            }
        }
    }
}

// Emit local linker metadata without compiling native code or accessing the network.
fn main() -> Result<(), Box<dyn std::error::Error>> {
    Link::run().map_err(|error| format!("Failed to link precompiled P4Rust: {error}").into())
}
