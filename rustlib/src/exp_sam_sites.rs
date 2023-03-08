use super::sam_sites;
use super::state;
use int_enum::IntEnum;

#[repr(C)]
#[derive(Copy, Clone)]
pub enum SamSite {
    SamSiteChitzena = 0, /* sam_sites::SamSite::Chitzena as isize */
    SamSiteDrassen = 1,  /* sam_sites::SamSite::Drassen as isize*/
    SamSiteCambria = 2,  /* sam_sites::SamSite::Cambria as isize*/
    SamSiteMeduna = 3,   /* sam_sites::SamSite::Meduna as isize*/
}

const SITES: [SamSite; 4] = [
    SamSite::SamSiteChitzena,
    SamSite::SamSiteDrassen,
    SamSite::SamSiteCambria,
    SamSite::SamSiteMeduna,
];

#[no_mangle]
/// Return total number of SAM sites
pub extern "C" fn GetSamSiteCount() -> u8 {
    return sam_sites::LOCATIONS.len() as u8;
}

#[no_mangle]
/// Return X location of the SAM site
pub extern "C" fn GetSamSiteX(site: SamSite) -> u8 {
    return sam_sites::LOCATIONS[site as usize].x;
}

#[no_mangle]
/// Return Y location of the SAM site
pub extern "C" fn GetSamSiteY(site: SamSite) -> u8 {
    return sam_sites::LOCATIONS[site as usize].y;
}

#[no_mangle]
/// Check if the SAM site was found.
pub extern "C" fn IsSamSiteFound(site: SamSite) -> bool {
    unsafe {
        let site = sam_sites::SamSite::from_int(site as u8).unwrap();
        return state::SAM.is_found(site);
    }
}

#[no_mangle]
/// Set if the SAM site was found.
pub extern "C" fn SetSamSiteFound(site: SamSite, value: bool) {
    unsafe {
        let site = sam_sites::SamSite::from_int(site as u8).unwrap();
        return state::SAM.set_found(site, value);
    }
}

#[no_mangle]
pub extern "C" fn GetSamGraphicsID(site: SamSite) -> u8 {
    // ATE: Update this w/ graphic used
    // Use 3 if / orientation, 4 if \ orientation
    const GRAPHICS_LIST: [u8; 4] = [4, 3, 3, 3];
    GRAPHICS_LIST[site as usize]
}

#[no_mangle]
pub extern "C" fn GetSamGridNoA(site: SamSite) -> i16 {
    const GRID_NO_A: [i16; 4] = [10196, 11295, 16080, 11913];
    GRID_NO_A[site as usize]
}

#[no_mangle]
pub extern "C" fn GetSamGridNoB(site: SamSite) -> i16 {
    const GRID_NO_B: [i16; 4] = [10195, 11135, 15920, 11912];
    GRID_NO_B[site as usize]
}

#[no_mangle]
pub extern "C" fn DoesSAMExistHere(sector_x: u8, sector_y: u8, sector_z: i8, grid_no: i16) -> bool {
    if sector_z != 0 {
        return false;
    }

    for site in SITES.iter() {
        if GetSamSiteX(*site) == sector_x && GetSamSiteY(*site) == sector_y {
            if GetSamGridNoA(*site) == grid_no || GetSamGridNoB(*site) == grid_no {
                return true;
            }
        }
    }

    false
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn count_and_coords() {
        assert_eq!(4, GetSamSiteCount());
        assert_eq!(15, GetSamSiteX(SamSite::SamSiteDrassen));
        assert_eq!(4, GetSamSiteY(SamSite::SamSiteDrassen));
    }

    #[test]
    fn sam_existance() {
        assert_eq!(false, DoesSAMExistHere(1, 1, 0, 1));

        assert_eq!(true, DoesSAMExistHere(15, 4, 0, 11295));
        assert_eq!(true, DoesSAMExistHere(15, 4, 0, 11135));
        assert_eq!(false, DoesSAMExistHere(15, 4, 1, 11135));
    }
}
