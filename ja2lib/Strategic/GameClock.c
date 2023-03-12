#include "Strategic/GameClock.h"

#include "FadeScreen.h"
#include "GameScreen.h"
#include "JAScreens.h"
#include "SGP/ButtonSystem.h"
#include "SGP/Debug.h"
#include "SGP/FileMan.h"
#include "SGP/SGP.h"
#include "SGP/VSurface.h"
#include "SGP/Video.h"
#include "ScreenIDs.h"
#include "Strategic/Assignments.h"
#include "Strategic/GameEvents.h"
#include "Strategic/MapScreen.h"
#include "Strategic/MapScreenInterface.h"
#include "Strategic/MapScreenInterfaceBottom.h"
#include "Strategic/MapScreenInterfaceMap.h"
#include "Strategic/PreBattleInterface.h"
#include "Tactical/MapInformation.h"
#include "Tactical/Overhead.h"
#include "TileEngine/Environment.h"
#include "TileEngine/Lighting.h"
#include "TileEngine/RenderDirty.h"
#include "TileEngine/RenderWorld.h"
#include "UI.h"
#include "Utils/EventPump.h"
#include "Utils/FontControl.h"
#include "Utils/MercTextBox.h"
#include "Utils/Message.h"
#include "Utils/Text.h"
#include "Utils/TimerControl.h"

// #define DEBUG_GAME_CLOCK

extern BOOLEAN gfFadeOut;

// These functions shouldn't be used anywhere else...
extern BOOLEAN GameEventsPending(uint32_t uiAdjustment);
extern void ProcessPendingGameEvents(uint32_t uiAdjustment, uint8_t ubWarpCode);
void PauseOfClockBtnCallback(struct MOUSE_REGION* pRegion, int32_t iReason);
void ScreenMaskForGamePauseBtnCallBack(struct MOUSE_REGION* pRegion, int32_t iReason);

void CreateDestroyScreenMaskForPauseGame(void);

void SetClockResolutionToCompressMode(int32_t iCompressMode);

// is the clock pause region created currently?
BOOLEAN fClockMouseRegionCreated = FALSE;

BOOLEAN fTimeCompressHasOccured = FALSE;

// This value represents the time that the sector was loaded.  If you are in sector A9, and leave
// the game clock at that moment will get saved into the temp file associated with it.  The next
// time you enter A9, this value will contain that time.  Used for scheduling purposes.
uint32_t guiTimeCurrentSectorWasLastLoaded = 0;

// did we JUST finish up a game pause by the player
BOOLEAN gfJustFinishedAPause = FALSE;

// clock mouse region
struct MOUSE_REGION gClockMouseRegion;
struct MOUSE_REGION gClockScreenMaskMouseRegion;
void AdvanceClock(uint8_t ubWarpCode);

extern BOOLEAN fMapScreenBottomDirty;

#define SECONDS_PER_COMPRESSION 1  // 1/2 minute passes every 1 second of real time
#define SECONDS_PER_COMPRESSION_IN_RTCOMBAT 10
#define SECONDS_PER_COMPRESSION_IN_TBCOMBAT 10
#define CLOCK_STRING_HEIGHT 13
#define CLOCK_STRING_WIDTH 66
#define CLOCK_FONT COMPFONT

// These contain all of the information about the game time, rate of time, etc.
// All of these get saved and loaded.
int32_t giTimeCompressMode = TIME_COMPRESS_X0;
uint8_t gubClockResolution = 1;
BOOLEAN gfGamePaused = TRUE;
BOOLEAN gfTimeInterrupt = FALSE;
BOOLEAN gfTimeInterruptPause = FALSE;
BOOLEAN fSuperCompression = FALSE;
uint32_t guiGameClock = STARTING_TIME;
uint32_t guiPreviousGameClock = 0;  // used only for error-checking purposes
uint32_t guiGameSecondsPerRealSecond;
uint32_t guiTimesThisSecondProcessed = 0;
int32_t iPausedPopUpBox = -1;
uint32_t guiDay;
uint32_t guiHour;
uint32_t guiMin;
wchar_t gswzWorldTimeStr[20];
int32_t giTimeCompressSpeeds[NUM_TIME_COMPRESS_SPEEDS] = {0, 1, 5 * 60, 30 * 60, 60 * 60};
uint16_t usPausedActualWidth;
uint16_t usPausedActualHeight;
uint32_t guiTimeOfLastEventQuery = 0;
BOOLEAN gfLockPauseState = FALSE;
BOOLEAN gfPauseDueToPlayerGamePause = FALSE;
BOOLEAN gfResetAllPlayerKnowsEnemiesFlags = FALSE;
BOOLEAN gfTimeCompressionOn = FALSE;
uint32_t guiLockPauseStateLastReasonId = 0;
//***When adding new saved time variables, make sure you remove the appropriate amount from the
// paddingbytes and
//   more IMPORTANTLY, add appropriate code in Save/LoadGameClock()!
#define TIME_PADDINGBYTES 20
uint8_t gubUnusedTimePadding[TIME_PADDINGBYTES];

extern uint32_t guiEnvTime;
extern uint32_t guiEnvDay;

