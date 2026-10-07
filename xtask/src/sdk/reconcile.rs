use crate::error::{Result, ResultExt, ensure};
use std::{fs, path::Path};

pub(super) struct Hooks;

impl Hooks {
    // Apply checked, marked changes only to the checksum-verified ignored SDK build copy.
    pub(super) fn install(root: &Path, source: &Path) -> Result<()> {
        for directory in ["client", "rpc", "sys"] {
            fs::copy(
                root.join("native/sdk_hooks.h"),
                source.join(directory).join("p4rust_hooks.h"),
            )
            .context("Failed to copy SDK hook declarations")?;
        }
        Self::client(source).context("Failed to connect SDK reconcile callbacks")?;
        Self::dispatcher(source).context("Failed to connect SDK reply dispatcher")?;
        for name in ["sys/fileio.cc", "sys/fileiont.cc"] {
            Self::reading(&source.join(name)).context("Failed to connect SDK read interruption")?;
        }
        Ok(())
    }

    // Replace a unique source anchor while preserving the vendor's original newline convention.
    fn replace(path: &Path, anchor: &str, replacement: &str) -> Result<()> {
        let text = fs::read_to_string(path).context("Failed to read SDK hook input")?;
        let newline = if text.contains("\r\n") { "\r\n" } else { "\n" };
        let anchor = anchor.replace('\n', newline);
        ensure!(
            text.matches(&anchor).count() == 1,
            "Failed to locate unique SDK hook anchor in {}",
            path.display()
        );
        let replacement = replacement.replace('\n', newline);
        fs::write(path, text.replacen(&anchor, &replacement, 1))
            .context("Failed to write marked SDK hook")
    }

    // Include the narrow hook interface without changing SDK public headers.
    fn include(path: &Path, anchor: &str) -> Result<()> {
        Self::replace(
            path,
            anchor,
            &format!("{anchor}\n// PR_001 Start\n#include \"p4rust_hooks.h\"\n// PR_001 End"),
        )
        .context("Failed to include SDK hook interface")
    }

    // Preserve the SDK reconcile handle while offloading independent local comparisons.
    fn client(source: &Path) -> Result<()> {
        let path = source.join("client/clientservicer.cc");
        Self::include(&path, "# include \"clientaltsynchandler.h\"")?;
        let anchor = "void\nclientReconcileFlush( Client *client, Error *e )";
        let helper = "// PR_001 Start\n// Commit a worker decision into the original SDK reconcile state.\nvoid p4rust::NoteEdit(Client* client, const char* path, bool missing, bool store)\n{\n    Error error;\n    auto* handle = ReconcileHandle::GetOrCreate(client, true, &error);\n    if (error.Test()) { client->OutputError(&error); return; }\n    if (missing) ++handle->delCount;\n    else if (store) handle->pathArray->Put()->Set(path);\n    handle->Increment(client, 1);\n}\n// PR_001 End\n\n";
        Self::replace(&path, anchor, &format!("{helper}{anchor}"))?;
        let helper = "// PR_001 Start\n// Preserve the SDK's owner-thread move index bookkeeping.\nbool p4rust::AlreadyMatched(Client* client, int index)\n{\n    Error error;\n    auto* handle = ReconcileHandle::GetOrCreate(client, true, &error);\n    if (error.Test()) { client->OutputError(&error); return true; }\n    return handle->AlreadyMatched(index) != 0;\n}\n// Commit an exact match to the original reconcile handle.\nvoid p4rust::NoteExact(Client* client, int index, bool matched)\n{\n    Error error;\n    auto* handle = ReconcileHandle::GetOrCreate(client, true, &error);\n    if (error.Test()) { client->OutputError(&error); return; }\n    if (matched) handle->SetMatch(index);\n    handle->Increment(client, matched ? 1 : 0);\n}\n// PR_001 End\n\n";
        Self::replace(&path, anchor, &format!("{helper}{anchor}"))?;
        let anchor = "\tint statVal = f->Stat();\n\n\t// Save the list of depot files.";
        Self::replace(
            &path,
            anchor,
            "\t// PR_001 Start\n\tint statVal = p4rust::ActiveReconcile() ? 0 : f->Stat();\n\t// PR_001 End\n\n\t// Save the list of depot files.",
        )?;
        let anchor = "\tif( !( statVal & ( FSF_SYMLINK|FSF_EXISTS ) ) )";
        Self::replace(
            &path,
            anchor,
            &format!(
                "\t// PR_001 Start\n\tif (p4rust::ScheduleEdit(client, f)) return;\n\t// PR_001 End\n{anchor}"
            ),
        )?;
        for function in [
            "clientReconcileFlush",
            "clientReconcileAdd",
            "clientExactMatch",
        ] {
            let anchor = format!("{function}( Client *client, Error *e )\n{{");
            Self::replace(
                &path,
                &anchor,
                &format!(
                    "{anchor}\n    // PR_001 Start\n    p4rust::Barrier(client);\n    // PR_001 End"
                ),
            )?;
        }
        Self::traversal(&path).context("Failed to connect SDK directory traversal")?;
        let anchor = "\tStrPtr *matchFile = 0;\n\tFileSys *f = 0;";
        Self::replace(
            &path,
            anchor,
            &format!(
                "    // PR_001 Start\n    if (p4rust::ScheduleExact(client)) return;\n    // PR_001 End\n{anchor}"
            ),
        )?;
        Self::moves(&path).context("Failed to connect SDK move scheduling")
    }

