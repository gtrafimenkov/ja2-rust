use super::state;

/// Handle game restart.
#[no_mangle]
pub extern "C" fn PrepareGameRestart() {
    state::get().clock.game_paused = true
}
