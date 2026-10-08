use std::hash::{Hash, Hasher};

#[repr(transparent)]
pub(super) struct PathRef(str);

#[derive(Clone)]
pub(super) struct Path {
    original: Box<str>,
    hash: u64,
}

impl Path {
    // Keep one original filename while hashing its normalized comparison form.
    pub(super) fn new(path: &str) -> Self {
        Self {
            original: path.into(),
            hash: Self::hash_of(path),
        }
    }

    // Compute the comparison hash without allocating an owned lookup key.
    pub(super) fn hash_of(path: &str) -> u64 {
        let mut hash = 14695981039346656037u64;
        for character in Self::characters(path) {
            for byte in (character as u32).to_le_bytes() {
                hash = (hash ^ u64::from(byte)).wrapping_mul(1099511628211);
            }
        }
        hash
    }

    // Verify equal hashes against normalized names instead of trusting collisions.
    pub(super) fn equivalent(left: &str, right: &str) -> bool {
        Self::characters(left).eq(Self::characters(right))
    }

    // Fold Windows case and platform separators without resolving filesystem links.
    pub(super) fn characters(path: &str) -> impl Iterator<Item = char> + '_ {
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

    // Stream normalized UTF-8 bytes for allocation-free compressed registry queries.
    pub(super) fn bytes(path: &str) -> impl Iterator<Item = u8> + '_ {
        Self::characters(path).flat_map(|character| {
            let mut bytes = [0; 4];
            let length = character.encode_utf8(&mut bytes).len();
            bytes.into_iter().take(length)
        })
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
        self.hash == other.hash && Self::equivalent(&self.original, &other.original)
    }
}

impl std::borrow::Borrow<PathRef> for Path {
    // Expose the same normalized equality and hash through a borrowed string view.
    fn borrow(&self) -> &PathRef {
        PathRef::new(&self.original)
    }
}

impl PathRef {
    // Borrow a transparent string wrapper without allocating or changing its lifetime.
    pub(super) fn new(path: &str) -> &Self {
        // PathRef has exactly str's layout and this shared view cannot mutate its bytes.
        unsafe { &*(std::ptr::from_ref(path) as *const Self) }
    }
}
impl PartialEq for PathRef {
    // Apply owned-key comparison rules to allocation-free lookups.
    fn eq(&self, other: &Self) -> bool {
        Path::equivalent(&self.0, &other.0)
    }
}
impl Eq for PathRef {}
impl Hash for PathRef {
    // Match the owned key's cached-hash encoding under the map's randomized hasher.
    fn hash<H: Hasher>(&self, state: &mut H) {
        Path::hash_of(&self.0).hash(state);
    }
}
impl Eq for Path {}
impl Hash for Path {
    // Hash the cached normalized value under each map's randomized hasher.
    fn hash<H: Hasher>(&self, state: &mut H) {
        self.hash.hash(state);
    }
}
