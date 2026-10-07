namespace p4rust {
struct MatchState {
    std::function<void()> function;
    ReconcileScope* scope;
    std::atomic<bool> done{false};
    // Retain the original SDK comparison until its borrowed sequence is joined.
    MatchState(std::function<void()> work, ReconcileScope* owner) : function(std::move(work)), scope(owner) {}
};
class MoveTask final : public Task {
    std::shared_ptr<MatchState> match;
    bool started = false;
public:
    // Transfer SDK content matching into the independently limited Rust move pool.
    explicit MoveTask(std::shared_ptr<MatchState> state) : Task(*state->scope, "move"), match(std::move(state)) {
        request.Copy(scope.client);
        request.Set("kind", "move");
    }
    // Release SDK comparison inputs even if a custom handler rejects this task.
    ~MoveTask() override {
        if (!started) {
            try { ThreadScope thread; TaskThread context(this); Run(); }
            catch (const std::exception& error) {
                std::fprintf(stderr, "Failed to release rejected move task: %s\n", error.what());
            }
        }
    }
    // Complete the SDK algorithm before reporting cancellation or releasing borrowed sequences.
    void Run() override {
        started = true;
        try { match->function(); }
        catch (...) { match->done.store(true); throw; }
        match->done.store(true);
        result.Set("completed", "1");
    }
    // Leave match selection and acknowledgements with the SDK's original owner-thread algorithm.
    void Commit(int) override {}
};
// Use Rust workers only when a command has explicitly installed a reconcile engine.
void MatchThread::Start(std::function<void()> function) {
    auto* scope = ReconcileScope::Current();
    if (!scope) { fallback = std::make_unique<std::thread>(std::move(function)); return; }
    state = std::make_shared<MatchState>(std::move(function), scope);
    try { scope->Submit(new MoveTask(state)); }
    catch (const std::exception& error) { scope->Fail(error.what()); }
}
// Join all borrowing workers before SDK stack-owned sequences can unwind.
void MatchThread::join() {
    if (fallback) { if (fallback->joinable()) fallback->join(); return; }
    if (!state) return;
    try { state->scope->Call(4); }
    catch (const std::exception& error) { state->scope->Fail(error.what()); }
    if (!state->done.load()) state->scope->Fail("Unfinished SDK move comparison");
    state.reset();
}
// Finish outstanding comparisons when SDK dispatch unwinds through an error.
MatchThread::~MatchThread() {
    try { join(); }
    catch (const std::exception& error) { std::fprintf(stderr, "Failed to join SDK move comparison: %s\n", error.what()); }
}
// Honor the Rust move pool limit while preserving ordinary SDK command execution.
int MoveWorkers(Client* client, int original) {
    auto* scope = ReconcileScope::Current();
    if (!scope) return original;
    scope->client = client;
    return static_cast<int>(scope->move_workers);
}
}
