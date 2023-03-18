use byteorder::{LittleEndian, ReadBytesExt};
use std;
use std::io;

#[derive(Debug)]
pub struct Header {
    pub lib_name: String,
    pub lib_path: String,
    pub num_entries: u32,
    pub used_entries: u32,
}

#[repr(C)]
struct RawHeader {
    lib_name: [u8; 256],
    lib_path: [u8; 256],
    num_entries: u32,
    used_entries: u32,
    unused1: u16,
    unused2: u16,
    unused3: u32,
    unused4: u32,
}

const RAW_HEADER_SIZE: usize = std::mem::size_of::<RawHeader>();

fn decode_str(reader: &mut dyn io::Read, size: usize) -> io::Result<String> {
    let mut buffer = vec![0; size];
    reader.read_exact(&mut buffer)?;
    Ok(String::from_utf8_lossy(&buffer)
        .trim_end_matches('\0')
        .to_string())
}

pub fn read_header(reader: &mut dyn io::Read) -> io::Result<Header> {
    // read the whole header
    let mut buffer = [0; RAW_HEADER_SIZE];
    reader.read_exact(&mut buffer).unwrap();

    // parse individual fields
    let mut reader = io::Cursor::new(buffer);
    Ok(Header {
        lib_name: decode_str(&mut reader, 256)?,
        lib_path: decode_str(&mut reader, 256)?,
        num_entries: reader.read_u32::<LittleEndian>()?,
        used_entries: reader.read_u32::<LittleEndian>()?,
    })
}

#[derive(Debug)]
pub struct Entry {
    pub file_name: String,
    pub offset: u32,
    pub size: u32,
}

#[repr(C)]
struct RawEntry {
    file_name: [u8; 256],
    offset: u32,
    size: u32,
    unused1: u32,
    unused2: u32,
    unused3: u32,
    unused4: u32,
}

const RAW_ENTRY_SIZE: usize = std::mem::size_of::<RawEntry>();

pub fn read_entry(reader: &mut dyn io::Read) -> io::Result<Entry> {
    // read the whole header
    let mut buffer = [0; RAW_ENTRY_SIZE];
    reader.read_exact(&mut buffer).unwrap();

    // parse individual fields
    let mut reader = io::Cursor::new(buffer);
    Ok(Entry {
        file_name: decode_str(&mut reader, 256)?,
        offset: reader.read_u32::<LittleEndian>()?,
        size: reader.read_u32::<LittleEndian>()?,
    })
}

#[derive(Debug)]
pub struct Slf {
    pub header: Header,
    pub entries: Vec<Entry>,
}

pub fn read_slf<T: io::Read + io::Seek>(reader: &mut T) -> io::Result<Slf> {
    let header = read_header(reader)?;
    let mut entries = Vec::with_capacity(header.used_entries as usize);
    if header.used_entries > 0 {
        let entries_start: i64 = header.num_entries as i64 * RAW_ENTRY_SIZE as i64;
        reader.seek(io::SeekFrom::End(-entries_start))?;
        for _i in 0..header.used_entries {
            entries.push(read_entry(reader)?);
        }
    }
    Ok(Slf { header, entries })
}
