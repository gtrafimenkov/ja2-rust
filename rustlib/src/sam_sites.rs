// use super::sector;

// pub const FOO: sector::SectorID8 = 100;

// SAM sites
pub const SAM_1_X: u8 = 2;
pub const SAM_2_X: u8 = 15;
pub const SAM_3_X: u8 = 8;
pub const SAM_4_X: u8 = 4;

pub const SAM_1_Y: u8 = 4;
pub const SAM_2_Y: u8 = 4;
pub const SAM_3_Y: u8 = 9;
pub const SAM_4_Y: u8 = 14;

#[repr(C)]
pub struct SamSiteLocation {
    pub x: u8,
    pub y: u8,
}

#[no_mangle]
pub static SamSiteLocations: [SamSiteLocation; 4] = [
    SamSiteLocation {
        x: SAM_1_X,
        y: SAM_1_Y,
    },
    SamSiteLocation {
        x: SAM_2_X,
        y: SAM_2_Y,
    },
    SamSiteLocation {
        x: SAM_3_X,
        y: SAM_3_Y,
    },
    SamSiteLocation {
        x: SAM_4_X,
        y: SAM_4_Y,
    },
];

// #[no_mangle]
// pub static BAZ: [i32; 3] = [1, 2, 3];
