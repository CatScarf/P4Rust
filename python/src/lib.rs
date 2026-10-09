use pyo3::prelude::*;
mod client;
mod conversion;
mod stream;
pyo3::create_exception!(_native, CommandError, pyo3::exceptions::PyRuntimeError);

// Register interpreter-independent wrappers with internally synchronized state.
#[pymodule(gil_used = false)]
fn _native(module: &Bound<'_, PyModule>) -> PyResult<()> {
    module.add_class::<client::Client>()?;
    module.add_class::<client::Command>()?;
    module.add_class::<client::CancellationToken>()?;
    module.add_class::<stream::Stream>()?;
    module.add("CommandError", module.py().get_type::<CommandError>())?;
    Ok(())
}
