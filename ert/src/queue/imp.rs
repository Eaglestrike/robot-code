//! A lock-free multi-producer/multi-consumer broadcast queue backed by a ring
//! buffer.
//!
//! Broadcast means that every reader will read every write.
//! Read the design section to see if the tradeoffs are right for your
//! application.
//!
//! # Design
//! For speed reasons, the queue will not ever allocate after initial creation.
//! Infinite size is emulated via a ring-buffer in a constant size allocation.
//! To avoid blocking, writers are free to overwrite data that some or all
//! readers have yet to consume. This means that readers are not guaranteed
//! to see all writes. As such, this queue is unfit for anything resembling a
//! task queue. Use `crossbeam-channel` or `bus` for that.
//!
//! Multi-consumer and broadcast means that only `Copy` types are supported.
//!
//! Since streaming readers need to know if writers have overwritten their
//! place in the buffer, each unit of data and index into the queue has an
//! associated write "epoch". This data, along with a write-in-progress tag,
//! is stored into an `AtomicUsize`. For this reason, allocations sizes are
//! rounded up to a power of two. After around `2^(ptr width) - size`
//! writes, information will overlap in the packed atomics, breaking the queue
//! in unpredictable ways. `size` refers to the allocation size, not the
//! user-requested size. Note that this happens before integer overflow.
//!
//! Writes are four step process. First, writers race for the next slot.
//! The winning writer then initiates the write to the buffer slot with
//! an atomic store, does the actual write, and then confirms it with another
//! atomic store. This ensures that readers will never see half-written data,
//! even if the data is larger than the size of atomic operations on the
//! platform.
//!
//! Readers will check whether the cell they are reading from has an epoch that
//! matches their index. They will also reject any cells currently undergoing a
//! write (steps 2-4 above). This means that streaming reads are not guaranteed
//! to get every message. Moreover, reads of the latest write have to deal with
//! possibly incomplete writes. Various choices are implemented as public
//! methods.
//!
//! The element type must implement `Default`, as several runtime checks are
//! eliminated by filling the internal buffer with default data. However, this
//! temporary data is never read and exists only to avoid `unsafe`.
//!
//! The only unsafe code is a `Sync` impl on the internal `Queue` type.

// use crate::sync::AosFutexNotifier;
use std::{
    cell::Cell as ICell,
    sync::{
        atomic::{AtomicUsize, Ordering::*},
        Arc,
    },
    usize,
};

// https://github.com/rust-lang/rfcs/blob/master/text/1443-extended-compare-and-swap.md

/// Write epochs: 0 represents defualt data, 1 is the first valid write
#[derive(Debug, Default)]
struct Cell<T: Copy> {
    data: ICell<T>,
    epoch: AtomicUsize,
}

// TODO: Test and possibly implement a solution for wrapping after 2^width messages

#[cfg(target_pointer_width = "16")]
const SENTINEL_MASK: usize = 1 << 15;

#[cfg(target_pointer_width = "32")]
const SENTINEL_MASK: usize = 1 << 31;

#[cfg(target_pointer_width = "64")]
const SENTINEL_MASK: usize = 1 << 63;

impl<T: Copy> Cell<T> {
    #[inline]
    pub fn write(&self, dat: T, new_epoch: usize, epoch_increment: usize) {
        // little CAS loop to ensure exclusive, complete, sequential writes
        // downside: newer writes can't "kick" off old writers
        // though, a sufficiently large queue will ensure this basically never happens as long
        // as each writer gets a chance to run

        let old_epoch = new_epoch - epoch_increment;
        loop {
            // Could possibly downgrade seqcst to acqrel
            match self.epoch.compare_exchange_weak(
                old_epoch,
                new_epoch | SENTINEL_MASK,
                SeqCst,
                Acquire,
            ) {
                Ok(_) => break,
                // if any race occurs, there's a chance for a deadlock here
                // ensure the epoch we are trying to advance from comes before us
                // if not, we will be stuck in a loop forever and have big problems
                Err(x) => debug_assert!(x & !SENTINEL_MASK <= old_epoch),
            }
        }
        self.data.set(dat);
        self.epoch.store(new_epoch, Release);
    }

