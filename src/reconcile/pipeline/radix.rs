use super::path::Path;
use crate::{Result, ResultExt};

const NONE: u32 = u32::MAX;

struct Node<T> {
    start: u32,
    end: u32,
    child: u32,
    sibling: u32,
    value: Option<T>,
}
pub(super) struct Radix<T> {
    bytes: Vec<u8>,
    nodes: Vec<Node<T>>,
}

impl<T> Radix<T> {
    // Release enumeration's spare edge and node capacity without changing directory identifiers.
    pub(super) fn compact(&mut self) {
        // Avoid doubling compact buffers when later server comparisons discover a few directories.
        self.bytes.shrink_to(self.bytes.len() + 1024);
        self.nodes.shrink_to(self.nodes.len() + 64);
    }

    // Keep compressed edges and directory slots in two contiguous allocations per shard.
    pub(super) fn new() -> Self {
        Self {
            bytes: Vec::new(),
            nodes: vec![Node {
                start: 0,
                end: 0,
                child: NONE,
                sibling: NONE,
                value: None,
            }],
        }
    }

    // Reuse an existing normalized path without allocating a query string.
    pub(super) fn get_or_insert_with(
        &mut self,
        path: &str,
        create: impl FnOnce() -> T,
    ) -> Result<&T> {
        let node = if let Some(index) = self.find(path) {
            index
        } else {
            let key: Vec<_> = Path::bytes(path).collect();
            self.insert(&key)
                .context("Failed to insert compressed directory key")?
        };
        Ok(self.nodes[node as usize].value.get_or_insert_with(create))
    }

    // Match normalized streamed bytes against compact edges and bounded byte fanout.
    fn find(&self, path: &str) -> Option<u32> {
        let mut bytes = Path::bytes(path);
        let mut parent = 0;
        while let Some(first) = bytes.next() {
            let (child, _) = self.child(parent, first)?;
            let node = &self.nodes[child as usize];
            for expected in &self.bytes[node.start as usize + 1..node.end as usize] {
                if bytes.next() != Some(*expected) {
                    return None;
                }
            }
            parent = child;
        }
        Some(parent)
    }

    // Find one edge with the requested first byte without allocating a child map.
    fn child(&self, parent: u32, first: u8) -> Option<(u32, u32)> {
        let mut index = self.nodes[parent as usize].child;
        let mut previous = NONE;
        while index != NONE {
            let node = &self.nodes[index as usize];
            if self.bytes[node.start as usize] == first {
                return Some((index, previous));
            }
            previous = index;
            index = node.sibling;
        }
        None
    }

    // Split only a divergent edge and reuse its existing prefix bytes in place.
    fn insert(&mut self, mut key: &[u8]) -> Result<u32> {
        let mut parent = 0;
        while let Some(first) = key.first() {
            let Some((child, previous)) = self.child(parent, *first) else {
                return self
                    .leaf(parent, key)
                    .context("Failed to append directory edge");
            };
            let node = &self.nodes[child as usize];
            let edge = &self.bytes[node.start as usize..node.end as usize];
            let common = edge
                .iter()
                .zip(key)
                .take_while(|(left, right)| left == right)
                .count();
            if common < edge.len() {
                let branch = self
                    .split(parent, child, previous, common)
                    .context("Failed to split directory prefix")?;
                return if common == key.len() {
                    Ok(branch)
                } else {
                    self.leaf(branch, &key[common..])
                        .context("Failed to append divergent directory suffix")
                };
            }
            key = &key[common..];
            parent = child;
        }
        Ok(parent)
    }

    // Append only the unmatched suffix and link its integer node under the shared prefix.
    fn leaf(&mut self, parent: u32, key: &[u8]) -> Result<u32> {
        let start =
            u32::try_from(self.bytes.len()).context("Failed to encode directory edge offset")?;
        let length = u32::try_from(key.len()).context("Failed to encode directory edge length")?;
        let end = start
            .checked_add(length)
            .context("Failed to fit directory edges within 4 GiB")?;
        let sibling = self.nodes[parent as usize].child;
        let index = self
            .node(Node {
                start,
                end,
                child: NONE,
                sibling,
                value: None,
            })
            .context("Failed to allocate directory leaf")?;
        self.bytes.extend_from_slice(key);
        self.nodes[parent as usize].child = index;
        Ok(index)
    }

    // Preserve sibling ownership while inserting a branch into the original edge position.
    fn split(&mut self, parent: u32, child: u32, previous: u32, common: usize) -> Result<u32> {
        let node = &self.nodes[child as usize];
        let end = node.start
            + u32::try_from(common).context("Failed to encode directory prefix length")?;
        let index = self
            .node(Node {
                start: node.start,
                end,
                child,
                sibling: node.sibling,
                value: None,
            })
            .context("Failed to allocate directory branch")?;
        self.nodes[child as usize].start = end;
        self.nodes[child as usize].sibling = NONE;
        if previous == NONE {
            self.nodes[parent as usize].child = index;
        } else {
            self.nodes[previous as usize].sibling = index;
        }
        Ok(index)
    }

    // Reject node identifier overflow before exposing a new directory slot.
    fn node(&mut self, node: Node<T>) -> Result<u32> {
        let index = u32::try_from(self.nodes.len())
            .context("Failed to encode directory node identifier")?;
        if index == NONE {
            return Err(crate::Error::new(
                "Failed to fit directory nodes within 32 bits",
            ));
        }
        self.nodes.push(node);
        Ok(index)
    }
}
