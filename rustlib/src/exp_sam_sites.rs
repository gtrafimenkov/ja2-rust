use super::sam_sites;

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
