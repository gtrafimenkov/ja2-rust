use chrono::prelude::*;
use once_cell::sync::Lazy;
use std::ffi::{c_char, CStr};
use std::fs;
use std::io::Write;

static mut DEBUG_FILE: Lazy<Option<fs::File>> = Lazy::new(create_debug_file);

fn create_debug_file() -> Option<fs::File> {
    if let Ok(path) = std::env::current_exe() {
        if let Some(path) = path.parent() {
            let name = format!("debug-{}.txt", Local::now().format("%Y%m%d-%H%M%S"));
            let mut path = path.to_path_buf();
            path.push(name);
            return fs::File::create(path).ok();
        }
    }
    None
}

/// Converts utf-8 encoded null-terminated C string into String.
/// If cstr is null, return None.
fn cstr_utf8_to_string(cstr: *const c_char) -> Option<String> {
    if cstr.is_null() {
        return None;
    }
    let str = unsafe { CStr::from_ptr(cstr) };
    Some(String::from_utf8_lossy(str.to_bytes()).to_string())
}

#[no_mangle]
/// Write message into the debug log.
/// The string must be utf-8 encoded.
pub extern "C" fn DebugLogWrite(message: *const c_char) -> bool {
    if let Some(message) = cstr_utf8_to_string(message) {
        return debug_log_write(&message);
    }
    false
}

/// Write message to the debug log
pub fn debug_log_write(message: &str) -> bool {
    unsafe {
        let ts = Local::now().format("%Y-%m-%d %H:%M:%S");
        if let Some(f) = DEBUG_FILE.as_mut() {
            if f.write_fmt(format_args!("{ts}: {message}\n")).is_ok() {
                return f.flush().is_ok();
            }
        }
    }
    false
}