void InitNewGameClock() {
  guiGameClock = STARTING_TIME;
  guiPreviousGameClock = STARTING_TIME;
  guiDay = (guiGameClock / NUM_SEC_IN_DAY);
  guiHour = (guiGameClock - (guiDay * NUM_SEC_IN_DAY)) / NUM_SEC_IN_HOUR;
  guiMin =
      (guiGameClock - ((guiDay * NUM_SEC_IN_DAY) + (guiHour * NUM_SEC_IN_HOUR))) / NUM_SEC_IN_MIN;
  swprintf(WORLDTIMESTR, ARR_SIZE(WORLDTIMESTR), L"%s %d, %02d:%02d", pDayStrings[0], guiDay,
           guiHour, guiMin);
  guiTimeCurrentSectorWasLastLoaded = 0;
  guiGameSecondsPerRealSecond = 0;
  gubClockResolution = 1;
  memset(gubUnusedTimePadding, 0, TIME_PADDINGBYTES);
}

uint32_t GetWorldTotalMin() { return (guiGameClock / NUM_SEC_IN_MIN); }

uint32_t GetWorldTotalSeconds() { return (guiGameClock); }

uint32_t GetWorldHour() { return (guiHour); }

uint32_t GetWorldMinutesInDay() { return ((guiHour * 60) + guiMin); }

uint32_t GetWorldDay() { return (guiDay); }

uint32_t GetWorldDayInSeconds() { return (guiDay * NUM_SEC_IN_DAY); }

uint32_t GetWorldDayInMinutes() { return ((guiDay * NUM_SEC_IN_DAY) / NUM_SEC_IN_MIN); }

uint32_t GetFutureDayInMinutes(uint32_t uiDay) {
  return ((uiDay * NUM_SEC_IN_DAY) / NUM_SEC_IN_MIN);
}

// this function returns the amount of minutes there has been from start of game to midnight of the
// uiDay.
uint32_t GetMidnightOfFutureDayInMinutes(uint32_t uiDay) {
  return (GetWorldTotalMin() + (uiDay * 1440) - GetWorldMinutesInDay());
}

// Not to be used too often by things other than internally
void WarpGameTime(uint32_t uiAdjustment, uint8_t ubWarpCode) {
  uint32_t uiSaveTimeRate;
  uiSaveTimeRate = guiGameSecondsPerRealSecond;
  guiGameSecondsPerRealSecond = uiAdjustment;
  AdvanceClock(ubWarpCode);
  guiGameSecondsPerRealSecond = uiSaveTimeRate;
}

void AdvanceClock(uint8_t ubWarpCode) {
  // Set value, to different things if we are in combat...
  if ((gTacticalStatus.uiFlags & INCOMBAT)) {
    if ((gTacticalStatus.uiFlags & TURNBASED)) {
    } else {
    }
  }

  if (ubWarpCode != WARPTIME_NO_PROCESSING_OF_EVENTS) {
    guiTimeOfLastEventQuery = guiGameClock;
    // First of all, events are posted for movements, pending attacks, equipment arrivals, etc. This
    // time adjustment using time compression can possibly pass one or more events in a single pass.
    // So, this list is looked at and processed in sequential order, until the uiAdjustment is fully
    // applied.
    if (GameEventsPending(guiGameSecondsPerRealSecond)) {
      // If a special event, justifying the cancellation of time compression is reached, the
      // adjustment will be shortened to the time of that event, and will stop processing events,
      // otherwise, all of the events in the time slice will be processed.  The time is adjusted
      // internally as events are processed.
      ProcessPendingGameEvents(guiGameSecondsPerRealSecond, ubWarpCode);
    } else {
      // Adjust the game clock now.
      guiGameClock += guiGameSecondsPerRealSecond;
    }
  } else {
    guiGameClock += guiGameSecondsPerRealSecond;
  }

  if (guiGameClock < guiPreviousGameClock) {
    AssertMsg(FALSE,
              String("AdvanceClock: TIME FLOWING BACKWARDS!!! guiPreviousGameClock %d, now %d",
                     guiPreviousGameClock, guiGameClock));

    // fix it if assertions are disabled
    guiGameClock = guiPreviousGameClock;
  }

  // store previous game clock value (for error-checking purposes only)
  guiPreviousGameClock = guiGameClock;

  // Calculate the day, hour, and minutes.
  guiDay = (guiGameClock / NUM_SEC_IN_DAY);
  guiHour = (guiGameClock - (guiDay * NUM_SEC_IN_DAY)) / NUM_SEC_IN_HOUR;
  guiMin =
      (guiGameClock - ((guiDay * NUM_SEC_IN_DAY) + (guiHour * NUM_SEC_IN_HOUR))) / NUM_SEC_IN_MIN;

  swprintf(WORLDTIMESTR, ARR_SIZE(WORLDTIMESTR), L"%s %d, %02d:%02d",
           gpGameClockString[STR_GAMECLOCK_DAY_NAME], guiDay, guiHour, guiMin);

  if (gfResetAllPlayerKnowsEnemiesFlags && !gTacticalStatus.fEnemyInSector) {
    ClearAnySectorsFlashingNumberOfEnemies();

    gfResetAllPlayerKnowsEnemiesFlags = FALSE;
  }

  ForecastDayEvents();
}

void AdvanceToNextDay() {
  int32_t uiDiff;
  uint32_t uiTomorrowTimeInSec;

  uiTomorrowTimeInSec = (guiDay + 1) * NUM_SEC_IN_DAY + 8 * NUM_SEC_IN_HOUR + 15 * NUM_SEC_IN_MIN;
  uiDiff = uiTomorrowTimeInSec - guiGameClock;
  WarpGameTime(uiDiff, WARPTIME_PROCESS_EVENTS_NORMALLY);

  ForecastDayEvents();
}

// set the flag that time compress has occured
void SetFactTimeCompressHasOccured(void) {
  fTimeCompressHasOccured = TRUE;
  return;
}

