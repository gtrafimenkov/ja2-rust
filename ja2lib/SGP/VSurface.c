#include "VSurface.h"

#include "Local.h"
#include "Rect.h"
#include "SGP/Debug.h"
#include "SGP/MemMan.h"
#include "SGP/Types.h"
#include "SGP/VObjectBlitters.h"
#include "SGP/VObjectInternal.h"
#include "SGP/VSurfaceInternal.h"
#include "SGP/WCheck.h"
#include "TileEngine/RenderWorld.h"
#include "platform.h"

struct VSurface *vsPrimary = NULL;
struct VSurface *vsBackBuffer = NULL;
struct VSurface *vsFrameBuffer = NULL;
struct VSurface *vsMouseCursor = NULL;
struct VSurface *vsMouseCursorOriginal = NULL;

//
// Refresh thread based variables
//

UINT32 guiFrameBufferState;    // BUFFER_READY, BUFFER_DIRTY
UINT32 guiMouseBufferState;    // BUFFER_READY, BUFFER_DIRTY, BUFFER_DISABLED
UINT32 guiVideoManagerState;   // VIDEO_ON, VIDEO_OFF, VIDEO_SUSPENDED, VIDEO_SHUTTING_DOWN
UINT32 guiRefreshThreadState;  // THREAD_ON, THREAD_OFF, THREAD_SUSPENDED

//
// Dirty rectangle management variables
//

SGPRect gListOfDirtyRegions[MAX_DIRTY_REGIONS];
UINT32 guiDirtyRegionCount;
BOOLEAN gfForceFullScreenRefresh;

SGPRect gDirtyRegionsEx[MAX_DIRTY_REGIONS];
UINT32 gDirtyRegionsFlagsEx[MAX_DIRTY_REGIONS];
UINT32 guiDirtyRegionExCount;

typedef struct VSURFACE_NODE {
  struct VSurface *hVSurface;
  UINT32 uiIndex;
  struct VSURFACE_NODE *next, *prev;
} VSURFACE_NODE;

VSURFACE_NODE *gpVSurfaceHead = NULL;
VSURFACE_NODE *gpVSurfaceTail = NULL;
UINT32 guiVSurfaceIndex = 0;
UINT32 guiVSurfaceSize = 0;
UINT32 guiVSurfaceTotalAdded = 0;

INT32 giMemUsedInSurfaces;

// Given an struct Image* object, blit imagery into existing Video Surface. Can be from 8->16
// BPP
BOOLEAN SetVideoSurfaceDataFromHImage(struct VSurface *hVSurface, struct Image *hImage, UINT16 usX,
                                      UINT16 usY, SGPRect *pSrcRect) {
  UINT32 fBufferBPP = 0;
  UINT16 usEffectiveWidth;
  SGPRect aRect;

  // Assertions
  Assert(hVSurface != NULL);
  Assert(hImage != NULL);

  // Get Size of hImage and determine if it can fit
  if (!(hImage->usWidth >= hVSurface->usWidth)) {
    return FALSE;
  }
  if (!(hImage->usHeight >= hVSurface->usHeight)) {
    return FALSE;
  }

  // Check BPP and see if they are the same
  if (hImage->ubBitDepth != hVSurface->ubBitDepth) {
    // They are not the same, but we can go from 8->16 without much cost
    if (hImage->ubBitDepth == 8 && hVSurface->ubBitDepth == 16) {
      fBufferBPP = BUFFER_16BPP;
    }
  } else {
    // Set buffer BPP
    switch (hImage->ubBitDepth) {
      case 8:

        fBufferBPP = BUFFER_8BPP;
        break;

      case 16:

        fBufferBPP = BUFFER_16BPP;
        break;
    }
  }

  Assert(fBufferBPP != 0);

  // Get surface buffer data
  struct BufferLockInfo lock = VSurfaceLock(hVSurface);

  // Effective width ( in PIXELS ) is Pitch ( in bytes ) converted to pitch ( IN PIXELS )
  usEffectiveWidth = (UINT16)(lock.pitch / (hVSurface->ubBitDepth / 8));

  if (!lock.dest) {
    return FALSE;
  }

  // Blit Surface
  // If rect is NULL, use entrie image size
  if (pSrcRect == NULL) {
    aRect.iLeft = 0;
    aRect.iTop = 0;
    aRect.iRight = hImage->usWidth;
    aRect.iBottom = hImage->usHeight;
  } else {
    aRect.iLeft = pSrcRect->iLeft;
    aRect.iTop = pSrcRect->iTop;
    aRect.iRight = pSrcRect->iRight;
    aRect.iBottom = pSrcRect->iBottom;
  }

  // This struct Image* function will transparently copy buffer
  if (!CopyImageToBuffer(hImage, fBufferBPP, lock.dest, usEffectiveWidth, hVSurface->usHeight, usX,
                         usY, &aRect)) {
    DebugMsg(TOPIC_VIDEOSURFACE, DBG_NORMAL,
             String("Error Occured Copying struct Image* to struct VSurface*"));
    UnLockVideoSurfaceBuffer(hVSurface);
    return (FALSE);
  }

  // All is OK
  UnLockVideoSurfaceBuffer(hVSurface);

  return (TRUE);
}

