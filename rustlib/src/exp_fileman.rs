//! Unified file access interface for the C code.
//!
//! This module exports functions File_* similar to C functions FileMan_*.
//! They privide unified access to regular files and files located inside
//! of slf archives.

use crate::slfdb;
use once_cell::sync::Lazy;
use std::ffi::{c_char, c_void, CStr};
use std::io::{Read, Seek, Write};
use std::{collections::HashMap, fs, io};

/// Identifier of an open file.  The file can be a regular file or
/// a file inside of an slf archive.  This is opaque to the user of the library.
pub type FileID = u32;

#[repr(C)]
#[allow(non_camel_case_types)]
pub enum FileSeekMode {
    FILE_SEEK_START = 0x01,
    FILE_SEEK_END = 0x02,
    FILE_SEEK_CURRENT = 0x04,
}

static mut FILE_DB: Lazy<DB> = Lazy::new(DB::new);

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
/// Register slf libraries from directory `dir_path`.
/// `dir_path` must be utf-8 encoded string.
/// Return false, if there were any errors.
pub extern "C" fn File_RegisterSlfLibraries(dir_path: *const c_char) -> bool {
    match cstr_utf8_to_string(dir_path) {
        None => false,
        Some(dir_path) => unsafe { FILE_DB.load_slf_from_dir(&dir_path).is_ok() },
    }
}

#[no_mangle]
/// Open file for reading.  `path` must be utf-8 encoded string.
/// The file can be a regular file or a file from a loaded slf archive.
/// If file is not found or any other error happened, return FILE_ID_ERR.
pub extern "C" fn File_OpenForReading(path: *const c_char) -> FileID {
    match cstr_utf8_to_string(path) {
        None => FILE_ID_ERR,
        Some(path) => match unsafe { FILE_DB.open_for_reading(&path) } {
            Err(_) => FILE_ID_ERR,
            Ok(id) => id,
        },
    }
}

#[no_mangle]
/// Open file for appending.  `path` must be utf-8 encoded string.
/// A recular file will be created in the process.
/// Writing to an slf library file is not supported.
pub extern "C" fn File_OpenForAppending(path: *const c_char) -> FileID {
    match cstr_utf8_to_string(path) {
        None => FILE_ID_ERR,
        Some(path) => match unsafe { FILE_DB.open_for_appending(&path) } {
            Err(_) => FILE_ID_ERR,
            Ok(id) => id,
        },
    }
}

#[no_mangle]
/// Open file for writing.  `path` must be utf-8 encoded string.
/// A recular file will be created in the process.
/// Writing to an slf library file is not supported.
pub extern "C" fn File_OpenForWriting(path: *const c_char) -> FileID {
    match cstr_utf8_to_string(path) {
        None => FILE_ID_ERR,
        Some(path) => match unsafe { FILE_DB.open_for_writing(&path) } {
            Err(_) => FILE_ID_ERR,
            Ok(id) => id,
        },
    }
}

#[no_mangle]
/// Read data from earlier opened file to the buffer.
/// Buffer must be no less than bytes_to_read in size.
///
/// # Safety
///
/// `bytes_read` can be null.
pub unsafe extern "C" fn File_Read(
    file_id: FileID,
    buf: *mut c_void,
    bytes_to_read: u32,
    bytes_read: *mut u32,
) -> bool {
    let buf = buf.cast::<u8>();
    let buf = std::ptr::slice_from_raw_parts_mut(buf, bytes_to_read as usize);
    unsafe {
        match FILE_DB.read_file(file_id, &mut *buf) {
            Ok(n) => {
                let n = n as u32;
                if !bytes_read.is_null() {
                    *bytes_read = n;
                }
                true
            }
            Err(_) => {
                if !bytes_read.is_null() {
                    *bytes_read = 0;
                }
                false
            }
        }
    }
}

#[no_mangle]
/// Write data to an earlier opened file.
///
/// # Safety
///
/// `bytes_written` can be null.
pub unsafe extern "C" fn File_Write(
    file_id: FileID,
    buf: *const c_void,
    bytes_to_write: u32,
    bytes_written: *mut u32,
) -> bool {
    let buf = buf.cast::<u8>();
    let buf = std::ptr::slice_from_raw_parts(buf, bytes_to_write as usize);
    unsafe {
        match FILE_DB.write_file(file_id, &*buf) {
            Ok(n) => {
                let n = n as u32;
                if !bytes_written.is_null() {
                    *bytes_written = n;
                }
                true
            }
            Err(_) => {
                if !bytes_written.is_null() {
                    *bytes_written = 0;
                }
                false
            }
        }
    }
}

