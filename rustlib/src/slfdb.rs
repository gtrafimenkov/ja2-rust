//! Database of files from slf archives and functions to read these files.

use crate::slf;
use std::collections::HashMap;
use std::fs::{self, File};
use std::io::{self, Read, Seek};

/// Name of an slf library.  The name is taken from slf file header.
type LibName = String;

/// Information about a file from an slf library.
#[derive(Debug)]
struct LibFile {
    lib: LibName,
    /// offset from the beginning of the slf file
    offset: u32,
    /// size of the file
    size: u32,
    /// read position for the file relative to offset
    read_pos: u32,
}

#[derive(Debug)]
#[allow(dead_code)]
struct Lib {
    slf_info: slf::Slf,
    slf_file_path: std::path::PathBuf,
    opened_file: fs::File,
}

// Contains information about files from slf archives.
#[derive(Debug)]
pub struct DB {
    /// Loaded slf libraries.  This is a map of library names to library structures.
    libs: HashMap<LibName, Lib>,

    /// Mapping of file names (paths) contained in libraries to library names.
    lib_files: HashMap<String, LibFile>,
}

impl DB {
    pub fn new() -> Self {
        DB {
            libs: HashMap::new(),
            lib_files: HashMap::new(),
        }
    }

    /// Load slf files from directory `dir`.
    /// After the load content of slf files will be availabe for access.
    pub fn load_slf_from_dir(&mut self, dir: &str) -> io::Result<()> {
        let iter = std::fs::read_dir(dir)?;
        for entry in iter {
            let entry = entry?;
            if entry.path().is_file() {
                let name = entry.file_name().to_string_lossy().to_lowercase();
                if name.ends_with(".slf") {
                    let mut f = File::open(entry.path())?;
                    let slf = slf::read_slf(&mut f)?;

                    for entry in slf.entries.iter() {
                        let libfile_path =
                            build_canonical_libfile_path(&slf.header.lib_path, &entry.file_name);
                        let location = LibFile {
                            lib: slf.header.lib_name.clone(),
                            offset: entry.offset,
                            size: entry.size,
                            read_pos: 0,
                        };
                        self.lib_files.insert(libfile_path, location);
                    }

                    self.libs.insert(
                        slf.header.lib_name.clone(),
                        Lib {
                            slf_info: slf,
                            slf_file_path: entry.path(),
                            opened_file: f,
                        },
                    );
                }
            }
        }
        Ok(())
    }

    /// Get number of libraries in the DB.
    pub fn count(&self) -> usize {
        self.libs.len()
    }

    /// Check if the database has library (slf file) with name `name` loaded.
    pub fn has_library(&self, name: &str) -> bool {
        self.libs.get(name).is_some()
    }

    /// Check if the database has the given file in one of the loaded libraries.
    pub fn has_libfile(&self, path: &str) -> bool {
        self.lib_files.get(&canonize_libfile_path(path)).is_some()
    }

    /// Return the size of a file from a library.
    /// If file doens't exist, io::ErrorKind::NotFound will be returned.
    pub fn get_libfile_size(&self, path: &str) -> io::Result<u32> {
        match self.lib_files.get(&canonize_libfile_path(path)) {
            None => Err(io::Error::from(io::ErrorKind::NotFound)),
            Some(location) => Ok(location.size),
        }
    }

    /// Read a library file into buffer.
    /// The buffer must not be bigger that the size of the file.
    pub fn read_libfile_exact(&mut self, path: &str, buffer: &mut [u8]) -> io::Result<()> {
        match self.lib_files.get_mut(&canonize_libfile_path(path)) {
            None => Err(io::Error::from(io::ErrorKind::NotFound)),
            Some(loc) => {
                if buffer.len() + loc.read_pos as usize > loc.size as usize {
                    return Err(io::Error::from(io::ErrorKind::InvalidInput));
                }
                let lib = self.libs.get_mut(&loc.lib).unwrap();
                let read_pos = loc.offset + loc.read_pos;
                lib.opened_file.seek(io::SeekFrom::Start(read_pos as u64))?;
                lib.opened_file.read_exact(buffer)?;
                loc.read_pos += buffer.len() as u32;
                Ok(())
            }
        }
    }

    /// Read up to buf.len() bytes from a file.
    /// Return number of bytes read (n).  If n == 0, the file end was reached.
    pub fn read_libfile(&mut self, path: &str, buf: &mut [u8]) -> io::Result<usize> {
        match self.lib_files.get_mut(&canonize_libfile_path(path)) {
            None => Err(io::Error::from(io::ErrorKind::NotFound)),
            Some(loc) => {
                let max_bytes_to_read =
                    std::cmp::min((loc.size - loc.read_pos) as usize, buf.len());
                let buffer = &mut buf[0..max_bytes_to_read];
                let lib = self.libs.get_mut(&loc.lib).unwrap();
                let read_pos = loc.offset + loc.read_pos;
                lib.opened_file.seek(io::SeekFrom::Start(read_pos as u64))?;
                let n = lib.opened_file.read(buffer)?;
                loc.read_pos += n as u32;
                Ok(n)
            }
        }
    }

