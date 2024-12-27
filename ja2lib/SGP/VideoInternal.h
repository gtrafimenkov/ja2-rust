// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include "SGP/Types.h"

extern uint16_t gusScreenWidth;
extern uint16_t gusScreenHeight;

#define BUFFER_READY 0x00
#define BUFFER_BUSY 0x01
#define BUFFER_DIRTY 0x02
#define BUFFER_DISABLED 0x03

extern uint32_t guiFrameBufferState;    // BUFFER_READY, BUFFER_DIRTY
extern uint32_t guiMouseBufferState;    // BUFFER_READY, BUFFER_DIRTY, BUFFER_DISABLED
extern uint32_t guiVideoManagerState;   // VIDEO_ON, VIDEO_OFF, VIDEO_SUSPENDED, VIDEO_SHUTTING_DOWN
extern uint32_t guiRefreshThreadState;  // THREAD_ON, THREAD_OFF, THREAD_SUSPENDED

extern uint16_t gusMouseCursorWidth;
extern uint16_t gusMouseCursorHeight;
extern int16_t gsMouseCursorXOffset;
extern int16_t gsMouseCursorYOffset;

#define MAX_CURSOR_WIDTH 64
#define MAX_CURSOR_HEIGHT 64

extern struct VObject *gpCursorStore;
