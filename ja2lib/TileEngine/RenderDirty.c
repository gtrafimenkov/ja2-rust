#include "TileEngine/RenderDirty.h"

#include <errno.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "Globals.h"
#include "JAScreens.h"
#include "SGP/Container.h"
#include "SGP/Debug.h"
#include "SGP/VObjectBlitters.h"
#include "SGP/VSurface.h"
#include "SGP/Video.h"
#include "SGP/WCheck.h"
#include "TileEngine/RenderWorld.h"
#include "Utils/FontControl.h"

#ifdef JA2BETAVERSION
#include "Utils/Message.h"
#endif

#define DIRTY_QUEUES 200
#define BACKGROUND_BUFFERS 500
#define VIDEO_OVERLAYS 100

BACKGROUND_SAVE gBackSaves[BACKGROUND_BUFFERS];
UINT32 guiNumBackSaves = 0;

VIDEO_OVERLAY gVideoOverlays[VIDEO_OVERLAYS];
UINT32 guiNumVideoOverlays = 0;

void AllocateVideoOverlayArea(UINT32 uiCount);
void SaveVideoOverlayArea(UINT32 uiSrcBuffer, UINT32 uiCount);

// BACKGROUND_SAVE	gTopmostSaves[BACKGROUND_BUFFERS];
// UINT32 guiNumTopmostSaves=0;

struct GRect gDirtyClipRect = {0, 0, 640, 480};

BOOLEAN gfViewportDirty = FALSE;

BOOLEAN InitializeBaseDirtyRectQueue() { return (TRUE); }

void ShutdownBaseDirtyRectQueue() {}

void AddBaseDirtyRect(INT32 iLeft, INT32 iTop, INT32 iRight, INT32 iBottom) {
  struct GRect aRect;

  if (iLeft < 0) {
    iLeft = 0;
  }
  if (iLeft > 640) {
    iLeft = 640;
  }

  if (iTop < 0) {
    iTop = 0;
  }
  if (iTop > 480) {
    iTop = 480;
  }

  if (iRight < 0) {
    iRight = 0;
  }
  if (iRight > 640) {
    iRight = 640;
  }

  if (iBottom < 0) {
    iBottom = 0;
  }
  if (iBottom > 480) {
    iBottom = 480;
  }

  if ((iRight - iLeft) == 0 || (iBottom - iTop) == 0) {
    return;
  }

  if ((iLeft == gsVIEWPORT_START_X) && (iRight == gsVIEWPORT_END_X) &&
      (iTop == gsVIEWPORT_WINDOW_START_Y) && (iBottom == gsVIEWPORT_WINDOW_END_Y)) {
    gfViewportDirty = TRUE;
    return;
  }

  // Add to list
  aRect.iLeft = iLeft;
  aRect.iTop = iTop;
  aRect.iRight = iRight;
  aRect.iBottom = iBottom;

  InvalidateRegionEx(aRect.iLeft, aRect.iTop, aRect.iRight, aRect.iBottom, 0);
}

BOOLEAN ExecuteBaseDirtyRectQueue() {
  if (gfViewportDirty) {
    // InvalidateRegion(gsVIEWPORT_START_X, gsVIEWPORT_START_Y, gsVIEWPORT_END_X, gsVIEWPORT_END_Y);
    InvalidateScreen();
    EmptyDirtyRectQueue();
    gfViewportDirty = FALSE;
    return (TRUE);
  }

  return (TRUE);
}

BOOLEAN EmptyDirtyRectQueue() { return (TRUE); }

INT32 GetFreeBackgroundBuffer(void) {
  UINT32 uiCount;

  for (uiCount = 0; uiCount < guiNumBackSaves; uiCount++) {
    if ((gBackSaves[uiCount].fAllocated == FALSE) && (gBackSaves[uiCount].fFilled == FALSE))
      return ((INT32)uiCount);
  }

  if (guiNumBackSaves < BACKGROUND_BUFFERS) return ((INT32)guiNumBackSaves++);
#ifdef JA2BETAVERSION
  else {
    // else display an error message
    DebugMsg(TOPIC_JA2, DBG_INFO,
             String("ERROR! GetFreeBackgroundBuffer(): Trying to allocate more saves then there is "
                    "room:  guiCurrentScreen = %d",
                    guiCurrentScreen));
  }
#endif

  return (-1);
}

void RecountBackgrounds(void) {
  INT32 uiCount;

  for (uiCount = guiNumBackSaves - 1; (uiCount >= 0); uiCount--) {
    if ((gBackSaves[uiCount].fAllocated) || (gBackSaves[uiCount].fFilled)) {
      guiNumBackSaves = (UINT32)(uiCount + 1);
      break;
    }
  }
}

INT32 RegisterBackgroundRect(UINT32 uiFlags, INT16 *pSaveArea, INT16 sLeft, INT16 sTop,
                             INT16 sRight, INT16 sBottom) {
  UINT32 uiBufSize;
  INT32 iBackIndex;
  INT32 ClipX1, ClipY1, ClipX2, ClipY2;
  INT32 uiLeftSkip, uiRightSkip, uiTopSkip, uiBottomSkip;
  UINT32 usHeight, usWidth;
  INT32 iTempX, iTempY;

  // Don't register if we are rendering and we are below the viewport
  // if ( sTop >= gsVIEWPORT_WINDOW_END_Y )
  //{
  //	return(-1 );
  //}

  ClipX1 = gDirtyClipRect.iLeft;
  ClipY1 = gDirtyClipRect.iTop;
  ClipX2 = gDirtyClipRect.iRight;
  ClipY2 = gDirtyClipRect.iBottom;

  usHeight = sBottom - sTop;
  usWidth = sRight - sLeft;

  // if((sClipLeft >= sClipRight) || (sClipTop >= sClipBottom))
  //	return(-1);
  iTempX = sLeft;
  iTempY = sTop;

  // Clip to rect
  uiLeftSkip = min(ClipX1 - min(ClipX1, iTempX), (INT32)usWidth);
  uiRightSkip = min(max(ClipX2, (iTempX + (INT32)usWidth)) - ClipX2, (INT32)usWidth);
  uiTopSkip = min(ClipY1 - min(ClipY1, iTempY), (INT32)usHeight);
  uiBottomSkip = min(max(ClipY2, (iTempY + (INT32)usHeight)) - ClipY2, (INT32)usHeight);

  // check if whole thing is clipped
  if ((uiLeftSkip >= (INT32)usWidth) || (uiRightSkip >= (INT32)usWidth)) return (-1);

  // check if whole thing is clipped
  if ((uiTopSkip >= (INT32)usHeight) || (uiBottomSkip >= (INT32)usHeight)) return (-1);

  // Set re-set values given based on clipping
  sLeft = sLeft + (INT16)uiLeftSkip;
  sRight = sRight - (INT16)uiRightSkip;
  sTop = sTop + (INT16)uiTopSkip;
  sBottom = sBottom - (INT16)uiBottomSkip;

  if ((iBackIndex = GetFreeBackgroundBuffer()) == (-1)) return (-1);

  memset(&gBackSaves[iBackIndex], 0, sizeof(BACKGROUND_SAVE));

  gBackSaves[iBackIndex].fZBuffer = FALSE;

  if (pSaveArea == NULL) {
    uiBufSize = ((sRight - sLeft) * 2) * (sBottom - sTop);

    if (uiBufSize == 0) return (-1);

    if (uiFlags & BGND_FLAG_SAVERECT) {
      if ((gBackSaves[iBackIndex].pSaveArea = (UINT16 *)MemAlloc(uiBufSize)) == NULL) return (-1);
    }

    if (uiFlags & BGND_FLAG_SAVE_Z) {
      if ((gBackSaves[iBackIndex].pZSaveArea = (UINT16 *)MemAlloc(uiBufSize)) == NULL) return (-1);
      gBackSaves[iBackIndex].fZBuffer = TRUE;
    }

    gBackSaves[iBackIndex].fFreeMemory = TRUE;
  }
  // else
  //	gBackSaves[iBackIndex].pSaveArea=pSaveArea;

  gBackSaves[iBackIndex].fAllocated = TRUE;
  gBackSaves[iBackIndex].uiFlags = uiFlags;
  gBackSaves[iBackIndex].sLeft = sLeft;
  gBackSaves[iBackIndex].sTop = sTop;
  gBackSaves[iBackIndex].sRight = sRight;
  gBackSaves[iBackIndex].sBottom = sBottom;
  gBackSaves[iBackIndex].sWidth = (sRight - sLeft);
  gBackSaves[iBackIndex].sHeight = (sBottom - sTop);

  gBackSaves[iBackIndex].fFilled = FALSE;

  return (iBackIndex);
}

void SetBackgroundRectFilled(UINT32 uiBackgroundID) {
  gBackSaves[uiBackgroundID].fFilled = TRUE;

  AddBaseDirtyRect(gBackSaves[uiBackgroundID].sLeft, gBackSaves[uiBackgroundID].sTop,
                   gBackSaves[uiBackgroundID].sRight, gBackSaves[uiBackgroundID].sBottom);
}

BOOLEAN RestoreBackgroundRects(void) {
  UINT32 uiCount, uiDestPitchBYTES, uiSrcPitchBYTES;
  UINT8 *pDestBuf, *pSrcBuf;

  pDestBuf = VSurfaceLockOld(vsFB, &uiDestPitchBYTES);
  pSrcBuf = VSurfaceLockOld(vsSB, &uiSrcPitchBYTES);

  for (uiCount = 0; uiCount < guiNumBackSaves; uiCount++) {
    if (gBackSaves[uiCount].fFilled && (!gBackSaves[uiCount].fDisabled)) {
      if (gBackSaves[uiCount].uiFlags & BGND_FLAG_SAVERECT) {
        if (gBackSaves[uiCount].pSaveArea != NULL) {
          Blt16BPPTo16BPP((UINT16 *)pDestBuf, uiDestPitchBYTES,
                          (UINT16 *)gBackSaves[uiCount].pSaveArea, gBackSaves[uiCount].sWidth * 2,
                          gBackSaves[uiCount].sLeft, gBackSaves[uiCount].sTop,
                          NewGRect(0, 0, gBackSaves[uiCount].sWidth, gBackSaves[uiCount].sHeight));

          AddBaseDirtyRect(gBackSaves[uiCount].sLeft, gBackSaves[uiCount].sTop,
                           gBackSaves[uiCount].sRight, gBackSaves[uiCount].sBottom);
        }
      } else if (gBackSaves[uiCount].uiFlags & BGND_FLAG_SAVE_Z) {
        if (gBackSaves[uiCount].fZBuffer) {
          Blt16BPPTo16BPP((UINT16 *)gpZBuffer, uiDestPitchBYTES,
                          (UINT16 *)gBackSaves[uiCount].pZSaveArea, gBackSaves[uiCount].sWidth * 2,
                          gBackSaves[uiCount].sLeft, gBackSaves[uiCount].sTop,
                          NewGRect(0, 0, gBackSaves[uiCount].sWidth, gBackSaves[uiCount].sHeight));
        }
      } else {
        Blt16BPPTo16BPP((UINT16 *)pDestBuf, uiDestPitchBYTES, (UINT16 *)pSrcBuf, uiSrcPitchBYTES,
                        gBackSaves[uiCount].sLeft, gBackSaves[uiCount].sTop,
                        NewGRect(gBackSaves[uiCount].sLeft, gBackSaves[uiCount].sTop,
                                 gBackSaves[uiCount].sWidth, gBackSaves[uiCount].sHeight));

        AddBaseDirtyRect(gBackSaves[uiCount].sLeft, gBackSaves[uiCount].sTop,
                         gBackSaves[uiCount].sRight, gBackSaves[uiCount].sBottom);
      }
    }
  }

  VSurfaceUnlock(vsFB);
  VSurfaceUnlock(vsSB);

  EmptyBackgroundRects();

  return (TRUE);
}

