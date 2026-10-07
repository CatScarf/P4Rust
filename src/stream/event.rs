use crate::{CommandStatus, ProgressKind, ProgressUnit, Record};

/// One native message identifier and its untranslated format bytes.
#[derive(Debug)]
pub struct MessageId {
    pub code: i32,
    pub format: Vec<u8>,
}

/// An SDK Error object, including its original Marshall2 representation.
#[derive(Debug)]
pub struct Message {
    pub severity: i32,
    pub generic: Option<i32>,
    pub ids: Vec<MessageId>,
    pub parameters: Vec<(Vec<u8>, Vec<u8>)>,
    pub text: String,
    pub raw: Vec<u8>,
    pub serialized: Vec<u8>,
}

/// The native progress method that produced a notification.
#[derive(Debug)]
pub enum ProgressCallback {
    Description,
    Total,
    Update,
    Done,
}

/// Progress values retain the SDK's signed counters, units, and failure value.
#[derive(Debug)]
pub struct Progress {
    pub id: u64,
    pub kind: ProgressKind,
    pub units: ProgressUnit,
    pub description: String,
    pub raw_description: Vec<u8>,
    pub total: i64,
    pub current: i64,
    pub callback: ProgressCallback,
    pub failure: i32,
}

/// SDK output callbacks remain distinct; Completed follows native resource cleanup.
#[derive(Debug)]
pub enum Event {
    Text {
        raw: Vec<u8>,
    },
    Info {
        level: u8,
        text: String,
        raw: Vec<u8>,
    },
    Binary(Vec<u8>),
    Record(Record),
    RecordPartial(Record),
    Message(Message),
    HandleError(Message),
    OutputError {
        text: String,
        raw: Vec<u8>,
    },
    NativeError {
        text: String,
        raw: Vec<u8>,
    },
    Progress(Progress),
    Finished,
    Completed(CommandStatus),
}
