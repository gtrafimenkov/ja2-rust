use super::state::STATE;

/// Handle game restart.
#[no_mangle]
pub extern "C" fn PrepareGameRestart() {
    unsafe { STATE.clock.game_paused = true }
}
