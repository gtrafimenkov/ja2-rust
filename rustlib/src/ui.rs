pub struct State {
    pub map_panel_dirty: bool,
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
        }
    }
}
