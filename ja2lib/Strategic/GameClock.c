#include "Strategic/GameClock.h"

#include "FadeScreen.h"
#include "GameScreen.h"
#include "JAScreens.h"
#include "SGP/ButtonSystem.h"
#include "SGP/Debug.h"
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
#include "rust_fileman.h"

// #define DEBUG_GAME_CLOCK

extern BOOLEAN gfFadeOut;

// These functions shouldn't be used anywhere else...
extern BOOLEAN GameEventsPending(UINT32 uiAdjustment);
extern void ProcessPendingGameEvents(UINT32 uiAdjustment, UINT8 ubWarpCode);
void PauseOfClockBtnCallback(struct MOUSE_REGION* pRegion, INT32 iReason);
void ScreenMaskForGamePauseBtnCallBack(struct MOUSE_REGION* pRegion, INT32 iReason);

void CreateDestroyScreenMaskForPauseGame(void);

void SetClockResolutionToCompressMode(INT32 iCompressMode);

// is the clock pause region created currently?
BOOLEAN fClockMouseRegionCreated = FALSE;

BOOLEAN fTimeCompressHasOccured = FALSE;

// This value represents the time that the sector was loaded.  If you are in sector A9, and leave
// the game clock at that moment will get saved into the temp file associated with it.  The next
// time you enter A9, this value will contain that time.  Used for scheduling purposes.
UINT32 guiTimeCurrentSectorWasLastLoaded = 0;

// did we JUST finish up a game pause by the player
BOOLEAN gfJustFinishedAPause = FALSE;

// clock mouse region
struct MOUSE_REGION gClockMouseRegion;
struct MOUSE_REGION gClockScreenMaskMouseRegion;
void AdvanceClock(UINT8 ubWarpCode);

#define SECONDS_PER_COMPRESSION 1  // 1/2 minute passes every 1 second of real time
#define SECONDS_PER_COMPRESSION_IN_RTCOMBAT 10
#define SECONDS_PER_COMPRESSION_IN_TBCOMBAT 10
#define CLOCK_STRING_HEIGHT 13
#define CLOCK_STRING_WIDTH 66
#define CLOCK_FONT COMPFONT

// These contain all of the information about the game time, rate of time, etc.
// All of these get saved and loaded.
INT32 giTimeCompressMode = TIME_COMPRESS_X0;
UINT8 gubClockResolution = 1;
BOOLEAN gfTimeInterrupt = FALSE;
BOOLEAN gfTimeInterruptPause = FALSE;
BOOLEAN fSuperCompression = FALSE;
UINT32 guiPreviousGameClock = 0;  // used only for error-checking purposes
UINT32 guiGameSecondsPerRealSecond;
UINT32 guiTimesThisSecondProcessed = 0;
INT32 iPausedPopUpBox = -1;
CHAR16 gswzWorldTimeStr[20];
INT32 giTimeCompressSpeeds[NUM_TIME_COMPRESS_SPEEDS] = {0, 1, 5 * 60, 30 * 60, 60 * 60};
UINT16 usPausedActualWidth;
UINT16 usPausedActualHeight;
UINT32 guiTimeOfLastEventQuery = 0;
BOOLEAN gfPauseDueToPlayerGamePause = FALSE;
BOOLEAN gfResetAllPlayerKnowsEnemiesFlags = FALSE;
BOOLEAN gfTimeCompressionOn = FALSE;

//***When adding new saved time variables, make sure you remove the appropriate amount from the
// paddingbytes and
//   more IMPORTANTLY, add appropriate code in Save/LoadGameClock()!
#define TIME_PADDINGBYTES 20
UINT8 gubUnusedTimePadding[TIME_PADDINGBYTES];

extern UINT32 guiEnvTime;
extern UINT32 guiEnvDay;