    #[inline]
    pub fn read(&self) -> T {
        self.data.get()
    }
}

#[derive(Debug)]
struct Queue<T: Copy> {
    data: Box<[Cell<T>]>,
    write_ptr: AtomicUsize,
    idx_mask: usize,
    // notifier: AosFutexNotifier,
}

impl<T: Default + Copy> Queue<T> {
    pub fn new(size: usize) -> Self {
        assert!(size > 0);
        let size = round_up_to_power_of_two(size);
        let mut data = Vec::with_capacity(size);
        // the vec! macro requires a Clone bound
        for _ in 0..size {
            data.push(Default::default());
        }
        let r = Self {
            data: data.into_boxed_slice(),
            write_ptr: AtomicUsize::new(size), // write epoch 1, idx 0
            idx_mask: size - 1,
        };
        assert_eq!(r.idx_mask + 1, r.data.len());
        r
    }
}

impl<T: Copy> Queue<T> {
    #[inline]
    fn size(&self) -> usize {
        self.idx_mask + 1
    }

    #[inline]
    fn epoch(&self, idx: usize) -> usize {
        idx & (!self.idx_mask)
    }

    #[inline]
    fn modu(&self, idx: usize) -> usize {
        idx & self.idx_mask
    }

    #[inline]
    fn next_write_ptr(&self) -> usize {
        self.write_ptr.load(Acquire)
    }

    #[inline]
    pub fn push(&self, data: T) {
        // CAS loop until we get our turn to write
        let mut old = self.write_ptr.load(Relaxed);
        loop {
            let new = old + 1;
            match self
                .write_ptr
                .compare_exchange_weak(old, new, SeqCst, Relaxed) // Could maybe improve the success ordering
            {
                Ok(_) => break,
                Err(x) => old = x,
            }
        }
        // now we can write our data into old
        self.data[self.modu(old)].write(data, self.epoch(old), self.size());
    }

    /// Reads the last value that has a write initiated. Returns `None` if the write has not completed.
    /// Fails if nothing has been written to the queue.
    #[inline]
    pub fn try_read_latest(&self) -> Option<T> {
        let idx = self.write_ptr.load(Acquire) - 1;
        self.read(idx).ok()
    }

    /// Starts at the most recently initiated write, walking backwards until it finds a successful write.
    /// Will hang if nothing has been written to the queue yet.
    #[inline]
    pub fn read_latest(&self) -> T {
        let mut idx = self.write_ptr.load(Acquire) - 1;
        loop {
            match self.read(idx) {
                Ok(data) => {
                    return data;
                }
                Err(_epoch) => {
                    idx -= 1;
                }
            }
        }
    }

    /// Waits for the most recently initiated write to complete. Will not chase new writes after inovacation.
    #[inline]
    pub fn read_latest_blocking(&self) -> T {
        let idx = self.write_ptr.load(Acquire) - 1;
        loop {
            match self.read(idx) {
                Ok(data) => {
                    return data;
                }
                Err(_epoch) => (),
            }
        }
    }

    /// If the idx is still valid, returns Ok(T), else Err(epoch)
    #[inline]
    pub fn read(&self, idx: usize) -> Result<T, usize> {
        let cell = &self.data[self.modu(idx)];
        let epoch = cell.epoch.load(Acquire);
        if epoch != self.epoch(idx) {
            // if epochs don't match, it's over
            return Err(epoch);
        }
        let rr = cell.read();
        // ensure that no writes occurred while we were reading
        // a write would store a sentinel during the write if it
        // didn't complete, and a new epoch if it did.
        let epoch = cell.epoch.load(Acquire);
        if epoch != self.epoch(idx) {
            return Err(epoch);
        }
        Ok(rr)
    }
}

// The way Queue writes to Cell guarantees no data races
unsafe impl<T: Copy> Sync for Queue<T> {}

