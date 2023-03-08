use super::sam_sites;

pub const SAM_SITE_CHITZENA: usize = 0;
pub const SAM_SITE_DRASSEN: usize = 1;
pub const SAM_SITE_CAMBRIA: usize = 2;
pub const SAM_SITE_MEDUNA: usize = 3;

#[no_mangle]
/// Return total number of SAM sites
pub extern "C" fn GetSamSiteCount() -> u8 {
    return sam_sites::LOCATIONS.len() as u8;
}

#[no_mangle]
/// Return X location of i-th SAM site
pub extern "C" fn GetSamSiteX(i: u8) -> u8 {
    return sam_sites::LOCATIONS[i as usize].x;
}

#[no_mangle]
/// Return Y location of i-th SAM site
pub extern "C" fn GetSamSiteY(i: u8) -> u8 {
    return sam_sites::LOCATIONS[i as usize].y;
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn count_and_coords() {
        assert_eq!(4, GetSamSiteCount());
        assert_eq!(15, GetSamSiteX(1));
        assert_eq!(4, GetSamSiteY(1));
    }
}
