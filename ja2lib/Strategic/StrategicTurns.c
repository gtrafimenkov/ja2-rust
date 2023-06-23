#include "Strategic/StrategicTurns.h"

#include "JAScreens.h"
#include "SGP/Types.h"
#include "ScreenIDs.h"
#include "Strategic/Assignments.h"
#include "Strategic/GameClock.h"
#include "Tactical/AnimationControl.h"
#include "Tactical/AnimationData.h"
#include "Tactical/Overhead.h"
#include "Tactical/RTTimeDefines.h"
#include "Tactical/RottingCorpses.h"
#include "Tactical/SoldierAdd.h"
#include "Tactical/TacticalTurns.h"
#include "TileEngine/Environment.h"
#include "TileEngine/IsometricUtils.h"
#include "UI.h"
#include "Utils/FontControl.h"
#include "Utils/Message.h"
#include "Utils/TimerControl.h"

#define NUM_SEC_PER_STRATEGIC_TURN (NUM_SEC_IN_MIN * 15)  // Every fifteen minutes

UINT32 guiLastStrategicTime = 0;
UINT32 guiLastTacticalRealTime = 0;

void StrategicTurnsNewGame() {
  // Sync game start time
  SyncStrategicTurnTimes();
}

void SyncStrategicTurnTimes() {
  guiLastStrategicTime = GetGameTimeInSec();
  guiLastTacticalRealTime = GetJA2Clock();
}

void HandleStrategicTurn() {
  UINT32 uiTime;
  UINT32 uiCheckTime;

  // OK, DO THIS CHECK EVERY ONCE AND A WHILE...
  if (COUNTERDONE(STRATEGIC_OVERHEAD)) {
    RESETCOUNTER(STRATEGIC_OVERHEAD);

    // if the game is paused, or we're in mapscreen and time is not being compressed
    if ((IsGamePaused() == TRUE) || ((IsMapScreen_2()) && !IsTimeBeingCompressed())) {
      // don't do any of this
      return;
    }

    uiTime = GetJA2Clock();

    // Do not handle turns update if in turnbased combat
    if ((gTacticalStatus.uiFlags & TURNBASED) && (gTacticalStatus.uiFlags & INCOMBAT)) {
      guiLastTacticalRealTime = uiTime;
    } else {
      if (giTimeCompressMode <= TIME_COMPRESS_X1) {
        uiCheckTime = NUM_REAL_SEC_PER_TACTICAL_TURN;
      } else {
        // OK, if we have compressed time...., adjust our check value to be faster....
        uiCheckTime = NUM_REAL_SEC_PER_TACTICAL_TURN / (GetTimeCompressSpeed(giTimeCompressMode) *
                                                        RT_COMPRESSION_TACTICAL_TURN_MODIFIER);
      }

      if ((uiTime - guiLastTacticalRealTime) > uiCheckTime) {
        HandleTacticalEndTurn();

        guiLastTacticalRealTime = uiTime;
      }
    }
  }
}

void HandleStrategicTurnImplicationsOfExitingCombatMode(void) {
  SyncStrategicTurnTimes();
  HandleTacticalEndTurn();
}