BOOLEAN DeleteVideoSurfaceFromIndex(VSurfID uiIndex) {
  VSURFACE_NODE *curr;

  curr = gpVSurfaceHead;
  while (curr) {
    if (curr->uiIndex == uiIndex) {
      // Found the node, so detach it and delete it.
      // Deallocate the memory for the video surface
      DeleteVideoSurface(curr->hVSurface);

      if (curr == gpVSurfaceHead) {
        // Advance the head, because we are going to remove the head node.
        gpVSurfaceHead = gpVSurfaceHead->next;
      }
      if (curr == gpVSurfaceTail) {
        // Back up the tail, because we are going to remove the tail node.
        gpVSurfaceTail = gpVSurfaceTail->prev;
      }
      // Detach the node from the vsurface list
      if (curr->next) {
        // Make the prev node point to the next
        curr->next->prev = curr->prev;
      }
      if (curr->prev) {
        // Make the next node point to the prev
        curr->prev->next = curr->next;
      }
      // The node is now detached.  Now deallocate it.
      MemFree(curr);
      guiVSurfaceSize--;
      return TRUE;
    }
    curr = curr->next;
  }
  return FALSE;
}

// ********************************************************
//
// Region manipulation functions
//
// ********************************************************

static BOOLEAN GetVSurfaceRect(struct VSurface *hVSurface, struct Rect *pRect) {
  Assert(hVSurface != NULL);
  Assert(pRect != NULL);

  pRect->left = 0;
  pRect->top = 0;
  pRect->right = hVSurface->usWidth;
  pRect->bottom = hVSurface->usHeight;

  return (TRUE);
}

// *******************************************************************
//
// Blitting Functions
//
// *******************************************************************

// Blt  will use DD Blt or BltFast depending on flags.
// Will drop down into user-defined blitter if 8->16 BPP blitting is being done

