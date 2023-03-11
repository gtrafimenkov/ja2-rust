/// Town IDs.
/// It is almost the same as exp_towns::TownID, but without BLANK_SECTOR.
pub enum Town {
    Omerta = 1,
    Drassen = 2,
    Alma = 3,
    Grumm = 4,
    Tixa = 5,
    Cambria = 6,
    SanMona = 7,
    Estoni = 8,
    Orta = 9,
    Balime = 10,
    Meduna = 11,
    Chitzena = 12,
}

impl Town {
    pub const fn is_militia_training_allowed(&self) -> bool {
        matches!(
            self,
            Town::Drassen
                | Town::Alma
                | Town::Grumm
                | Town::Cambria
                | Town::Balime
                | Town::Meduna
                | Town::Chitzena
        )
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn militia_training_allowed() {
        assert_eq!(false, Town::Tixa.is_militia_training_allowed());
        assert_eq!(true, Town::Meduna.is_militia_training_allowed());
    }
}
