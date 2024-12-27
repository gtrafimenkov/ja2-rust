// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#ifndef _MOUSE_SYSTEM_MACROS_H_
#define _MOUSE_SYSTEM_MACROS_H_

#include "SGP/Types.h"

// Special macro hook for the mouse handler. Allows a call to a secondary mouse handler.
// Define the label _MOUSE_SYSTEM_HOOK_ to activate. Undef it to deactivate.
//
// The actual function prototype is shown below

#define _MOUSE_SYSTEM_HOOK_

#ifdef _MOUSE_SYSTEM_HOOK_
#define MouseSystemHook(t, x, y, l, r) MSYS_SGP_Mouse_Handler_Hook(t, x, y, l, r)
#else
#define MouseSystemHook(t, x, y, l, r)
#endif

// Special prototype for mouse handler hook
extern void MSYS_SGP_Mouse_Handler_Hook(uint16_t Type, uint16_t Xcoord, uint16_t Ycoord,
                                        BOOLEAN LeftButton, BOOLEAN RightButton);

#endif
