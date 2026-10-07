#ifndef P4RUST_CLIENT_H
#define P4RUST_CLIENT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum p4rust_event_v1 {
    P4RUST_TEXT = 1, P4RUST_BINARY = 2, P4RUST_RECORD = 3,
    P4RUST_FIELD = 4, P4RUST_WARNING = 5, P4RUST_ERROR = 6, P4RUST_RECORD_END = 7, P4RUST_PROGRESS = 8
};

typedef int32_t (*p4rust_callback_v1)(void*, uint32_t,
    const uint8_t*, size_t, const uint8_t*, size_t);
typedef int32_t (*p4rust_alive_v1)(void*);

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

#ifdef __cplusplus
}
#endif
#endif
