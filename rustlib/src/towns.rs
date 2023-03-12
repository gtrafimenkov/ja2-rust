use super::sector::Point;

/// Town IDs.
/// It is almost the same as exp_towns::TownID, but without BLANK_SECTOR.
#[derive(Copy, Clone, Debug, PartialEq)]
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

pub const ALL_TOWNS: [Town; 12] = [
    Town::Omerta,
    Town::Drassen,
    Town::Alma,
    Town::Grumm,
    Town::Tixa,
    Town::Cambria,
    Town::SanMona,
    Town::Estoni,
    Town::Orta,
    Town::Balime,
    Town::Meduna,
    Town::Chitzena,
];

/// TownMap implements mapping from sector to Town.
pub struct TownMap {
    towns: [[Option<Town>; 17]; 17],
}

impl Default for TownMap {
    fn default() -> Self {
        TownMap::new()
    }
}
impl TownMap {
    pub fn new() -> Self {
        let mut map = TownMap {
            towns: [[None; 17]; 17],
        };
        for town in ALL_TOWNS {
            for sector in town.get_sectors() {
                map.towns[sector.y as usize][sector.x as usize] = Some(town);
            }
        }
        map
    }

    /// Returns a town associated with the give sector
    pub const fn get(&self, x: u8, y: u8) -> Option<Town> {
        debug_assert!(x >= 1 && x <= 16);
        debug_assert!(y >= 1 && y <= 16);
        self.towns[y as usize][x as usize]
    }
}

const OMERTA_SECTORS: [Point; 2] = [Point::new(9, 1), Point::new(10, 1)];
const CHITZENA_SECTORS: [Point; 2] = [Point::new(2, 2), Point::new(2, 1)];
const SAN_MONA_SECTORS: [Point; 4] = [
    Point::new(5, 3),
    Point::new(6, 3),
    Point::new(5, 4),
    Point::new(4, 4),
];
const DRASSEN_SECTORS: [Point; 3] = [Point::new(13, 2), Point::new(13, 3), Point::new(13, 4)];
const GRUMM_SECTORS: [Point; 5] = [
    Point::new(1, 7),
    Point::new(1, 8),
    Point::new(2, 7),
    Point::new(2, 8),
    Point::new(3, 8),
];
const ESTONI_SECTORS: [Point; 1] = [Point::new(6, 9)];
const TIXA_SECTORS: [Point; 1] = [Point::new(9, 10)];
const CAMBRIA_SECTORS: [Point; 5] = [
    Point::new(8, 6),
    Point::new(9, 6),
    Point::new(8, 7),
    Point::new(9, 7),
    Point::new(8, 8),
];
const ALMA_SECTORS: [Point; 4] = [
    Point::new(13, 9),
    Point::new(14, 9),
    Point::new(13, 8),
    Point::new(14, 8),
];
const ORTA_SECTORS: [Point; 1] = [Point::new(4, 11)];
const BALIME_SECTORS: [Point; 2] = [Point::new(11, 12), Point::new(12, 12)];
const MEDUNA_SECTORS: [Point; 6] = [
    Point::new(3, 14),
    Point::new(4, 14),
    Point::new(5, 14),
    Point::new(3, 15),
    Point::new(4, 15),
    Point::new(3, 16),
];

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

    pub const fn does_use_loyalty(&self) -> bool {
        matches!(
            self,
            Town::Omerta
                | Town::Drassen
                | Town::Alma
                | Town::Grumm
                | Town::Cambria
                | Town::Balime
                | Town::Meduna
                | Town::Chitzena
        )
    }

    pub fn get_sectors(&self) -> core::slice::Iter<Point> {
        match self {
            Town::Omerta => OMERTA_SECTORS.iter(),
            Town::Chitzena => CHITZENA_SECTORS.iter(),
            Town::SanMona => SAN_MONA_SECTORS.iter(),
            Town::Drassen => DRASSEN_SECTORS.iter(),
            Town::Grumm => GRUMM_SECTORS.iter(),
            Town::Estoni => ESTONI_SECTORS.iter(),
            Town::Tixa => TIXA_SECTORS.iter(),
            Town::Cambria => CAMBRIA_SECTORS.iter(),
            Town::Alma => ALMA_SECTORS.iter(),
            Town::Orta => ORTA_SECTORS.iter(),
            Town::Balime => BALIME_SECTORS.iter(),
            Town::Meduna => MEDUNA_SECTORS.iter(),
        }
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

    #[test]
    fn omerta_sectors_count() {
        assert_eq!(2, Town::Omerta.get_sectors().len());
    }

    #[test]
    fn town_sectors() {
        let map = TownMap::new();

        assert_eq!(None, map.get(8, 1));
        assert_eq!(Some(Town::Omerta), map.get(9, 1));
        assert_eq!(Some(Town::Omerta), map.get(10, 1));
        assert_eq!(None, map.get(11, 1));
        assert_eq!(None, map.get(8, 2));
        assert_eq!(None, map.get(9, 2));
        assert_eq!(None, map.get(10, 2));
        assert_eq!(None, map.get(11, 2));

        assert_eq!(None, map.get(13, 1));
        assert_eq!(Some(Town::Drassen), map.get(13, 2));
        assert_eq!(Some(Town::Drassen), map.get(13, 3));
        assert_eq!(Some(Town::Drassen), map.get(13, 4));
        assert_eq!(None, map.get(13, 5));
    }

    #[test]
    fn does_use_loyalty() {
        assert_eq!(false, Town::Tixa.does_use_loyalty());
        assert_eq!(true, Town::Omerta.does_use_loyalty());
        assert_eq!(true, Town::Alma.does_use_loyalty());
    }
}
