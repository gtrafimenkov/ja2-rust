use super::sam_sites;
use super::sector;
use super::towns;
use once_cell::sync::Lazy;

pub static mut STATE: Lazy<State> = Lazy::new(State::new);

pub struct State {
    pub sam_sites: sam_sites::State,
    pub sectors: [[sector::State; 18]; 18],
    pub town_map: towns::TownMap,
}

impl Default for State {
    fn default() -> Self {
        Self::new()
    }
}

impl State {
    pub fn new() -> Self {
        let mut state = State {
            sam_sites: sam_sites::State::new(),
            sectors: [[sector::State::new(); 18]; 18],
            town_map: Default::default(),
        };
        for y in 1..17 {
            for x in 1..17 {
                state.sectors[y][x].enemy_controlled = true;
            }
        }
        state
    }

    pub fn get_sector(&self, x: u8, y: u8) -> &sector::State {
        &self.sectors[y as usize][x as usize]
    }

    pub fn get_mut_sector(&mut self, x: u8, y: u8) -> &mut sector::State {
        &mut self.sectors[y as usize][x as usize]
    }

    pub fn get_number_of_sam_under_player_control(&self) -> u8 {
        let mut counter = 0;
        for loc in sam_sites::LOCATIONS.iter() {
            if !self.get_sector(loc.x, loc.y).enemy_controlled {
                counter += 1;
            }
        }
        counter
    }

    /// Update airspace control map based on which SAM sectors are controlled by the enemy.
    pub fn update_airspace_control_map(&mut self) {
        for y in 0..17 {
            for x in 0..17 {
                let sam = sam_sites::get_sam_controlling_sector(x, y);
                self.get_mut_sector(x, y).enemy_air_controlled = match sam {
                    None => false,
                    Some(sam) => {
                        self.get_sector(
                            sam_sites::get_sam_location(sam).x,
                            sam_sites::get_sam_location(sam).y,
                        )
                        .enemy_controlled
                    }
                }
            }
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn sector_enemy_controlled() {
        let mut state = State::new();

        assert_eq!(true, state.get_sector(1, 1).enemy_controlled);
        assert_eq!(false, state.get_sector(1, 1).enemy_air_controlled);

        state.get_mut_sector(1, 1).enemy_controlled = true;

        assert_eq!(true, state.get_sector(1, 1).enemy_controlled);
        assert_eq!(false, state.get_sector(1, 1).enemy_air_controlled);

        state.get_mut_sector(1, 1).enemy_air_controlled = true;

        assert_eq!(true, state.get_sector(1, 1).enemy_controlled);
        assert_eq!(true, state.get_sector(1, 1).enemy_air_controlled);
    }

    #[test]
    fn sam_controlled_by_player() {
        let mut st = State::new();

        assert_eq!(0, st.get_number_of_sam_under_player_control());

        let loc = sam_sites::get_sam_location(sam_sites::SamSite::Cambria);
        st.get_mut_sector(loc.x, loc.y).enemy_controlled = false;
        assert_eq!(1, st.get_number_of_sam_under_player_control());
    }
}
