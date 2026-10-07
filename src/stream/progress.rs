/// The SDK progress type, retaining unrecognized native values.
#[derive(Clone, Copy, Debug, Eq, Hash, PartialEq)]
pub enum ProgressKind {
    /// CPT_UNKNOWN or an unrecognized SDK value.
    Unknown(i32),
    /// CPT_SENDFILE.
    SendFile,
    /// CPT_RECVFILE.
    ReceiveFile,
    /// CPT_FILES and its CPT_FILESTRANS alias.
    Files,
    /// CPT_COMPUTATION.
    Computation,
    /// CPT_ITEMS.
    Items,
    /// CPT_DIRS.
    Directories,
    /// CPT_DELFILE.
    DeleteFile,
}

impl ProgressKind {
    /// Return the original SDK progress type code.
    pub const fn code(self) -> i32 {
        match self {
            Self::Unknown(code) => code,
            Self::SendFile => 1,
            Self::ReceiveFile => 2,
            Self::Files => 3,
            Self::Computation => 4,
            Self::Items => 5,
            Self::Directories => 6,
            Self::DeleteFile => 7,
        }
    }
}

impl From<i32> for ProgressKind {
    /// Decode an SDK progress type without discarding unknown codes.
    fn from(code: i32) -> Self {
        match code {
            1 => Self::SendFile,
            2 => Self::ReceiveFile,
            3 => Self::Files,
            4 => Self::Computation,
            5 => Self::Items,
            6 => Self::Directories,
            7 => Self::DeleteFile,
            code => Self::Unknown(code),
        }
    }
}

/// The SDK counter unit, retaining unrecognized native values.
#[derive(Clone, Copy, Debug, Eq, Hash, PartialEq)]
pub enum ProgressUnit {
    /// CPU_UNSPECIFIED.
    Unspecified,
    /// CPU_PERCENT.
    Percent,
    /// CPU_FILES.
    Files,
    /// CPU_KBYTES.
    Kilobytes,
    /// CPU_MBYTES.
    Megabytes,
    /// CPU_DELTAS.
    Deltas,
    /// CPU_ITEMS.
    Items,
    /// CPU_DIRS.
    Directories,
    /// An unrecognized SDK value.
    Unknown(i32),
}

impl ProgressUnit {
    /// Return the original SDK counter unit code.
    pub const fn code(self) -> i32 {
        match self {
            Self::Unspecified => 0,
            Self::Percent => 1,
            Self::Files => 2,
            Self::Kilobytes => 3,
            Self::Megabytes => 4,
            Self::Deltas => 5,
            Self::Items => 6,
            Self::Directories => 7,
            Self::Unknown(code) => code,
        }
    }
}

impl From<i32> for ProgressUnit {
    /// Decode an SDK counter unit without discarding unknown codes.
    fn from(code: i32) -> Self {
        match code {
            0 => Self::Unspecified,
            1 => Self::Percent,
            2 => Self::Files,
            3 => Self::Kilobytes,
            4 => Self::Megabytes,
            5 => Self::Deltas,
            6 => Self::Items,
            7 => Self::Directories,
            code => Self::Unknown(code),
        }
    }
}