BOOLEAN BltVideoSurfaceToVideoSurface(struct VSurface *hDestVSurface, struct VSurface *hSrcVSurface,
                                      INT32 iDestX, INT32 iDestY, INT32 fBltFlags,
                                      struct BltOpts *pBltFx) {
  VSURFACE_REGION aRegion;
  struct Rect SrcRect, DestRect;
  UINT32 uiWidth, uiHeight;

  // Assertions
  Assert(hDestVSurface != NULL);

  // Check for source coordinate options - from region, specific rect or full src dimensions
  do {
    // Use SUBRECT if specified
    if (fBltFlags & VS_BLT_SRCSUBRECT) {
      SGPRect aSubRect;

      if (!(pBltFx != NULL)) {
        return FALSE;
      }

      aSubRect = pBltFx->SrcRect;

      SrcRect.top = (int)aSubRect.iTop;
      SrcRect.left = (int)aSubRect.iLeft;
      SrcRect.bottom = (int)aSubRect.iBottom;
      SrcRect.right = (int)aSubRect.iRight;

      break;
    }

    // Here, use default, which is entire Video Surface
    // Check Sizes, SRC size MUST be <= DEST size
    if (hDestVSurface->usHeight < hSrcVSurface->usHeight) {
      DebugMsg(TOPIC_VIDEOSURFACE, DBG_NORMAL,
               String("Incompatible height size given in Video Surface blit"));
      return (FALSE);
    }
    if (hDestVSurface->usWidth < hSrcVSurface->usWidth) {
      DebugMsg(TOPIC_VIDEOSURFACE, DBG_NORMAL,
               String("Incompatible height size given in Video Surface blit"));
      return (FALSE);
    }

    SrcRect.top = (int)0;
    SrcRect.left = (int)0;
    SrcRect.bottom = (int)hSrcVSurface->usHeight;
    SrcRect.right = (int)hSrcVSurface->usWidth;

  } while (FALSE);

  // Once here, assert valid Src
  Assert(hSrcVSurface != NULL);

  // clipping -- added by DB
  GetVSurfaceRect(hDestVSurface, &DestRect);
  uiWidth = SrcRect.right - SrcRect.left;
  uiHeight = SrcRect.bottom - SrcRect.top;

  // check for position entirely off the screen
  if (iDestX >= DestRect.right) return (FALSE);
  if (iDestY >= DestRect.bottom) return (FALSE);
  if ((iDestX + (INT32)uiWidth) < (INT32)DestRect.left) return (FALSE);
  if ((iDestY + (INT32)uiHeight) < (INT32)DestRect.top) return (FALSE);

  {
    if ((iDestX + (INT32)uiWidth) >= (INT32)DestRect.right) {
      SrcRect.right -= ((iDestX + uiWidth) - DestRect.right);
      uiWidth -= ((iDestX + uiWidth) - DestRect.right);
    }
    if ((iDestY + (INT32)uiHeight) >= (INT32)DestRect.bottom) {
      SrcRect.bottom -= ((iDestY + uiHeight) - DestRect.bottom);
      uiHeight -= ((iDestY + uiHeight) - DestRect.bottom);
    }
    if (iDestX < DestRect.left) {
      SrcRect.left += (DestRect.left - iDestX);
      uiWidth -= (DestRect.left - iDestX);
      iDestX = DestRect.left;
    }
    if (iDestY < (INT32)DestRect.top) {
      SrcRect.top += (DestRect.top - iDestY);
      uiHeight -= (DestRect.top - iDestY);
      iDestY = DestRect.top;
    }
  }

  // Send dest position, rectangle, etc to DD bltfast function
  // First check BPP values for compatibility
  if (hDestVSurface->ubBitDepth == 16 && hSrcVSurface->ubBitDepth == 16) {
    struct Rect srcRect = {SrcRect.left, SrcRect.top, SrcRect.right, SrcRect.bottom};
    if (!(BltVSurfaceUsingDD(hDestVSurface, hSrcVSurface, fBltFlags, iDestX, iDestY, &srcRect))) {
      return FALSE;
    }

  } else if (hDestVSurface->ubBitDepth == 8 && hSrcVSurface->ubBitDepth == 8) {
    struct BufferLockInfo srcLock = VSurfaceLock(hSrcVSurface);
    if (!srcLock.dest) {
      DebugMsg(TOPIC_VIDEOSURFACE, DBG_NORMAL, "Failed on lock of 8BPP surface for blitting");
      return (FALSE);
    }

    struct BufferLockInfo destLock = VSurfaceLock(hDestVSurface);
    if (!destLock.dest) {
      UnLockVideoSurfaceBuffer(hSrcVSurface);
      DebugMsg(TOPIC_VIDEOSURFACE, DBG_NORMAL, "Failed on lock of 8BPP dest surface for blitting");
      return (FALSE);
    }

    Blt8BPPTo8BPP(destLock.dest, destLock.pitch, srcLock.dest, srcLock.pitch, iDestX, iDestY,
                  SrcRect.left, SrcRect.top, uiWidth, uiHeight);
    UnLockVideoSurfaceBuffer(hSrcVSurface);
    UnLockVideoSurfaceBuffer(hDestVSurface);
    return (TRUE);
  } else {
    DebugMsg(TOPIC_VIDEOSURFACE, DBG_NORMAL,
             String("Incompatible BPP values with src and dest Video Surfaces for blitting"));
    return (FALSE);
  }

  return (TRUE);
}

BOOLEAN Blt16BPPBufferShadowRectAlternateTable(UINT16 *pBuffer, UINT32 uiDestPitchBYTES,
                                               SGPRect *area);

