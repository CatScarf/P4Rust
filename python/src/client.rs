use crate::{conversion::Conversion, stream::Stream};
use p4rust::ResultExt;
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
    #[pyo3(signature = (port, user, client, cwd, charset, password=String::new(), ignore_file=String::new(), tagged=true))]
    #[allow(clippy::too_many_arguments)]
    fn new(
        port: String,
        user: String,
        client: String,
        cwd: String,
        charset: String,
        password: String,
        ignore_file: String,
        tagged: bool,
    ) -> PyResult<Self> {
        let mut config = p4rust::Config::new(port, user, client);
        config.cwd = cwd;
        config.charset = charset;
        config.password = password;
        config.ignore_file = ignore_file;
        config.tagged = tagged;
        p4rust::Client::new(config.clone()).map_err(Conversion::error)?;
        Ok(Self(config))
    }

    // Resolve SDK connection settings without opening a server connection.
    #[staticmethod]
    fn discover(
        py: Python<'_>,
        cwd: String,
    ) -> PyResult<(String, String, String, String, String, String, String)> {
        let config = py
            .detach(|| p4rust::Config::discover(cwd))
            .map_err(Conversion::error)?;
        Ok((
            config.port,
            config.user,
            config.client,
            config.cwd,
            config.charset,
            config.ignore_file,
            config.password,
        ))
    }

    // Apply this client's SDK ignore policy without server traffic.
    fn is_ignored(&self, py: Python<'_>, path: String) -> PyResult<bool> {
        py.detach(|| {
            let client = p4rust::Client::new(self.0.clone())
                .context("Failed to construct local SDK ignore client")?;
            client.is_ignored(path)
        })
        .map_err(Conversion::error)
    }

    // Create an immutable command builder with owned connection settings.
    fn command(&self, name: String) -> Command {
        Command::new(self.0.clone(), name, None)
    }

    // Create a persistent SDK connection owner without borrowing this Python client.
    fn session(&self) -> PyResult<Session> {
        let client = p4rust::Client::new(self.0.clone()).map_err(Conversion::error)?;
        Ok(Session {
            inner: client.session().map_err(Conversion::error)?,
            config: self.0.clone(),
        })
    }
}

#[pyclass(name = "_Session", frozen)]
pub(crate) struct Session {
    inner: p4rust::Session,
    config: p4rust::Config,
}

#[pymethods]
impl Session {
    // Retain the connection owner in each immutable command builder.
    fn command(&self, name: String) -> Command {
        Command::new(self.config.clone(), name, Some(self.inner.clone()))
    }

    // Cancel active work and request finalization on the native owner thread.
    fn close(&self) -> PyResult<()> {
        self.inner.close().map_err(Conversion::error)
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
    session: Option<p4rust::Session>,
}

impl Command {
    // Build owned command state for an isolated client or persistent session.
    fn new(config: p4rust::Config, name: String, session: Option<p4rust::Session>) -> Self {
        Self {
            config,
            name,
            session,
            arguments: Vec::new(),
            form: String::new(),
            timeout: None,
            token: None,
            reconcile: None,
        }
    }
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
        let command = match &self.session {
            Some(session) => session.command(&self.name),
            None => client.command(&self.name),
        };
        let mut command = command.args(self.arguments.clone()).input(&self.form);
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
