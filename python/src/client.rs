use crate::{conversion::Conversion, stream::Stream};
use pyo3::{exceptions::PyValueError, prelude::*};
use std::time::Duration;

#[pyclass(name = "_CancellationToken", frozen, skip_from_py_object)]
#[derive(Clone, Default)]
pub(crate) struct CancellationToken(pub(crate) p4rust::CancellationToken);

#[pymethods]
impl CancellationToken {
    // Create an independent cancellation signal.
    #[new]
    fn new() -> Self {
        Self::default()
    }

    // Request cancellation of every command attached to this token.
    fn cancel(&self) {
        self.0.cancel();
    }

    // Inspect cancellation without acquiring a Python or command lock.
    fn is_cancelled(&self) -> bool {
        self.0.is_cancelled()
    }
}

#[pyclass(name = "_Client", frozen)]
pub(crate) struct Client(p4rust::Config);

#[pymethods]
impl Client {
    // Validate explicit connection settings before creating a Python client.
    #[new]
    fn new(
        port: String,
        user: String,
        client: String,
        cwd: String,
        charset: String,
    ) -> PyResult<Self> {
        let mut config = p4rust::Config::new(port, user, client);
        config.cwd = cwd;
        config.charset = charset;
        p4rust::Client::new(config.clone()).map_err(Conversion::error)?;
        Ok(Self(config))
    }

    // Create an immutable command builder with owned connection settings.
    fn command(&self, name: String) -> Command {
        Command {
            config: self.0.clone(),
            name,
            arguments: Vec::new(),
            form: String::new(),
            timeout: None,
            token: None,
            reconcile: None,
        }
    }
}

#[pyclass(name = "_Command", frozen, skip_from_py_object)]
#[derive(Clone)]
pub(crate) struct Command {
    config: p4rust::Config,
    name: String,
    arguments: Vec<String>,
    form: String,
    timeout: Option<Duration>,
    token: Option<CancellationToken>,
    reconcile: Option<p4rust::FastReconcile>,
}

#[pymethods]
impl Command {
    // Append arguments without changing the original builder.
    fn args(&self, args: Vec<String>) -> Self {
        let mut command = self.clone();
        command.arguments.extend(args);
        command
    }

    // Set form input without changing the original builder.
    fn input(&self, form: String) -> Self {
        let mut command = self.clone();
        command.form = form;
        command
    }

    // Convert a finite nonnegative timeout into a Rust duration.
    fn timeout(&self, seconds: f64) -> PyResult<Self> {
        let mut command = self.clone();
        command.timeout = Some(Duration::try_from_secs_f64(seconds).map_err(|error| {
            PyValueError::new_err(format!("Failed to configure timeout: {error}"))
        })?);
        Ok(command)
    }

    // Share a cancellation token without borrowing the Python object.
    fn cancellation(&self, token: &CancellationToken) -> Self {
        let mut command = self.clone();
        command.token = Some(token.clone());
        command
    }

    // Configure the existing bounded Rust reconcile pipeline.
    fn reconcile(
        &self,
        metadata: usize,
        digest: usize,
        moves: usize,
        capacity: usize,
        bytes: usize,
    ) -> PyResult<Self> {
        if [metadata, digest, moves, capacity, bytes].contains(&0) {
            return Err(PyValueError::new_err(
                "Failed to configure reconcile: limits must be positive",
            ));
        }
        let mut command = self.clone();
        command.reconcile = Some(
            p4rust::FastReconcile::new()
                .metadata_workers(metadata)
                .digest_workers(digest)
                .move_workers(moves)
                .queue_capacity(capacity)
                .queue_bytes(bytes),
        );
        Ok(command)
    }

    // Start the Rust worker while retaining no borrowed Python configuration.
    fn run(&self, py: Python<'_>) -> PyResult<Stream> {
        let client = p4rust::Client::new(self.config.clone()).map_err(Conversion::error)?;
        let mut command = client
            .command(&self.name)
            .args(self.arguments.clone())
            .input(&self.form);
        if let Some(timeout) = self.timeout {
            command = command.timeout(timeout);
        }
        if let Some(token) = &self.token {
            command = command.cancellation(&token.0);
        }
        if let Some(handler) = &self.reconcile {
            command = command.reconcile_handler(handler.clone());
        }
        let events = command.run().map_err(Conversion::error)?;
        Stream::new(py, events)
    }
}
