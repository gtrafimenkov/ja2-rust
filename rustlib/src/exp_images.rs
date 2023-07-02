use crate::exp_alloc::RustDealloc;

use super::exp_alloc;
use super::exp_debug;
use super::exp_fileman::FileID;
use super::exp_fileman::FILE_DB;
use byteorder::{LittleEndian, ReadBytesExt};
use std::io;

#[repr(C)]
#[allow(non_snake_case)]
/// Palette structure, mimics that of Win32
pub struct SGPPaletteEntry {
    peRed: u8,
    peGreen: u8,
    peBlue: u8,
    _unused: u8,
}

#[repr(C)]
#[allow(non_snake_case)]
#[derive(Debug)]
/// Structure that describes one image from an indexed STCI file
pub struct ETRLEObject {
    uiDataOffset: u32,
    uiDataLength: u32,
    sOffsetX: i16,
    sOffsetY: i16,
    usHeight: u16,
    usWidth: u16,
}

#[no_mangle]
pub extern "C" fn TmpImageFunc(_pe: ETRLEObject) {}

/*
Sir-Tech's Crazy Image (STCI) file format specifications.  Each file is composed of:
- ImageFileHeader, uncompressed
- Palette (STCI_INDEXED, size = uiNumberOfColours * PALETTE_ELEMENT_SIZE), uncompressed
- SubRectInfo's (usNumberOfRects > 0, size = usNumberOfSubRects * sizeof(SubRectInfo)), uncompressed
- Bytes of image data, possibly compressed
*/

pub const STCI_ID_LEN: usize = 4;

const STCI_ID_STRING: [u8; 4] = [b'S', b'T', b'C', b'I'];

#[repr(C)]
#[allow(non_snake_case)]
/// Last part of STCI image header
pub struct STCIHeader {
    head: STCIHeaderHead,
    middle: STCIHeaderMiddle,
    end: STCIHeaderEnd,
}

const STCI_RGB: u32 = 0x0004;
const STCI_INDEXED: u32 = 0x0008;
const STCI_ZLIB_COMPRESSED: u32 = 0x0010;
const STCI_ETRLE_COMPRESSED: u32 = 0x0020;

#[repr(C)]
#[allow(non_snake_case)]
#[derive(Default, Debug)]
/// First part of STCI image header
pub struct STCIHeaderHead {
    ID: [u8; STCI_ID_LEN],
    OriginalSize: u32,
    StoredSize: u32, // equal to uiOriginalSize if data uncompressed
    TransparentValue: u32,
    Flags: u32,
    Height: u16,
    Width: u16,
}

#[repr(C)]
// #[derive(Default)]
/// Middle part of STCI image header
enum STCIHeaderMiddle {
    Rgb(STCIHeaderMiddleRGB),
    Indexed(STCIHeaderMiddleIndexed),
}

#[repr(C)]
#[allow(non_snake_case)]
#[derive(Default, Debug, Copy, Clone)]
/// Middle part of STCI image header describing RGB image
pub struct STCIHeaderMiddleRGB {
    uiRedMask: u32,
    uiGreenMask: u32,
    uiBlueMask: u32,
    uiAlphaMask: u32,
    ubRedDepth: u8,
    ubGreenDepth: u8,
    ubBlueDepth: u8,
    ubAlphaDepth: u8,
}

#[repr(C)]
#[allow(non_snake_case)]
#[derive(Default, Debug, Copy, Clone)]
/// Middle part of STCI image header describing RGB image
pub struct STCIHeaderMiddleIndexed {
    // For indexed files, the palette will contain 3 separate bytes for red, green, and
    // blue
    uiNumberOfColours: u32,
    usNumberOfSubImages: u16,
    ubRedDepth: u8,
    ubGreenDepth: u8,
    ubBlueDepth: u8,
    cIndexedUnused: [u8; 11],
}

#[repr(C)]
#[allow(non_snake_case)]
#[derive(Default, Debug)]
/// Last part of STCI image header
pub struct STCIHeaderEnd {
    Depth: u8,   // size in bits of one pixel as stored in the file
    unused1: u8, // added to adjust for memory alignment
    unused2: u8, // added to adjust for memory alignment
    unused3: u8, // added to adjust for memory alignment
    AppDataSize: u32,
    Unused: [u8; 12],
}

