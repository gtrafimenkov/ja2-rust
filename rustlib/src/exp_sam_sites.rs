use super::sam_sites;

#[repr(C)]
pub enum SamSite {
    SamSiteChitzena = 0,
    SamSiteDrassen = 1,
    SamSiteCambria = 2,
    SamSiteMeduna = 3,
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
