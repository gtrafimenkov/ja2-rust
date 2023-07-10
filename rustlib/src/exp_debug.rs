use chrono::prelude::*;
use std::ffi::{c_char, CStr};
use std::fs;
use std::io::Write;
use std::sync::OnceLock;

fn get_file() -> &'static mut Option<fs::File> {
    static mut DEBUG_FILE: OnceLock<Option<fs::File>> = OnceLock::new();
    unsafe {
        DEBUG_FILE.get_or_init(create_debug_file);
        return DEBUG_FILE.get_mut().unwrap();
    }
}

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
    let ts = Local::now().format("%Y-%m-%d %H:%M:%S");
    if let Some(f) = get_file() {
        let message = message.trim_end();
        if f.write_fmt(format_args!("{ts}: {message}\n")).is_ok() {
            return f.flush().is_ok();
        }
    }
    false
}

/// Write message to the debug log and panic.
pub fn debug_log_write_and_panic(message: &str) -> bool {
    debug_log_write(message);
    panic!("{message}");
}
