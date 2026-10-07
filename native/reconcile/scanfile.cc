namespace p4rust {
class AddFileTask final : public Task {
    std::shared_ptr<ScanContext> context;
    std::string path, wire;
    std::unique_ptr<CharSetCvt> translator;
public:
    // Retain immutable scan settings and a task-specific content translator.
    AddFileTask(std::shared_ptr<ScanContext> scan, const std::string& local, const std::string& name)
        : Task(scan->scope, "untracked"), context(std::move(scan)), path(local), wire(name) {
        request = context->metadata;
        request.Set("kind", "untracked");
        request.Set("localPath", path);
        if (context->contents) translator.reset(context->contents->Clone());
        result.Set("localPath", path);
        result.Set("file", wire);
    }
    // Use the SDK's canonical type and digest behavior rather than hashing raw bytes in Rust.
    void Run() override {
        Check();
        std::unique_ptr<FileSys> original(FileSys::Create(FST_BINARY));
        if (!original) throw std::runtime_error("Failed to create reconcile file probe");
        original->EnableStatCache();
        original->SetContentCharSetPriv(context->charset);
        original->Set(StrRef(path.c_str()));
        original->Stat();
        result.Set("time", std::to_string(original->StatModTime()));
        if (!context->digests && !context->types) {
            result.Set("fileSize", std::to_string(original->GetSize()));
            return;
        }
        Compare(*original);
    }
    // Preserve text conversion, symlink classification, and stat-only size shortcuts.
    void Compare(FileSys& original) {
        const auto type = original.CheckType();
        std::unique_ptr<FileSys> converted(FileSys::Create(type));
        if (converted) {
            converted->SetContentCharSetPriv(context->charset);
            converted->Set(StrRef(wire.c_str()));
            converted->Translator(translator.get());
        }
        auto* file = converted ? converted.get() : &original;
        Error error;
        StrBuf digest;
        const auto size = context->digests ? file->Digest(&digest, &error)
            : ClientSvc::SizeMatchesDepot(file->GetType(), translator.get())
                ? file->GetSize() : file->Digest(nullptr, &error);
        Check();
        if (error.Test()) {
            result.Set("digest", ""); result.Set("type", ""); result.Set("fileSize", "");
            return;
        }
        result.Set("digest", std::string(digest.Text(), digest.Length()));
        result.Set("fileSize", std::to_string(size));
        if (context->types) {
            Error message;
            const auto* name = clientCheckFileType(file, type, context->xfiles, 1, -1,
                nullptr, nullptr, nullptr, &message);
            result.Set("type", name ? name : "");
            if (!name) result.Set("typeError", "1");
        }
    }
    // Append candidates only on the connection thread for deterministic SDK batch output.
    void Commit(int) override {
        Check();
        if (result.Has("typeError")) scope.client->SetError();
        context->output.push_back(std::move(result));
    }
};
// Schedule a canonical content comparison without sharing mutable SDK objects.
void ScheduleFile(const std::shared_ptr<ScanContext>& context, const std::string& path, const std::string& wire) {
    context->scope.Submit(new AddFileTask(context, path, wire));
}
}
