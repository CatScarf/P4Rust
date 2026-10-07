#ifndef P4RUST_CLIENT_H
#define P4RUST_CLIENT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum p4rust_event_v1 {
    P4RUST_TEXT = 1, P4RUST_BINARY = 2, P4RUST_RECORD = 3,
    P4RUST_ERROR = 6, P4RUST_PROGRESS = 8,
    P4RUST_INFO = 9, P4RUST_RECORD_PARTIAL = 10, P4RUST_MESSAGE = 11,
    P4RUST_HANDLE_ERROR = 12, P4RUST_OUTPUT_ERROR = 13, P4RUST_FINISHED = 14,
    P4RUST_STATUS = 15
};

typedef struct p4rust_field_v2 {
    const uint8_t* key;
    size_t key_length;
    const uint8_t* value;
    size_t value_length;
} p4rust_field_v2;

typedef int32_t (*p4rust_callback_v1)(void*, uint32_t,
    const uint8_t*, size_t, const uint8_t*, size_t);
typedef int32_t (*p4rust_alive_v1)(void*);
typedef int32_t (*p4rust_reconcile_v3)(void*, uint32_t, void*, const uint8_t*, size_t);

typedef struct p4rust_options_v1 {
    uint32_t abi_version;
    const char* port;
    const char* user;
    const char* client;
    const char* cwd;
    const char* charset;
    const char* input;
} p4rust_options_v1;

// Return the supported ABI version without initializing the SDK.
uint32_t p4rust_abi_version(void);

// Run synchronously; strings are NUL-terminated and callback bytes are borrowed.
int32_t p4rust_execute_v1(const p4rust_options_v1*, const char*, int32_t,
    const char* const*, p4rust_callback_v1, void*);

// Run with an optional, synchronous cancellation callback owned by the caller.
int32_t p4rust_execute_controlled_v1(const p4rust_options_v1*, const char*, int32_t,
    const char* const*, p4rust_callback_v1, void*, p4rust_alive_v1, void*);

// Execute with an owned Rust scheduler for local reconcile work.
int32_t p4rust_execute_reconcile_v3(const p4rust_options_v1*, const char*, int32_t,
    const char* const*, p4rust_callback_v1, void*, p4rust_alive_v1, void*,
    p4rust_reconcile_v3, void*, uint32_t);
// Compute an isolated task with SDK file conversion and thread initialization.
int32_t p4rust_reconcile_execute_v3(void*);
// Borrow the result fields until the synchronous callback returns.
int32_t p4rust_reconcile_result_v3(void*, p4rust_callback_v1, void*);
// Commit on the SDK connection thread after local work finishes.
int32_t p4rust_reconcile_commit_v3(void*, int32_t);
// Borrow task diagnostics while the task is alive.
const uint8_t* p4rust_reconcile_error_v3(void*, size_t*);
// Destroy an exclusively owned task after all callbacks finish.
void p4rust_reconcile_drop_v3(void*);

#ifdef __cplusplus
}
#endif
#endif