BOOLEAN EmptyBackgroundRects(void) {
  UINT32 uiCount;

  for (uiCount = 0; uiCount < guiNumBackSaves; uiCount++) {
    if (gBackSaves[uiCount].fFilled) {
      gBackSaves[uiCount].fFilled = FALSE;

      if (!(gBackSaves[uiCount].fAllocated) && (gBackSaves[uiCount].fFreeMemory == TRUE)) {
        if (gBackSaves[uiCount].uiFlags & BGND_FLAG_SAVERECT) {
          if (gBackSaves[uiCount].pSaveArea != NULL) {
            MemFree(gBackSaves[uiCount].pSaveArea);
          }
        }
        if (gBackSaves[uiCount].fZBuffer) MemFree(gBackSaves[uiCount].pZSaveArea);

        gBackSaves[uiCount].fZBuffer = FALSE;
        gBackSaves[uiCount].fAllocated = FALSE;
        gBackSaves[uiCount].fFreeMemory = FALSE;
        gBackSaves[uiCount].fFilled = FALSE;
        gBackSaves[uiCount].pSaveArea = NULL;

        RecountBackgrounds();
      }
    }

    if (gBackSaves[uiCount].uiFlags & BGND_FLAG_SINGLE || gBackSaves[uiCount].fPendingDelete) {
      if (gBackSaves[uiCount].fFreeMemory == TRUE) {
        if (gBackSaves[uiCount].uiFlags & BGND_FLAG_SAVERECT) {
          if (gBackSaves[uiCount].pSaveArea != NULL) {
            MemFree(gBackSaves[uiCount].pSaveArea);
          }
        }

        if (gBackSaves[uiCount].fZBuffer) MemFree(gBackSaves[uiCount].pZSaveArea);
      }

      gBackSaves[uiCount].fZBuffer = FALSE;
      gBackSaves[uiCount].fAllocated = FALSE;
      gBackSaves[uiCount].fFreeMemory = FALSE;
      gBackSaves[uiCount].fFilled = FALSE;
      gBackSaves[uiCount].pSaveArea = NULL;
      gBackSaves[uiCount].fPendingDelete = FALSE;

      RecountBackgrounds();
    }
  }

  return (TRUE);
}

BOOLEAN SaveBackgroundRects(void) {
  UINT32 uiCount, uiDestPitchBYTES;
  UINT8 *pSrcBuf;

  pSrcBuf = VSurfaceLockOld(vsFB, &uiDestPitchBYTES);

  for (uiCount = 0; uiCount < guiNumBackSaves; uiCount++) {
    if (gBackSaves[uiCount].fAllocated && (!gBackSaves[uiCount].fDisabled)) {
      if (gBackSaves[uiCount].uiFlags & BGND_FLAG_SAVERECT) {
        if (gBackSaves[uiCount].pSaveArea != NULL) {
          Blt16BPPTo16BPP((UINT16 *)gBackSaves[uiCount].pSaveArea, gBackSaves[uiCount].sWidth * 2,
                          (UINT16 *)pSrcBuf, uiDestPitchBYTES, 0, 0,
                          NewGRect(gBackSaves[uiCount].sLeft, gBackSaves[uiCount].sTop,
                                   gBackSaves[uiCount].sWidth, gBackSaves[uiCount].sHeight));
        }

      } else if (gBackSaves[uiCount].fZBuffer) {
        Blt16BPPTo16BPP(gBackSaves[uiCount].pZSaveArea, gBackSaves[uiCount].sWidth * 2,
                        (UINT16 *)gpZBuffer, uiDestPitchBYTES, 0, 0,
                        NewGRect(gBackSaves[uiCount].sLeft, gBackSaves[uiCount].sTop,
                                 gBackSaves[uiCount].sWidth, gBackSaves[uiCount].sHeight));
      } else {
        AddBaseDirtyRect(gBackSaves[uiCount].sLeft, gBackSaves[uiCount].sTop,
                         gBackSaves[uiCount].sRight, gBackSaves[uiCount].sBottom);
      }

      gBackSaves[uiCount].fFilled = TRUE;
    }
  }

  VSurfaceUnlock(vsFB);
  VSurfaceUnlock(vsSB);

  return (TRUE);
}

BOOLEAN FreeBackgroundRect(INT32 iIndex) {
  if (iIndex != -1) {
    gBackSaves[iIndex].fAllocated = FALSE;

    RecountBackgrounds();
  }

  return (TRUE);
}

BOOLEAN FreeBackgroundRectPending(INT32 iIndex) {
  gBackSaves[iIndex].fPendingDelete = TRUE;

  return (TRUE);
}

BOOLEAN FreeBackgroundRectNow(INT32 uiCount) {
  if (gBackSaves[uiCount].fFreeMemory == TRUE) {
    // MemFree(gBackSaves[uiCount].pSaveArea);
    if (gBackSaves[uiCount].fZBuffer) MemFree(gBackSaves[uiCount].pZSaveArea);
  }

  gBackSaves[uiCount].fZBuffer = FALSE;
  gBackSaves[uiCount].fAllocated = FALSE;
  gBackSaves[uiCount].fFreeMemory = FALSE;
  gBackSaves[uiCount].fFilled = FALSE;
  gBackSaves[uiCount].pSaveArea = NULL;

  RecountBackgrounds();
  return (TRUE);
}

BOOLEAN FreeBackgroundRectType(UINT32 uiFlags) {
  UINT32 uiCount;

  for (uiCount = 0; uiCount < guiNumBackSaves; uiCount++) {
    if (gBackSaves[uiCount].uiFlags & uiFlags) {
      if (gBackSaves[uiCount].fFreeMemory == TRUE) {
        // MemFree(gBackSaves[uiCount].pSaveArea);
        if (gBackSaves[uiCount].fZBuffer) MemFree(gBackSaves[uiCount].pZSaveArea);
      }

      gBackSaves[uiCount].fZBuffer = FALSE;
      gBackSaves[uiCount].fAllocated = FALSE;
      gBackSaves[uiCount].fFreeMemory = FALSE;
      gBackSaves[uiCount].fFilled = FALSE;
      gBackSaves[uiCount].pSaveArea = NULL;
    }
  }

  RecountBackgrounds();

  return (TRUE);
}