BOOLEAN InternalShadowVideoSurfaceRect(VSurfID destSurface, INT32 X1, INT32 Y1, INT32 X2, INT32 Y2,
                                       BOOLEAN fLowPercentShadeTable) {
  UINT16 *pBuffer;
  UINT32 uiPitch;
  SGPRect area;
  struct VSurface *hVSurface;

  // CLIP IT!
  // FIRST GET SURFACE

  //
  // Get Video Surface
  //
  if (!(GetVideoSurface(&hVSurface, destSurface))) {
    return FALSE;
  }

  if (X1 < 0) X1 = 0;

  if (X2 < 0) return (FALSE);

  if (Y2 < 0) return (FALSE);

  if (Y1 < 0) Y1 = 0;

  if (X2 >= hVSurface->usWidth) X2 = hVSurface->usWidth - 1;

  if (Y2 >= hVSurface->usHeight) Y2 = hVSurface->usHeight - 1;

  if (X1 >= hVSurface->usWidth) return (FALSE);

  if (Y1 >= hVSurface->usHeight) return (FALSE);

  if ((X2 - X1) <= 0) return (FALSE);

  if ((Y2 - Y1) <= 0) return (FALSE);

  area.iTop = Y1;
  area.iBottom = Y2;
  area.iLeft = X1;
  area.iRight = X2;

  // Lock video surface
  pBuffer = (UINT16 *)LockVideoSurface(destSurface, &uiPitch);
  // UnLockVideoSurface( destSurface );

  if (pBuffer == NULL) {
    return (FALSE);
  }

  if (!fLowPercentShadeTable) {
    // Now we have the video object and surface, call the shadow function
    if (!Blt16BPPBufferShadowRect(pBuffer, uiPitch, &area)) {
      // Blit has failed if false returned
      return (FALSE);
    }
  } else {
    // Now we have the video object and surface, call the shadow function
    if (!Blt16BPPBufferShadowRectAlternateTable(pBuffer, uiPitch, &area)) {
      // Blit has failed if false returned
      return (FALSE);
    }
  }

  // Mark as dirty if it's the backbuffer
  // if ( destSurface == BACKBUFFER )
  //{
  //	InvalidateBackbuffer( );
  //}

  UnLockVideoSurface(destSurface);
  return (TRUE);
}

BOOLEAN ShadowVideoSurfaceRect(VSurfID destSurface, INT32 X1, INT32 Y1, INT32 X2, INT32 Y2) {
  return (InternalShadowVideoSurfaceRect(destSurface, X1, Y1, X2, Y2, FALSE));
}

BOOLEAN ShadowVideoSurfaceRectUsingLowPercentTable(VSurfID destSurface, INT32 X1, INT32 Y1,
                                                   INT32 X2, INT32 Y2) {
  return (InternalShadowVideoSurfaceRect(destSurface, X1, Y1, X2, Y2, TRUE));
}

// This function will stretch the source image to the size of the dest rect.
// If the 2 images are not 16 Bpp, it returns false.
BOOLEAN BltStretchVideoSurface(VSurfID destSurface, VSurfID srcSurface, INT32 iDestX, INT32 iDestY,
                               UINT32 fBltFlags, SGPRect *SrcRect, SGPRect *DestRect) {
  struct VSurface *dest;
  struct VSurface *src;

  if (!GetVideoSurface(&dest, destSurface)) {
    return FALSE;
  }
  if (!GetVideoSurface(&src, srcSurface)) {
    return FALSE;
  }

  // if the 2 images are not both 16bpp, return FALSE
  if ((dest->ubBitDepth != 16) && (src->ubBitDepth != 16)) return (FALSE);

  struct Rect srcRect = {SrcRect->iLeft, SrcRect->iTop, SrcRect->iRight, SrcRect->iBottom};
  struct Rect destRect = {DestRect->iLeft, DestRect->iTop, DestRect->iRight, DestRect->iBottom};
  if (!BltVSurfaceUsingDDBlt(dest, src, fBltFlags, iDestX, iDestY, &srcRect, &destRect)) {
    return (FALSE);
  }

  return (TRUE);
}

BOOLEAN ShadowVideoSurfaceImage(VSurfID destSurface, struct VObject *hImageHandle, INT32 iPosX,
                                INT32 iPosY) {
  // Horizontal shadow
  ShadowVideoSurfaceRect(destSurface, iPosX + 3, iPosY + hImageHandle->pETRLEObject->usHeight,
                         iPosX + hImageHandle->pETRLEObject->usWidth,
                         iPosY + hImageHandle->pETRLEObject->usHeight + 3);

  // vertical shadow
  ShadowVideoSurfaceRect(destSurface, iPosX + hImageHandle->pETRLEObject->usWidth, iPosY + 3,
                         iPosX + hImageHandle->pETRLEObject->usWidth + 3,
                         iPosY + hImageHandle->pETRLEObject->usHeight);
  return (TRUE);
}

