#ifndef P4RUST_SESSION_H
#define P4RUST_SESSION_H
#include <memory>

namespace p4rust {
class CommandBinding {
    ClientApi& client;
public:
    // Install command-local interruption only while its owner remains alive.
    CommandBinding(ClientApi& owner, Interrupt& interrupt) : client(owner) {
        client.SetBreak(&interrupt);
    }

    // Remove borrowed argument and interruption pointers before retaining the connection.
    ~CommandBinding() {
        client.SetBreak(nullptr);
        client.SetArgv(0, nullptr);
    }
};

struct Invocation {
    const p4rust_options_v1& options;
    const char* command;
    int32_t argc;
    const char* const* args;
    p4rust_callback_v1 callback;
    void* context;
    p4rust_alive_v1 alive;
    void* control;
    p4rust_reconcile_v3 reconcile;
    void* scheduler;
    uint32_t moves;

    // Validate all borrowed ABI arguments before touching session state.
    void Validate() const {
        if (options.abi_version != 6 || !command || argc < 0 || (argc && !args) ||
            !callback || !options.port || !options.user || !options.client ||
            !options.cwd || !options.charset || !options.input || (alive && !control))
            throw std::runtime_error("Failed to validate session bridge arguments");
        if (reconcile && (!scheduler || moves < 1 || moves > 32 ||
            (std::strcmp(command, "reconcile") && std::strcmp(command, "rec") && std::strcmp(command, "status"))))
            throw std::runtime_error("Failed to validate session reconcile arguments");
        for (int32_t index = 0; index < argc; ++index)
            if (!args[index]) throw std::runtime_error("Failed to validate session command argument");
    }

    // Execute using fresh callbacks and retain only an idle SDK connection.
    int32_t Run(Session& session, bool close) const {
        Interrupt interrupt;
        interrupt.callback = alive;
        interrupt.context = control;
        interrupt.Check();
        User user;
        user.callback = callback;
        user.context = context;
        user.input = options.input;
        user.interrupt = &interrupt;
        ReconcileScope scheduling(user, reconcile, scheduler, moves);
        if (!session.initialized) session.Open(user);
        CommandBinding binding(session.client, interrupt);
        std::vector<std::string> storage;
        std::vector<char*> pointers;
        storage.reserve(static_cast<size_t>(argc));
        for (int32_t index = 0; index < argc; ++index) storage.emplace_back(args[index]);
        for (auto& arg : storage) pointers.push_back(arg.data());
        session.client.SetArgv(static_cast<int>(pointers.size()), pointers.data());
        interrupt.Check();
        session.client.Run(command, &user);
        if (reconcile) scheduling.Drain();
        scheduling.Close();
        const int32_t errors = session.client.GetErrors();
        user.Emit(P4RUST_STATUS, reinterpret_cast<const char*>(&errors), sizeof(errors));
        if (close) session.Close(user);
        return errors != 0 || user.command_failed.load() || interrupt.failed.load() ||
            (alive && alive(control) == 0) ? 1 : 0;
    }
};

class Persistent {
    ThreadScope thread;
    std::vector<std::string> settings;
    std::unique_ptr<Session> session;
public:
    // Retain immutable connection settings and thread-local SDK initialization.
    explicit Persistent(const p4rust_options_v1& options)
        : settings{options.port, options.user, options.client, options.cwd, options.charset} {}

    // Reconnect before a later command without replaying a failed operation.
    int32_t Run(const Invocation& request) {
        const auto& options = request.options;
        if (settings != std::vector<std::string>{options.port, options.user, options.client,
                options.cwd, options.charset})
            throw std::runtime_error("Failed to reuse session: connection settings changed");
        try {
            if (session && session->client.Dropped()) session.reset();
            if (!session) {
                session = std::make_unique<Session>();
                session->Configure(options);
            }
            const int32_t status = request.Run(*session, false);
            if (status || session->client.Dropped()) session.reset();
            return status;
        } catch (...) {
            session.reset();
            throw;
        }
    }
};
}
#endif
