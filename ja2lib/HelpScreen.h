#ifndef _HELP_SCREEN__H_
#define _HELP_SCREEN__H_

#include "SGP/Types.h"

// enum used for the different help screens that can come up
enum {
  HELP_SCREEN_LAPTOP,
  HELP_SCREEN_MAPSCREEN,
  HELP_SCREEN_MAPSCREEN_NO_ONE_HIRED,
  HELP_SCREEN_MAPSCREEN_NOT_IN_ARULCO,
  HELP_SCREEN_MAPSCREEN_SECTOR_INVENTORY,
  HELP_SCREEN_TACTICAL,
  HELP_SCREEN_OPTIONS,
  HELP_SCREEN_LOAD_GAME,

  HELP_SCREEN_NUMBER_OF_HELP_SCREENS,
};

typedef struct {
  INT8 bCurrentHelpScreen;
  uint32_t uiFlags;

  uint16_t usHasPlayerSeenHelpScreenInCurrentScreen;

  UINT8 ubHelpScreenDirty;

  uint16_t usScreenLocX;
  uint16_t usScreenLocY;
  uint16_t usScreenWidth;
  uint16_t usScreenHeight;

  INT32 iLastMouseClickY;  // last position the mouse was clicked ( if != -1 )

  INT8 bCurrentHelpScreenActiveSubPage;  // used to keep track of the current page being displayed

  INT8 bNumberOfButtons;

  // used so if the user checked the box to show the help, it doesnt automatically come up every
  // frame
  BOOLEAN fHaveAlreadyBeenInHelpScreenSinceEnteringCurrenScreen;

  INT8 bDelayEnteringHelpScreenBy1FrameCount;
  uint16_t usLeftMarginPosX;

  uint16_t usCursor;

  BOOLEAN fWasTheGamePausedPriorToEnteringHelpScreen;

  // scroll variables
  uint16_t usTotalNumberOfPixelsInBuffer;
  INT32 iLineAtTopOfTextBuffer;
  uint16_t usTotalNumberOfLinesInBuffer;
  BOOLEAN fForceHelpScreenToComeUp;

} HELP_SCREEN_STRUCT;

extern HELP_SCREEN_STRUCT gHelpScreen;

BOOLEAN ShouldTheHelpScreenComeUp(UINT8 ubScreenID, BOOLEAN fForceHelpScreenToComeUp);
void HelpScreenHandler();
void InitHelpScreenSystem();
void NewScreenSoResetHelpScreen();
INT8 HelpScreenDetermineWhichMapScreenHelpToShow();

#endif
