// use super::sector;

// pub const FOO: sector::SectorID8 = 100;

#[repr(C)]
pub struct SamSiteLocation {
    pub x: u8,
    pub y: u8,
}

#[no_mangle]
pub static SamSiteLocations: [SamSiteLocation; 4] = [
    SamSiteLocation { x: 2, y: 4 },
    SamSiteLocation { x: 15, y: 4 },
    SamSiteLocation { x: 8, y: 9 },
    SamSiteLocation { x: 4, y: 14 },
];

// #[no_mangle]
// pub static BAZ: [i32; 3] = [1, 2, 3];
