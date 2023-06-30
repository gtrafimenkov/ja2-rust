#ifndef __VIDEO_H
#define __VIDEO_H

#include "SGP/Types.h"

struct SGPPaletteEntry;
struct VSurface;

#define VIDEO_NO_CURSOR 0xFFFF

extern INT32 giNumFrames;

struct PlatformInitParams;
extern BOOLEAN InitializeVideoManager(struct PlatformInitParams *params);

extern void ShutdownVideoManager(void);
extern void SuspendVideoManager(void);
extern BOOLEAN RestoreVideoManager(void);

extern void GetCurrentVideoSettings(UINT16 *usWidth, UINT16 *usHeight);

extern void InvalidateRegion(INT32 iLeft, INT32 iTop, INT32 iRight, INT32 iBottom);
extern void InvalidateRegionEx(INT32 iLeft, INT32 iTop, INT32 iRight, INT32 iBottom,
                               UINT32 uiFlags);
extern void InvalidateScreen(void);

extern BOOLEAN GetPrimaryRGBDistributionMasks(UINT32 *RedBitMask, UINT32 *GreenBitMask,
                                              UINT32 *BblueBitMask);
extern void StartFrameBufferRender(void);
extern void EndFrameBufferRender(void);
extern void PrintScreen(void);

extern BOOLEAN EraseMouseCursor();
void SetMouseCursorProperties(INT16 sOffsetX, INT16 sOffsetY, UINT16 usCursorHeight,
                              UINT16 usCursorWidth);

BOOLEAN Set8BPPPalette(struct SGPPaletteEntry *pPalette);

void RefreshScreen();

void FatalError(STR8 pError, ...);

void DirtyCursor();

#endif
