use super::clock;
use super::exp_fileman::FileID;
use super::exp_fileman::FILE_DB;
use super::exp_ui;
use super::state::STATE;
use std::io;

#[repr(C)]
#[allow(non_camel_case_types)]
pub enum TIME_COMPRESS_MODE {
    TIME_COMPRESS_X0 = 0, // time pause
    TIME_COMPRESS_X1,     // ? tactical mode
    TIME_COMPRESS_5MINS,  // ? strategic mode 5 min compression
    TIME_COMPRESS_30MINS, // ? strategic mode 30 min compression
    TIME_COMPRESS_60MINS, // ? strategic mode 60 min compression
}

impl TIME_COMPRESS_MODE {
    fn from_internal(value: clock::TimeCompressMode) -> Self {
        match value {
            clock::TimeCompressMode::TimeCompressX0 => TIME_COMPRESS_MODE::TIME_COMPRESS_X0,
            clock::TimeCompressMode::TimeCompressX1 => TIME_COMPRESS_MODE::TIME_COMPRESS_X1,
            clock::TimeCompressMode::TimeCompress5mins => TIME_COMPRESS_MODE::TIME_COMPRESS_5MINS,
            clock::TimeCompressMode::TimeCompress30mins => TIME_COMPRESS_MODE::TIME_COMPRESS_30MINS,
            clock::TimeCompressMode::TimeCompress60mins => TIME_COMPRESS_MODE::TIME_COMPRESS_60MINS,
        }
    }
    fn from_i32(value: i32) -> Self {
        match value {
            0 => TIME_COMPRESS_MODE::TIME_COMPRESS_X0,
            1 => TIME_COMPRESS_MODE::TIME_COMPRESS_X1,
            2 => TIME_COMPRESS_MODE::TIME_COMPRESS_5MINS,
            3 => TIME_COMPRESS_MODE::TIME_COMPRESS_30MINS,
            4 => TIME_COMPRESS_MODE::TIME_COMPRESS_60MINS,
            _ => panic!("invalid value {value} for TIME_COMPRESS_MODE enum"),
        }
    }
    fn to_internal(&self) -> clock::TimeCompressMode {
        match self {
            TIME_COMPRESS_MODE::TIME_COMPRESS_X0 => clock::TimeCompressMode::TimeCompressX0,
            TIME_COMPRESS_MODE::TIME_COMPRESS_X1 => clock::TimeCompressMode::TimeCompressX1,
            TIME_COMPRESS_MODE::TIME_COMPRESS_5MINS => clock::TimeCompressMode::TimeCompress5mins,
            TIME_COMPRESS_MODE::TIME_COMPRESS_30MINS => clock::TimeCompressMode::TimeCompress30mins,
            TIME_COMPRESS_MODE::TIME_COMPRESS_60MINS => clock::TimeCompressMode::TimeCompress60mins,
        }
    }
}

#[no_mangle]
pub extern "C" fn GetTimeCompressMode() -> TIME_COMPRESS_MODE {
    unsafe { TIME_COMPRESS_MODE::from_internal(STATE.clock.time_compress_mode) }
}

#[no_mangle]
pub extern "C" fn SetTimeCompressMode(mode: TIME_COMPRESS_MODE) {
    unsafe { STATE.clock.time_compress_mode = mode.to_internal() }
}

/// Returns some modifier of the game speed.
#[no_mangle]
pub extern "C" fn GetTimeCompressSpeed() -> i32 {
    match GetTimeCompressMode() {
        TIME_COMPRESS_MODE::TIME_COMPRESS_X0 => 0,
        TIME_COMPRESS_MODE::TIME_COMPRESS_X1 => 1,
        TIME_COMPRESS_MODE::TIME_COMPRESS_5MINS => 5 * 60,
        TIME_COMPRESS_MODE::TIME_COMPRESS_30MINS => 30 * 60,
        TIME_COMPRESS_MODE::TIME_COMPRESS_60MINS => 60 * 60,
    }
}

#[repr(C)]
#[allow(non_snake_case)]
#[derive(Default)]
/// C part of the saved gameclock state
pub struct SavedClockStateC {
    TimeInterrupt: bool,
    SuperCompression: bool,
    AmbientLightLevel: u8,
    EnvTime: u32,
    EnvDay: u32,
    EnvLightValue: u8,
    TimeOfLastEventQuery: u32,
    PauseDueToPlayerGamePause: bool,
    ResetAllPlayerKnowsEnemiesFlags: bool,
    PreviousGameClock: u32,
    LockPauseStateLastReasonId: u32,
}

