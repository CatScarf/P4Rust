use crate::control::Control;
use crate::error::{Result, ResultExt, ensure};
use crate::ffi;
use crate::{Config, Output, capture::Capture};
use std::ffi::{CString, c_char, c_void};

pub(crate) struct Native;

impl Native {
    // Encode NUL-terminated strings while preserving contextual validation errors.
    fn string(value: &str) -> Result<CString> {
        CString::new(value).context("Failed to encode native argument: embedded NUL")
    }

    // Call the precompiled C ABI without sharing allocation ownership across runtimes.
    pub(crate) fn execute(
        config: &Config,
        command: &str,
        args: &[&str],
        input: &str,
        control: Option<&Control>,
    ) -> Result<Output> {
        // The version function has no pointer arguments or runtime side effects.
        ensure!(
            unsafe { ffi::p4rust_abi_version() } == ffi::ABI_VERSION,
            "Failed to validate native ABI version"
        );
        let settings: [&str; 6] = [
            &config.port,
            &config.user,
            &config.client,
            &config.cwd,
            &config.charset,
            input,
        ];
        let values: Vec<CString> = settings
            .into_iter()
            .map(Self::string)
            .collect::<Result<_>>()
            .context("Failed to encode native connection settings")?;
        let options = Self::options(&values);
        let command = Self::string(command).context("Failed to encode native command")?;
        Self::invoke(&options, &command, args, control).context("Failed to invoke native command")
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

    // Keep argument and callback storage alive through the synchronous ABI call.
    fn invoke(
        options: &ffi::Options,
        command: &CString,
        args: &[&str],
        control: Option<&Control>,
    ) -> Result<Output> {
        let strings: Vec<CString> = args
            .iter()
            .map(|arg| Self::string(arg))
            .collect::<Result<_>>()
            .context("Failed to encode native command arguments")?;
        let pointers: Vec<*const c_char> = strings.iter().map(|arg| arg.as_ptr()).collect();
        let count = i32::try_from(pointers.len()).context("Failed to encode argument count")?;
        let mut capture = Capture::new();
        // Every pointer stays valid through this synchronous call; callbacks copy borrowed bytes.
        let status = unsafe {
            ffi::p4rust_execute_controlled_v1(
                options,
                command.as_ptr(),
                count,
                pointers.as_ptr(),
                Capture::callback,
                (&mut capture as *mut Capture).cast::<c_void>(),
                control.map(|_| Control::alive as ffi::Alive),
                control
                    .map(|control| std::ptr::from_ref(control).cast_mut().cast::<c_void>())
                    .unwrap_or(std::ptr::null_mut()),
            )
        };
        capture
            .finish(status)
            .context("Failed to collect native P4 output")
    }
}
