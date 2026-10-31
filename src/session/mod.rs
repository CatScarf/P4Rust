use crate::{Command, CommandStatus, Config, Result, ResultExt};
use crate::{capture::Capture, control::Control, native::Native, stream::CommandStream};
use std::sync::{Arc, Mutex, atomic, mpsc};
use std::thread;

/// A clonable sequential session with one dedicated SDK connection thread.
#[derive(Clone)]
pub struct Session {
    config: Config,
    worker: Arc<Worker>,
}

struct Shared {
    closed: atomic::AtomicBool,
    active: Mutex<Option<Control>>,
}

struct Job {
    command: Command,
    control: Control,
    capture: Capture,
}

struct Worker {
    sender: mpsc::Sender<Option<Job>>,
    shared: Arc<Shared>,
    thread: Option<thread::JoinHandle<()>>,
}

impl Session {
    // Start the connection owner without opening a network connection yet.
    pub(crate) fn new(config: Config) -> Result<Self> {
        let (sender, receiver) = mpsc::channel();
        let shared = Arc::new(Shared {
            closed: atomic::AtomicBool::new(false),
            active: Mutex::new(None),
        });
        let state = Arc::clone(&shared);
        let thread = thread::Builder::new()
            .name("p4rust-session".into())
            .spawn(move || Worker::run(receiver, state))
            .context("Failed to spawn P4 session owner")?;
        Ok(Self {
            config,
            worker: Arc::new(Worker {
                sender,
                shared,
                thread: Some(thread),
            }),
        })
    }

    /// Create a command that retains this session until its stream is released.
    pub fn command(&self, name: impl Into<String>) -> Command {
        let mut command = Command::new(&self.config, name);
        command.session = Some(self.clone());
        command
    }

    /// Close all clones and cancel active work without waiting for SDK cleanup.
    pub fn close(&self) -> Result<()> {
        self.worker.close().context("Failed to close P4 session")
    }

    // Reserve the single connection and return an independently cancellable stream.
    pub(crate) fn start(&self, command: Command, control: Control) -> Result<CommandStream> {
        let mut active = self
            .worker
            .shared
            .active
            .lock()
            .map_err(|_| crate::Error::new("Failed to reserve P4 session: poisoned mutex"))?;
        crate::error::ensure!(
            !self.worker.shared.closed.load(atomic::Ordering::Acquire),
            "Failed to start P4 session command: session is closed"
        );
        crate::error::ensure!(
            active.is_none(),
            "Failed to start P4 session command: session is busy"
        );
        *active = Some(control.clone());
        let (stream, capture) = CommandStream::session(control.clone(), self.clone());
        if let Err(error) = self.worker.sender.send(Some(Job {
            command,
            control,
            capture,
        })) {
            *active = None;
            return Err(crate::Error::new(format!(
                "Failed to queue P4 session command: {error}"
            )));
        }
        Ok(stream)
    }
}

impl Worker {
    // Keep SDK creation, every command, and finalization on the same owner thread.
    fn run(receiver: mpsc::Receiver<Option<Job>>, shared: Arc<Shared>) {
        let mut native = Native::reusable();
        while let Ok(Some(job)) = receiver.recv() {
            let result =
                CommandStream::execute(&job.command, &job.control, &job.capture, &mut native);
            let result = shared.release(result);
            job.capture.complete(result);
        }
        shared.closed.store(true, atomic::Ordering::Release);
    }

    // Stop accepting work and signal cancellation before requesting owner-thread cleanup.
    fn close(&self) -> Result<()> {
        let active = self
            .shared
            .active
            .lock()
            .map_err(|_| crate::Error::new("Failed to close P4 session: poisoned mutex"))?;
        if self.shared.closed.swap(true, atomic::Ordering::AcqRel) {
            return Ok(());
        }
        if let Some(control) = active.as_ref() {
            control.cancel();
        }
        self.sender
            .send(None)
            .context("Failed to signal P4 session shutdown")
    }
}

impl Shared {
    // Release ownership before completion delivery so the next command can be submitted.
    fn release(&self, result: Result<CommandStatus>) -> Result<CommandStatus> {
        match self.active.lock() {
            Ok(mut active) => {
                *active = None;
                result
            }
            Err(_) => Err(crate::Error::new(format!(
                "Failed to release P4 session: poisoned mutex; previous result: {result:?}"
            ))),
        }
    }
}

impl Drop for Worker {
    // Request cleanup without blocking an unconsumed or uninterruptible SDK command.
    fn drop(&mut self) {
        if let Err(error) = self.close() {
            eprintln!("Failed to drop P4 session: {error}");
        }
        if let Some(worker) = self.thread.take()
            && worker.is_finished()
            && worker.join().is_err()
        {
            eprintln!("Failed to join P4 session owner: worker panicked");
        }
    }
}