BOOLEAN InitializeBackgroundRects(void) {
  guiNumBackSaves = 0;
  return (TRUE);
}

BOOLEAN InvalidateBackgroundRects(void) {
  UINT32 uiCount;

  for (uiCount = 0; uiCount < guiNumBackSaves; uiCount++) gBackSaves[uiCount].fFilled = FALSE;

  return (TRUE);
}

BOOLEAN ShutdownBackgroundRects(void) {
  UINT32 uiCount;

  for (uiCount = 0; uiCount < guiNumBackSaves; uiCount++) {
    if (gBackSaves[uiCount].fAllocated) FreeBackgroundRectNow((INT32)uiCount);
  }

  return (TRUE);
}

void DisableBackgroundRect(INT32 iIndex, BOOLEAN fDisabled) {
  gBackSaves[iIndex].fDisabled = fDisabled;
}

BOOLEAN UpdateSaveBuffer(void) {
  UINT32 uiDestPitchBYTES, uiSrcPitchBYTES;
  UINT8 *pDestBuf, *pSrcBuf;
  UINT16 usWidth, usHeight;

  // Update saved buffer - do for the viewport size ony!
  GetCurrentVideoSettings(&usWidth, &usHeight);

  pSrcBuf = VSurfaceLockOld(vsFB, &uiSrcPitchBYTES);
  pDestBuf = VSurfaceLockOld(vsSB, &uiDestPitchBYTES);

  Blt16BPPTo16BPP((UINT16 *)pDestBuf, uiDestPitchBYTES, (UINT16 *)pSrcBuf, uiSrcPitchBYTES, 0,
                  gsVIEWPORT_WINDOW_START_Y,
                  NewGRect(0, gsVIEWPORT_WINDOW_START_Y, usWidth,
                           (gsVIEWPORT_WINDOW_END_Y - gsVIEWPORT_WINDOW_START_Y)));

  VSurfaceUnlock(vsFB);
  VSurfaceUnlock(vsSB);

  return (TRUE);
}

BOOLEAN RestoreExternBackgroundRect(INT16 sLeft, INT16 sTop, INT16 sWidth, INT16 sHeight) {
  UINT32 uiDestPitchBYTES, uiSrcPitchBYTES;
  UINT8 *pDestBuf, *pSrcBuf;

  Assert((sLeft >= 0) && (sTop >= 0) && (sLeft + sWidth <= 640) && (sTop + sHeight <= 480));

  pDestBuf = VSurfaceLockOld(vsFB, &uiDestPitchBYTES);
  pSrcBuf = VSurfaceLockOld(vsSB, &uiSrcPitchBYTES);

  Blt16BPPTo16BPP((UINT16 *)pDestBuf, uiDestPitchBYTES, (UINT16 *)pSrcBuf, uiSrcPitchBYTES, sLeft,
                  sTop, NewGRect(sLeft, sTop, sWidth, sHeight));
  VSurfaceUnlock(vsFB);
  VSurfaceUnlock(vsSB);

  // Add rect to frame buffer queue
  InvalidateRegionEx(sLeft, sTop, (sLeft + sWidth), (sTop + sHeight), 0);

  return (TRUE);
}

BOOLEAN RestoreExternBackgroundRectGivenID(INT32 iBack) {
  UINT32 uiDestPitchBYTES, uiSrcPitchBYTES;
  INT16 sLeft, sTop, sWidth, sHeight;
  UINT8 *pDestBuf, *pSrcBuf;

  if (!gBackSaves[iBack].fAllocated) {
    return (FALSE);
  }

  sLeft = gBackSaves[iBack].sLeft;
  sTop = gBackSaves[iBack].sTop;
  sWidth = gBackSaves[iBack].sWidth;
  sHeight = gBackSaves[iBack].sHeight;

  Assert((sLeft >= 0) && (sTop >= 0) && (sLeft + sWidth <= 640) && (sTop + sHeight <= 480));

  pDestBuf = VSurfaceLockOld(vsFB, &uiDestPitchBYTES);
  pSrcBuf = VSurfaceLockOld(vsSB, &uiSrcPitchBYTES);

  Blt16BPPTo16BPP((UINT16 *)pDestBuf, uiDestPitchBYTES, (UINT16 *)pSrcBuf, uiSrcPitchBYTES, sLeft,
                  sTop, NewGRect(sLeft, sTop, sWidth, sHeight));
  VSurfaceUnlock(vsFB);
  VSurfaceUnlock(vsSB);

  // Add rect to frame buffer queue
  InvalidateRegionEx(sLeft, sTop, (sLeft + sWidth), (sTop + sHeight), 0);

  return (TRUE);
}

BOOLEAN CopyExternBackgroundRect(INT16 sLeft, INT16 sTop, INT16 sWidth, INT16 sHeight) {
  UINT32 uiDestPitchBYTES, uiSrcPitchBYTES;
  UINT8 *pDestBuf, *pSrcBuf;

  Assert((sLeft >= 0) && (sTop >= 0) && (sLeft + sWidth <= 640) && (sTop + sHeight <= 480));

  pDestBuf = VSurfaceLockOld(vsSB, &uiDestPitchBYTES);
  pSrcBuf = VSurfaceLockOld(vsFB, &uiSrcPitchBYTES);

  Blt16BPPTo16BPP((UINT16 *)pDestBuf, uiDestPitchBYTES, (UINT16 *)pSrcBuf, uiSrcPitchBYTES, sLeft,
                  sTop, NewGRect(sLeft, sTop, sWidth, sHeight));
  VSurfaceUnlock(vsSB);
  VSurfaceUnlock(vsFB);

  return (TRUE);
}

