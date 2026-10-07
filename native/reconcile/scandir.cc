namespace p4rust {
class DirectoryTask final : public Task {
    struct Entry { std::string path, wire; bool directory; };
    std::shared_ptr<ScanContext> context;
    std::vector<Entry> entries;
    std::unique_ptr<CharSetCvt> names;
    MapApi mapping;
    Ignore ignore;
    std::string path;
    std::shared_ptr<StrArray> listing;
    int first;
public:
    // Give each scan its own mapping, ignore cache, and filename converter.
    DirectoryTask(std::shared_ptr<ScanContext> scan, const std::string& local,
        std::shared_ptr<StrArray> names_list = {}, int start = -1)
        : Task(scan->scope, "directory"), context(std::move(scan)), path(local), listing(std::move(names_list)), first(start) {
        request = context->metadata;
        request.Set("kind", "directory"); request.Set("localPath", path);
        if (first >= 0) request.Set("first", std::to_string(first));
        mapping.SetCaseSensitivity(context->nocase ? Insensitive : Sensitive);
        for (const auto& rule : context->mappings)
            mapping.Insert(StrRef(rule.left.c_str()), StrRef(rule.right.c_str()), rule.type);
        if (context->names) names.reset(context->names->Clone());
        result.Set("localPath", path);
    }
    // Enumerate one directory with SDK paths, ignore rules, and symlink behavior.
    void Run() override {
        Check();
        if (first >= 0) { ProbeChunk(); return; }
        std::unique_ptr<FileSys> file(FileSys::Create(FST_BINARY));
        if (!file) throw std::runtime_error("Failed to create reconcile directory probe");
        file->EnableStatCache(); file->SetContentCharSetPriv(context->charset);
        file->Set(StrRef(path.c_str()));
        const auto stat = file->Stat();
        if (!(stat & FSF_DIRECTORY) || (stat & FSF_SYMLINK)) {
            if ((stat & (FSF_EXISTS | FSF_SYMLINK)) && !Rejected(path, false))
                entries.push_back({path, Wire(path), false});
            return;
        }
        if (Rejected(path, true)) return;
        Error error;
        listing.reset(file->ScanDir(&error));
        CheckError(error, "Failed to scan reconcile directory");
        if (!listing) throw std::runtime_error("Failed to receive reconcile directory entries");
        listing->Sort(!StrBuf::CaseUsage());
        result.Set("entries", std::to_string(listing->Count()));
    }
    // Stat bounded filename chunks concurrently without retaining full paths for an entire directory.
    void ProbeChunk() {
        std::unique_ptr<FileSys> file(FileSys::Create(FST_BINARY));
        if (!file) throw std::runtime_error("Failed to create reconcile metadata probe");
        file->EnableStatCache(); file->SetContentCharSetPriv(context->charset);
        file->Set(StrRef(path.c_str()));
        std::unique_ptr<PathSys> joined(PathSys::Create());
        joined->SetCharSet(file->GetCharSetPriv());
        const int end = std::min(listing->Count(), first + 64);
        for (int i = first; i < end; ++i) {
            Check(); joined->SetLocal(StrRef(path.c_str()), *listing->Get(i));
            const std::string child = joined->Text();
            if (context->Known(child.c_str())) continue;
            file->Set(*joined);
            Select(child, file->Stat());
        }
        result.Set("entries", std::to_string(entries.size()));
    }
    // Apply server mappings to files while permitting traversal through unmapped parent directories.
    void Select(const std::string& local, int stat) {
        if ((stat & FSF_DIRECTORY) && !(stat & FSF_SYMLINK)) {
            if (context->traverse && !Rejected(local, true)) entries.push_back({local, {}, true});
            return;
        }
        if (!(stat & (FSF_EXISTS | FSF_SYMLINK))) return;
        const auto wire = Wire(local);
        auto mapped = wire;
        if ((stat & FSF_DIRECTORY) && (stat & FSF_SYMLINK)) mapped += "/";
#ifdef OS_NT
        std::replace(mapped.begin(), mapped.end(), '\\', '/');
#endif
        StrBuf destination;
        if (mapping.Translate(StrRef(mapped.c_str()), destination) && !Rejected(local, false))
            entries.push_back({local, wire, false});
    }
    // Consult an isolated SDK ignore cache with the captured configuration filename.
    bool Rejected(const std::string& local, bool directory) {
        if (context->no_ignore) return false;
        const char* config = context->config.empty() ? nullptr : context->config.c_str();
        return directory ? ignore.RejectDir(StrRef(local.c_str()), StrRef(context->ignore.c_str()), config) != 0
            : ignore.Reject(StrRef(local.c_str()), StrRef(context->ignore.c_str()), config) != 0;
    }
    // Convert local names to wire UTF-8 with the SDK's original fallback behavior.
    std::string Wire(const std::string& local) {
        if (!names) return local;
        const auto* converted = names->FastCvt(local.c_str(), static_cast<int>(local.size()), 0);
        return converted ? converted : local;
    }
    // Schedule child work through bounded Rust admission on the connection thread.
    void Commit(int) override {
        Check();
        if (first < 0 && context->progress) context->progress->Increment(1);
        if (first < 0 && listing) {
            for (int i = 0; i < listing->Count(); i += 64) {
                Check();
                scope.Submit(new DirectoryTask(context, path, listing, i));
            }
            return;
        }
        for (const auto& entry : entries) {
            Check();
            if (entry.directory) ScheduleDirectory(context, entry.path);
            else ScheduleFile(context, entry.path, entry.wire);
        }
    }
};
// Transfer one directory to the metadata pool without invoking RPC from a worker.
void ScheduleDirectory(const std::shared_ptr<ScanContext>& context, const std::string& path) {
    context->scope.Submit(new DirectoryTask(context, path));
}
}