#[no_mangle]
/// Check if a file exists (regular or inside of slf library).
pub extern "C" fn File_Exists(path: *const c_char) -> bool {
    let id = File_OpenForReading(path);
    File_Close(id);
    id != FILE_ID_ERR
}

#[no_mangle]
/// Get size of a file.  In case of an error, return 0.
pub extern "C" fn File_GetSize(file_id: FileID) -> u32 {
    unsafe {
        match FILE_DB.get_size(file_id) {
            Ok(size) => size as u32,
            Err(_) => 0,
        }
    }
}

#[no_mangle]
/// Change file read or write position.
pub extern "C" fn File_Seek(file_id: FileID, distance: u32, how: FileSeekMode) -> bool {
    let pos = match how {
        FileSeekMode::FILE_SEEK_START => io::SeekFrom::Start(distance as u64),
        FileSeekMode::FILE_SEEK_END => io::SeekFrom::End(-(distance as i64)),
        FileSeekMode::FILE_SEEK_CURRENT => io::SeekFrom::Current(distance as i64),
    };
    unsafe {
        let res = FILE_DB.seek(file_id, pos);
        // eprintln!("{:?}", res);
        res.is_ok()
    }
}

#[no_mangle]
/// Close earlie opened file.
pub extern "C" fn File_Close(file_id: FileID) -> bool {
    unsafe { FILE_DB.close_file(file_id).is_ok() }
}

/// Special FileID value meaning an error.
pub const FILE_ID_ERR: u32 = 0;

const MAX_FILE_ID: u32 = 0xffff;

enum OpenedFile {
    Regular(fs::File),
    LibFile(String),
}
struct DB {
    next_id: FileID,
    file_map: HashMap<FileID, OpenedFile>,
    slfdb: slfdb::DB,
}

impl DB {
    fn new() -> Self {
        Self {
            next_id: 1,
            file_map: HashMap::new(),
            slfdb: Default::default(),
        }
    }

    /// Load slf files from directory `dir`.
    /// After the load content of slf files will be availabe for access.
    pub fn load_slf_from_dir(&mut self, dir: &str) -> io::Result<()> {
        self.slfdb.load_slf_from_dir(dir)
    }

    fn move_to_next_id(&mut self) {
        self.next_id += 1;
        if self.next_id == MAX_FILE_ID {
            self.next_id = 1;
        }
    }

    /// Store opened file and return ID.
    /// The function will panic if there are no free IDs.
    fn store_opened_file(&mut self, f: OpenedFile) -> FileID {
        let start_id = self.next_id;
        // we are going to search for the first unused id
        loop {
            if self.file_map.get(&self.next_id).is_none() {
                // free id found
                break;
            }
            self.move_to_next_id();
            if self.next_id == start_id {
                panic!("FileMan: no free ids left");
            }
        }
        let id = self.next_id;
        self.file_map.insert(id, f);
        self.move_to_next_id();
        id
    }

    /// Open the given file for reading.
    /// First try to open it as a regular file, if not successful, try to find
    /// it in one of the loaded slf libraries.  If not found, return
    /// io::ErrorKind::NotFound
    pub fn open_for_reading(&mut self, path: &str) -> io::Result<FileID> {
        match fs::File::open(path) {
            Ok(f) => Ok(self.store_opened_file(OpenedFile::Regular(f))),

            Err(_) => {
                // checking slf libraries
                match self.slfdb.has_libfile(path) {
                    true => Ok(self.store_opened_file(OpenedFile::LibFile(path.to_owned()))),
                    false => Err(io::Error::from(io::ErrorKind::NotFound)),
                }
            }
        }
    }

    /// Open the given file for appending.
    /// Writing to a file from slf library is not supported.
    pub fn open_for_appending(&mut self, path: &str) -> io::Result<FileID> {
        match fs::OpenOptions::new().append(true).open(path) {
            Ok(f) => Ok(self.store_opened_file(OpenedFile::Regular(f))),
            Err(err) => Err(err),
        }
    }