//*****************************************************************************
// gprintfdirty
//
//		Dirties a single-frame rect exactly the size needed to save the
// background for a given call to gprintf. Note that this must be called before
// the backgrounds are saved, and before the actual call to gprintf that writes
// to the video buffer.
//
//*****************************************************************************
UINT16 gprintfdirty(INT16 x, INT16 y, STR16 pFontString, ...) {
  va_list argptr;
  wchar_t string[512];
  UINT16 uiStringLength, uiStringHeight;
  INT32 iBack;

  Assert(pFontString != NULL);

  va_start(argptr, pFontString);  // Set up variable argument pointer
  vswprintf(string, ARR_SIZE(string), pFontString,
            argptr);  // process gprintf string (get output str)
  va_end(argptr);

  uiStringLength = StringPixLength(string, FontDefault);
  uiStringHeight = GetFontHeight(FontDefault);

  if (uiStringLength > 0) {
    iBack = RegisterBackgroundRect(BGND_FLAG_SINGLE, NULL, x, y, (INT16)(x + uiStringLength),
                                   (INT16)(y + uiStringHeight));

    if (iBack != -1) {
      SetBackgroundRectFilled(iBack);
    }
  }

  return (uiStringLength);
}

UINT16 gprintfinvalidate(INT16 x, INT16 y, STR16 pFontString, ...) {
  va_list argptr;
  wchar_t string[512];
  UINT16 uiStringLength, uiStringHeight;

  Assert(pFontString != NULL);

  va_start(argptr, pFontString);  // Set up variable argument pointer
  vswprintf(string, ARR_SIZE(string), pFontString,
            argptr);  // process gprintf string (get output str)
  va_end(argptr);

  uiStringLength = StringPixLength(string, FontDefault);
  uiStringHeight = GetFontHeight(FontDefault);

  if (uiStringLength > 0) {
    InvalidateRegionEx(x, y, (INT16)(x + uiStringLength), (INT16)(y + uiStringHeight), 0);
  }
  return (uiStringLength);
}

UINT16 gprintfRestore(INT16 x, INT16 y, STR16 pFontString, ...) {
  va_list argptr;
  wchar_t string[512];
  UINT16 uiStringLength, uiStringHeight;

  Assert(pFontString != NULL);

  va_start(argptr, pFontString);  // Set up variable argument pointer
  vswprintf(string, ARR_SIZE(string), pFontString,
            argptr);  // process gprintf string (get output str)
  va_end(argptr);

  uiStringLength = StringPixLength(string, FontDefault);
  uiStringHeight = GetFontHeight(FontDefault);

  if (uiStringLength > 0) {
    RestoreExternBackgroundRect(x, y, uiStringLength, uiStringHeight);
  }

  return (uiStringLength);
}

// OVERLAY STUFF
INT32 GetFreeVideoOverlay(void) {
  UINT32 uiCount;

  for (uiCount = 0; uiCount < guiNumVideoOverlays; uiCount++) {
    if ((gVideoOverlays[uiCount].fAllocated == FALSE)) return ((INT32)uiCount);
  }

  if (guiNumVideoOverlays < BACKGROUND_BUFFERS) return ((INT32)guiNumVideoOverlays++);

  return (-1);
}

void RecountVideoOverlays(void) {
  INT32 uiCount;

  for (uiCount = guiNumVideoOverlays - 1; (uiCount >= 0); uiCount--) {
    if ((gVideoOverlays[uiCount].fAllocated)) {
      guiNumVideoOverlays = (UINT32)(uiCount + 1);
      break;
    }
  }
}

INT32 RegisterVideoOverlay(UINT32 uiFlags, VIDEO_OVERLAY_DESC *pTopmostDesc) {
  UINT32 iBlitterIndex;
  UINT32 iBackIndex;
  UINT16 uiStringLength, uiStringHeight;

  if (uiFlags & VOVERLAY_DIRTYBYTEXT) {
    // Get dims by supplied text
    uiStringLength = StringPixLength(pTopmostDesc->pzText, pTopmostDesc->uiFontID);
    uiStringHeight = GetFontHeight(pTopmostDesc->uiFontID);

    iBackIndex =
        RegisterBackgroundRect(BGND_FLAG_PERMANENT, NULL, pTopmostDesc->sLeft, pTopmostDesc->sTop,
                               (INT16)(pTopmostDesc->sLeft + uiStringLength),
                               (INT16)(pTopmostDesc->sTop + uiStringHeight));

  } else {
    // Register background
    iBackIndex =
        RegisterBackgroundRect(BGND_FLAG_PERMANENT, NULL, pTopmostDesc->sLeft, pTopmostDesc->sTop,
                               pTopmostDesc->sRight, pTopmostDesc->sBottom);
  }

  if (iBackIndex == -1) {
    return (-1);
  }

  // Get next free topmost blitter index
  if ((iBlitterIndex = GetFreeVideoOverlay()) == (-1)) return (-1);

  // Init new blitter
  memset(&gVideoOverlays[iBlitterIndex], 0, sizeof(VIDEO_OVERLAY));

  gVideoOverlays[iBlitterIndex].uiFlags = uiFlags;
  gVideoOverlays[iBlitterIndex].fAllocated = 2;
  gVideoOverlays[iBlitterIndex].uiBackground = iBackIndex;
  gVideoOverlays[iBlitterIndex].pBackground = &(gBackSaves[iBackIndex]);
  gVideoOverlays[iBlitterIndex].BltCallback = pTopmostDesc->BltCallback;

  // Update blitter info
  // Set update flags to zero since we are forcing all updates
  pTopmostDesc->uiFlags = 0;
  UpdateVideoOverlay(pTopmostDesc, iBlitterIndex, TRUE);

  // Set disabled flag to true
  if (uiFlags & VOVERLAY_STARTDISABLED) {
    gVideoOverlays[iBlitterIndex].fDisabled = TRUE;
    DisableBackgroundRect(gVideoOverlays[iBlitterIndex].uiBackground, TRUE);
  }

  gVideoOverlays[iBlitterIndex].uiDestBuff = FRAME_BUFFER;

  // DebugMsg( TOPIC_JA2, DBG_ERROR, String( "Register Overlay %d %S", iBlitterIndex,
  // gVideoOverlays[ iBlitterIndex ].zText ) );

  return (iBlitterIndex);
}

void SetVideoOverlayPendingDelete(INT32 iVideoOverlay) {
  if (iVideoOverlay != -1) {
    gVideoOverlays[iVideoOverlay].fDeletionPending = TRUE;
  }
}

