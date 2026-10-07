use super::{Capture, State};
use crate::{Event, Message, Progress, ProgressCallback, Record};
use crate::{Result, ResultExt, error::ensure, ffi};

impl Capture {
    // Convert one SDK callback into an owned event without accessing the delivery queue.
    pub(super) unsafe fn prepare(
        kind: u32,
        data: &[u8],
        value: &[u8],
        state: &mut State,
    ) -> Result<Option<Event>> {
        ensure!(data.len() <= 1_048_576, "Failed to bound SDK callback size");
        let event = match kind {
            ffi::TEXT => Event::Text { raw: data.to_vec() },
            ffi::BINARY => Event::Binary(data.to_vec()),
            ffi::RECORD | ffi::RECORD_PARTIAL => {
                // The descriptor array borrows dictionary bytes until this callback returns.
                let record =
                    unsafe { Record::copy(data) }.context("Failed to copy native tagged record")?;
                if kind == ffi::RECORD {
                    Event::Record(record)
                } else {
                    Event::RecordPartial(record)
                }
            }
            ffi::PROGRESS => Event::Progress(
                Self::progress(data, value).context("Failed to decode SDK progress")?,
            ),
            ffi::STATUS => {
                state.error_count = Some(i32::from_ne_bytes(
                    data.try_into()
                        .context("Failed to decode SDK error count")?,
                ));
                return Ok(None);
            }
            _ => Self::other(kind, data, value, state).context("Failed to decode SDK callback")?,
        };
        Ok(Some(event))
    }

    // Preserve native messages and bridge failures without redirecting SDK callback types.
    fn other(kind: u32, data: &[u8], value: &[u8], state: &mut State) -> Result<Event> {
        Ok(match kind {
            ffi::INFO => {
                ensure!(value.len() == 1, "Failed to read native information level");
                Event::Info {
                    level: value[0],
                    text: String::from_utf8_lossy(data).into_owned(),
                    raw: data.to_vec(),
                }
            }
            ffi::MESSAGE | ffi::HANDLE_ERROR => {
                let message =
                    Message::decode(data, value).context("Failed to decode SDK Error object")?;
                if kind == ffi::MESSAGE {
                    Event::Message(message)
                } else {
                    Event::HandleError(message)
                }
            }
            ffi::OUTPUT_ERROR => Event::OutputError {
                text: String::from_utf8_lossy(data).into_owned(),
                raw: data.to_vec(),
            },
            ffi::ERROR => {
                let text = String::from_utf8_lossy(data).into_owned();
                state.failure = Some(crate::Error::new(format!(
                    "Failed to execute native command: {text}"
                )));
                Event::NativeError {
                    text,
                    raw: data.to_vec(),
                }
            }
            ffi::FINISHED => Event::Finished,
            _ => {
                return Err(crate::Error::new(format!(
                    "Failed to collect unknown native event: {kind}"
                )));
            }
        })
    }

    // Decode exact progress method metadata into an owned callback snapshot.
    fn progress(description: &[u8], metadata: &[u8]) -> Result<Progress> {
        ensure!(
            metadata.len() == 56 && description.len() <= 16_384,
            "Failed to validate SDK progress frame"
        );
        let mut values = [0_i64; 7];
        for (index, bytes) in metadata.as_chunks::<8>().0.iter().enumerate() {
            values[index] = i64::from_ne_bytes(*bytes);
        }
        Ok(Progress {
            id: values[0] as u64,
            kind: (values[1] as i32).into(),
            units: (values[2] as i32).into(),
            description: String::from_utf8_lossy(description).into_owned(),
            raw_description: description.to_vec(),
            total: values[3],
            current: values[4],
            callback: match values[5] {
                1 => ProgressCallback::Description,
                2 => ProgressCallback::Total,
                3 => ProgressCallback::Update,
                4 => ProgressCallback::Done,
                _ => {
                    return Err(crate::Error::new(
                        "Failed to identify native progress callback",
                    ));
                }
            },
            failure: values[6] as i32,
        })
    }
}
