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

/// Is militia training allowed in this town?
#[no_mangle]
pub extern "C" fn MilitiaTrainingAllowedInTown(town: TownID) -> bool {
    matches!(
        town,
        TownID::DRASSEN
            | TownID::ALMA
            | TownID::GRUMM
            | TownID::CAMBRIA
            | TownID::BALIME
            | TownID::MEDUNA
            | TownID::CHITZENA
    )
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
