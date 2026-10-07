#include "client.h"
#include <clientapi.h>
#include <p4libs.h>
#include <clientprog.h>
#include <array>
#include <atomic>
#include <mutex>
#include <cstdio>
#include <cstring>
#include <stdexcept>
#include <string>
#include <vector>

namespace p4rust {
class Runtime {
public:
    bool initialized = false;

    // Initialize Perforce runtime support once for the process.
    Runtime() {
        Error error;
        P4Libraries::Initialize(P4LIBRARIES_INIT_ALL, &error);
        if (error.Test()) {
            StrBuf message;
            error.Fmt(&message);
            throw std::runtime_error("Failed to initialize P4 runtime: " +
                                     std::string(message.Text()));
        }
        initialized = true;
        P4Libraries::DisableFileSysCreateOnIntr();
    }

    // Report runtime cleanup failures during exception unwinding.
    ~Runtime() {
        if (initialized) {
            Error error;
            P4Libraries::Shutdown(P4LIBRARIES_INIT_ALL, &error);
            Report(error);
        }
    }

    // Report cleanup errors that cannot propagate through a destructor.
    static void Report(Error& error) {
        if (!error.Test()) return;
        StrBuf message;
        error.Fmt(&message);
        std::fprintf(stderr, "Failed to clean up P4 resources: %s\n", message.Text());
    }

};

class ThreadScope {
public:
    // Initialize the Perforce runtime state of the calling thread.
    ThreadScope() {
        Error error;
        P4Libraries::InitializeThread(P4LIBRARIES_INIT_ALL, &error);
        if (error.Test()) {
            StrBuf message;
            error.Fmt(&message);
            throw std::runtime_error("Failed to initialize P4 thread: " +
                                     std::string(message.Text()));
        }
    }

    // Release thread-local runtime state after the client is destroyed.
    ~ThreadScope() {
        Error error;
        P4Libraries::ShutdownThread(P4LIBRARIES_INIT_ALL, &error);
        Runtime::Report(error);
    }
};

class Interrupt final : public KeepAlive {
public:
    std::atomic<bool> failed{false};
    p4rust_alive_v1 callback;
    void* context;

    // Request disconnection only from the native session's owning thread.
    int IsAlive() override { return !failed.load() && (!callback || callback(context) != 0); }

    // Poll cancellation without imposing the SDK's default half-second delay.
    int PollMs() override { return 50; }

    // Stop before connection setup or command dispatch when cancellation is already requested.
    void Check() {
        if (!IsAlive()) throw std::runtime_error("Failed to execute P4 command: interrupted");
    }
};

class User final : public ClientUser {
public:
    p4rust_callback_v1 callback;
    void* context;
    std::string input;
    Interrupt* interrupt = nullptr;
    std::recursive_mutex mutex;
    std::atomic<int64_t> next_progress{0};
    std::atomic<bool> command_failed{false};

    // Copy event bytes to the caller without sharing allocator ownership.
    void Emit(uint32_t event, const char* data = nullptr, size_t length = 0,
              const char* value = nullptr, size_t value_length = 0) {
        std::lock_guard<std::recursive_mutex> lock(mutex);
        if (interrupt && !interrupt->IsAlive()) return;
        if (callback(context, event, reinterpret_cast<const uint8_t*>(data), length,
                     reinterpret_cast<const uint8_t*>(value), value_length) != 0)
            if (interrupt) interrupt->failed.store(true);
    }

    // Supply explicit form input without interactive prompts.
    void InputData(StrBuf* buffer, Error*) override { buffer->Set(input.c_str()); }

    // Supply explicit password input without blocking for an interactive prompt.
    void Prompt(const StrPtr&, StrBuf& buffer, int, Error* error) override {
        if (input.empty())
            error->Set(E_FAILED, "Interactive prompts are unsupported; provide explicit input.");
        else
            buffer.Set(input.c_str());
    }

    // Collect text while preserving the provided byte count.
    void OutputText(const char* data, int length) override {
        Emit(P4RUST_TEXT, data, static_cast<size_t>(length));
    }

    // Collect informational output from the server.
    void OutputInfo(char level, const char* data) override {
        Emit(P4RUST_INFO, data, std::strlen(data), &level, 1);
    }

    // Preserve binary command output without text conversion.
    void OutputBinary(const char* data, int length) override {
        Emit(P4RUST_BINARY, data, static_cast<size_t>(length));
    }

    // Preserve each tagged record and its field order.
    void OutputStat(StrDict* dictionary) override {
        Record(dictionary, P4RUST_RECORD);
    }

