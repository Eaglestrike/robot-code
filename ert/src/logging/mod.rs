//! Log is a stream of bincoded `LogMessages`

// In AOS, log's are persisted to an IPC queue for live-viewing, and occasionally replicated into an mmapped log file.
// Since we don't have IPC queues, instead we'll directly write to an mmap region (should be able to make that path lockess? no real need tho)
// and then duplicate over a unix domain socket for live viewing
// Since we serialize with bincode it's basically memcpy

// problem: unix sockets block if the reader aint there
// for that reason, maybe want to add a logging thread

pub mod mmap_file;

use serde::{Deserialize, Serialize};
use serde_big_array::big_array;
use std::mem::size_of;

/// Ord impl's are purely for use in data structures, they have no semantic meaning.
#[derive(
    Debug, Copy, Clone, Default, PartialEq, Eq, PartialOrd, Ord, Hash, Serialize, Deserialize,
)]
#[repr(transparent)]
pub struct ErtTypeId(u16);

impl ErtTypeId {
    pub fn of<T: ErtLoggable>() -> ErtTypeId {
        <T as ErtLoggable>::type_id()
    }

    /// Callers MUST ensure that the id matches the type and is unique
    pub unsafe fn from_raw(id: u16) -> Self {
        Self(id)
    }
}

pub trait ErtLoggable: Serialize + Deserialize<'static> {
    fn type_id() -> ErtTypeId;

    fn type_name() -> &'static str {
        std::any::type_name::<Self>()
    }
}

impl ErtLoggable for () {
    fn type_id() -> ErtTypeId {
        unsafe { ErtTypeId::from_raw(0) }
    }
}

const LOG_MSG_DATA_SIZE: usize = 512;

big_array! {
    BigArray;
    +(LOG_MSG_DATA_SIZE - size_of::<ErtTypeId>() - size_of::<u16>()),
}

#[derive(Copy, Clone, Serialize, Deserialize)]
enum LogMessageData {
    Message(#[serde(with = "BigArray")] [u8; LOG_MSG_DATA_SIZE]),
    Struct {
        typeid: ErtTypeId,
        msg_len: u16,
        #[serde(with = "BigArray")]
        buf: [u8; LOG_MSG_DATA_SIZE - size_of::<ErtTypeId>() - size_of::<u16>()],
    },
}

#[derive(Copy, Clone, Serialize, Deserialize)]
struct QueueLogMessage {
    time: crate::time::Duration,
    level: log::Level,
    thread_local_seq: u16,
    context_size: u16,
    #[serde(with = "BigArray")]
    context: [u8; 64],
    data_size: u16,
    message: LogMessageData,
}

impl Default for QueueLogMessage {
    fn default() -> Self {
        Self {
            time: crate::time::Duration::from_nanos(0),
            level: log::Level::Trace,
            thread_local_seq: 0,
            context_size: 0,
            context: [0; 64],
            data_size: 0,
            message: LogMessageData::Message([0; LOG_MSG_DATA_SIZE]),
        }
    }
}

impl crate::queue::QType for QueueLogMessage {}

// We must define our own macros and such for struct logging
// The easiest way to do this is copy and slightly modify stuff from log 0.4
#[macro_use]
mod log_crate_clone;

// expose a hybrid public interface
pub use log::{max_level, set_max_level};
pub use log_crate_clone::{block_on_log_flush, configure_log_crate_logger, logger};

// symbols for macros to use
// re-exported at the crate level
#[doc(hidden)]
pub mod rexp {
    pub use super::log_crate_clone::{
        __private_api_enabled, __private_api_log, __private_api_log_struct,
    };
    pub use log::{max_level, Level, Log, Metadata, Record, STATIC_MAX_LEVEL};
}

pub struct ErtLogger {
    q: crate::queue::QClient<QueueLogMessage>,
}

impl ErtLogger {
    fn new() -> Self {
        unimplemented!()
    }

    fn log_struct<T: ErtLoggable>(&self, strct: &T, record: &log::Record) {
        unimplemented!()
    }
}

impl log::Log for ErtLogger {
    fn enabled(&self, metadata: &log::Metadata) -> bool {
        unimplemented!()
    }
    fn log(&self, record: &log::Record) {
        unimplemented!()
    }
    fn flush(&self) {
        unimplemented!()
    }
}

#[cfg(test)]
mod test {
    use super::*;

    #[derive(Copy, Clone, Debug, Serialize, Deserialize)]
    pub struct Dummy(u32);

    impl ErtLoggable for Dummy {
        fn type_id() -> ErtTypeId {
            unsafe { ErtTypeId::from_raw(7312) }
        }
    }

    #[test]
    fn log_compile_test() {
        log!(log::Level::Error, "STRING {} {} {:?}", 5, 1, Dummy(1));
        error!("STRING {} {} {:?}", 5, 1, Dummy(1));
        warn!("STRING {} {} {:?}", 5, 1, Dummy(1));
        info!("STRING {} {} {:?}", 5, 1, Dummy(1));
        debug!("STRING {} {} {:?}", 5, 1, Dummy(1));
        trace!("STRING {} {} {:?}", 5, 1, Dummy(1));

        log!(target: "target", log::Level::Error, "STRING {} {} {:?}", 5, 1, Dummy(1));
        error!(target: "target", "STRING {} {} {:?}", 5, 1, Dummy(1));
        warn!(target: "target", "STRING {} {} {:?}", 5, 1, Dummy(1));
        info!(target: "target", "STRING {} {} {:?}", 5, 1, Dummy(1));
        debug!(target: "target", "STRING {} {} {:?}", 5, 1, Dummy(1));
        trace!(target: "target", "STRING {} {} {:?}", 5, 1, Dummy(1));

        let x = Dummy(6);
        let y = &x;
        log_struct!(log::Level::Error, &x);
        error_struct!(&Dummy(3));
        warn_struct!(&x);
        info_struct!(y);
        debug_struct!(&x);
        trace_struct!(&x);

        log_struct!(target: "target", log::Level::Error, &x);
        error_struct!(target: "target", &Dummy(3));
        warn_struct!(target: "target", &x);
        info_struct!(target: "target", y);
        debug_struct!(target: "target", &x);
        trace_struct!(target: "target", &x);
    }
}
