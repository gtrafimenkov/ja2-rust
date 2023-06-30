use super::exp_fileman::FileID;

pub const STCI_ID_LEN: usize = 4;

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

#[no_mangle]
pub extern "C" fn ReadSTCIHeader(file_id: FileID, data: &mut STCIHeaderHead) -> bool {
    false
}
