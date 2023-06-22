#pragma once

/* Warning, this file is autogenerated by cbindgen. Don't modify this manually. */

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

/**
 * C part of the saved gameclock state
 */
struct SavedClockStateC {
  int32_t TimeCompressMode;
  uint8_t ClockResolution;
  bool TimeInterrupt;
  bool SuperCompression;
  uint32_t GameSecondsPerRealSecond;
  uint8_t AmbientLightLevel;
  uint32_t EnvTime;
  uint32_t EnvDay;
  uint8_t EnvLightValue;
  uint32_t TimeOfLastEventQuery;
  bool PauseDueToPlayerGamePause;
  bool ResetAllPlayerKnowsEnemiesFlags;
  bool TimeCompressionOn;
  uint32_t PreviousGameClock;
  uint32_t LockPauseStateLastReasonId;
};

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

bool LoadSavedClockState(FileID file_id, struct SavedClockStateC *data);

/**
 * Get game starting time in seconds.
 */
uint32_t GetGameStartingTime(void);

/**
 * Get game time in seconds
 */
uint32_t GetGameTimeInSec(void);

/**
 * Set game time in seconds
 */
void SetGameTimeSec(uint32_t value);

/**
 * Adjust game time forward by given number of seconds.
 */
void MoveGameTimeForward(uint32_t seconds);

/**
 * Get game time in minutes
 */
uint32_t GetGameTimeInMin(void);

/**
 * Get game time in days
 */
uint32_t GetGameTimeInDays(void);

/**
 * Get game clock hour - returns hour shown on the game clock (0 - 23)
 */
uint32_t GetGameClockHour(void);

/**
 * Get game clock minutes - returns minutes shown on the game clock (0 - 59)
 */
uint32_t GetGameClockMinutes(void);

/**
 * Get number of minutes passed since game day start
 */
uint32_t GetMinutesSinceDayStart(void);

/**
 * Get time of future midnight in minutes
 */
uint32_t GetFutureMidnightInMinutes(uint32_t days);

bool IsDayTime(void);

bool IsNightTime(void);

void PauseGame(void);

void UnPauseGame(void);

void TogglePause(void);

bool IsGamePaused(void);

void LockPause(void);

void UnlockPause(void);

bool IsPauseLocked(void);

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus
