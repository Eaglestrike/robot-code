//! Log is a stream of bincoded `LogMessages`

// In AOS, log's are persisted to an IPC queue for live-viewing, and occasionally replicated into an mmapped log file.
// Since we don't have IPC queues, instead we'll directly write to an mmap region (should be able to make that path lockess? no real need tho)
// and then duplicate over a unix domain socket for live viewing
// Since we serialize with bincode it's basically memcpy

// problem: unix sockets block if the reader aint there
// for that reason, maybe want to add a logging thread

mod mmap_file;

const LOG_MSG_DATA_SIZE: usize = 512;
use std::mem::size_of;

#[derive(Copy, Clone)]
enum LogMessageData {
    Message([u8; LOG_MSG_DATA_SIZE]),
    Struct {
        typeid: std::any::TypeId,
        msg_len: u16,
        buf: [u8; LOG_MSG_DATA_SIZE - size_of::<std::any::TypeId>() - size_of::<u16>()],
    },
}

#[derive(Copy, Clone)]
struct QueueLogMessage {
    time: crate::time::Duration,
    level: log::Level,
    thread_local_seq: u16,
    conext_size: u16,
    context: [u8; 100],
    data_size: u16,
    message: LogMessageData,
}

impl Default for QueueLogMessage {
    fn default() -> Self {
        Self {
            time: crate::time::Duration::from_nanos(0),
            level: log::Level::Trace,
            thread_local_seq: 0,
            conext_size: 0,
            context: [0; 100],
            data_size: 0,
            message: LogMessageData::Message([0; LOG_MSG_DATA_SIZE]),
        }
    }
}

impl crate::queue::QType for QueueLogMessage {}
