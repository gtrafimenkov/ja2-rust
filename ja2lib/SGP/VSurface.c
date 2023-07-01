#include "VSurface.h"

#include "Local.h"
#include "Rect.h"
#include "SGP/Debug.h"
#include "SGP/MemMan.h"
#include "SGP/Types.h"
#include "SGP/VObjectBlitters.h"
#include "SGP/VObjectInternal.h"
#include "SGP/VSurfaceInternal.h"
#include "SGP/Video.h"
#include "SGP/VideoInternal.h"
#include "SGP/WCheck.h"
#include "TileEngine/RenderWorld.h"
#include "platform.h"

struct VSurface *vsPrimary = NULL;
struct VSurface *vsBB = NULL;
struct VSurface *vsFB = NULL;
struct VSurface *vsMouseCursor = NULL;
struct VSurface *vsMouseBuffer = NULL;

//
// Dirty rectangle management variables
//

SGPRect gListOfDirtyRegions[MAX_DIRTY_REGIONS];
uint32_t guiDirtyRegionCount;
BOOLEAN gfForceFullScreenRefresh;

SGPRect gDirtyRegionsEx[MAX_DIRTY_REGIONS];
uint32_t gDirtyRegionsFlagsEx[MAX_DIRTY_REGIONS];
uint32_t guiDirtyRegionExCount;

typedef struct VSURFACE_NODE {
  struct VSurface *hVSurface;
  uint32_t uiIndex;
  struct VSURFACE_NODE *next, *prev;
} VSURFACE_NODE;

VSURFACE_NODE *gpVSurfaceHead = NULL;
VSURFACE_NODE *gpVSurfaceTail = NULL;
uint32_t guiVSurfaceIndex = 0;
uint32_t guiVSurfaceSize = 0;
uint32_t guiVSurfaceTotalAdded = 0;

// Given an struct Image* object, blit imagery into existing Video Surface. Can be from 8->16
// BPP
BOOLEAN SetVideoSurfaceDataFromHImage(struct VSurface *hVSurface, struct Image *hImage,
                                      uint16_t usX, uint16_t usY, SGPRect *pSrcRect) {
  uint32_t fBufferBPP = 0;
  uint16_t usEffectiveWidth;
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
  usEffectiveWidth = (uint16_t)(lock.pitch / (hVSurface->ubBitDepth / 8));

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
    VSurfaceUnlock(hVSurface);
    return (FALSE);
  }

  // All is OK
  VSurfaceUnlock(hVSurface);

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

