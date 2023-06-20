use super::clock;
use super::exp_ui;
use super::state::STATE;

/// Get game starting time in seconds.
#[no_mangle]
pub extern "C" fn GetGameStartingTime() -> u32 {
    clock::STARTING_TIME
}

/// Get game time in seconds
#[no_mangle]
pub extern "C" fn GetGameTimeInSec() -> u32 {
    unsafe { STATE.clock.game_clock_sec }
}

/// Set game time in seconds
#[no_mangle]
pub extern "C" fn SetGameTimeSec(value: u32) {
    unsafe {
        STATE.clock.game_clock_sec = value;
    }
}

/// Adjust game time forward by given number of seconds.
#[no_mangle]
pub extern "C" fn MoveGameTimeForward(seconds: u32) {
    unsafe {
        STATE.clock.move_time_forward(seconds);
    }
}

/// Get game time in minutes
#[no_mangle]
pub extern "C" fn GetGameTimeInMin() -> u32 {
    unsafe { STATE.clock.get_time_in_min() }
}

/// Get game time in days
#[no_mangle]
pub extern "C" fn GetGameTimeInDays() -> u32 {
    unsafe { STATE.clock.get_time_in_days() }
}

/// Get game clock hour - returns hour shown on the game clock (0 - 23)
#[no_mangle]
pub extern "C" fn GetGameClockHour() -> u32 {
    unsafe { STATE.clock.get_clock_hour() }
}

/// Get game clock minutes - returns minutes shown on the game clock (0 - 59)
#[no_mangle]
pub extern "C" fn GetGameClockMinutes() -> u32 {
    (GetGameTimeInSec() % clock::NUM_SEC_IN_HOUR) / clock::NUM_SEC_IN_MIN
}

/// Get number of minutes passed since game day start
#[no_mangle]
pub extern "C" fn GetMinutesSinceDayStart() -> u32 {
    (GetGameTimeInSec() % clock::NUM_SEC_IN_DAY) / clock::NUM_SEC_IN_MIN
}

/// Get time of future midnight in minutes
#[no_mangle]
pub extern "C" fn GetFutureMidnightInMinutes(days: u32) -> u32 {
    (GetGameTimeInDays() + days) * 24 * 60
}

#[no_mangle]
pub extern "C" fn IsDayTime() -> bool {
    unsafe { STATE.clock.is_day_time() }
}

#[no_mangle]
pub extern "C" fn IsNightTime() -> bool {
    unsafe { STATE.clock.is_night_time() }
}

#[no_mangle]
pub extern "C" fn PauseGame() {
    if !IsGamePaused() {
        unsafe {
            STATE.clock.game_paused = true;
        }
        exp_ui::SetMapScreenBottomDirty(true);
    }
}

#[no_mangle]
pub extern "C" fn UnPauseGame() {
    if IsGamePaused() && !IsPauseLocked() {
        unsafe {
            STATE.clock.game_paused = false;
        }
        exp_ui::SetMapScreenBottomDirty(true);
    }
}

#[no_mangle]
pub extern "C" fn TogglePause() {
    if IsGamePaused() {
        UnPauseGame();
    } else {
        PauseGame();
    }
}

#[no_mangle]
pub extern "C" fn IsGamePaused() -> bool {
    unsafe { STATE.clock.game_paused }
}

#[no_mangle]
pub extern "C" fn LockPause() {
    unsafe {
        STATE.clock.locked_pause = true;
    }
}

#[no_mangle]
pub extern "C" fn UnlockPause() {
    unsafe {
        STATE.clock.locked_pause = false;
    }
}

#[no_mangle]
pub extern "C" fn IsPauseLocked() -> bool {
    unsafe { STATE.clock.locked_pause }
}
