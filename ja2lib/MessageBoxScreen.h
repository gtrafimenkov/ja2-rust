#ifndef _MSGBOX_H
#define _MSGBOX_H

#include "SGP/MouseSystem.h"
#include "SGP/Types.h"
#include "UI.h"

typedef struct {
  uint16_t usFlags;
  uint32_t uiExitScreen;
  MSGBOX_CALLBACK ExitCallback;
  INT16 sX;
  INT16 sY;
  uint32_t uiSaveBuffer;
  struct MOUSE_REGION BackRegion;
  uint16_t usWidth;
  uint16_t usHeight;
  INT32 iButtonImages;
  union {
    struct {
      uint32_t uiOKButton;
      uint32_t uiYESButton;
      uint32_t uiNOButton;
      uint32_t uiUnusedButton;
    };
    struct {
      uint32_t uiButton[4];
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

void DoUpperScreenIndependantMessageBox(CHAR16 *zString, uint16_t usFlags,
                                        MSGBOX_CALLBACK ReturnCallback);
void DoScreenIndependantMessageBoxWithRect(CHAR16 *zString, uint16_t usFlags,
                                           MSGBOX_CALLBACK ReturnCallback,
                                           const SGPRect *pCenteringRect);

// wrappers for other screens
BOOLEAN DoLapTopSystemMessageBoxWithRect(uint8_t ubStyle, CHAR16 *zString, uint32_t uiExitScreen,
                                         uint16_t usFlags, MSGBOX_CALLBACK ReturnCallback,
                                         const SGPRect *pCenteringRect);
INT32 DoMapMessageBoxWithRect(uint8_t ubStyle, CHAR16 *zString, uint32_t uiExitScreen,
                              uint16_t usFlags, MSGBOX_CALLBACK ReturnCallback,
                              const SGPRect *pCenteringRect);
BOOLEAN DoOptionsMessageBoxWithRect(uint8_t ubStyle, CHAR16 *zString, uint32_t uiExitScreen,
                                    uint16_t usFlags, MSGBOX_CALLBACK ReturnCallback,
                                    const SGPRect *pCenteringRect);
BOOLEAN DoSaveLoadMessageBoxWithRect(uint8_t ubStyle, CHAR16 *zString, uint32_t uiExitScreen,
                                     uint16_t usFlags, MSGBOX_CALLBACK ReturnCallback,
                                     const SGPRect *pCenteringRect);

#endif
