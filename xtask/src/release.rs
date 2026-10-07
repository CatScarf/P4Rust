use crate::{
    error::{Result, ResultExt, ensure},
    platform::Platform,
    prune::Prune,
};
use std::{
    fs,
    path::{Path, PathBuf},
};

pub(crate) struct Release;

impl Release {
    // Install compact target archives without generating a release inventory.
    pub(crate) fn install(
        root: &Path,
        output: &Path,
        bridge: &Path,
        ssl: &Path,
        platform: &Platform,
    ) -> Result<()> {
        let staging = output.join("release-libraries");
        let destination = root.join("native/lib").join(&platform.target);
        fs::create_dir_all(&staging).context("Failed to create release staging directory")?;
        fs::create_dir_all(&destination).context("Failed to create release library directory")?;
        let mut paths = vec![bridge.to_path_buf()];
        paths.extend(platform.ssl_names().iter().map(|name| ssl.join(name)));
        for name in ["client", "p4script_cstub", "rpc", "supp"] {
            let filename = if platform.msvc() {
                format!("lib{name}.lib")
            } else {
                format!("lib{name}.a")
            };
            paths.push(root.join("sdk/lib").join(&platform.target).join(filename));
        }
        if platform.windows() && !platform.msvc() {
            paths.extend(
                Self::runtime(root, platform).context("Failed to prepare GNU runtime archives")?,
            );
        }
        let mut libraries = Vec::new();
        for path in &paths {
            let name = path
                .file_name()
                .context("Failed to identify native archive")?;
            let staged = staging.join(name);
            fs::copy(path, &staged)
                .with_context(|| format!("Failed to stage {}", path.display()))?;
            libraries.push(staged);
        }
        Prune::compact(root, &staging, &libraries, platform)
            .context("Failed to compact native release")?;
        for path in libraries {
            fs::copy(
                &path,
                destination.join(
                    path.file_name()
                        .context("Failed to identify compact archive")?,
                ),
            )
            .context("Failed to install compact native archive")?;
        }
        Ok(())
    }

    // Bundle GNU support archives so consumers need no C++ development libraries.
    fn runtime(root: &Path, platform: &Platform) -> Result<Vec<PathBuf>> {
        let compiler = platform
            .compiler()
            .try_get_compiler()
            .context("Failed to locate GNU compiler")?;
        let mut paths = Vec::new();
        for name in ["libstdc++.a", "libwinpthread.a", "libgcc.a", "libgcc_eh.a"] {
            let output = compiler
                .to_command()
                .arg(format!("-print-file-name={name}"))
                .output()
                .context("Failed to locate GNU runtime archive")?;
            ensure!(
                output.status.success(),
                "Failed to query GNU runtime archive"
            );
            let path = PathBuf::from(
                String::from_utf8(output.stdout)
                    .context("Failed to decode GNU runtime path")?
                    .trim(),
            );
            ensure!(path.is_file(), "Failed to find GNU runtime archive: {name}");
            paths.push(path);
        }
        let licenses = std::env::var_os("P4RUST_RUNTIME_LICENSE_DIR")
            .context("Failed to locate GNU runtime license directory")?;
        let directory = Path::new(&licenses);
        let gcc_notices: &[&str] = if directory.join("gcc-libs").is_dir() {
            &["gcc-libs", "winpthreads"]
        } else {
            &["libgcc", "libstdc++", "winpthreads"]
        };
        for name in gcc_notices {
            Self::licenses(
                &directory.join(name),
                &root.join("native/licenses").join(name),
            )
            .context("Failed to include GNU runtime license notices")?;
        }
        Ok(paths)
    }

    // Preserve vendor license notices alongside bundled GNU runtime libraries.
    fn licenses(source: &Path, destination: &Path) -> Result<()> {
        fs::create_dir_all(destination).context("Failed to create runtime license directory")?;
        for entry in fs::read_dir(source).context("Failed to enumerate runtime licenses")? {
            let entry = entry.context("Failed to read runtime license entry")?;
            let target = destination.join(entry.file_name());
            if entry
                .file_type()
                .context("Failed to inspect runtime license")?
                .is_dir()
            {
                Self::licenses(&entry.path(), &target)
                    .context("Failed to copy runtime license subtree")?;
            } else {
                fs::copy(entry.path(), target).context("Failed to copy runtime license notice")?;
            }
        }
        Ok(())
    }
}
