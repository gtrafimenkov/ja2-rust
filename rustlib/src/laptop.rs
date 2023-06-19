use crate::finances;

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

    pub fn get_balance_from_disk(&mut self) {
        let value = finances::read_balance_from_disk(finances::FINANCES_DATA_FILE).unwrap_or(0);
        self.current_balance = value;
    }
}
