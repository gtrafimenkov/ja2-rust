pub struct State {
    pub current_balance: i32,
}

impl Default for State {
    fn default() -> Self {
        Self::new()
    }
}

impl State {
    pub fn new() -> Self {
        State { current_balance: 0 }
    }
}