BOOLEAN ImageFillVideoSurfaceArea(VSurfID destSurface, INT32 iDestX1, INT32 iDestY1, INT32 iDestX2,
                                  INT32 iDestY2, struct VObject *BkgrndImg, UINT16 Index, INT16 Ox,
                                  INT16 Oy) {
  INT16 xc, yc, hblits, wblits, aw, pw, ah, ph, w, h, xo, yo;
  ETRLEObject *pTrav;
  SGPRect NewClip, OldClip;

  pTrav = &(BkgrndImg->pETRLEObject[Index]);
  ph = (INT16)(pTrav->usHeight + pTrav->sOffsetY);
  pw = (INT16)(pTrav->usWidth + pTrav->sOffsetX);

  ah = (INT16)(iDestY2 - iDestY1);
  aw = (INT16)(iDestX2 - iDestX1);

  Ox %= pw;
  Oy %= ph;

  if (Ox > 0) Ox -= pw;
  xo = (-Ox) % pw;

  if (Oy > 0) Oy -= ph;
  yo = (-Oy) % ph;

  if (Ox < 0)
    xo = (-Ox) % pw;
  else {
    xo = pw - (Ox % pw);
    Ox -= pw;
  }

  if (Oy < 0)
    yo = (-Oy) % ph;
  else {
    yo = ph - (Oy % pw);
    Oy -= ph;
  }

  hblits = ((ah + yo) / ph) + (((ah + yo) % ph) ? 1 : 0);
  wblits = ((aw + xo) / pw) + (((aw + xo) % pw) ? 1 : 0);

  if ((hblits == 0) || (wblits == 0)) return (FALSE);

  //
  // Clip fill region coords
  //

  GetClippingRect(&OldClip);

  NewClip.iLeft = iDestX1;
  NewClip.iTop = iDestY1;
  NewClip.iRight = iDestX2;
  NewClip.iBottom = iDestY2;

  if (NewClip.iLeft < OldClip.iLeft) NewClip.iLeft = OldClip.iLeft;

  if (NewClip.iLeft > OldClip.iRight) return (FALSE);

  if (NewClip.iRight > OldClip.iRight) NewClip.iRight = OldClip.iRight;

  if (NewClip.iRight < OldClip.iLeft) return (FALSE);

  if (NewClip.iTop < OldClip.iTop) NewClip.iTop = OldClip.iTop;

  if (NewClip.iTop > OldClip.iBottom) return (FALSE);

  if (NewClip.iBottom > OldClip.iBottom) NewClip.iBottom = OldClip.iBottom;

  if (NewClip.iBottom < OldClip.iTop) return (FALSE);

  if ((NewClip.iRight <= NewClip.iLeft) || (NewClip.iBottom <= NewClip.iTop)) return (FALSE);

  SetClippingRect(&NewClip);

  yc = (INT16)iDestY1;
  for (h = 0; h < hblits; h++) {
    xc = (INT16)iDestX1;
    for (w = 0; w < wblits; w++) {
      BltVideoObject(destSurface, BkgrndImg, Index, xc + Ox, yc + Oy, VO_BLT_SRCTRANSPARENCY, NULL);
      xc += pw;
    }
    yc += ph;
  }

  SetClippingRect(&OldClip);
  return (TRUE);
}

BOOLEAN ColorFillVideoSurfaceArea(VSurfID destSurface, INT32 iDestX1, INT32 iDestY1, INT32 iDestX2,
                                  INT32 iDestY2, UINT16 Color16BPP) {
  struct BltOpts BltFx;
  struct VSurface *hDestVSurface;
  SGPRect Clip;

  if (!GetVideoSurface(&hDestVSurface, destSurface)) {
    return FALSE;
  }

  BltFx.ColorFill = Color16BPP;

  //
  // Clip fill region coords
  //

  GetClippingRect(&Clip);

  if (iDestX1 < Clip.iLeft) iDestX1 = Clip.iLeft;

  if (iDestX1 > Clip.iRight) return (FALSE);

  if (iDestX2 > Clip.iRight) iDestX2 = Clip.iRight;

  if (iDestX2 < Clip.iLeft) return (FALSE);

  if (iDestY1 < Clip.iTop) iDestY1 = Clip.iTop;

  if (iDestY1 > Clip.iBottom) return (FALSE);

  if (iDestY2 > Clip.iBottom) iDestY2 = Clip.iBottom;

  if (iDestY2 < Clip.iTop) return (FALSE);

  if ((iDestX2 <= iDestX1) || (iDestY2 <= iDestY1)) return (FALSE);

  BltFx.SrcRect.iLeft = BltFx.FillRect.iLeft = iDestX1;
  BltFx.SrcRect.iTop = BltFx.FillRect.iTop = iDestY1;
  BltFx.SrcRect.iRight = BltFx.FillRect.iRight = iDestX2;
  BltFx.SrcRect.iBottom = BltFx.FillRect.iBottom = iDestY2;

  return (FillSurfaceRect(hDestVSurface, &BltFx));
}