BOOLEAN BltVideoSurface(struct VSurface *dest, struct VSurface *src, int32_t iDestX, int32_t iDestY,
                        int32_t fBltFlags, struct BltOpts *pBltFx) {
  if (!dest || !src) {
    return FALSE;
  }
  struct Rect SrcRect, DestRect;
  uint32_t uiWidth, uiHeight;

  // Assertions
  Assert(dest != NULL);

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
    if (dest->usHeight < src->usHeight) {
      DebugMsg(TOPIC_VIDEOSURFACE, DBG_NORMAL,
               "Incompatible height size given in Video Surface blit");
      return (FALSE);
    }
    if (dest->usWidth < src->usWidth) {
      DebugMsg(TOPIC_VIDEOSURFACE, DBG_NORMAL,
               "Incompatible height size given in Video Surface blit");
      return (FALSE);
    }

    SrcRect.top = (int)0;
    SrcRect.left = (int)0;
    SrcRect.bottom = (int)src->usHeight;
    SrcRect.right = (int)src->usWidth;

  } while (FALSE);

  // Once here, assert valid Src
  Assert(src != NULL);

  // clipping -- added by DB
  GetVSurfaceRect(dest, &DestRect);
  uiWidth = SrcRect.right - SrcRect.left;
  uiHeight = SrcRect.bottom - SrcRect.top;

  // check for position entirely off the screen
  if (iDestX >= DestRect.right) return (FALSE);
  if (iDestY >= DestRect.bottom) return (FALSE);
  if ((iDestX + (int32_t)uiWidth) < (int32_t)DestRect.left) return (FALSE);
  if ((iDestY + (int32_t)uiHeight) < (int32_t)DestRect.top) return (FALSE);

  {
    if ((iDestX + (int32_t)uiWidth) >= (int32_t)DestRect.right) {
      SrcRect.right -= ((iDestX + uiWidth) - DestRect.right);
      uiWidth -= ((iDestX + uiWidth) - DestRect.right);
    }
    if ((iDestY + (int32_t)uiHeight) >= (int32_t)DestRect.bottom) {
      SrcRect.bottom -= ((iDestY + uiHeight) - DestRect.bottom);
      uiHeight -= ((iDestY + uiHeight) - DestRect.bottom);
    }
    if (iDestX < DestRect.left) {
      SrcRect.left += (DestRect.left - iDestX);
      uiWidth -= (DestRect.left - iDestX);
      iDestX = DestRect.left;
    }
    if (iDestY < (int32_t)DestRect.top) {
      SrcRect.top += (DestRect.top - iDestY);
      uiHeight -= (DestRect.top - iDestY);
      iDestY = DestRect.top;
    }
  }

  // Send dest position, rectangle, etc to DD bltfast function
  // First check BPP values for compatibility
  if (dest->ubBitDepth == 16 && src->ubBitDepth == 16) {
    struct Rect srcRect = {SrcRect.left, SrcRect.top, SrcRect.right, SrcRect.bottom};
    if (!(BltVSurfaceUsingDD(dest, src, fBltFlags, iDestX, iDestY, &srcRect))) {
      return FALSE;
    }

  } else if (dest->ubBitDepth == 8 && src->ubBitDepth == 8) {
    struct BufferLockInfo srcLock = VSurfaceLock(src);
    if (!srcLock.dest) {
      DebugMsg(TOPIC_VIDEOSURFACE, DBG_NORMAL, "Failed on lock of 8BPP surface for blitting");
      return (FALSE);
    }

    struct BufferLockInfo destLock = VSurfaceLock(dest);
    if (!destLock.dest) {
      VSurfaceUnlock(src);
      DebugMsg(TOPIC_VIDEOSURFACE, DBG_NORMAL, "Failed on lock of 8BPP dest surface for blitting");
      return (FALSE);
    }

    Blt8BPPTo8BPP(destLock.dest, destLock.pitch, srcLock.dest, srcLock.pitch, iDestX, iDestY,
                  SrcRect.left, SrcRect.top, uiWidth, uiHeight);
    VSurfaceUnlock(src);
    VSurfaceUnlock(dest);
    return (TRUE);
  } else {
    DebugMsg(TOPIC_VIDEOSURFACE, DBG_NORMAL,
             String("Incompatible BPP values with src and dest Video Surfaces for blitting"));
    return (FALSE);
  }

  return (TRUE);
}

BOOLEAN Blt16BPPBufferShadowRectAlternateTable(uint16_t *pBuffer, uint32_t uiDestPitchBYTES,
                                               SGPRect *area);

BOOLEAN InternalShadowVideoSurfaceRect(struct VSurface *dest, int32_t X1, int32_t Y1, int32_t X2,
                                       int32_t Y2, BOOLEAN fLowPercentShadeTable) {
  if (!dest) {
    return FALSE;
  }

  uint16_t *pBuffer;
  uint32_t uiPitch;
  SGPRect area;

  if (X1 < 0) X1 = 0;

  if (X2 < 0) return (FALSE);

  if (Y2 < 0) return (FALSE);

  if (Y1 < 0) Y1 = 0;

  if (X2 >= dest->usWidth) X2 = dest->usWidth - 1;

  if (Y2 >= dest->usHeight) Y2 = dest->usHeight - 1;

  if (X1 >= dest->usWidth) return (FALSE);

  if (Y1 >= dest->usHeight) return (FALSE);

  if ((X2 - X1) <= 0) return (FALSE);

  if ((Y2 - Y1) <= 0) return (FALSE);

  area.iTop = Y1;
  area.iBottom = Y2;
  area.iLeft = X1;
  area.iRight = X2;

  // Lock video surface
  pBuffer = (uint16_t *)VSurfaceLockOld(dest, &uiPitch);

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

  VSurfaceUnlock(dest);
  return (TRUE);
}

BOOLEAN ShadowVideoSurfaceRect(struct VSurface *dest, int32_t X1, int32_t Y1, int32_t X2,
                               int32_t Y2) {
  return (InternalShadowVideoSurfaceRect(dest, X1, Y1, X2, Y2, FALSE));
}

BOOLEAN ShadowVideoSurfaceRectUsingLowPercentTable(struct VSurface *dest, int32_t X1, int32_t Y1,
                                                   int32_t X2, int32_t Y2) {
  return (InternalShadowVideoSurfaceRect(dest, X1, Y1, X2, Y2, TRUE));
}