// reset fact the time compress has occured
void ResetTimeCompressHasOccured(void) {
  fTimeCompressHasOccured = FALSE;
  return;
}

// has time compress occured?
BOOLEAN HasTimeCompressOccured(void) { return (fTimeCompressHasOccured); }

void RenderClock(int16_t sX, int16_t sY) {
  SetFont(CLOCK_FONT);
  SetFontBackground(FONT_MCOLOR_BLACK);

#ifdef CRIPPLED_VERSION
  if (guiDay >= 8) {
    SetFontForeground(FONT_FCOLOR_NICERED);
    // Erase first!
    RestoreExternBackgroundRect(sX, sY, CLOCK_STRING_WIDTH, CLOCK_STRING_HEIGHT);
    swprintf(gswzWorldTimeStr, ARR_SIZE(gswzWorldTimeStr), L"GAME OVER");
    mprintf(sX + (CLOCK_STRING_WIDTH - StringPixLength(WORLDTIMESTR, CLOCK_FONT)) / 2, sY,
            WORLDTIMESTR);
    return;
  }
#endif

  // Are we in combat?
  if (gTacticalStatus.uiFlags & INCOMBAT) {
    SetFontForeground(FONT_FCOLOR_NICERED);
  } else {
    SetFontForeground(FONT_LTGREEN);
  }

  // Erase first!
  RestoreExternBackgroundRect(sX, sY, CLOCK_STRING_WIDTH, CLOCK_STRING_HEIGHT);

  if ((gfPauseDueToPlayerGamePause == FALSE)) {
    mprintf(sX + (CLOCK_STRING_WIDTH - StringPixLength(WORLDTIMESTR, CLOCK_FONT)) / 2, sY,
            WORLDTIMESTR);
  } else {
    mprintf(sX + (CLOCK_STRING_WIDTH - StringPixLength(pPausedGameText[0], CLOCK_FONT)) / 2, sY,
            pPausedGameText[0]);
  }
}

void ToggleSuperCompression() {
  static uint32_t uiOldTimeCompressMode = 0;

  // Display message
  if (gTacticalStatus.uiFlags & INCOMBAT) {
    // ScreenMsg( MSG_FONT_YELLOW, MSG_INTERFACE, L"Cannot toggle compression in Combat Mode."  );
    return;
  }

  fSuperCompression = (BOOLEAN)(!fSuperCompression);

  if (fSuperCompression) {
    uiOldTimeCompressMode = giTimeCompressMode;
    giTimeCompressMode = TIME_SUPER_COMPRESS;
    guiGameSecondsPerRealSecond =
        giTimeCompressSpeeds[giTimeCompressMode] * SECONDS_PER_COMPRESSION;

    // ScreenMsg( MSG_FONT_YELLOW, MSG_INTERFACE, L"Time compression ON."  );
  } else {
    giTimeCompressMode = uiOldTimeCompressMode;
    guiGameSecondsPerRealSecond =
        giTimeCompressSpeeds[giTimeCompressMode] * SECONDS_PER_COMPRESSION;

    // ScreenMsg( MSG_FONT_YELLOW, MSG_INTERFACE, L"Time compression OFF."  );
  }
}

BOOLEAN DidGameJustStart() {
  if (gTacticalStatus.fDidGameJustStart)
    return (TRUE);
  else
    return (FALSE);
}

void StopTimeCompression(void) {
  if (gfTimeCompressionOn) {
    // change the clock resolution to no time passage, but don't actually change the compress mode
    // (remember it)
    SetClockResolutionToCompressMode(TIME_COMPRESS_X0);
  }
}

void StartTimeCompression(void) {
  if (!gfTimeCompressionOn) {
    if (GamePaused()) {
      // first have to be allowed to unpause the game
      UnPauseGame();

      // if we couldn't, ignore this request
      if (GamePaused()) {
        return;
      }
    }

    // check that we can start compressing
    if (!AllowedToTimeCompress()) {
      // not allowed to compress time
      TellPlayerWhyHeCantCompressTime();
      return;
    }

    // if no compression mode is set, increase it first
    if (giTimeCompressMode <= TIME_COMPRESS_X1) {
      IncreaseGameTimeCompressionRate();
    }

    // change clock resolution to the current compression mode
    SetClockResolutionToCompressMode(giTimeCompressMode);

    // if it's the first time we're doing this since entering map screen (which reset the flag)
    if (!HasTimeCompressOccured()) {
      // set fact that we have compressed time during this map screen session
      SetFactTimeCompressHasOccured();

      ClearTacticalStuffDueToTimeCompression();
    }
  }
}

// returns FALSE if time isn't currently being compressed for ANY reason (various pauses, etc.)
BOOLEAN IsTimeBeingCompressed(void) {
  if (!gfTimeCompressionOn || (giTimeCompressMode == TIME_COMPRESS_X0) || gfGamePaused)
    return (FALSE);
  else
    return (TRUE);
}

// returns TRUE if the player currently doesn't want time to be compressing
BOOLEAN IsTimeCompressionOn(void) { return (gfTimeCompressionOn); }

void IncreaseGameTimeCompressionRate() {
  // if not already at maximum time compression rate
  if (giTimeCompressMode < TIME_COMPRESS_60MINS) {
    // check that we can
    if (!AllowedToTimeCompress()) {
      // not allowed to compress time
      TellPlayerWhyHeCantCompressTime();
      return;
    }

    giTimeCompressMode++;

    // in map screen, we wanna have to skip over x1 compression and go straight to 5x
    if ((IsMapScreen_2()) && (giTimeCompressMode == TIME_COMPRESS_X1)) {
      giTimeCompressMode++;
    }

    SetClockResolutionToCompressMode(giTimeCompressMode);
  }
}

