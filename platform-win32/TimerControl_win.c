// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include <windows.h>

#include "SGP/Debug.h"
#include "Tactical/Overhead.h"
#include "Tactical/SoldierControl.h"
#include "UI.h"
#include "Utils/TimerControl.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

extern UINT32 guiBaseJA2Clock;
extern UINT32 guiBaseJA2NoPauseClock;
extern BOOLEAN gfPauseClock;
extern int32_t giTimerAirRaidQuote;
extern int32_t giTimerAirRaidDiveStarted;
extern int32_t giTimerAirRaidUpdate;
extern int32_t giTimerCustomizable;
extern int32_t giTimerTeamTurnUpdate;
extern UINT32 gCNT;
extern struct SOLDIERTYPE *gPSOLDIER;

MMRESULT gTimerID;

void CALLBACK TimeProc(UINT uID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2) {
  static BOOLEAN fInFunction = FALSE;

  if (!fInFunction) {
    fInFunction = TRUE;

    guiBaseJA2NoPauseClock += BASETIMESLICE;

    if (!gfPauseClock) {
      guiBaseJA2Clock += BASETIMESLICE;

      for (gCNT = 0; gCNT < NUMTIMERS; gCNT++) {
        UPDATECOUNTER(gCNT);
      }

      // Update some specialized countdown timers...
      UPDATETIMECOUNTER(giTimerAirRaidQuote);
      UPDATETIMECOUNTER(giTimerAirRaidDiveStarted);
      UPDATETIMECOUNTER(giTimerAirRaidUpdate);
      UPDATETIMECOUNTER(giTimerTeamTurnUpdate);

      if (gpCustomizableTimerCallback) {
        UPDATETIMECOUNTER(giTimerCustomizable);
      }

      // If mapscreen...
      if (IsMapScreen()) {
        // IN Mapscreen, loop through player's team.....
        for (gCNT = gTacticalStatus.Team[gbPlayerNum].bFirstID;
             gCNT <= gTacticalStatus.Team[gbPlayerNum].bLastID; gCNT++) {
          gPSOLDIER = MercPtrs[gCNT];
          UPDATETIMECOUNTER(gPSOLDIER->PortraitFlashCounter);
          UPDATETIMECOUNTER(gPSOLDIER->PanelAnimateCounter);
        }
      } else {
        // Set update flags for soldiers
        ////////////////////////////
        for (gCNT = 0; gCNT < guiNumMercSlots; gCNT++) {
          gPSOLDIER = MercSlots[gCNT];

          if (gPSOLDIER != NULL) {
            UPDATETIMECOUNTER(gPSOLDIER->UpdateCounter);
            UPDATETIMECOUNTER(gPSOLDIER->DamageCounter);
            UPDATETIMECOUNTER(gPSOLDIER->ReloadCounter);
            UPDATETIMECOUNTER(gPSOLDIER->FlashSelCounter);
            UPDATETIMECOUNTER(gPSOLDIER->BlinkSelCounter);
            UPDATETIMECOUNTER(gPSOLDIER->PortraitFlashCounter);
            UPDATETIMECOUNTER(gPSOLDIER->AICounter);
            UPDATETIMECOUNTER(gPSOLDIER->FadeCounter);
            UPDATETIMECOUNTER(gPSOLDIER->NextTileCounter);
            UPDATETIMECOUNTER(gPSOLDIER->PanelAnimateCounter);
          }
        }
      }
    }

    fInFunction = FALSE;
  }
}

BOOLEAN InitializeJA2Clock(void) {
  MMRESULT mmResult;
  TIMECAPS tc;
  int32_t cnt;

  // Init timer delays
  for (cnt = 0; cnt < NUMTIMERS; cnt++) {
    giTimerCounters[cnt] = giTimerIntervals[cnt];
  }

  // First get timer resolutions
  mmResult = timeGetDevCaps(&tc, sizeof(tc));

  if (mmResult != TIMERR_NOERROR) {
    DebugMsg(TOPIC_JA2, DBG_INFO, "Could not get timer properties");
  }

  // Set timer at lowest resolution. Could use middle of lowest/highest, we'll see how this performs
  // first
  gTimerID = timeSetEvent(BASETIMESLICE, BASETIMESLICE, TimeProc, (DWORD)0, TIME_PERIODIC);

  if (!gTimerID) {
    DebugMsg(TOPIC_JA2, DBG_INFO, "Could not create timer callback");
  }

  return TRUE;
}

void ShutdownJA2Clock(void) { timeKillEvent(gTimerID); }

//////////////////////////////////////////////////////////////////////////////////////////////
// TIMER CALLBACK S
//////////////////////////////////////////////////////////////////////////////////////////////

void PauseTime(BOOLEAN fPaused) { gfPauseClock = fPaused; }

void SetCustomizableTimerCallbackAndDelay(int32_t iDelay, CUSTOMIZABLE_TIMER_CALLBACK pCallback,
                                          BOOLEAN fReplace) {
  if (gpCustomizableTimerCallback) {
    if (!fReplace) {
      // replace callback but call the current callback first
      gpCustomizableTimerCallback();
    }
  }

  RESETTIMECOUNTER(giTimerCustomizable, iDelay);
  gpCustomizableTimerCallback = pCallback;
}

void CheckCustomizableTimer(void) {
  if (gpCustomizableTimerCallback) {
    if (TIMECOUNTERDONE(giTimerCustomizable, 0)) {
      // set the callback to a temp variable so we can reset the global variable
      // before calling the callback, so that if the callback sets up another
      // instance of the timer, we don't reset it afterwards
      CUSTOMIZABLE_TIMER_CALLBACK pTempCallback;

      pTempCallback = gpCustomizableTimerCallback;
      gpCustomizableTimerCallback = NULL;
      pTempCallback();
    }
  }
}
