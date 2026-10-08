use super::path::Path;
use crate::{ReconcileKind, ReconcileReply, ReconcileRequest, Record, Result, ResultExt};
use std::collections::{HashMap, VecDeque};

#[derive(Clone, Hash, Eq, PartialEq)]
struct DigestKey {
    digest: Vec<u8>,
    file_type: Vec<u8>,
    algorithm: Vec<u8>,
}
#[derive(Default)]
struct Candidates {
    add: VecDeque<String>,
    delete: VecDeque<String>,
}
#[derive(Default)]
struct Operation {
    action: String,
    counterpart: Option<String>,
    record: Option<Record>,
}
#[derive(Default)]
pub(super) struct Results {
    digests: HashMap<DigestKey, Candidates>,
    operations: HashMap<String, Operation>,
    paired: usize,
}

impl Results {
    // Classify same-path edits separately from complementary add/delete candidates.
    pub(super) fn completed(
        &mut self,
        request: &ReconcileRequest,
        reply: &ReconcileReply,
    ) -> Result<()> {
        let (action, fields) = match request.kind {
            ReconcileKind::TrackedFile => match reply.result.get_raw(b"status") {
                Some(b"exists") => ("edit", &request.metadata),
                Some(b"missing") => ("delete", &request.metadata),
                _ => return Ok(()),
            },
            ReconcileKind::UntrackedFile => ("add", &reply.result),
            _ => return Ok(()),
        };
        let path = request
            .path_bytes()
            .context("Failed to read candidate path")?;
        if action == "edit" && request.metadata.get_raw(b"digest").is_none() {
            return Ok(());
        }
        let path =
            Path::normalized(std::str::from_utf8(path).context("Failed to decode candidate path")?);
        self.operations.entry(path.clone()).or_default().action = action.into();
        if action == "edit" {
            return Ok(());
        }
        if let Some(digest) = fields
            .get_raw(b"candidateDigest")
            .or_else(|| fields.get_raw(b"digest"))
            .filter(|digest| !digest.is_empty())
        {
            let key = DigestKey {
                digest: digest.to_ascii_lowercase(),
                file_type: reply
                    .result
                    .get_raw(b"canonicalType")
                    .unwrap_or(b"unknown")
                    .to_vec(),
                algorithm: fields
                    .get_raw(b"digestType")
                    .unwrap_or(b"MD5")
                    .to_ascii_lowercase(),
            };
            self.candidate(key, path, action == "add")
                .context("Failed to pair digest candidate")?;
        }
        Ok(())
    }

    // Pair duplicates one-to-one and retain tentative move relations in C until SDK confirmation.
    fn candidate(&mut self, key: DigestKey, path: String, addition: bool) -> Result<()> {
        let candidates = self.digests.entry(key).or_default();
        if addition {
            candidates.add.push_back(path);
        } else {
            candidates.delete.push_back(path);
        }
        if candidates.add.is_empty() || candidates.delete.is_empty() {
            return Ok(());
        }
        let add = candidates
            .add
            .pop_front()
            .context("Failed to take addition candidate")?;
        let delete = candidates
            .delete
            .pop_front()
            .context("Failed to take deletion candidate")?;
        let added = self
            .operations
            .get_mut(&add)
            .context("Failed to find move addition in C")?;
        added.action = "move/add".into();
        added.counterpart = Some(delete.clone());
        let deleted = self
            .operations
            .get_mut(&delete)
            .context("Failed to find move deletion in C")?;
        deleted.action = "move/delete".into();
        deleted.counterpart = Some(add);
        self.paired += 1;
        Ok(())
    }

    // Replace tentative operations with complete authoritative records, including similarity moves.
    pub(super) fn output(&mut self, record: Record) {
        let path = record.get("clientFile").or_else(|| record.get("depotFile"));
        if let (Some(path), Some(action)) = (path, record.get("action")) {
            let operation = self.operations.entry(Path::normalized(&path)).or_default();
            operation.action = action.into_owned();
            operation.counterpart = None;
            operation.record = Some(record);
        }
    }

    // Drain unmatched B entries after SDK add/delete decisions have populated C.
    pub(super) fn finish(&mut self) {
        self.digests = HashMap::new();
        self.operations
            .retain(|_, operation| operation.record.is_some());
    }

    // Report pending candidates, proposed exact pairs, and authoritative result counts.
    pub(super) fn counts(&self) -> (usize, usize, usize) {
        (
            self.digests
                .values()
                .map(|candidates| candidates.add.len() + candidates.delete.len())
                .sum(),
            self.paired,
            self.operations
                .values()
                .filter(|operation| operation.record.is_some())
                .count(),
        )
    }
}
