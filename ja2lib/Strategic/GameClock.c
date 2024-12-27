// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

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
extern BOOLEAN GameEventsPending(uint32_t uiAdjustment);
extern void ProcessPendingGameEvents(uint32_t uiAdjustment, uint8_t ubWarpCode);
void PauseOfClockBtnCallback(struct MOUSE_REGION* pRegion, int32_t iReason);
void ScreenMaskForGamePauseBtnCallBack(struct MOUSE_REGION* pRegion, int32_t iReason);

void CreateDestroyScreenMaskForPauseGame(void);

static void UpdateClockResolution() {
  UpdateClockResolutionRust();
  if (GetTimeCompressionOn()) {
    HandleTimeCompressWithTeamJackedInAndGearedToGo();
  }
}

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
void AdvanceClock(uint8_t ubWarpCode, u32 game_seconds);

#define SECONDS_PER_COMPRESSION_IN_RTCOMBAT 10
#define SECONDS_PER_COMPRESSION_IN_TBCOMBAT 10
#define CLOCK_STRING_HEIGHT 13
#define CLOCK_STRING_WIDTH 66
#define CLOCK_FONT COMPFONT

// These contain all of the information about the game time, rate of time, etc.
// All of these get saved and loaded.
BOOLEAN gfTimeInterrupt = FALSE;
BOOLEAN gfTimeInterruptPause = FALSE;
uint32_t guiPreviousGameClock = 0;  // used only for error-checking purposes
uint32_t guiTimesThisSecondProcessed = 0;
int32_t iPausedPopUpBox = -1;
wchar_t gswzWorldTimeStr[20];
uint16_t usPausedActualWidth;
uint16_t usPausedActualHeight;
uint32_t guiTimeOfLastEventQuery = 0;
BOOLEAN gfPauseDueToPlayerGamePause = FALSE;
BOOLEAN gfResetAllPlayerKnowsEnemiesFlags = FALSE;

//***When adding new saved time variables, make sure you remove the appropriate amount from the
// paddingbytes and
//   more IMPORTANTLY, add appropriate code in Save/LoadGameClock()!
#define TIME_PADDINGBYTES 20
uint8_t gubUnusedTimePadding[TIME_PADDINGBYTES];

extern uint32_t guiEnvTime;
extern uint32_t guiEnvDay;

static void UpdateGameClockString() {
  swprintf(gswzWorldTimeStr, ARR_SIZE(gswzWorldTimeStr), L"%s %d, %02d:%02d", pDayStrings[0],
           GetGameTimeInDays(), GetGameClockHour(), GetGameClockMinutes());
}

void UpdateGameClockString2() {
  swprintf(gswzWorldTimeStr, ARR_SIZE(gswzWorldTimeStr), L"%s %d, %02d:%02d",
           gpGameClockString[STR_GAMECLOCK_DAY_NAME], GetGameTimeInDays(), GetGameClockHour(),
           GetGameClockMinutes());
}

void InitNewGameClock() {
  InitNewGameClockRust();
  guiPreviousGameClock = GetGameStartingTime();
  UpdateGameClockString();
  guiTimeCurrentSectorWasLastLoaded = 0;
  memset(gubUnusedTimePadding, 0, TIME_PADDINGBYTES);
}

// Not to be used too often by things other than internally
void WarpGameTime(uint32_t uiAdjustment, uint8_t ubWarpCode) { AdvanceClock(ubWarpCode, uiAdjustment); }

