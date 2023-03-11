use super::state::STATE;
use crate::militia;

#[repr(C)]
#[allow(non_camel_case_types)]
pub enum MilitiaRank {
    GREEN_MILITIA = 0,
    REGULAR_MILITIA,
    ELITE_MILITIA,
}

pub const MAX_MILITIA_LEVELS: u8 = 3;

#[repr(C)]
#[allow(non_snake_case)]
/// Militia force in a sector.
pub struct MilitiaCount {
    pub green: u8,
    pub regular: u8,
    pub elite: u8,
}

impl MilitiaCount {
    // const fn to_internal(&self) -> militia::Force {
    //     militia::Force {
    //         green: self.green,
    //         regular: self.regular,
    //         elite: self.elite,
    //     }
    // }
    const fn from_internal(value: &militia::Force) -> Self {
        MilitiaCount {
            green: value.green,
            regular: value.regular,
            elite: value.elite,
        }
    }
}

#[no_mangle]
/// Return militia force in a sector.
pub extern "C" fn GetMilitiaInSector(x: u8, y: u8) -> MilitiaCount {
    MilitiaCount::from_internal(unsafe { STATE.get_militia_force(x, y) })
}

#[no_mangle]
/// Set militia force in a sector.
pub extern "C" fn SetMilitiaInSector(x: u8, y: u8, value: MilitiaCount) {
    let force = unsafe { STATE.get_mut_militia_force(x, y) };
    force.green = value.green;
    force.regular = value.regular;
    force.elite = value.elite;
}

#[no_mangle]
pub extern "C" fn GetMilitiaOfRankInSector(x: u8, y: u8, rank: MilitiaRank) -> u8 {
    let mil = unsafe { STATE.get_militia_force(x, y) };
    match rank {
        MilitiaRank::GREEN_MILITIA => mil.green,
        MilitiaRank::REGULAR_MILITIA => mil.regular,
        MilitiaRank::ELITE_MILITIA => mil.elite,
    }
}

#[no_mangle]
pub extern "C" fn CountMilitiaInSector(x: u8, y: u8) -> u8 {
    let mil = unsafe { STATE.get_militia_force(x, y) };
    mil.count()
}

#[no_mangle]
pub extern "C" fn SetMilitiaOfRankInSector(x: u8, y: u8, rank: MilitiaRank, value: u8) {
    let mut mil = unsafe { STATE.get_mut_militia_force(x, y) };
    match rank {
        MilitiaRank::GREEN_MILITIA => mil.green = value,
        MilitiaRank::REGULAR_MILITIA => mil.regular = value,
        MilitiaRank::ELITE_MILITIA => mil.elite = value,
    }
}

#[no_mangle]
pub extern "C" fn IncMilitiaOfRankInSector(x: u8, y: u8, rank: MilitiaRank, increase: u8) {
    let mut mil = unsafe { STATE.get_mut_militia_force(x, y) };
    match rank {
        MilitiaRank::GREEN_MILITIA => mil.green += increase,
        MilitiaRank::REGULAR_MILITIA => mil.regular += increase,
        MilitiaRank::ELITE_MILITIA => mil.elite += increase,
    }
}

// UINT8 CountMilitiaInSector(u8 mapX, u8 mapY) {
//     struct MilitiaCount milCount = GetMilitiaInSector(mapX, mapY);
//     return milCount.green + milCount.regular + milCount.elite;
//   }
