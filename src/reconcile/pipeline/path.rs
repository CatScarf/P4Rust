use std::hash::{Hash, Hasher};

#[derive(Clone)]
pub(super) struct Path {
    original: Box<str>,
    hash: u64,
}

impl Path {
    // Keep one original filename while hashing its normalized comparison form.
    pub(super) fn new(path: &str) -> Self {
        let mut hash = 14695981039346656037u64;
        for character in Self::characters(path) {
            for byte in (character as u32).to_le_bytes() {
                hash = (hash ^ u64::from(byte)).wrapping_mul(1099511628211);
            }
        }
        Self {
            original: path.into(),
            hash,
        }
    }

    // Fold Windows case and platform separators without resolving filesystem links.
    fn characters(path: &str) -> impl Iterator<Item = char> + '_ {
        path.chars().flat_map(|character| {
            let character = if cfg!(windows) && character == '\\' {
                '/'
            } else {
                character
            };
            let mut lowered = cfg!(windows).then(|| character.to_lowercase());
            let mut original = Some(character);
            std::iter::from_fn(move || match lowered.as_mut() {
                Some(lowered) => lowered.next(),
                None => original.take(),
            })
        })
    }

    // Select a stable shard without retaining a second lowercased filename.
    pub(super) fn shard(&self, count: usize) -> usize {
        self.hash as usize % count
    }

    // Borrow the spelling needed by filesystem probes and SDK filename conversion.
    pub(super) fn original(&self) -> &str {
        &self.original
    }

    // Normalize short-lived B/C keys outside the million-entry path table.
    pub(super) fn normalized(path: &str) -> String {
        Self::characters(path).collect()
    }

    // Match a directory boundary without allocating another filename.
    pub(super) fn within(&self, directory: &str) -> bool {
        let mut path = Self::characters(&self.original);
        directory
            .chars()
            .all(|character| path.next() == Some(character))
            && matches!(path.next(), None | Some('/'))
    }
}

impl PartialEq for Path {
    // Compare normalized filenames independently of their retained spelling.
    fn eq(&self, other: &Self) -> bool {
        self.hash == other.hash
            && Self::characters(&self.original).eq(Self::characters(&other.original))
    }
}
impl Eq for Path {}
impl Hash for Path {
    // Hash the cached normalized value under each map's randomized hasher.
    fn hash<H: Hasher>(&self, state: &mut H) {
        self.hash.hash(state);
    }
}
