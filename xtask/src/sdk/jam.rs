use super::source::Sources;
use crate::{
    automation::command::Runner,
    error::{Result, ResultExt},
    platform::Platform,
};
use std::{
    fs,
    path::{Path, PathBuf},
};

pub(super) struct Jam;

impl Jam {
    // Build the vendor's small Jam driver with the native host compiler.
    pub(super) fn build(root: &Path, platform: &Platform) -> Result<PathBuf> {
        let source = Sources::directory(root, "jam").context("Failed to locate Jam sources")?;
        let executable = source.join(if platform.windows() { "jam.exe" } else { "jam" });
        if executable.is_file() {
            return Ok(executable);
        }
        let header = source.join("jam.h");
        let text = fs::read_to_string(&header).context("Failed to read Jam host configuration")?;
        fs::write(
            header,
            text.replace(
                "# define MAXLINE 996",
                "/* P4Rust: allow modern compiler command lengths. */\n# define MAXLINE 32760",
            ),
        )
        .context("Failed to configure Jam command capacity")?;
        let mut compiler = cc::Build::new();
        compiler
            .target(&platform.host)
            .host(&platform.host)
            .opt_level(2)
            .cargo_metadata(false);
        let tool = compiler
            .try_get_compiler()
            .context("Failed to locate Jam host compiler")?;
        let mut command = tool.to_command();
        command.current_dir(&source);
        if platform.windows() {
            command
                .args(["/O2", "/MD", "/DNT", "/D_CRT_SECURE_NO_WARNINGS", "/wd4996"])
                .arg(format!("/Fe{}", executable.display()));
        } else {
            command
                .args([
                    "-O2",
                    "-std=gnu89",
                    "-Wno-implicit-function-declaration",
                    "-Wno-int-conversion",
                    "-o",
                ])
                .arg(&executable);
            if platform.apple() {
                command.arg("-D__DARWIN__");
            }
        }
        command.args([
            "builtins.c",
            "command.c",
            "compile.c",
            "execas400.c",
            "execunix.c",
            "execvms.c",
            "expand.c",
            "filent.c",
            "fileos2.c",
            "fileunix.c",
            "filevms.c",
            "glob.c",
            "hash.c",
            "headers.c",
            "jam.c",
            "jambase.c",
            "jamgram.c",
            "lists.c",
            "make.c",
            "make1.c",
            "newstr.c",
            "option.c",
            "parse.c",
            "pathunix.c",
            "pathvms.c",
            "regexp.c",
            "rules.c",
            "scan.c",
            "search.c",
            "timestamp.c",
            "variable.c",
        ]);
        Runner::run(&mut command, false).context("Failed to compile Jam build driver")?;
        Ok(executable)
    }
}
