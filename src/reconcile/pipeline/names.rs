mod codec;
mod storage;

use super::path::Path;
use crate::{Result, ResultExt};
pub(super) use storage::{Storage, Typed};

pub(super) struct Names<T, S = Typed<T>> {
    bytes: codec::Arena,
    entries: S,
    overflow: Option<Box<Overflow>>,
    marker: std::marker::PhantomData<T>,
}

#[derive(Default)]
struct Overflow {
    locations: Vec<(u32, u32)>,
}

impl<T, S: Storage<T>> Names<T, S> {
    // Retain compressed spelling and fixed-width keys in directory-local arenas.
    pub(super) fn new() -> Self {
        Self {
            bytes: codec::Arena::new(),
            entries: S::default(),
            overflow: None,
            marker: std::marker::PhantomData,
        }
    }

    // Append an enumerated name before the directory index becomes visible to readers.
    pub(super) fn push(&mut self, name: &str, value: T) -> Result<()> {
        let location = self
            .entry(name)
            .context("Failed to encode enumerated filename")?;
        self.entries
            .push(Path::hash_of(name), location, value)
            .context("Failed to retain enumerated file slot")
    }

    // Share adjacent name prefixes before publishing a collision-safe sorted hash index.
    pub(super) fn finish(&mut self) -> Result<()> {
        let mut order: Vec<_> = (0..self.entries.len()).collect();
        order.sort_unstable_by(|left, right| {
            self.entries
                .key(*left)
                .0
                .cmp(&self.entries.key(*right).0)
                .then_with(|| {
                    Path::characters(self.raw(*left)).cmp(Path::characters(self.raw(*right)))
                })
                .then_with(|| self.location(*right).0.cmp(&self.location(*left).0))
        });
        order.dedup_by(|left, right| {
            self.entries.key(*left).0 == self.entries.key(*right).0
                && Path::equivalent(self.raw(*left), self.raw(*right))
        });
        order.sort_unstable_by(|left, right| self.raw(*left).cmp(self.raw(*right)));
        *self = self
            .compress(&order)
            .context("Failed to compress sorted filename index")?;
        self.entries.compact();
        self.bytes.compact();
        Ok(())
    }

    // Encode name locations before reordering values without changing their cached identity.
    fn compress(&self, order: &[usize]) -> Result<Self> {
        let mut result = Self::new();
        let mut records = Vec::with_capacity(order.len());
        let mut previous = "";
        let mut block = 0;
        for (index, source) in order.iter().copied().enumerate() {
            let name = self.raw(source);
            let ordinal = index as u32 % 8;
            let prefix = if ordinal == 0 {
                0
            } else {
                name.bytes()
                    .zip(previous.bytes())
                    .take_while(|(left, right)| left == right)
                    .count()
            };
            let offset = result
                .bytes
                .append(prefix, &name.as_bytes()[prefix..])
                .context("Failed to compress directory filename")?;
            if ordinal == 0 {
                block = offset;
            }
            let location = result
                .encode(block, ordinal)
                .context("Failed to encode compressed name location")?;
            records.push((self.entries.key(source).0, location, source));
            previous = name;
        }
        records.sort_unstable_by_key(|record| record.0);
        for (hash, location, source) in records {
            result
                .entries
                .push(hash, location, self.entries.value(source))
                .context("Failed to retain compressed file slot")?;
        }
        Ok(result)
    }

    // Borrow an initial standalone name before its directory has been compressed.
    fn raw(&self, index: usize) -> &str {
        self.bytes.raw(self.location(index).0)
    }

    // Return an owned cached value while preserving shared content and error ownership.
    pub(super) fn get(&self, name: &str) -> Option<T> {
        self.position(name)
            .ok()
            .map(|index| self.entries.value(index))
    }

    // Cache missing paths and lazy content slots without allocating on ordinary lookup hits.
    pub(super) fn get_or_insert_with(
        &mut self,
        name: &str,
        create: impl FnOnce() -> T,
    ) -> Result<T> {
        let index = match self.position(name) {
            Ok(index) => index,
            Err(index) => {
                let location = self.entry(name).context("Failed to encode late filename")?;
                self.entries
                    .insert(index, Path::hash_of(name), location, create())
                    .context("Failed to cache directory file slot")?;
                index
            }
        };
        Ok(self.entries.value(index))
    }

    // Locate an equal original spelling or its insertion point among equal cached hashes.
    fn position(&self, name: &str) -> std::result::Result<usize, usize> {
        let hash = Path::hash_of(name);
        let mut lower = 0;
        let mut upper = self.entries.len();
        while lower < upper {
            let middle = lower + (upper - lower) / 2;
            if self.entries.key(middle).0 < hash {
                lower = middle + 1;
            } else {
                upper = middle;
            }
        }
        let first = lower;
        let mut scratch = codec::Scratch::new();
        while lower < self.entries.len() && self.entries.key(lower).0 == hash {
            let (offset, ordinal) = self.location(lower);
            if Path::equivalent(self.bytes.name(offset, ordinal, &mut scratch), name) {
                return Ok(lower);
            }
            lower += 1;
        }
        Err(first)
    }

    // Retain a standalone spelling for enumeration or a missing-path classification.
    fn entry(&mut self, name: &str) -> Result<u32> {
        let offset = self
            .bytes
            .append(0, name.as_bytes())
            .context("Failed to retain original filename")?;
        self.encode(offset, 0)
            .context("Failed to encode standalone name location")
    }

    // Pack ordinary locations and retain full-width offsets in a lazy overflow table.
    fn encode(&mut self, offset: u32, ordinal: u32) -> Result<u32> {
        if offset < 1 << 28 {
            return Ok(offset << 3 | ordinal);
        }
        let overflow = self
            .overflow
            .get_or_insert_with(|| Box::new(Overflow::default()));
        let index = u32::try_from(overflow.locations.len())
            .context("Failed to encode overflow location index")?;
        if index >= 1 << 31 {
            return Err(crate::Error::new(
                "Failed to fit overflow locations within 31 bits",
            ));
        }
        overflow.locations.push((offset, ordinal));
        Ok(1 << 31 | index)
    }

    // Decode an internally validated location without discarding large-directory offsets.
    fn location(&self, index: usize) -> (u32, u32) {
        let location = self.entries.key(index).1;
        if location & (1 << 31) == 0 {
            (location >> 3, location & 7)
        } else {
            // Overflow markers are published only after their table entry has been inserted.
            let locations = self
                .overflow
                .as_deref()
                .map(|overflow| overflow.locations.as_slice())
                .unwrap_or_default();
            locations[(location & !(1 << 31)) as usize]
        }
    }
}