    // Preserve a partial tagged callback while retaining the SDK's default accumulation.
    int OutputStatPartial(StrDict* dictionary) override {
        Record(dictionary, P4RUST_RECORD_PARTIAL);
        return 0;
    }

    // Copy an atomic tagged callback with its original field order.
    void Record(StrDict* dictionary, uint32_t event) {
        std::lock_guard<std::recursive_mutex> lock(mutex);
        Emit(event);
        StrRef key, value;
        for (int index = 0; dictionary->GetVar(index, key, value); ++index)
            Emit(P4RUST_FIELD, key.Text(), key.Length(), value.Text(), value.Length());
        Emit(P4RUST_RECORD_END);
    }

    // Enable SDK progress notifications for ordinary and parallel transfers.
    int ProgressIndicator() override { return 1; }

    // Allow independent progress objects in parallel transfer workers.
    int CanParallelProgress() override { return 1; }

    // Construct an owned progress handler with a unique operation identity.
    ClientProgress* CreateProgress(int type) override;

    // Construct progress metadata with the SDK's supplied transfer size.
    ClientProgress* CreateProgress(int type, P4INT64 size) override;
    // Preserve structured server messages without the default callback redirection.
    void Message(Error* error) override { ErrorEvent(error, P4RUST_MESSAGE); }

    // Preserve legacy structured errors and warnings as distinct SDK callbacks.
    void HandleError(Error* error) override {
        ErrorEvent(error, P4RUST_HANDLE_ERROR);
    }

    // Serialize every SDK error identifier, format, and parameter without losing metadata.
    void ErrorEvent(Error* error, uint32_t event) {
        std::lock_guard<std::recursive_mutex> lock(mutex);
        StrBuf message;
        error->Fmt(&message);
        StrBuf serialized;
        error->Marshall2(serialized);
        if (error->GetSeverity() >= E_FAILED) command_failed.store(true);
        Emit(event, message.Text(), message.Length(), serialized.Text(), serialized.Length());
    }

    // Capture legacy server errors instead of printing them to the console.
    void OutputError(const char* message) override {
        command_failed.store(true);
        Emit(P4RUST_OUTPUT_ERROR, message, std::strlen(message));
    }

    // Forward SDK completion separately from post-cleanup command completion.
    void Finished() override { Emit(P4RUST_FINISHED); }
};

class Progress final : public ClientProgress {
public:
    User& user;
    std::array<int64_t, 7> values;
    std::string description;
    std::mutex mutex;

    // Initialize isolated metadata for one SDK progress operation.
    Progress(User& owner, int type, int64_t total)
        : user(owner), values{owner.next_progress.fetch_add(1), type, 0, total, 0, 0, 0} {}

    // Publish a bounded description and its SDK measurement unit.
    void Description(const StrPtr* text, int units) override {
        std::lock_guard<std::mutex> lock(mutex);
        description.assign(text->Text(), text->Length());
        values[2] = units;
        values[5] = 1;
        Emit();
    }

    // Publish the expected amount of work when the SDK supplies it.
    void Total(long total) override {
        std::lock_guard<std::mutex> lock(mutex);
        values[3] = total; values[5] = 2; Emit();
    }

    // Publish completed work and cooperate with cancellation from any SDK worker.
    int Update(long current) override {
        std::lock_guard<std::mutex> lock(mutex);
        values[4] = current;
        values[5] = 3;
        Emit();
        return user.interrupt && !user.interrupt->IsAlive();
    }

    // Publish the final progress status without completing the enclosing command.
    void Done(int fail) override {
        std::lock_guard<std::mutex> lock(mutex);
        values[5] = 4; values[6] = fail; Emit();
    }

    // Copy fixed-width metadata through the synchronous callback contract.
    void Emit() {
        user.Emit(P4RUST_PROGRESS, description.data(), description.size(),
                  reinterpret_cast<const char*>(values.data()), sizeof(values));
    }
};

// Create an SDK progress operation with an initially unknown total.
ClientProgress* User::CreateProgress(int type) { return new Progress(*this, type, -1); }

// Create an SDK transfer progress operation with its initial size.
ClientProgress* User::CreateProgress(int type, P4INT64 size) { return new Progress(*this, type, size); }
class Session {
public:
    ClientApi client;
    bool initialized = false;

    // Finalize an initialized session during exception unwinding.
    ~Session() {
        if (initialized) {
            Error error;
            client.Final(&error);
            Runtime::Report(error);
        }
    }

