use pyo3::{
    prelude::*,
    types::{PyBytes, PyDict, PyDictMethods},
};
use std::error::Error;

pub(crate) struct Conversion(Py<PyModule>);

impl Conversion {
    // Retain event classes from the current interpreter rather than global Python objects.
    pub(crate) fn new(py: Python<'_>) -> PyResult<Self> {
        Ok(Self(py.import("p4rust._events")?.unbind()))
    }

    // Preserve contextual failures, native exit status, and interruption categories.
    pub(crate) fn error(error: p4rust::Error) -> PyErr {
        let status = error.command_status();
        let mut source: &(dyn Error + 'static) = &error;
        let mut kind = "command";
        loop {
            if let Some(io) = source.downcast_ref::<std::io::Error>() {
                kind = match io.kind() {
                    std::io::ErrorKind::TimedOut => "timeout",
                    std::io::ErrorKind::Interrupted => "cancelled",
                    _ => "command",
                };
                break;
            }
            match source.source() {
                Some(next) => source = next,
                None => break,
            }
        }
        crate::CommandError::new_err((
            error.to_string(),
            status.map(|s| s.exit_code),
            status.and_then(|s| s.error_count),
            kind,
        ))
    }

    // Instantiate a typed event from explicitly named Python fields.
    fn create(
        &self,
        py: Python<'_>,
        name: &str,
        fields: &Bound<'_, PyDict>,
    ) -> PyResult<Py<PyAny>> {
        Ok(self
            .0
            .bind(py)
            .getattr(name)?
            .call((), Some(fields))?
            .unbind())
    }

    // Convert status values without replacing the native exit code.
    fn status(&self, py: Python<'_>, status: p4rust::CommandStatus) -> PyResult<Py<PyAny>> {
        let fields = PyDict::new(py);
        fields.set_item("exit_code", status.exit_code)?;
        fields.set_item("error_count", status.error_count)?;
        fields.set_item("success", status.success)?;
        self.create(py, "CommandStatus", &fields)
    }

    // Preserve ordered tagged fields and duplicate keys as original bytes.
    fn record(&self, py: Python<'_>, record: p4rust::Record) -> PyResult<Py<PyAny>> {
        let values: Vec<_> = record
            .raw_fields()
            .map(|(key, value)| (PyBytes::new(py, key), PyBytes::new(py, value)))
            .collect();
        let fields = PyDict::new(py);
        fields.set_item("raw_fields", pyo3::types::PyTuple::new(py, values)?)?;
        self.create(py, "Record", &fields)
    }

    // Forward all native message fields including serialized SDK bytes.
    fn message(&self, py: Python<'_>, message: p4rust::Message) -> PyResult<Py<PyAny>> {
        let fields = PyDict::new(py);
        let mut ids = Vec::new();
        for id in message.ids {
            let value = PyDict::new(py);
            value.set_item("code", id.code)?;
            value.set_item("format", PyBytes::new(py, &id.format))?;
            ids.push(self.create(py, "MessageId", &value)?);
        }
        let parameters: Vec<_> = message
            .parameters
            .iter()
            .map(|(key, value)| (PyBytes::new(py, key), PyBytes::new(py, value)))
            .collect();
        fields.set_item("severity", message.severity)?;
        fields.set_item("generic", message.generic)?;
        fields.set_item("ids", pyo3::types::PyTuple::new(py, ids)?)?;
        fields.set_item("parameters", pyo3::types::PyTuple::new(py, parameters)?)?;
        fields.set_item("text", message.text)?;
        fields.set_item("raw", PyBytes::new(py, &message.raw))?;
        fields.set_item("serialized", PyBytes::new(py, &message.serialized))?;
        self.create(py, "Message", &fields)
    }

    // Convert progress enums through integer-preserving Python enum constructors.
    fn progress(&self, py: Python<'_>, progress: p4rust::Progress) -> PyResult<Py<PyAny>> {
        let module = self.0.bind(py);
        let fields = PyDict::new(py);
        fields.set_item("id", progress.id)?;
        fields.set_item(
            "kind",
            module
                .getattr("ProgressKind")?
                .call1((progress.kind.code(),))?,
        )?;
        fields.set_item(
            "units",
            module
                .getattr("ProgressUnit")?
                .call1((progress.units.code(),))?,
        )?;
        fields.set_item("description", progress.description)?;
        fields.set_item(
            "raw_description",
            PyBytes::new(py, &progress.raw_description),
        )?;
        fields.set_item("total", progress.total)?;
        fields.set_item("current", progress.current)?;
        let callback = match progress.callback {
            p4rust::ProgressCallback::Description => "description",
            p4rust::ProgressCallback::Total => "total",
            p4rust::ProgressCallback::Update => "update",
            p4rust::ProgressCallback::Done => "done",
        };
        fields.set_item(
            "callback",
            module.getattr("ProgressCallback")?.call1((callback,))?,
        )?;
        fields.set_item("failure", progress.failure)?;
        self.create(py, "Progress", &fields)
    }

    // Map each SDK event to its own typed Python event without coalescing callbacks.
    pub(crate) fn event(&self, py: Python<'_>, event: p4rust::Event) -> PyResult<Py<PyAny>> {
        let fields = PyDict::new(py);
        let name = match event {
            p4rust::Event::Text { raw } => {
                fields.set_item("raw", PyBytes::new(py, &raw))?;
                "Text"
            }
            p4rust::Event::Info { level, text, raw } => {
                fields.set_item("level", level)?;
                fields.set_item("text", text)?;
                fields.set_item("raw", PyBytes::new(py, &raw))?;
                "Info"
            }
            p4rust::Event::Binary(bytes) => {
                fields.set_item("data", PyBytes::new(py, &bytes))?;
                "Binary"
            }
            p4rust::Event::Record(record) => {
                fields.set_item("record", self.record(py, record)?)?;
                "RecordEvent"
            }
            p4rust::Event::RecordPartial(record) => {
                fields.set_item("record", self.record(py, record)?)?;
                "RecordPartial"
            }
            p4rust::Event::Message(message) => {
                fields.set_item("message", self.message(py, message)?)?;
                "MessageEvent"
            }
            p4rust::Event::HandleError(message) => {
                fields.set_item("message", self.message(py, message)?)?;
                "HandleError"
            }
            p4rust::Event::OutputError { text, raw } => {
                fields.set_item("text", text)?;
                fields.set_item("raw", PyBytes::new(py, &raw))?;
                "OutputError"
            }
            p4rust::Event::NativeError { text, raw } => {
                fields.set_item("text", text)?;
                fields.set_item("raw", PyBytes::new(py, &raw))?;
                "NativeError"
            }
            p4rust::Event::Progress(progress) => {
                fields.set_item("progress", self.progress(py, progress)?)?;
                "ProgressEvent"
            }
            p4rust::Event::Finished => "Finished",
            p4rust::Event::Completed(status) => {
                fields.set_item("status", self.status(py, status)?)?;
                "Completed"
            }
        };
        self.create(py, name, &fields)
    }

    // Reuse the complete Rust output and retain every collected raw representation.
    pub(crate) fn output(&self, py: Python<'_>, output: p4rust::Output) -> PyResult<Py<PyAny>> {
        let fields = PyDict::new(py);
        fields.set_item("text", output.text)?;
        fields.set_item("binary", PyBytes::new(py, &output.binary))?;
        let records = output
            .records
            .into_iter()
            .map(|r| self.record(py, r))
            .collect::<PyResult<Vec<_>>>()?;
        fields.set_item("records", pyo3::types::PyTuple::new(py, records)?)?;
        let partial = output
            .partial_records
            .into_iter()
            .map(|r| self.record(py, r))
            .collect::<PyResult<Vec<_>>>()?;
        fields.set_item("partial_records", pyo3::types::PyTuple::new(py, partial)?)?;
        let messages = output
            .messages
            .into_iter()
            .map(|m| self.message(py, m))
            .collect::<PyResult<Vec<_>>>()?;
        fields.set_item("messages", pyo3::types::PyTuple::new(py, messages)?)?;
        fields.set_item("warnings", pyo3::types::PyTuple::new(py, output.warnings)?)?;
        fields.set_item("raw_text", PyBytes::new(py, &output.raw.text))?;
        fields.set_item(
            "raw_warnings",
            pyo3::types::PyTuple::new(py, output.raw.warnings.iter().map(|b| PyBytes::new(py, b)))?,
        )?;
        fields.set_item(
            "errors",
            pyo3::types::PyTuple::new(py, output.errors.iter().map(|b| PyBytes::new(py, b)))?,
        )?;
        fields.set_item(
            "info",
            pyo3::types::PyTuple::new(
                py,
                output
                    .info
                    .iter()
                    .map(|(level, bytes)| (*level, PyBytes::new(py, bytes))),
            )?,
        )?;
        fields.set_item("finished_callbacks", output.finished_callbacks)?;
        fields.set_item(
            "status",
            output.status.map(|s| self.status(py, s)).transpose()?,
        )?;
        self.create(py, "Output", &fields)
    }
}
