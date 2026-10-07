use crate::error::ensure;
use crate::{Message, MessageId, Result, ResultExt};

struct Reader<'a> {
    bytes: &'a [u8],
}

impl Reader<'_> {
    // Consume a checked span from the SDK's serialized message.
    fn take(&mut self, count: usize) -> Result<&[u8]> {
        ensure!(
            count <= self.bytes.len(),
            "Failed to read truncated SDK message"
        );
        let (bytes, rest) = self.bytes.split_at(count);
        self.bytes = rest;
        Ok(bytes)
    }

    // Decode the little-endian integers used by StrOps::PackInt.
    fn number(&mut self) -> Result<i32> {
        let bytes = self.take(4).context("Failed to read SDK message integer")?;
        Ok(i32::from_le_bytes(
            bytes.try_into().context("Failed to decode SDK integer")?,
        ))
    }

    // Decode a checked length-prefixed string without changing its bytes.
    fn string(&mut self) -> Result<Vec<u8>> {
        let length = usize::try_from(self.number().context("Failed to read SDK string length")?)
            .context("Failed to validate SDK string length")?;
        Ok(self
            .take(length)
            .context("Failed to read SDK string")?
            .to_vec())
    }
}

impl Message {
    // Decode exact SDK message identifiers, parameters, and serialized bytes.
    pub(crate) fn decode(raw: &[u8], serialized: &[u8]) -> Result<Self> {
        ensure!(
            raw.len() <= 1_048_576 && serialized.len() <= 1_048_576 - raw.len(),
            "Failed to bound SDK message size"
        );
        let mut reader = Reader { bytes: serialized };
        let severity = reader.number().context("Failed to read message severity")?;
        let mut message = Self {
            severity,
            generic: None,
            ids: Vec::new(),
            parameters: Vec::new(),
            text: String::from_utf8_lossy(raw).into_owned(),
            raw: raw.to_vec(),
            serialized: serialized.to_vec(),
        };
        if severity != 0 {
            message.generic = Some(
                reader
                    .number()
                    .context("Failed to read generic error code")?,
            );
            let count = reader
                .number()
                .context("Failed to read message identifier count")?;
            ensure!(
                (0..=16_384).contains(&count),
                "Failed to bound message identifier count"
            );
            for _ in 0..count {
                let code = reader
                    .number()
                    .context("Failed to read native error code")?;
                let format = reader
                    .string()
                    .context("Failed to read native error format")?;
                ensure!(
                    reader.take(1).context("Failed to read format terminator")? == [0],
                    "Failed to validate native error format terminator"
                );
                message.ids.push(MessageId { code, format });
            }
            while !reader.bytes.is_empty() {
                ensure!(
                    message.parameters.len() < 16_384,
                    "Failed to bound error parameters"
                );
                let key = reader
                    .string()
                    .context("Failed to read error parameter name")?;
                let value = reader
                    .string()
                    .context("Failed to read error parameter value")?;
                message.parameters.push((key, value));
            }
        }
        ensure!(
            reader.bytes.is_empty(),
            "Failed to consume native message frame"
        );
        Ok(message)
    }
}
