namespace p4rust {
static thread_local ReconcileScope* reconcile_scope = nullptr;
static thread_local Task* reconcile_task = nullptr;

// Capture this thread's command-local native scheduling scope.
ReconcileScope::ReconcileScope(User& owner, p4rust_reconcile_v3 hook, void* context, uint32_t moves)
    : previous(reconcile_scope), callback(hook), scheduler(context), user(owner), move_workers(moves) {
    if (callback) reconcile_scope = this;
}
// Stop all workers during unwinding before any borrowed SDK state expires.
ReconcileScope::~ReconcileScope() {
    if (!closed) {
        try { Close(); }
        catch (const std::exception& error) { std::fprintf(stderr, "Failed to stop reconcile workers: %s\n", error.what()); }
    }
    reconcile_scope = previous;
}
// Inspect the active scope only on the native connection thread.
ReconcileScope* ReconcileScope::Current() { return reconcile_scope; }
// Keep scheduling opt-in so ordinary SDK commands retain their original behavior.
bool ActiveReconcile() { return ReconcileScope::Current() != nullptr; }
// Execute one scheduling operation and retain callback failures as command errors.
int ReconcileScope::Call(uint32_t operation) {
    if (!callback) return 0;
    int result = callback(scheduler, operation, nullptr, nullptr, 0);
    if (result < 0) throw std::runtime_error("Failed to dispatch Rust reconcile scheduler");
    return result;
}
// Transfer ownership even when admission fails so Rust can clean up the job.
void ReconcileScope::Submit(Task* pointer) {
    std::unique_ptr<Task> task(pointer);
    auto fields = task->request.Frame();
    auto* owned = task.release();
    if (callback(scheduler, 1, owned, reinterpret_cast<const uint8_t*>(fields.data()),
                 fields.size() * sizeof(p4rust_field_v2)) != 0)
        throw std::runtime_error("Failed to schedule Rust reconcile request");
}
// Join native-data users before closing the SDK session.
void ReconcileScope::Close() {
    if (closed) return;
    closed = true;
    producers.clear();
    if (callback) Call(6);
}
// Keep child generation lazy so queue backpressure cannot create a recursive commit chain.
void ReconcileScope::Defer(std::function<Task*()> supplier) { producers.push_front(std::move(supplier)); }
// Reserve no more than one additional task before asking Rust to take ownership.
bool ReconcileScope::Produce() {
    if (closed || producers.empty() || !Call(7)) return false;
    auto* task = producers.front()();
    if (!task) { producers.pop_front(); return true; }
    Submit(task);
    return true;
}
// Pump completions before supplying another child without holding any scheduler mutex.
void ReconcileScope::Pump(bool wait) { Call(2); if (!Produce() && wait) Call(3); }
// Drain local child generation and worker replies iteratively instead of recursively.
void ReconcileScope::Drain() { while (Pending()) { if (!Produce()) Call(3); } }
// Include deferred children when deciding whether a stage has completed.
bool ReconcileScope::Pending() { return !producers.empty() || Call(5) != 0; }
// Stop the connection and join local work while keeping SDK stack cleanup intact.
void ReconcileScope::Fail(const char* message) noexcept {
    user.command_failed.store(true);
    try { user.Emit(P4RUST_ERROR, message, std::strlen(message)); }
    catch (const std::exception& error) { std::fprintf(stderr, "Failed to deliver reconcile error: %s\n", error.what()); }
    if (user.interrupt) user.interrupt->failed.store(true);
    std::fprintf(stderr, "Failed to execute reconcile stage: %s\n", message);
    try { Close(); }
    catch (const std::exception& error) { std::fprintf(stderr, "Failed to close reconcile workers: %s\n", error.what()); }
}
// Copy the receive dictionary into stable command-owned strings.
void Fields::Copy(StrDict* dictionary) {
    StrRef key, value;
    for (int index = 0; dictionary->GetVar(index, key, value); ++index)
        Set(std::string(key.Text(), key.Length()), std::string(value.Text(), value.Length()));
}
// Retain exactly one owned entry for a field name.
void Fields::Set(const std::string& name, const std::string& value) {
    for (auto& field : values) if (field.first == name) { field.second = value; return; }
    values.emplace_back(name, value);
}
// Read an optional owned field without inserting or modifying the dictionary.
std::string Fields::Get(const std::string& name) const {
    for (const auto& field : values) if (field.first == name) return field.second;
    return {};
}
// Distinguish missing protocol values from empty values.
bool Fields::Has(const std::string& name) const {
    for (const auto& field : values) if (field.first == name) return true;
    return false;
}
// Bound protocol metadata before constructing borrowed FFI descriptors.
std::vector<p4rust_field_v2> Fields::Frame() const {
    if (values.size() > 16384) throw std::runtime_error("Failed to bound reconcile metadata fields");
    size_t bytes = 0;
    std::vector<p4rust_field_v2> frame;
    frame.reserve(values.size());
    for (const auto& field : values) {
        if (field.first.size() > 1048576 - bytes || field.second.size() > 1048576 - bytes - field.first.size())
            throw std::runtime_error("Failed to bound reconcile metadata: exceeds 1 MiB");
        bytes += field.first.size() + field.second.size();
        frame.push_back({reinterpret_cast<const uint8_t*>(field.first.data()), field.first.size(),
                         reinterpret_cast<const uint8_t*>(field.second.data()), field.second.size()});
    }
    return frame;
}
// Reproduce Confirm's field copying from the original request rather than the current buffer.
void Fields::Reply(Client* client) const {
    if (client->protocolServer < 6) client->GetEnv();
    for (const auto& field : values) {
        if (field.first == "func" || field.first == "data" || field.first == "kind" || field.first == "localPath") continue;
        StrRef key(field.first.data(), static_cast<int>(field.first.size()));
        StrRef value(field.second.data(), static_cast<int>(field.second.size()));
        client->SetVar(key, value);
    }
    const auto confirm = Get("confirm");
    if (confirm.empty()) throw std::runtime_error("Failed to confirm reconcile request: missing callback");
    client->Invoke(confirm.c_str());
}
// Capture one command-local task category without borrowing RPC buffers.
Task::Task(ReconcileScope& owner, const char* kind) : scope(owner) { request.Set("kind", kind); result.Set("kind", kind); }
// Check cancellation before each local filesystem operation.
void Task::Check() const { if (scope.user.interrupt) scope.user.interrupt->Check(); }
// Preserve SDK diagnostics at native computation boundaries.
void Task::CheckError(Error& error, const char* operation) {
    if (!error.Test()) return;
    StrBuf text;
    error.Fmt(&text);
    throw std::runtime_error(std::string(operation) + ": " + text.Text());
}
// Report pending work only for the active command's SDK connection.
bool Pending(Rpc* rpc) {
    auto* scope = ReconcileScope::Current();
    if (!scope || scope->client != rpc || (scope->user.interrupt && !scope->user.interrupt->IsAlive())) return false;
    try { return scope->Pending(); }
    catch (const std::exception& error) { scope->Fail(error.what()); return false; }
}
// Pump worker replies exclusively from the SDK dispatcher thread.
void Pump(Rpc* rpc, bool wait) {
    auto* scope = ReconcileScope::Current();
    if (scope && scope->client == rpc) {
        try { scope->Pump(wait); }
        catch (const std::exception& error) { scope->Fail(error.what()); }
    }
}
// Drain all local work before the SDK advances to a dependent reconcile phase.
void Barrier(Client* client) {
    auto* scope = ReconcileScope::Current();
    if (scope && scope->client == client) {
        try { scope->Drain(); }
        catch (const std::exception& error) { scope->Fail(error.what()); }
    }
}
// Poll command cancellation from an isolated SDK worker without accessing its connection.
bool WorkerAlive() {
    auto* scope = reconcile_task ? &reconcile_task->scope : ReconcileScope::Current();
    return !scope || !scope->user.interrupt || scope->user.interrupt->IsAlive();
}
// Install task-local cancellation for canonical SDK reads on this worker.
TaskThread::TaskThread(Task* task) : previous(reconcile_task) { reconcile_task = task; }
// Restore task-local cancellation after SDK reads and converter cleanup finish.
TaskThread::~TaskThread() { reconcile_task = previous; }
}

#include "edit.cc"
#include "scan.cc"
#include "moves.cc"
#include "exports.cc"
