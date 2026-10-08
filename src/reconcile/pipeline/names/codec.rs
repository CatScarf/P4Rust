use crate::{Result, ResultExt};

pub(super) struct Arena {
    bytes: Vec<u8>,
}
pub(super) struct Scratch {
    small: [u8; 1024],
    large: Vec<u8>,
    length: usize,
}

impl Arena {
    // Retain encoded names in one directory-local allocation.
    pub(super) fn new() -> Self {
        Self { bytes: Vec::new() }
    }

    // Append a prefix length and an exact suffix without retaining another name copy.
    pub(super) fn append(&mut self, prefix: usize, suffix: &[u8]) -> Result<u32> {
        let offset =
            u32::try_from(self.bytes.len()).context("Failed to encode name block offset")?;
        let length = u32::try_from(suffix.len()).context("Failed to encode name suffix length")?;
        let prefix = u32::try_from(prefix).context("Failed to encode shared name prefix")?;
        offset
            .checked_add(length)
            .and_then(|size| size.checked_add(10))
            .context("Failed to fit encoded names within 4 GiB")?;
        Self::write(&mut self.bytes, prefix);
        Self::write(&mut self.bytes, length);
        self.bytes.extend_from_slice(suffix);
        Ok(offset)
    }

    // Read a standalone name while sorting the initial uncompressed entries.
    pub(super) fn raw(&self, offset: u32) -> &str {
        let mut position = offset as usize;
        let _prefix = Self::read(&self.bytes, &mut position);
        let length = Self::read(&self.bytes, &mut position);
        // Standalone records are written from complete UTF-8 strings with a zero prefix.
        unsafe { std::str::from_utf8_unchecked(&self.bytes[position..position + length]) }
    }

    // Reconstruct at most eight original names using stack storage for ordinary filenames.
    pub(super) fn name<'a>(&self, offset: u32, ordinal: u32, scratch: &'a mut Scratch) -> &'a str {
        let mut position = offset as usize;
        for _ in 0..=ordinal {
            let prefix = Self::read(&self.bytes, &mut position);
            let length = Self::read(&self.bytes, &mut position);
            scratch.update(prefix, &self.bytes[position..position + length]);
            position += length;
        }
        scratch.spelling()
    }

    // Release spare encoded capacity after the immutable directory index is published.
    pub(super) fn compact(&mut self) {
        self.bytes.shrink_to_fit();
    }

    // Encode compact unsigned lengths without alignment padding.
    fn write(bytes: &mut Vec<u8>, mut value: u32) {
        while value >= 128 {
            bytes.push(value as u8 | 128);
            value >>= 7;
        }
        bytes.push(value as u8);
    }

    // Decode a length written by this arena's internal append operation.
    fn read(bytes: &[u8], position: &mut usize) -> usize {
        let mut value = 0usize;
        let mut shift = 0;
        loop {
            let byte = bytes[*position];
            *position += 1;
            value |= usize::from(byte & 127) << shift;
            if byte < 128 {
                return value;
            }
            shift += 7;
        }
    }
}

impl Scratch {
    // Avoid heap allocation when decoding a bounded block of ordinary filenames.
    pub(super) fn new() -> Self {
        Self {
            small: [0; 1024],
            large: Vec::new(),
            length: 0,
        }
    }

    // Preserve the preceding prefix and overwrite only the next name's suffix.
    fn update(&mut self, prefix: usize, suffix: &[u8]) {
        self.length = prefix + suffix.len();
        if self.length <= self.small.len() && self.large.is_empty() {
            self.small[prefix..self.length].copy_from_slice(suffix);
        } else {
            if self.large.is_empty() {
                self.large.extend_from_slice(&self.small[..prefix]);
            }
            self.large.resize(self.length, 0);
            self.large[prefix..self.length].copy_from_slice(suffix);
        }
    }

    // Borrow the reconstructed original UTF-8 spelling for normalized comparison.
    fn spelling(&self) -> &str {
        let bytes = if self.large.is_empty() {
            &self.small[..self.length]
        } else {
            &self.large
        };
        // Prefix and suffix bytes reconstruct exactly the UTF-8 string supplied at insertion.
        unsafe { std::str::from_utf8_unchecked(bytes) }
    }
}
