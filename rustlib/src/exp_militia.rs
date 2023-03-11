use super::state::STATE;
use crate::militia;

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
