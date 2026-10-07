#include "scan.h"
#include "scanfile.cc"
#include "scandir.cc"
namespace p4rust {
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
    ScheduleDirectory(context, dir);
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
