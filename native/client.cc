#include "client.h"
#include <clientapi.h>
#include <p4libs.h>
#include <cstdio>
#include <cstring>
#include <mutex>
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

class User final : public ClientUser {
public:
    p4rust_callback_v1 callback;
    void* context;
    std::string input;

    // Copy event bytes to the caller without sharing allocator ownership.
    void Emit(uint32_t event, const char* data = nullptr, size_t length = 0,
              const char* value = nullptr, size_t value_length = 0) {
        if (callback(context, event, reinterpret_cast<const uint8_t*>(data), length,
                     reinterpret_cast<const uint8_t*>(value), value_length) != 0)
            throw std::runtime_error("Failed to collect native output");
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
    void OutputInfo(char, const char* data) override {
        Emit(P4RUST_TEXT, data, std::strlen(data));
        Emit(P4RUST_TEXT, "\n", 1);
    }

    // Preserve binary command output without text conversion.
    void OutputBinary(const char* data, int length) override {
        Emit(P4RUST_BINARY, data, static_cast<size_t>(length));
    }

    // Preserve each tagged record and its field order.
    void OutputStat(StrDict* dictionary) override {
        Emit(P4RUST_RECORD);
        StrRef key, value;
        for (int index = 0; dictionary->GetVar(index, key, value); ++index)
            Emit(P4RUST_FIELD, key.Text(), key.Length(), value.Text(), value.Length());
    }

    // Separate warning messages from command failures.
    void HandleError(Error* error) override {
        StrBuf message;
        error->Fmt(&message);
        const uint32_t event = error->GetSeverity() >= E_FAILED ? P4RUST_ERROR : P4RUST_WARNING;
        Emit(event, message.Text(), message.Length());
    }

    // Capture legacy server errors instead of printing them to the console.
    void OutputError(const char* message) override {
        Emit(P4RUST_ERROR, message, std::strlen(message));
    }
};

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
    void Open() {
        Error error;
        initialized = true;
        client.Init(&error);
        Check(error, "Failed to initialize P4 connection");
    }

    // Close the connection and report transport errors explicitly.
    void Close() {
        Error error;
        initialized = false;
        client.Final(&error);
        Check(error, "Failed to finalize P4 connection");
    }
};

// Execute a command while keeping argument storage alive through Run.
void execute(const p4rust_options_v1& options, const char* command, int32_t argc,
             const char* const* args, p4rust_callback_v1 callback, void* context) {
    static std::mutex mutex;
    const std::lock_guard<std::mutex> lock(mutex);
    static Runtime runtime;
    ThreadScope thread;
    Session session;
    session.Configure(options);
    session.Open();
    User user;
    user.callback = callback;
    user.context = context;
    user.input = options.input;
    std::vector<std::string> storage;
    std::vector<char*> pointers;
    storage.reserve(static_cast<size_t>(argc));
    for (int32_t index = 0; index < argc; ++index) storage.emplace_back(args[index]);
    for (auto& arg : storage) pointers.push_back(arg.data());
    session.client.SetArgv(static_cast<int>(pointers.size()), pointers.data());
    session.client.Run(command, &user);
    session.Close();
}
}

// Return the ABI contract implemented by this precompiled bridge.
extern "C" uint32_t p4rust_abi_version(void) { return 1; }

// Contain all C++ exceptions before returning across the C ABI boundary.
extern "C" int32_t p4rust_execute_v1(const p4rust_options_v1* options,
    const char* command, int32_t argc, const char* const* argv,
    p4rust_callback_v1 callback, void* context) {
    if (!callback) return 1;
    try {
        if (!options || options->abi_version != 1 || !command || argc < 0 ||
            (argc && !argv) || !options->port || !options->user || !options->client ||
            !options->cwd || !options->charset || !options->input)
            throw std::runtime_error("Failed to validate ABI v1 arguments");
        for (int32_t index = 0; index < argc; ++index)
            if (!argv[index]) throw std::runtime_error("Failed to validate native argument");
        p4rust::execute(*options, command, argc, argv, callback, context);
        return 0;
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
