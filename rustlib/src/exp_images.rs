use crate::exp_alloc::RustDealloc;

use super::exp_alloc;
use super::exp_debug;
use super::exp_fileman::FileID;
use super::exp_fileman::FILE_DB;
use byteorder::{LittleEndian, ReadBytesExt};
use std::io;

#[repr(C)]
#[allow(non_snake_case)]
#[derive(Default, Copy, Clone)]
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
pub const STCI_ZLIB_COMPRESSED: u32 = 0x0010;
pub const STCI_ETRLE_COMPRESSED: u32 = 0x0020;

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

#[repr(C)]
#[allow(non_snake_case)]
/// Results of loading STI image.
pub struct STIImageLoaded {
    success: bool,
    image_data_size: u32, // equal to uiOriginalSize if data uncompressed
    Height: u16,
    Width: u16,
    number_of_subimages: u16,
    pixel_depth: u8,
    app_data_size: u32,
    image_data: *mut u8,
    indexed: bool,                 // indexed (true) or rgb (false)
    palette: *mut SGPPaletteEntry, // only for indexed images
    subimages: *mut ETRLEObject,
    app_data: *mut u8,
    zlib_compressed: bool,
}

impl Default for STIImageLoaded {
    fn default() -> Self {
        Self {
            success: false,
            image_data_size: 0,
            Height: 0,
            Width: 0,
            number_of_subimages: 0,
            pixel_depth: 0,
            app_data_size: 0,
            indexed: false,
            palette: std::ptr::null_mut(),
            subimages: std::ptr::null_mut(),
            app_data: std::ptr::null_mut(),
            image_data: std::ptr::null_mut(),
            zlib_compressed: false,
        }
    }
}

#[no_mangle]
pub extern "C" fn LoadSTIImage2(file_id: FileID, load_app_data: bool) -> STIImageLoaded {
    let failure = STIImageLoaded {
        success: false,
        image_data_size: 0,
        Height: 0,
        Width: 0,
        number_of_subimages: 0,
        pixel_depth: 0,
        app_data_size: 0,
        indexed: false,
        palette: std::ptr::null_mut(),
        subimages: std::ptr::null_mut(),
        app_data: std::ptr::null_mut(),
        image_data: std::ptr::null_mut(),
        zlib_compressed: false,
    };
    let img = read_stci(file_id, true);

    if let Err(err) = img {
        exp_debug::debug_log_write(&format!("failed to read stci image: {err}"));
        return failure;
    }

    let img = img.unwrap();

    let mut number_of_subimages = 0;
    if let Some(subimages) = img.subimages {
        number_of_subimages = subimages.len();
        // TODO: allocate and copy memory
    }

    return STIImageLoaded {
        success: true,
        Height: img.height,
        Width: img.width,
        indexed: img.indexed,
        pixel_depth: img.pixel_depth,
        image_data_size: todo!(),
        image_data: todo!(),
        palette: todo!(),
        number_of_subimages,
        subimages: todo!(),
        app_data_size: todo!(),
        app_data: todo!(),
        zlib_compressed: todo!(),
    };
}

