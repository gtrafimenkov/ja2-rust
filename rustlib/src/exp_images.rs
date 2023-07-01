use super::exp_debug;
use super::exp_fileman::FileID;
use super::exp_fileman::FILE_DB;
use std::io;

pub const STCI_ID_LEN: usize = 4;

#[repr(C)]
#[allow(non_snake_case)]
// #[derive(Default)]
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
#[derive(Default)]
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
    RGB(STCIHeaderMiddleRGB),
    Indexed(STCIHeaderMiddleIndexed),
}

#[repr(C)]
#[allow(non_snake_case)]
#[derive(Default, Copy, Clone)]
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
#[derive(Default, Copy, Clone)]
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
#[derive(Default)]
/// Last part of STCI image header
pub struct STCIHeaderEnd {
    Depth: u8, // size in bits of one pixel as stored in the file
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
        } else if head.Flags & STCI_RGB != 0 {
            middle = STCIHeaderMiddle::RGB(STCIHeaderMiddleRGB {
                uiRedMask: FILE_DB.read_file_u32(file_id)?,
                uiGreenMask: FILE_DB.read_file_u32(file_id)?,
                uiBlueMask: FILE_DB.read_file_u32(file_id)?,
                uiAlphaMask: FILE_DB.read_file_u32(file_id)?,
                ubRedDepth: FILE_DB.read_file_u8(file_id)?,
                ubGreenDepth: FILE_DB.read_file_u8(file_id)?,
                ubBlueDepth: FILE_DB.read_file_u8(file_id)?,
                ubAlphaDepth: FILE_DB.read_file_u8(file_id)?,
            });
        } else {
            return Err(io::Error::new(io::ErrorKind::Other, "unknown image format"));
        }

        let mut end = STCIHeaderEnd {
            Depth: FILE_DB.read_file_u8(file_id)?,
            AppDataSize: FILE_DB.read_file_u32(file_id)?,
            ..Default::default()
        };
        FILE_DB.read_file_exact(file_id, &mut end.Unused)?;

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
