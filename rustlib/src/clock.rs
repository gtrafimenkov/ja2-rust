pub struct State {
    pub game_clock_sec: u32, // current game time in seconds
    pub game_paused: bool,
    pub locked_pause: bool, // gfLockPauseState
}

pub const NUM_SEC_IN_MIN: u32 = 60;
pub const NUM_SEC_IN_HOUR: u32 = 60 * NUM_SEC_IN_MIN;
pub const NUM_SEC_IN_DAY: u32 = 24 * NUM_SEC_IN_HOUR;
pub const STARTING_TIME: u32 = NUM_SEC_IN_DAY + NUM_SEC_IN_HOUR; // Day 1, 01:00

impl Default for State {
    fn default() -> Self {
        Self::new()
    }
}

impl State {
    pub fn new() -> Self {
        State {
            game_clock_sec: STARTING_TIME,
            game_paused: true,
            locked_pause: false,
        }
    }

    pub fn move_time_forward(&mut self, seconds: u32) {
        self.game_clock_sec += seconds;
    }

    pub fn get_time_in_min(&self) -> u32 {
        self.game_clock_sec / NUM_SEC_IN_MIN
    }

    pub fn get_time_in_days(&self) -> u32 {
        self.game_clock_sec / NUM_SEC_IN_DAY
    }

    /// Get game clock hour - returns hour shown on the game clock (0 - 23)
    pub fn get_clock_hour(&self) -> u32 {
        (self.game_clock_sec % NUM_SEC_IN_DAY) / NUM_SEC_IN_HOUR
    }

    pub fn is_day_time(&self) -> bool {
        (7..21).contains(&self.get_clock_hour())
    }

    pub fn is_night_time(&self) -> bool {
        !self.is_day_time()
    }
}

#[cfg(test)]
mod tests {

    use super::*;

    #[test]
    fn default() {
        let clock = State::default();
        assert_eq!(1 * 60 * 60, clock.game_clock_sec);
        assert_eq!(true, clock.game_paused);
        assert_eq!(false, clock.locked_pause);
    }

    #[test]
    fn start_time() {
        let clock = State::default();
        assert_eq!(1 * 60 * 60, clock.game_clock_sec);
        assert_eq!(60, clock.get_time_in_min());
    }

    #[test]
    fn move_time_forward() {
        let mut clock = State::default();
        assert_eq!(1 * 60 * 60, clock.game_clock_sec);
        assert_eq!(60, clock.get_time_in_min());
        assert_eq!(0, clock.get_time_in_days());
        assert_eq!(1, clock.get_clock_hour());

        clock.move_time_forward(1);
        assert_eq!(3601, clock.game_clock_sec);
        assert_eq!(60, clock.get_time_in_min());

        clock.move_time_forward(59);
        assert_eq!(3660, clock.game_clock_sec);
        assert_eq!(61, clock.get_time_in_min());
        assert_eq!(1, clock.get_clock_hour());

        // move to 02:00
        clock.move_time_forward(59 * 60); // 59 minutes
        assert_eq!(7200, clock.game_clock_sec);
        assert_eq!(120, clock.get_time_in_min());
        assert_eq!(2, clock.get_clock_hour());
        assert_eq!(false, clock.is_day_time());
        assert_eq!(true, clock.is_night_time());

        // move to 07:00
        clock.move_time_forward(5 * 60 * 60);
        assert_eq!(true, clock.is_day_time());
        assert_eq!(false, clock.is_night_time());

        // move to 21:00
        clock.move_time_forward(14 * 60 * 60);
        assert_eq!(false, clock.is_day_time());
        assert_eq!(true, clock.is_night_time());
    }
}