#[no_mangle]
pub extern "C" fn LoadSTIImage(file_id: FileID, load_app_data: bool) -> STIImageLoaded {
    let mut results = STIImageLoaded::default();
    match read_stci_header(file_id) {
        Ok(header) => {
            results.zlib_compressed = header.head.Flags & STCI_ZLIB_COMPRESSED != 0;
            results.image_data_size = header.head.StoredSize;
            results.Height = header.head.Height;
            results.Width = header.head.Width;
            results.pixel_depth = header.end.Depth;
            results.indexed = header.head.Flags & STCI_INDEXED != 0;
            if results.indexed {
                results.palette = {
                    // palette is 256 rgb u8 values
                    let mut data: [u8; 256 * 3] = [0; 256 * 3];
                    unsafe {
                        match FILE_DB.read_file_exact(file_id, &mut data) {
                            Ok(_) => {
                                let size = 256 * std::mem::size_of::<SGPPaletteEntry>();
                                let pointer: *mut SGPPaletteEntry =
                                    std::mem::transmute(exp_alloc::RustAlloc(size));
                                let palette: &mut [SGPPaletteEntry] =
                                    std::slice::from_raw_parts_mut(pointer, 256);
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
                                exp_debug::debug_log_write(&format!(
                                    "failed to read STCI palette: {err:?}"
                                ));
                                std::ptr::null_mut()
                            }
                        }
                    }
                };
                if results.palette.is_null() {
                    exp_debug::debug_log_write("failed to read palette data");
                    return STIImageLoaded::default();
                }

                if header.head.Flags & STCI_ETRLE_COMPRESSED != 0 {
                    if let STCIHeaderMiddle::Indexed(indexed) = header.middle {
                        results.subimages =
                            read_stci_subimages(file_id, indexed.usNumberOfSubImages as usize);
                        if results.subimages.is_null() {
                            unsafe {
                                exp_debug::debug_log_write("failed to read the palette");
                                RustDealloc(std::mem::transmute(results.palette));
                                return STIImageLoaded::default();
                            }
                        }
                        results.number_of_subimages = indexed.usNumberOfSubImages;
                    } else {
                        exp_debug::debug_log_write("it must have been indexed image");
                        return STIImageLoaded::default();
                    }
                }
                results.image_data = read_stci_image_data(file_id, &header);
                if results.image_data.is_null() {
                    unsafe {
                        exp_debug::debug_log_write("failed to read image data");
                        RustDealloc(std::mem::transmute(results.subimages));
                        RustDealloc(std::mem::transmute(results.palette));
                        return STIImageLoaded::default();
                    }
                }

                if header.end.AppDataSize > 0 && load_app_data {
                    results.app_data = read_stci_app_data(file_id, &header);
                    if results.app_data.is_null() {
                        unsafe {
                            exp_debug::debug_log_write("failed to read app data");
                            RustDealloc(results.image_data);
                            RustDealloc(std::mem::transmute(results.subimages));
                            RustDealloc(std::mem::transmute(results.palette));
                            return STIImageLoaded::default();
                        }
                    }
                    results.app_data_size = header.end.AppDataSize;
                }
            } else {
                results.image_data = read_stci_image_data(file_id, &header);
                if results.image_data.is_null() {
                    exp_debug::debug_log_write("failed to read image data");
                    return STIImageLoaded::default();
                }
            }
            results.success = true;
            results
        }
        Err(err) => {
            exp_debug::debug_log_write(&format!("failed to read STCI header: {err}"));
            STIImageLoaded::default()
        }
    }
}

