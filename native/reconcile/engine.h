#ifndef P4RUST_RECONCILE_ENGINE_H
#define P4RUST_RECONCILE_ENGINE_H
#include "../sdk_hooks.h"
#include <strarray.h>
#include <strtable.h>
#include <rpc.h>
#include <timer.h>
#include <progress.h>
#include <client.h>
#include <clientprogressreport.h>
#include <clientservice.h>
#include <i18napi.h>
#include <charcvt.h>
#include <transdict.h>
#include <pathsys.h>
#include <mapapi.h>
#include <ignore.h>
#include <enviro.h>
#include <algorithm>
#include <utility>
#include <map>
#include <deque>
#include <optional>
#include "pipeline/cache.h"

namespace p4rust {
class User;
class Task;
class ReconcileScope {
    ReconcileScope* previous;
    p4rust_reconcile_v3 callback;
    void* scheduler;
    bool closed = false;
    std::deque<std::function<Task*()>> producers;
public:
    Client* client = nullptr;
    User& user;
    uint32_t move_workers;
    std::map<std::string, CachedDigest> digest_cache;
    // Install command-local scheduling without sharing state between connections.
    ReconcileScope(User&, p4rust_reconcile_v3, void*, uint32_t);
    // Join every Rust worker before SDK connection state is destroyed.
    ~ReconcileScope();
    // Dispatch a control operation without exposing network objects to workers.
    int Call(uint32_t operation);
    // Transfer one isolated task and its frozen RPC fields to Rust.
    void Submit(Task*);
    // Stop workers exactly once before the native session is finalized.
    void Close();
    // Mark a failed local stage without unwinding through SDK-owned temporary allocations.
    void Fail(const char*) noexcept;
    // Retain a lazy child supplier without recursively committing other completed tasks.
    void Defer(std::function<Task*()>);
    // Admit one deferred child only when Rust has a free task slot.
    bool Produce();
    // Deliver ready results and admit deferred local work on the owner thread.
    void Pump(bool);
    // Finish both deferred children and queued workers before advancing an SDK phase.
    void Drain();
    // Inspect lazy children and Rust-owned work together.
    bool Pending();
    // Read unmatched local paths without repeating the filesystem enumeration.
    bool Paths(const char*, std::vector<PathCandidate>&);
    // Retain final SDK output in the Rust result table.
    void Output(const std::vector<p4rust_field_v2>&);
    // Borrow one cached snapshot on the SDK connection thread.
    bool Snapshot(const char*, p4rust_local_v5&);
    // Expose only the active scheduling context on the connection thread.
    static ReconcileScope* Current();
};

class Fields {
    std::vector<std::pair<std::string, std::string>> values;
    const Fields* inherited = nullptr;
    size_t inherited_count = 0;
    // Find an owned override without allocating a temporary string.
    const std::pair<std::string, std::string>* Find(const std::string& name) const {
        for (const auto& field : values) if (field.first == name) return &field;
        return nullptr;
    }
    // Visit inherited entries in their original order and append only new overrides.
    template<class Consumer> void Each(Consumer consume) const {
        if (inherited) for (size_t index = 0; index < inherited_count; ++index) {
            const auto& field = inherited->values[index];
            const auto* override = Find(field.first);
            consume(override ? *override : field);
        }
        for (const auto& field : values) {
            bool present = false;
            if (inherited) for (size_t index = 0; index < inherited_count; ++index)
                if (inherited->values[index].first == field.first) { present = true; break; }
            if (!present) consume(field);
        }
    }
public:
    // Reuse an exclusively owned task's frozen metadata without duplicating every value.
    void Inherit(const Fields& source);
    // Copy SDK receive variables before the dispatcher reuses their storage.
    void Copy(StrDict*);
    // Replace one owned metadata value without retaining borrowed SDK pointers.
    void Set(const std::string&, const std::string&);
    // Borrow an inherited or owned value, using the empty optional-field fallback.
    const std::string& Get(const std::string&) const;
    // Preserve the distinction between a missing field and an empty field.
    bool Has(const std::string&) const;
    // Borrow bounded field descriptors during one synchronous FFI callback.
    std::vector<p4rust_field_v2> Frame() const;
    // Send a frozen confirmation without copying the dispatcher's current request.
    void Reply(Client*) const;
};
// Inspect interruption state on an independent local scanning worker.
bool ScanAlive();

class Task {
public:
    ReconcileScope& scope;
    Fields request, result;
    std::string diagnostic;
    std::optional<p4rust_local_v5> local;
    // Capture the command context and category for one exclusively owned job.
    Task(ReconcileScope&, const char*);
    // Release isolated SDK objects after worker execution and owner-thread commit.
    virtual ~Task() = default;
    // Compute local data without touching the connection.
    virtual void Run() = 0;
    // Apply a completed result only on its connection's owning thread.
    virtual void Commit(int) = 0;
    // Check the shared interruption callback without acquiring application locks.
    void Check() const;
    // Attach canonical local metadata without exposing it to the RPC confirmation.
    void Snapshot(const p4rust_local_v5&);
    // Install the task's immutable metadata on each SDK file interpretation.
    void Apply(FileSys&) const;
    // Borrow a matching local digest and its canonical byte count.
    bool Reuse(FileSys&, StrBuf&, offL_t* = nullptr) const;
    // Format a contextual SDK error before returning across the ABI.
    static void CheckError(Error&, const char*);
};
class TaskThread {
    Task* previous;
public:
    // Install interruption context while this thread owns a local SDK computation.
    explicit TaskThread(Task*);
    // Restore the preceding context before returning to the caller.
    ~TaskThread();
};
}
#endif
