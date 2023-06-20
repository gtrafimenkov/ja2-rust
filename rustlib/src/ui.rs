pub struct State {
    pub map_panel_dirty: bool,
    pub map_screen_bottom_dirty: bool, // the dirty state of the mapscreen interface bottom
}

impl Default for State {
    fn default() -> Self {
        Self::new()
    }
}

impl State {
    pub fn new() -> Self {
        State {
            map_panel_dirty: true,
            map_screen_bottom_dirty: true,
        }
    }
}
