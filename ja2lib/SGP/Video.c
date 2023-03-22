#include "SGP/Video.h"

#include "SGP/VSurfaceInternal.h"
#include "SGP/VideoInternal.h"

UINT16 gusScreenWidth;
UINT16 gusScreenHeight;

void GetCurrentVideoSettings(UINT16 *usWidth, UINT16 *usHeight, UINT8 *ubBitDepth) {
  *usWidth = (UINT16)gusScreenWidth;
  *usHeight = (UINT16)gusScreenHeight;
  *ubBitDepth = 16;
}

UINT32 guiFrameBufferState;    // BUFFER_READY, BUFFER_DIRTY
UINT32 guiMouseBufferState;    // BUFFER_READY, BUFFER_DIRTY, BUFFER_DISABLED
UINT32 guiVideoManagerState;   // VIDEO_ON, VIDEO_OFF, VIDEO_SUSPENDED, VIDEO_SHUTTING_DOWN
UINT32 guiRefreshThreadState;  // THREAD_ON, THREAD_OFF, THREAD_SUSPENDED

void StartFrameBufferRender(void) {}
void EndFrameBufferRender(void) { guiFrameBufferState = BUFFER_DIRTY; }
void DirtyCursor() { guiMouseBufferState = BUFFER_DIRTY; }

//
// Globals for mouse cursor
//

UINT16 gusMouseCursorWidth;
UINT16 gusMouseCursorHeight;
INT16 gsMouseCursorXOffset;
INT16 gsMouseCursorYOffset;

void SetMouseCursorProperties(INT16 sOffsetX, INT16 sOffsetY, UINT16 usCursorHeight,
                              UINT16 usCursorWidth) {
  gsMouseCursorXOffset = sOffsetX;
  gsMouseCursorYOffset = sOffsetY;
  gusMouseCursorWidth = usCursorWidth;
  gusMouseCursorHeight = usCursorHeight;
}

BOOLEAN EraseMouseCursor() {
  struct BufferLockInfo lock = VSurfaceLock(vsMouseBuffer);
  memset(lock.dest, 0, MAX_CURSOR_HEIGHT * lock.pitch);
  VSurfaceUnlock(vsMouseBuffer);
  return (TRUE);
}

struct VObject *gpCursorStore;