void DecreaseGameTimeCompressionRate() {
  // if not already at minimum time compression rate
  if (giTimeCompressMode > TIME_COMPRESS_X0) {
    // check that we can
    if (!AllowedToTimeCompress()) {
      // not allowed to compress time
      TellPlayerWhyHeCantCompressTime();
      return;
    }

    giTimeCompressMode--;

    // in map screen, we wanna have to skip over x1 compression and go straight to 5x
    if ((IsMapScreen_2()) && (giTimeCompressMode == TIME_COMPRESS_X1)) {
      giTimeCompressMode--;
    }

    SetClockResolutionToCompressMode(giTimeCompressMode);
  }
}

void SetGameTimeCompressionLevel(uint32_t uiCompressionRate) {
  Assert(uiCompressionRate < NUM_TIME_COMPRESS_SPEEDS);

  if (IsTacticalMode()) {
    if (uiCompressionRate != TIME_COMPRESS_X1) {
      uiCompressionRate = TIME_COMPRESS_X1;
    }
  }

  if (IsMapScreen_2()) {
    if (uiCompressionRate == TIME_COMPRESS_X1) {
      uiCompressionRate = TIME_COMPRESS_X0;
    }
  }

  // if we're attempting time compression
  if (uiCompressionRate >= TIME_COMPRESS_5MINS) {
    // check that we can
    if (!AllowedToTimeCompress()) {
      // not allowed to compress time
      TellPlayerWhyHeCantCompressTime();
      return;
    }
  }

  giTimeCompressMode = uiCompressionRate;
  SetClockResolutionToCompressMode(giTimeCompressMode);
}

void SetClockResolutionToCompressMode(int32_t iCompressMode) {
  guiGameSecondsPerRealSecond = giTimeCompressSpeeds[iCompressMode] * SECONDS_PER_COMPRESSION;

  // ok this is a bit confusing, but for time compression (e.g. 30x60) we want updates
  // 30x per second, but for standard unpaused time, like in tactical, we want 1x per second
  if (guiGameSecondsPerRealSecond == 0) {
    SetClockResolutionPerSecond(0);
  } else {
    SetClockResolutionPerSecond((uint8_t)max(1, (uint8_t)(guiGameSecondsPerRealSecond / 60)));
  }

  // if the compress mode is X0 or X1
  if (iCompressMode <= TIME_COMPRESS_X1) {
    gfTimeCompressionOn = FALSE;
  } else {
    gfTimeCompressionOn = TRUE;

    // handle the player just starting a game
    HandleTimeCompressWithTeamJackedInAndGearedToGo();
  }

  fMapScreenBottomDirty = TRUE;
}

void SetGameHoursPerSecond(uint32_t uiGameHoursPerSecond) {
  giTimeCompressMode = NOT_USING_TIME_COMPRESSION;
  guiGameSecondsPerRealSecond = uiGameHoursPerSecond * 3600;
  if (uiGameHoursPerSecond == 1) {
    SetClockResolutionPerSecond(60);
  } else {
    SetClockResolutionPerSecond(59);
  }
}

void SetGameMinutesPerSecond(uint32_t uiGameMinutesPerSecond) {
  giTimeCompressMode = NOT_USING_TIME_COMPRESSION;
  guiGameSecondsPerRealSecond = uiGameMinutesPerSecond * 60;
  SetClockResolutionPerSecond((uint8_t)uiGameMinutesPerSecond);
}

void SetGameSecondsPerSecond(uint32_t uiGameSecondsPerSecond) {
  giTimeCompressMode = NOT_USING_TIME_COMPRESSION;
  guiGameSecondsPerRealSecond = uiGameSecondsPerSecond;
  //	SetClockResolutionPerSecond( (uint8_t)(guiGameSecondsPerRealSecond / 60) );
  if (guiGameSecondsPerRealSecond == 0) {
    SetClockResolutionPerSecond(0);
  } else {
    SetClockResolutionPerSecond((uint8_t)max(1, (uint8_t)(guiGameSecondsPerRealSecond / 60)));
  }
}

// call this to prevent player from changing the time compression state via the interface

void LockPauseState(uint32_t uiUniqueReasonId) {
  gfLockPauseState = TRUE;

  // if adding a new call, please choose a new uiUniqueReasonId, this helps track down the cause
  // when it's left locked Highest # used was 21 on Feb 15 '99.
  guiLockPauseStateLastReasonId = uiUniqueReasonId;
}

// call this to allow player to change the time compression state via the interface once again
void UnLockPauseState() { gfLockPauseState = FALSE; }

// tells you whether the player is currently locked out from messing with the time compression state
BOOLEAN PauseStateLocked() { return gfLockPauseState; }

void PauseGame() {
  // always allow pausing, even if "locked".  Locking applies only to trying to compress time, not
  // to pausing it
  if (!gfGamePaused) {
    gfGamePaused = TRUE;
    fMapScreenBottomDirty = TRUE;
  }
}

void UnPauseGame() {
  // if we're paused
  if (gfGamePaused) {
    // ignore request if locked
    if (gfLockPauseState) {
      ScreenMsg(FONT_ORANGE, MSG_TESTVERSION,
                L"Call to UnPauseGame() while Pause State is LOCKED! AM-4");
      return;
    }

    gfGamePaused = FALSE;
    fMapScreenBottomDirty = TRUE;
  }
}

