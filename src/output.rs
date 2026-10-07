/// Original bytes for text output, tagged fields, and warning messages.
#[derive(Debug, Default)]
pub struct RawOutput {
    pub text: Vec<u8>,
    pub records: Vec<Vec<(Vec<u8>, Vec<u8>)>>,
    pub warnings: Vec<Vec<u8>>,
}

/// UTF-8 display strings, binary output, and lossless original text bytes.
#[derive(Debug, Default)]
pub struct Output {
    pub text: String,
    pub binary: Vec<u8>,
    pub records: Vec<Vec<(String, String)>>,
    pub warnings: Vec<String>,
    pub raw: RawOutput,
}

impl Output {
    // Decode complete text streams, replacing invalid UTF-8 only in display strings.
    pub(crate) fn decode(&mut self) {
        self.text = String::from_utf8_lossy(&self.raw.text).into_owned();
        self.records = self
            .raw
            .records
            .iter()
            .map(|record| {
                record
                    .iter()
                    .map(|(key, value)| {
                        (
                            String::from_utf8_lossy(key).into_owned(),
                            String::from_utf8_lossy(value).into_owned(),
                        )
                    })
                    .collect()
            })
            .collect();
        self.warnings = self
            .raw
            .warnings
            .iter()
            .map(|warning| String::from_utf8_lossy(warning).into_owned())
            .collect();
    }
}
