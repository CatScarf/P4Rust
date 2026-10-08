//! Raw bridge protocol v5; callers own buffers and must uphold C pointer validity.

use std::ffi::{c_char, c_void};

#[cfg(all(target_os = "macos", target_arch = "aarch64"))]
use p4rust_resources_aarch64_apple_darwin as resources;
#[cfg(all(target_os = "windows", target_arch = "aarch64", target_env = "msvc"))]
use p4rust_resources_aarch64_pc_windows_msvc as resources;
#[cfg(all(target_os = "linux", target_arch = "aarch64", target_env = "gnu"))]
use p4rust_resources_aarch64_unknown_linux_gnu as resources;
#[cfg(all(target_os = "macos", target_arch = "x86_64"))]
use p4rust_resources_x86_64_apple_darwin as resources;
#[cfg(all(target_os = "windows", target_arch = "x86_64", target_env = "msvc"))]
use p4rust_resources_x86_64_pc_windows_msvc as resources;
#[cfg(all(target_os = "linux", target_arch = "x86_64", target_env = "gnu"))]
use p4rust_resources_x86_64_unknown_linux_gnu as resources;

pub const ABI_VERSION: u32 = resources::ABI_VERSION;
pub const TEXT: u32 = 1;
pub const BINARY: u32 = 2;
pub const RECORD: u32 = 3;
pub const ERROR: u32 = 6;
pub const PROGRESS: u32 = 8;
pub const INFO: u32 = 9;
pub const RECORD_PARTIAL: u32 = 10;
pub const MESSAGE: u32 = 11;
pub const HANDLE_ERROR: u32 = 12;
pub const OUTPUT_ERROR: u32 = 13;
pub const FINISHED: u32 = 14;
pub const STATUS: u32 = 15;

pub type Callback =
    unsafe extern "C" fn(*mut c_void, u32, *const u8, usize, *const u8, usize) -> i32;
pub type Alive = unsafe extern "C" fn(*mut c_void) -> i32;
pub type Reconcile = unsafe extern "C" fn(*mut c_void, u32, *mut c_void, *const u8, usize) -> i32;

#[repr(C)]
pub(crate) struct Field {
    pub key: *const u8,
    pub key_length: usize,
    pub value: *const u8,
    pub value_length: usize,
}

#[repr(C)]
pub struct Options {
    pub abi_version: u32,
    pub port: *const c_char,
    pub user: *const c_char,
    pub client: *const c_char,
    pub cwd: *const c_char,
    pub charset: *const c_char,
    pub input: *const c_char,
}

unsafe extern "C" {
    /// Create a thread-owned SDK ignore and digest agent without a server connection.
    pub fn p4rust_scan_open_v5(
        cwd: *const c_char,
        charset: *const c_char,
        ignore: i32,
        alive: Alive,
        control: *mut c_void,
        output: *mut *mut c_void,
    ) -> i32;
    /// Probe a local path with SDK ignore, file classification, and canonical digest rules.
    pub fn p4rust_scan_file_v5(
        agent: *mut c_void,
        path: *const c_char,
        directory: i32,
        hashes: i32,
        output: *mut crate::reconcile::pipeline::local::Snapshot,
    ) -> i32;
    /// Release scan policy and SDK thread state on their creating thread.
    pub fn p4rust_scan_close_v5(agent: *mut c_void);
    /// Attach a local snapshot to an exclusively owned comparison task.
    pub fn p4rust_reconcile_snapshot_v5(
        task: *mut c_void,
        snapshot: *const crate::reconcile::pipeline::local::Snapshot,
    ) -> i32;
    /// Append an owned path to a synchronous native candidate sink.
    pub fn p4rust_reconcile_path_v5(
        sink: *mut c_void,
        path: *const c_char,
        snapshot: *const crate::reconcile::pipeline::local::Snapshot,
    ) -> i32;
    /// Execute reconciliation with an owned Rust scheduler and synchronous protocol callbacks.
    pub fn p4rust_execute_reconcile_v3(
        options: *const Options,
        command: *const c_char,
        argc: i32,
        argv: *const *const c_char,
        callback: Callback,
        context: *mut c_void,
        alive: Option<Alive>,
        control: *mut c_void,
        reconcile: Reconcile,
        scheduler: *mut c_void,
        move_workers: u32,
    ) -> i32;
    /// Execute isolated local work without reading or writing its SDK connection.
    pub fn p4rust_reconcile_execute_v3(task: *mut c_void) -> i32;
    /// Read the completed local result through a borrowed SDK dictionary callback.
    pub fn p4rust_reconcile_result_v3(
        task: *mut c_void,
        callback: Callback,
        context: *mut c_void,
    ) -> i32;
    /// Send a frozen confirmation on the native connection's owning thread.
    pub fn p4rust_reconcile_commit_v3(task: *mut c_void, status: i32) -> i32;
    /// Borrow diagnostic bytes while the uniquely owned task is alive.
    pub fn p4rust_reconcile_error_v3(task: *mut c_void, length: *mut usize) -> *const u8;
    /// Release one task only after its worker and final native callback finish.
    pub fn p4rust_reconcile_drop_v3(task: *mut c_void);
    /// Query the version of the linked native ABI.
    pub fn p4rust_abi_version() -> u32;
    /// Execute synchronously with valid strings, pointers, and a non-unwinding callback.
    pub fn p4rust_execute_controlled_v1(
        options: *const Options,
        command: *const c_char,
        argc: i32,
        argv: *const *const c_char,
        callback: Callback,
        context: *mut c_void,
        alive: Option<Alive>,
        control: *mut c_void,
    ) -> i32;
}
