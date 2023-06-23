#ifndef __WORLD_CLOCK
#define __WORLD_CLOCK

#include "SGP/Types.h"
#include "rust_clock.h"

// where the time string itself is rendered
#define CLOCK_X 554
#define CLOCK_Y 459

// the mouse region around the clock (bigger)
#define CLOCK_REGION_START_X 552
#define CLOCK_REGION_START_Y 456
#define CLOCK_REGION_WIDTH (620 - CLOCK_REGION_START_X)
#define CLOCK_REGION_HEIGHT (468 - CLOCK_REGION_START_Y)

#define NUM_SEC_IN_DAY 86400
#define NUM_SEC_IN_HOUR 3600
#define NUM_SEC_IN_MIN 60
#define ROUNDTO_MIN 5

#define NUM_MIN_IN_DAY 1440
#define NUM_MIN_IN_HOUR 60

// Kris:
// This is the plan for game time...
// Game time should be restricted to outside code.  Think of it as encapsulation.  Anyway, using
// these simple functions, you will be able to change the amount of time that passes per frame.  The
// gameloop will automatically update the clock once per cycle, regardless of the mode you are in.
// This does pose potential problems in modes such as the editor, or similar where time shouldn't
// pass, and isn't currently handled.  The best thing to do in these cases is call the PauseGame()
// function when entering such a mode, and UnPauseGame() when finished.  Everything will be restored
// just the way you left it.  This is much simpler to handle in the overall scheme of things.

// Allows access to the current time rate.
UINT32 GetGameSecondsPerFrame();
void RenderPausedGameBox(void);

void StartTimeCompression(void);
BOOLEAN IsTimeBeingCompressed(void);  // returns FALSE if time isn't currently being compressed for
                                      // ANY reason (various pauses, etc.)

void SetGameTimeCompressionLevel(enum TIME_COMPRESS_MODE uiCompressionRate);
void DecreaseGameTimeCompressionRate();
void IncreaseGameTimeCompressionRate();

#define FIRST_ARRIVAL_DELAY ((6 * NUM_SEC_IN_HOUR) + (0 * NUM_SEC_IN_MIN))  // 7am ( 6hours later)

// compress mode now in use
extern INT32 giTimeCompressMode;

enum {
  WARPTIME_NO_PROCESSING_OF_EVENTS,
  WARPTIME_PROCESS_EVENTS_NORMALLY,
  WARPTIME_PROCESS_TARGET_TIME_FIRST,
};
void WarpGameTime(UINT32 uiAdjustment, UINT8 ubWarpCode);

// This function is called once per cycle in the game loop.  This determine how often the clock
// should be as well as how much to update the clock by.
void UpdateClock();

extern CHAR16 gswzWorldTimeStr[20];  // Day 99, 23:55

// Advanced function used by certain event callbacks.  In the case where time is warped, certain
// event need to know how much time was warped since the last query to the event list. This function
// returns that value
extern UINT32 guiTimeOfLastEventQuery;

// This value represents the time that the sector was loaded.  If you are in sector A9, and leave
// the game clock at that moment will get saved into the temp file associated with it.  The next
// time you enter A9, this value will contain that time.  Used for scheduling purposes.
extern UINT32 guiTimeCurrentSectorWasLastLoaded;

// is the current pause state due to the player?
extern BOOLEAN gfPauseDueToPlayerGamePause;

// we've just clued up a pause by the player, the tactical screen will need a full one shot refresh
// to remove a 2 frame update problem
extern BOOLEAN gfJustFinishedAPause;

extern BOOLEAN gfResetAllPlayerKnowsEnemiesFlags;

void InitNewGameClock();

void GotoNextTimeOfDay(UINT32 uiTOD);

void RenderClock(INT16 sX, INT16 sY);

// IMPORTANT FUNCTION:  Used whenever an event or situation is deemed important enough to cancel the
// further processing of time in this current time slice!  This can only be used inside of event
// callback functions -- otherwise, it'll be ignored and automatically reset.  An example of this
// would be when arriving in a new sector and being prompted to attack or retreat.
void InterruptTime();
void PauseTimeForInterupt();

extern BOOLEAN gfTimeInterrupt;

BOOLEAN DidGameJustStart();

BOOLEAN SaveGameClock(FileID hFile, BOOLEAN fGamePaused, BOOLEAN fLockPauseState);
BOOLEAN LoadGameClock(FileID hFile);

// time compress flag stuff
BOOLEAN HasTimeCompressOccured(void);
void ResetTimeCompressHasOccured(void);

// create mouse region to pause game
void CreateMouseRegionForPauseOfClock(INT16 sX, INT16 sY);

// remove mouse region for pause game
void RemoveMouseRegionForPauseOfClock(void);

// handle pausing and unpausing of game
void HandlePlayerPauseUnPauseOfGame(void);

void ClearTacticalStuffDueToTimeCompression(void);

#endif