void RemoveVideoOverlay(INT32 iVideoOverlay) {
  if (iVideoOverlay != -1 && gVideoOverlays[iVideoOverlay].fAllocated) {
    // Check if we are actively scrolling
    if (gVideoOverlays[iVideoOverlay].fActivelySaving) {
      //		DebugMsg( TOPIC_JA2, DBG_ERROR, String( "Overlay Actively saving %d %S",
      // iVideoOverlay, gVideoOverlays[ iVideoOverlay ].zText ) );

      gVideoOverlays[iVideoOverlay].fDeletionPending = TRUE;
    } else {
      // RestoreExternBackgroundRectGivenID( gVideoOverlays[ iVideoOverlay ].uiBackground );

      // Remove background
      FreeBackgroundRect(gVideoOverlays[iVideoOverlay].uiBackground);

      // DebugMsg( TOPIC_JA2, DBG_ERROR, String( "Delete Overlay %d %S", iVideoOverlay,
      // gVideoOverlays[ iVideoOverlay ].zText ) );

      // Remove save buffer if not done so
      if (gVideoOverlays[iVideoOverlay].pSaveArea != NULL) {
        MemFree(gVideoOverlays[iVideoOverlay].pSaveArea);
      }
      gVideoOverlays[iVideoOverlay].pSaveArea = NULL;

      // Set as not allocated
      gVideoOverlays[iVideoOverlay].fAllocated = FALSE;
    }
  }
}

BOOLEAN UpdateVideoOverlay(VIDEO_OVERLAY_DESC *pTopmostDesc, UINT32 iBlitterIndex,
                           BOOLEAN fForceAll) {
  UINT32 uiFlags;
  UINT16 uiStringLength, uiStringHeight;

  if (iBlitterIndex != -1) {
    if (!gVideoOverlays[iBlitterIndex].fAllocated) {
      return (FALSE);
    }

    uiFlags = pTopmostDesc->uiFlags;

    if (fForceAll) {
      gVideoOverlays[iBlitterIndex].uiFontID = pTopmostDesc->uiFontID;
      gVideoOverlays[iBlitterIndex].sX = pTopmostDesc->sX;
      gVideoOverlays[iBlitterIndex].sY = pTopmostDesc->sY;
      gVideoOverlays[iBlitterIndex].ubFontBack = pTopmostDesc->ubFontBack;
      gVideoOverlays[iBlitterIndex].ubFontFore = pTopmostDesc->ubFontFore;

      wcscpy(gVideoOverlays[iBlitterIndex].zText, pTopmostDesc->pzText);
    } else {
      if (uiFlags & VOVERLAY_DESC_TEXT) {
        wcscpy(gVideoOverlays[iBlitterIndex].zText, pTopmostDesc->pzText);
      }

      if (uiFlags & VOVERLAY_DESC_DISABLED) {
        gVideoOverlays[iBlitterIndex].fDisabled = pTopmostDesc->fDisabled;
        DisableBackgroundRect(gVideoOverlays[iBlitterIndex].uiBackground, pTopmostDesc->fDisabled);
      }

      // If position has changed and flags are of type that use dirty rects, adjust
      if ((uiFlags & VOVERLAY_DESC_POSITION)) {
        if (gVideoOverlays[iBlitterIndex].uiFlags & VOVERLAY_DIRTYBYTEXT) {
          // Get dims by supplied text
          uiStringLength = StringPixLength(gVideoOverlays[iBlitterIndex].zText,
                                           gVideoOverlays[iBlitterIndex].uiFontID);
          uiStringHeight = GetFontHeight(gVideoOverlays[iBlitterIndex].uiFontID);

          // Delete old rect
          // Remove background
          FreeBackgroundRectPending(gVideoOverlays[iBlitterIndex].uiBackground);

          gVideoOverlays[iBlitterIndex].uiBackground = RegisterBackgroundRect(
              BGND_FLAG_PERMANENT, NULL, pTopmostDesc->sLeft, pTopmostDesc->sTop,
              (INT16)(pTopmostDesc->sLeft + uiStringLength),
              (INT16)(pTopmostDesc->sTop + uiStringHeight));
          gVideoOverlays[iBlitterIndex].sX = pTopmostDesc->sX;
          gVideoOverlays[iBlitterIndex].sY = pTopmostDesc->sY;
        }
      }
    }
  }
  return (TRUE);
}

// FUnctions for entrie array of blitters
void ExecuteVideoOverlays() {
  UINT32 uiCount;

  for (uiCount = 0; uiCount < guiNumVideoOverlays; uiCount++) {
    if (gVideoOverlays[uiCount].fAllocated) {
      if (!gVideoOverlays[uiCount].fDisabled) {
        // If we are scrolling but havn't saved yet, don't!
        if (!gVideoOverlays[uiCount].fActivelySaving && gfScrollInertia > 0) {
          continue;
        }

        // ATE: Wait a frame before executing!
        if (gVideoOverlays[uiCount].fAllocated == 1) {
          // Call Blit Function
          (*(gVideoOverlays[uiCount].BltCallback))(&(gVideoOverlays[uiCount]));
        } else if (gVideoOverlays[uiCount].fAllocated == 2) {
          gVideoOverlays[uiCount].fAllocated = 1;
        }
      }

      // Remove if pending
      // if ( gVideoOverlays[uiCount].fDeletionPending )
      //{
      //	RemoveVideoOverlay( uiCount );
      //}
    }
  }
}

void ExecuteVideoOverlaysToAlternateBuffer(UINT32 uiNewDestBuffer) {
  UINT32 uiCount;
  UINT32 uiOldDestBuffer;

  for (uiCount = 0; uiCount < guiNumVideoOverlays; uiCount++) {
    if (gVideoOverlays[uiCount].fAllocated && !gVideoOverlays[uiCount].fDisabled) {
      if (gVideoOverlays[uiCount].fActivelySaving) {
        uiOldDestBuffer = gVideoOverlays[uiCount].uiDestBuff;

        gVideoOverlays[uiCount].uiDestBuff = uiNewDestBuffer;

        // Call Blit Function
        (*(gVideoOverlays[uiCount].BltCallback))(&(gVideoOverlays[uiCount]));

        gVideoOverlays[uiCount].uiDestBuff = uiOldDestBuffer;
      }
    }
  }
}

