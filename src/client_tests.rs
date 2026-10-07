use crate::{Client, Config, Error, Result, ResultExt};

struct Fixture;

impl Fixture {
    // Verify fingerprint trust, rejected passwords, and ticket authentication over TLS.
    fn authenticate(client: &Client) -> Result<()> {
        let fingerprint = std::env::var("P4RUST_TEST_FINGERPRINT")
            .context("Failed to read isolated server fingerprint")?;
        assert!(client.run("info", &[]).is_err());
        client
            .run("trust", &["-i", &fingerprint])
            .context("Failed to trust isolated server fingerprint")?;
        client
            .run_with_input(
                "user",
                &["-i"],
                "User: tester\nEmail: fixture@example.invalid\nFullName: TLS Fixture\n",
            )
            .context("Failed to create isolated authentication user")?;
        client
            .run("configure", &["set", "security=3"])
            .context("Failed to enable isolated ticket authentication")?;
        let password = "P4Rust-Fixture-Only-42!";
        client
            .run_with_input("passwd", &[], password)
            .context("Failed to set isolated secure test password")?;
        client
            .run_with_input("login", &[], password)
            .context("Failed to obtain initial test ticket")?;
        client
            .run("logout", &[])
            .context("Failed to clear test ticket")?;
        assert!(client.run("login", &["-s"]).is_err());
        assert!(
            client
                .run_with_input("login", &[], "incorrect-password")
                .is_err()
        );
        client
            .run_with_input("login", &[], password)
            .context("Failed to authenticate explicitly over TLS")?;
        client
            .run("login", &["-s"])
            .context("Failed to validate TLS login ticket")?;
        Ok(())
    }

    // Construct an offline client for boundary validation tests.
    fn offline() -> Result<Client> {
        Client::new(Config::new("127.0.0.1:1", "tester", "test-client"))
            .context("Failed to construct offline test client")
    }

    // Configure an isolated workspace with an explicit non-Unicode charset.
    fn local() -> Result<(Client, std::path::PathBuf)> {
        let port = std::env::var("P4RUST_TEST_PORT").context("Failed to read test server port")?;
        let timestamp = std::time::SystemTime::now()
            .duration_since(std::time::UNIX_EPOCH)
            .context("Failed to timestamp disposable test workspace")?
            .as_nanos();
        let root = std::env::current_dir()
            .context("Failed to read test project root")?
            .join(format!("temp/server-workspace-{timestamp}"));
        std::fs::create_dir_all(&root).context("Failed to create test workspace root")?;
        let mut config = Config::new(port, "tester", "test-client");
        config.cwd = root.to_string_lossy().into_owned();
        config.charset = "none".to_owned();
        let client = Client::new(config).context("Failed to create local test client")?;
        Ok((client, root))
    }

    // Verify text and binary output using files on the disposable test server.
    fn files(client: &Client, root: &std::path::Path) -> Result<()> {
        let text_path = root.join("text.txt");
        let binary_path = root.join("binary.dat");
        std::fs::write(&text_path, b"P4Rust text\n")
            .context("Failed to write text test fixture")?;
        let bytes = [0_u8, 1, 127, 128, 255];
        std::fs::write(&binary_path, bytes).context("Failed to write binary test fixture")?;
        let text = text_path.to_string_lossy();
        let binary = binary_path.to_string_lossy();
        client
            .run("add", &["-t", "text", &text])
            .context("Failed to add text fixture")?;
        client
            .run("add", &["-t", "binary", &binary])
            .context("Failed to add binary fixture")?;
        client
            .run("submit", &["-d", "Add native binding test fixtures"])
            .context("Failed to submit disposable test fixtures")?;
        let text_output = client
            .run("print", &["-q", "//depot/text.txt"])
            .context("Failed to print text fixture")?;
        assert!(text_output.text.contains("P4Rust text"));
        let binary_output = client
            .run("print", &["-q", "//depot/binary.dat"])
            .context("Failed to print binary fixture")?;
        assert_eq!(binary_output.binary, bytes);
        Ok(())
    }
}

// Exercise certificate trust and authenticated text/binary transfer on an SSL server.
#[test]
#[ignore = "Requires an isolated SSL server and P4RUST_TEST_FINGERPRINT"]
fn ssl_server_commands() -> Result<()> {
    let (client, _) = Fixture::local().context("Failed to prepare SSL authentication test")?;
    Fixture::authenticate(&client).context("Failed to verify SSL authentication")?;
    local_server_commands().context("Failed to verify authenticated SSL data transfer")
}

// Reject invalid configuration before initializing the native API.
#[test]
fn rejects_invalid_configuration() {
    assert!(Client::new(Config::new("", "tester", "client")).is_err());
    assert!(Client::new(Config::new("localhost:1666", "test\0er", "client")).is_err());
}

// Reject embedded NUL bytes before they can truncate native arguments.
#[test]
fn rejects_invalid_arguments() -> Result<()> {
    let client = Fixture::offline().context("Failed to prepare argument test")?;
    assert!(client.run("info\0", &[]).is_err());
    assert!(client.run("info", &["bad\0argument"]).is_err());
    assert!(
        client
            .run_with_input("client", &["-i"], "bad\0input")
            .is_err()
    );
    Ok(())
}

// Propagate native connection failures with the command and operation context.
#[test]
fn reports_connection_failure() -> Result<()> {
    let client = Fixture::offline().context("Failed to prepare connection test")?;
    match client.run("info", &[]) {
        Ok(_) => {
            return Err(Error::new(
                "Failed to verify connection error: unexpectedly connected",
            ));
        }
        Err(error) => {
            let message = format!("{error:#}");
            assert!(message.contains("info"));
            assert!(message.contains("Failed to initialize P4 connection"));
        }
    }
    Ok(())
}

// Verify tagged output, form input, and server errors against a disposable server.
#[test]
#[ignore = "Requires P4RUST_TEST_PORT pointing to a disposable local p4d"]
fn local_server_commands() -> Result<()> {
    let offline = Fixture::offline().context("Failed to prepare exception recovery test")?;
    assert!(offline.run("info", &[]).is_err());
    let (client, root) = Fixture::local().context("Failed to prepare local server test")?;
    let info = client
        .run("info", &[])
        .context("Failed to query test server info")?;
    assert!(
        info.records
            .iter()
            .flatten()
            .any(|(key, _)| key == "serverVersion")
    );
    let form = format!(
        "Client: test-client\nOwner: tester\nRoot: {}\nView:\n\t//depot/... //test-client/...\n",
        root.display()
    );
    client
        .run_with_input("client", &["-i"], &form)
        .context("Failed to create disposable client specification")?;
    let spec = client
        .run("client", &["-o"])
        .context("Failed to read client specification")?;
    assert!(
        spec.records
            .iter()
            .flatten()
            .any(|(key, value)| key == "Client" && value == "test-client")
    );
    assert!(client.run("p4rust-invalid-command", &[]).is_err());
    Fixture::files(&client, &root).context("Failed to verify native output callbacks")?;
    Ok(())
}