void InitNewGameClock() {
  SetGameTimeSec(GetGameStartingTime());
  guiPreviousGameClock = GetGameStartingTime();
  swprintf(gswzWorldTimeStr, ARR_SIZE(gswzWorldTimeStr), L"%s %d, %02d:%02d", pDayStrings[0],
           GetGameTimeInDays(), GetGameClockHour(), GetGameClockMinutes());
  guiTimeCurrentSectorWasLastLoaded = 0;
  guiGameSecondsPerRealSecond = 0;
  gubClockResolution = 1;
  memset(gubUnusedTimePadding, 0, TIME_PADDINGBYTES);
}

// Not to be used too often by things other than internally
void WarpGameTime(UINT32 uiAdjustment, UINT8 ubWarpCode) {
  UINT32 uiSaveTimeRate;
  uiSaveTimeRate = guiGameSecondsPerRealSecond;
  guiGameSecondsPerRealSecond = uiAdjustment;
  AdvanceClock(ubWarpCode);
  guiGameSecondsPerRealSecond = uiSaveTimeRate;
}

void AdvanceClock(UINT8 ubWarpCode) {
  // Set value, to different things if we are in combat...
  if ((gTacticalStatus.uiFlags & INCOMBAT)) {
    if ((gTacticalStatus.uiFlags & TURNBASED)) {
    } else {
    }
  }

  if (ubWarpCode != WARPTIME_NO_PROCESSING_OF_EVENTS) {
    guiTimeOfLastEventQuery = GetGameTimeInSec();
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
      MoveGameTimeForward(guiGameSecondsPerRealSecond);
    }
  } else {
    MoveGameTimeForward(guiGameSecondsPerRealSecond);
  }

  if (GetGameTimeInSec() < guiPreviousGameClock) {
    AssertMsg(FALSE,
              String("AdvanceClock: TIME FLOWING BACKWARDS!!! guiPreviousGameClock %d, now %d",
                     guiPreviousGameClock, GetGameTimeInSec()));

    // fix it if assertions are disabled
    SetGameTimeSec(guiPreviousGameClock);
  }

  // store previous game clock value (for error-checking purposes only)
  guiPreviousGameClock = GetGameTimeInSec();

  swprintf(gswzWorldTimeStr, ARR_SIZE(gswzWorldTimeStr), L"%s %d, %02d:%02d",
           gpGameClockString[STR_GAMECLOCK_DAY_NAME], GetGameTimeInDays(), GetGameClockHour(),
           GetGameClockMinutes());

  if (gfResetAllPlayerKnowsEnemiesFlags && !gTacticalStatus.fEnemyInSector) {
    ClearAnySectorsFlashingNumberOfEnemies();

    gfResetAllPlayerKnowsEnemiesFlags = FALSE;
  }

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

void RenderClock(INT16 sX, INT16 sY) {
  SetFont(CLOCK_FONT);
  SetFontBackground(FONT_MCOLOR_BLACK);

  // Are we in combat?
  if (gTacticalStatus.uiFlags & INCOMBAT) {
    SetFontForeground(FONT_FCOLOR_NICERED);
  } else {
    SetFontForeground(FONT_LTGREEN);
  }

  // Erase first!
  RestoreExternBackgroundRect(sX, sY, CLOCK_STRING_WIDTH, CLOCK_STRING_HEIGHT);

  if ((gfPauseDueToPlayerGamePause == FALSE)) {
    mprintf(sX + (CLOCK_STRING_WIDTH - StringPixLength(gswzWorldTimeStr, CLOCK_FONT)) / 2, sY,
            gswzWorldTimeStr);
  } else {
    mprintf(sX + (CLOCK_STRING_WIDTH - StringPixLength(pPausedGameText[0], CLOCK_FONT)) / 2, sY,
            pPausedGameText[0]);
  }
}