/// A streaming reader and writer holding an`Arc` to a queue buffer.
///
/// Use `Clone::clone` to create another reader/writer to the same queue.
/// The new client will start reading at original's read location at the time
/// of the clone.
///
/// # Example
///
/// ```
/// use std::sync::{
///     atomic::{AtomicBool, Ordering},
///     Arc,
/// };
/// use std::thread;
/// use ert::queue::QueueClient;
///
/// let w = QueueClient::new_queue(100);
/// assert_eq!(w.size(), 128);
///
/// let mut r = w.clone();
/// let messages = w.size() * 20;
///
/// let finished = Arc::new(AtomicBool::new(false));
/// let stop = finished.clone();
/// let thread = thread::spawn(move || {
///     let mut last = 0;
///     while !stop.load(Ordering::Relaxed) {
///         let result = r.latest();
///         assert!(result >= last);
///         last = result;
///     }
/// });
///
/// for data in 0..messages {
///     w.push(data);
/// }
/// finished.store(true, Ordering::Relaxed);
/// thread.join().unwrap();
/// ```
#[derive(Debug, Clone)]
pub struct QueueClient<T: Copy> {
    queue: Arc<Queue<T>>,
    to_read: usize,
}

impl<T: Default + Copy> QueueClient<T> {
    /// Create a new queue and return a client to it. Allocates a buffer of
    /// `size` rounded up to a power of two. The first element is the next
    /// to be read.
    pub fn new_queue(size: usize) -> Self {
        let q = Queue::new(size);
        let to_read = q.size();
        Self {
            queue: Arc::new(q),
            to_read,
        }
    }
}
impl<T: Copy> QueueClient<T> {
    /// Resets the read stream to a valid message with a margin for writes
    /// "from behind" before the next read. This usually should not be used;
    /// `next` uses it internally.
    #[inline]
    pub fn catch_up(&mut self, margin: usize) {
        self.to_read = self.queue.next_write_ptr() - self.queue.size() + margin;
    }

    /// Resets the read stream to the most recently written data. This guarantees
    /// at least one valid read provided the thread is not pre-empted.
    #[inline]
    pub fn reset(&mut self) {
        self.to_read = self.queue.next_write_ptr() - 1;
    }

    /// Advances the read pointer `n` elements, faster than calling
    /// `next` `n` times.
    #[inline]
    pub fn skip(&mut self, n: usize) {
        self.to_read += n;
    }

    /// The size of the internal buffer. History is readable this far back.
    #[inline]
    pub fn size(&self) -> usize {
        self.queue.size()
    }

    /// Push an element onto the end of the queue.
    #[inline]
    pub fn push(&self, data: T) {
        self.queue.push(data)
    }

    /// Get the next message if it is still in the history.
    /// If not, the read pointer is reset to the oldest valid data, skipping
    /// dropped messages.
    /// If our reads are racing with a write, more messages are dropped until
    /// we outpace the writer. This means that in the case of a race, this
    /// method may block.
    ///
    /// Returns `None` the next message has not been written or is currently being written.
    pub fn next(&mut self) -> Option<T> {
        // "backoff" our catch up in case writes are really fast
        let mut margin = 1;
        let size = self.queue.size();
        while margin < size {
            match self.queue.read(self.to_read) {
                Ok(data) => {
                    self.to_read += 1;
                    return Some(data);
                }
                Err(epoch) => {
                    // first handle the case of a write
                    let write_in_progress = epoch & SENTINEL_MASK > 0;
                    let epoch = epoch & !SENTINEL_MASK;
                    if epoch <= self.queue.epoch(self.to_read) || write_in_progress {
                        // either, we are trying to read ahead, or trying to read data that is currently being written
                        return None;
                    } else {
                        // this means data_epoch > read_epoch, so the writers have overtaken us
                        self.catch_up(margin);
                    }
                }
            }
            margin *= 2;
        }
        // if all of our backoff doesnt work, something is seriously wrong
        None
    }

    /// The same as `next()`, but blocks until there is a newly written
    /// message to read if we have read all of them.
    #[inline]
    pub fn next_blocking(&mut self) -> T {
        loop {
            if let Some(data) = self.next() {
                return data;
            }
        }
    }

    /// Reads the latest complete write to the queue.
    ///
    /// It is possible for a writer to be pre-empted before the write is
    /// completed. In this case, this method walks backwards from the write
    /// pointer until it finds the latest completed write. As such, this will
    /// HANG if no data has been written yet.
    #[inline]
    pub fn latest(&self) -> T {
        self.queue.read_latest()
    }

