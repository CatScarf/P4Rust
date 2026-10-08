namespace p4rust {
class ScanAgent {
    ThreadScope thread;
    ClientApi settings;
    Ignore ignore;
    std::unique_ptr<CharSetCvt> converter;
    std::string rules, config;
    bool use_ignore;
    p4rust_alive_v1 alive;
    void* control;
    int charset;
public:
    // Capture configuration once per worker and retain independent ignore and converter state.
    ScanAgent(const char* cwd, const char* encoding, bool enabled, p4rust_alive_v1 callback, void* context)
        : use_ignore(enabled), alive(callback), control(context) {
        settings.SetCwd(cwd);
        rules = settings.GetIgnoreFile().Text();
        const auto* value = settings.GetEnviro()->Get("P4CONFIG");
        config = value ? value : "";
        charset = static_cast<int>(CharSetApi::Lookup(encoding[0] ? encoding : "none"));
        if (charset < 0) throw std::runtime_error("Failed to resolve local scan charset");
        converter.reset(CharSetCvt::FindCvt(static_cast<CharSetCvt::CharSet>(charset), CharSetCvt::UTF_8));
    }
    // Poll command cancellation without borrowing a mutable SDK connection.
    bool Alive() const { return !alive || alive(control) != 0; }
    // Preserve ignore behavior and avoid reading contents in timestamp mode.
    int Inspect(const char* path, int directory, bool hashes, p4rust_local_v5& snapshot) {
        if (!Alive()) return 0;
        const char* configuration = config.empty() ? nullptr : config.c_str();
        if (directory != -1 && use_ignore && (directory > 0 ? ignore.RejectDir(StrRef(path), StrRef(rules.c_str()), configuration)
                                    : ignore.Reject(StrRef(path), StrRef(rules.c_str()), configuration))) return 0;
        if (directory > 0 || directory == -2) return 1;
        std::unique_ptr<FileSys> original(FileSys::Create(FST_BINARY));
        if (!original) throw std::runtime_error("Failed to create local scan probe");
        original->SetContentCharSetPriv(charset); original->Set(StrRef(path));
        if (snapshot.stat >= 0) original->UseStatSnapshot(snapshot.stat, snapshot.size, snapshot.time);
        original->EnableStatCache();
        const auto stat = original->Stat();
        snapshot.stat = stat;
        if (!(stat & (FSF_EXISTS | FSF_SYMLINK))) return 0;
        snapshot.size = original->GetSize(); snapshot.time = original->StatModTime();
        if (!hashes) return 1;
        const auto type = original->CheckType();
        std::unique_ptr<FileSys> file(FileSys::Create(type));
        if (!file) return 1;
        file->SetContentCharSetPriv(charset); file->Set(StrRef(path));
        file->UseStatSnapshot(snapshot.stat, snapshot.size, file->IsSymlink() ? snapshot.link_time : snapshot.time);
        file->Translator(converter.get());
        Error error; StrBuf digest;
        const auto size = file->Digest(&digest, &error);
        if (!Alive()) return 0;
        if (error.Test() || digest.Length() != 32) return 1;
        snapshot.file_type = file->GetType(); snapshot.hashed = 1;
        snapshot.canonical_size = size; snapshot.charset = file->GetContentCharSetPriv();
        std::memcpy(snapshot.digest, digest.Text(), 32);
        return 1;
    }
};
static thread_local ScanAgent* scan_agent = nullptr;
// Include local enumeration in the existing SDK read-loop cancellation hook.
bool ScanAlive() { return !scan_agent || scan_agent->Alive(); }
}
// Construct independent scan policy and contain initialization failures at the ABI boundary.
extern "C" int32_t p4rust_scan_open_v5(const char* cwd, const char* charset, int32_t ignore,
    p4rust_alive_v1 alive, void* control, void** output) {
    try {
        if (!cwd || !charset || !output || p4rust::scan_agent) return 1;
        p4rust::Runtime::Ready();
        *output = nullptr;
        auto agent = std::make_unique<p4rust::ScanAgent>(cwd, charset, ignore != 0, alive, control);
        p4rust::scan_agent = agent.get(); *output = agent.release(); return 0;
    } catch (const std::exception& error) { std::fprintf(stderr, "Failed to open local scan agent: %s\n", error.what()); return 1; }
}
// Probe one file without constructing another SDK thread or connection.
extern "C" int32_t p4rust_scan_file_v5(void* pointer, const char* path, int32_t directory,
    int32_t hashes, p4rust_local_v5* output) {
    try {
        if (!pointer || pointer != p4rust::scan_agent || !path || !output) return -1;
        return static_cast<p4rust::ScanAgent*>(pointer)->Inspect(path, directory, hashes != 0, *output);
    } catch (const std::exception& error) { std::fprintf(stderr, "Failed to scan local file: %s\n", error.what()); return -1; }
}
// Release SDK state only after this worker's last digest read finishes.
extern "C" void p4rust_scan_close_v5(void* pointer) {
    p4rust::scan_agent = nullptr;
    delete static_cast<p4rust::ScanAgent*>(pointer);
}
// Transfer canonical digest metadata while Rust exclusively owns the comparison task.
extern "C" int32_t p4rust_reconcile_snapshot_v5(void* pointer, const p4rust_local_v5* snapshot) {
    auto* task = static_cast<p4rust::Task*>(pointer);
    return p4rust::TaskBoundary::Run(task, [task, snapshot] {
        if (!snapshot) throw std::runtime_error("Failed to attach null local snapshot");
        task->Snapshot(*snapshot);
    });
}
// Copy a candidate into the connection-owned list before the callback returns.
extern "C" int32_t p4rust_reconcile_path_v5(void* pointer, const char* path, const p4rust_local_v5* snapshot) {
    try {
        if (!pointer || !path || !snapshot) return 1;
        static_cast<std::vector<p4rust::PathCandidate>*>(pointer)->push_back({path, *snapshot}); return 0;
    } catch (...) { return 1; }
}
