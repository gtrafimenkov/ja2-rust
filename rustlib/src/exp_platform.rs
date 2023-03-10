//! Exported functions to implement platform-dependent code.

use std::env;
use std::ffi::{c_char, CStr};
use std::fs;
use std::path::PathBuf;

/// Converts utf-8 encoded null-terminated C string into PathBuf.
fn cstr_utf8_to_pathbuf(cstr: *const c_char) -> Option<PathBuf> {
    unsafe {
        if cstr.is_null() {
            return None;
        }
        let str = CStr::from_ptr(cstr);
        let str_decoded = String::from_utf8_lossy(str.to_bytes()).to_string();
        Some(PathBuf::from(&str_decoded))
    }
}

#[no_mangle]
/// Change the current working directory.
/// The path string must be utf8 encoded.
pub extern "C" fn Plat_SetCurrentDirectory(path_utf8: *const c_char) -> bool {
    match cstr_utf8_to_pathbuf(path_utf8) {
        None => false,
        Some(path) => env::set_current_dir(path).is_ok(),
    }
}

#[no_mangle]
/// Check if directory exists.
pub extern "C" fn Plat_DirectoryExists(path_utf8: *const c_char) -> bool {
    match cstr_utf8_to_pathbuf(path_utf8) {
        None => false,
        Some(path) => path.is_dir(),
    }
}

#[no_mangle]
/// Create directory.
pub extern "C" fn Plat_CreateDirectory(path_utf8: *const c_char) -> bool {
    match cstr_utf8_to_pathbuf(path_utf8) {
        None => false,
        Some(path) => fs::create_dir(path).is_ok(),
    }
}
