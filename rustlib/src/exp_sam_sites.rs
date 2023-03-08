use super::sam_sites;
use super::state;
use int_enum::IntEnum;

#[repr(C)]
pub enum SamSite {
    SamSiteChitzena = 0, /* sam_sites::SamSite::Chitzena as isize */
    SamSiteDrassen = 1,  /* sam_sites::SamSite::Drassen as isize*/
    SamSiteCambria = 2,  /* sam_sites::SamSite::Cambria as isize*/
    SamSiteMeduna = 3,   /* sam_sites::SamSite::Meduna as isize*/
}

#[no_mangle]
/// Return total number of SAM sites
pub extern "C" fn GetSamSiteCount() -> u8 {
    return sam_sites::LOCATIONS.len() as u8;
}

#[no_mangle]
/// Return X location of the SAM site
pub extern "C" fn GetSamSiteX(sam_site: SamSite) -> u8 {
    return sam_sites::LOCATIONS[sam_site as usize].x;
}

#[no_mangle]
/// Return Y location of the SAM site
pub extern "C" fn GetSamSiteY(sam_site: SamSite) -> u8 {
    return sam_sites::LOCATIONS[sam_site as usize].y;
}

#[no_mangle]
/// Check if the SAM site was found.
pub extern "C" fn IsSamSiteFound(sam_site: SamSite) -> bool {
    unsafe {
        let site = sam_sites::SamSite::from_int(sam_site as u8).unwrap();
        return state::SAM.is_found(site);
    }
}

#[no_mangle]
/// Set if the SAM site was found.
pub extern "C" fn SetSamSiteFound(sam_site: SamSite, value: bool) {
    unsafe {
        let site = sam_sites::SamSite::from_int(sam_site as u8).unwrap();
        return state::SAM.set_found(site, value);
    }
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
}