void AllocateVideoOverlaysArea() {
  UINT32 uiCount;
  UINT32 uiBufSize;
  UINT32 iBackIndex;

  for (uiCount = 0; uiCount < guiNumVideoOverlays; uiCount++) {
    if (gVideoOverlays[uiCount].fAllocated && !gVideoOverlays[uiCount].fDisabled) {
      iBackIndex = gVideoOverlays[uiCount].uiBackground;

      // Get buffer size
      uiBufSize = ((gBackSaves[iBackIndex].sRight - gBackSaves[iBackIndex].sLeft) * 2) *
                  (gBackSaves[iBackIndex].sBottom - gBackSaves[iBackIndex].sTop);

      gVideoOverlays[uiCount].fActivelySaving = TRUE;

      // DebugMsg( TOPIC_JA2, DBG_ERROR, String( "Setting Overlay Actively saving %d %S", uiCount,
      // gVideoOverlays[ uiCount ].zText ) );

      // Allocate
      if ((gVideoOverlays[uiCount].pSaveArea = (UINT16 *)MemAlloc(uiBufSize)) == NULL) {
        continue;
      }
    }
  }
}

void AllocateVideoOverlayArea(UINT32 uiCount) {
  UINT32 uiBufSize;
  UINT32 iBackIndex;

  if (gVideoOverlays[uiCount].fAllocated && !gVideoOverlays[uiCount].fDisabled) {
    iBackIndex = gVideoOverlays[uiCount].uiBackground;

    // Get buffer size
    uiBufSize = ((gBackSaves[iBackIndex].sRight - gBackSaves[iBackIndex].sLeft) * 2) *
                (gBackSaves[iBackIndex].sBottom - gBackSaves[iBackIndex].sTop);

    gVideoOverlays[uiCount].fActivelySaving = TRUE;

    // DebugMsg( TOPIC_JA2, DBG_ERROR, String( "Setting Overlay Actively saving %d %S", uiCount,
    // gVideoOverlays[ uiCount ].zText ) );

    // Allocate
    if ((gVideoOverlays[uiCount].pSaveArea = (UINT16 *)MemAlloc(uiBufSize)) == NULL) {
    }
  }
}

void SaveVideoOverlaysArea(struct VSurface *src) {
  UINT32 uiCount;
  UINT32 iBackIndex;
  UINT32 uiSrcPitchBYTES;
  UINT8 *pSrcBuf;

  pSrcBuf = VSurfaceLockOld(src, &uiSrcPitchBYTES);

  for (uiCount = 0; uiCount < guiNumVideoOverlays; uiCount++) {
    if (gVideoOverlays[uiCount].fAllocated && !gVideoOverlays[uiCount].fDisabled) {
      // OK, if our saved area is null, allocate it here!
      if (gVideoOverlays[uiCount].pSaveArea == NULL) {
        AllocateVideoOverlayArea(uiCount);
      }

      if (gVideoOverlays[uiCount].pSaveArea != NULL) {
        iBackIndex = gVideoOverlays[uiCount].uiBackground;

        // Save data from frame buffer!
        Blt16BPPTo16BPP((UINT16 *)gVideoOverlays[uiCount].pSaveArea,
                        gBackSaves[iBackIndex].sWidth * 2, (UINT16 *)pSrcBuf, uiSrcPitchBYTES, 0, 0,
                        NewGRect(gBackSaves[iBackIndex].sLeft, gBackSaves[iBackIndex].sTop,
                                 gBackSaves[iBackIndex].sWidth, gBackSaves[iBackIndex].sHeight));
      }
    }
  }

  VSurfaceUnlock(src);
}

void SaveVideoOverlayArea(UINT32 uiSrcBuffer, UINT32 uiCount) {
  UINT32 iBackIndex;
  UINT32 uiSrcPitchBYTES;
  UINT8 *pSrcBuf;

  pSrcBuf = VSurfaceLockOld(GetVSByID(uiSrcBuffer), &uiSrcPitchBYTES);

  if (gVideoOverlays[uiCount].fAllocated && !gVideoOverlays[uiCount].fDisabled) {
    // OK, if our saved area is null, allocate it here!
    if (gVideoOverlays[uiCount].pSaveArea == NULL) {
      AllocateVideoOverlayArea(uiCount);
    }

    if (gVideoOverlays[uiCount].pSaveArea != NULL) {
      iBackIndex = gVideoOverlays[uiCount].uiBackground;

      // Save data from frame buffer!
      Blt16BPPTo16BPP((UINT16 *)gVideoOverlays[uiCount].pSaveArea,
                      gBackSaves[iBackIndex].sWidth * 2, (UINT16 *)pSrcBuf, uiSrcPitchBYTES, 0, 0,
                      NewGRect(gBackSaves[iBackIndex].sLeft, gBackSaves[iBackIndex].sTop,
                               gBackSaves[iBackIndex].sWidth, gBackSaves[iBackIndex].sHeight));
    }
  }

  VSurfaceUnlock(GetVSByID(uiSrcBuffer));
}

void DeleteVideoOverlaysArea() {
  UINT32 uiCount;

  for (uiCount = 0; uiCount < guiNumVideoOverlays; uiCount++) {
    if (gVideoOverlays[uiCount].fAllocated && !gVideoOverlays[uiCount].fDisabled) {
      if (gVideoOverlays[uiCount].pSaveArea != NULL) {
        MemFree(gVideoOverlays[uiCount].pSaveArea);
      }

      gVideoOverlays[uiCount].fActivelySaving = FALSE;

      gVideoOverlays[uiCount].pSaveArea = NULL;

      // DebugMsg( TOPIC_JA2, DBG_ERROR, String( "Removing Overlay Actively saving %d %S",
      // uiCount, gVideoOverlays[ uiCount ].zText ) );

      // Remove if pending
      if (gVideoOverlays[uiCount].fDeletionPending) {
        RemoveVideoOverlay(uiCount);
      }
    }
  }
}

