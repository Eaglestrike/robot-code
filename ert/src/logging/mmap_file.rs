//! Rusty interface to an mmapped file

use std::fs::File;
use std::io;
use std::os::unix::io::*;

use crate::{die_with_errno, unlikely};
use libc::*;

const PAGE_SIZE: usize = 16384;

struct Read {}
pub struct Write {}
pub trait Mode: private::Sealed {
    fn for_writing() -> bool;
}
mod private {
    pub trait Sealed {}
    impl Sealed for super::Read {}
    impl Sealed for super::Write {}
}
impl Mode for Read {
    fn for_writing() -> bool {
        false
    }
}
impl Mode for Write {
    fn for_writing() -> bool {
        true
    }
}

pub struct MMapedFile<M: Mode> {
    /// Underlying file we are mapping
    file: File,
    /// len, so readers know when to stop
    len: u64,
    /// offset of our mapping into the file, multiple of PAGE_SIZE
    /// This represents the offset of the next page, not the current,
    /// it starts at 0 before the first mmap
    offset: usize,
    /// our relative position in the mapping
    position: usize,
    /// the virtual address of our mmapped pages,
    /// used for bookkeeping with mmap et al
    current: *mut c_void,

    phantom: std::marker::PhantomData<M>,
}

impl MMapedFile<Write> {
    pub fn writer(file: File) -> Self {
        let flags = unsafe { fcntl(file.as_raw_fd() as c_int, F_GETFL) };
        assert!(flags & (O_RDWR) != 0);
        Self::new(file)
    }
}

impl MMapedFile<Read> {
    pub fn reader(file: File) -> Self {
        let flags = unsafe { fcntl(file.as_raw_fd() as c_int, F_GETFL) };
        assert!(flags & (O_RDWR) != 0);
        Self::new(file)
    }
}

impl<M: Mode> MMapedFile<M> {
    fn new(file: File) -> Self {
        let page_size = unsafe { sysconf(_SC_PAGESIZE) };
        assert!(PAGE_SIZE >= page_size as usize);
        assert!(PAGE_SIZE % page_size as usize == 0);
        let mut r = Self {
            file,
            offset: 0,
            position: 0,
            current: std::ptr::null_mut(),
            phantom: std::marker::PhantomData,
            len: 0,
        };
        r.map_next_page();
        r
    }

    fn for_writing(&self) -> bool {
        M::for_writing()
    }

    fn map_next_page(&mut self) {
        if self.for_writing() {
            let ret = unsafe {
                ftruncate(
                    self.file.as_raw_fd() as c_int,
                    (self.offset + PAGE_SIZE) as off_t,
                )
            };
            if unlikely!(ret == -1) {
                die_with_errno!(
                    "ftruncate, fd: {}, page size: {}",
                    self.file.as_raw_fd(),
                    PAGE_SIZE
                );
            }
        } else {
            self.len = self.file.metadata().unwrap().len();
        }

        self.current = unsafe {
            mmap(
                std::ptr::null_mut(),
                PAGE_SIZE,
                PROT_READ | (if self.for_writing() { PROT_WRITE } else { 0 }),
                MAP_SHARED,
                self.file.as_raw_fd() as c_int,
                self.offset as off_t,
            )
        };
        if unlikely!(self.current == MAP_FAILED) {
            die_with_errno!("mmap failed");
        }
        let ret = unsafe { madvise(self.current, PAGE_SIZE, MADV_SEQUENTIAL | MADV_WILLNEED) };
        if unlikely!(ret == -1) {
            // TODO log warning...
        }
        self.offset += PAGE_SIZE;
    }

    pub fn sync_async(&self) {
        unsafe { msync(self.current, PAGE_SIZE, MS_ASYNC | MS_INVALIDATE) };
    }

    pub fn sync_blocking(&self) {
        unsafe { msync(self.current, PAGE_SIZE, MS_INVALIDATE) };
    }

    fn map_slice<'a>(&'a self) -> &'a [u8] {
        unsafe { std::slice::from_raw_parts(self.current as *const u8, PAGE_SIZE) }
    }

    fn map_slice_mut<'a>(&'a mut self) -> &'a mut [u8] {
        unsafe { std::slice::from_raw_parts_mut(self.current as *mut u8, PAGE_SIZE) }
    }

    fn unmap(pages: *mut c_void) {
        if unlikely!(unsafe { munmap(pages, PAGE_SIZE) } == -1) {
            die_with_errno!("munmap");
        }
    }
}

impl<M: Mode> Drop for MMapedFile<M> {
    fn drop(&mut self) {
        Self::unmap(self.current);

        if !self.for_writing() {
            return;
        }

        let ret = unsafe {
            ftruncate(
                self.file.as_raw_fd() as c_int,
                (self.offset - PAGE_SIZE + self.position) as off_t,
            )
        };
        if unlikely!(ret == -1) {
            die_with_errno!(
                "ftruncate, fd: {}, page size: {}",
                self.file.as_raw_fd(),
                PAGE_SIZE
            );
        }
    }
}

impl io::Write for MMapedFile<Write> {
    fn write(&mut self, data: &[u8]) -> io::Result<usize> {
        let position = self.position; // borrowck
        let slice = &mut self.map_slice_mut()[position..];
        let bytes_len = data.len().min(slice.len());
        slice[..bytes_len].copy_from_slice(&data[..bytes_len]);
        self.position += bytes_len;
        if self.position >= PAGE_SIZE {
            Self::unmap(self.current);
            self.map_next_page();
            self.position = 0;
        }
        Ok(bytes_len)
    }

    fn flush(&mut self) -> io::Result<()> {
        self.sync_blocking();
        Ok(())
    }
}

impl io::Read for MMapedFile<Read> {
    fn read(&mut self, buf: &mut [u8]) -> io::Result<usize> {
        let file_len = self.len;
        let file_bytes_remaining = file_len as usize - (self.offset - PAGE_SIZE + self.position);
        let position = self.position; // borrowck
        let slice = &self.map_slice()[position..];
        let bytes_len = buf.len().min(slice.len()).min(file_bytes_remaining);
        buf[..bytes_len].copy_from_slice(&slice[..bytes_len]);
        self.position += bytes_len;
        if self.position >= PAGE_SIZE {
            Self::unmap(self.current);
            self.map_next_page();
            self.position = 0;
        }
        Ok(bytes_len)
    }
}

#[cfg(test)]
mod test {
    use super::*;
    use crate::test_prelude::*;
    use std::io::{Read, Write};

    rusty_fork_test! {
        #[test]
        fn mmap_file_prop() {
            let path = "/tmp/ert_mmap_prop_test";
            let mut options = std::fs::OpenOptions::new();
            options.truncate(true).read(true).write(true);
            let file = options.clone().open(path).unwrap();
            let mut wtr = MMapedFile::writer(file);
            let data: Vec<_> = (0..(1024*1024 + 7)).map(|b| b as u8).collect();
            wtr.write_all(data.as_slice()).unwrap();
            std::mem::drop(wtr);
            let file = options.truncate(false).open(path).unwrap();
            let mut read_back = Vec::new();
            MMapedFile::reader(file).read_to_end(&mut read_back).unwrap();
            assert_eq!(data, read_back);
        }
    }
}

// TODO investigate performance implications of writes crossing page boundaries