fn read_stci_header(file_id: FileID) -> io::Result<STCIHeader> {
    unsafe {
        let mut id: [u8; STCI_ID_LEN] = [0; STCI_ID_LEN];
        FILE_DB.read_file_exact(file_id, &mut id)?;
        let head = STCIHeaderHead {
            ID: id,
            OriginalSize: FILE_DB.read_file_u32(file_id)?,
            StoredSize: FILE_DB.read_file_u32(file_id)?,
            TransparentValue: FILE_DB.read_file_u32(file_id)?,
            Flags: FILE_DB.read_file_u32(file_id)?,
            Height: FILE_DB.read_file_u16(file_id)?,
            Width: FILE_DB.read_file_u16(file_id)?,
        };
        exp_debug::debug_log_write(&format!("STCI header head: {head:?}"));
        if head.ID != STCI_ID_STRING {
            return Err(io::Error::new(io::ErrorKind::Other, "not STCI file"));
        }
        let middle: STCIHeaderMiddle;
        if head.Flags & STCI_INDEXED != 0 {
            let mut indexed = STCIHeaderMiddleIndexed {
                uiNumberOfColours: FILE_DB.read_file_u32(file_id)?,
                usNumberOfSubImages: FILE_DB.read_file_u16(file_id)?,
                ubRedDepth: FILE_DB.read_file_u8(file_id)?,
                ubGreenDepth: FILE_DB.read_file_u8(file_id)?,
                ubBlueDepth: FILE_DB.read_file_u8(file_id)?,
                ..Default::default()
            };
            FILE_DB.read_file_exact(file_id, &mut indexed.cIndexedUnused)?;
            middle = STCIHeaderMiddle::Indexed(indexed);
            exp_debug::debug_log_write(&format!("STCI header middle indexed: {indexed:?}"));
            if indexed.uiNumberOfColours != 256 {
                return Err(io::Error::new(
                    io::ErrorKind::Other,
                    "indexed STCI image must have 256 colors",
                ));
            }
        } else if head.Flags & STCI_RGB != 0 {
            let rgb = STCIHeaderMiddleRGB {
                uiRedMask: FILE_DB.read_file_u32(file_id)?,
                uiGreenMask: FILE_DB.read_file_u32(file_id)?,
                uiBlueMask: FILE_DB.read_file_u32(file_id)?,
                uiAlphaMask: FILE_DB.read_file_u32(file_id)?,
                ubRedDepth: FILE_DB.read_file_u8(file_id)?,
                ubGreenDepth: FILE_DB.read_file_u8(file_id)?,
                ubBlueDepth: FILE_DB.read_file_u8(file_id)?,
                ubAlphaDepth: FILE_DB.read_file_u8(file_id)?,
            };
            middle = STCIHeaderMiddle::Rgb(rgb);
            exp_debug::debug_log_write(&format!("STCI header middle rgb: {rgb:?}"));
        } else {
            return Err(io::Error::new(io::ErrorKind::Other, "unknown image format"));
        }

        let mut end = STCIHeaderEnd {
            Depth: FILE_DB.read_file_u8(file_id)?,
            unused1: FILE_DB.read_file_u8(file_id)?,
            unused2: FILE_DB.read_file_u8(file_id)?,
            unused3: FILE_DB.read_file_u8(file_id)?,
            AppDataSize: FILE_DB.read_file_u32(file_id)?,
            ..Default::default()
        };
        FILE_DB.read_file_exact(file_id, &mut end.Unused)?;
        exp_debug::debug_log_write(&format!("STCI header end: {end:?}"));

        Ok(STCIHeader { head, middle, end })
    }
}

#[no_mangle]
pub extern "C" fn ReadSTCIHeader(file_id: FileID, data: &mut STCIHeader) -> bool {
    match read_stci_header(file_id) {
        Ok(header) => {
            *data = header;
            true
        }
        Err(err) => {
            exp_debug::debug_log_write(&format!("failed to read STCI header: {err}"));
            false
        }
    }
}

#[repr(C)]
#[allow(non_snake_case)]
/// Results of loading STI image.
pub struct STIImageLoaded {
    success: bool,
    //     ID: [u8; STCI_ID_LEN],
    //     OriginalSize: u32,
    StoredSize: u32, // equal to uiOriginalSize if data uncompressed
    //     TransparentValue: u32,
    //     Flags: u32,
    Height: u16,
    Width: u16,
    // }

    // #[repr(C)]
    // // #[derive(Default)]
    // /// Middle part of STCI image header
    // enum STCIHeaderMiddle {
    //     Rgb(STCIHeaderMiddleRGB),
    //     Indexed(STCIHeaderMiddleIndexed),
    // }

    // #[repr(C)]
    // #[allow(non_snake_case)]
    // #[derive(Default, Debug, Copy, Clone)]
    // /// Middle part of STCI image header describing RGB image
    // pub struct STCIHeaderMiddleRGB {
    //     uiRedMask: u32,
    //     uiGreenMask: u32,
    //     uiBlueMask: u32,
    //     uiAlphaMask: u32,
    //     ubRedDepth: u8,
    //     ubGreenDepth: u8,
    //     ubBlueDepth: u8,
    //     ubAlphaDepth: u8,
    // }

