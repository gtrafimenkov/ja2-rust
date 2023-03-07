pub const MAP_WORLD_X: i16 = 18;
pub const MAP_WORLD_Y: i16 = 18;

/// Sector ID 0-255 (16x16)
type SectorID8 = u8;

/// Sector ID 0-324 (18x18)
type SectorID16 = i16;

/// Convert coordinates ([1-16], [1-16]) to 0-255 index.
/// This function should be prefered over GetSectorID8_STATIC macro.
#[no_mangle]
pub extern "C" fn GetSectorID8(x: u8, y: u8) -> SectorID8 {
    debug_assert!(x >= 1 && x <= 16);
    debug_assert!(y >= 1 && y <= 16);
    return (y - 1) * 16 + x - 1;
}

/// Get X [1-16] from SectorID8
#[no_mangle]
pub extern "C" fn SectorID8_X(sector_id: SectorID8) -> u8 {
    return sector_id % 16 + 1;
}

/// Get Y [1-16] from SectorID8
#[no_mangle]
pub extern "C" fn SectorID8_Y(sector_id: SectorID8) -> u8 {
    return sector_id / 16 + 1;
}

/// Convert coordinates (1-16, 1-16) to 0-324 index.
#[no_mangle]
pub extern "C" fn GetSectorID16(x: u8, y: u8) -> SectorID16 {
    return x as i16 + y as i16 * MAP_WORLD_X;
}

/// Get X [1-16] from SectorID16
#[no_mangle]
pub extern "C" fn SectorID16_X(sector_id: SectorID16) -> u8 {
    debug_assert!(sector_id >= 0 && sector_id < 18 * 18);
    return (sector_id % MAP_WORLD_X) as u8;
}

/// Get Y [1-16] from SectorID16
#[no_mangle]
pub extern "C" fn SectorID16_Y(sector_id: SectorID16) -> u8 {
    debug_assert!(sector_id >= 0 && sector_id < 18 * 18);
    return (sector_id / MAP_WORLD_X) as u8;
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn sector_id8_xy() {
        assert_eq!(0, GetSectorID8(1, 1));

        let sec0: SectorID8 = 0;
        assert_eq!(1, SectorID8_X(sec0));
        assert_eq!(1, SectorID8_Y(sec0));
    }

    #[test]
    fn sector_id16_xy() {
        assert_eq!(19, GetSectorID16(1, 1));

        let sec0: SectorID16 = 19;
        assert_eq!(1, SectorID16_X(sec0));
        assert_eq!(1, SectorID16_Y(sec0));
    }
}
