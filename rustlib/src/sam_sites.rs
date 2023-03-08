use int_enum::IntEnum;

#[repr(u8)]
#[derive(IntEnum, Copy, Clone, PartialEq, Debug)]
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
    pub fn is_found(&self, site: SamSite) -> bool {
        return self.found[site as usize];
    }
    pub fn set_found(&mut self, site: SamSite, value: bool) {
        self.found[site as usize] = value
    }
}

const SAM_CONTROLLED_SECTORS: [[u8; 18]; 18] = [
    //  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16
    [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
    [0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 2, 2, 2, 2, 2, 2, 0], // A
    [0, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 0], // B
    [0, 1, 1, 1, 1, 1, 1, 1, 3, 2, 2, 2, 2, 2, 2, 2, 2, 0], // C
    [0, 1, 1, 1, 1, 1, 1, 1, 3, 3, 2, 2, 2, 2, 2, 2, 2, 0], // D
    [0, 1, 1, 1, 1, 1, 1, 3, 3, 3, 3, 2, 2, 2, 2, 2, 2, 0], // E
    [0, 1, 1, 1, 1, 1, 3, 3, 3, 3, 3, 3, 2, 2, 2, 2, 2, 0], // F
    [0, 1, 1, 1, 1, 3, 3, 3, 3, 3, 3, 3, 3, 2, 2, 2, 2, 0], // G
    [0, 1, 1, 1, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 2, 2, 0], // H
    [0, 1, 1, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 2, 2, 0], // I
    [0, 1, 4, 4, 4, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 2, 2, 0], // J
    [0, 4, 4, 4, 4, 4, 3, 3, 3, 3, 3, 3, 3, 3, 2, 2, 2, 0], // K
    [0, 4, 4, 4, 4, 4, 4, 3, 3, 3, 3, 3, 3, 3, 2, 2, 2, 0], // L
    [0, 4, 4, 4, 4, 4, 4, 4, 3, 3, 3, 3, 3, 3, 2, 2, 2, 0], // M
    [0, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 2, 2, 2, 0], // N
    [0, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 2, 2, 0], // O
    [0, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 2, 0], // P
    [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
];

pub fn get_sam_controlling_sector(x: u8, y: u8) -> Option<SamSite> {
    let index = SAM_CONTROLLED_SECTORS[y as usize][x as usize];
    match index {
        0 => None,
        _ => SamSite::from_int(index - 1).ok(),
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_get_sam_controlling_sector() {
        // assert_
        assert_eq!(SamSite::Drassen, get_sam_controlling_sector(15, 4).unwrap());
        assert!(get_sam_controlling_sector(0, 0).is_none()); // invaid coordinates
        assert!(get_sam_controlling_sector(9, 1).is_none()); // Omerta
    }
}