    // #[repr(C)]
    // #[allow(non_snake_case)]
    // #[derive(Default, Debug, Copy, Clone)]
    // /// Middle part of STCI image header describing RGB image
    // pub struct STCIHeaderMiddleIndexed {
    //     // For indexed files, the palette will contain 3 separate bytes for red, green, and
    //     // blue
    //     uiNumberOfColours: u32,
    usNumberOfSubImages: u16,
    //     ubRedDepth: u8,
    //     ubGreenDepth: u8,
    //     ubBlueDepth: u8,
    //     cIndexedUnused: [u8; 11],
    // }

    // #[repr(C)]
    // #[allow(non_snake_case)]
    // #[derive(Default, Debug)]
    // /// Last part of STCI image header
    // pub struct STCIHeaderEnd {
    Depth: u8, // size in bits of one pixel as stored in the file
    //     unused1: u8, // added to adjust for memory alignment
    //     unused2: u8, // added to adjust for memory alignment
    //     unused3: u8, // added to adjust for memory alignment
    AppDataSize: u32,
    //     Unused: [u8; 12],
    image_data: *mut u8,
    indexed: bool,                 // indexed (true) or rgb (false)
    palette: *mut SGPPaletteEntry, // only for indexed images
    subimages: *mut ETRLEObject,
    app_data: *mut u8,
    compressed: bool,
}

impl Default for STIImageLoaded {
    fn default() -> Self {
        Self {
            success: false,
            StoredSize: 0,
            Height: 0,
            Width: 0,
            usNumberOfSubImages: 0,
            Depth: 0,
            AppDataSize: 0,
            indexed: false,
            palette: std::ptr::null_mut(),
            subimages: std::ptr::null_mut(),
            app_data: std::ptr::null_mut(),
            image_data: std::ptr::null_mut(),
            compressed: false,
        }
    }
}

#[no_mangle]
pub extern "C" fn LoadSTIImage(file_id: FileID, load_app_data: bool) -> STIImageLoaded {
    let mut results = STIImageLoaded::default();
    match read_stci_header(file_id) {
        Ok(header) => {
            results.compressed = header.head.Flags & STCI_ZLIB_COMPRESSED != 0;
            results.StoredSize = header.head.StoredSize;
            results.Height = header.head.Height;
            results.Width = header.head.Width;
            results.Depth = header.end.Depth;
            results.indexed = header.head.Flags & STCI_INDEXED != 0;
            if results.indexed {
                results.palette = ReadSTCIPalette(file_id);
                if header.head.Flags & STCI_ETRLE_COMPRESSED != 0 {
                    if let STCIHeaderMiddle::Indexed(indexed) = header.middle {
                        results.subimages =
                            ReadSTCISubimages(file_id, indexed.usNumberOfSubImages as usize);
                        if results.subimages.is_null() {
                            unsafe {
                                RustDealloc(std::mem::transmute(results.palette));
                                return STIImageLoaded::default();
                            }
                        }
                    }
                }
                results.image_data = ReadSTCIImageData(file_id, &header);
                if results.image_data.is_null() {
                    unsafe {
                        RustDealloc(std::mem::transmute(results.subimages));
                        RustDealloc(std::mem::transmute(results.palette));
                        return STIImageLoaded::default();
                    }
                }

                if header.end.AppDataSize > 0 && load_app_data {
                    results.app_data = ReadSTCIAppData(file_id, &header);
                    if results.app_data.is_null() {
                        unsafe {
                            RustDealloc(results.image_data);
                            RustDealloc(std::mem::transmute(results.subimages));
                            RustDealloc(std::mem::transmute(results.palette));
                            return STIImageLoaded::default();
                        }
                    }
                }
            } else {
                results.image_data = ReadSTCIImageData(file_id, &header);
            }
            results
        }
        Err(err) => {
            exp_debug::debug_log_write(&format!("failed to read STCI header: {err}"));
            STIImageLoaded::default()
        }
    }
}

#[no_mangle]
// Read image data of STCI image.
// Returns pointer to the read data or null in case of an error.
// The memory should be free afterwards using RustDealloc function.
pub extern "C" fn ReadSTCIImageData(file_id: FileID, header: &STCIHeader) -> *mut u8 {
    let size = header.head.StoredSize as usize;
    exp_debug::debug_log_write(&format!("reading STCI image data, {size} bytes"));
    let pointer = exp_alloc::RustAlloc(size);
    unsafe {
        let slice: &mut [u8] = std::slice::from_raw_parts_mut(pointer, size);
        match FILE_DB.read_file_exact(file_id, slice) {
            Ok(_) => pointer,
            Err(err) => {
                exp_debug::debug_log_write(&format!("failed to read STCI image data: {err:?}"));
                let pointer: *mut u8 = std::ptr::null_mut();
                pointer
            }
        }
    }
}

