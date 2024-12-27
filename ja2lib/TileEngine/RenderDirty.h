#ifndef __RENDER_DIRTY_H
#define __RENDER_DIRTY_H

#include "SGP/Types.h"

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
  INT16 sLeft;
  INT16 sTop;
  INT16 sRight;
  INT16 sBottom;
  INT16 sWidth;
  INT16 sHeight;
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
  INT32 uiBackground;
  BACKGROUND_SAVE *pBackground;
  uint16_t *pSaveArea;
  uint32_t uiUserData[5];
  uint32_t uiFontID;
  INT16 sX;
  INT16 sY;
  uint8_t ubFontBack;
  uint8_t ubFontFore;
  CHAR16 zText[200];
  uint32_t uiDestBuff;
  OVERLAY_CALLBACK BltCallback;

} VIDEO_OVERLAY;

// Struct for init topmost blitter
typedef struct {
  uint32_t uiFlags;
  BOOLEAN fDisabled;
  INT16 sLeft;
  INT16 sTop;
  INT16 sRight;
  INT16 sBottom;
  uint32_t uiFontID;
  INT16 sX;
  INT16 sY;
  uint8_t ubFontBack;
  uint8_t ubFontFore;
  CHAR16 pzText[200];
  OVERLAY_CALLBACK BltCallback;

} VIDEO_OVERLAY_DESC;

// GLOBAL VARIABLES
extern SGPRect gDirtyClipRect;

// FUNCTIONS
////////////

// DIRTY QUEUE
BOOLEAN InitializeBaseDirtyRectQueue();
void ShutdownBaseDirtyRectQueue();
void AddBaseDirtyRect(INT32 iLeft, INT32 iTop, INT32 iRight, INT32 iBottom);
BOOLEAN ExecuteBaseDirtyRectQueue();
BOOLEAN EmptyDirtyRectQueue();

// BACKGROUND RECT BUFFERING STUFF
void DisableBackgroundRect(INT32 iIndex, BOOLEAN fDisabled);
BOOLEAN InitializeBackgroundRects(void);
BOOLEAN ShutdownBackgroundRects(void);
INT32 RegisterBackgroundRect(uint32_t uiFlags, INT16 *pSaveArea, INT16 sLeft, INT16 sTop,
                             INT16 sRight, INT16 sBottom);
BOOLEAN FreeBackgroundRect(INT32 iIndex);
BOOLEAN FreeBackgroundRectPending(INT32 iIndex);
BOOLEAN FreeBackgroundRectType(uint32_t uiFlags);
BOOLEAN RestoreBackgroundRects(void);
BOOLEAN SaveBackgroundRects(void);
BOOLEAN InvalidateBackgroundRects(void);
BOOLEAN UpdateSaveBuffer(void);
BOOLEAN RestoreExternBackgroundRect(INT16 sLeft, INT16 sTop, INT16 sWidth, INT16 sHeight);
void SetBackgroundRectFilled(uint32_t uiBackgroundID);
BOOLEAN EmptyBackgroundRects(void);

// GPRINTF DIRTY STUFF
uint16_t gprintfdirty(INT16 x, INT16 y, STR16 pFontString, ...);
uint16_t gprintfinvalidate(INT16 x, INT16 y, STR16 pFontString, ...);
uint16_t gprintfRestore(INT16 x, INT16 y, STR16 pFontString, ...);

// VIDEO OVERLAY STUFF
INT32 GetFreeVideoOverlay(void);
void RecountVideoOverlays(void);
INT32 RegisterVideoOverlay(uint32_t uiFlags, VIDEO_OVERLAY_DESC *pTopmostDesc);
void ExecuteVideoOverlays();
BOOLEAN UpdateVideoOverlay(VIDEO_OVERLAY_DESC *pTopmostDesc, uint32_t iBlitterIndex,
                           BOOLEAN fForceAll);
void SaveVideoOverlaysArea(uint32_t uiSrcBuffer);
void DeleteVideoOverlaysArea();
void AllocateVideoOverlaysArea();
void ExecuteVideoOverlaysToAlternateBuffer(uint32_t uiNewDestBuffer);
void RemoveVideoOverlay(INT32 iVideoOverlay);
BOOLEAN RestoreShiftedVideoOverlays(INT16 sShiftX, INT16 sShiftY);
BOOLEAN SetOverlayUserData(INT32 iVideoOverlay, uint8_t ubNum, uint32_t uiData);
void EnableVideoOverlay(BOOLEAN fEnable, INT32 iOverlayIndex);

void BlitMFont(VIDEO_OVERLAY *pBlitter);

BOOLEAN BlitBufferToBuffer(uint32_t uiSrcBuffer, uint32_t uiDestBuffer, uint16_t usSrcX,
                           uint16_t usSrcY, uint16_t usWidth, uint16_t usHeight);

#endif
