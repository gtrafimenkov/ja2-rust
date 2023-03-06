/// Sector ID 0-255 (16x16)
type SectorID8 = u8;

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
}