// Read image data of STCI image.
// Returns pointer to the read data or null in case of an error.
// The memory should be free afterwards using RustDealloc function.
fn read_stci_image_data(file_id: FileID, header: &STCIHeader) -> *mut u8 {
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

/// Read STCI indexed image subimages info from the file and return it as ETRLEObject[num_subimages] array.
/// If NULL is returned, there was an error reading data from file.
/// Memory must be freed afterwards using RustDealloc function.
fn read_stci_subimages(file_id: FileID, num_subimages: usize) -> *mut ETRLEObject {
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

// Read application data of STCI image.
// Returns pointer to the read data or null in case of an error.
// The memory should be free afterwards using RustDealloc function.
fn read_stci_app_data(file_id: FileID, header: &STCIHeader) -> *mut u8 {
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

/// Sir-Tech's Crazy Image
pub struct STImage {
    height: u16,
    width: u16,
    pixel_depth: u8,
    image_data: Vec<u8>,
    indexed: bool,                           // indexed (true) or rgb (false)
    palette: Option<[SGPPaletteEntry; 256]>, // only for indexed images
    subimages: Option<Vec<ETRLEObject>>,
    app_data: Option<Vec<u8>>,
    zlib_compressed: bool,
}

// fn read_stci_header(file_id: FileID) -> io::Result<STCIHeader> {
fn read_stci(file_id: FileID, load_app_data: bool) -> io::Result<STImage> {
    unsafe {
        let mut id: [u8; STCI_ID_LEN] = [0; STCI_ID_LEN];
        FILE_DB.read_file_exact(file_id, &mut id)?;
        if id != STCI_ID_STRING {
            return Err(io::Error::new(
                io::ErrorKind::Other,
                format!("not STCI file ({id:?})"),
            ));
        }
        let _original_size = FILE_DB.read_file_u32(file_id)?;
        let stored_size = FILE_DB.read_file_u32(file_id)? as usize;
        let _transparent_value = FILE_DB.read_file_u32(file_id)?;
        let flags = FILE_DB.read_file_u32(file_id)?;
        let height = FILE_DB.read_file_u16(file_id)?;
        let width = FILE_DB.read_file_u16(file_id)?;
        if flags & STCI_INDEXED == 0 && flags & STCI_RGB == 0 {
            return Err(io::Error::new(io::ErrorKind::Other, "unknown image format"));
        }
        let indexed = flags & STCI_INDEXED != 0;
        let mut num_subimages = 0;
        if indexed {
            // index
            let number_of_colours = FILE_DB.read_file_u32(file_id)?;
            if number_of_colours != 256 {
                return Err(io::Error::new(
                    io::ErrorKind::Other,
                    "indexed STCI image must have 256 colors",
                ));
            }
            num_subimages = FILE_DB.read_file_u16(file_id)? as usize;
            let _red_depth = FILE_DB.read_file_u8(file_id)?;
            let _green_depth = FILE_DB.read_file_u8(file_id)?;
            let _blue_depth = FILE_DB.read_file_u8(file_id)?;
            let mut unused: [u8; 11] = [0; 11];
            FILE_DB.read_file_exact(file_id, &mut unused)?;
        } else {
            // RGB
            let _red_mask = FILE_DB.read_file_u32(file_id)?;
            let _green_mask = FILE_DB.read_file_u32(file_id)?;
            let _blue_mask = FILE_DB.read_file_u32(file_id)?;
            let _alpha_mask = FILE_DB.read_file_u32(file_id)?;
            let _red_depth = FILE_DB.read_file_u8(file_id)?;
            let _green_depth = FILE_DB.read_file_u8(file_id)?;
            let _blue_depth = FILE_DB.read_file_u8(file_id)?;
            let _alpha_depth = FILE_DB.read_file_u8(file_id)?;
        }

        let pixel_depth = FILE_DB.read_file_u8(file_id)?;
        let _unused1 = FILE_DB.read_file_u8(file_id)?;
        let _unused2 = FILE_DB.read_file_u8(file_id)?;
        let _unused3 = FILE_DB.read_file_u8(file_id)?;
        let app_data_size = FILE_DB.read_file_u32(file_id)? as usize;
        let mut unused: [u8; 12] = [0; 12];
        FILE_DB.read_file_exact(file_id, &mut unused)?;

        let zlib_compressed = flags & STCI_ZLIB_COMPRESSED != 0;
        let mut image_data = vec![0; stored_size];
        let mut palette: Option<[SGPPaletteEntry; 256]> = None;
        let mut subimages: Option<Vec<ETRLEObject>> = None;
        let mut app_data: Option<Vec<u8>> = None;

        if indexed {
            // palette
            palette = Some([Default::default(); 256]);
            {
                // palette is 256 rgb u8 values
                let mut data: [u8; 256 * 3] = [0; 256 * 3];
                FILE_DB.read_file_exact(file_id, &mut data)?;
                for (i, item) in palette.as_mut().unwrap().iter_mut().enumerate() {
                    let start = i * 3;
                    item.peRed = data[start];
                    item.peGreen = data[start + 1];
                    item.peBlue = data[start + 2];
                    item._unused = 0;
                }
            }

            // subimages
            if num_subimages > 0 {
                let mut collector = Vec::with_capacity(num_subimages);
                let size = 16 * num_subimages;
                let mut buffer = vec![0; size];
                FILE_DB.read_file_exact(file_id, &mut buffer)?;
                let mut reader = io::Cursor::new(buffer);
                for _i in 0..num_subimages {
                    let subimage = ETRLEObject {
                        uiDataOffset: reader.read_u32::<LittleEndian>()?,
                        uiDataLength: reader.read_u32::<LittleEndian>()?,
                        sOffsetX: reader.read_i16::<LittleEndian>()?,
                        sOffsetY: reader.read_i16::<LittleEndian>()?,
                        usHeight: reader.read_u16::<LittleEndian>()?,
                        usWidth: reader.read_u16::<LittleEndian>()?,
                    };
                    exp_debug::debug_log_write(&format!("subimage: {subimage:?}"));
                    collector.push(subimage);
                }
                subimages = Some(collector);
            }

            FILE_DB.read_file_exact(file_id, &mut image_data)?;

            if app_data_size > 0 {
                app_data = Some(vec![0; app_data_size]);
                if app_data_size > 0 && load_app_data {
                    FILE_DB.read_file_exact(file_id, app_data.as_mut().unwrap())?;
                }
            }
        } else {
            FILE_DB.read_file_exact(file_id, &mut image_data)?;
        }
        return Ok(STImage {
            height,
            width,
            pixel_depth,
            image_data,
            indexed,
            palette,
            subimages,
            app_data,
            zlib_compressed,
        });
    }
}

#[cfg(test)]
mod tests {
    use super::super::exp_fileman::FILE_DB;
    use std::mem;

    use super::*;

    #[test]
    fn stci_structure_sizes() {
        assert_eq!(24, mem::size_of::<STCIHeaderHead>());
        assert_eq!(20, mem::size_of::<STCIHeaderMiddleRGB>());
        assert_eq!(20, mem::size_of::<STCIHeaderMiddleIndexed>());
        assert_eq!(20, mem::size_of::<STCIHeaderEnd>());
    }

    #[test]
    fn load_stci() {
        unsafe {
            FILE_DB.load_slf_from_dir("../tools/editor").unwrap();
            let file_id = FILE_DB.open_for_reading("Editor\\CANCEL.STI").unwrap();
            // LoadSTIImage(file_id, true);
            let img = read_stci(file_id, true).unwrap();
            assert_eq!(640, img.width);
            assert_eq!(480, img.height);
            assert_eq!(true, img.indexed);
            assert_eq!(false, img.zlib_compressed);
            assert_eq!(8, img.pixel_depth);
            assert_eq!(4800, img.image_data.len());
            assert_eq!(true, img.app_data.is_none());
            let subimage0 = &img.subimages.as_ref().unwrap()[0];
            let subimage4 = &img.subimages.as_ref().unwrap()[4];
            assert_eq!(5, img.subimages.as_ref().unwrap().len());
            assert_eq!(0, subimage0.sOffsetX);
            assert_eq!(0, subimage0.sOffsetY);
            assert_eq!(30, subimage0.usHeight);
            assert_eq!(30, subimage0.usWidth);
            assert_eq!(0, subimage0.uiDataOffset);
            assert_eq!(960, subimage0.uiDataLength);
            assert_eq!(0, subimage4.sOffsetX);
            assert_eq!(0, subimage4.sOffsetY);
            assert_eq!(30, subimage4.usHeight);
            assert_eq!(30, subimage4.usWidth);
            assert_eq!(3840, subimage4.uiDataOffset);
            assert_eq!(960, subimage4.uiDataLength);

            // STCI header head: STCIHeaderHead { ID: [83, 84, 67, 73], OriginalSize: 307200, StoredSize: 4800, TransparentValue: 0, Flags: 40, Height: 480, Width: 640 }
            // STCI header middle indexed: STCIHeaderMiddleIndexed { uiNumberOfColours: 256, usNumberOfSubImages: 5, ubRedDepth: 8, ubGreenDepth: 8, ubBlueDepth: 8, cIndexedUnused: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0] }
            // STCI header end: STCIHeaderEnd { Depth: 8, unused1: 0, unused2: 0, unused3: 0, AppDataSize: 0, Unused: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0] }
            // rust_alloc: allocated 1024 bytes at 0x166c7a9f8b0
            // going to read 5 subimages of STCI
            // rust_alloc: allocated 80 bytes at 0x166c7a9bd40
            // subimage: ETRLEObject { uiDataOffset: 0, uiDataLength: 960, sOffsetX: 0, sOffsetY: 0, usHeight: 30, usWidth: 30 }
            // subimage: ETRLEObject { uiDataOffset: 960, uiDataLength: 960, sOffsetX: 0, sOffsetY: 0, usHeight: 30, usWidth: 30 }
            // subimage: ETRLEObject { uiDataOffset: 1920, uiDataLength: 960, sOffsetX: 0, sOffsetY: 0, usHeight: 30, usWidth: 30 }
            // subimage: ETRLEObject { uiDataOffset: 2880, uiDataLength: 960, sOffsetX: 0, sOffsetY: 0, usHeight: 30, usWidth: 30 }
            // subimage: ETRLEObject { uiDataOffset: 3840, uiDataLength: 960, sOffsetX: 0, sOffsetY: 0, usHeight: 30, usWidth: 30 }
            // reading STCI image data, 4800 bytes
            // rust_alloc: allocated 4800 bytes at 0x166c7aa6eb0
        }
    }
}
