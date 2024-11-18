#ifndef __INPUT_
#define __INPUT_

#include "Point.h"
#include "SGP/Types.h"

struct GRect;

#define KEY_DOWN 0x0001
#define KEY_UP 0x0002
#define KEY_REPEAT 0x0004
#define LEFT_BUTTON_DOWN 0x0008
#define LEFT_BUTTON_UP 0x0010
#define LEFT_BUTTON_DBL_CLK 0x0020
#define LEFT_BUTTON_REPEAT 0x0040
#define RIGHT_BUTTON_DOWN 0x0080
#define RIGHT_BUTTON_UP 0x0100
#define RIGHT_BUTTON_REPEAT 0x0200
#define MOUSE_POS 0x0400
#define MOUSE_WHEEL 0x0800

#define SHIFT_DOWN 0x01
#define CTRL_DOWN 0x02
#define ALT_DOWN 0x04

#define DBL_CLK_TIME 300  // Increased by Alex, Jun-10-97, 200 felt too short
#define BUTTON_REPEAT_TIMEOUT 250
#define BUTTON_REPEAT_TIME 50

typedef struct {
  uint32_t uiTimeStamp;
  uint16_t usKeyState;
  uint16_t usEvent;
  uint32_t usParam;
  uint32_t uiParam;
} InputAtom;

typedef struct StringInput {
  uint16_t *pString;
  uint16_t *pOriginalString;
  uint16_t *pFilter;
  uint16_t usMaxStringLength;
  uint16_t usCurrentStringLength;
  uint16_t usStringOffset;
  uint16_t usLastCharacter;
  BOOLEAN fInsertMode;
  BOOLEAN fFocus;
  struct StringInput *pPreviousString;
  struct StringInput *pNextString;

} StringInput;

extern BOOLEAN InitializeInputManager(void);
extern void ShutdownInputManager(void);
extern BOOLEAN DequeueEvent(InputAtom *Event);
extern void QueueEvent(uint16_t ubInputEvent, uint32_t usParam, uint32_t uiParam);

extern void KeyDown(uint32_t usParam, uint32_t uiParam);
extern void KeyUp(uint32_t usParam, uint32_t uiParam);

extern void GetMousePos(SGPPoint *Point);

extern BOOLEAN DequeueSpecificEvent(InputAtom *Event, uint32_t uiMaskFlags);

extern void RestrictMouseToXYXY(uint16_t usX1, uint16_t usY1, uint16_t usX2, uint16_t usY2);
extern void RestrictMouseCursor(struct GRect *pRectangle);
extern void FreeMouseCursor(void);
extern BOOLEAN IsCursorRestricted(void);
extern void GetRestrictedClipCursor(struct GRect *pRectangle);
extern void ReapplyCursorClipRect(void);

void SimulateMouseMovement(uint32_t uiNewXPos, uint32_t uiNewYPos);

extern void DequeueAllKeyBoardEvents();

extern BOOLEAN gfKeyState[256];  // TRUE = Pressed, FALSE = Not Pressed

extern uint16_t gusMouseXPos;         // X position of the mouse on screen
extern uint16_t gusMouseYPos;         // y position of the mouse on screen
extern BOOLEAN gfLeftButtonState;   // TRUE = Pressed, FALSE = Not Pressed
extern BOOLEAN gfRightButtonState;  // TRUE = Pressed, FALSE = Not Pressed

extern BOOLEAN gfSGPInputReceived;

#define _KeyDown(a) gfKeyState[(a)]
#define _LeftButtonDown gfLeftButtonState
#define _RightButtonDown gfRightButtonState
#define _MouseXPos gusMouseXPos
#define _MouseYPos gusMouseYPos

// NOTE: this may not be the absolute most-latest current mouse co-ordinates, use GetCursorPos for
// that
#define _gusMouseInside(x1, y1, x2, y2) \
  ((gusMouseXPos >= x1) && (gusMouseXPos <= x2) && (gusMouseYPos >= y1) && (gusMouseYPos <= y2))

#define _EvType(a) ((InputAtom *)(a))->usEvent
#define _EvTimeStamp(a) ((InputAtom *)(a))->uiTimeStamp
#define _EvKey(a) ((InputAtom *)(a))->usParam
#define _EvMouseX(a) (uint16_t)(((InputAtom *)(a))->uiParam & 0x0000ffff)
#define _EvMouseY(a) (uint16_t)((((InputAtom *)(a))->uiParam & 0xffff0000) >> 16)
#define _EvShiftDown(a) (((InputAtom *)(a))->usKeyState & SHIFT_DOWN)
#define _EvCtrlDown(a) (((InputAtom *)(a))->usKeyState & CTRL_DOWN)
#define _EvAltDown(a) (((InputAtom *)(a))->usKeyState & ALT_DOWN)

extern struct Point GetMousePoint();

#endif
