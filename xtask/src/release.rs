use crate::{
    error::{Result, ResultExt},
    platform::Platform,
    prune::Prune,
};
use std::{fs, path::Path};

pub(crate) struct Release;

impl Release {
    // Install compact target archives without generating a release inventory.
    pub(crate) fn install(
        root: &Path,
        output: &Path,
        bridge: &Path,
        ssl: &Path,
        sdk: &Path,
        platform: &Platform,
    ) -> Result<()> {
        let staging = output.join("release-libraries");
        let package = root.join("resources").join(&platform.target);
        let destination = package.join("native");
        fs::create_dir_all(&staging).context("Failed to create release staging directory")?;
        fs::create_dir_all(&destination).context("Failed to create release library directory")?;
        let stale = destination.join(if platform.msvc() {
            "libp4script_cstub.lib"
        } else {
            "libp4script_cstub.a"
        });
        if stale.is_file() {
            fs::remove_file(stale).context("Failed to remove obsolete scripting stub archive")?;
        }
        let mut paths = vec![bridge.to_path_buf()];
        paths.extend(platform.ssl_names().iter().map(|name| ssl.join(name)));
        for name in ["client", "rpc", "supp"] {
            let filename = if platform.msvc() {
                format!("lib{name}.lib")
            } else {
                format!("lib{name}.a")
            };
            paths.push(sdk.join(filename));
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
        fs::copy(
            root.join("sdk/LICENSE"),
            package.join("PERFORCE-LICENSE.txt"),
        )
        .context("Failed to include Perforce source license")?;
        fs::copy(
            root.join("native/OPENSSL-LICENSE.txt"),
            package.join("OPENSSL-LICENSE.txt"),
        )
        .context("Failed to include OpenSSL license")?;
        Ok(())
    }
}