void ToggleSuperCompression() {
  static UINT32 uiOldTimeCompressMode = 0;

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
    if (IsGamePaused()) {
      // first have to be allowed to unpause the game
      UnPauseGame();

      // if we couldn't, ignore this request
      if (IsGamePaused()) {
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
  if (!gfTimeCompressionOn || (giTimeCompressMode == TIME_COMPRESS_X0) || IsGamePaused())
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

void SetGameTimeCompressionLevel(UINT32 uiCompressionRate) {
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

void SetClockResolutionToCompressMode(INT32 iCompressMode) {
  guiGameSecondsPerRealSecond = giTimeCompressSpeeds[iCompressMode] * SECONDS_PER_COMPRESSION;

  // ok this is a bit confusing, but for time compression (e.g. 30x60) we want updates
  // 30x per second, but for standard unpaused time, like in tactical, we want 1x per second
  if (guiGameSecondsPerRealSecond == 0) {
    SetClockResolutionPerSecond(0);
  } else {
    SetClockResolutionPerSecond((UINT8)max(1, (UINT8)(guiGameSecondsPerRealSecond / 60)));
  }

  // if the compress mode is X0 or X1
  if (iCompressMode <= TIME_COMPRESS_X1) {
    gfTimeCompressionOn = FALSE;
  } else {
    gfTimeCompressionOn = TRUE;

    // handle the player just starting a game
    HandleTimeCompressWithTeamJackedInAndGearedToGo();
  }

  SetMapScreenBottomDirty(true);
}

void SetGameHoursPerSecond(UINT32 uiGameHoursPerSecond) {
  giTimeCompressMode = NOT_USING_TIME_COMPRESSION;
  guiGameSecondsPerRealSecond = uiGameHoursPerSecond * 3600;
  if (uiGameHoursPerSecond == 1) {
    SetClockResolutionPerSecond(60);
  } else {
    SetClockResolutionPerSecond(59);
  }
}

void SetGameMinutesPerSecond(UINT32 uiGameMinutesPerSecond) {
  giTimeCompressMode = NOT_USING_TIME_COMPRESSION;
  guiGameSecondsPerRealSecond = uiGameMinutesPerSecond * 60;
  SetClockResolutionPerSecond((UINT8)uiGameMinutesPerSecond);
}

void SetGameSecondsPerSecond(UINT32 uiGameSecondsPerSecond) {
  giTimeCompressMode = NOT_USING_TIME_COMPRESSION;
  guiGameSecondsPerRealSecond = uiGameSecondsPerSecond;
  //	SetClockResolutionPerSecond( (UINT8)(guiGameSecondsPerRealSecond / 60) );
  if (guiGameSecondsPerRealSecond == 0) {
    SetClockResolutionPerSecond(0);
  } else {
    SetClockResolutionPerSecond((UINT8)max(1, (UINT8)(guiGameSecondsPerRealSecond / 60)));
  }
}

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
void SetClockResolutionPerSecond(UINT8 ubNumTimesPerSecond) {
  ubNumTimesPerSecond = (UINT8)(max(0, min(60, ubNumTimesPerSecond)));
  gubClockResolution = ubNumTimesPerSecond;
}

// Function for accessing the current rate
UINT8 ClockResolution() { return gubClockResolution; }

// There are two factors that influence the flow of time in the game.
//-Speed:  The speed is the amount of game time passes per real second of time.  The higher this
//         value, the faster the game time flows.
//-Resolution:  The higher the resolution, the more often per second the clock is actually updated.
//				 This value doesn't affect how much game time passes per real
// second, but allows for 				 a more accurate representation of faster
// time
// flows.
void UpdateClock() {
  UINT32 uiNewTime;
  UINT32 uiThousandthsOfThisSecondProcessed;
  UINT32 uiTimeSlice;
  UINT32 uiNewTimeProcessed;
  static UINT8 ubLastResolution = 1;
  static UINT32 uiLastSecondTime = 0;
  static UINT32 uiLastTimeProcessed = 0;
#ifdef DEBUG_GAME_CLOCK
  UINT32 uiOrigNewTime;
  UINT32 uiOrigLastSecondTime;
  UINT32 uiOrigThousandthsOfThisSecondProcessed;
  UINT8 ubOrigClockResolution;
  UINT32 uiOrigTimesThisSecondProcessed;
  UINT8 ubOrigLastResolution;
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

  if (IsGamePaused() || gfTimeInterruptPause || (gubClockResolution == 0) ||
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
          GetGameTimeInSec() + uiAmountToAdvanceTime < guiPreviousGameClock) {
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

BOOLEAN SaveGameClock(FileID hFile, BOOLEAN fGamePaused, BOOLEAN fLockPauseState) {
  UINT32 uiNumBytesWritten = 0;

  File_Write(hFile, &giTimeCompressMode, sizeof(INT32), &uiNumBytesWritten);
  if (uiNumBytesWritten != sizeof(INT32)) return (FALSE);

  File_Write(hFile, &gubClockResolution, sizeof(UINT8), &uiNumBytesWritten);
  if (uiNumBytesWritten != sizeof(UINT8)) return (FALSE);

  File_Write(hFile, &fGamePaused, sizeof(BOOLEAN), &uiNumBytesWritten);
  if (uiNumBytesWritten != sizeof(BOOLEAN)) return (FALSE);

  File_Write(hFile, &gfTimeInterrupt, sizeof(BOOLEAN), &uiNumBytesWritten);
  if (uiNumBytesWritten != sizeof(BOOLEAN)) return (FALSE);

  File_Write(hFile, &fSuperCompression, sizeof(BOOLEAN), &uiNumBytesWritten);
  if (uiNumBytesWritten != sizeof(BOOLEAN)) return (FALSE);

  {
    i32 gameClock = GetGameTimeInSec();
    File_Write(hFile, &gameClock, sizeof(UINT32), &uiNumBytesWritten);
    if (uiNumBytesWritten != sizeof(UINT32)) return (FALSE);
  }

  File_Write(hFile, &guiGameSecondsPerRealSecond, sizeof(UINT32), &uiNumBytesWritten);
  if (uiNumBytesWritten != sizeof(UINT32)) return (FALSE);

  File_Write(hFile, &ubAmbientLightLevel, sizeof(UINT8), &uiNumBytesWritten);
  if (uiNumBytesWritten != sizeof(UINT8)) return (FALSE);

  File_Write(hFile, &guiEnvTime, sizeof(UINT32), &uiNumBytesWritten);
  if (uiNumBytesWritten != sizeof(UINT32)) return (FALSE);

  File_Write(hFile, &guiEnvDay, sizeof(UINT32), &uiNumBytesWritten);
  if (uiNumBytesWritten != sizeof(UINT32)) return (FALSE);

  File_Write(hFile, &gubEnvLightValue, sizeof(UINT8), &uiNumBytesWritten);
  if (uiNumBytesWritten != sizeof(UINT8)) return (FALSE);

  File_Write(hFile, &guiTimeOfLastEventQuery, sizeof(UINT32), &uiNumBytesWritten);
  if (uiNumBytesWritten != sizeof(UINT32)) return (FALSE);

  File_Write(hFile, &fLockPauseState, sizeof(BOOLEAN), &uiNumBytesWritten);
  if (uiNumBytesWritten != sizeof(BOOLEAN)) return (FALSE);

  File_Write(hFile, &gfPauseDueToPlayerGamePause, sizeof(BOOLEAN), &uiNumBytesWritten);
  if (uiNumBytesWritten != sizeof(BOOLEAN)) return (FALSE);

  File_Write(hFile, &gfResetAllPlayerKnowsEnemiesFlags, sizeof(BOOLEAN), &uiNumBytesWritten);
  if (uiNumBytesWritten != sizeof(BOOLEAN)) return (FALSE);

  File_Write(hFile, &gfTimeCompressionOn, sizeof(BOOLEAN), &uiNumBytesWritten);
  if (uiNumBytesWritten != sizeof(BOOLEAN)) return (FALSE);

  File_Write(hFile, &guiPreviousGameClock, sizeof(UINT32), &uiNumBytesWritten);
  if (uiNumBytesWritten != sizeof(UINT32)) return (FALSE);

  {
    UINT32 guiLockPauseStateLastReasonId = 0;
    File_Write(hFile, &guiLockPauseStateLastReasonId, sizeof(UINT32), &uiNumBytesWritten);
    if (uiNumBytesWritten != sizeof(UINT32)) return (FALSE);
  }

  File_Write(hFile, gubUnusedTimePadding, TIME_PADDINGBYTES, &uiNumBytesWritten);
  if (uiNumBytesWritten != TIME_PADDINGBYTES) return (FALSE);
  return (TRUE);
}

BOOLEAN LoadGameClock(FileID hFile) {
  UINT32 uiNumBytesRead;

  File_Read(hFile, &giTimeCompressMode, sizeof(INT32), &uiNumBytesRead);
  if (uiNumBytesRead != sizeof(INT32)) return (FALSE);

  File_Read(hFile, &gubClockResolution, sizeof(UINT8), &uiNumBytesRead);
  if (uiNumBytesRead != sizeof(UINT8)) return (FALSE);

  {
    BOOLEAN gamePaused = FALSE;
    File_Read(hFile, &gamePaused, sizeof(BOOLEAN), &uiNumBytesRead);
    if (uiNumBytesRead != sizeof(BOOLEAN)) return (FALSE);
    if (gamePaused) {
      PauseGame();
    }
  }

  File_Read(hFile, &gfTimeInterrupt, sizeof(BOOLEAN), &uiNumBytesRead);
  if (uiNumBytesRead != sizeof(BOOLEAN)) return (FALSE);

  File_Read(hFile, &fSuperCompression, sizeof(BOOLEAN), &uiNumBytesRead);
  if (uiNumBytesRead != sizeof(BOOLEAN)) return (FALSE);

  {
    u32 gameClock = 0;
    File_Read(hFile, &gameClock, sizeof(UINT32), &uiNumBytesRead);
    if (uiNumBytesRead != sizeof(UINT32)) return (FALSE);
    SetGameTimeSec(gameClock);
  }

  File_Read(hFile, &guiGameSecondsPerRealSecond, sizeof(UINT32), &uiNumBytesRead);
  if (uiNumBytesRead != sizeof(UINT32)) return (FALSE);

  File_Read(hFile, &ubAmbientLightLevel, sizeof(UINT8), &uiNumBytesRead);
  if (uiNumBytesRead != sizeof(UINT8)) return (FALSE);

  File_Read(hFile, &guiEnvTime, sizeof(UINT32), &uiNumBytesRead);
  if (uiNumBytesRead != sizeof(UINT32)) return (FALSE);

  File_Read(hFile, &guiEnvDay, sizeof(UINT32), &uiNumBytesRead);
  if (uiNumBytesRead != sizeof(UINT32)) return (FALSE);

  File_Read(hFile, &gubEnvLightValue, sizeof(UINT8), &uiNumBytesRead);
  if (uiNumBytesRead != sizeof(UINT8)) return (FALSE);

  File_Read(hFile, &guiTimeOfLastEventQuery, sizeof(UINT32), &uiNumBytesRead);
  if (uiNumBytesRead != sizeof(UINT32)) return (FALSE);

  {
    BOOLEAN lockedPause = FALSE;
    File_Read(hFile, &lockedPause, sizeof(BOOLEAN), &uiNumBytesRead);
    if (uiNumBytesRead != sizeof(BOOLEAN)) return (FALSE);
    if (lockedPause) {
      LockPause();
    }
  }

  File_Read(hFile, &gfPauseDueToPlayerGamePause, sizeof(BOOLEAN), &uiNumBytesRead);
  if (uiNumBytesRead != sizeof(BOOLEAN)) return (FALSE);

  File_Read(hFile, &gfResetAllPlayerKnowsEnemiesFlags, sizeof(BOOLEAN), &uiNumBytesRead);
  if (uiNumBytesRead != sizeof(BOOLEAN)) return (FALSE);

  File_Read(hFile, &gfTimeCompressionOn, sizeof(BOOLEAN), &uiNumBytesRead);
  if (uiNumBytesRead != sizeof(BOOLEAN)) return (FALSE);

  File_Read(hFile, &guiPreviousGameClock, sizeof(UINT32), &uiNumBytesRead);
  if (uiNumBytesRead != sizeof(UINT32)) return (FALSE);

  {
    UINT32 guiLockPauseStateLastReasonId = 0;
    File_Read(hFile, &guiLockPauseStateLastReasonId, sizeof(UINT32), &uiNumBytesRead);
    if (uiNumBytesRead != sizeof(UINT32)) return (FALSE);
  }

  File_Read(hFile, gubUnusedTimePadding, TIME_PADDINGBYTES, &uiNumBytesRead);
  if (uiNumBytesRead != TIME_PADDINGBYTES) return (FALSE);

  swprintf(gswzWorldTimeStr, ARR_SIZE(gswzWorldTimeStr), L"%s %d, %02d:%02d", pDayStrings[0],
           GetGameTimeInDays(), GetGameClockHour(), GetGameClockMinutes());

  if (!gfBasement && !gfCaves) gfDoLighting = TRUE;

  return (TRUE);
}

void CreateMouseRegionForPauseOfClock(INT16 sX, INT16 sY) {
  if (fClockMouseRegionCreated == FALSE) {
    // create a mouse region for pausing of game clock
    MSYS_DefineRegion(&gClockMouseRegion, (UINT16)(sX), (UINT16)(sY),
                      (UINT16)(sX + CLOCK_REGION_WIDTH), (UINT16)(sY + CLOCK_REGION_HEIGHT),
                      MSYS_PRIORITY_HIGHEST, MSYS_NO_CURSOR, MSYS_NO_CALLBACK,
                      PauseOfClockBtnCallback);

    fClockMouseRegionCreated = TRUE;

    if (!IsGamePaused()) {
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

void PauseOfClockBtnCallback(struct MOUSE_REGION* pRegion, INT32 iReason) {
  if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    HandlePlayerPauseUnPauseOfGame();
  }
}

void HandlePlayerPauseUnPauseOfGame(void) {
  if (gTacticalStatus.uiFlags & ENGAGED_IN_CONV) {
    return;
  }

  // check if the game is paused BY THE PLAYER or not and reverse
  if (IsGamePaused() && gfPauseDueToPlayerGamePause) {
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
  INT16 sX = 0, sY = 0;

  if (((fClockMouseRegionCreated == FALSE) || (!IsGamePaused()) ||
       (gfPauseDueToPlayerGamePause == FALSE)) &&
      (fCreated == TRUE)) {
    fCreated = FALSE;
    MSYS_RemoveRegion(&gClockScreenMaskMouseRegion);
    RemoveMercPopupBoxFromIndex(iPausedPopUpBox);
    iPausedPopUpBox = -1;
    SetRenderFlags(RENDER_FLAG_FULL);
    fTeamPanelDirty = TRUE;
    SetMapPanelDirty(true);
    SetMapScreenBottomDirty(true);
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

    SetMapScreenBottomDirty(true);

    // UnMarkButtonsDirty( );

    // now create the pop up box to say the game is paused
    iPausedPopUpBox = PrepareMercPopupBox(iPausedPopUpBox, BASIC_MERC_POPUP_BACKGROUND,
                                          BASIC_MERC_POPUP_BORDER, pPausedGameText[0], 300, 0, 0, 0,
                                          &usPausedActualWidth, &usPausedActualHeight);
  }
}

void ScreenMaskForGamePauseBtnCallBack(struct MOUSE_REGION* pRegion, INT32 iReason) {
  if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    // unpause the game
    HandlePlayerPauseUnPauseOfGame();
  }
}

void RenderPausedGameBox(void) {
  if ((gfPauseDueToPlayerGamePause == TRUE) && IsGamePaused() && (iPausedPopUpBox != -1)) {
    RenderMercPopUpBoxFromIndex(iPausedPopUpBox, (INT16)(320 - usPausedActualWidth / 2),
                                (INT16)(200 - usPausedActualHeight / 2), FRAME_BUFFER);
    InvalidateRegion((INT16)(320 - usPausedActualWidth / 2),
                     (INT16)(200 - usPausedActualHeight / 2),
                     (INT16)(320 - usPausedActualWidth / 2 + usPausedActualWidth),
                     (INT16)(200 - usPausedActualHeight / 2 + usPausedActualHeight));
  }

  // reset we've just finished a pause by the player
  gfJustFinishedAPause = FALSE;
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