    /// Change read position for the library file.
    pub fn seek_libfile(&mut self, path: &str, pos: io::SeekFrom) -> io::Result<u64> {
        match self.lib_files.get_mut(&canonize_libfile_path(path)) {
            None => Err(io::Error::from(io::ErrorKind::NotFound)),
            Some(loc) => {
                // checking that we are not going out of the available range
                let cur_abs_pos = (loc.offset + loc.read_pos) as i64;
                let end_abs_pos = (loc.offset + loc.size) as i64;
                let new_abs_pos = match pos {
                    io::SeekFrom::Start(val) => loc.offset as i64 + val as i64,
                    io::SeekFrom::Current(val) => cur_abs_pos + val,
                    io::SeekFrom::End(val) => end_abs_pos + val,
                };
                if new_abs_pos < loc.offset as i64 || new_abs_pos >= end_abs_pos {
                    return Err(io::Error::from(io::ErrorKind::InvalidInput));
                }

                // actual seek will happen just before the read
                // for now just storing the new position
                loc.read_pos = (new_abs_pos - loc.offset as i64) as u32;
                Ok(loc.read_pos as u64)
            }
        }
    }
}

impl Default for DB {
    fn default() -> Self {
        Self::new()
    }
}

/// Build canonical path for a library file given the library path (from slf header)
/// and the file name from slf entry.
fn build_canonical_libfile_path(lib_path: &str, file_name: &str) -> String {
    canonize_libfile_path(&format!(
        "{}/{}",
        lib_path.trim_end_matches('\\').trim_end_matches('/'),
        file_name
    ))
}

// Modify library file path to the form which is used internally.
fn canonize_libfile_path(path: &str) -> String {
    path.to_lowercase().replace('\\', "/")
}

#[cfg(test)]
mod tests {
    use super::*;
    use crypto::digest::Digest;
    use crypto::sha2::Sha256;

    #[test]
    fn loading_slfs() {
        let mut fdb = DB::new();
        fdb.load_slf_from_dir("../tools/editor").unwrap();
        assert_eq!(true, fdb.has_library("EDITOR.SLF"));

        assert_eq!(true, fdb.has_libfile("editor/exitgridbut.sti"));
        assert_eq!(true, fdb.has_libfile("editor\\exitgridbut.sti"));
        assert_eq!(true, fdb.has_libfile("Editor\\EXITGRIDBUT.STI"));

        assert_eq!(
            5712,
            fdb.get_libfile_size("Editor\\EXITGRIDBUT.STI").unwrap()
        );
    }

    #[test]
    fn reading_file_from_slf() {
        let mut fdb = DB::new();
        fdb.load_slf_from_dir("../tools/editor").unwrap();

        let path = "Editor\\EXITGRIDBUT.STI";
        let hash = "b7099f0fefcc7cff52081c12d304b84d712fb36c4212407afe921cb4208b7adc";

        assert_eq!(5712, fdb.get_libfile_size(path).unwrap());

        // reading the file and testing the content
        {
            let mut buffer = [0; 5712];
            fdb.read_libfile_exact(path, &mut buffer).unwrap();

            let mut hasher = Sha256::new();
            hasher.input(&buffer);
            assert_eq!(hash, hasher.result_str());
        }

        // reading again
        {
            fdb.seek_libfile(path, io::SeekFrom::Start(0)).unwrap();

            let mut buffer = [0; 5712];
            fdb.read_libfile_exact(path, &mut buffer).unwrap();

            let mut hasher = Sha256::new();
            hasher.input(&buffer);
            assert_eq!(hash, hasher.result_str());
        }

        // reading again with read_libfile function
        {
            fdb.seek_libfile(path, io::SeekFrom::Start(0)).unwrap();

            let mut buffer = [0; 6000];
            let bytes_read = fdb.read_libfile(path, &mut buffer).unwrap();
            assert_eq!(5712, bytes_read);

            let mut hasher = Sha256::new();
            hasher.input(&buffer[0..5712]);
            assert_eq!(hash, hasher.result_str());
        }

        // reading again with many read_libfile reads
        {
            fdb.seek_libfile(path, io::SeekFrom::Start(0)).unwrap();

            let mut buffer = [0; 6000];
            assert_eq!(2000, fdb.read_libfile(path, &mut buffer[0..2000]).unwrap());
            assert_eq!(
                2000,
                fdb.read_libfile(path, &mut buffer[2000..4000]).unwrap()
            );
            assert_eq!(
                1712,
                fdb.read_libfile(path, &mut buffer[4000..5712]).unwrap()
            );

            let mut hasher = Sha256::new();
            hasher.input(&buffer[0..5712]);
            assert_eq!(hash, hasher.result_str());
        }

        // reading again with many read_libfile reads and seek operations
        {
            let mut buffer = [0; 6000];

            fdb.seek_libfile(path, io::SeekFrom::Start(0)).unwrap();
            assert_eq!(2000, fdb.read_libfile(path, &mut buffer[0..2000]).unwrap());

            fdb.seek_libfile(path, io::SeekFrom::End(-1712)).unwrap();
            assert_eq!(
                1712,
                fdb.read_libfile(path, &mut buffer[4000..5712]).unwrap()
            );

            fdb.seek_libfile(path, io::SeekFrom::Start(0)).unwrap();
            fdb.seek_libfile(path, io::SeekFrom::Current(2000)).unwrap();
            assert_eq!(
                2000,
                fdb.read_libfile(path, &mut buffer[2000..4000]).unwrap()
            );

            let mut hasher = Sha256::new();
            hasher.input(&buffer[0..5712]);
            assert_eq!(hash, hasher.result_str());

            assert_eq!('S', buffer[0] as char);
            assert_eq!('T', buffer[1] as char);
            assert_eq!('C', buffer[2] as char);
        }
    }

    #[test]
    fn canonical_libfile_path() {
        assert_eq!(
            "editor/exitgridbut.sti",
            build_canonical_libfile_path("Editor\\", "EXITGRIDBUT.STI")
        );
    }
}