    /// Open the given file for writing.
    /// Writing to a file from slf library is not supported.
    pub fn open_for_writing(&mut self, path: &str) -> io::Result<FileID> {
        match fs::File::create(path) {
            Ok(f) => Ok(self.store_opened_file(OpenedFile::Regular(f))),
            Err(err) => Err(err),
        }
    }

    /// Read opened earlier file
    pub fn read_file(&mut self, file_id: FileID, buf: &mut [u8]) -> io::Result<usize> {
        match self.file_map.get_mut(&file_id) {
            None => Err(io::Error::from(io::ErrorKind::NotFound)),
            Some(OpenedFile::LibFile(path)) => self.slfdb.read_libfile(path, buf),
            Some(OpenedFile::Regular(f)) => f.read(buf),
        }
    }

    /// Write to opened earlier file.  Only writing to regular files is supported.
    pub fn write_file(&mut self, file_id: FileID, buf: &[u8]) -> io::Result<usize> {
        match self.file_map.get_mut(&file_id) {
            None => Err(io::Error::from(io::ErrorKind::NotFound)),
            Some(OpenedFile::LibFile(_)) => Err(io::Error::from(io::ErrorKind::Unsupported)),
            Some(OpenedFile::Regular(f)) => f.write(buf),
        }
    }

    /// Close opened earlier file
    pub fn close_file(&mut self, file_id: FileID) -> io::Result<()> {
        match self.file_map.get_mut(&file_id) {
            None => Err(io::Error::from(io::ErrorKind::NotFound)),
            Some(OpenedFile::LibFile(_)) => {
                self.file_map.remove(&file_id);
                Ok(())
            }
            Some(OpenedFile::Regular(_)) => {
                self.file_map.remove(&file_id);
                Ok(())
            }
        }
    }

    /// Get size of a file.
    pub fn get_size(&mut self, file_id: FileID) -> io::Result<u64> {
        match self.file_map.get_mut(&file_id) {
            None => Err(io::Error::from(io::ErrorKind::NotFound)),
            Some(OpenedFile::LibFile(path)) => match self.slfdb.get_libfile_size(path) {
                Ok(size) => Ok(size as u64),
                Err(err) => Err(err),
            },
            Some(OpenedFile::Regular(f)) => match f.metadata() {
                Ok(metadata) => Ok(metadata.len()),
                Err(err) => Err(err),
            },
        }
    }

    /// Change file position.  Return new file position from start of the file.
    pub fn seek(&mut self, file_id: FileID, pos: io::SeekFrom) -> io::Result<u64> {
        match self.file_map.get_mut(&file_id) {
            None => Err(io::Error::from(io::ErrorKind::NotFound)),
            Some(OpenedFile::LibFile(path)) => self.slfdb.seek_libfile(path, pos),
            Some(OpenedFile::Regular(f)) => f.seek(pos),
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn loading_slfs() {
        let mut fdb = DB::new();
        fdb.load_slf_from_dir("../tools/editor").unwrap();
        // assert_eq!(true, fdb.has_library("EDITOR.SLF"));
    }

    #[test]
    fn opening_and_closing() {
        let mut fdb = DB::new();
        fdb.load_slf_from_dir("../tools/editor").unwrap();
        let id = fdb.open_for_reading("./Cargo.toml").unwrap();
        assert_eq!(fdb.file_map.len(), 1);
        fdb.close_file(id).unwrap();
        assert_eq!(fdb.file_map.len(), 0);
    }

    #[test]
    fn get_size() {
        // get size of a library file
        {
            let mut fdb = DB::new();
            fdb.load_slf_from_dir("../tools/editor").unwrap();
            let file_id = fdb.open_for_reading("Editor\\EXITGRIDBUT.STI").unwrap();
            assert_eq!(5712, fdb.get_size(file_id).unwrap());
        }

        // get size of a regular file
        {
            let mut fdb = DB::new();
            fdb.load_slf_from_dir("../tools/editor").unwrap();
            let file_id = fdb.open_for_reading("../tools/editor/Editor.slf").unwrap();
            assert_eq!(3444529, fdb.get_size(file_id).unwrap());
        }
    }
}