#[derive(Default)]
struct SavedClockState {
    cpart: SavedClockStateC,
    time_compression_on: bool,
    time_compress_mode: i32,
    game_seconds_per_real_second: u32,
    clock_resolution: u8,
    game_paused: bool,
    game_clock: u32,
    locked_pause: bool,
    padding: [u8; 20],
}

#[no_mangle]
pub extern "C" fn LoadSavedClockState(file_id: FileID, data: &mut SavedClockStateC) -> bool {
    match read_saved_clock_state(file_id) {
        Ok(state) => {
            if state.game_paused {
                PauseGame();
            }
            SetGameTimeSec(state.game_clock);
            if state.locked_pause {
                LockPause();
            }
            SetClockResolutionPerSecond(state.clock_resolution);
            SetGameSecondsPerRealSecond(state.game_seconds_per_real_second);
            SetTimeCompressionOn(state.time_compression_on);
            SetTimeCompressMode(TIME_COMPRESS_MODE::from_i32(state.time_compress_mode));
            *data = state.cpart;
            true
        }
        Err(_) => false,
    }
}

fn read_saved_clock_state(file_id: FileID) -> io::Result<SavedClockState> {
    let mut data = SavedClockState::default();
    unsafe {
        data.time_compress_mode = FILE_DB.read_file_i32(file_id)?;
        data.clock_resolution = FILE_DB.read_file_u8(file_id)?;
        data.game_paused = FILE_DB.read_file_bool(file_id)?;
        data.cpart.TimeInterrupt = FILE_DB.read_file_bool(file_id)?;
        data.cpart.SuperCompression = FILE_DB.read_file_bool(file_id)?;
        data.game_clock = FILE_DB.read_file_u32(file_id)?;
        data.game_seconds_per_real_second = FILE_DB.read_file_u32(file_id)?;
        data.cpart.AmbientLightLevel = FILE_DB.read_file_u8(file_id)?;
        data.cpart.EnvTime = FILE_DB.read_file_u32(file_id)?;
        data.cpart.EnvDay = FILE_DB.read_file_u32(file_id)?;
        data.cpart.EnvLightValue = FILE_DB.read_file_u8(file_id)?;
        data.cpart.TimeOfLastEventQuery = FILE_DB.read_file_u32(file_id)?;
        data.locked_pause = FILE_DB.read_file_bool(file_id)?;
        data.cpart.PauseDueToPlayerGamePause = FILE_DB.read_file_bool(file_id)?;
        data.cpart.ResetAllPlayerKnowsEnemiesFlags = FILE_DB.read_file_bool(file_id)?;
        data.time_compression_on = FILE_DB.read_file_bool(file_id)?;
        data.cpart.PreviousGameClock = FILE_DB.read_file_u32(file_id)?;
        data.cpart.LockPauseStateLastReasonId = FILE_DB.read_file_u32(file_id)?;
        FILE_DB.read_file_exact(file_id, &mut data.padding)?;
    }
    Ok(data)
}

#[no_mangle]
pub extern "C" fn InitNewGameClockRust() {
    SetGameTimeSec(GetGameStartingTime());
    SetGameSecondsPerRealSecond(0);
    SetTimeCompressionOn(false);
    unsafe {
        STATE.clock.clock_resolution = 1;
    }
}

#[no_mangle]
pub extern "C" fn SetClockResolutionPerSecond(timer_per_second: u8) {
    let timer_per_second = std::cmp::max(0, std::cmp::min(60, timer_per_second));
    unsafe {
        STATE.clock.clock_resolution = timer_per_second;
    }
}

/// Returns number of clock updates per second
#[no_mangle]
pub extern "C" fn GetClockResolution() -> u8 {
    unsafe { STATE.clock.clock_resolution }
}

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

#[no_mangle]
pub extern "C" fn GetGameSecondsPerRealSecond() -> u32 {
    unsafe { STATE.clock.game_seconds_per_real_second }
}

#[no_mangle]
pub extern "C" fn SetGameSecondsPerRealSecond(value: u32) {
    unsafe {
        STATE.clock.game_seconds_per_real_second = value;
    }
}

#[no_mangle]
pub extern "C" fn GetTimeCompressionOn() -> bool {
    unsafe { STATE.clock.time_compression_on }
}

#[no_mangle]
pub extern "C" fn SetTimeCompressionOn(value: bool) {
    unsafe {
        STATE.clock.time_compression_on = value;
    }
}

#[no_mangle]
pub extern "C" fn StopTimeCompression() {
    if GetTimeCompressionOn() {
        // change the clock resolution to no time passage, but don't actually change the compress mode
        // (remember it)
        SetGameSecondsPerRealSecond(0);
        SetClockResolutionPerSecond(0);
        SetTimeCompressionOn(false);
        exp_ui::SetMapScreenBottomDirty(true);
    }
}
