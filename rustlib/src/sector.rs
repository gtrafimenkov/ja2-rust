pub const MAP_WORLD_X: i16 = 18;
pub const MAP_WORLD_Y: i16 = 18;

pub struct Point {
    pub x: u8,
    pub y: u8,
}

impl Point {
    pub const fn new(x: u8, y: u8) -> Self {
        debug_assert!(x >= 1 && x <= 16);
        debug_assert!(y >= 1 && y <= 16);
        Point { x, y }
    }
}

#[derive(Copy, Clone)]
pub struct State {
    pub enemy_controlled: bool,
    pub enemy_air_controlled: bool,
}

impl State {
    pub const fn new() -> Self {
        Self {
            enemy_controlled: false,
            enemy_air_controlled: false,
        }
    }
}
