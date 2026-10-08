use super::path::Path;
use crate::{Result, ResultExt};

struct Entry<T> {
    hash: u64,
    offset: u32,
    length: u32,
    value: T,
}
pub(super) struct Names<T> {
    bytes: String,
    entries: Vec<Entry<T>>,
}

impl<T> Names<T> {
    // Store directory-local names in one arena instead of independent allocations.
    pub(super) fn new() -> Self {
        Self {
            bytes: String::new(),
            entries: Vec::new(),
        }
    }

    // Append an enumerated name before the directory index becomes visible to readers.
    pub(super) fn push(&mut self, name: &str, value: T) -> Result<()> {
        let entry = self
            .entry(name, value)
            .context("Failed to append directory name")?;
        self.entries.push(entry);
        Ok(())
    }

    // Sort the compact index once and release enumeration's spare capacity.
    pub(super) fn finish(&mut self) {
        self.entries
            .sort_unstable_by_key(|entry| (entry.hash, std::cmp::Reverse(entry.offset)));
        self.entries.shrink_to_fit();
        self.bytes.shrink_to_fit();
    }

    // Find a cached value using borrowed name bytes and collision-safe comparison.
    pub(super) fn get(&self, name: &str) -> Option<&T> {
        self.position(name)
            .ok()
            .map(|index| &self.entries[index].value)
    }

    // Cache missing paths and lazy content slots without allocating on lookup hits.
    pub(super) fn get_or_insert_with(
        &mut self,
        name: &str,
        create: impl FnOnce() -> T,
    ) -> Result<&T> {
        let index = match self.position(name) {
            Ok(index) => index,
            Err(index) => {
                let entry = self
                    .entry(name, create())
                    .context("Failed to cache directory name")?;
                self.entries.insert(index, entry);
                index
            }
        };
        Ok(&self.entries[index].value)
    }

    // Locate an equal name or its insertion point among equal cached hashes.
    fn position(&self, name: &str) -> std::result::Result<usize, usize> {
        let hash = Path::hash_of(name);
        let first = self.entries.partition_point(|entry| entry.hash < hash);
        for (index, entry) in self.entries.iter().enumerate().skip(first) {
            if entry.hash != hash {
                break;
            }
            let start = entry.offset as usize;
            let end = start + entry.length as usize;
            if Path::equivalent(&self.bytes[start..end], name) {
                return Ok(index);
            }
        }
        Err(first)
    }

    // Validate compact offsets before appending an immutable UTF-8 name to the arena.
    fn entry(&mut self, name: &str, value: T) -> Result<Entry<T>> {
        let offset = u32::try_from(self.bytes.len()).context("Failed to encode name offset")?;
        let length = u32::try_from(name.len()).context("Failed to encode name length")?;
        offset
            .checked_add(length)
            .context("Failed to fit directory names within 4 GiB")?;
        self.bytes.push_str(name);
        Ok(Entry {
            hash: Path::hash_of(name),
            offset,
            length,
            value,
        })
    }
}
