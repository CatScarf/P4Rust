"""Lossless SDK output and completion values."""

from __future__ import annotations

from dataclasses import dataclass
from enum import Enum, IntEnum
from typing import Optional, Union


class ProgressKind(IntEnum):
    UNKNOWN = 0
    SEND_FILE = 1
    RECEIVE_FILE = 2
    FILES = 3
    COMPUTATION = 4
    ITEMS = 5
    DIRECTORIES = 6
    DELETE_FILE = 7

    # Retain progress codes introduced by later SDK versions.
    @classmethod
    def _missing_(cls, value: object) -> Optional[ProgressKind]:
        if not isinstance(value, int):
            return None
        member = int.__new__(cls, value)
        member._name_ = f"UNKNOWN_{value}"
        member._value_ = value
        return member


class ProgressUnit(IntEnum):
    UNSPECIFIED = 0
    PERCENT = 1
    FILES = 2
    KILOBYTES = 3
    MEGABYTES = 4
    DELTAS = 5
    ITEMS = 6
    DIRECTORIES = 7

    # Retain unit codes introduced by later SDK versions.
    @classmethod
    def _missing_(cls, value: object) -> Optional[ProgressUnit]:
        if not isinstance(value, int):
            return None
        member = int.__new__(cls, value)
        member._name_ = f"UNKNOWN_{value}"
        member._value_ = value
        return member


class ProgressCallback(str, Enum):
    DESCRIPTION = "description"
    TOTAL = "total"
    UPDATE = "update"
    DONE = "done"


@dataclass(frozen=True)
class CommandStatus:
    exit_code: int
    error_count: Optional[int]
    success: bool


@dataclass(frozen=True)
class Record:
    raw_fields: tuple[tuple[bytes, bytes], ...]

    # Decode display strings while preserving original ordered fields separately.
    def fields(self) -> tuple[tuple[str, str], ...]:
        return tuple((key.decode("utf-8", "replace"), value.decode("utf-8", "replace"))
                     for key, value in self.raw_fields)

    # Find the first matching key without discarding duplicate fields.
    def get_raw(self, key: bytes) -> Optional[bytes]:
        return next((value for name, value in self.raw_fields if name == key), None)

    # Read the first matching field as UTF-8 display text.
    def get(self, key: str) -> Optional[str]:
        value = self.get_raw(key.encode("utf-8"))
        return None if value is None else value.decode("utf-8", "replace")


@dataclass(frozen=True)
class MessageId:
    code: int
    format: bytes


@dataclass(frozen=True)
class Message:
    severity: int
    generic: Optional[int]
    ids: tuple[MessageId, ...]
    parameters: tuple[tuple[bytes, bytes], ...]
    text: str
    raw: bytes
    serialized: bytes


@dataclass(frozen=True)
class Progress:
    id: int
    kind: ProgressKind
    units: ProgressUnit
    description: str
    raw_description: bytes
    total: int
    current: int
    callback: ProgressCallback
    failure: int


@dataclass(frozen=True)
class Text:
    raw: bytes


@dataclass(frozen=True)
class Info:
    level: int
    text: str
    raw: bytes


@dataclass(frozen=True)
class Binary:
    data: bytes


@dataclass(frozen=True)
class RecordEvent:
    record: Record


@dataclass(frozen=True)
class RecordPartial:
    record: Record


@dataclass(frozen=True)
class MessageEvent:
    message: Message


@dataclass(frozen=True)
class HandleError:
    message: Message


@dataclass(frozen=True)
class OutputError:
    text: str
    raw: bytes


@dataclass(frozen=True)
class NativeError:
    text: str
    raw: bytes


@dataclass(frozen=True)
class ProgressEvent:
    progress: Progress


@dataclass(frozen=True)
class Finished:
    pass


@dataclass(frozen=True)
class Completed:
    status: CommandStatus


Event = Union[Text, Info, Binary, RecordEvent, RecordPartial, MessageEvent,
              HandleError, OutputError, NativeError, ProgressEvent, Finished, Completed]


@dataclass(frozen=True)
class Output:
    text: str
    binary: bytes
    records: tuple[Record, ...]
    warnings: tuple[str, ...]
    raw_text: bytes
    raw_warnings: tuple[bytes, ...]
    info: tuple[tuple[int, bytes], ...]
    partial_records: tuple[Record, ...]
    messages: tuple[Message, ...]
    errors: tuple[bytes, ...]
    finished_callbacks: int
    status: Optional[CommandStatus]
