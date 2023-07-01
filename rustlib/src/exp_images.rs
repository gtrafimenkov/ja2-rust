use super::exp_debug;
use super::exp_fileman::FileID;
use super::exp_fileman::FILE_DB;
use std::io;

/*
Sir-Tech's Crazy Image (STCI) file format specifications.  Each file is composed of:
- ImageFileHeader, uncompressed
- Palette (STCI_INDEXED, size = uiNumberOfColours * PALETTE_ELEMENT_SIZE), uncompressed
- SubRectInfo's (usNumberOfRects > 0, size = usNumberOfSubRects * sizeof(SubRectInfo)), uncompressed
- Bytes of image data, possibly compressed
*/

const STCI_ID_LEN: usize = 4;

const STCI_ID_STRING: [u8; 4] = [b'S', b'T', b'C', b'I'];

#[repr(C)]
#[allow(non_snake_case)]
/// Last part of STCI image header
pub struct STCIHeaderTmp {
    head: STCIHeaderHead,
    middle: STCIHeaderMiddle,
    end: STCIHeaderEnd,
}

const STCI_INDEXED: u32 = 0x0008;
const STCI_RGB: u32 = 0x0004;

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

fn read_stci_header(file_id: FileID) -> io::Result<STCIHeaderTmp> {
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

        Ok(STCIHeaderTmp { head, middle, end })
    }
}

#[no_mangle]
pub extern "C" fn ReadSTCIHeader(file_id: FileID, data: &mut STCIHeaderTmp) -> bool {
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
