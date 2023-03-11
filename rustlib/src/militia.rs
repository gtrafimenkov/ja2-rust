/// Militia force in a sector.
pub struct Force {
    pub green: u8,
    pub regular: u8,
    pub elite: u8,
}

impl Default for Force {
    fn default() -> Self {
        Self::new()
    }
}

impl Force {
    pub const fn new() -> Self {
        Force {
            green: 0,
            regular: 0,
            elite: 0,
        }
    }
}
