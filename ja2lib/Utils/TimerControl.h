#ifndef __TIMER_CONTROL_H
#define __TIMER_CONTROL_H

#include "SGP/Types.h"

typedef INT32 TIMECOUNTER;

typedef void (*CUSTOMIZABLE_TIMER_CALLBACK)(void);

// TIMER DEFINES
enum {
  TOVERHEAD = 0,              // Overhead time slice
  NEXTSCROLL,                 // Scroll Speed timer
  STARTSCROLL,                // Scroll Start timer
  ANIMATETILES,               // Animate tiles timer
  FPSCOUNTER,                 // FPS value
  PATHFINDCOUNTER,            // PATH FIND COUNTER
  CURSORCOUNTER,              // ANIMATED CURSOR
  RMOUSECLICK_DELAY_COUNTER,  // RIGHT BUTTON CLICK DELAY
  LMOUSECLICK_DELAY_COUNTER,  // LEFT	 BUTTON CLICK DELAY
  SLIDETEXT,                  // DAMAGE DISPLAY
  TARGETREFINE,               // TARGET REFINE
  CURSORFLASH,                // Cursor/AP flash
  FADE_GUY_OUT,               // FADE MERCS OUT
  PANELSLIDE_UNUSED,          // PANLE SLIDE
  TCLOCKUPDATE,               // CLOCK UPDATE
  PHYSICSUPDATE,              // PHYSICS UPDATE.
  GLOW_ENEMYS,
  STRATEGIC_OVERHEAD,    // STRATEGIC OVERHEAD
  CYCLERENDERITEMCOLOR,  // CYCLE COLORS
  NONGUNTARGETREFINE,    // TARGET REFINE
  CURSORFLASHUPDATE,     //
  INVALID_AP_HOLD,       // TIME TO HOLD INVALID AP
  RADAR_MAP_BLINK,       // BLINK DELAY FOR RADAR MAP
  OVERHEAD_MAP_BLINK,    // OVERHEADMAP
  MUSICOVERHEAD,         // MUSIC TIMER
  RUBBER_BAND_START_DELAY,
  NUMTIMERS
};

// Base resultion of callback timer
#define BASETIMESLICE 10

// TIMER INTERVALS
extern INT32 giTimerIntervals[NUMTIMERS];
// TIMER COUNTERS
extern INT32 giTimerCounters[NUMTIMERS];

extern INT32 giTimerDiag;

extern INT32 giTimerTeamTurnUpdate;

BOOLEAN InitializeJA2Clock(void);
void ShutdownJA2Clock(void);

uint32_t GetJA2Clock();
void SetJA2Clock(uint32_t time);

void PauseTime(BOOLEAN fPaused);

void SetCustomizableTimerCallbackAndDelay(INT32 iDelay, CUSTOMIZABLE_TIMER_CALLBACK pCallback,
                                          BOOLEAN fReplace);
void CheckCustomizableTimer(void);

extern CUSTOMIZABLE_TIMER_CALLBACK gpCustomizableTimerCallback;

#define UPDATECOUNTER(c)                                                \
  ((giTimerCounters[c] - BASETIMESLICE) < 0) ? (giTimerCounters[c] = 0) \
                                             : (giTimerCounters[c] -= BASETIMESLICE)
#define RESETCOUNTER(c) (giTimerCounters[c] = giTimerIntervals[c])
#define COUNTERDONE(c) (giTimerCounters[c] == 0) ? TRUE : FALSE

#define UPDATETIMECOUNTER(c) ((c - BASETIMESLICE) < 0) ? (c = 0) : (c -= BASETIMESLICE)
#define RESETTIMECOUNTER(c, d) (c = d)

#define TIMECOUNTERDONE(c, d) (c == 0) ? TRUE : FALSE

#define ZEROTIMECOUNTER(c) (c = 0)

#endif
