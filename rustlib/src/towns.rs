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

const NUM_TOWNS: usize = 12;

pub const ALL_TOWNS: [Town; NUM_TOWNS] = [
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

#[derive(Clone, Copy)]
pub struct Loyalty {
    pub rating: u8,
    pub change: i16,
    pub started: bool,
    pub liberated: bool,
}

impl Default for Loyalty {
    fn default() -> Self {
        Self::new()
    }
}

impl Loyalty {
    pub fn new() -> Self {
        Loyalty {
            rating: 0,
            change: 0,
            started: false,
            liberated: false,
        }
    }

    pub fn set_loyalty(&mut self, rating: u8) {
        self.rating = rating;
        self.change = 0;
        self.started = true;
    }

    // TODO: rustlib
    // // update town loyalty rating based on gain values
    // void UpdateTownLoyaltyRating(TownID bTownId) {
    //   // check gain value and update loyaty
    //   UINT8 ubOldLoyaltyRating = 0;
    //   INT16 sRatingChange = 0;
    //   UINT8 ubMaxLoyalty = 0;

    //   Assert((bTownId >= FIRST_TOWN) && (bTownId < NUM_TOWNS));

    //   // remember previous loyalty value
    //   ubOldLoyaltyRating = gTownLoyalty[bTownId].ubRating;

    //   sRatingChange = gTownLoyalty[bTownId].sChange / GAIN_PTS_PER_LOYALTY_PT;

    //   // if loyalty is ready to increase
    //   if (sRatingChange > 0) {
    //     // if the town is Omerta, and the rebels are/will become hostile
    //     if ((bTownId == OMERTA) && (GetCivGroupHostility(REBEL_CIV_GROUP) != CIV_GROUP_NEUTRAL)) {
    //       // maximum loyalty is much less than normal
    //       ubMaxLoyalty = HOSTILE_OMERTA_LOYALTY_RATING;
    //     } else {
    //       ubMaxLoyalty = MAX_LOYALTY_VALUE;
    //     }

    //     // check if we'd be going over the max
    //     if ((GetTownLoyaltyRating(bTownId) + sRatingChange) >= ubMaxLoyalty) {
    //       // set to max and null out gain pts
    //       gTownLoyalty[bTownId].ubRating = ubMaxLoyalty;
    //       gTownLoyalty[bTownId].sChange = 0;
    //     } else {
    //       // increment loyalty rating, reduce sChange
    //       gTownLoyalty[bTownId].ubRating += sRatingChange;
    //       gTownLoyalty[bTownId].sChange %= GAIN_PTS_PER_LOYALTY_PT;
    //     }
    //   } else
    //     // if loyalty is ready to decrease
    //     if (sRatingChange < 0) {
    //       // check if we'd be going below zero
    //       if ((GetTownLoyaltyRating(bTownId) + sRatingChange) < 0) {
    //         // set to zero and null out gain pts
    //         gTownLoyalty[bTownId].ubRating = 0;
    //         gTownLoyalty[bTownId].sChange = 0;
    //       } else {
    //         // decrement loyalty rating, reduce sChange
    //         gTownLoyalty[bTownId].ubRating += sRatingChange;
    //         gTownLoyalty[bTownId].sChange %= GAIN_PTS_PER_LOYALTY_PT;
    //       }
    //     }

    //   // check old aginst new, if diff, dirty map panel
    //   if (ubOldLoyaltyRating != GetTownLoyaltyRating(bTownId)) {
    //     SetMapPanelDirty(true);
    //   }

    //   return;
    // }
}

pub struct State {
    pub loyalty: [Loyalty; NUM_TOWNS],
}

impl Default for State {
    fn default() -> Self {
        Self::new()
    }
}

impl State {
    pub fn new() -> Self {
        State {
            loyalty: [Loyalty::new(); NUM_TOWNS],
        }
    }

    pub fn init_loyalty(&mut self) {
        for mut item in self.loyalty {
            item.rating = 0;
            item.change = 0;
            item.started = false;
            item.liberated = false;
        }
    }

    pub fn set_town_loyalty(&mut self, town: Town, rating: u8) {
        if town.does_use_loyalty() {
            self.loyalty[town as usize].set_loyalty(rating)
        }
    }
}

// on a scale of 1-100, this is a measure of how much each town hates the Queen's opression & is
// willing to stand against it it primarily controls the RATE of loyalty change in each town: the
// loyalty effect of the same events depends on it
const REBEL_SENTIMENT: [u8; NUM_TOWNS] = [
    90, // OMERTA	- They ARE the rebels!!!
    30, // DRASSEN	- Rebel friendly, makes it pretty easy to get first mine's income going at the start
    12, // ALMA	- Military town, high loyalty to Queen, need quests to get 100%
    15, // GRUMM - Close to Meduna, strong influence
    20, // TIXA - Not a real town
    15, // CAMBRIA, - Artificially much lower 'cause it's big and central and too easy to get loyalty up there
    20, // SAN_MONA - Neutral ground, loyalty doesn't vary
    20, // ESTONI,	- Not a real town
    20, // ORTA - Not a real town
    12, // BALIME,	- Rich town, high loyalty to Queen
    10, // MEDUNA,	- Enemy HQ, for God's sake!
    35, // CHITZENA, - Artificially high 'cause there's not enough fights near it to get the loyalty up otherwise
];

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