BOOLEAN AddVideoSurface(VSURFACE_DESC *pVSurfaceDesc, VSurfID *puiIndex) {
  struct VSurface *hVSurface;

  // Assertions
  Assert(puiIndex);
  Assert(pVSurfaceDesc);

  // Create video object
  hVSurface = CreateVideoSurface(pVSurfaceDesc);

  if (!hVSurface) {
    // Video Object will set error condition.
    return FALSE;
  }

  // Set transparency to default
  SetVideoSurfaceTransparencyColor(hVSurface, FROMRGB(0, 0, 0));

  // Set into video object list
  if (gpVSurfaceHead) {  // Add node after tail
    gpVSurfaceTail->next = (VSURFACE_NODE *)MemAlloc(sizeof(VSURFACE_NODE));
    Assert(gpVSurfaceTail->next);  // out of memory?
    gpVSurfaceTail->next->prev = gpVSurfaceTail;
    gpVSurfaceTail->next->next = NULL;
    gpVSurfaceTail = gpVSurfaceTail->next;
  } else {  // new list
    gpVSurfaceHead = (VSURFACE_NODE *)MemAlloc(sizeof(VSURFACE_NODE));
    Assert(gpVSurfaceHead);  // out of memory?
    gpVSurfaceHead->prev = gpVSurfaceHead->next = NULL;
    gpVSurfaceTail = gpVSurfaceHead;
  }
  // Set the hVSurface into the node.
  gpVSurfaceTail->hVSurface = hVSurface;
  gpVSurfaceTail->uiIndex = guiVSurfaceIndex += 2;
  *puiIndex = gpVSurfaceTail->uiIndex;
  Assert(guiVSurfaceIndex < 0xfffffff0);  // unlikely that we will ever use 2 billion VSurfaces!
  // We would have to create about 70 VSurfaces per second for 1 year straight to achieve this...
  guiVSurfaceSize++;
  guiVSurfaceTotalAdded++;

  return TRUE;
}

// TODO
BYTE *LockVideoSurface(VSurfID uiVSurface, UINT32 *puiPitch) {
  if (uiVSurface == BACKBUFFER) {
    struct BufferLockInfo res = VSurfaceLock(vsBackBuffer);
    *puiPitch = res.pitch;
    return res.dest;
  }

  if (uiVSurface == FRAME_BUFFER) {
    struct BufferLockInfo res = VSurfaceLock(vsFrameBuffer);
    *puiPitch = res.pitch;
    return res.dest;
  }

  if (uiVSurface == MOUSE_BUFFER) {
    struct BufferLockInfo res = VSurfaceLock(vsMouseCursorOriginal);
    *puiPitch = res.pitch;
    return res.dest;
  }

  //
  // Otherwise, use list
  //

  VSURFACE_NODE *curr = gpVSurfaceHead;
  while (curr) {
    if (curr->uiIndex == uiVSurface) {
      break;
    }
    curr = curr->next;
  }
  if (!curr) {
    return FALSE;
  }

  //
  // Lock buffer
  //

  struct BufferLockInfo res = VSurfaceLock(curr->hVSurface);
  *puiPitch = res.pitch;
  return res.dest;
}

void UnLockVideoSurface(VSurfID uiVSurface) {
  VSURFACE_NODE *curr;

  if (uiVSurface == BACKBUFFER) {
    VSurfaceUnlock(vsBackBuffer);
    return;
  }

  if (uiVSurface == FRAME_BUFFER) {
    VSurfaceUnlock(vsFrameBuffer);
    return;
  }

  if (uiVSurface == MOUSE_BUFFER) {
    VSurfaceUnlock(vsMouseCursorOriginal);
    return;
  }

  curr = gpVSurfaceHead;
  while (curr) {
    if (curr->uiIndex == uiVSurface) {
      break;
    }
    curr = curr->next;
  }
  if (!curr) {
    return;
  }

  //
  // unlock buffer
  //

  UnLockVideoSurfaceBuffer(curr->hVSurface);
}

