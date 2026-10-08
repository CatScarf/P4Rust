use super::{Stat, Stored};
use crate::{Result, ResultExt};

#[derive(Default)]
pub(super) struct Records {
    entries: Vec<[u8; 29]>,
    exceptions: Option<Box<Exceptions>>,
}

#[derive(Default)]
struct Exceptions {
    entries: Vec<Stored<Stat>>,
}

impl Records {
    // Encode ordinary metadata without alignment padding or a per-file Result tag.
    fn encode(&mut self, hash: u64, location: u32, value: Stored<Stat>) -> Result<[u8; 29]> {
        let stat = match value {
            Ok(stat) if (0..128).contains(&stat.flags) => stat,
            value => {
                let exceptions = self
                    .exceptions
                    .get_or_insert_with(|| Box::new(Exceptions::default()));
                let index = u64::try_from(exceptions.entries.len())
                    .context("Failed to encode metadata exception index")?;
                exceptions.entries.push(value);
                Stat {
                    size: index,
                    time: 0,
                    flags: 128,
                }
            }
        };
        let mut record = [0; 29];
        record[..8].copy_from_slice(&hash.to_le_bytes());
        record[8..12].copy_from_slice(&location.to_le_bytes());
        record[12..20].copy_from_slice(&stat.size.to_le_bytes());
        record[20..28].copy_from_slice(&stat.time.to_le_bytes());
        record[28] = stat.flags as u8;
        Ok(record)
    }
}

impl super::super::names::Storage<Stored<Stat>> for Records {
    // Report compact metadata slot count independently of exception records.
    fn len(&self) -> usize {
        self.entries.len()
    }
    // Decode full-width hashes and compact locations using aligned integer values.
    fn key(&self, index: usize) -> (u64, u32) {
        let record = &self.entries[index];
        (
            u64::from_le_bytes(std::array::from_fn(|i| record[i])),
            u32::from_le_bytes(std::array::from_fn(|i| record[8 + i])),
        )
    }
    // Restore normal metadata or the original cached error without filesystem queries.
    fn value(&self, index: usize) -> Stored<Stat> {
        let record = &self.entries[index];
        let size = u64::from_le_bytes(std::array::from_fn(|i| record[12 + i]));
        if record[28] == 128 {
            let index = usize::try_from(size)
                .context("Failed to decode metadata exception index")
                .map_err(std::sync::Arc::new)?;
            return self
                .exceptions
                .as_ref()
                .context("Failed to locate metadata exception table")
                .map_err(std::sync::Arc::new)?
                .entries
                .get(index)
                .cloned()
                .context("Failed to locate cached metadata exception")
                .map_err(std::sync::Arc::new)?;
        }
        Ok(Stat {
            size,
            time: i64::from_le_bytes(std::array::from_fn(|i| record[20 + i])),
            flags: i32::from(record[28]),
        })
    }
    // Retain one tightly encoded slot during the directory's initial read.
    fn push(&mut self, hash: u64, location: u32, value: Stored<Stat>) -> Result<()> {
        let record = self
            .encode(hash, location, value)
            .context("Failed to encode enumerated metadata")?;
        self.entries.push(record);
        Ok(())
    }
    // Cache a missing path without padding or an additional metadata query.
    fn insert(
        &mut self,
        index: usize,
        hash: u64,
        location: u32,
        value: Stored<Stat>,
    ) -> Result<()> {
        let record = self
            .encode(hash, location, value)
            .context("Failed to encode late metadata")?;
        self.entries.insert(index, record);
        Ok(())
    }
    // Release spare record capacity while preserving every cached exception.
    fn compact(&mut self) {
        self.entries.shrink_to_fit();
        if let Some(exceptions) = &mut self.exceptions {
            exceptions.entries.shrink_to_fit();
        }
    }
}
