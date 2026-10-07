namespace p4rust {
class TaskBoundary {
public:
    // Contain all native exceptions and retain a diagnostic in the owned task.
    template<class Function> static int Run(Task* task, Function function) {
        if (!task) return 1;
        try { function(); return 0; }
        catch (const std::exception& error) { task->diagnostic = error.what(); }
        catch (...) { task->diagnostic = "Failed to process reconcile task: unknown native exception"; }
        return 1;
    }
};
}
// Execute an exclusively owned task with independent SDK thread-local initialization.
extern "C" int32_t p4rust_reconcile_execute_v3(void* pointer) {
    auto* task = static_cast<p4rust::Task*>(pointer);
    return p4rust::TaskBoundary::Run(task, [task] {
        p4rust::ThreadScope thread;
        p4rust::TaskThread context(task);
        task->Run();
    });
}
// Borrow computed fields for one synchronous Rust-owned result copy.
extern "C" int32_t p4rust_reconcile_result_v3(void* pointer, p4rust_callback_v1 callback, void* context) {
    auto* task = static_cast<p4rust::Task*>(pointer);
    return p4rust::TaskBoundary::Run(task, [task, callback, context] {
        if (!callback) throw std::runtime_error("Failed to read reconcile result: missing callback");
        auto fields = task->result.Frame();
        if (callback(context, P4RUST_RECORD, reinterpret_cast<const uint8_t*>(fields.data()),
                fields.size() * sizeof(p4rust_field_v2), nullptr, 0) != 0)
            throw std::runtime_error("Failed to copy reconcile result callback");
    });
}
// Apply local results exclusively on the original SDK connection thread.
extern "C" int32_t p4rust_reconcile_commit_v3(void* pointer, int32_t status) {
    auto* task = static_cast<p4rust::Task*>(pointer);
    return p4rust::TaskBoundary::Run(task, [task, status] { task->Commit(status); });
}
// Borrow a diagnostic until Rust releases the task handle.
extern "C" const uint8_t* p4rust_reconcile_error_v3(void* pointer, size_t* length) {
    auto* task = static_cast<p4rust::Task*>(pointer);
    if (!length) return nullptr;
    *length = task ? task->diagnostic.size() : 0;
    return task ? reinterpret_cast<const uint8_t*>(task->diagnostic.data()) : nullptr;
}
// Release task-local files after execution and owner-thread confirmation have finished.
extern "C" void p4rust_reconcile_drop_v3(void* pointer) { delete static_cast<p4rust::Task*>(pointer); }
