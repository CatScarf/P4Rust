#ifndef P4RUST_SETTINGS_H
#define P4RUST_SETTINGS_H
namespace p4rust {
class LocalSettings {
public:
    // Resolve only non-secret SDK settings without opening a connection.
    static void Discover(Session& session, User& user) {
        auto& client = session.client;
        StrBufDict record;
        record.SetVar("port", client.GetPort().Text());
        record.SetVar("user", client.GetUser().Text());
        record.SetVar("client", client.GetClient().Text());
        record.SetVar("cwd", client.GetCwd().Text());
        record.SetVar("charset", client.GetCharset().Text());
        record.SetVar("ignore_file", client.GetIgnoreFile().Text());
        user.OutputStat(&record);
    }

    // Match P4Python's SDK ignore query with per-instance configuration.
    static void Ignored(Session& session, User& user, const char* path) {
        auto& client = session.client;
        StrBufDict record;
        const bool ignored = client.GetIgnore()->Reject(StrRef(path), client.GetIgnoreFile());
        record.SetVar("ignored", ignored ? "1" : "0");
        user.OutputStat(&record);
    }
};
}
#endif