// This function will stretch the source image to the size of the dest rect.
// If the 2 images are not 16 Bpp, it returns false.
BOOLEAN BltStretchVideoSurface(struct VSurface *dest, struct VSurface *src, int32_t iDestX,
                               int32_t iDestY, uint32_t fBltFlags, SGPRect *SrcRect,
                               SGPRect *DestRect) {
  if (!dest || !src) {
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

BOOLEAN VSurfaceColorFill(struct VSurface *dest, int32_t x1, int32_t y1, int32_t x2, int32_t y2,
                          uint16_t Color16BPP) {
  SGPRect Clip;
  GetClippingRect(&Clip);
  if (x1 < Clip.iLeft) x1 = Clip.iLeft;
  if (x1 > Clip.iRight) return (FALSE);
  if (x2 > Clip.iRight) x2 = Clip.iRight;
  if (x2 < Clip.iLeft) return (FALSE);
  if (y1 < Clip.iTop) y1 = Clip.iTop;
  if (y1 > Clip.iBottom) return (FALSE);
  if (y2 > Clip.iBottom) y2 = Clip.iBottom;
  if (y2 < Clip.iTop) return (FALSE);
  if ((x2 <= x1) || (y2 <= y1)) return (FALSE);

  struct BltOpts opts;
  opts.ColorFill = Color16BPP;
  opts.SrcRect.iLeft = opts.FillRect.iLeft = x1;
  opts.SrcRect.iTop = opts.FillRect.iTop = y1;
  opts.SrcRect.iRight = opts.FillRect.iRight = x2;
  opts.SrcRect.iBottom = opts.FillRect.iBottom = y2;

  return FillSurfaceRect(dest, &opts);
}

static uint32_t addVSurfaceToList(struct VSurface *vs) {
  // Set into video object list
  if (gpVSurfaceHead) {
    // Add node after tail
    gpVSurfaceTail->next = (VSURFACE_NODE *)MemAlloc(sizeof(VSURFACE_NODE));
    Assert(gpVSurfaceTail->next);  // out of memory?
    gpVSurfaceTail->next->prev = gpVSurfaceTail;
    gpVSurfaceTail->next->next = NULL;
    gpVSurfaceTail = gpVSurfaceTail->next;
  } else {
    // new list
    gpVSurfaceHead = (VSURFACE_NODE *)MemAlloc(sizeof(VSURFACE_NODE));
    Assert(gpVSurfaceHead);  // out of memory?
    gpVSurfaceHead->prev = gpVSurfaceHead->next = NULL;
    gpVSurfaceTail = gpVSurfaceHead;
  }
  // Set the hVSurface into the node.
  gpVSurfaceTail->hVSurface = vs;
  gpVSurfaceTail->uiIndex = guiVSurfaceIndex += 2;
  Assert(guiVSurfaceIndex < 0xfffffff0);  // unlikely that we will ever use 2 billion VSurfaces!
  // We would have to create about 70 VSurfaces per second for 1 year straight to achieve this...
  guiVSurfaceSize++;
  guiVSurfaceTotalAdded++;
  return gpVSurfaceTail->uiIndex;
}

struct VSurface *VSurfaceAdd(uint16_t width, uint16_t height, VSurfID *puiIndex) {
  struct VSurface *vs = CreateVideoSurface(width, height, 16);
  if (vs) {
    SetVideoSurfaceTransparencyColor(vs, FROMRGB(0, 0, 0));
    if (puiIndex) {
      *puiIndex = addVSurfaceToList(vs);
    }
    return vs;
  }
  return NULL;
}

BOOLEAN AddVideoSurfaceFromFile(const char *fileName, VSurfID *puiIndex) {
  Assert(puiIndex);
  Assert(fileName);

  struct VSurface *vs = CreateVideoSurfaceFromFile(fileName);

  if (!vs) {
    return FALSE;
  }

  SetVideoSurfaceTransparencyColor(vs, FROMRGB(0, 0, 0));
  *puiIndex = addVSurfaceToList(vs);
  return TRUE;
}

BOOLEAN AddVideoSurface(VSURFACE_DESC *desc, VSurfID *puiIndex) {
  Assert(puiIndex);
  Assert(desc);

  struct VSurface *vs = CreateVideoSurface(desc->usWidth, desc->usHeight, 16);

  if (!vs) {
    return FALSE;
  }

  SetVideoSurfaceTransparencyColor(vs, FROMRGB(0, 0, 0));
  *puiIndex = addVSurfaceToList(vs);
  return TRUE;
}

// Old interface to locking VSurface
BYTE *VSurfaceLockOld(struct VSurface *vs, uint32_t *pitch) {
  struct BufferLockInfo res = VSurfaceLock(vs);
  *pitch = res.pitch;
  return res.dest;
}

BOOLEAN SetVideoSurfaceTransparency(uint32_t uiIndex, COLORVAL TransColor) {
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

struct VSurface *GetVSByID(VSurfID id) {
  switch (id) {
    case BACKBUFFER:
      return vsBB;

    case FRAME_BUFFER:
      return vsFB;

    default: {
      VSURFACE_NODE *curr;
      curr = gpVSurfaceHead;
      while (curr) {
        if (curr->uiIndex == id) {
          return curr->hVSurface;
        }
        curr = curr->next;
      }
    }
  }
  return NULL;
}

BOOLEAN GetVideoSurface(struct VSurface **hVSurface, VSurfID uiIndex) {
  struct VSurface *vs = GetVSByID(uiIndex);
  if (vs) {
    *hVSurface = vs;
  }
  return vs != NULL;
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
  return TRUE;
}

void InvalidateRegion(int32_t iLeft, int32_t iTop, int32_t iRight, int32_t iBottom) {
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
    // The MAX_DIRTY_REGIONS limit has been exceeded. Therefore we arbitrarely invalidate the
    // entire screen and force a full screen refresh
    //
    guiDirtyRegionExCount = 0;
    guiDirtyRegionCount = 0;
    gfForceFullScreenRefresh = TRUE;
  }
}

static void AddRegionEx(int32_t iLeft, int32_t iTop, int32_t iRight, int32_t iBottom,
                        uint32_t uiFlags);

void InvalidateRegionEx(int32_t iLeft, int32_t iTop, int32_t iRight, int32_t iBottom,
                        uint32_t uiFlags) {
  int32_t iOldBottom;

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

static void AddRegionEx(int32_t iLeft, int32_t iTop, int32_t iRight, int32_t iBottom,
                        uint32_t uiFlags) {
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
  guiDirtyRegionCount = 0;
  guiDirtyRegionExCount = 0;
  gfForceFullScreenRefresh = TRUE;
  guiFrameBufferState = BUFFER_DIRTY;
}

uint16_t GetVSurfaceHeight(const struct VSurface *vs) { return vs->usHeight; }
uint16_t GetVSurfaceWidth(const struct VSurface *vs) { return vs->usWidth; }
uint16_t *GetVSurface16BPPPalette(struct VSurface *vs) { return vs->p16BPPPalette; }
void SetVSurface16BPPPalette(struct VSurface *vs, uint16_t *palette) {
  vs->p16BPPPalette = palette;
}

struct VSurface *VSurfaceNew() { return zmalloc(sizeof(struct VSurface)); }

struct VSurface *CreateVideoSurfaceFromFile(const char *path) {
  struct Image *image = CreateImage(path, IMAGE_ALLIMAGEDATA);
  if (image == NULL) {
    DebugMsg(TOPIC_VIDEOSURFACE, DBG_NORMAL, "Invalid Image Filename given");
    return (NULL);
  }

  struct VSurface *vs = CreateVideoSurface(image->usWidth, image->usHeight, image->ubBitDepth);

  if (vs) {
    SGPRect tempRect;
    tempRect.iLeft = 0;
    tempRect.iTop = 0;
    tempRect.iRight = image->usWidth - 1;
    tempRect.iBottom = image->usHeight - 1;
    SetVideoSurfaceDataFromHImage(vs, image, 0, 0, &tempRect);
    if (image->ubBitDepth == 8) {
      SetVideoSurfacePalette(vs, image->pPalette);
    }
    DestroyImage(image);
  }

  return vs;
}

uint32_t guiRIGHTPANEL = 0;
uint32_t guiSAVEBUFFER = 0;

struct VSurface *vsExtraBuffer = NULL;
struct VSurface *vsSB = NULL;

BOOLEAN InitializeSystemVideoObjects() { return (TRUE); }

BOOLEAN InitializeGameVideoObjects() {
  uint16_t usWidth;
  uint16_t usHeight;

  GetCurrentVideoSettings(&usWidth, &usHeight);

  vsSB = VSurfaceAdd(usWidth, usHeight, &guiSAVEBUFFER);
  if (!vsSB) {
    return FALSE;
  }

  vsExtraBuffer = VSurfaceAdd(usWidth, usHeight, NULL);
  if (!vsExtraBuffer) {
    return FALSE;
  }

  return (TRUE);
}
