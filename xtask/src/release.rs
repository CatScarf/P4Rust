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
}
