#ifndef P4RUST_RECONCILE_CACHE_H
#define P4RUST_RECONCILE_CACHE_H
namespace p4rust {
struct PathCandidate {
    std::string path;
    p4rust_local_v5 snapshot;
};
struct CachedDigest {
    std::string digest;
    offL_t size;
    time_t time;
    FileSysType type;
    int charset;
    // Capture the file interpretation and state that produced a canonical digest.
    explicit CachedDigest(FileSys& file)
        : size(file.GetSize()), time(file.StatModTime()), type(file.GetType()),
          charset(file.GetContentCharSetPriv()) {}
    // Reuse contents only when the current SDK interpretation and file state still agree.
    bool Matches(FileSys& file) const {
        return type == file.GetType() && charset == file.GetContentCharSetPriv() &&
            size == file.GetSize() && time == file.StatModTime();
    }
};
}
#endif
