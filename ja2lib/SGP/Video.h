#ifndef __VIDEO_H
#define __VIDEO_H

#include "SGP/Types.h"

struct SGPPaletteEntry;
struct VSurface;

#define VIDEO_NO_CURSOR 0xFFFF

extern int32_t giNumFrames;

struct PlatformInitParams;
extern BOOLEAN InitializeVideoManager(struct PlatformInitParams *params);

extern void ShutdownVideoManager(void);
extern void SuspendVideoManager(void);
extern BOOLEAN RestoreVideoManager(void);

extern void GetCurrentVideoSettings(uint16_t *usWidth, uint16_t *usHeight);

extern void InvalidateRegion(int32_t iLeft, int32_t iTop, int32_t iRight, int32_t iBottom);
extern void InvalidateRegionEx(int32_t iLeft, int32_t iTop, int32_t iRight, int32_t iBottom,
                               uint32_t uiFlags);
extern void InvalidateScreen(void);

extern BOOLEAN GetPrimaryRGBDistributionMasks(uint32_t *RedBitMask, uint32_t *GreenBitMask,
                                              uint32_t *BblueBitMask);
extern void StartFrameBufferRender(void);
extern void EndFrameBufferRender(void);
extern void PrintScreen(void);

extern BOOLEAN EraseMouseCursor();
void SetMouseCursorProperties(int16_t sOffsetX, int16_t sOffsetY, uint16_t usCursorHeight,
                              uint16_t usCursorWidth);

BOOLEAN Set8BPPPalette(struct SGPPaletteEntry *pPalette);

void RefreshScreen();

void FatalError(char *pError, ...);

void DirtyCursor();

#endif
