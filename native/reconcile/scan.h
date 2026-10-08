#ifndef P4RUST_RECONCILE_SCAN_H
#define P4RUST_RECONCILE_SCAN_H
namespace p4rust {
struct ScanContext {
    struct Mapping { std::string left, right; MapType type; };
    ReconcileScope& scope;
    Fields metadata;
    std::vector<Mapping> mappings;
    StrArray* known;
    std::vector<Fields> output;
    std::unique_ptr<CharSetCvt> contents, names;
    std::string ignore, config;
    bool traverse, no_ignore, digests, types, nocase;
    int charset, xfiles;
    ClientProgressReport* progress;
    // Freeze mapping, ignore, charset, and server options before directory workers start.
    ScanContext(ReconcileScope& owner, Client* client, MapApi*, StrArray* has,
        const char* configuration, int recursive, int ignore_disabled, int digest, int type,
        ClientProgressReport* reporter)
        : scope(owner), known(has), ignore(client->GetIgnoreFile().Text()), config(configuration ? configuration : ""),
          traverse(recursive != 0), no_ignore(ignore_disabled != 0), digests(digest != 0),
          types(type != 0), nocase(client->protocolNocase != 0), charset(client->content_charset),
          xfiles(client->protocolXfiles), progress(reporter) {
        metadata.Copy(client);
        for (int i = 0; auto* value = client->GetVar(StrRef("mapTable"), i); ++i) {
            std::string path = value->Text();
            MapType flag = MapInclude;
            if (!path.empty() && (path.front() == '-' || path.front() == '+' || path.front() == '&')) {
                flag = path.front() == '-' ? MapExclude : path.front() == '+' ? MapOverlay : MapInclude;
                path.erase(0, 1);
            }
#ifdef OS_NT
            std::replace(path.begin(), path.end(), '\\', '/');
#endif
            mappings.push_back({path, path, flag});
        }
        if (auto* cvt = ClientSvc::XCharset(client, FromClient)) contents.reset(cvt->Clone());
        if (client != client->translated) names.reset(static_cast<TransDict*>(client->transfname)->ToCvt()->Clone());
    }
    // Check the SDK's sorted known-file list without changing global case-folding state.
    bool Known(const char* path) const {
        if (!known) return false;
        int first = 0, last = known->Count();
        while (first < last) {
            const int middle = first + (last - first) / 2;
            const int order = known->Get(middle)->SCompare(StrRef(path));
            if (!order) return true;
            if (order < 0) first = middle + 1;
            else last = middle;
        }
        return false;
    }
};
// Construct one independently owned canonical file probe on the connection thread.
void ScheduleFile(const std::shared_ptr<ScanContext>&, const std::string&, const std::string&);
// Construct one independently owned directory scan on the connection thread.
void ScheduleDirectory(const std::shared_ptr<ScanContext>&, const std::string&);
}
#endif
