#include "SGP/Types.h"

extern UINT16 gusScreenWidth;
extern UINT16 gusScreenHeight;

#define BUFFER_READY 0x00
#define BUFFER_BUSY 0x01
#define BUFFER_DIRTY 0x02
#define BUFFER_DISABLED 0x03

extern UINT32 guiFrameBufferState;    // BUFFER_READY, BUFFER_DIRTY
extern UINT32 guiMouseBufferState;    // BUFFER_READY, BUFFER_DIRTY, BUFFER_DISABLED
extern UINT32 guiVideoManagerState;   // VIDEO_ON, VIDEO_OFF, VIDEO_SUSPENDED, VIDEO_SHUTTING_DOWN
extern UINT32 guiRefreshThreadState;  // THREAD_ON, THREAD_OFF, THREAD_SUSPENDED

extern UINT16 gusMouseCursorWidth;
extern UINT16 gusMouseCursorHeight;
extern INT16 gsMouseCursorXOffset;
extern INT16 gsMouseCursorYOffset;

#define MAX_CURSOR_WIDTH 64
#define MAX_CURSOR_HEIGHT 64

extern struct VObject *gpCursorStore;
