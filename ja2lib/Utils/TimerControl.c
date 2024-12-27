// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include "Utils/TimerControl.h"

#include "SGP/WCheck.h"
#include "Tactical/Overhead.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

int32_t giTimerDiag = 0;

uint32_t guiBaseJA2Clock = 0;
uint32_t guiBaseJA2NoPauseClock = 0;

BOOLEAN gfPauseClock = FALSE;

int32_t giTimerIntervals[NUMTIMERS] = {
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
int32_t giTimerCounters[NUMTIMERS];

int32_t giTimerAirRaidQuote = 0;
int32_t giTimerAirRaidDiveStarted = 0;
int32_t giTimerAirRaidUpdate = 0;
int32_t giTimerCustomizable = 0;
int32_t giTimerTeamTurnUpdate = 0;

CUSTOMIZABLE_TIMER_CALLBACK gpCustomizableTimerCallback = NULL;

// GLOBALS FOR CALLBACK
uint32_t gCNT;
struct SOLDIERTYPE *gPSOLDIER;

// GLobal for displaying time diff ( DIAG )
uint32_t guiClockDiff = 0;
uint32_t guiClockStart = 0;

extern uint32_t guiCompressionStringBaseTime;
extern int32_t giFlashHighlightedItemBaseTime;
extern int32_t giCompatibleItemBaseTime;
extern int32_t giAnimateRouteBaseTime;
extern int32_t giPotHeliPathBaseTime;
extern int32_t giClickHeliIconBaseTime;
extern int32_t giExitToTactBaseTime;
extern uint32_t guiSectorLocatorBaseTime;
extern int32_t giCommonGlowBaseTime;
extern int32_t giFlashAssignBaseTime;
extern int32_t giFlashContractBaseTime;
extern uint32_t guiFlashCursorBaseTime;
extern int32_t giPotCharPathBaseTime;

//////////////////////////////////////////////////////////////////////////////////////////////
// TIMER CALLBACK S
//////////////////////////////////////////////////////////////////////////////////////////////

static void ResetJA2ClockGlobalTimers(void) {
  uint32_t uiCurrentTime = GetJA2Clock();

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

uint32_t GetJA2Clock() { return guiBaseJA2Clock; }

void SetJA2Clock(uint32_t time) {
  guiBaseJA2Clock = time;
  // whenever guiBaseJA2Clock changes, we must reset all the timer variables that use it as a
  // reference
  ResetJA2ClockGlobalTimers();
}
