use crate::towns;

#[repr(C)]
#[allow(non_camel_case_types)]
pub enum TownID {
    BLANK_SECTOR = 0,
    OMERTA = 1,
    DRASSEN = 2,
    ALMA = 3,
    GRUMM = 4,
    TIXA = 5,
    CAMBRIA = 6,
    SAN_MONA = 7,
    ESTONI = 8,
    ORTA = 9,
    BALIME = 10,
    MEDUNA = 11,
    CHITZENA = 12,
}

impl TownID {
    #[allow(dead_code)]
    fn from_internal(value: towns::Town) -> Self {
        match value {
            towns::Town::Omerta => TownID::OMERTA,
            towns::Town::Drassen => TownID::DRASSEN,
            towns::Town::Alma => TownID::ALMA,
            towns::Town::Grumm => TownID::GRUMM,
            towns::Town::Tixa => TownID::TIXA,
            towns::Town::Cambria => TownID::CAMBRIA,
            towns::Town::SanMona => TownID::SAN_MONA,
            towns::Town::Estoni => TownID::ESTONI,
            towns::Town::Orta => TownID::ORTA,
            towns::Town::Balime => TownID::BALIME,
            towns::Town::Meduna => TownID::MEDUNA,
            towns::Town::Chitzena => TownID::CHITZENA,
        }
    }
    const fn to_internal(&self) -> towns::Town {
        match self {
            TownID::OMERTA => towns::Town::Omerta,
            TownID::DRASSEN => towns::Town::Drassen,
            TownID::ALMA => towns::Town::Alma,
            TownID::GRUMM => towns::Town::Grumm,
            TownID::TIXA => towns::Town::Tixa,
            TownID::CAMBRIA => towns::Town::Cambria,
            TownID::SAN_MONA => towns::Town::SanMona,
            TownID::ESTONI => towns::Town::Estoni,
            TownID::ORTA => towns::Town::Orta,
            TownID::BALIME => towns::Town::Balime,
            TownID::MEDUNA => towns::Town::Meduna,
            TownID::CHITZENA => towns::Town::Chitzena,
            TownID::BLANK_SECTOR => {
                // this inconsistency should not leak to the proper code
                panic!("an attempt to convert BLANK_SECTOR to a proper town");
            }
        }
    }
}

/// Is militia training allowed in this town?
#[no_mangle]
pub extern "C" fn MilitiaTrainingAllowedInTown(town: TownID) -> bool {
    match town {
        TownID::BLANK_SECTOR => false,
        _ => town.to_internal().is_militia_training_allowed(),
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn militia_training_allowed() {
        assert_eq!(false, MilitiaTrainingAllowedInTown(TownID::BLANK_SECTOR));
        assert_eq!(false, MilitiaTrainingAllowedInTown(TownID::TIXA));

        assert_eq!(true, MilitiaTrainingAllowedInTown(TownID::MEDUNA));
    }
}
