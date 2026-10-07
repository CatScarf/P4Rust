use crate::{Event, Result, ResultExt, control::Control, error::ensure};
use std::{collections::VecDeque, sync, time::Duration};

struct State {
    events: VecDeque<Result<Event>>,
    next_ticket: u64,
    closed: bool,
}

pub(crate) struct Queue {
    state: sync::Mutex<State>,
    ready: sync::Condvar,
}

impl Queue {
    // Allocate a fixed-capacity queue shared by SDK callback producers and the stream.
    pub(crate) fn new() -> Self {
        Self {
            state: sync::Mutex::new(State {
                events: VecDeque::with_capacity(64),
                next_ticket: 0,
                closed: false,
            }),
            ready: sync::Condvar::new(),
        }
    }

    // Wait outside callback state locks and publish tickets in their assigned order.
    pub(crate) fn push(&self, ticket: u64, event: Result<Event>, control: &Control) -> Result<()> {
        let mut state = self
            .state
            .lock()
            .map_err(|_| crate::Error::new("Failed to lock event queue: poisoned mutex"))?;
        loop {
            if let Some(error) = control.error() {
                return Err(error).context("Failed to deliver P4 event");
            }
            ensure!(
                !state.closed,
                "Failed to deliver P4 event: stream is closed"
            );
            if ticket == state.next_ticket && state.events.len() < 64 {
                state.next_ticket = state
                    .next_ticket
                    .checked_add(1)
                    .context("Failed to advance delivery ticket")?;
                state.events.push_back(event);
                self.ready.notify_all();
                return Ok(());
            }
            state = self
                .ready
                .wait_timeout(state, Duration::from_millis(10))
                .map_err(|_| crate::Error::new("Failed to wait for event queue: poisoned mutex"))?
                .0;
        }
    }

    // Wake blocked producers immediately after removing an event from the bounded queue.
    pub(crate) fn pop(&self, control: &Control) -> Result<Option<Result<Event>>> {
        let mut state = self
            .state
            .lock()
            .map_err(|_| crate::Error::new("Failed to lock event receiver: poisoned mutex"))?;
        loop {
            if let Some(error) = control.error() {
                return Err(error).context("Failed to wait for P4 event");
            }
            if let Some(event) = state.events.pop_front() {
                self.ready.notify_all();
                return Ok(Some(event));
            }
            if state.closed {
                return Ok(None);
            }
            state = self
                .ready
                .wait_timeout(state, Duration::from_millis(10))
                .map_err(|_| {
                    crate::Error::new("Failed to wait for event receiver: poisoned mutex")
                })?
                .0;
        }
    }

    // Wake all waiters when production ends or a dropped stream discards its pending events.
    pub(crate) fn close(&self, discard: bool) -> Result<()> {
        let mut state = self
            .state
            .lock()
            .map_err(|_| crate::Error::new("Failed to close event queue: poisoned mutex"))?;
        state.closed = true;
        if discard {
            state.events.clear();
        }
        self.ready.notify_all();
        Ok(())
    }
}