void TogglePause() {
  if (gfGamePaused) {
    UnPauseGame();
  } else {
    PauseGame();
  }
}

BOOLEAN GamePaused() { return gfGamePaused; }

// ONLY APPLICABLE INSIDE EVENT CALLBACKS!
void InterruptTime() { gfTimeInterrupt = TRUE; }

void PauseTimeForInterupt() { gfTimeInterruptPause = TRUE; }

// USING CLOCK RESOLUTION
// Note, that changing the clock resolution doesn't effect the amount of game time that passes per
// real second, but how many times per second the clock is updated.  This rate will break up the
// actual time slices per second into smaller chunks.  This is useful for animating strategic
// movement under fast time compression, so objects don't warp around.
void SetClockResolutionToDefault() { gubClockResolution = 1; }

// Valid range is 0 - 60 times per second.
void SetClockResolutionPerSecond(uint8_t ubNumTimesPerSecond) {
  ubNumTimesPerSecond = (uint8_t)(max(0, min(60, ubNumTimesPerSecond)));
  gubClockResolution = ubNumTimesPerSecond;
}

// Function for accessing the current rate
uint8_t ClockResolution() { return gubClockResolution; }

// There are two factors that influence the flow of time in the game.
//-Speed:  The speed is the amount of game time passes per real second of time.  The higher this
//         value, the faster the game time flows.
//-Resolution:  The higher the resolution, the more often per second the clock is actually updated.
//				 This value doesn't affect how much game time passes per real
// second, but allows for 				 a more accurate representation of faster
// time
// flows.
void UpdateClock() {
  uint32_t uiNewTime;
  uint32_t uiThousandthsOfThisSecondProcessed;
  uint32_t uiTimeSlice;
  uint32_t uiNewTimeProcessed;
  static uint8_t ubLastResolution = 1;
  static uint32_t uiLastSecondTime = 0;
  static uint32_t uiLastTimeProcessed = 0;
#ifdef DEBUG_GAME_CLOCK
  uint32_t uiOrigNewTime;
  uint32_t uiOrigLastSecondTime;
  uint32_t uiOrigThousandthsOfThisSecondProcessed;
  uint8_t ubOrigClockResolution;
  uint32_t uiOrigTimesThisSecondProcessed;
  uint8_t ubOrigLastResolution;
#endif
  // check game state for pause screen masks
  CreateDestroyScreenMaskForPauseGame();

#ifdef JA2BETAVERSION
  if (guiCurrentScreen != GAME_SCREEN && !IsMapScreen_2() && guiCurrentScreen != AIVIEWER_SCREEN &&
      guiCurrentScreen != GAME_SCREEN)
#else
  if (guiCurrentScreen != GAME_SCREEN && !IsMapScreen_2() && guiCurrentScreen != GAME_SCREEN)
#endif
  {
    uiLastSecondTime = GetJA2Clock();
    gfTimeInterruptPause = FALSE;
    return;
  }

  if (gfGamePaused || gfTimeInterruptPause || (gubClockResolution == 0) ||
      !guiGameSecondsPerRealSecond || ARE_IN_FADE_IN() || gfFadeOut) {
    uiLastSecondTime = GetJA2Clock();
    gfTimeInterruptPause = FALSE;
    return;
  }

  if ((gTacticalStatus.uiFlags & TURNBASED && gTacticalStatus.uiFlags & INCOMBAT))
    return;  // time is currently stopped!

  uiNewTime = GetJA2Clock();

#ifdef DEBUG_GAME_CLOCK
  uiOrigNewTime = uiNewTime;
  uiOrigLastSecondTime = uiLastSecondTime;
  uiOrigThousandthsOfThisSecondProcessed = uiThousandthsOfThisSecondProcessed;
  ubOrigClockResolution = gubClockResolution;
  uiOrigTimesThisSecondProcessed = guiTimesThisSecondProcessed;
  ubOrigLastResolution = ubLastResolution;
#endif

  // Because we debug so much, breakpoints tend to break the game, and cause unnecessary headaches.
  // This line ensures that no more than 1 real-second passes between frames.  This otherwise has
  // no effect on anything else.
  uiLastSecondTime = max(uiNewTime - 1000, uiLastSecondTime);

  // 1000's of a second difference since last second.
  uiThousandthsOfThisSecondProcessed = uiNewTime - uiLastSecondTime;

  if (uiThousandthsOfThisSecondProcessed >= 1000 && gubClockResolution == 1) {
    uiLastSecondTime = uiNewTime;
    guiTimesThisSecondProcessed = uiLastTimeProcessed = 0;
    AdvanceClock(WARPTIME_PROCESS_EVENTS_NORMALLY);
  } else if (gubClockResolution > 1) {
    if (gubClockResolution != ubLastResolution) {
      // guiTimesThisSecondProcessed = guiTimesThisSecondProcessed * ubLastResolution /
      // gubClockResolution % gubClockResolution;
      guiTimesThisSecondProcessed =
          guiTimesThisSecondProcessed * gubClockResolution / ubLastResolution;
      uiLastTimeProcessed = uiLastTimeProcessed * gubClockResolution / ubLastResolution;
      ubLastResolution = gubClockResolution;
    }
    uiTimeSlice = 1000000 / gubClockResolution;
    if (uiThousandthsOfThisSecondProcessed >=
        uiTimeSlice * (guiTimesThisSecondProcessed + 1) / 1000) {
      guiTimesThisSecondProcessed = uiThousandthsOfThisSecondProcessed * 1000 / uiTimeSlice;
      uiNewTimeProcessed =
          guiGameSecondsPerRealSecond * guiTimesThisSecondProcessed / gubClockResolution;

      uiNewTimeProcessed = max(uiNewTimeProcessed, uiLastTimeProcessed);

#ifdef DEBUG_GAME_CLOCK
      if (uiAmountToAdvanceTime > 0x80000000 ||
          guiGameClock + uiAmountToAdvanceTime < guiPreviousGameClock) {
        uiNewTimeProcessed = uiNewTimeProcessed;
      }
#endif

      WarpGameTime(uiNewTimeProcessed - uiLastTimeProcessed, WARPTIME_PROCESS_EVENTS_NORMALLY);
      if (uiNewTimeProcessed < guiGameSecondsPerRealSecond) {  // Processed the same real second
        uiLastTimeProcessed = uiNewTimeProcessed;
      } else {  // We have moved into a new real second.
        uiLastTimeProcessed = uiNewTimeProcessed % guiGameSecondsPerRealSecond;
        if (gubClockResolution > 0) {
          guiTimesThisSecondProcessed %= gubClockResolution;
        } else {
          // this branch occurs whenever an event during WarpGameTime stops time compression!
          guiTimesThisSecondProcessed = 0;
        }
        uiLastSecondTime = uiNewTime;
      }
    }
  }
}

