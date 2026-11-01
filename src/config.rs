use crate::{Command, Record, Result, ResultExt, error::ensure};
use std::fmt;

/// Explicit connection settings with instance-local credentials and protocol policy.
#[derive(Clone)]
pub struct Config {
    pub port: String,
    pub user: String,
    pub client: String,
    pub cwd: String,
    pub charset: String,
    pub password: String,
    pub ignore_file: String,
    pub tagged: bool,
}

impl Config {
    /// Create connection settings without changing process environment variables.
    pub fn new(
        port: impl Into<String>,
        user: impl Into<String>,
        client: impl Into<String>,
    ) -> Self {
        Self {
            port: port.into(),
            user: user.into(),
            client: client.into(),
            cwd: String::new(),
            charset: String::new(),
            password: String::new(),
            ignore_file: String::new(),
            tagged: true,
        }
    }

    /// Resolve SDK defaults, P4CONFIG and registry settings without connecting to a server.
    pub fn discover(cwd: impl Into<String>) -> Result<Self> {
        let mut config = Self::new("", "", "");
        config.cwd = cwd.into();
        let output = Command::new(&config, "settings")
            .local(1)
            .run()
            .context("Failed to start local SDK configuration discovery")?
            .collect_output()
            .context("Failed to discover local SDK configuration")?;
        let record = output
            .records
            .first()
            .context("Failed to read SDK configuration record")?;
        config.port = Self::field(record, "port").context("Failed to discover port")?;
        config.user = Self::field(record, "user").context("Failed to discover user")?;
        config.client = Self::field(record, "client").context("Failed to discover client")?;
        config.cwd = Self::field(record, "cwd").context("Failed to discover cwd")?;
        config.charset = Self::field(record, "charset").context("Failed to discover charset")?;
        config.password =
            Self::field(record, "password").context("Failed to discover credentials")?;
        config.ignore_file =
            Self::field(record, "ignore_file").context("Failed to discover ignore_file")?;
        config
            .validate()
            .context("Failed to validate discovered SDK configuration")?;
        Ok(config)
    }

    /// Apply the SDK's instance-local ignore policy without connecting to a server.
    pub fn is_ignored(&self, path: impl Into<String>) -> Result<bool> {
        let path = path.into();
        ensure!(
            std::path::Path::new(&path).is_absolute(),
            "Failed to check ignore policy: path must be absolute"
        );
        let output = Command::new(self, "ignored")
            .local(2)
            .args([path])
            .run()
            .context("Failed to start local SDK ignore query")?
            .collect_output()
            .context("Failed to query local SDK ignore policy")?;
        let record = output
            .records
            .first()
            .context("Failed to read SDK ignore record")?;
        match Self::field(record, "ignored")
            .context("Failed to read ignore decision")?
            .as_str()
        {
            "1" => Ok(true),
            "0" => Ok(false),
            _ => Err(crate::Error::new("Failed to decode SDK ignore decision")),
        }
    }

    /// Read one required SDK field while retaining operation-specific error context.
    fn field(record: &Record, name: &str) -> Result<String> {
        record
            .get(name)
            .map(|value| value.into_owned())
            .context(format!("Failed to read SDK configuration field {name}"))
    }

    /// Reject invalid C strings and missing explicit connection identifiers.
    pub(crate) fn validate(&self) -> Result<()> {
        for (name, value) in [
            ("port", &self.port),
            ("user", &self.user),
            ("client", &self.client),
            ("cwd", &self.cwd),
            ("charset", &self.charset),
            ("password", &self.password),
            ("ignore_file", &self.ignore_file),
        ] {
            ensure!(
                !value.contains('\0'),
                "Failed to validate {name}: embedded NUL"
            );
            if matches!(name, "port" | "user" | "client") {
                ensure!(
                    !value.is_empty(),
                    "Failed to validate {name}: value is empty"
                );
            }
        }
        Ok(())
    }
}

impl fmt::Debug for Config {
    /// Exclude credentials from diagnostics and debug formatting.
    fn fmt(&self, formatter: &mut fmt::Formatter<'_>) -> fmt::Result {
        formatter
            .debug_struct("Config")
            .field("port", &self.port)
            .field("user", &self.user)
            .field("client", &self.client)
            .field("cwd", &self.cwd)
            .field("charset", &self.charset)
            .field("ignore_file", &self.ignore_file)
            .field("tagged", &self.tagged)
            .finish_non_exhaustive()
    }
}
