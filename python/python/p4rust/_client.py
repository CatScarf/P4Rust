"""Typed command builders and bounded streaming execution."""

from __future__ import annotations

from collections.abc import Iterable, Iterator
from dataclasses import dataclass, replace, field
from types import TracebackType
from typing import Optional

from . import _native
from ._events import CommandStatus, Event, Output


@dataclass(frozen=True)
class Config:
    port: str
    user: str
    client: str
    cwd: str = ""
    charset: str = ""
    password: str = field(default="", repr=False)
    ignore_file: str = ""
    tagged: bool = True

    # Resolve SDK environment, registry and P4CONFIG defaults without connecting.
    @classmethod
    def discover(cls, cwd: str = "") -> Config:
        try:
            port, user, client, directory, charset, ignore, password = _native._Client.discover(cwd)
            return cls(port, user, client, directory, charset, password=password, ignore_file=ignore)
        except _native.CommandError as error:
            raise P4Error._from_native(error) from error


class P4Error(RuntimeError):
    # Retain command failure context and native status when available.
    def __init__(self, message: str, status: Optional[CommandStatus], kind: str = "command") -> None:
        super().__init__(message)
        self.status: Optional[CommandStatus] = status
        self.kind: str = kind

    # Decode the private native exception's explicitly defined argument schema.
    @classmethod
    def _from_native(cls, error: _native.CommandError) -> P4Error:
        values: tuple[object, ...] = error.args
        message = str(values[0])
        exit_code = values[1]
        error_count = values[2]
        status = None
        if isinstance(exit_code, int):
            status = CommandStatus(exit_code, error_count if isinstance(error_count, int) else None, False)
        return cls(message, status, str(values[3]))


class CancellationToken:
    # Create an independent shared cancellation signal.
    def __init__(self) -> None:
        self._native = _native._CancellationToken()

    # Request cancellation without waiting for the executing command.
    def cancel(self) -> None:
        self._native.cancel()

    # Report whether cancellation has been requested.
    def is_cancelled(self) -> bool:
        return self._native.is_cancelled()


@dataclass(frozen=True)
class FastReconcile:
    _metadata: int = 8
    _digest: int = 4
    _moves: int = 4
    _capacity: int = 128
    _bytes: int = 16 * 1024 * 1024

    # Configure all local worker categories together.
    def workers(self, count: int) -> FastReconcile:
        return replace(self, _metadata=count, _digest=count, _moves=count)

    # Configure parallel local directory and metadata scanning.
    def metadata_workers(self, count: int) -> FastReconcile:
        return replace(self, _metadata=count)

    # Configure parallel canonical file comparisons.
    def digest_workers(self, count: int) -> FastReconcile:
        return replace(self, _digest=count)

    # Configure parallel SDK move comparisons.
    def move_workers(self, count: int) -> FastReconcile:
        return replace(self, _moves=count)

    # Bound retained reconcile requests.
    def queue_capacity(self, count: int) -> FastReconcile:
        return replace(self, _capacity=count)

    # Bound retained reconcile metadata bytes.
    def queue_bytes(self, count: int) -> FastReconcile:
        return replace(self, _bytes=count)


class Client:
    # Create a client with explicit connection settings.
    def __init__(self, config: Config) -> None:
        try:
            self._native = _native._Client(config.port, config.user, config.client, config.cwd, config.charset,
                                          config.password, config.ignore_file, config.tagged)
        except _native.CommandError as error:
            raise P4Error._from_native(error) from error

    # Configure a command without opening a network connection yet.
    def command(self, name: str) -> Command:
        return Command(self._native.command(name))

    # Check per-instance SDK ignore rules without connecting to a server.
    def is_ignored(self, path: str) -> bool:
        try:
            return self._native.is_ignored(path)
        except _native.CommandError as error:
            raise P4Error._from_native(error) from error

    # Create an independently closable connection for sequential commands.
    def session(self) -> Session:
        try:
            return Session(self._native.session())
        except _native.CommandError as error:
            raise P4Error._from_native(error) from error


class Session:
    # Retain the dedicated SDK connection owner.
    def __init__(self, native: _native._Session) -> None:
        self._native = native

    # Create a command that retains this session until stream cleanup completes.
    def command(self, name: str) -> Command:
        return Command(self._native.command(name))

    # Close all session clones and cancel an active command without blocking.
    def close(self) -> None:
        try:
            self._native.close()
        except _native.CommandError as error:
            raise P4Error._from_native(error) from error

    # Keep the persistent connection available within an explicit scope.
    def __enter__(self) -> Session:
        return self

    # Request native owner-thread cleanup on every scope exit.
    def __exit__(self, exc_type: Optional[type[BaseException]], exc_value: Optional[BaseException],
                 traceback: Optional[TracebackType]) -> None:
        self.close()


class Command:
    # Retain an immutable native command builder.
    def __init__(self, native: _native._Command) -> None:
        self._native = native

    # Append arguments in their supplied order.
    def args(self, args: Iterable[str]) -> Command:
        return Command(self._native.args(list(args)))

    # Supply noninteractive form input.
    def input(self, form: str) -> Command:
        return Command(self._native.input(form))

    # Set a command deadline in seconds.
    def timeout(self, seconds: float) -> Command:
        return Command(self._native.timeout(seconds))

    # Share a cancellation token across selected commands.
    def cancellation(self, token: CancellationToken) -> Command:
        return Command(self._native.cancellation(token._native))

    # Attach the parallel Rust reconcile engine.
    def reconcile_handler(self, handler: FastReconcile) -> Command:
        return Command(self._native.reconcile(handler._metadata, handler._digest, handler._moves,
                                              handler._capacity, handler._bytes))

    # Start background execution and return its event stream.
    def run(self) -> CommandStream:
        try:
            return CommandStream(self._native.run())
        except _native.CommandError as error:
            raise P4Error._from_native(error) from error


class CommandStream(Iterator[Event]):
    # Retain a Rust event stream whose lifetime owns the command.
    def __init__(self, native: _native._Stream) -> None:
        self._native = native

    # Iterate this stream without buffering the complete command output.
    def __iter__(self) -> CommandStream:
        return self

    # Receive one typed SDK event while native code releases the Python runtime.
    def __next__(self) -> Event:
        try:
            event = self._native.next()
        except _native.CommandError as error:
            raise P4Error._from_native(error) from error
        if event is None:
            raise StopIteration
        return event

    # Collect remaining events into the Rust output representation.
    def collect_output(self) -> Output:
        try:
            return self._native.collect_output()
        except _native.CommandError as error:
            raise P4Error._from_native(error) from error

    # Cancel this stream without cancelling a caller-shared token.
    def close(self) -> None:
        self._native.close()

    # Keep a scoped stream available until context exit.
    def __enter__(self) -> CommandStream:
        return self

    # Cancel and release the scoped stream on every exit path.
    def __exit__(self, exc_type: Optional[type[BaseException]], exc_value: Optional[BaseException],
                 traceback: Optional[TracebackType]) -> None:
        self.close()