#[no_mangle]
/// Read STCI indexed image palette from file and return it as SGPPaletteEntry[256] array.
/// If NULL is returned, there was an error reading data from file.
/// Memory must be freed afterwards using RustDealloc function.
pub extern "C" fn ReadSTCIPalette(file_id: FileID) -> *mut SGPPaletteEntry {
    // palette is 256 rgb u8 values
    let mut data: [u8; 256 * 3] = [0; 256 * 3];
    unsafe {
        // let slice: &mut [u8] = std::slice::from_raw_parts_mut(pointer, size);
        match FILE_DB.read_file_exact(file_id, &mut data) {
            Ok(_) => {
                let size = 256 * std::mem::size_of::<SGPPaletteEntry>();
                let pointer: *mut SGPPaletteEntry = std::mem::transmute(exp_alloc::RustAlloc(size));
                let palette: &mut [SGPPaletteEntry] = std::slice::from_raw_parts_mut(pointer, 256);
                for (i, item) in palette.iter_mut().enumerate().take(256) {
                    let start = i * 3;
                    item.peRed = data[start];
                    item.peGreen = data[start + 1];
                    item.peBlue = data[start + 2];
                    item._unused = 0;
                }
                pointer
            }
            Err(err) => {
                exp_debug::debug_log_write(&format!("failed to read STCI palette: {err:?}"));
                std::ptr::null_mut()
            }
        }
    }
}

#[no_mangle]
/// Read STCI indexed image subimages info from the file and return it as ETRLEObject[num_subimages] array.
/// If NULL is returned, there was an error reading data from file.
/// Memory must be freed afterwards using RustDealloc function.
pub extern "C" fn ReadSTCISubimages(file_id: FileID, num_subimages: usize) -> *mut ETRLEObject {
    exp_debug::debug_log_write(&format!("going to read {num_subimages} subimages of STCI"));
    let size = std::mem::size_of::<ETRLEObject>() * num_subimages;
    let mut buffer = vec![0; size];
    unsafe {
        match FILE_DB.read_file_exact(file_id, &mut buffer) {
            Ok(_) => {
                // we can just copy the read data, but going to read them explicitly to fields to respect
                // endianess
                let mut reader = io::Cursor::new(buffer);
                let p: *mut ETRLEObject = std::mem::transmute(exp_alloc::RustAlloc(size));
                let objects: &mut [ETRLEObject] = std::slice::from_raw_parts_mut(p, num_subimages);
                for obj in objects.iter_mut().take(num_subimages) {
                    // can safely do unwrap here because the data is already in memory
                    obj.uiDataOffset = reader.read_u32::<LittleEndian>().unwrap();
                    obj.uiDataLength = reader.read_u32::<LittleEndian>().unwrap();
                    obj.sOffsetX = reader.read_i16::<LittleEndian>().unwrap();
                    obj.sOffsetY = reader.read_i16::<LittleEndian>().unwrap();
                    obj.usHeight = reader.read_u16::<LittleEndian>().unwrap();
                    obj.usWidth = reader.read_u16::<LittleEndian>().unwrap();
                    exp_debug::debug_log_write(&format!("subimage: {obj:?}"));
                }
                p
            }
            Err(err) => {
                exp_debug::debug_log_write(&format!("failed to read STCI subimages: {err:?}"));
                std::ptr::null_mut()
            }
        }
    }
}

#[no_mangle]
// Read application data of STCI image.
// Returns pointer to the read data or null in case of an error.
// The memory should be free afterwards using RustDealloc function.
pub extern "C" fn ReadSTCIAppData(file_id: FileID, header: &STCIHeader) -> *mut u8 {
    let size = header.end.AppDataSize as usize;
    exp_debug::debug_log_write(&format!("reading STCI app data, {size} bytes"));
    let pointer = exp_alloc::RustAlloc(size);
    unsafe {
        let slice: &mut [u8] = std::slice::from_raw_parts_mut(pointer, size);
        match FILE_DB.read_file_exact(file_id, slice) {
            Ok(_) => pointer,
            Err(err) => {
                exp_debug::debug_log_write(&format!("failed to read STCI app data: {err:?}"));
                let pointer: *mut u8 = std::ptr::null_mut();
                pointer
            }
        }
    }
}

#[cfg(test)]
mod tests {
    use std::mem;

    use super::*;

    #[test]
    fn stci_structure_sizes() {
        assert_eq!(24, mem::size_of::<STCIHeaderHead>());
        assert_eq!(20, mem::size_of::<STCIHeaderMiddleRGB>());
        assert_eq!(20, mem::size_of::<STCIHeaderMiddleIndexed>());
        assert_eq!(20, mem::size_of::<STCIHeaderEnd>());
    }
}
