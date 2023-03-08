use super::sam_sites;

pub static mut STATE: State = State::new();

pub struct State {
    pub sam_sites: sam_sites::State,
}

impl State {
    pub const fn new() -> Self {
        State {
            sam_sites: sam_sites::State::new(),
        }
    }
}
