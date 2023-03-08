use int_enum::IntEnum;

#[repr(u8)]
#[derive(IntEnum, Copy, Clone)]
pub enum SamSite {
    Chitzena = 0,
    Drassen = 1,
    Cambria = 2,
    Meduna = 3,
}

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

pub struct State {
    found: [bool; 4],
}

impl State {
    pub const fn new() -> Self {
        State { found: [false; 4] }
    }
    pub fn is_found(&self, sam_site: SamSite) -> bool {
        return self.found[sam_site as usize];
    }
    pub fn set_found(&mut self, sam_site: SamSite, value: bool) {
        self.found[sam_site as usize] = value
    }
}
