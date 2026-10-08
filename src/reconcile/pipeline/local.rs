use crate::{Config, Result, ResultExt, control::Control, error::ensure, ffi};
use std::{
    ffi::{CString, c_void},
    ptr::NonNull,
};

#[repr(C)]
#[derive(Clone, Copy, Debug, Default)]
pub(crate) struct Snapshot {
    pub size: u64,
    pub time: i64,
    pub canonical_size: u64,
    pub file_type: i32,
    pub hashed: i32,
    pub charset: i32,
    pub stat: i32,
    pub link_time: i64,
    pub digest: [u8; 32],
}

struct AgentControl<'a> {
    command: Control,
    stop: &'a std::sync::atomic::AtomicBool,
}
pub(super) struct Agent<'a> {
    pointer: NonNull<c_void>,
    control: Box<AgentControl<'a>>,
}

impl<'a> Agent<'a> {
    // Create thread-local SDK ignore and canonical hashing state without a server connection.
    pub(super) fn new(
        config: &Config,
        control: &Control,
        ignore: bool,
        stop: &'a std::sync::atomic::AtomicBool,
    ) -> Result<Self> {
        let cwd = CString::new(config.cwd.as_str()).context("Failed to encode scan directory")?;
        let charset =
            CString::new(config.charset.as_str()).context("Failed to encode scan charset")?;
        let control = Box::new(AgentControl {
            command: control.clone(),
            stop,
        });
        let mut pointer = std::ptr::null_mut();
        // All borrowed settings expire only after the synchronous constructor returns.
        let code = unsafe {
            ffi::p4rust_scan_open_v5(
                cwd.as_ptr(),
                charset.as_ptr(),
                i32::from(ignore),
                Self::alive,
                std::ptr::from_ref(control.as_ref()).cast_mut().cast(),
                &mut pointer,
            )
        };
        ensure!(code == 0, "Failed to initialize SDK local scan policy");
        Ok(Self {
            pointer: NonNull::new(pointer).context("Failed to own SDK scan policy")?,
            control,
        })
    }

    // Apply SDK ignore rules and compute a canonical digest when content comparison is selected.
    pub(super) fn inspect(
        &self,
        path: &str,
        directory: bool,
        hashes: bool,
    ) -> Result<Option<Snapshot>> {
        self.inspect_kind(path, i32::from(directory), hashes)
            .context("Failed to inspect scanned path")
    }

    // Check SDK ignore policy without querying file metadata or reading contents.
    pub(super) fn accepted(&self, path: &str, directory: bool) -> Result<bool> {
        let path = CString::new(path).context("Failed to encode ignore path")?;
        let mut snapshot = Snapshot::default();
        let code = unsafe {
            ffi::p4rust_scan_file_v5(
                self.pointer.as_ptr(),
                path.as_ptr(),
                if directory { 1 } else { -2 },
                0,
                &mut snapshot,
            )
        };
        ensure!(code >= 0, "Failed to check SDK ignore policy");
        Ok(code > 0)
    }

    // Share the synchronous SDK probe boundary for enumerated and server-prioritized paths.
    fn inspect_kind(&self, path: &str, directory: i32, hashes: bool) -> Result<Option<Snapshot>> {
        if let Some(error) = self.control.command.error() {
            return Err(error).context("Failed to inspect local path");
        }
        let path = CString::new(path).context("Failed to encode local scan path")?;
        if self.control.stop.load(std::sync::atomic::Ordering::Acquire) {
            return Ok(None);
        }
        let mut snapshot = Snapshot {
            stat: -1,
            ..Snapshot::default()
        };
        // This agent is used and destroyed exclusively on its creating scan worker.
        let code = unsafe {
            ffi::p4rust_scan_file_v5(
                self.pointer.as_ptr(),
                path.as_ptr(),
                directory,
                i32::from(hashes),
                &mut snapshot,
            )
        };
        ensure!(
            code >= 0,
            "Failed to inspect SDK local file: {}",
            path.to_string_lossy()
        );
        Ok((code > 0).then_some(snapshot))
    }
    // Apply SDK policy and canonical hashing to metadata already owned by this command.
    pub(super) fn cached(
        &self,
        path: &str,
        mut snapshot: Snapshot,
        hashes: bool,
    ) -> Result<Snapshot> {
        let path = CString::new(path).context("Failed to encode cached scan path")?;
        let code = unsafe {
            ffi::p4rust_scan_file_v5(
                self.pointer.as_ptr(),
                path.as_ptr(),
                -1,
                i32::from(hashes),
                &mut snapshot,
            )
        };
        ensure!(
            code >= 0,
            "Failed to compute cached SDK scan: {}",
            path.to_string_lossy()
        );
        Ok(snapshot)
    }
    // Cancel SDK reads when either the command stops or enumeration closes early.
    unsafe extern "C" fn alive(context: *mut c_void) -> i32 {
        // The agent owns this control until all of its synchronous reads finish.
        let control = unsafe { &*context.cast::<AgentControl<'_>>() };
        i32::from(
            !control.stop.load(std::sync::atomic::Ordering::Acquire)
                && control.command.error().is_none(),
        )
    }
}

impl Drop for Agent<'_> {
    // Release converters and thread-local SDK state on their creating worker.
    fn drop(&mut self) {
        unsafe { ffi::p4rust_scan_close_v5(self.pointer.as_ptr()) };
    }
}
