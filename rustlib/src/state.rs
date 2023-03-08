use super::sam_sites;
use super::sector;

pub static mut STATE: State = State::new();

pub struct State {
    pub sam_sites: sam_sites::State,
    pub sectors: [[sector::State; 18]; 18],
}

impl State {
    pub const fn new() -> Self {
        State {
            sam_sites: sam_sites::State::new(),
            sectors: [[sector::State::new(); 18]; 18],
        }
    }

    pub fn get_sector(&self, x: u8, y: u8) -> &sector::State {
        &self.sectors[y as usize][x as usize]
    }

    pub fn get_mut_sector(&mut self, x: u8, y: u8) -> &mut sector::State {
        &mut self.sectors[y as usize][x as usize]
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn sector_enemy_controlled() {
        let mut state = State::new();

        assert_eq!(false, state.get_sector(1, 1).enemy_controlled);
        assert_eq!(false, state.get_sector(1, 1).enemy_air_controlled);

        state.get_mut_sector(1, 1).enemy_controlled = true;

        assert_eq!(true, state.get_sector(1, 1).enemy_controlled);
        assert_eq!(false, state.get_sector(1, 1).enemy_air_controlled);

        state.get_mut_sector(1, 1).enemy_air_controlled = true;

        assert_eq!(true, state.get_sector(1, 1).enemy_controlled);
        assert_eq!(true, state.get_sector(1, 1).enemy_air_controlled);
    }
}
