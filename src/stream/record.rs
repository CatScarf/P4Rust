use std::{borrow::Cow, ops::Range};

/// One SDK tagged callback with a single owned byte buffer and ordered field offsets.
#[derive(Debug)]
pub struct Record {
    pub(crate) bytes: Vec<u8>,
    pub(crate) spans: Vec<(Range<usize>, Range<usize>)>,
}

impl Record {
    /// Borrow each original key and value in the SDK's field order.
    pub fn raw_fields(&self) -> impl ExactSizeIterator<Item = (&[u8], &[u8])> {
        self.spans
            .iter()
            .map(|(key, value)| (&self.bytes[key.clone()], &self.bytes[value.clone()]))
    }

    /// Read field text lazily, borrowing valid UTF-8 and replacing invalid bytes on demand.
    pub fn fields(&self) -> impl ExactSizeIterator<Item = (Cow<'_, str>, Cow<'_, str>)> {
        self.raw_fields()
            .map(|(key, value)| (String::from_utf8_lossy(key), String::from_utf8_lossy(value)))
    }

    /// Find the first original value without allocating a string or field collection.
    pub fn get_raw(&self, key: &[u8]) -> Option<&[u8]> {
        self.raw_fields()
            .find_map(|(name, value)| (name == key).then_some(value))
    }

    /// Find display text while borrowing the stored bytes whenever UTF-8 is valid.
    pub fn get(&self, key: &str) -> Option<Cow<'_, str>> {
        self.get_raw(key.as_bytes()).map(String::from_utf8_lossy)
    }
}
