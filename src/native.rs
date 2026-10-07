use crate::control::Control;
use crate::error::{Result, ResultExt, ensure};
use crate::ffi;
use crate::{Config, capture::Capture};
use std::ffi::{CString, c_char, c_void};

pub(crate) struct Native;

impl Native {
    // Encode NUL-terminated strings while preserving contextual validation errors.
    fn string(value: &str) -> Result<CString> {
        CString::new(value).context("Failed to encode native argument: embedded NUL")
    }

    // Invoke the native ABI while delivering events directly to the bounded stream.
    pub(crate) fn execute(
        config: &Config,
        command: &str,
        args: &[&str],
        input: &str,
        control: &Control,
        capture: &Capture,
    ) -> Result<()> {
        // This version query has no pointer arguments or runtime side effects.
        ensure!(
            unsafe { ffi::p4rust_abi_version() } == ffi::ABI_VERSION,
            "Failed to validate native ABI version"
        );
        let values: Vec<CString> = [
            &config.port[..],
            &config.user,
            &config.client,
            &config.cwd,
            &config.charset,
            input,
        ]
        .into_iter()
        .map(Self::string)
        .collect::<Result<_>>()
        .context("Failed to encode native connection settings")?;
        let options = Self::options(&values);
        let command = Self::string(command).context("Failed to encode native command")?;
        Self::invoke(&options, &command, args, control, capture)
            .context("Failed to invoke native command")
    }

    // Build borrowed options from the six encoded connection fields.
    fn options(values: &[CString]) -> ffi::Options {
        ffi::Options {
            abi_version: ffi::ABI_VERSION,
            port: values[0].as_ptr(),
            user: values[1].as_ptr(),
            client: values[2].as_ptr(),
            cwd: values[3].as_ptr(),
            charset: values[4].as_ptr(),
            input: values[5].as_ptr(),
        }
    }

    // Keep every borrowed buffer and callback context alive until all SDK workers finish.
    fn invoke(
        options: &ffi::Options,
        command: &CString,
        args: &[&str],
        control: &Control,
        capture: &Capture,
    ) -> Result<()> {
        let strings: Vec<CString> = args
            .iter()
            .map(|arg| Self::string(arg))
            .collect::<Result<_>>()
            .context("Failed to encode native command arguments")?;
        let pointers: Vec<*const c_char> = strings.iter().map(|arg| arg.as_ptr()).collect();
        let count = i32::try_from(pointers.len()).context("Failed to encode argument count")?;
        // The owned worker outlives the synchronous native call and every parallel callback.
        let status = unsafe {
            ffi::p4rust_execute_controlled_v1(
                options,
                command.as_ptr(),
                count,
                pointers.as_ptr(),
                Capture::callback,
                std::ptr::from_ref(capture).cast_mut().cast::<c_void>(),
                Some(Control::alive as ffi::Alive),
                std::ptr::from_ref(control).cast_mut().cast::<c_void>(),
            )
        };
        capture
            .finish(status)
            .context("Failed to collect native P4 output")
    }
}
