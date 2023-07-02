use crate::exp_fileman;

use super::exp_alloc;
use super::exp_debug::debug_log_write;
use super::exp_fileman::FileID;
use super::exp_fileman::FILE_DB;
use byteorder::{LittleEndian, ReadBytesExt};
use std::ffi::{c_char, CStr};
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
pub struct Subimage {
    data_offset: u32,
    data_length: u32,
    x_offset: i16,
    y_offset: i16,
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

const STCI_RGB: u32 = 0x0004;
const STCI_INDEXED: u32 = 0x0008;
pub const STCI_ZLIB_COMPRESSED: u32 = 0x0010;
pub const STCI_ETRLE_COMPRESSED: u32 = 0x0020;

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
    subimages: *mut Subimage,
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
/// Load STI image from file or library.
/// If the function was successful, don't forget to free memory allocated for palette, subimages, app_data, image_data.
/// Memory must be freed with RustDealloc.
pub extern "C" fn LoadSTIImageFromFile(path: *const c_char, load_app_data: bool) -> STIImageLoaded {
    let failure = STIImageLoaded::default();
    match cstr_utf8_to_string(path) {
        None => failure,
        Some(path) => match unsafe { FILE_DB.open_for_reading(&path) } {
            Err(err) => {
                debug_log_write(&format!("failed to open file {path} for reading: {err}"));
                failure
            }
            Ok(file_id) => {
                let res = LoadSTIImage(file_id, load_app_data);
                exp_fileman::File_Close(file_id);
                res
            }
        },
    }
}

#[no_mangle]
/// Load STI image.
/// If the function was successful, don't forget to free memory allocated for palette, subimages, app_data, image_data.
/// Memory must be freed with RustDealloc.
pub extern "C" fn LoadSTIImage(file_id: FileID, load_app_data: bool) -> STIImageLoaded {
    let failure = STIImageLoaded::default();
    let img = read_stci(file_id, load_app_data);

    if let Err(err) = img {
        debug_log_write(&format!("failed to read stci image: {err}"));
        return failure;
    }

    let img = img.unwrap();

    // allocate memory and copy palette array
    let mut palette_data_copy: *mut SGPPaletteEntry = std::ptr::null_mut();
    if let Some(palette) = img.palette {
        let palette_data_size = palette.len() * std::mem::size_of::<Subimage>();
        unsafe {
            palette_data_copy = std::mem::transmute(exp_alloc::RustAlloc(palette_data_size));
            std::ptr::copy(palette.as_ptr(), palette_data_copy, palette.len());
        }
    }

    // allocate memory and copy image data
    let image_data_size = img.image_data.len();
    let image_data_copy = exp_alloc::RustAlloc(image_data_size);
    unsafe {
        std::ptr::copy(img.image_data.as_ptr(), image_data_copy, image_data_size);
    }

    // allocate memory for subimages array and copy data
    let mut number_of_subimages = 0;
    let mut subimages_ptr: *mut Subimage = std::ptr::null_mut();
    if let Some(subimages) = img.subimages {
        number_of_subimages = subimages.len();
        let data_size = number_of_subimages * std::mem::size_of::<Subimage>();
        unsafe {
            subimages_ptr = std::mem::transmute(exp_alloc::RustAlloc(data_size));
            std::ptr::copy(subimages.as_ptr(), subimages_ptr, number_of_subimages);
        }
    }

    // allocate memory and copy application data
    let mut app_data_size = 0;
    let mut app_data_copy: *mut u8 = std::ptr::null_mut();
    if let Some(app_data) = img.app_data {
        app_data_size = app_data.len();
        unsafe {
            app_data_copy = exp_alloc::RustAlloc(app_data_size);
            std::ptr::copy(app_data.as_ptr(), app_data_copy, app_data.len());
        }
    }

    STIImageLoaded {
        success: true,
        Height: img.height,
        Width: img.width,
        indexed: img.indexed,
        pixel_depth: img.pixel_depth,
        image_data_size: image_data_size as u32,
        image_data: image_data_copy,
        palette: palette_data_copy,
        number_of_subimages: number_of_subimages as u16,
        subimages: subimages_ptr,
        app_data_size: app_data_size as u32,
        app_data: app_data_copy,
        zlib_compressed: img.zlib_compressed,
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
    subimages: Option<Vec<Subimage>>,
    app_data: Option<Vec<u8>>,
    zlib_compressed: bool,
}

/// Read STCI image from an opened file.
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
        let mut subimages: Option<Vec<Subimage>> = None;
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
                    let subimage = Subimage {
                        data_offset: reader.read_u32::<LittleEndian>()?,
                        data_length: reader.read_u32::<LittleEndian>()?,
                        x_offset: reader.read_i16::<LittleEndian>()?,
                        y_offset: reader.read_i16::<LittleEndian>()?,
                        usHeight: reader.read_u16::<LittleEndian>()?,
                        usWidth: reader.read_u16::<LittleEndian>()?,
                    };
                    debug_log_write(&format!("subimage: {subimage:?}"));
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
        Ok(STImage {
            height,
            width,
            pixel_depth,
            image_data,
            indexed,
            palette,
            subimages,
            app_data,
            zlib_compressed,
        })
    }
}

#[cfg(test)]
mod tests {
    use super::super::exp_fileman::FILE_DB;

    use super::*;

    #[test]
    fn load_stci() {
        unsafe {
            FILE_DB.load_slf_from_dir("../tools/editor").unwrap();
            let file_id = FILE_DB.open_for_reading("Editor\\CANCEL.STI").unwrap();
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
            assert_eq!(0, subimage0.x_offset);
            assert_eq!(0, subimage0.y_offset);
            assert_eq!(30, subimage0.usHeight);
            assert_eq!(30, subimage0.usWidth);
            assert_eq!(0, subimage0.data_offset);
            assert_eq!(960, subimage0.data_length);
            assert_eq!(0, subimage4.x_offset);
            assert_eq!(0, subimage4.y_offset);
            assert_eq!(30, subimage4.usHeight);
            assert_eq!(30, subimage4.usWidth);
            assert_eq!(3840, subimage4.data_offset);
            assert_eq!(960, subimage4.data_length);
        }
    }
}