BOOLEAN SaveGameClock(HWFILE hFile, BOOLEAN fGamePaused, BOOLEAN fLockPauseState) {
  uint32_t uiNumBytesWritten = 0;

  FileMan_Write(hFile, &giTimeCompressMode, sizeof(int32_t), &uiNumBytesWritten);
  if (uiNumBytesWritten != sizeof(int32_t)) return (FALSE);

  FileMan_Write(hFile, &gubClockResolution, sizeof(uint8_t), &uiNumBytesWritten);
  if (uiNumBytesWritten != sizeof(uint8_t)) return (FALSE);

  FileMan_Write(hFile, &fGamePaused, sizeof(BOOLEAN), &uiNumBytesWritten);
  if (uiNumBytesWritten != sizeof(BOOLEAN)) return (FALSE);

  FileMan_Write(hFile, &gfTimeInterrupt, sizeof(BOOLEAN), &uiNumBytesWritten);
  if (uiNumBytesWritten != sizeof(BOOLEAN)) return (FALSE);

  FileMan_Write(hFile, &fSuperCompression, sizeof(BOOLEAN), &uiNumBytesWritten);
  if (uiNumBytesWritten != sizeof(BOOLEAN)) return (FALSE);

  FileMan_Write(hFile, &guiGameClock, sizeof(uint32_t), &uiNumBytesWritten);
  if (uiNumBytesWritten != sizeof(uint32_t)) return (FALSE);

  FileMan_Write(hFile, &guiGameSecondsPerRealSecond, sizeof(uint32_t), &uiNumBytesWritten);
  if (uiNumBytesWritten != sizeof(uint32_t)) return (FALSE);

  FileMan_Write(hFile, &ubAmbientLightLevel, sizeof(uint8_t), &uiNumBytesWritten);
  if (uiNumBytesWritten != sizeof(uint8_t)) return (FALSE);

  FileMan_Write(hFile, &guiEnvTime, sizeof(uint32_t), &uiNumBytesWritten);
  if (uiNumBytesWritten != sizeof(uint32_t)) return (FALSE);

  FileMan_Write(hFile, &guiEnvDay, sizeof(uint32_t), &uiNumBytesWritten);
  if (uiNumBytesWritten != sizeof(uint32_t)) return (FALSE);

  FileMan_Write(hFile, &gubEnvLightValue, sizeof(uint8_t), &uiNumBytesWritten);
  if (uiNumBytesWritten != sizeof(uint8_t)) return (FALSE);

  FileMan_Write(hFile, &guiTimeOfLastEventQuery, sizeof(uint32_t), &uiNumBytesWritten);
  if (uiNumBytesWritten != sizeof(uint32_t)) return (FALSE);

  FileMan_Write(hFile, &fLockPauseState, sizeof(BOOLEAN), &uiNumBytesWritten);
  if (uiNumBytesWritten != sizeof(BOOLEAN)) return (FALSE);

  FileMan_Write(hFile, &gfPauseDueToPlayerGamePause, sizeof(BOOLEAN), &uiNumBytesWritten);
  if (uiNumBytesWritten != sizeof(BOOLEAN)) return (FALSE);

  FileMan_Write(hFile, &gfResetAllPlayerKnowsEnemiesFlags, sizeof(BOOLEAN), &uiNumBytesWritten);
  if (uiNumBytesWritten != sizeof(BOOLEAN)) return (FALSE);

  FileMan_Write(hFile, &gfTimeCompressionOn, sizeof(BOOLEAN), &uiNumBytesWritten);
  if (uiNumBytesWritten != sizeof(BOOLEAN)) return (FALSE);

  FileMan_Write(hFile, &guiPreviousGameClock, sizeof(uint32_t), &uiNumBytesWritten);
  if (uiNumBytesWritten != sizeof(uint32_t)) return (FALSE);

  FileMan_Write(hFile, &guiLockPauseStateLastReasonId, sizeof(uint32_t), &uiNumBytesWritten);
  if (uiNumBytesWritten != sizeof(uint32_t)) return (FALSE);

  FileMan_Write(hFile, gubUnusedTimePadding, TIME_PADDINGBYTES, &uiNumBytesWritten);
  if (uiNumBytesWritten != TIME_PADDINGBYTES) return (FALSE);
  return (TRUE);
}

