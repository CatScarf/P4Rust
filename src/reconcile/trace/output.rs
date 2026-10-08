use super::ReconcileTrace;
use crate::{Result, ResultExt};
use std::io::Write;

pub(super) struct Output;

impl Output {
    // Serialize a completed trace atomically enough to reject any partial I/O failure.
    pub fn write(trace: &ReconcileTrace, path: &std::path::Path) -> Result<()> {
        crate::error::ensure!(
            !trace
                .inner
                .failed
                .load(std::sync::atomic::Ordering::Relaxed),
            "Failed to export reconcile trace: timing buffer failed or capture limit exceeded"
        );
        let threads = trace
            .inner
            .threads
            .lock()
            .map_err(|_| crate::Error::new("Failed to lock reconcile trace threads"))?;
        let file = std::fs::File::create(path).context("Failed to create reconcile timeline")?;
        let mut writer = std::io::BufWriter::new(file);
        write!(
            writer,
            "{{\"resolution_ns\":{},\"duration_ns\":{},\"threads\":[",
            trace.inner.resolution,
            trace.inner.started.elapsed().as_nanos()
        )
        .context("Failed to write reconcile trace header")?;
        for (index, local) in threads.iter().enumerate() {
            let local = local
                .lock()
                .map_err(|_| crate::Error::new("Failed to lock trace worker"))?;
            crate::error::ensure!(local.complete(), "Failed to export active trace worker");
            if index != 0 {
                write!(writer, ",").context("Failed to separate trace workers")?;
            }
            write!(
                writer,
                "{{\"name\":{},\"stages\":[",
                Self::string(&local.name)
            )
            .context("Failed to write trace worker")?;
            for (index, (name, stage)) in local.stages.iter().enumerate() {
                if index != 0 {
                    write!(writer, ",").context("Failed to separate trace stages")?;
                }
                Self::stage(&mut writer, name, stage).context("Failed to write worker stage")?;
            }
            write!(writer, "]}}").context("Failed to finish trace worker")?;
        }
        write!(writer, "]}}").context("Failed to finish reconcile trace")?;
        writer.flush().context("Failed to flush reconcile timeline")
    }

    // Escape dynamic thread names as valid JSON while keeping stage labels unchanged.
    fn string(value: &str) -> String {
        let mut result = String::from("\"");
        for character in value.chars() {
            match character {
                '"' => result.push_str("\\\""),
                '\\' => result.push_str("\\\\"),
                character if character.is_control() => {
                    result.push_str(&format!("\\u{:04x}", character as u32));
                }
                character => result.push(character),
            }
        }
        result.push('"');
        result
    }

    // Emit exact totals and sparse exclusive bins alongside bounded individual slow spans.
    fn stage(writer: &mut impl Write, name: &str, stage: &super::local::Stage) -> Result<()> {
        write!(writer, "{{\"name\":{},\"count\":{},\"first_ns\":{},\"last_ns\":{},\"inclusive_ns\":{},\"exclusive_ns\":{},\"bins\":[", Self::string(name), stage.count, stage.first, stage.last, stage.inclusive, stage.exclusive)
            .context("Failed to write trace stage totals")?;
        let mut first = true;
        for (index, nanos) in stage
            .bins
            .iter()
            .enumerate()
            .filter(|(_, nanos)| **nanos != 0)
        {
            if !first {
                write!(writer, ",").context("Failed to separate trace bins")?;
            }
            first = false;
            write!(writer, "[{index},{nanos}]").context("Failed to write trace activity bin")?;
        }
        write!(writer, "],\"slow_spans\":[").context("Failed to start slow trace spans")?;
        for (index, (start, end)) in stage.slow.iter().enumerate() {
            if index != 0 {
                write!(writer, ",").context("Failed to separate slow spans")?;
            }
            write!(writer, "[{start},{end}]").context("Failed to write slow trace span")?;
        }
        write!(writer, "]}}").context("Failed to finish trace stage")
    }
}
