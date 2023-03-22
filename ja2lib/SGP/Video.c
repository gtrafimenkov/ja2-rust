#include "SGP/Video.h"

#include <string.h>

#include "SGP/VSurfaceInternal.h"
#include "SGP/VideoInternal.h"

uint16_t gusScreenWidth;
uint16_t gusScreenHeight;

void GetCurrentVideoSettings(uint16_t *usWidth, uint16_t *usHeight, uint8_t *ubBitDepth) {
  *usWidth = (uint16_t)gusScreenWidth;
  *usHeight = (uint16_t)gusScreenHeight;
  *ubBitDepth = 16;
}

uint32_t guiFrameBufferState;    // BUFFER_READY, BUFFER_DIRTY
uint32_t guiMouseBufferState;    // BUFFER_READY, BUFFER_DIRTY, BUFFER_DISABLED
uint32_t guiVideoManagerState;   // VIDEO_ON, VIDEO_OFF, VIDEO_SUSPENDED, VIDEO_SHUTTING_DOWN
uint32_t guiRefreshThreadState;  // THREAD_ON, THREAD_OFF, THREAD_SUSPENDED

void StartFrameBufferRender(void) {}
void EndFrameBufferRender(void) { guiFrameBufferState = BUFFER_DIRTY; }
void DirtyCursor() { guiMouseBufferState = BUFFER_DIRTY; }

//
// Globals for mouse cursor
//

uint16_t gusMouseCursorWidth;
uint16_t gusMouseCursorHeight;
int16_t gsMouseCursorXOffset;
int16_t gsMouseCursorYOffset;

void SetMouseCursorProperties(int16_t sOffsetX, int16_t sOffsetY, uint16_t usCursorHeight,
                              uint16_t usCursorWidth) {
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

int32_t giNumFrames = 0;