BOOLEAN LoadGameClock(HWFILE hFile) {
  uint32_t uiNumBytesRead;

  FileMan_Read(hFile, &giTimeCompressMode, sizeof(int32_t), &uiNumBytesRead);
  if (uiNumBytesRead != sizeof(int32_t)) return (FALSE);

  FileMan_Read(hFile, &gubClockResolution, sizeof(uint8_t), &uiNumBytesRead);
  if (uiNumBytesRead != sizeof(uint8_t)) return (FALSE);

  FileMan_Read(hFile, &gfGamePaused, sizeof(BOOLEAN), &uiNumBytesRead);
  if (uiNumBytesRead != sizeof(BOOLEAN)) return (FALSE);

  FileMan_Read(hFile, &gfTimeInterrupt, sizeof(BOOLEAN), &uiNumBytesRead);
  if (uiNumBytesRead != sizeof(BOOLEAN)) return (FALSE);

  FileMan_Read(hFile, &fSuperCompression, sizeof(BOOLEAN), &uiNumBytesRead);
  if (uiNumBytesRead != sizeof(BOOLEAN)) return (FALSE);

  FileMan_Read(hFile, &guiGameClock, sizeof(uint32_t), &uiNumBytesRead);
  if (uiNumBytesRead != sizeof(uint32_t)) return (FALSE);

  FileMan_Read(hFile, &guiGameSecondsPerRealSecond, sizeof(uint32_t), &uiNumBytesRead);
  if (uiNumBytesRead != sizeof(uint32_t)) return (FALSE);

  FileMan_Read(hFile, &ubAmbientLightLevel, sizeof(uint8_t), &uiNumBytesRead);
  if (uiNumBytesRead != sizeof(uint8_t)) return (FALSE);

  FileMan_Read(hFile, &guiEnvTime, sizeof(uint32_t), &uiNumBytesRead);
  if (uiNumBytesRead != sizeof(uint32_t)) return (FALSE);

  FileMan_Read(hFile, &guiEnvDay, sizeof(uint32_t), &uiNumBytesRead);
  if (uiNumBytesRead != sizeof(uint32_t)) return (FALSE);

  FileMan_Read(hFile, &gubEnvLightValue, sizeof(uint8_t), &uiNumBytesRead);
  if (uiNumBytesRead != sizeof(uint8_t)) return (FALSE);

  FileMan_Read(hFile, &guiTimeOfLastEventQuery, sizeof(uint32_t), &uiNumBytesRead);
  if (uiNumBytesRead != sizeof(uint32_t)) return (FALSE);

  FileMan_Read(hFile, &gfLockPauseState, sizeof(BOOLEAN), &uiNumBytesRead);
  if (uiNumBytesRead != sizeof(BOOLEAN)) return (FALSE);

  FileMan_Read(hFile, &gfPauseDueToPlayerGamePause, sizeof(BOOLEAN), &uiNumBytesRead);
  if (uiNumBytesRead != sizeof(BOOLEAN)) return (FALSE);

  FileMan_Read(hFile, &gfResetAllPlayerKnowsEnemiesFlags, sizeof(BOOLEAN), &uiNumBytesRead);
  if (uiNumBytesRead != sizeof(BOOLEAN)) return (FALSE);

  FileMan_Read(hFile, &gfTimeCompressionOn, sizeof(BOOLEAN), &uiNumBytesRead);
  if (uiNumBytesRead != sizeof(BOOLEAN)) return (FALSE);

  FileMan_Read(hFile, &guiPreviousGameClock, sizeof(uint32_t), &uiNumBytesRead);
  if (uiNumBytesRead != sizeof(uint32_t)) return (FALSE);

  FileMan_Read(hFile, &guiLockPauseStateLastReasonId, sizeof(uint32_t), &uiNumBytesRead);
  if (uiNumBytesRead != sizeof(uint32_t)) return (FALSE);

  FileMan_Read(hFile, gubUnusedTimePadding, TIME_PADDINGBYTES, &uiNumBytesRead);
  if (uiNumBytesRead != TIME_PADDINGBYTES) return (FALSE);

  // Update the game clock
  guiDay = (guiGameClock / NUM_SEC_IN_DAY);
  guiHour = (guiGameClock - (guiDay * NUM_SEC_IN_DAY)) / NUM_SEC_IN_HOUR;
  guiMin =
      (guiGameClock - ((guiDay * NUM_SEC_IN_DAY) + (guiHour * NUM_SEC_IN_HOUR))) / NUM_SEC_IN_MIN;

  swprintf(WORLDTIMESTR, ARR_SIZE(WORLDTIMESTR), L"%s %d, %02d:%02d", pDayStrings[0], guiDay,
           guiHour, guiMin);

  if (!gfBasement && !gfCaves) gfDoLighting = TRUE;

  return (TRUE);
}

void CreateMouseRegionForPauseOfClock(int16_t sX, int16_t sY) {
  if (fClockMouseRegionCreated == FALSE) {
    // create a mouse region for pausing of game clock
    MSYS_DefineRegion(&gClockMouseRegion, (uint16_t)(sX), (uint16_t)(sY),
                      (uint16_t)(sX + CLOCK_REGION_WIDTH), (uint16_t)(sY + CLOCK_REGION_HEIGHT),
                      MSYS_PRIORITY_HIGHEST, MSYS_NO_CURSOR, MSYS_NO_CALLBACK,
                      PauseOfClockBtnCallback);

    fClockMouseRegionCreated = TRUE;

    if (gfGamePaused == FALSE) {
      SetRegionFastHelpText(&gClockMouseRegion, pPausedGameText[2]);
    } else {
      SetRegionFastHelpText(&gClockMouseRegion, pPausedGameText[1]);
    }
  }
}

