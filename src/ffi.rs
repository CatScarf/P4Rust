//! Raw ABI v1 bindings; callers own every buffer and must uphold C pointer validity.

use std::ffi::{c_char, c_void};

pub const ABI_VERSION: u32 = 1;
pub const TEXT: u32 = 1;
pub const BINARY: u32 = 2;
pub const RECORD: u32 = 3;
pub const FIELD: u32 = 4;
pub const WARNING: u32 = 5;
pub const ERROR: u32 = 6;

pub type Callback =
    unsafe extern "C" fn(*mut c_void, u32, *const u8, usize, *const u8, usize) -> i32;
pub type Alive = unsafe extern "C" fn(*mut c_void) -> i32;

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
