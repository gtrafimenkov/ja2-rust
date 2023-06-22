use super::clock;
use super::exp_fileman::FileID;
use super::exp_fileman::FILE_DB;
use super::exp_ui;
use super::state::STATE;
use std::io;

#[repr(C)]
#[allow(non_snake_case)]
#[derive(Default)]
pub struct SavedClockState {
    TimeCompressMode: i32,
    ClockResolution: u8,
    gamePaused: bool,
    TimeInterrupt: bool,
    SuperCompression: bool,
    gameClock: u32,
    GameSecondsPerRealSecond: u32,
    AmbientLightLevel: u8,
    EnvTime: u32,
    EnvDay: u32,
    EnvLightValue: u8,
    TimeOfLastEventQuery: u32,
    lockedPause: bool,
    PauseDueToPlayerGamePause: bool,
    ResetAllPlayerKnowsEnemiesFlags: bool,
    TimeCompressionOn: bool,
    PreviousGameClock: u32,
    LockPauseStateLastReasonId: u32,
    UnusedTimePadding: [u8; 20],
}

#[no_mangle]
pub extern "C" fn LoadSavedClockState(file_id: FileID, data: &mut SavedClockState) -> bool {
    match read_saved_clock_state(file_id) {
        Ok(res) => {
            if res.gamePaused {
                PauseGame();
            }
            SetGameTimeSec(res.gameClock);
            if res.lockedPause {
                LockPause();
            }
            *data = res;
            true
        }
        Err(_) => false,
    }
}

fn read_saved_clock_state(file_id: FileID) -> io::Result<SavedClockState> {
    let mut data = SavedClockState::default();
    unsafe {
        data.TimeCompressMode = FILE_DB.read_file_i32(file_id)?;
        data.ClockResolution = FILE_DB.read_file_u8(file_id)?;
        data.gamePaused = FILE_DB.read_file_bool(file_id)?;
        data.TimeInterrupt = FILE_DB.read_file_bool(file_id)?;
        data.SuperCompression = FILE_DB.read_file_bool(file_id)?;
        data.gameClock = FILE_DB.read_file_u32(file_id)?;
        data.GameSecondsPerRealSecond = FILE_DB.read_file_u32(file_id)?;
        data.AmbientLightLevel = FILE_DB.read_file_u8(file_id)?;
        data.EnvTime = FILE_DB.read_file_u32(file_id)?;
        data.EnvDay = FILE_DB.read_file_u32(file_id)?;
        data.EnvLightValue = FILE_DB.read_file_u8(file_id)?;
        data.TimeOfLastEventQuery = FILE_DB.read_file_u32(file_id)?;
        data.lockedPause = FILE_DB.read_file_bool(file_id)?;
        data.PauseDueToPlayerGamePause = FILE_DB.read_file_bool(file_id)?;
        data.ResetAllPlayerKnowsEnemiesFlags = FILE_DB.read_file_bool(file_id)?;
        data.TimeCompressionOn = FILE_DB.read_file_bool(file_id)?;
        data.PreviousGameClock = FILE_DB.read_file_u32(file_id)?;
        data.LockPauseStateLastReasonId = FILE_DB.read_file_u32(file_id)?;
        FILE_DB.read_file_exact(file_id, &mut data.UnusedTimePadding)?;
    }
    Ok(data)
}

// #[repr(C)]
// pub enum SavedClockStateOrError {
//     Data(SavedClockState),
//     Error,
// }

// #[no_mangle]
// pub extern "C" fn LoadSavedClockState2(file_id: FileID) -> SavedClockStateOrError {
//     fn read(file_id: FileID) -> io::Result<SavedClockState> {
//         let mut data = SavedClockState::default();
//         unsafe {
//             data.TimeCompressMode = FILE_DB.read_file_i32(file_id)?;
//             data.ClockResolution = FILE_DB.read_file_u8(file_id)?;
//             data.gamePaused = FILE_DB.read_file_bool(file_id)?;
//             data.TimeInterrupt = FILE_DB.read_file_bool(file_id)?;
//             data.SuperCompression = FILE_DB.read_file_bool(file_id)?;
//             data.gameClock = FILE_DB.read_file_u32(file_id)?;
//             data.GameSecondsPerRealSecond = FILE_DB.read_file_u32(file_id)?;
//             data.AmbientLightLevel = FILE_DB.read_file_u8(file_id)?;
//             data.EnvTime = FILE_DB.read_file_u32(file_id)?;
//             data.EnvDay = FILE_DB.read_file_u32(file_id)?;
//             data.EnvLightValue = FILE_DB.read_file_u8(file_id)?;
//             data.TimeOfLastEventQuery = FILE_DB.read_file_u32(file_id)?;
//             data.lockedPause = FILE_DB.read_file_bool(file_id)?;
//             data.PauseDueToPlayerGamePause = FILE_DB.read_file_bool(file_id)?;
//             data.ResetAllPlayerKnowsEnemiesFlags = FILE_DB.read_file_bool(file_id)?;
//             data.TimeCompressionOn = FILE_DB.read_file_bool(file_id)?;
//             data.PreviousGameClock = FILE_DB.read_file_u32(file_id)?;
//             data.LockPauseStateLastReasonId = FILE_DB.read_file_u32(file_id)?;
//             data.UnusedTimePadding = FILE_DB.read_file_u8(file_id)?;
//         }
//         Ok(data)
//     }
//     match read(file_id) {
//         Ok(res) => SavedClockStateOrError::Data(res),
//         Err(_) => SavedClockStateOrError::Error,
//     }
// }

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
