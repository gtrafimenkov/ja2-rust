#ifndef _MOUSE_SYSTEM_H_
#define _MOUSE_SYSTEM_H_

#include "SGP/MouseSystemMacros.h"
#include "SGP/Types.h"

struct MOUSE_REGION;

typedef void (*MOUSE_CALLBACK)(struct MOUSE_REGION *,
                               INT32);           // Define MOUSE_CALLBACK type as pointer to void
typedef void (*MOUSE_HELPTEXT_DONE_CALLBACK)();  // the help is done callback

struct MOUSE_REGION {
  uint16_t IDNumber;       // Region's ID number, set by mouse system
  int8_t PriorityLevel;    // Region's Priority, set by system and/or caller
  uint32_t uiFlags;        // Region's state flags
  int16_t RegionTopLeftX;  // Screen area affected by this region (absolute coordinates)
  int16_t RegionTopLeftY;
  int16_t RegionBottomRightX;
  int16_t RegionBottomRightY;
  int16_t MouseXPos;  // Mouse's Coordinates in absolute screen coordinates
  int16_t MouseYPos;
  int16_t RelativeXPos;  // Mouse's Coordinates relative to the Top-Left corner of the region
  int16_t RelativeYPos;
  uint16_t ButtonState;  // Current state of the mouse buttons
  uint16_t Cursor;       // Cursor to use when mouse in this region (see flags)
  MOUSE_CALLBACK
  MovementCallback;  // Pointer to callback function if movement occured in this region
  MOUSE_CALLBACK
  ButtonCallback;     // Pointer to callback function if button action occured in this region
  INT32 UserData[4];  // User Data, can be set to anything!

  // Fast help vars.
  int16_t FastHelpTimer;  // Countdown timer for FastHelp text
  STR16 FastHelpText;     // Text string for the FastHelp (describes buttons if left there a while)
  INT32 FastHelpRect;
  MOUSE_HELPTEXT_DONE_CALLBACK HelpDoneCallback;

  struct MOUSE_REGION *next;  // List maintenance, do NOT touch these entries
  struct MOUSE_REGION *prev;
};

// *****************************************************************************
//
//				Defines
//
// *****************************************************************************

// Mouse Region Flags
#define MSYS_NO_FLAGS 0x00000000
#define MSYS_MOUSE_IN_AREA 0x00000001
#define MSYS_SET_CURSOR 0x00000002
#define MSYS_MOVE_CALLBACK 0x00000004
#define MSYS_BUTTON_CALLBACK 0x00000008
#define MSYS_REGION_EXISTS 0x00000010
#define MSYS_SYSTEM_INIT 0x00000020
#define MSYS_REGION_ENABLED 0x00000040
#define MSYS_FASTHELP 0x00000080
#define MSYS_GOT_BACKGROUND 0x00000100
#define MSYS_HAS_BACKRECT 0x00000200
#define MSYS_FASTHELP_RESET 0x00000400
#define MSYS_ALLOW_DISABLED_FASTHELP 0x00000800

// Mouse region IDs
#define MSYS_ID_BASE 1
#define MSYS_ID_MAX 0xfffffff  // ( INT32 max )
#define MSYS_ID_SYSTEM 0

// Mouse region priorities
#define MSYS_PRIORITY_LOWEST 0
#define MSYS_PRIORITY_LOW 15
#define MSYS_PRIORITY_BASE 31
#define MSYS_PRIORITY_NORMAL 31
#define MSYS_PRIORITY_HIGH 63
#define MSYS_PRIORITY_HIGHEST 127
#define MSYS_PRIORITY_SYSTEM -1
#define MSYS_PRIORITY_AUTO -1

// Mouse system defines used during updates
#define MSYS_NO_ACTION 0
#define MSYS_DO_MOVE 1
#define MSYS_DO_LBUTTON_DWN 2
#define MSYS_DO_LBUTTON_UP 4
#define MSYS_DO_RBUTTON_DWN 8
#define MSYS_DO_RBUTTON_UP 16
#define MSYS_DO_LBUTTON_REPEAT 32
#define MSYS_DO_RBUTTON_REPEAT 64

#define MSYS_DO_BUTTONS                                                                  \
  (MSYS_DO_LBUTTON_DWN | MSYS_DO_LBUTTON_UP | MSYS_DO_RBUTTON_DWN | MSYS_DO_RBUTTON_UP | \
   MSYS_DO_RBUTTON_REPEAT | MSYS_DO_LBUTTON_REPEAT)

// Mouse system button masks
#define MSYS_LEFT_BUTTON 1
#define MSYS_RIGHT_BUTTON 2

// Mouse system special values
#define MSYS_NO_CALLBACK NULL
#define MSYS_NO_CURSOR 65534