    /// Selects the most recently earned write (ie: a thread has earned the
    /// slot, but not necessarily completed the write) and then waits for
    /// its completion. Use of `latest` is recommended over this method.
    #[inline]
    pub fn latest_write(&self) -> T {
        self.queue.read_latest_blocking()
    }

    /// Selects the most recently earned write (ie: a thread has earned the
    /// slot, but not necessarily completed the write) and returns `None` if
    /// the write has not yet completed.
    #[inline]
    pub fn try_latest_write(&self) -> Option<T> {
        self.queue.try_read_latest()
    }

    /// Create a blocking iterator from this client. Read the documentation
    /// on the two `Iterator` implementations before use.
    pub fn into_iter(self) -> QueueReadIter<T> {
        QueueReadIter(self)
    }
}

/// Note that the iterator `next` is identical to the ordinary `next`. Because
/// `None` may be yielded, and then `Some` again later, some iterator methods
/// may not work normally. Use `into_iter()` for an iterator with more reliable
/// functionality.
impl<T: Copy> Iterator for QueueClient<T> {
    type Item = T;

    fn next(&mut self) -> Option<T> {
        QueueClient::next(self)
    }

    fn count(self) -> usize {
        unimplemented!()
    }

    fn nth(&mut self, n: usize) -> Option<T> {
        QueueClient::skip(self, n);
        self.next()
    }
}

pub struct QueueReadIter<T: Copy>(QueueClient<T>);
/// Here, `next` is identical to `QueueClient::next_blocking`. `None` is NEVER
/// yielded, so `for_each` and similar methods will never terminate.
impl<T: Copy> Iterator for QueueReadIter<T> {
    type Item = T;

    fn next(&mut self) -> Option<T> {
        Some(QueueClient::next_blocking(&mut self.0))
    }

    fn count(self) -> usize {
        unimplemented!()
    }

    fn nth(&mut self, n: usize) -> Option<T> {
        QueueClient::skip(&mut self.0, n);
        self.next()
    }
}

// only has code for these three widths
#[cfg(any(
    target_pointer_width = "64",
    target_pointer_width = "32",
    target_pointer_width = "16"
))]
fn round_up_to_power_of_two(mut u: usize) -> usize {
    u -= 1;
    u |= u >> 1;
    u |= u >> 2;
    u |= u >> 4;
    if cfg!(target_pointer_width = "16")
        || cfg!(target_pointer_width = "32")
        || cfg!(target_pointer_width = "64")
    {
        u |= u >> 8;
    }
    if cfg!(target_pointer_width = "32") || cfg!(target_pointer_width = "64") {
        u |= u >> 16;
    }
    if cfg!(target_pointer_width = "64") {
        u |= u >> 32;
    }
    u += 1;
    u
}

#[cfg(test)]
mod tests {
    use super::*;
    #[test]
    fn rounding() {
        assert_eq!(round_up_to_power_of_two(1), 1);
        assert_eq!(round_up_to_power_of_two(5), 8);
        assert_eq!(round_up_to_power_of_two(11), 16);
        assert_eq!(round_up_to_power_of_two(15), 16);
        assert_eq!(round_up_to_power_of_two(16), 16);
        assert_eq!(round_up_to_power_of_two(17), 32);
        assert_eq!(round_up_to_power_of_two(28), 32);
        assert_eq!(round_up_to_power_of_two(56), 64);
        assert_eq!(round_up_to_power_of_two(45), 64);
        assert_eq!(round_up_to_power_of_two(100), 128);
        assert_eq!(round_up_to_power_of_two(128), 128);
        assert_eq!(round_up_to_power_of_two(423), 512);
        assert_eq!(round_up_to_power_of_two(1000), 1024);
        assert_eq!(round_up_to_power_of_two(2000), 2048);
        assert_eq!(
            round_up_to_power_of_two(1_152_921_504_606_846_000),
            1_152_921_504_606_846_976
        );
        assert_eq!(
            round_up_to_power_of_two(9_223_372_036_854_000_000),
            9_223_372_036_854_775_808
        );
    }

    fn get_incrementor() -> impl Iterator<Item = u32> {
        std::iter::successors(Some(1), |n| Some(n + 1))
    }

