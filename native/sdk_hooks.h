#ifndef P4RUST_SDK_HOOKS_H
#define P4RUST_SDK_HOOKS_H
#include <functional>
#include <memory>
#include <thread>
class Rpc;
class Client;
class FileSys;
class MapApi;
class StrArray;
class ClientProgressReport;
class Error;
namespace p4rust {
// Detect a command-local reconcile scheduler without reading shared process state.
bool ActiveReconcile();
// Transfer a tracked-file probe to the Rust scheduler after SDK preparation.
bool ScheduleEdit(Client*, FileSys*);
// Record a tracked-file result in the SDK's original reconcile handle.
void NoteEdit(Client*, const char*, bool, bool);
// Delegate exact move-candidate digest comparisons to isolated Rust workers.
bool ScheduleExact(Client*);
// Consult already selected move indices on the owning SDK connection thread.
bool AlreadyMatched(Client*, int);
// Commit exact-match selection and progress to the original SDK handle.
void NoteExact(Client*, int, bool);
// Delegate traversal while retaining the SDK's mapping and candidate arrays.
bool Traverse(Client*, const char*, int, int, int, int, MapApi*, StrArray*, StrArray*,
    StrArray*, StrArray*, StrArray*, int&, StrArray*, const char*, ClientProgressReport*, Error*);
// Drain completed replies on the connection's owning thread.
void Pump(Rpc*, bool);
// Inspect pending work before blocking for another server request.
bool Pending(Rpc*);
// Complete one stage before a dependent SDK callback starts.
void Barrier(Client*);
// Check cancellation while an isolated worker reads file contents.
bool WorkerAlive();
// Select command-local move parallelism without changing server command flags.
int MoveWorkers(Client*, int);
struct MatchState;
class MatchThread {
    std::shared_ptr<MatchState> state;
    std::unique_ptr<std::thread> fallback;
public:
    // Schedule the original SDK move comparison through the Rust move pool.
    template<class Function, class... Args>
    MatchThread(Function function, Args... args) { Start(std::bind(function, args...)); }
    // Preserve movable ownership for the SDK's bounded comparison vector.
    MatchThread(MatchThread&&) noexcept = default;
    // Preserve movable ownership when the SDK removes a completed comparison.
    MatchThread& operator=(MatchThread&&) noexcept = default;
    // Finish comparisons before their SDK sequence or result storage can expire.
    ~MatchThread();
    // Wait for the original SDK comparison to finish.
    void join();
private:
    // Select Rust scheduling only for commands with an active reconcile engine.
    void Start(std::function<void()>);
};
}
#endif