BOOLEAN SetVideoSurfaceTransparency(UINT32 uiIndex, COLORVAL TransColor) {
  struct VSurface *hVSurface;

  //
  // Get Video Surface
  //

  if (!(GetVideoSurface(&hVSurface, uiIndex))) {
    return FALSE;
  }

  //
  // Set transparency
  //

  SetVideoSurfaceTransparencyColor(hVSurface, TransColor);

  return (TRUE);
}

BOOLEAN GetVideoSurface(struct VSurface **hVSurface, VSurfID uiIndex) {
  VSURFACE_NODE *curr;

  if (uiIndex == BACKBUFFER) {
    *hVSurface = vsBackBuffer;
    return TRUE;
  }

  if (uiIndex == FRAME_BUFFER) {
    *hVSurface = vsFrameBuffer;
    return TRUE;
  }

  if (uiIndex == MOUSE_BUFFER) {
    *hVSurface = vsMouseCursor;
    return TRUE;
  }

  curr = gpVSurfaceHead;
  while (curr) {
    if (curr->uiIndex == uiIndex) {
      *hVSurface = curr->hVSurface;
      return TRUE;
    }
    curr = curr->next;
  }
  return FALSE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// Given an index to the dest and src vobject contained in our private VSurface list
// Based on flags, blit accordingly
// There are two types, a BltFast and a Blt. BltFast is 10% faster, uses no
// clipping lists
//
///////////////////////////////////////////////////////////////////////////////////////////////////

BOOLEAN BltVideoSurface(VSurfID destSurface, VSurfID srcSurface, INT32 iDestX, INT32 iDestY,
                        UINT32 fBltFlags, struct BltOpts *pBltFx) {
  struct VSurface *hDestVSurface;
  struct VSurface *hSrcVSurface;

  if (!GetVideoSurface(&hDestVSurface, destSurface)) {
    return FALSE;
  }
  if (!GetVideoSurface(&hSrcVSurface, srcSurface)) {
    return FALSE;
  }
  if (!BltVideoSurfaceToVideoSurface(hDestVSurface, hSrcVSurface, iDestX, iDestY, fBltFlags,
                                     pBltFx)) {
    // VO Blitter will set debug messages for error conditions
    return FALSE;
  }
  return TRUE;
}

BOOLEAN RestoreVideoSurfaces() {
  VSURFACE_NODE *curr;

  //
  // Loop through Video Surfaces and Restore
  //
  curr = gpVSurfaceTail;
  while (curr) {
    if (!RestoreVideoSurface(curr->hVSurface)) {
      return FALSE;
    }
    curr = curr->prev;
  }
  return TRUE;
}

BOOLEAN ShutdownVideoSurfaceManager() {
  VSURFACE_NODE *curr;

  DebugMsg(TOPIC_VIDEOSURFACE, DBG_ERROR, "Shutting down the Video Surface manager");

  while (gpVSurfaceHead) {
    curr = gpVSurfaceHead;
    gpVSurfaceHead = gpVSurfaceHead->next;
    DeleteVideoSurface(curr->hVSurface);
    MemFree(curr);
  }
  gpVSurfaceHead = NULL;
  gpVSurfaceTail = NULL;
  guiVSurfaceIndex = 0;
  guiVSurfaceSize = 0;
  guiVSurfaceTotalAdded = 0;
  return TRUE;
}

BOOLEAN InitializeVideoSurfaceManager() {
  // Shouldn't be calling this if the video surface manager already exists.
  // Call shutdown first...
  Assert(!gpVSurfaceHead);
  Assert(!gpVSurfaceTail);
  gpVSurfaceHead = gpVSurfaceTail = NULL;

  giMemUsedInSurfaces = 0;

  return TRUE;
}

void InvalidateRegion(INT32 iLeft, INT32 iTop, INT32 iRight, INT32 iBottom) {
  if (gfForceFullScreenRefresh == TRUE) {
    //
    // There's no point in going on since we are forcing a full screen refresh
    //

    return;
  }

  if (guiDirtyRegionCount < MAX_DIRTY_REGIONS) {
    //
    // Well we haven't broken the MAX_DIRTY_REGIONS limit yet, so we register the new region
    //

    // DO SOME PREMIMARY CHECKS FOR VALID RECTS
    if (iLeft < 0) iLeft = 0;

    if (iTop < 0) iTop = 0;

    if (iRight > SCREEN_WIDTH) iRight = SCREEN_WIDTH;

    if (iBottom > SCREEN_HEIGHT) iBottom = SCREEN_HEIGHT;

    if ((iRight - iLeft) <= 0) return;

    if ((iBottom - iTop) <= 0) return;

    gListOfDirtyRegions[guiDirtyRegionCount].iLeft = iLeft;
    gListOfDirtyRegions[guiDirtyRegionCount].iTop = iTop;
    gListOfDirtyRegions[guiDirtyRegionCount].iRight = iRight;
    gListOfDirtyRegions[guiDirtyRegionCount].iBottom = iBottom;

    //		gDirtyRegionFlags[ guiDirtyRegionCount ] = TRUE;

    guiDirtyRegionCount++;

  } else {
    //
    // The MAX_DIRTY_REGIONS limit has been exceeded. Therefore we arbitrarely invalidate the entire
    // screen and force a full screen refresh
    //
    guiDirtyRegionExCount = 0;
    guiDirtyRegionCount = 0;
    gfForceFullScreenRefresh = TRUE;
  }
}

static void AddRegionEx(INT32 iLeft, INT32 iTop, INT32 iRight, INT32 iBottom, UINT32 uiFlags);

void InvalidateRegionEx(INT32 iLeft, INT32 iTop, INT32 iRight, INT32 iBottom, UINT32 uiFlags) {
  INT32 iOldBottom;

  iOldBottom = iBottom;

  // Check if we are spanning the rectangle - if so slit it up!
  if (iTop <= gsVIEWPORT_WINDOW_END_Y && iBottom > gsVIEWPORT_WINDOW_END_Y) {
    // Add new top region
    iBottom = gsVIEWPORT_WINDOW_END_Y;
    AddRegionEx(iLeft, iTop, iRight, iBottom, uiFlags);

    // Add new bottom region
    iTop = gsVIEWPORT_WINDOW_END_Y;
    iBottom = iOldBottom;
    AddRegionEx(iLeft, iTop, iRight, iBottom, uiFlags);

  } else {
    AddRegionEx(iLeft, iTop, iRight, iBottom, uiFlags);
  }
}

static void AddRegionEx(INT32 iLeft, INT32 iTop, INT32 iRight, INT32 iBottom, UINT32 uiFlags) {
  if (guiDirtyRegionExCount < MAX_DIRTY_REGIONS) {
    // DO SOME PREMIMARY CHECKS FOR VALID RECTS
    if (iLeft < 0) iLeft = 0;

    if (iTop < 0) iTop = 0;

    if (iRight > SCREEN_WIDTH) iRight = SCREEN_WIDTH;

    if (iBottom > SCREEN_HEIGHT) iBottom = SCREEN_HEIGHT;

    if ((iRight - iLeft) <= 0) return;

    if ((iBottom - iTop) <= 0) return;

    gDirtyRegionsEx[guiDirtyRegionExCount].iLeft = iLeft;
    gDirtyRegionsEx[guiDirtyRegionExCount].iTop = iTop;
    gDirtyRegionsEx[guiDirtyRegionExCount].iRight = iRight;
    gDirtyRegionsEx[guiDirtyRegionExCount].iBottom = iBottom;

    gDirtyRegionsFlagsEx[guiDirtyRegionExCount] = uiFlags;

    guiDirtyRegionExCount++;

  } else {
    guiDirtyRegionExCount = 0;
    guiDirtyRegionCount = 0;
    gfForceFullScreenRefresh = TRUE;
  }
}

void InvalidateScreen(void) {
  //
  // W A R N I N G ---- W A R N I N G ---- W A R N I N G ---- W A R N I N G ---- W A R N I N G ----
  //
  // This function is intended to be called by a thread which has already locked the
  // FRAME_BUFFER_MUTEX mutual exclusion section. Anything else will cause the application to
  // yack
  //

  guiDirtyRegionCount = 0;
  guiDirtyRegionExCount = 0;
  gfForceFullScreenRefresh = TRUE;
  guiFrameBufferState = BUFFER_DIRTY;
}

UINT16 GetVSurfaceHeight(const struct VSurface *vs) { return vs->usHeight; }
UINT16 GetVSurfaceWidth(const struct VSurface *vs) { return vs->usWidth; }
UINT16 *GetVSurface16BPPPalette(struct VSurface *vs) { return vs->p16BPPPalette; }
void SetVSurface16BPPPalette(struct VSurface *vs, UINT16 *palette) { vs->p16BPPPalette = palette; }

struct VSurface *VSurfaceNew() { return zmalloc(sizeof(struct VSurface)); }
// void VSurfaceErase(struct VSurface *vs) {
//   // TODO
// }
