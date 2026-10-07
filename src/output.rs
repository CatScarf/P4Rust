/// Original bytes for text output and warning messages.
#[derive(Debug, Default)]
pub struct RawOutput {
    pub text: Vec<u8>,
    pub warnings: Vec<Vec<u8>>,
}

/// UTF-8 display strings, binary output, and lossless original text bytes.
#[derive(Debug, Default)]
pub struct Output {
    pub text: String,
    pub binary: Vec<u8>,
    pub records: Vec<crate::Record>,
    pub warnings: Vec<String>,
    pub raw: RawOutput,
    pub info: Vec<(u8, Vec<u8>)>,
    pub partial_records: Vec<crate::Record>,
    pub messages: Vec<crate::Message>,
    pub errors: Vec<Vec<u8>>,
    pub finished_callbacks: usize,
    pub status: Option<crate::CommandStatus>,
}

impl Output {
    // Decode complete text streams, replacing invalid UTF-8 only in display strings.
    pub(crate) fn decode(&mut self) {
        self.text = String::from_utf8_lossy(&self.raw.text).into_owned();
        self.warnings = self
            .raw
            .warnings
            .iter()
            .map(|warning| String::from_utf8_lossy(warning).into_owned())
            .collect();
    }
}