    #[derive(Debug, Default, Clone)]
    struct Chomp(Option<u32>);

    impl Chomp {
        fn eat(&mut self, u: u32) {
            // dbg!((u, self.0));
            match self.0 {
                None => (),
                Some(x) if x == u - 1 => (),
                Some(_) => panic!(),
            }
            self.0 = Some(u);
        }
    }

    fn write(q: &QueueClient<u32>, data_iter: &mut impl Iterator<Item = u32>, n: u32) {
        for _ in 0..n {
            q.push(data_iter.next().unwrap());
        }
    }

    fn read(q: &mut QueueClient<u32>, ch: &mut Chomp, n: u32) {
        for _ in 0..n {
            ch.eat(q.next_blocking());
        }
    }

    #[test]
    fn single_threaded_single_client() {
        let q = &mut QueueClient::new_queue(100);
        let di = &mut get_incrementor();
        let ch = &mut Chomp::default();
        write(q, di, 20);
        read(q, ch, 20);
        write(q, di, 30);
        read(q, ch, 20);
        write(q, di, 40);
        read(q, ch, 50);
        assert_eq!(q.next(), None);
        write(q, di, 1);
        read(q, ch, 1);
    }

    #[test]
    fn single_threaded_multi_client() {
        let q1 = &mut QueueClient::new_queue(100);
        let q2 = &mut Clone::clone(q1);
        let di = &mut get_incrementor();
        let c1 = &mut Chomp::default();
        let c2 = &mut Chomp::default();

        // indivudal
        write(q1, di, 20);
        read(q1, c1, 20);
        read(q2, c2, 20);

        // exchanging
        write(q1, di, 20);
        read(q2, c2, 20);
        write(q2, di, 20);
        read(q1, c1, 20);
        read(q2, c2, 20);
        read(q1, c1, 20);

        assert_eq!(q1.next(), None);
        write(q1, di, 1);
        read(q1, c1, 1);
        read(q2, c2, 1);

        assert_eq!(q2.next(), None);
        write(q2, di, 1);
        read(q2, c2, 1);
        read(q1, c1, 1);

        assert_eq!(q2.next(), None);
        assert_eq!(q1.next(), None);

        write(q1, di, 500);
        // 562 written so far
        // default read margin is 1
        // read 128 back, meaning
        assert_eq!(q2.next(), Some(62 + 500 - 127 + 1));
        assert_eq!(q1.latest(), 62 + 500);
    }

    use std::sync::{
        atomic::{AtomicBool, Ordering},
        Arc,
    };
    use std::thread;
    use std::time::Duration;
    #[test]
    fn multithreaded_single_producer() {
        let mut q1 = QueueClient::<u32>::new_queue(4000);
        let mut q2 = q1.clone();
        let q3 = q1.clone();
        let q4 = q1.clone();

        let messages = q1.size() * 20;
        let write_delay = 25; // nanos

        // simple reader
        let t1 = thread::spawn(move || {
            let mut c1 = Chomp::default();
            for _ in 0..messages {
                c1.eat(q1.next_blocking());
            }
        });
        // reader joins late and will "catch up"
        let t2 = thread::spawn(move || {
            let mut c2 = Chomp::default();
            let skip_first = (q2.size() as u64) * 5;
            thread::sleep(Duration::from_nanos(write_delay * skip_first));
            let skip_first = skip_first * 3 / 2; //1.5x margin to ensure we don't just deadlock
            for _ in 0..(messages - skip_first as usize) {
                c2.eat(q2.next_blocking());
            }
        });
        // reader ensures latest is at least monotonic
        let end_thread = Arc::new(AtomicBool::new(false));
        let stop = end_thread.clone();
        let t3 = thread::spawn(move || {
            let mut last = 0;
            while !stop.load(Ordering::Relaxed) {
                let r = q3.latest();
                assert!(r >= last, "{} >= {}", r, last);
                last = r;
            }
        });

        for data in 0..messages {
            q4.push(data as u32);
            thread::sleep(Duration::from_nanos(write_delay));
        }

        end_thread.store(true, Ordering::Relaxed);
        t1.join().unwrap();
        t2.join().unwrap();
        t3.join().unwrap();
    }
}