    // Apply explicit connection settings before opening the session.
    void Configure(const p4rust_options_v1& options) {
        client.SetPort(options.port);
        client.SetUser(options.user);
        client.SetClient(options.client);
        if (options.cwd[0]) client.SetCwd(options.cwd);
        if (options.charset[0]) client.SetCharset(options.charset);
        client.SetProg("P4Rust");
        client.SetProtocol("tag", "");
    }

    // Translate connection and finalization failures to C++ exceptions.
    static void Check(Error& error, const char* operation) {
        if (!error.Test()) return;
        StrBuf message;
        error.Fmt(&message);
        throw std::runtime_error(std::string(operation) + ": " + message.Text());
    }

    // Open a native connection and pair it with exactly one finalization.
    void Open(User& user) {
        Error error;
        initialized = true;
        client.Init(&error);
        if (error.Test()) user.HandleError(&error);
        Check(error, "Failed to initialize P4 connection");
    }

    // Close the connection and report transport errors explicitly.
    void Close(User& user) {
        Error error;
        initialized = false;
        client.Final(&error);
        if (error.Test()) user.HandleError(&error);
        Check(error, "Failed to finalize P4 connection");
    }
};

// Execute a command while keeping argument storage alive through Run.
int32_t execute(const p4rust_options_v1& options, const char* command, int32_t argc,
             const char* const* args, p4rust_callback_v1 callback, void* context,
             p4rust_alive_v1 alive, void* control) {
    static Runtime runtime;
    ThreadScope thread;
    Interrupt interrupt;
    interrupt.callback = alive;
    interrupt.context = control;
    interrupt.Check();
    Session session;
    session.Configure(options);
    User user;
    user.callback = callback;
    user.context = context;
    user.input = options.input;
    user.interrupt = alive ? &interrupt : nullptr;
    if (alive) session.client.SetBreak(&interrupt);
    session.Open(user);
    interrupt.Check();
    std::vector<std::string> storage;
    std::vector<char*> pointers;
    storage.reserve(static_cast<size_t>(argc));
    for (int32_t index = 0; index < argc; ++index) storage.emplace_back(args[index]);
    for (auto& arg : storage) pointers.push_back(arg.data());
    session.client.SetArgv(static_cast<int>(pointers.size()), pointers.data());
    interrupt.Check();
    session.client.Run(command, &user);
    const int32_t errors = session.client.GetErrors();
    user.Emit(P4RUST_STATUS, reinterpret_cast<const char*>(&errors), sizeof(errors));
    session.Close(user);
    return errors != 0 || user.command_failed.load() || interrupt.failed.load() ? 1 : 0;
}
}

// Return the ABI contract implemented by this precompiled bridge.
extern "C" uint32_t p4rust_abi_version(void) { return 1; }

// Contain all C++ exceptions before returning across the C ABI boundary.
extern "C" int32_t p4rust_execute_v1(const p4rust_options_v1* options,
    const char* command, int32_t argc, const char* const* argv,
    p4rust_callback_v1 callback, void* context) {
    return p4rust_execute_controlled_v1(options, command, argc, argv, callback, context,
                                        nullptr, nullptr);
}

// Contain C++ exceptions and keep the interruption callback alive through session cleanup.
extern "C" int32_t p4rust_execute_controlled_v1(const p4rust_options_v1* options,
    const char* command, int32_t argc, const char* const* argv,
    p4rust_callback_v1 callback, void* context, p4rust_alive_v1 alive, void* control) {
    if (!callback) return 1;
    try {
        if (!options || options->abi_version != 1 || !command || argc < 0 ||
            (argc && !argv) || !options->port || !options->user || !options->client ||
            !options->cwd || !options->charset || !options->input || (alive && !control))
            throw std::runtime_error("Failed to validate ABI v1 arguments");
        for (int32_t index = 0; index < argc; ++index)
            if (!argv[index]) throw std::runtime_error("Failed to validate native argument");
        return p4rust::execute(*options, command, argc, argv, callback, context, alive, control);
    } catch (const std::exception& error) {
        callback(context, P4RUST_ERROR, reinterpret_cast<const uint8_t*>(error.what()),
                 std::strlen(error.what()), nullptr, 0);
    } catch (...) {
        const char* message = "Failed to execute P4 command: unknown native exception";
        callback(context, P4RUST_ERROR, reinterpret_cast<const uint8_t*>(message),
                 std::strlen(message), nullptr, 0);
    }
    return 1;
}