// Mouse system callback reasons
#define MSYS_CALLBACK_REASON_NONE 0
#define MSYS_CALLBACK_REASON_INIT 1
#define MSYS_CALLBACK_REASON_MOVE 2
#define MSYS_CALLBACK_REASON_LBUTTON_DWN 4
#define MSYS_CALLBACK_REASON_LBUTTON_UP 8
#define MSYS_CALLBACK_REASON_RBUTTON_DWN 16
#define MSYS_CALLBACK_REASON_RBUTTON_UP 32
#define MSYS_CALLBACK_REASON_BUTTONS                                    \
  (MSYS_CALLBACK_REASON_LBUTTON_DWN | MSYS_CALLBACK_REASON_LBUTTON_UP | \
   MSYS_CALLBACK_REASON_RBUTTON_DWN | MSYS_CALLBACK_REASON_RBUTTON_UP)
#define MSYS_CALLBACK_REASON_LOST_MOUSE 64
#define MSYS_CALLBACK_REASON_GAIN_MOUSE 128

#define MSYS_CALLBACK_REASON_LBUTTON_REPEAT 256
#define MSYS_CALLBACK_REASON_RBUTTON_REPEAT 512

// Kris:  Nov 31, 1999
// Added support for double clicks.  The DOUBLECLICK event is passed combined with
// the LBUTTON_DWN event if two LBUTTON_DWN events are detected on the same button/region
// within the delay defined by MSYS_DOUBLECLICK_DELAY (in milliseconds).  If your button/region
// supports double clicks and single clicks, make sure the DOUBLECLICK event is checked first
// (rejecting the LBUTTON_DWN event if detected)
#define MSYS_CALLBACK_REASON_LBUTTON_DOUBLECLICK 1024

// Mouse grabbing return codes
#define MSYS_GRABBED_OK 0
#define MSYS_ALREADY_GRABBED 1
#define MSYS_REGION_NOT_IN_LIST 2

// Note:
//		The prototype for MSYS_SGP_Mouse_Handler_Hook() is defined in mousesystem_macros.h

// Internal Functions
INT32 MSYS_GetNewID(void);
void MSYS_TrashRegList(void);
void MSYS_AddRegionToList(struct MOUSE_REGION *region);
INT32 MSYS_RegionInList(struct MOUSE_REGION *region);
void MSYS_DeleteRegionFromList(struct MOUSE_REGION *region);
void MSYS_UpdateMouseRegion(void);
void MSYS_SetCurrentCursor(uint16_t Cursor);

// External
INT32 MSYS_Init(void);
void MSYS_Shutdown(void);
void MSYS_DefineRegion(struct MOUSE_REGION *region, uint16_t tlx, uint16_t tly, uint16_t brx,
                       uint16_t bry, int8_t priority, uint16_t crsr, MOUSE_CALLBACK movecallback,
                       MOUSE_CALLBACK buttoncallback);
void MSYS_ChangeRegionCursor(struct MOUSE_REGION *region, uint16_t crsr);
INT32 MSYS_AddRegion(struct MOUSE_REGION *region);
void MSYS_RemoveRegion(struct MOUSE_REGION *region);
void MSYS_EnableRegion(struct MOUSE_REGION *region);
void MSYS_DisableRegion(struct MOUSE_REGION *region);
void MSYS_ChangeRegionPriority(struct MOUSE_REGION *region, int8_t priority);
void MSYS_SetRegionUserData(struct MOUSE_REGION *region, INT32 index, INT32 userdata);
INT32 MSYS_GetRegionUserData(struct MOUSE_REGION *region, INT32 index);

INT32 MSYS_GrabMouse(struct MOUSE_REGION *region);
void MSYS_ReleaseMouse(struct MOUSE_REGION *region);
void MSYS_MoveMouseRegionBy(struct MOUSE_REGION *region, int16_t sDeltaX, int16_t sDeltaY);
void MSYS_MoveMouseRegionTo(struct MOUSE_REGION *region, int16_t sX, int16_t sY);

void MSYS_AllowDisabledRegionFastHelp(struct MOUSE_REGION *region, BOOLEAN fAllow);

// This function will force a re-evaluation of mous regions
// Usually used to force change of mouse cursor if panels switch, etc
void RefreshMouseRegions();

void SetRegionFastHelpText(struct MOUSE_REGION *region, STR16 szText);

void SetRegionHelpEndCallback(struct MOUSE_REGION *region,
                              MOUSE_HELPTEXT_DONE_CALLBACK CallbackFxn);

// Now also used by Wizardry -- DB
void DisplayFastHelp(struct MOUSE_REGION *region);
void RenderFastHelp();

void SetFastHelpDelay(int16_t sFastHelpDelay);
void EnableMouseFastHelp(void);
void DisableMouseFastHelp(void);

void ResetClickedMode(void);

BOOLEAN SetRegionSavedRect(struct MOUSE_REGION *region);
void FreeRegionSavedRect(struct MOUSE_REGION *region);

#endif
