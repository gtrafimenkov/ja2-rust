#ifndef _MSGBOX_H
#define _MSGBOX_H

#include "SGP/MouseSystem.h"
#include "SGP/Types.h"
#include "UI.h"

typedef struct {
  UINT16 usFlags;
  UINT32 uiExitScreen;
  MSGBOX_CALLBACK ExitCallback;
  INT16 sX;
  INT16 sY;
  UINT32 uiSaveBuffer;
  struct MOUSE_REGION BackRegion;
  UINT16 usWidth;
  UINT16 usHeight;
  INT32 iButtonImages;
  union {
    struct {
      UINT32 uiOKButton;
      UINT32 uiYESButton;
      UINT32 uiNOButton;
      UINT32 uiUnusedButton;
    };
    struct {
      UINT32 uiButton[4];
    };
  };
  BOOLEAN fRenderBox;
  INT8 bHandled;
  INT32 iBoxId;

} MESSAGE_BOX_STRUCT;

extern MESSAGE_BOX_STRUCT gMsgBox;
extern BOOLEAN fRestoreBackgroundForMessageBox;

// this variable can be unset if ur in a non gamescreen and DONT want the msg box to use the save
// buffer
extern BOOLEAN gfDontOverRideSaveBuffer;

////////////////////////////////
// ubStyle:				Determines the look of graphics including buttons
// zString:				16-bit string
// uiExitScreen		The screen to exit to
// ubFlags				Some flags for button style
// ReturnCallback	Callback for return. Can be NULL. Returns any above return value
// pCenteringRect	Rect to send if MSG_BOX_FLAG_USE_CENTERING_RECT set. Can be NULL
////////////////////////////////

void DoUpperScreenIndependantMessageBox(CHAR16 *zString, UINT16 usFlags,
                                        MSGBOX_CALLBACK ReturnCallback);
void DoScreenIndependantMessageBoxWithRect(CHAR16 *zString, UINT16 usFlags,
                                           MSGBOX_CALLBACK ReturnCallback,
                                           const struct GRect *pCenteringRect);

// wrappers for other screens
BOOLEAN DoLapTopSystemMessageBoxWithRect(UINT8 ubStyle, CHAR16 *zString, UINT32 uiExitScreen,
                                         UINT16 usFlags, MSGBOX_CALLBACK ReturnCallback,
                                         const struct GRect *pCenteringRect);
INT32 DoMapMessageBoxWithRect(UINT8 ubStyle, CHAR16 *zString, UINT32 uiExitScreen, UINT16 usFlags,
                              MSGBOX_CALLBACK ReturnCallback, const struct GRect *pCenteringRect);
BOOLEAN DoOptionsMessageBoxWithRect(UINT8 ubStyle, CHAR16 *zString, UINT32 uiExitScreen,
                                    UINT16 usFlags, MSGBOX_CALLBACK ReturnCallback,
                                    const struct GRect *pCenteringRect);
BOOLEAN DoSaveLoadMessageBoxWithRect(UINT8 ubStyle, CHAR16 *zString, UINT32 uiExitScreen,
                                     UINT16 usFlags, MSGBOX_CALLBACK ReturnCallback,
                                     const struct GRect *pCenteringRect);

#endif
