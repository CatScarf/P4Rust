namespace p4rust {
class EditTask final : public Task {
    std::unique_ptr<FileSys> file;
    std::unique_ptr<CharSetCvt> translator;
    bool store = false;
public:
    // Freeze server metadata and isolate the file translator before enqueueing the probe.
    EditTask(ReconcileScope& scope, Client* client, FileSys* input) : Task(scope, "tracked"), file(input) {
        request.Copy(client);
        request.Set("localPath", file->Name());
        result = request;
        if (auto* cvt = ClientSvc::XCharset(client, FromClient)) translator.reset(cvt->Clone());
    }
    // Preserve the SDK's missing, symlink, size, time, and digest comparison order.
    void Run() override {
        Check();
        result.Set("canonicalType", std::to_string(file->GetType()));
        Apply(*file);
        const int stat = file->Stat();
        const bool missing = !(stat & (FSF_SYMLINK | FSF_EXISTS));
        const bool different_link = ((stat & FSF_SYMLINK) != 0) != (file->IsSymlink() != 0);
        std::string status = missing ? "missing" : "exists";
        store = !missing && (different_link || request.Has("digest"));
        if (!missing && !different_link && request.Has("digest")) status = Compare();
        result.Set("status", status);
    }
    // Compare canonical SDK contents and use timestamps only when the original branch permits it.
    std::string Compare() {
        Error error;
        StrBuf digest;
        if (request.Has("digestType")) {
            const auto type = request.Get("digestType");
            FileDigestType kind = FS_DIGEST_UNKNOWN;
            if (type == P4Tag::v_digestTypeMD5) kind = FS_DIGEST_MD5;
            else if (type == P4Tag::v_digestTypeGitText) kind = FS_DIGEST_GIT_TEXT_SHA1;
            else if (type == P4Tag::v_digestTypeGitBinary) kind = FS_DIGEST_GIT_BINARY_SHA1;
            else if (type == P4Tag::v_digestTypeSHA256) kind = FS_DIGEST_SHA256;
            file->ComputeDigest(kind, &digest, &error);
            result.Set("hashedBytes", std::to_string(file->GetSize()));
        } else {
            const auto size_text = request.Get("fileSize");
            StrRef expected_size(size_text.c_str());
            const auto size = expected_size.Atoi64();
            if (size && size != file->GetSize()) return "exists";
            file->Translator(translator.get());
            const int time = static_cast<int>(file->StatModTime());
            const auto timestamp = request.Get("time");
            if (request.Has("time") && time == StrRef(timestamp.c_str()).Atoi()) {
                result.Set("timestampMatch", "1"); return "same";
            }
            if (Reuse(*file, digest)) result.Set("cachedDigest", "1");
            else {
                file->Digest(&digest, &error);
                result.Set("hashedBytes", std::to_string(file->GetSize()));
            }
            if (!error.Test() && !digest.XCompare(StrRef(request.Get("digest").c_str()))) {
                result.Set("time", std::to_string(time));
                return "same";
            }
        }
        Check();
        // The original SDK treats unreadable contents as different and clears the read error.
        return !error.Test() && !digest.XCompare(StrRef(request.Get("digest").c_str())) ? "same" : "exists";
    }
    // Update the original SDK handle and send only this request's frozen confirmation.
    void Commit(int override_status) override {
        Check();
        if (override_status >= 0) {
            if (override_status > 2) throw std::runtime_error("Failed to validate reconcile status override");
            result.Set("status", override_status == 0 ? "same" : override_status == 1 ? "missing" : "exists");
        }
        const bool missing = result.Get("status") == "missing";
        NoteEdit(scope.client, file->Name(), missing, !missing && store);
        if (!result.Has("type")) result.Set("type", "text");
        result.Reply(scope.client);
    }
};
// Transfer the prepared SDK file and its metadata only when a Rust engine is installed.
bool ScheduleEdit(Client* client, FileSys* file) {
    auto* scope = ReconcileScope::Current();
    if (!scope) return false;
    scope->client = client;
    std::unique_ptr<FileSys> owned(file);
    try { scope->Submit(new EditTask(*scope, client, owned.release())); }
    catch (const std::exception& error) { scope->Fail(error.what()); }
    return true;
}

struct ExactGroup {
    Fields request;
    int first = -1;
    std::string path, index;
};
class ExactTask final : public Task {
    std::unique_ptr<FileSys> file;
    std::unique_ptr<CharSetCvt> translator;
    std::shared_ptr<ExactGroup> group;
    int position;
    std::optional<CachedDigest> cached;
public:
    // Capture one candidate and reuse only owner-committed SDK digest results.
    ExactTask(ReconcileScope& owner, FileSys* input, std::shared_ptr<ExactGroup> batch, int candidate)
        : Task(owner, "exact"), file(input), group(std::move(batch)), position(candidate) {
        request = group->request;
        request.Set("kind", "exact"); request.Set("localPath", file->Name());
        request.Set("candidate", std::to_string(position));
        if (auto* cvt = ClientSvc::XCharset(scope.client, FromClient)) translator.reset(cvt->Clone());
        const auto found = scope.digest_cache.find(file->Name());
        if (found != scope.digest_cache.end()) cached = found->second;
    }
    // Compare a move candidate using SDK type, symlink, charset, and canonical digest rules.
    void Run() override {
        Check();
        Apply(*file);
        const auto stat = file->Stat();
        result.Set("matched", "0");
        if (!(stat & (FSF_SYMLINK | FSF_EXISTS)) ||
            (((stat & FSF_SYMLINK) != 0) != (file->IsSymlink() != 0)) || !request.Has("digest")) return;
        if (!cached || !cached->Matches(*file)) {
            cached.emplace(*file);
            file->Translator(translator.get());
            Error error;
            StrBuf digest;
            file->Digest(&digest, &error);
            result.Set("hashedBytes", std::to_string(file->GetSize()));
            Check();
            if (error.Test()) return;
            result.Set("digest", std::string(digest.Text(), digest.Length()));
            if (!cached->Matches(*file)) cached.reset();
        } else result.Set("digest", cached->digest);
        if (cached) cached->digest = result.Get("digest");
        const auto local = result.Get("digest"), expected = request.Get("digest");
        if (!StrRef(local.c_str()).XCompare(StrRef(expected.c_str()))) result.Set("matched", "1");
    }
    // Select the earliest original candidate independently of worker completion order.
    void Commit(int) override {
        Check();
        if (cached && result.Has("digest")) scope.digest_cache.insert_or_assign(file->Name(), *cached);
        if (result.Get("matched") == "1" && (group->first < 0 || position < group->first)) {
            group->first = position;
            group->path = request.Get("toFile" + std::to_string(position));
            group->index = request.Get("index" + std::to_string(position));
        }
    }
};
// Freeze a digest-match RPC, parallelize candidates, and confirm once on its owning connection.
bool ScheduleExact(Client* client) {
    auto* scope = ReconcileScope::Current();
    if (!scope) return false;
    scope->client = client;
    try {
        scope->Drain();
        auto group = std::make_shared<ExactGroup>();
        group->request.Copy(client);
        for (int i = 0; client->GetVar(StrRef(P4Tag::v_toFile), i); ++i) {
            auto* index = client->GetVar(StrRef(P4Tag::v_index), i);
            if (!index || AlreadyMatched(client, index->Atoi())) continue;
            Error error;
            StrVarName name(StrRef(P4Tag::v_toFile), i);
            std::unique_ptr<FileSys> file(ClientSvc::FileFromPath(client, name.Text(), &error));
            if (error.Test() || !file) continue;
            scope->Submit(new ExactTask(*scope, file.release(), group, i));
        }
        scope->Drain();
        Fields reply = group->request;
        if (group->first >= 0) {
            reply.Set("toFile", group->path); reply.Set("index", group->index);
            NoteExact(client, StrRef(group->index.c_str()).Atoi(), true);
        } else NoteExact(client, 0, false);
        reply.Reply(client);
    } catch (const std::exception& error) { scope->Fail(error.what()); }
    return true;
}
}
