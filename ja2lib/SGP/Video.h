#ifndef __VIDEO_H
#define __VIDEO_H

#include "SGP/Types.h"

struct SGPPaletteEntry;
struct VSurface;

#define VIDEO_NO_CURSOR 0xFFFF

extern int32_t giNumFrames;

extern struct VSurface *ghFrameBuffer;

struct PlatformInitParams;
extern BOOLEAN InitializeVideoManager(struct PlatformInitParams *params);

extern void ShutdownVideoManager(void);
extern void SuspendVideoManager(void);
extern BOOLEAN RestoreVideoManager(void);
extern void GetCurrentVideoSettings(uint16_t *usWidth, uint16_t *usHeight, uint8_t *ubBitDepth);
extern void InvalidateRegion(int32_t iLeft, int32_t iTop, int32_t iRight, int32_t iBottom);
extern void InvalidateScreen(void);
extern void *LockPrimarySurface(uint32_t *uiPitch);
extern void UnlockPrimarySurface(void);
extern void *LockBackBuffer(uint32_t *uiPitch);
extern void UnlockBackBuffer(void);
extern void *LockFrameBuffer(uint32_t *uiPitch);
extern void UnlockFrameBuffer(void);
extern void *LockMouseBuffer(uint32_t *uiPitch);
extern void UnlockMouseBuffer(void);
extern BOOLEAN GetPrimaryRGBDistributionMasks(uint32_t *RedBitMask, uint32_t *GreenBitMask,
                                              uint32_t *BblueBitMask);
extern BOOLEAN SetCurrentCursor(uint16_t usVideoObjectSubIndex, uint16_t usOffsetX,
                                uint16_t usOffsetY);
extern void StartFrameBufferRender(void);
extern void EndFrameBufferRender(void);
extern void PrintScreen(void);

extern BOOLEAN EraseMouseCursor();
extern BOOLEAN SetMouseCursorProperties(int16_t sOffsetX, int16_t sOffsetY, uint16_t usCursorHeight,
                                        uint16_t usCursorWidth);
void DirtyCursor();

BOOLEAN Set8BPPPalette(struct SGPPaletteEntry *pPalette);

void VideoCaptureToggle(void);

void InvalidateRegionEx(int32_t iLeft, int32_t iTop, int32_t iRight, int32_t iBottom,
                        uint32_t uiFlags);

void RefreshScreen(void *DummyVariable);

void FatalError(STR8 pError, ...);

#endif
