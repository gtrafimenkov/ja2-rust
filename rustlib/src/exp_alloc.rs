use super::exp_debug;
// use super::exp_fileman::FileID;
// use super::exp_fileman::FILE_DB;
// use std::io;

use once_cell::sync::Lazy;
use std::{
    alloc::{alloc, Layout},
    collections::HashMap,
};

static mut ALLOC_DB: Lazy<DB> = Lazy::new(DB::new);

pub struct DB {
    // mapping of allocated addresses to layout objects used for allocating these addresses
    alloc_map: HashMap<*mut u8, Layout>,
}

impl DB {
    fn new() -> Self {
        Self {
            alloc_map: HashMap::new(),
        }
    }

    fn store(&mut self, address: *mut u8, layout: Layout) {
        if self.alloc_map.contains_key(&address) {
            panic!("address {address:?} is already in the allocation map")
        }
        self.alloc_map.insert(address, layout);
    }

    fn pull(&mut self, address: *mut u8) -> Layout {
        match self.alloc_map.remove(&address) {
            Some(layout) => layout,
            None => panic!("address {address:?} is not found in the allocation map"),
        }
    }
}

#[no_mangle]
/// Allocate memory in Rust.
/// The program will panic if memory cannot be allocated.
pub extern "C" fn RustAlloc(size: usize) -> *mut u8 {
    let layout = Layout::from_size_align(size, std::mem::align_of::<u8>()).unwrap();
    let buffer = unsafe { alloc(layout) as *mut u8 };
    if buffer.is_null() {
        panic!("failed to allocate {size} bytes");
    }
    exp_debug::debug_log_write(&format!("rust_alloc: allocated {size} bytes at {buffer:?}"));
    unsafe { ALLOC_DB.store(buffer, layout) };
    buffer
}

#[no_mangle]
/// Deallocate memory allocated earlier in Rust.
///
/// # Safety
///
/// Pass only the pointer returned earlier by RustAlloc.
/// Don't deallocate memory more that once.
pub unsafe extern "C" fn RustDealloc(pointer: *mut u8) {
    if !pointer.is_null() {
        exp_debug::debug_log_write(&format!("rust_alloc: deallocating {pointer:?}"));
        unsafe {
            let layout = ALLOC_DB.pull(pointer);
            std::alloc::dealloc(pointer, layout);
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn simple_alloc_dealloc() {
        let buf = RustAlloc(10);
        assert!(!buf.is_null());
        unsafe {
            RustDealloc(buf);
        }
    }

    #[test]
    #[should_panic]
    fn double_dealloc() {
        let buf = RustAlloc(100);
        assert!(!buf.is_null());
        unsafe {
            RustDealloc(buf);
            RustDealloc(buf);
        }
    }
}