void AdvanceClock(uint8_t ubWarpCode, u32 game_seconds) {
  if (ubWarpCode != WARPTIME_NO_PROCESSING_OF_EVENTS) {
    guiTimeOfLastEventQuery = GetGameTimeInSec();
    // First of all, events are posted for movements, pending attacks, equipment arrivals, etc. This
    // time adjustment using time compression can possibly pass one or more events in a single pass.
    // So, this list is looked at and processed in sequential order, until the uiAdjustment is fully
    // applied.
    if (GameEventsPending(game_seconds)) {
      // If a special event, justifying the cancellation of time compression is reached, the
      // adjustment will be shortened to the time of that event, and will stop processing events,
      // otherwise, all of the events in the time slice will be processed.  The time is adjusted
      // internally as events are processed.
      ProcessPendingGameEvents(game_seconds, ubWarpCode);
    } else {
      // Adjust the game clock now.
      MoveGameTimeForward(game_seconds);
    }
  } else {
    MoveGameTimeForward(game_seconds);
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

  UpdateGameClockString2();

  if (gfResetAllPlayerKnowsEnemiesFlags && !gTacticalStatus.fEnemyInSector) {
    ClearAnySectorsFlashingNumberOfEnemies();

    gfResetAllPlayerKnowsEnemiesFlags = FALSE;
  }

  ForecastDayEvents();
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

BOOLEAN DidGameJustStart() {
  if (gTacticalStatus.fDidGameJustStart)
    return (TRUE);
  else
    return (FALSE);
}

void StartTimeCompression(void) {
  if (!GetTimeCompressionOn()) {
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
    if (GetTimeCompressMode() <= TIME_COMPRESS_X1) {
      IncreaseGameTimeCompressionRate();
    }

    // change clock resolution to the current compression mode
    UpdateClockResolution();

    // if it's the first time we're doing this since entering map screen (which reset the flag)
    if (!HasTimeCompressOccured()) {
      // set fact that we have compressed time during this map screen session
      fTimeCompressHasOccured = TRUE;
      ClearTacticalStuffDueToTimeCompression();
    }
  }
}

void IncreaseGameTimeCompressionRate() {
  // if not already at maximum time compression rate
  if (GetTimeCompressMode() < TIME_COMPRESS_60MINS) {
    // check that we can
    if (!AllowedToTimeCompress()) {
      // not allowed to compress time
      TellPlayerWhyHeCantCompressTime();
      return;
    }

    IncTimeCompressMode();

    // in map screen, we wanna have to skip over x1 compression and go straight to 5x
    if ((IsMapScreen_2()) && (GetTimeCompressMode() == TIME_COMPRESS_X1)) {
      IncTimeCompressMode();
    }

    UpdateClockResolution();
  }
}

void DecreaseGameTimeCompressionRate() {
  // if not already at minimum time compression rate
  if (GetTimeCompressMode() > TIME_COMPRESS_X0) {
    // check that we can
    if (!AllowedToTimeCompress()) {
      // not allowed to compress time
      TellPlayerWhyHeCantCompressTime();
      return;
    }

    DecTimeCompressMode();

    // in map screen, we wanna have to skip over x1 compression and go straight to 5x
    if ((IsMapScreen_2()) && (GetTimeCompressMode() == TIME_COMPRESS_X1)) {
      DecTimeCompressMode();
    }

    UpdateClockResolution();
  }
}

void SetGameTimeCompressionLevel(enum TIME_COMPRESS_MODE compress_mode) {
  if (IsTacticalMode()) {
    if (compress_mode != TIME_COMPRESS_X1) {
      compress_mode = TIME_COMPRESS_X1;
    }
  }

  if (IsMapScreen_2()) {
    if (compress_mode == TIME_COMPRESS_X1) {
      compress_mode = TIME_COMPRESS_X0;
    }
  }

  // if we're attempting time compression
  if (compress_mode >= TIME_COMPRESS_5MINS) {
    // check that we can
    if (!AllowedToTimeCompress()) {
      // not allowed to compress time
      TellPlayerWhyHeCantCompressTime();
      return;
    }
  }

  SetTimeCompressMode(compress_mode);
  UpdateClockResolution();
}

// ONLY APPLICABLE INSIDE EVENT CALLBACKS!
void InterruptTime() { gfTimeInterrupt = TRUE; }

void PauseTimeForInterupt() { gfTimeInterruptPause = TRUE; }

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

  u32 seconds_per_real_second = GetGameSecondsPerRealSecond();

  // check game state for pause screen masks
  CreateDestroyScreenMaskForPauseGame();

  if (guiCurrentScreen != GAME_SCREEN && !IsMapScreen_2() && guiCurrentScreen != GAME_SCREEN) {
    uiLastSecondTime = GetJA2Clock();
    gfTimeInterruptPause = FALSE;
    return;
  }

  if (IsGamePaused() || gfTimeInterruptPause || (GetClockResolution() == 0) ||
      !seconds_per_real_second || ARE_IN_FADE_IN() || gfFadeOut) {
    uiLastSecondTime = GetJA2Clock();
    gfTimeInterruptPause = FALSE;
    return;
  }

  if ((gTacticalStatus.uiFlags & TURNBASED && gTacticalStatus.uiFlags & INCOMBAT))
    return;  // time is currently stopped!

  uiNewTime = GetJA2Clock();

  // Because we debug so much, breakpoints tend to break the game, and cause unnecessary headaches.
  // This line ensures that no more than 1 real-second passes between frames.  This otherwise has
  // no effect on anything else.
  uiLastSecondTime = max(uiNewTime - 1000, uiLastSecondTime);

  // 1000's of a second difference since last second.
  uiThousandthsOfThisSecondProcessed = uiNewTime - uiLastSecondTime;

  if (uiThousandthsOfThisSecondProcessed >= 1000 && GetClockResolution() == 1) {
    uiLastSecondTime = uiNewTime;
    guiTimesThisSecondProcessed = uiLastTimeProcessed = 0;
    AdvanceClock(WARPTIME_PROCESS_EVENTS_NORMALLY, seconds_per_real_second);
  } else if (GetClockResolution() > 1) {
    if (GetClockResolution() != ubLastResolution) {
      guiTimesThisSecondProcessed =
          guiTimesThisSecondProcessed * GetClockResolution() / ubLastResolution;
      uiLastTimeProcessed = uiLastTimeProcessed * GetClockResolution() / ubLastResolution;
      ubLastResolution = GetClockResolution();
    }
    uiTimeSlice = 1000000 / GetClockResolution();
    if (uiThousandthsOfThisSecondProcessed >=
        uiTimeSlice * (guiTimesThisSecondProcessed + 1) / 1000) {
      guiTimesThisSecondProcessed = uiThousandthsOfThisSecondProcessed * 1000 / uiTimeSlice;
      uiNewTimeProcessed =
          seconds_per_real_second * guiTimesThisSecondProcessed / GetClockResolution();

      uiNewTimeProcessed = max(uiNewTimeProcessed, uiLastTimeProcessed);

      WarpGameTime(uiNewTimeProcessed - uiLastTimeProcessed, WARPTIME_PROCESS_EVENTS_NORMALLY);
      if (uiNewTimeProcessed < seconds_per_real_second) {  // Processed the same real second
        uiLastTimeProcessed = uiNewTimeProcessed;
      } else {  // We have moved into a new real second.
        uiLastTimeProcessed = uiNewTimeProcessed % seconds_per_real_second;
        if (GetClockResolution() > 0) {
          guiTimesThisSecondProcessed %= GetClockResolution();
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
  uint32_t uiNumBytesWritten = 0;

  {
    i32 time_compress_mode = GetTimeCompressMode();
    File_Write(hFile, &time_compress_mode, sizeof(int32_t), &uiNumBytesWritten);
    if (uiNumBytesWritten != sizeof(int32_t)) return (FALSE);
  }

  {
    u8 clock_resolution = GetClockResolution();
    File_Write(hFile, &clock_resolution, sizeof(uint8_t), &uiNumBytesWritten);
    if (uiNumBytesWritten != sizeof(uint8_t)) return (FALSE);
  }

  File_Write(hFile, &fGamePaused, sizeof(BOOLEAN), &uiNumBytesWritten);
  if (uiNumBytesWritten != sizeof(BOOLEAN)) return (FALSE);

  File_Write(hFile, &gfTimeInterrupt, sizeof(BOOLEAN), &uiNumBytesWritten);
  if (uiNumBytesWritten != sizeof(BOOLEAN)) return (FALSE);

  {
    BOOLEAN fSuperCompression = FALSE;
    File_Write(hFile, &fSuperCompression, sizeof(BOOLEAN), &uiNumBytesWritten);
    if (uiNumBytesWritten != sizeof(BOOLEAN)) return (FALSE);
  }

  {
    i32 gameClock = GetGameTimeInSec();
    File_Write(hFile, &gameClock, sizeof(uint32_t), &uiNumBytesWritten);
    if (uiNumBytesWritten != sizeof(uint32_t)) return (FALSE);
  }

  {
    u32 seconds_per_real_second = GetGameSecondsPerRealSecond();
    File_Write(hFile, &seconds_per_real_second, sizeof(uint32_t), &uiNumBytesWritten);
    if (uiNumBytesWritten != sizeof(uint32_t)) return (FALSE);
  }

  File_Write(hFile, &ubAmbientLightLevel, sizeof(uint8_t), &uiNumBytesWritten);
  if (uiNumBytesWritten != sizeof(uint8_t)) return (FALSE);

  File_Write(hFile, &guiEnvTime, sizeof(uint32_t), &uiNumBytesWritten);
  if (uiNumBytesWritten != sizeof(uint32_t)) return (FALSE);

  File_Write(hFile, &guiEnvDay, sizeof(uint32_t), &uiNumBytesWritten);
  if (uiNumBytesWritten != sizeof(uint32_t)) return (FALSE);

  File_Write(hFile, &gubEnvLightValue, sizeof(uint8_t), &uiNumBytesWritten);
  if (uiNumBytesWritten != sizeof(uint8_t)) return (FALSE);

  File_Write(hFile, &guiTimeOfLastEventQuery, sizeof(uint32_t), &uiNumBytesWritten);
  if (uiNumBytesWritten != sizeof(uint32_t)) return (FALSE);

  File_Write(hFile, &fLockPauseState, sizeof(BOOLEAN), &uiNumBytesWritten);
  if (uiNumBytesWritten != sizeof(BOOLEAN)) return (FALSE);

  File_Write(hFile, &gfPauseDueToPlayerGamePause, sizeof(BOOLEAN), &uiNumBytesWritten);
  if (uiNumBytesWritten != sizeof(BOOLEAN)) return (FALSE);

  File_Write(hFile, &gfResetAllPlayerKnowsEnemiesFlags, sizeof(BOOLEAN), &uiNumBytesWritten);
  if (uiNumBytesWritten != sizeof(BOOLEAN)) return (FALSE);

  {
    BOOLEAN time_compression_on = GetTimeCompressionOn();
    File_Write(hFile, &time_compression_on, sizeof(BOOLEAN), &uiNumBytesWritten);
    if (uiNumBytesWritten != sizeof(BOOLEAN)) return (FALSE);
  }

  File_Write(hFile, &guiPreviousGameClock, sizeof(uint32_t), &uiNumBytesWritten);
  if (uiNumBytesWritten != sizeof(uint32_t)) return (FALSE);

  {
    uint32_t guiLockPauseStateLastReasonId = 0;
    File_Write(hFile, &guiLockPauseStateLastReasonId, sizeof(uint32_t), &uiNumBytesWritten);
    if (uiNumBytesWritten != sizeof(uint32_t)) return (FALSE);
  }

  File_Write(hFile, gubUnusedTimePadding, TIME_PADDINGBYTES, &uiNumBytesWritten);
  if (uiNumBytesWritten != TIME_PADDINGBYTES) return (FALSE);
  return (TRUE);
}

BOOLEAN LoadGameClock(FileID hFile) {
  struct SavedClockStateC state;
  bool res = LoadSavedClockState(hFile, &state);
  if (!res) {
    return false;
  }

  gfTimeInterrupt = state.TimeInterrupt;
  ubAmbientLightLevel = state.AmbientLightLevel;
  guiEnvTime = state.EnvTime;
  guiEnvDay = state.EnvDay;
  gubEnvLightValue = state.EnvLightValue;
  guiTimeOfLastEventQuery = state.TimeOfLastEventQuery;
  gfResetAllPlayerKnowsEnemiesFlags = state.ResetAllPlayerKnowsEnemiesFlags;
  guiPreviousGameClock = state.PreviousGameClock;

  UpdateGameClockString();

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
  if (IsGamePaused() && gfPauseDueToPlayerGamePause) {
    // If in game screen...
    if (IsTacticalMode()) {
      if (GetTimeCompressMode() == TIME_COMPRESS_X0) {
        IncTimeCompressMode();
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

void ScreenMaskForGamePauseBtnCallBack(struct MOUSE_REGION* pRegion, int32_t iReason) {
  if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    // unpause the game
    HandlePlayerPauseUnPauseOfGame();
  }
}

void RenderPausedGameBox(void) {
  if ((gfPauseDueToPlayerGamePause == TRUE) && IsGamePaused() && (iPausedPopUpBox != -1)) {
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
