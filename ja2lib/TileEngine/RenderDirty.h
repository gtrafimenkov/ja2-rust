#ifndef __RENDER_DIRTY_H
#define __RENDER_DIRTY_H

#include "SGP/Types.h"

struct VSurface;
struct _VIDEO_OVERLAY;

// DEFINES
#define BGND_FLAG_PERMANENT 0x80000000
#define BGND_FLAG_SINGLE 0x40000000
#define BGND_FLAG_SAVE_Z 0x20000000
#define BGND_FLAG_MERC 0x10000000
#define BGND_FLAG_SAVERECT 0x08000000
#define BGND_FLAG_TOPMOST 0x04000000
#define BGND_FLAG_ANIMATED 0x00000001

#define VOVERLAY_DIRTYBYTEXT 0x00000001
#define VOVERLAY_STARTDISABLED 0x00000002

#define VOVERLAY_DESC_TEXT 0x00001000
#define VOVERLAY_DESC_DISABLED 0x00002000
#define VOVERLAY_DESC_POSITION 0x00004000

// STRUCTURES

// Callback for topmost blitters
typedef void (*OVERLAY_CALLBACK)(struct _VIDEO_OVERLAY *);

// Struct for backgrounds
typedef struct {
  BOOLEAN fAllocated;
  BOOLEAN fFilled;
  BOOLEAN fFreeMemory;
  BOOLEAN fZBuffer;
  uint32_t uiFlags;
  uint16_t *pSaveArea;
  uint16_t *pZSaveArea;
  int16_t sLeft;
  int16_t sTop;
  int16_t sRight;
  int16_t sBottom;
  int16_t sWidth;
  int16_t sHeight;
  BOOLEAN fPendingDelete;
  BOOLEAN fDisabled;

} BACKGROUND_SAVE;

// Struct for topmost blitters
typedef struct _VIDEO_OVERLAY {
  uint32_t uiFlags;
  BOOLEAN fAllocated;
  BOOLEAN fDisabled;
  BOOLEAN fActivelySaving;
  BOOLEAN fDeletionPending;
  int32_t uiBackground;
  BACKGROUND_SAVE *pBackground;
  uint16_t *pSaveArea;
  uint32_t uiUserData[5];
  uint32_t uiFontID;
  int16_t sX;
  int16_t sY;
  uint8_t ubFontBack;
  uint8_t ubFontFore;
  wchar_t zText[200];
  uint32_t uiDestBuff;
  OVERLAY_CALLBACK BltCallback;

} VIDEO_OVERLAY;

// Struct for init topmost blitter
typedef struct {
  uint32_t uiFlags;
  BOOLEAN fDisabled;
  int16_t sLeft;
  int16_t sTop;
  int16_t sRight;
  int16_t sBottom;
  uint32_t uiFontID;
  int16_t sX;
  int16_t sY;
  uint8_t ubFontBack;
  uint8_t ubFontFore;
  wchar_t pzText[200];
  OVERLAY_CALLBACK BltCallback;

} VIDEO_OVERLAY_DESC;

// GLOBAL VARIABLES
extern struct GRect gDirtyClipRect;

// FUNCTIONS
////////////

// DIRTY QUEUE
BOOLEAN InitializeBaseDirtyRectQueue();
void ShutdownBaseDirtyRectQueue();
void AddBaseDirtyRect(int32_t iLeft, int32_t iTop, int32_t iRight, int32_t iBottom);
BOOLEAN ExecuteBaseDirtyRectQueue();
BOOLEAN EmptyDirtyRectQueue();

// BACKGROUND RECT BUFFERING STUFF
void DisableBackgroundRect(int32_t iIndex, BOOLEAN fDisabled);
BOOLEAN InitializeBackgroundRects(void);
BOOLEAN ShutdownBackgroundRects(void);
int32_t RegisterBackgroundRect(uint32_t uiFlags, int16_t *pSaveArea, int16_t sLeft, int16_t sTop,
                             int16_t sRight, int16_t sBottom);
BOOLEAN FreeBackgroundRect(int32_t iIndex);
BOOLEAN FreeBackgroundRectPending(int32_t iIndex);
BOOLEAN FreeBackgroundRectType(uint32_t uiFlags);
BOOLEAN RestoreBackgroundRects(void);
BOOLEAN SaveBackgroundRects(void);
BOOLEAN InvalidateBackgroundRects(void);
BOOLEAN UpdateSaveBuffer(void);
BOOLEAN RestoreExternBackgroundRect(int16_t sLeft, int16_t sTop, int16_t sWidth, int16_t sHeight);
void SetBackgroundRectFilled(uint32_t uiBackgroundID);
BOOLEAN EmptyBackgroundRects(void);

// GPRINTF DIRTY STUFF
uint16_t gprintfdirty(int16_t x, int16_t y, wchar_t* pFontString, ...);
uint16_t gprintfinvalidate(int16_t x, int16_t y, wchar_t* pFontString, ...);
uint16_t gprintfRestore(int16_t x, int16_t y, wchar_t* pFontString, ...);

// VIDEO OVERLAY STUFF
int32_t GetFreeVideoOverlay(void);
void RecountVideoOverlays(void);
int32_t RegisterVideoOverlay(uint32_t uiFlags, VIDEO_OVERLAY_DESC *pTopmostDesc);
void ExecuteVideoOverlays();
BOOLEAN UpdateVideoOverlay(VIDEO_OVERLAY_DESC *pTopmostDesc, uint32_t iBlitterIndex,
                           BOOLEAN fForceAll);
void SaveVideoOverlaysArea(struct VSurface *src);
void DeleteVideoOverlaysArea();
void AllocateVideoOverlaysArea();
void ExecuteVideoOverlaysToAlternateBuffer(uint32_t uiNewDestBuffer);
void RemoveVideoOverlay(int32_t iVideoOverlay);
BOOLEAN RestoreShiftedVideoOverlays(int16_t sShiftX, int16_t sShiftY);
BOOLEAN SetOverlayUserData(int32_t iVideoOverlay, uint8_t ubNum, uint32_t uiData);
void EnableVideoOverlay(BOOLEAN fEnable, int32_t iOverlayIndex);

void BlitMFont(VIDEO_OVERLAY *pBlitter);

bool VSurfaceBlitBufToBuf(struct VSurface *src, struct VSurface *dest, u16 x, u16 y, u16 width,
                          u16 height);
#endif
