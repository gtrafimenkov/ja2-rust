pub struct SamSiteLocation {
    pub x: u8,
    pub y: u8,
}

pub const LOCATIONS: [SamSiteLocation; 4] = [
    SamSiteLocation { x: 2, y: 4 },
    SamSiteLocation { x: 15, y: 4 },
    SamSiteLocation { x: 8, y: 9 },
    SamSiteLocation { x: 4, y: 14 },
];