    // Route directory traversal through the Rust pools while keeping the original fallback intact.
    fn traversal(path: &Path) -> Result<()> {
        let anchor = "\t// Return all files in dir, and optionally traverse dirs in dir,";
        Self::replace(path, anchor,
            &format!("    // PR_001 Start\n    if (p4rust::Traverse(client, dir, traverse, noIgnore, getDigests, getTypes,\n        map, files, sizes, times, digests, types, hasIndex, hasList, config, progress, e)) return;\n    // PR_001 End\n{anchor}"))
            .context("Failed to replace SDK traversal entry")
    }

    // Reuse the SDK move algorithm while giving Rust ownership of its worker scheduling.
    fn moves(path: &Path) -> Result<()> {
        Self::replace(
            path,
            "\tstd::vector< std::thread > ts;",
            "\t// PR_001 Start\n\tstd::vector< p4rust::MatchThread > ts;\n\t// PR_001 End",
        )?;
        let anchor = "\tint threads = strThreads ? strThreads->Atoi() : 1;";
        Self::replace(
            path,
            anchor,
            &format!(
                "{anchor}\n\t// PR_001 Start\n\tthreads = p4rust::MoveWorkers(client, threads);\n\t// PR_001 End"
            ),
        )?;
        let anchor =
            "\t    *res = DiffMatchFiles( s, f2, s2 );\n\t    s.Release();\n\t}\n\tdelete f1;";
        let replacement = "\t    *res = DiffMatchFiles( s, f2, s2 );\n\t    s.Release();\n\t}\n    // PR_001 Start\n    else { s2->Release(); delete f2; }\n    // PR_001 End\n\tdelete f1;";
        Self::replace(path, anchor, replacement)
            .context("Failed to preserve interrupted move cleanup")
    }

    // Deliver ready confirmations before the dispatcher blocks waiting for the server.
    fn dispatcher(source: &Path) -> Result<()> {
        let path = source.join("rpc/rpc.cc");
        Self::include(&path, "# include <msgrpc.h>")?;
        let anchor = "\t\tDispatchOne( dispatcher, flag == DfContain );";
        Self::replace(&path, anchor,
            &format!("\t\t// PR_001 Start\n\t\tp4rust::Pump(this, false);\n\t\twhile (p4rust::Pending(this) && transport && !transport->RecvReady())\n\t\t    p4rust::Pump(this, true);\n\t\t// PR_001 End\n{anchor}"))
            .context("Failed to connect SDK dispatcher polling")
    }

    // Check cancellation before every platform-specific binary read used by SDK converters.
    fn reading(path: &Path) -> Result<()> {
        Self::include(path, "# include <error.h>")?;
        let anchor = "FileIOBinary::Read( char *buf, int len, Error *e )\n{";
        Self::replace(path, anchor,
            &format!("{anchor}\n    // PR_001 Start\n    if (!p4rust::WorkerAlive()) {{\n        e->Set(E_FAILED, \"Local reconcile computation interrupted.\");\n        return 0;\n    }}\n    // PR_001 End"))
            .context("Failed to connect SDK binary read polling")
    }
}
