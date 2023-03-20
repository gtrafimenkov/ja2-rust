#include "VSurface.h"

#include "SGP/Debug.h"
#include "SGP/Types.h"
#include "SGP/VSurfaceInternal.h"
#include "platform.h"

// Given an struct Image* object, blit imagery into existing Video Surface. Can be from 8->16
// BPP
BOOLEAN SetVideoSurfaceDataFromHImage(struct VSurface *hVSurface, struct Image *hImage, UINT16 usX,
                                      UINT16 usY, SGPRect *pSrcRect) {
  BYTE *pDest;
  UINT32 fBufferBPP = 0;
  UINT32 uiPitch;
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
  pDest = LockVideoSurfaceBuffer(hVSurface, &uiPitch);

  // Effective width ( in PIXELS ) is Pitch ( in bytes ) converted to pitch ( IN PIXELS )
  usEffectiveWidth = (UINT16)(uiPitch / (hVSurface->ubBitDepth / 8));

  if (!(pDest != NULL)) {
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
  if (!CopyImageToBuffer(hImage, fBufferBPP, pDest, usEffectiveWidth, hVSurface->usHeight, usX, usY,
                         &aRect)) {
    DebugMsg(TOPIC_VIDEOSURFACE, DBG_NORMAL,
             String("Error Occured Copying struct Image* to struct VSurface*"));
    UnLockVideoSurfaceBuffer(hVSurface);
    return (FALSE);
  }

  // All is OK
  UnLockVideoSurfaceBuffer(hVSurface);

  return (TRUE);
}
