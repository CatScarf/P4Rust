use crate::Result;

pub(in crate::reconcile::pipeline) trait Storage<T>:
    Default
{
    // Report the number of retained file identities.
    fn len(&self) -> usize;
    // Borrow the normalized hash and encoded original-name location.
    fn key(&self, index: usize) -> (u64, u32);
    // Materialize one cached value with its existing shared ownership.
    fn value(&self, index: usize) -> T;
    // Append one initial file identity and cached value.
    fn push(&mut self, hash: u64, location: u32, value: T) -> Result<()>;
    // Insert a late identity at its sorted hash position.
    fn insert(&mut self, index: usize, hash: u64, location: u32, value: T) -> Result<()>;
    // Release unused capacity after initial directory enumeration.
    fn compact(&mut self);
}

struct Entry<T> {
    hash: u64,
    location: u32,
    value: T,
}
pub(in crate::reconcile::pipeline) struct Typed<T> {
    entries: Vec<Entry<T>>,
}

impl<T> Default for Typed<T> {
    // Allocate ordinary typed slots lazily for canonical-content ownership.
    fn default() -> Self {
        Self {
            entries: Vec::new(),
        }
    }
}

impl<T: Clone> Storage<T> for Typed<T> {
    // Report the number of typed cached slots.
    fn len(&self) -> usize {
        self.entries.len()
    }
    // Read the compact key without copying its value.
    fn key(&self, index: usize) -> (u64, u32) {
        let entry = &self.entries[index];
        (entry.hash, entry.location)
    }
    // Clone shared ownership when a caller consumes a cached slot.
    fn value(&self, index: usize) -> T {
        self.entries[index].value.clone()
    }
    // Append an owned typed value during enumeration.
    fn push(&mut self, hash: u64, location: u32, value: T) -> Result<()> {
        self.entries.push(Entry {
            hash,
            location,
            value,
        });
        Ok(())
    }
    // Publish a late owned value at its hash insertion point.
    fn insert(&mut self, index: usize, hash: u64, location: u32, value: T) -> Result<()> {
        self.entries.insert(
            index,
            Entry {
                hash,
                location,
                value,
            },
        );
        Ok(())
    }
    // Retain only the occupied typed index capacity.
    fn compact(&mut self) {
        self.entries.shrink_to_fit();
    }
}
