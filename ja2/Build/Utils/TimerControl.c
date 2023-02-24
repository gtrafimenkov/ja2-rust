#include "Utils/TimerControl.h"

#include "SGP/Debug.h"
#include "SGP/WCheck.h"
#include "Tactical/InterfaceControl.h"
#include "Tactical/Overhead.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

INT32 giTimerDiag = 0;

UINT32 guiBaseJA2Clock = 0;
UINT32 guiBaseJA2NoPauseClock = 0;

BOOLEAN gfPauseClock = FALSE;

INT32 giTimerIntervals[NUMTIMERS] = {
    5,     // Tactical Overhead
    20,    // NEXTSCROLL
    200,   // Start Scroll
    200,   // Animate tiles
    1000,  // FPS Counter
    80,    // PATH FIND COUNTER
    150,   // CURSOR TIMER
    250,   // RIGHT CLICK FOR MENU
    300,   // LEFT
    30,    // SLIDING TEXT
    200,   // TARGET REFINE TIMER
    150,   // CURSOR/AP FLASH
    60,    // FADE MERCS OUT
    160,   // PANEL SLIDE
    1000,  // CLOCK UPDATE DELAY
    20,    // PHYSICS UPDATE
    100,   // FADE ENEMYS
    20,    // STRATEGIC OVERHEAD
    40,
    500,  // NON GUN TARGET REFINE TIMER
    250,  // IMPROVED CURSOR FLASH
    500,  // 2nd CURSOR FLASH
    400,  // RADARMAP BLINK AND OVERHEAD MAP BLINK SHOUDL BE THE SAME
    400,
    10,   // Music Overhead
    100,  // Rubber band start delay
};

// TIMER COUNTERS
INT32 giTimerCounters[NUMTIMERS];

INT32 giTimerAirRaidQuote = 0;
INT32 giTimerAirRaidDiveStarted = 0;
INT32 giTimerAirRaidUpdate = 0;
INT32 giTimerCustomizable = 0;
INT32 giTimerTeamTurnUpdate = 0;

CUSTOMIZABLE_TIMER_CALLBACK gpCustomizableTimerCallback = NULL;

// GLOBALS FOR CALLBACK
UINT32 gCNT;
SOLDIERTYPE *gPSOLDIER;

// GLobal for displaying time diff ( DIAG )
UINT32 guiClockDiff = 0;
UINT32 guiClockStart = 0;

extern UINT32 guiCompressionStringBaseTime;
extern INT32 giFlashHighlightedItemBaseTime;
extern INT32 giCompatibleItemBaseTime;
extern INT32 giAnimateRouteBaseTime;
extern INT32 giPotHeliPathBaseTime;
extern INT32 giClickHeliIconBaseTime;
extern INT32 giExitToTactBaseTime;
extern UINT32 guiSectorLocatorBaseTime;
extern INT32 giCommonGlowBaseTime;
extern INT32 giFlashAssignBaseTime;
extern INT32 giFlashContractBaseTime;
extern UINT32 guiFlashCursorBaseTime;
extern INT32 giPotCharPathBaseTime;

//////////////////////////////////////////////////////////////////////////////////////////////
// TIMER CALLBACK S
//////////////////////////////////////////////////////////////////////////////////////////////

static void ResetJA2ClockGlobalTimers(void) {
  UINT32 uiCurrentTime = GetJA2Clock();

  guiCompressionStringBaseTime = uiCurrentTime;
  giFlashHighlightedItemBaseTime = uiCurrentTime;
  giCompatibleItemBaseTime = uiCurrentTime;
  giAnimateRouteBaseTime = uiCurrentTime;
  giPotHeliPathBaseTime = uiCurrentTime;
  giClickHeliIconBaseTime = uiCurrentTime;
  giExitToTactBaseTime = uiCurrentTime;
  guiSectorLocatorBaseTime = uiCurrentTime;

  giCommonGlowBaseTime = uiCurrentTime;
  giFlashAssignBaseTime = uiCurrentTime;
  giFlashContractBaseTime = uiCurrentTime;
  guiFlashCursorBaseTime = uiCurrentTime;
  giPotCharPathBaseTime = uiCurrentTime;
}

UINT32 GetJA2Clock() { return guiBaseJA2Clock; }

void SetJA2Clock(UINT32 time) {
  guiBaseJA2Clock = time;
  // whenever guiBaseJA2Clock changes, we must reset all the timer variables that use it as a
  // reference
  ResetJA2ClockGlobalTimers();
}
