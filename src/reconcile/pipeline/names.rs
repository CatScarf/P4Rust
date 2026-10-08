mod codec;

use super::path::Path;
use crate::{Result, ResultExt};

struct Entry<T> {
    hash: u64,
    offset: u32,
    ordinal: u32,
    value: T,
}
pub(super) struct Names<T> {
    bytes: codec::Arena,
    entries: Vec<Entry<T>>,
}

impl<T> Names<T> {
    // Store directory-local names in one arena instead of independent allocations.
    pub(super) fn new() -> Self {
        Self {
            bytes: codec::Arena::new(),
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

    // Share adjacent name prefixes in bounded blocks and keep collision-safe hash lookups.
    pub(super) fn finish(&mut self) -> Result<()> {
        self.deduplicate();
        self.entries.sort_unstable_by(|left, right| {
            self.bytes
                .raw(left.offset)
                .cmp(self.bytes.raw(right.offset))
        });
        let mut compressed = codec::Arena::new();
        let mut previous = "";
        let mut block = 0;
        for (index, entry) in self.entries.iter_mut().enumerate() {
            let name = self.bytes.raw(entry.offset);
            let ordinal = index as u32 % 8;
            let prefix = if ordinal == 0 {
                0
            } else {
                name.bytes()
                    .zip(previous.bytes())
                    .take_while(|(left, right)| left == right)
                    .count()
            };
            let offset = compressed
                .append(prefix, &name.as_bytes()[prefix..])
                .context("Failed to compress directory filename")?;
            if ordinal == 0 {
                block = offset;
            }
            entry.offset = block;
            entry.ordinal = ordinal;
            previous = name;
        }
        self.bytes = compressed;
        self.entries.sort_unstable_by_key(|entry| entry.hash);
        self.entries.shrink_to_fit();
        self.bytes.compact();
        Ok(())
    }

    // Retain the newest normalized spelling before the physical compression order changes.
    fn deduplicate(&mut self) {
        self.entries.sort_unstable_by(|left, right| {
            left.hash
                .cmp(&right.hash)
                .then_with(|| {
                    Path::characters(self.bytes.raw(left.offset))
                        .cmp(Path::characters(self.bytes.raw(right.offset)))
                })
                .then_with(|| right.offset.cmp(&left.offset))
        });
        self.entries.dedup_by(|left, right| {
            left.hash == right.hash
                && Path::equivalent(self.bytes.raw(left.offset), self.bytes.raw(right.offset))
        });
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
        let mut scratch = codec::Scratch::new();
        for (index, entry) in self.entries.iter().enumerate().skip(first) {
            if entry.hash != hash {
                break;
            }
            if Path::equivalent(
                self.bytes.name(entry.offset, entry.ordinal, &mut scratch),
                name,
            ) {
                return Ok(index);
            }
        }
        Err(first)
    }

    // Append a standalone original spelling for enumeration or a late missing-path slot.
    fn entry(&mut self, name: &str, value: T) -> Result<Entry<T>> {
        let offset = self
            .bytes
            .append(0, name.as_bytes())
            .context("Failed to retain original filename")?;
        Ok(Entry {
            hash: Path::hash_of(name),
            offset,
            ordinal: 0,
            value,
        })
    }
}
