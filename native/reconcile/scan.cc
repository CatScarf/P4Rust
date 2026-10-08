#include "scan.h"
#include "scanfile.cc"
#include "scandir.cc"
namespace p4rust {
class PipelineCandidates {
public:
    // Apply the server's mappings and SDK ignore policy once to unmatched local slots.
    static void Schedule(const std::shared_ptr<ScanContext>& context, const std::vector<PathCandidate>& paths, MapApi* map, const char* directory) {
        Ignore ignore;
        const char* config = context->config.empty() ? nullptr : context->config.c_str();
        for (const auto& candidate : paths) {
            const auto& path = candidate.path;
            if (context->Known(path.c_str())) continue;
            if (!context->traverse && path.substr(0, path.find_last_of("/\\")) != directory) continue;
            if (!context->no_ignore && ignore.Reject(StrRef(path.c_str()), StrRef(context->ignore.c_str()), config)) continue;
            std::string wire = path;
            if (context->names) {
                const auto* text = context->names->FastCvt(path.c_str(), static_cast<int>(path.size()), 0);
                if (text) wire = text;
            }
            std::string mapped = wire;
            const int stat = candidate.snapshot.stat;
            if (!(stat & (FSF_EXISTS | FSF_SYMLINK))) continue;
            if ((stat & FSF_DIRECTORY) && (stat & FSF_SYMLINK)) mapped += "/";
#ifdef OS_NT
            std::replace(mapped.begin(), mapped.end(), '\\', '/');
#endif
            StrBuf destination;
            if (map->Translate(StrRef(mapped.c_str()), destination)) {
                auto task = std::make_unique<AddFileTask>(context, path, wire);
                task->Snapshot(candidate.snapshot);
                context->scope.Submit(task.release());
            }
        }
    }
};
// Parallelize directory and content work while preserving the original SDK candidate arrays.
bool Traverse(Client* client, const char* dir, int traverse, int no_ignore, int get_digests,
    int get_types, MapApi* map, StrArray* files, StrArray* sizes, StrArray* times,
    StrArray* digests, StrArray* types, int&, StrArray* known, const char* config,
    ClientProgressReport* progress, Error*) {
    auto* scope = ReconcileScope::Current();
    if (!scope) return false;
    scope->client = client;
    try {
    scope->Drain();
    auto context = std::make_shared<ScanContext>(*scope, client, map, known, config,
        traverse, no_ignore, get_digests, get_types, progress);
    std::vector<PathCandidate> paths;
    if (scope->Paths(dir, paths)) PipelineCandidates::Schedule(context, paths, map, dir);
    else ScheduleDirectory(context, dir);
    scope->Drain();
    std::sort(context->output.begin(), context->output.end(), [](const Fields& a, const Fields& b) {
        const auto left = a.Get("file"), right = b.Get("file");
        return StrRef(left.c_str()).SCompare(StrRef(right.c_str())) < 0;
    });
    for (const auto& file : context->output) {
        files->Put()->Set(file.Get("file").c_str());
        times->Put()->Set(file.Get("time").c_str());
        if (sizes) sizes->Put()->Set(file.Get("fileSize").c_str());
        if (get_digests && digests) digests->Put()->Set(file.Get("digest").c_str());
        if (get_types && types) types->Put()->Set(file.Get("type").c_str());
    }
    } catch (const std::exception& error) { scope->Fail(error.what()); }
    return true;
}
}
