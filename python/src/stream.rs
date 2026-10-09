use crate::conversion::Conversion;
use pyo3::{exceptions::PyRuntimeError, prelude::*};
use std::sync::{Mutex, TryLockError};

#[pyclass(name = "_Stream", frozen)]
pub(crate) struct Stream {
    inner: Mutex<Option<p4rust::CommandStream>>,
    cancel: p4rust::CancellationToken,
    conversion: Conversion,
}

impl Stream {
    // Cache Python event constructors and retain the command-local cancellation signal.
    pub(crate) fn new(py: Python<'_>, stream: p4rust::CommandStream) -> PyResult<Self> {
        Ok(Self {
            cancel: stream.cancellation_token(),
            inner: Mutex::new(Some(stream)),
            conversion: Conversion::new(py)?,
        })
    }
}

#[pymethods]
impl Stream {
    // Block on the Rust event queue while detached from either Python threading mode.
    fn next(&self, py: Python<'_>) -> PyResult<Option<Py<PyAny>>> {
        let event = py.detach(|| {
            let mut guard = self.inner.lock().map_err(|error| {
                PyRuntimeError::new_err(format!("Failed to lock event stream: {error}"))
            })?;
            guard
                .as_mut()
                .and_then(Iterator::next)
                .transpose()
                .map_err(Conversion::error)
        })?;
        event
            .map(|event| self.conversion.event(py, event))
            .transpose()
    }

    // Consume remaining output using the existing Rust collection implementation.
    fn collect_output(&self, py: Python<'_>) -> PyResult<Py<PyAny>> {
        let output = py.detach(|| {
            let stream = self
                .inner
                .lock()
                .map_err(|error| {
                    PyRuntimeError::new_err(format!("Failed to lock collected stream: {error}"))
                })?
                .take()
                .ok_or_else(|| {
                    PyRuntimeError::new_err("Failed to collect output: stream is closed")
                })?;
            stream.collect_output().map_err(Conversion::error)
        })?;
        self.conversion.output(py, output)
    }

    // Cancel immediately and drop idle streams without waiting for an active reader.
    fn close(&self, py: Python<'_>) -> PyResult<()> {
        self.cancel.cancel();
        py.detach(|| match self.inner.try_lock() {
            Ok(mut inner) => {
                inner.take();
                Ok(())
            }
            Err(TryLockError::WouldBlock) => Ok(()),
            Err(TryLockError::Poisoned(error)) => Err(PyRuntimeError::new_err(format!(
                "Failed to close event stream: {error}"
            ))),
        })
    }
}

impl Drop for Stream {
    // Interrupt the command even when its Python stream was never consumed.
    fn drop(&mut self) {
        self.cancel.cancel();
    }
}