void RemoveMouseRegionForPauseOfClock(void) {
  // remove pause region
  if (fClockMouseRegionCreated == TRUE) {
    MSYS_RemoveRegion(&gClockMouseRegion);
    fClockMouseRegionCreated = FALSE;
  }
}

void PauseOfClockBtnCallback(struct MOUSE_REGION* pRegion, int32_t iReason) {
  if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    HandlePlayerPauseUnPauseOfGame();
  }
}

void HandlePlayerPauseUnPauseOfGame(void) {
  if (gTacticalStatus.uiFlags & ENGAGED_IN_CONV) {
    return;
  }

  // check if the game is paused BY THE PLAYER or not and reverse
  if (gfGamePaused && gfPauseDueToPlayerGamePause) {
    // If in game screen...
    if (IsTacticalMode()) {
      if (giTimeCompressMode == TIME_COMPRESS_X0) {
        giTimeCompressMode++;
      }

      // ATE: re-render
      SetRenderFlags(RENDER_FLAG_FULL);
    }

    UnPauseGame();
    PauseTime(FALSE);
    gfIgnoreScrolling = FALSE;
    gfPauseDueToPlayerGamePause = FALSE;
  } else {
    // pause game
    PauseGame();
    PauseTime(TRUE);
    gfIgnoreScrolling = TRUE;
    gfPauseDueToPlayerGamePause = TRUE;
  }

  return;
}

void CreateDestroyScreenMaskForPauseGame(void) {
  static BOOLEAN fCreated = FALSE;
  int16_t sX = 0, sY = 0;

  if (((fClockMouseRegionCreated == FALSE) || (gfGamePaused == FALSE) ||
       (gfPauseDueToPlayerGamePause == FALSE)) &&
      (fCreated == TRUE)) {
    fCreated = FALSE;
    MSYS_RemoveRegion(&gClockScreenMaskMouseRegion);
    RemoveMercPopupBoxFromIndex(iPausedPopUpBox);
    iPausedPopUpBox = -1;
    SetRenderFlags(RENDER_FLAG_FULL);
    fTeamPanelDirty = TRUE;
    SetMapPanelDirty(true);
    fMapScreenBottomDirty = TRUE;
    gfJustFinishedAPause = TRUE;
    MarkButtonsDirty();
    SetRenderFlags(RENDER_FLAG_FULL);
  } else if ((gfPauseDueToPlayerGamePause == TRUE) && (fCreated == FALSE)) {
    // create a mouse region for pausing of game clock
    MSYS_DefineRegion(&gClockScreenMaskMouseRegion, 0, 0, 640, 480, MSYS_PRIORITY_HIGHEST, 0,
                      MSYS_NO_CALLBACK, ScreenMaskForGamePauseBtnCallBack);
    fCreated = TRUE;

    // get region x and y values
    sX = (&gClockMouseRegion)->RegionTopLeftX;
    sY = (&gClockMouseRegion)->RegionTopLeftY;

    // re create region on top of this
    RemoveMouseRegionForPauseOfClock();
    CreateMouseRegionForPauseOfClock(sX, sY);

    SetRegionFastHelpText(&gClockMouseRegion, pPausedGameText[1]);

    fMapScreenBottomDirty = TRUE;

    // UnMarkButtonsDirty( );

    // now create the pop up box to say the game is paused
    iPausedPopUpBox = PrepareMercPopupBox(iPausedPopUpBox, BASIC_MERC_POPUP_BACKGROUND,
                                          BASIC_MERC_POPUP_BORDER, pPausedGameText[0], 300, 0, 0, 0,
                                          &usPausedActualWidth, &usPausedActualHeight);
  }
}

void ScreenMaskForGamePauseBtnCallBack(struct MOUSE_REGION* pRegion, int32_t iReason) {
  if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    // unpause the game
    HandlePlayerPauseUnPauseOfGame();
  }
}

void RenderPausedGameBox(void) {
  if ((gfPauseDueToPlayerGamePause == TRUE) && (gfGamePaused == TRUE) && (iPausedPopUpBox != -1)) {
    RenderMercPopUpBoxFromIndex(iPausedPopUpBox, (int16_t)(320 - usPausedActualWidth / 2),
                                (int16_t)(200 - usPausedActualHeight / 2), FRAME_BUFFER);
    InvalidateRegion((int16_t)(320 - usPausedActualWidth / 2),
                     (int16_t)(200 - usPausedActualHeight / 2),
                     (int16_t)(320 - usPausedActualWidth / 2 + usPausedActualWidth),
                     (int16_t)(200 - usPausedActualHeight / 2 + usPausedActualHeight));
  }

  // reset we've just finished a pause by the player
  gfJustFinishedAPause = FALSE;
}

BOOLEAN DayTime() {  // between 7AM and 9PM
  return (guiHour >= 7 && guiHour < 21);
}

BOOLEAN NightTime() {  // before 7AM or after 9PM
  return (guiHour < 7 || guiHour >= 21);
}

void ClearTacticalStuffDueToTimeCompression(void) {
  // is this test the right thing?  ARM
  if (IsMapScreen()) {
    // clear tactical event queue
    ClearEventQueue();

    // clear tactical message queue
    ClearTacticalMessageQueue();

    if (gfWorldLoaded) {
      // clear tactical actions
      CencelAllActionsForTimeCompression();
    }
  }
}
