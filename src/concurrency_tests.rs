use crate as api;
use api::ResultExt;
use std::{io, net, thread, time};

struct Fixture;

impl Fixture {
    // Bind an isolated nonresponding endpoint without depending on a Perforce server.
    fn server() -> api::Result<(net::TcpListener, api::Client)> {
        let listener = net::TcpListener::bind("127.0.0.1:0")
            .context("Failed to bind blocked server fixture")?;
        listener
            .set_nonblocking(true)
            .context("Failed to configure blocked server fixture")?;
        let address = listener
            .local_addr()
            .context("Failed to read blocked server address")?;
        let client = api::Client::new(api::Config::new(address.to_string(), "tester", "fixture"))
            .context("Failed to configure blocked server client")?;
        Ok((listener, client))
    }

    // Wait for a native connection with a bounded fixture timeout.
    fn accept(listener: &net::TcpListener) -> api::Result<net::TcpStream> {
        let deadline = time::Instant::now() + time::Duration::from_secs(2);
        loop {
            match listener.accept() {
                Ok((stream, _)) => return Ok(stream),
                Err(error)
                    if error.kind() == io::ErrorKind::WouldBlock
                        && time::Instant::now() < deadline =>
                {
                    thread::sleep(time::Duration::from_millis(5));
                }
                Err(error) => {
                    return Err(error).context("Failed to accept blocked native connection");
                }
            }
        }
    }

    // Find the machine-readable interruption cause through contextual wrappers.
    fn kind(error: &api::Error) -> Option<io::ErrorKind> {
        let mut cause: &dyn std::error::Error = error;
        loop {
            if let Some(error) = cause.downcast_ref::<io::Error>() {
                return Some(error.kind());
            }
            cause = cause.source()?;
        }
    }

    // Release Unix initialization stalls and confirm the owned native session closes its socket.
    fn closed(mut stream: net::TcpStream) -> api::Result<()> {
        use io::Read;
        stream
            .set_nonblocking(false)
            .context("Failed to configure cleanup socket")?;
        stream
            .set_read_timeout(Some(time::Duration::from_secs(3)))
            .context("Failed to bound native cleanup wait")?;
        if cfg!(unix) {
            // Unix SDK initialization waits for a server handshake before SetBreak is available.
            stream
                .shutdown(net::Shutdown::Write)
                .context("Failed to release stalled fixture handshake")?;
        }
        let mut buffer = [0; 4096];
        loop {
            match stream.read(&mut buffer) {
                Ok(0) => return Ok(()),
                Ok(_) => continue,
                Err(error)
                    if matches!(
                        error.kind(),
                        io::ErrorKind::ConnectionReset | io::ErrorKind::ConnectionAborted
                    ) =>
                {
                    return Ok(());
                }
                Err(error) => {
                    return Err(error).context("Failed to observe native cancellation cleanup");
                }
            }
        }
    }
}

// Prove shared and separate Clients connect concurrently before any command is released.
#[test]
fn clients_execute_without_a_global_lock() -> api::Result<()> {
    let (listener, client) = Fixture::server().context("Failed to prepare concurrent commands")?;
    let second = api::Client::new(client.config.clone())
        .context("Failed to create separate concurrent client")?;
    let options = api::RunOptions {
        timeout: Some(time::Duration::from_secs(5)),
        ..api::RunOptions::default()
    };
    thread::scope(|scope| -> api::Result<()> {
        let workers: Vec<_> = [&client, &client, &second]
            .into_iter()
            .map(|client| {
                let options = &options;
                scope.spawn(move || client.run_with_options("info", &[], options))
            })
            .collect();
        let mut connections = Vec::new();
        for _ in 0..3 {
            connections.push(
                Fixture::accept(&listener)
                    .context("Failed to establish overlapping native sessions")?,
            );
        }
        assert_eq!(connections.len(), 3);
        options.cancellation.cancel();
        for worker in workers {
            let result = worker
                .join()
                .map_err(|_| api::Error::new("Concurrent caller panicked"))
                .context("Failed to join concurrent caller")?;
            let error = result
                .err()
                .context("Failed to interrupt blocked concurrent command")?;
            assert_eq!(Fixture::kind(&error), Some(io::ErrorKind::Interrupted));
        }
        for stream in connections {
            Fixture::closed(stream).context("Failed to close concurrent session")?;
        }
        Ok(())
    })
    .context("Failed to verify concurrent clients")
}

// Return a precise deadline error and release native resources on a stalled command.
#[test]
fn times_out_a_blocked_command() -> api::Result<()> {
    let (listener, client) = Fixture::server().context("Failed to prepare timeout fixture")?;
    let start = time::Instant::now();
    let worker = thread::spawn(move || {
        client.run_with_options(
            "info",
            &[],
            &api::RunOptions {
                timeout: Some(time::Duration::from_millis(300)),
                ..api::RunOptions::default()
            },
        )
    });
    let stream = Fixture::accept(&listener).context("Failed to begin blocked command")?;
    let result = worker
        .join()
        .map_err(|_| api::Error::new("Timeout caller panicked"))
        .context("Failed to join timeout caller")?;
    let error = result
        .err()
        .context("Failed to interrupt stalled command")?;
    assert_eq!(Fixture::kind(&error), Some(io::ErrorKind::TimedOut));
    assert!(start.elapsed() < time::Duration::from_secs(2));
    Fixture::closed(stream).context("Failed to clean up timed-out command")
}

// Cancel a running command from another thread without an application mutex.
#[test]
fn cancels_a_blocked_command() -> api::Result<()> {
    let (listener, client) = Fixture::server().context("Failed to prepare cancellation fixture")?;
    let token = api::CancellationToken::default();
    let options = api::RunOptions {
        cancellation: token.clone(),
        ..api::RunOptions::default()
    };
    let worker = thread::spawn(move || client.run_with_options("info", &[], &options));
    let stream = Fixture::accept(&listener).context("Failed to begin cancellable command")?;
    let start = time::Instant::now();
    token.cancel();
    let result = worker
        .join()
        .map_err(|_| api::Error::new("Cancellation caller panicked"))
        .context("Failed to join cancellation caller")?;
    let error = result.err().context("Failed to cancel running command")?;
    assert_eq!(Fixture::kind(&error), Some(io::ErrorKind::Interrupted));
    assert!(start.elapsed() < time::Duration::from_secs(1));
    Fixture::closed(stream).context("Failed to clean up cancelled command")
}

// Reject cancelled and expired commands before starting a native connection.
#[test]
fn rejects_preinterrupted_commands() -> api::Result<()> {
    let client = api::Client::new(api::Config::new("127.0.0.1:1", "tester", "fixture"))
        .context("Failed to prepare interrupted client")?;
    let options = api::RunOptions::default();
    options.cancellation.cancel();
    let error = client
        .run_with_options("info", &[], &options)
        .err()
        .context("Failed to reject pre-cancelled command")?;
    assert_eq!(Fixture::kind(&error), Some(io::ErrorKind::Interrupted));
    let options = api::RunOptions {
        timeout: Some(time::Duration::ZERO),
        ..api::RunOptions::default()
    };
    let error = client
        .run_with_options("info", &[], &options)
        .err()
        .context("Failed to reject expired deadline")?;
    assert_eq!(Fixture::kind(&error), Some(io::ErrorKind::TimedOut));
    Ok(())
}

// Return the caller deadline while TLS setup is blocked and native state remains owned.
#[test]
fn times_out_during_connection_setup() -> api::Result<()> {
    let (listener, _) = Fixture::server().context("Failed to prepare blocked TLS setup")?;
    let address = listener
        .local_addr()
        .context("Failed to read TLS fixture address")?;
    let client = api::Client::new(api::Config::new(
        format!("ssl:{address}"),
        "tester",
        "fixture",
    ))
    .context("Failed to configure TLS setup client")?;
    let start = time::Instant::now();
    let worker = thread::spawn(move || {
        client.run_with_options(
            "info",
            &[],
            &api::RunOptions {
                timeout: Some(time::Duration::from_millis(300)),
                ..api::RunOptions::default()
            },
        )
    });
    let stream = Fixture::accept(&listener).context("Failed to begin blocked TLS handshake")?;
    let result = worker
        .join()
        .map_err(|_| api::Error::new("TLS timeout caller panicked"))
        .context("Failed to join TLS timeout caller")?;
    let error = result
        .err()
        .context("Failed to interrupt blocked TLS setup")?;
    assert_eq!(Fixture::kind(&error), Some(io::ErrorKind::TimedOut));
    assert!(start.elapsed() < time::Duration::from_secs(2));
    drop(stream);
    Ok(())
}