BOOLEAN RestoreShiftedVideoOverlays(INT16 sShiftX, INT16 sShiftY) {
  UINT32 uiCount, uiDestPitchBYTES;
  UINT8 *pDestBuf;
  UINT32 iBackIndex;

  INT32 ClipX1, ClipY1, ClipX2, ClipY2;
  INT32 uiLeftSkip, uiRightSkip, uiTopSkip, uiBottomSkip;
  UINT32 usHeight, usWidth;
  INT32 iTempX, iTempY;
  INT16 sLeft, sTop, sRight, sBottom;

  ClipX1 = 0;
  ClipY1 = gsVIEWPORT_WINDOW_START_Y;
  ClipX2 = 640;
  ClipY2 = gsVIEWPORT_WINDOW_END_Y - 1;

  pDestBuf = VSurfaceLockOld(vsBB, &uiDestPitchBYTES);

  for (uiCount = 0; uiCount < guiNumVideoOverlays; uiCount++) {
    if (gVideoOverlays[uiCount].fAllocated && !gVideoOverlays[uiCount].fDisabled) {
      iBackIndex = gVideoOverlays[uiCount].uiBackground;

      if (gVideoOverlays[uiCount].pSaveArea != NULL) {
        // Get restore background values
        sLeft = gBackSaves[iBackIndex].sLeft;
        sTop = gBackSaves[iBackIndex].sTop;
        sRight = gBackSaves[iBackIndex].sRight;
        sBottom = gBackSaves[iBackIndex].sBottom;
        usHeight = gBackSaves[iBackIndex].sHeight;
        usWidth = gBackSaves[iBackIndex].sWidth;

        // Clip!!
        iTempX = sLeft + sShiftX;
        iTempY = sTop + sShiftY;

        // Clip to rect
        uiLeftSkip = min(ClipX1 - min(ClipX1, iTempX), (INT32)usWidth);
        uiRightSkip = min(max(ClipX2, (iTempX + (INT32)usWidth)) - ClipX2, (INT32)usWidth);
        uiTopSkip = min(ClipY1 - min(ClipY1, iTempY), (INT32)usHeight);
        uiBottomSkip = min(max(ClipY2, (iTempY + (INT32)usHeight)) - ClipY2, (INT32)usHeight);

        // check if whole thing is clipped
        if ((uiLeftSkip >= (INT32)usWidth) || (uiRightSkip >= (INT32)usWidth)) continue;

        // check if whole thing is clipped
        if ((uiTopSkip >= (INT32)usHeight) || (uiBottomSkip >= (INT32)usHeight)) continue;

        // Set re-set values given based on clipping
        sLeft = iTempX + (INT16)uiLeftSkip;
        sTop = iTempY + (INT16)uiTopSkip;
        sRight = sRight + sShiftX - (INT16)uiRightSkip;
        sBottom = sBottom + sShiftY - (INT16)uiBottomSkip;

        usHeight = sBottom - sTop;
        usWidth = sRight - sLeft;

        Blt16BPPTo16BPP((UINT16 *)(UINT16 *)pDestBuf, uiDestPitchBYTES,
                        (UINT16 *)gVideoOverlays[uiCount].pSaveArea,
                        gBackSaves[iBackIndex].sWidth * 2, sLeft, sTop,
                        NewGRect(uiLeftSkip, uiTopSkip, usWidth, usHeight));

        // Once done, check for pending deletion
        if (gVideoOverlays[uiCount].fDeletionPending) {
          RemoveVideoOverlay(uiCount);
        }
      }
    }
  }

  VSurfaceUnlock(vsBB);

  return (TRUE);
}

BOOLEAN SetOverlayUserData(INT32 iVideoOverlay, UINT8 ubNum, UINT32 uiData) {
  if (!gVideoOverlays[iVideoOverlay].fAllocated) {
    return (FALSE);
  }

  if (ubNum > 4) {
    return (FALSE);
  }

  gVideoOverlays[iVideoOverlay].uiUserData[ubNum] = uiData;

  return (TRUE);
}

// Common callbacks for topmost blitters
void BlitMFont(VIDEO_OVERLAY *pBlitter) {
  UINT8 *pDestBuf;
  UINT32 uiDestPitchBYTES;

  pDestBuf = VSurfaceLockOld(GetVSByID(pBlitter->uiDestBuff), &uiDestPitchBYTES);

  SetFont(pBlitter->uiFontID);
  SetFontBackground(pBlitter->ubFontBack);
  SetFontForeground(pBlitter->ubFontFore);

  mprintf_buffer(pDestBuf, uiDestPitchBYTES, pBlitter->uiFontID, pBlitter->sX, pBlitter->sY,
                 pBlitter->zText);

  VSurfaceUnlock(GetVSByID(pBlitter->uiDestBuff));
}

bool VSurfaceBlitBufToBuf(struct VSurface *src, struct VSurface *dest, u16 x, u16 y, u16 width,
                          u16 height) {
  struct BufferLockInfo srcLock = VSurfaceLock(src);
  struct BufferLockInfo destLock = VSurfaceLock(dest);

  bool res = Blt16BPPTo16BPP((UINT16 *)destLock.dest, destLock.pitch, (UINT16 *)srcLock.dest,
                             srcLock.pitch, x, y, NewGRect(x, y, width, height));

  VSurfaceUnlock(src);
  VSurfaceUnlock(dest);

  return (res);
}

void EnableVideoOverlay(BOOLEAN fEnable, INT32 iOverlayIndex) {
  VIDEO_OVERLAY_DESC VideoOverlayDesc;

  memset(&VideoOverlayDesc, 0, sizeof(VideoOverlayDesc));

  // enable or disable
  VideoOverlayDesc.fDisabled = !fEnable;

  // go play with enable/disable state
  VideoOverlayDesc.uiFlags = VOVERLAY_DESC_DISABLED;

  UpdateVideoOverlay(&VideoOverlayDesc, iOverlayIndex, FALSE);
}
