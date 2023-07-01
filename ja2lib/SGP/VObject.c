#include "SGP/VObject.h"

#include <stdio.h>
#include <string.h>

#include "Globals.h"
#include "SGP/Debug.h"
#include "SGP/HImage.h"
#include "SGP/PaletteEntry.h"
#include "SGP/Shading.h"
#include "SGP/Types.h"
#include "SGP/VObjectBlitters.h"
#include "SGP/VObjectInternal.h"
#include "SGP/VSurface.h"
#include "SGP/Video.h"
#include "SGP/WCheck.h"
#include "platform_strings.h"

// ******************************************************************************
//
// Video Object SGP Module
//
// Video Objects are used to contain any imagery which requires blitting. The data
// is contained within a Direct Draw surface. Palette information is in both
// a Direct Draw Palette and a 16BPP palette structure for 8->16 BPP Blits.
// Blitting is done via Direct Draw as well as custum blitters. Regions are
// used to define local coordinates within the surface
//
// Second Revision: Dec 10, 1996, Andrew Emmons
//
// *******************************************************************************

// *******************************************************************************
// Defines
// *******************************************************************************

// This define is sent to CreateList SGP function. It dynamically re-sizes if
// the list gets larger
#define DEFAULT_VIDEO_OBJECT_LIST_SIZE 10

#define COMPRESS_TRANSPARENT 0x80
#define COMPRESS_RUN_MASK 0x7F

// *******************************************************************************
// External Functions and variables
// *******************************************************************************

// *******************************************************************************
// LOCAL functions
// *******************************************************************************

// *******************************************************************************
// LOCAL global variables
// *******************************************************************************

HLIST ghVideoObjects = NULL;
BOOLEAN gfVideoObjectsInit = FALSE;

typedef struct VOBJECT_NODE {
  struct VObject *hVObject;
  uint32_t uiIndex;
  struct VOBJECT_NODE *next, *prev;
} VOBJECT_NODE;

// TODO: rust
VOBJECT_NODE *gpVObjectHead = NULL;
VOBJECT_NODE *gpVObjectTail = NULL;
uint32_t guiVObjectIndex = 1;
uint32_t guiVObjectSize = 0;
uint32_t guiVObjectTotalAdded = 0;

static BOOLEAN BltVideoObjectToBuffer(uint16_t *pBuffer, uint32_t uiDestPitchBYTES,
                                      struct VObject *hSrcVObject, uint16_t usIndex, int32_t iDestX,
                                      int32_t iDestY);

// Sets struct VObject* palette, creates if nessessary. Also sets 16BPP palette
static BOOLEAN SetVideoObjectPalette(struct VObject *hVObject, struct SGPPaletteEntry *pSrcPalette);

// **************************************************************
//
// Video Object Manager functions
//
// **************************************************************

BOOLEAN InitializeVideoObjectManager() {
  // Shouldn't be calling this if the video object manager already exists.
  // Call shutdown first...
  Assert(!gpVObjectHead);
  Assert(!gpVObjectTail);
  gpVObjectHead = gpVObjectTail = NULL;
  gfVideoObjectsInit = TRUE;
  return TRUE;
}

BOOLEAN ShutdownVideoObjectManager() {
  VOBJECT_NODE *curr;
  while (gpVObjectHead) {
    curr = gpVObjectHead;
    gpVObjectHead = gpVObjectHead->next;
    DeleteVideoObject(curr->hVObject);
    MemFree(curr);
  }
  gpVObjectHead = NULL;
  gpVObjectTail = NULL;
  guiVObjectIndex = 1;
  guiVObjectSize = 0;
  guiVObjectTotalAdded = 0;
  gfVideoObjectsInit = FALSE;
  return TRUE;
}

// TODO: rust
uint32_t CountVideoObjectNodes() {
  VOBJECT_NODE *curr;
  uint32_t i = 0;
  curr = gpVObjectHead;
  while (curr) {
    i++;
    curr = curr->next;
  }
  return i;
}

// This structure describes the creation parameters for a Video Object
typedef struct {
  uint32_t fCreateFlags;  // Specifies creation flags like from file or not
  union {
    struct {
      SGPFILENAME ImageFile;  // Filename of image data to use
    };
    struct {
      struct Image *hImage;
    };
  };
} VOBJECT_INFO;
#define VOBJECT_CREATE_FROMFILE \
  0x00000040  // Creates a video object from a file ( using struct Image* )
#define VOBJECT_CREATE_FROMHIMAGE 0x00000080  // Creates a video object from a pre-loaded hImage

BOOLEAN _AddVideoObject(VOBJECT_INFO *pVObjectDesc, uint32_t *puiIndex);

BOOLEAN AddVideoObject(VOBJECT_DESC *desc, uint32_t *puiIndex) {
  VOBJECT_INFO info;
  info.fCreateFlags = VOBJECT_CREATE_FROMFILE;
  strcpy(info.ImageFile, desc->ImageFile);
  return _AddVideoObject(&info, puiIndex);
}

struct VObject *LoadVObjectFromFile(const char *path) {
  struct VObject *vo = CreateVObjectFromFile(path);
  if (vo) {
    vo->TransparentColor = FROMRGB(0, 0, 0);
  }
  return vo;
}

BOOLEAN AddVObjectFromFile(const char *path, uint32_t *puiIndex) {
  VOBJECT_INFO desc;
  desc.fCreateFlags = VOBJECT_CREATE_FROMFILE;
  strcpy(desc.ImageFile, path);
  return _AddVideoObject(&desc, puiIndex);
}

BOOLEAN AddVObjectFromHImage(struct Image *hImage, uint32_t *puiIndex) {
  VOBJECT_INFO desc;
  desc.fCreateFlags = VOBJECT_CREATE_FROMHIMAGE;
  desc.hImage = hImage;
  return _AddVideoObject(&desc, puiIndex);
}

// TODO: rust
// TODO: debug print how many video objects are there
// TODO: probably replace the list with array of fixed size
BOOLEAN _AddVideoObject(VOBJECT_INFO *pVObjectDesc, uint32_t *puiIndex) {
  struct VObject *hVObject;

  // Assertions
  Assert(puiIndex);
  Assert(pVObjectDesc);

  // Create video object
  if (pVObjectDesc->fCreateFlags & VOBJECT_CREATE_FROMFILE) {
    hVObject = CreateVObjectFromFile(pVObjectDesc->ImageFile);
  } else if (pVObjectDesc->fCreateFlags & VOBJECT_CREATE_FROMHIMAGE) {
    hVObject = CreateVObjectFromHImage(pVObjectDesc->hImage);
  } else {
    DebugMsg(TOPIC_VIDEOOBJECT, DBG_NORMAL, "Invalid VObject creation flags given.");
    return FALSE;
  }

  if (!hVObject) {
    // Video Object will set error condition.
    return FALSE;
  }

  // Set transparency to default
  hVObject->TransparentColor = FROMRGB(0, 0, 0);

  // Set into video object list
  if (gpVObjectHead) {  // Add node after tail
    gpVObjectTail->next = (VOBJECT_NODE *)MemAlloc(sizeof(VOBJECT_NODE));
    Assert(gpVObjectTail->next);  // out of memory?
    gpVObjectTail->next->prev = gpVObjectTail;
    gpVObjectTail->next->next = NULL;
    gpVObjectTail = gpVObjectTail->next;
  } else {  // new list
    gpVObjectHead = (VOBJECT_NODE *)MemAlloc(sizeof(VOBJECT_NODE));
    Assert(gpVObjectHead);  // out of memory?
    gpVObjectHead->prev = gpVObjectHead->next = NULL;
    gpVObjectTail = gpVObjectHead;
  }
  // Set the hVObject into the node.
  gpVObjectTail->hVObject = hVObject;
  gpVObjectTail->uiIndex = guiVObjectIndex += 2;
  *puiIndex = gpVObjectTail->uiIndex;
  Assert(guiVObjectIndex < 0xfffffff0);  // unlikely that we will ever use 2 billion vobjects!
  // We would have to create about 70 vobjects per second for 1 year straight to achieve this...
  guiVObjectSize++;
  guiVObjectTotalAdded++;

#ifdef JA2TESTVERSION
  if (CountVideoObjectNodes() != guiVObjectSize) {
    guiVObjectSize = guiVObjectSize;
  }
#endif

  return TRUE;
}

BOOLEAN SetVideoObjectTransparency(uint32_t uiIndex, COLORVAL TransColor) {
  struct VObject *hVObject;

  // Get video object
  if (!(GetVideoObject(&hVObject, uiIndex))) {
    return FALSE;
  }

  // Set transparency
  hVObject->TransparentColor = TransColor;

  return (TRUE);
}

// TODO: rust
BOOLEAN GetVideoObject(struct VObject **hVObject, uint32_t uiIndex) {
  VOBJECT_NODE *curr;

  curr = gpVObjectHead;
  while (curr) {
    if (curr->uiIndex == uiIndex) {
      *hVObject = curr->hVObject;
      return TRUE;
    }
    curr = curr->next;
  }
  return FALSE;
}

bool BltVObjectFromIndex(struct VSurface *dest, uint32_t uiSrcVObject, uint16_t usRegionIndex,
                         int32_t iDestX, int32_t iDestY) {
  uint16_t *pBuffer;
  uint32_t uiPitch;
  struct VObject *hSrcVObject;

  // Lock video surface
  pBuffer = (uint16_t *)VSurfaceLockOld(dest, &uiPitch);

  if (pBuffer == NULL) {
    return (FALSE);
  }

  // Get video object
  if (!GetVideoObject(&hSrcVObject, uiSrcVObject)) {
    VSurfaceUnlock(dest);
    return FALSE;
  }

  // Now we have the video object and surface, call the VO blitter function
  if (!BltVideoObjectToBuffer(pBuffer, uiPitch, hSrcVObject, usRegionIndex, iDestX, iDestY)) {
    VSurfaceUnlock(dest);
    return FALSE;
  }

  VSurfaceUnlock(dest);
  return (TRUE);
}

BOOLEAN DeleteVideoObjectFromIndex(uint32_t uiVObject) {
  VOBJECT_NODE *curr;

  curr = gpVObjectHead;
  while (curr) {
    if (curr->uiIndex == uiVObject) {  // Found the node, so detach it and delete it.

      // Deallocate the memory for the video object
      DeleteVideoObject(curr->hVObject);

      if (curr == gpVObjectHead) {
        // Advance the head, because we are going to remove the head node.
        gpVObjectHead = gpVObjectHead->next;
      }
      if (curr == gpVObjectTail) {
        // Back up the tail, because we are going to remove the tail node.
        gpVObjectTail = gpVObjectTail->prev;
      }
      // Detach the node from the vobject list
      if (curr->next) {  // Make the prev node point to the next
        curr->next->prev = curr->prev;
      }
      if (curr->prev) {  // Make the next node point to the prev
        curr->prev->next = curr->next;
      }
      // The node is now detached.  Now deallocate it.
      MemFree(curr);
      curr = NULL;
      guiVObjectSize--;
#ifdef JA2TESTVERSION
      if (CountVideoObjectNodes() != guiVObjectSize) {
        guiVObjectSize = guiVObjectSize;
      }
#endif
      return TRUE;
    }
    curr = curr->next;
  }
  return FALSE;
}

bool BltVObject(struct VSurface *dest, struct VObject *vobj, uint16_t regionIndex, int32_t x,
                int32_t y) {
  bool res = false;
  if (dest) {
    struct BufferLockInfo lock = VSurfaceLock(dest);

    if (lock.dest == NULL) {
      return false;
    }

    res = BltVideoObjectToBuffer((uint16_t *)lock.dest, lock.pitch, vobj, regionIndex, x, y);
    VSurfaceUnlock(dest);
  }
  return res;
}

// *******************************************************************************
// Video Object Manipulation Functions
// *******************************************************************************

struct VObject *CreateVObjectFromFile(const char *path) {
  // Allocate memory for video object data and initialize
  struct VObject *hVObject = (struct VObject *)MemAlloc(sizeof(struct VObject));
  if (!(hVObject != NULL)) {
    return FALSE;
  }
  memset(hVObject, 0, sizeof(struct VObject));

  // Create himage object from file
  struct Image *hImage = CreateImage(path, IMAGE_ALLIMAGEDATA);

  if (hImage == NULL) {
    MemFree(hVObject);
    DebugMsg(TOPIC_VIDEOOBJECT, DBG_NORMAL, "Invalid Image Filename given");
    return (NULL);
  }

  // Check if returned himage is TRLE compressed - return error if not
  if (!(hImage->fFlags & IMAGE_TRLECOMPRESSED)) {
    MemFree(hVObject);
    DebugMsg(TOPIC_VIDEOOBJECT, DBG_NORMAL, "Invalid Image format given.");
    DestroyImage(hImage);
    return (NULL);
  }

  // Set values from himage
  hVObject->ubBitDepth = hImage->ubBitDepth;

  // Get TRLE data
  ETRLEData TempETRLEData;
  if (!(GetETRLEImageData(hImage, &TempETRLEData))) {
    return FALSE;
  }

  // Set values
  hVObject->usNumberOfObjects = TempETRLEData.usNumberOfObjects;
  hVObject->pETRLEObject = TempETRLEData.pETRLEObject;
  hVObject->pPixData = TempETRLEData.pPixData;
  hVObject->uiSizePixData = TempETRLEData.uiSizePixData;

  // Set palette from himage
  if (hImage->ubBitDepth == 8) {
    hVObject->pShade8 = ubColorTables[DEFAULT_SHADE_LEVEL];
    hVObject->pGlow8 = ubColorTables[0];
    SetVideoObjectPalette(hVObject, hImage->pPalette);
  }

  // Delete himage object
  DestroyImage(hImage);

  return (hVObject);
}

struct VObject *CreateVObjectFromHImage(struct Image *hImage) {
  struct VObject *hVObject;
  ETRLEData TempETRLEData;

  // Allocate memory for video object data and initialize
  hVObject = (struct VObject *)MemAlloc(sizeof(struct VObject));
  if (!(hVObject != NULL)) {
    return FALSE;
  }
  memset(hVObject, 0, sizeof(struct VObject));

  if (hImage == NULL) {
    MemFree(hVObject);
    DebugMsg(TOPIC_VIDEOOBJECT, DBG_NORMAL, "Invalid hImage pointer given");
    return (NULL);
  }

  // Check if returned himage is TRLE compressed - return error if not
  if (!(hImage->fFlags & IMAGE_TRLECOMPRESSED)) {
    MemFree(hVObject);
    DebugMsg(TOPIC_VIDEOOBJECT, DBG_NORMAL, "Invalid Image format given.");
    DestroyImage(hImage);
    return (NULL);
  }

  // Set values from himage
  hVObject->ubBitDepth = hImage->ubBitDepth;

  // Get TRLE data
  if (!(GetETRLEImageData(hImage, &TempETRLEData))) {
    return FALSE;
  }

  // Set values
  hVObject->usNumberOfObjects = TempETRLEData.usNumberOfObjects;
  hVObject->pETRLEObject = TempETRLEData.pETRLEObject;
  hVObject->pPixData = TempETRLEData.pPixData;
  hVObject->uiSizePixData = TempETRLEData.uiSizePixData;

  // Set palette from himage
  if (hImage->ubBitDepth == 8) {
    hVObject->pShade8 = ubColorTables[DEFAULT_SHADE_LEVEL];
    hVObject->pGlow8 = ubColorTables[0];
    SetVideoObjectPalette(hVObject, hImage->pPalette);
  }

  return (hVObject);
}

// Palette setting is expensive, need to set both DDPalette and create 16BPP palette
static BOOLEAN SetVideoObjectPalette(struct VObject *hVObject,
                                     struct SGPPaletteEntry *pSrcPalette) {
  Assert(hVObject != NULL);
  Assert(pSrcPalette != NULL);

  // Create palette object if not already done so
  if (hVObject->pPaletteEntry == NULL) {
    // Create palette
    hVObject->pPaletteEntry =
        (struct SGPPaletteEntry *)MemAlloc(sizeof(struct SGPPaletteEntry) * 256);
    if (!(hVObject->pPaletteEntry != NULL)) {
      return FALSE;
    }

    // Copy src into palette
    memcpy(hVObject->pPaletteEntry, pSrcPalette, sizeof(struct SGPPaletteEntry) * 256);

  } else {
    // Just Change entries
    memcpy(hVObject->pPaletteEntry, pSrcPalette, sizeof(struct SGPPaletteEntry) * 256);
  }

  // Delete 16BPP Palette if one exists
  if (hVObject->p16BPPPalette != NULL) {
    MemFree(hVObject->p16BPPPalette);
    hVObject->p16BPPPalette = NULL;
  }

  // Create 16BPP Palette
  hVObject->p16BPPPalette = Create16BPPPalette(pSrcPalette);
  hVObject->pShadeCurrent = hVObject->p16BPPPalette;

  //  DebugMsg(TOPIC_VIDEOOBJECT, DBG_INFO, String("Video Object Palette change successfull"
  //  ));
  return (TRUE);
}

// Deletes all palettes, surfaces and region data
BOOLEAN DeleteVideoObject(struct VObject *hVObject) {
  uint16_t usLoop;

  // Assertions
  if (!(hVObject != NULL)) {
    return FALSE;
  }

  DestroyObjectPaletteTables(hVObject);

  // Release palette
  if (hVObject->pPaletteEntry != NULL) {
    MemFree(hVObject->pPaletteEntry);
    //		hVObject->pPaletteEntry = NULL;
  }

  if (hVObject->pPixData != NULL) {
    MemFree(hVObject->pPixData);
    //		hVObject->pPixData = NULL;
  }

  if (hVObject->pETRLEObject != NULL) {
    MemFree(hVObject->pETRLEObject);
    //		hVObject->pETRLEObject = NULL;
  }

  if (hVObject->ppZStripInfo != NULL) {
    for (usLoop = 0; usLoop < hVObject->usNumberOfObjects; usLoop++) {
      if (hVObject->ppZStripInfo[usLoop] != NULL) {
        MemFree(hVObject->ppZStripInfo[usLoop]->pbZChange);
        MemFree(hVObject->ppZStripInfo[usLoop]);
      }
    }
    MemFree(hVObject->ppZStripInfo);
    //		hVObject->ppZStripInfo = NULL;
  }

  if (hVObject->usNumberOf16BPPObjects > 0) {
    for (usLoop = 0; usLoop < hVObject->usNumberOf16BPPObjects; usLoop++) {
      MemFree(hVObject->p16BPPObject[usLoop].p16BPPData);
    }
    MemFree(hVObject->p16BPPObject);
  }

  // Release object
  MemFree(hVObject);

  return (TRUE);
}

/**********************************************************************************************
 CreateObjectPaletteTables

                Creates the shading tables for 8-bit brushes. One highlight table is created, based
on the object-type, 3 brightening tables, 1 normal, and 11 darkening tables. The entries are created
iteratively, rather than in a loop to allow hand-tweaking of the values. If you change the
HVOBJECT_SHADE_TABLES symbol, remember to add/delete entries here, it won't adjust automagically.

**********************************************************************************************/

uint16_t CreateObjectPaletteTables(struct VObject *pObj, uint32_t uiType) {
  uint32_t count;

  // this creates the highlight table. Specify the glow-type when creating the tables
  // through uiType, symbols are from VOBJECT.H
  for (count = 0; count < 16; count++) {
    if ((count == 4) && (pObj->p16BPPPalette == pObj->pShades[count]))
      pObj->pShades[count] = NULL;
    else if (pObj->pShades[count] != NULL) {
      MemFree(pObj->pShades[count]);
      pObj->pShades[count] = NULL;
    }
  }

  switch (uiType) {
    case HVOBJECT_GLOW_GREEN:  // green glow
      VObjectUpdateShade(pObj, 0, 0, 255, 0, TRUE);
      break;
    case HVOBJECT_GLOW_BLUE:  // blue glow
      VObjectUpdateShade(pObj, 0, 0, 0, 255, TRUE);
      break;
    case HVOBJECT_GLOW_YELLOW:  // yellow glow
      VObjectUpdateShade(pObj, 0, 255, 255, 0, TRUE);
      break;
    case HVOBJECT_GLOW_RED:  // red glow
      VObjectUpdateShade(pObj, 0, 255, 0, 0, TRUE);
      break;
  }

  // these are the brightening tables, 115%-150% brighter than original
  VObjectUpdateShade(pObj, 1, 293, 293, 293, FALSE);
  VObjectUpdateShade(pObj, 2, 281, 281, 281, FALSE);
  VObjectUpdateShade(pObj, 3, 268, 268, 268, FALSE);

  // palette 4 is the non-modified palette.
  // if the standard one has already been made, we'll use it
  if (pObj->p16BPPPalette != NULL)
    pObj->pShades[4] = pObj->p16BPPPalette;
  else {
    // or create our own, and assign it to the standard one
    VObjectUpdateShade(pObj, 4, 255, 255, 255, FALSE);
    pObj->p16BPPPalette = pObj->pShades[4];
  }

  // the rest are darkening tables, right down to all-black.
  VObjectUpdateShade(pObj, 5, 195, 195, 195, FALSE);
  VObjectUpdateShade(pObj, 6, 165, 165, 165, FALSE);
  VObjectUpdateShade(pObj, 7, 135, 135, 135, FALSE);
  VObjectUpdateShade(pObj, 8, 105, 105, 105, FALSE);
  VObjectUpdateShade(pObj, 9, 75, 75, 75, FALSE);
  VObjectUpdateShade(pObj, 10, 45, 45, 45, FALSE);
  VObjectUpdateShade(pObj, 11, 36, 36, 36, FALSE);
  VObjectUpdateShade(pObj, 12, 27, 27, 27, FALSE);
  VObjectUpdateShade(pObj, 13, 18, 18, 18, FALSE);
  VObjectUpdateShade(pObj, 14, 9, 9, 9, FALSE);
  VObjectUpdateShade(pObj, 15, 0, 0, 0, FALSE);

  // Set current shade table to neutral color
  pObj->pShadeCurrent = pObj->pShades[4];

  // check to make sure every table got a palette
  for (count = 0; (count < HVOBJECT_SHADE_TABLES) && (pObj->pShades[count] != NULL); count++);

  // return the result of the check
  return (count == HVOBJECT_SHADE_TABLES);
}

// *******************************************************************
//
// Blitting Functions
//
// *******************************************************************

// High level blit function encapsolates ALL effects and BPP
static BOOLEAN BltVideoObjectToBuffer(uint16_t *pBuffer, uint32_t uiDestPitchBYTES,
                                      struct VObject *hSrcVObject, uint16_t usIndex, int32_t iDestX,
                                      int32_t iDestY) {
  Assert(pBuffer != NULL);
  Assert(hSrcVObject != NULL);

  switch (hSrcVObject->ubBitDepth) {
    case 16:
      break;

    case 8:
      if (BltIsClipped(hSrcVObject, iDestX, iDestY, usIndex, &ClippingRect)) {
        Blt8BPPDataTo16BPPBufferTransparentClip(pBuffer, uiDestPitchBYTES, hSrcVObject, iDestX,
                                                iDestY, usIndex, &ClippingRect);
      } else {
        Blt8BPPDataTo16BPPBufferTransparent(pBuffer, uiDestPitchBYTES, hSrcVObject, iDestX, iDestY,
                                            usIndex);
      }
      break;
  }

  return (TRUE);
}

/**********************************************************************************************
 DestroyObjectPaletteTables

        Destroys the palette tables of a video object. All memory is deallocated, and
        the pointers set to NULL. Be careful not to try and blit this object until new
        tables are calculated, or things WILL go boom.

**********************************************************************************************/
BOOLEAN DestroyObjectPaletteTables(struct VObject *hVObject) {
  uint32_t x;
  BOOLEAN f16BitPal;

  for (x = 0; x < HVOBJECT_SHADE_TABLES; x++) {
    if (!(hVObject->fFlags & VOBJECT_FLAG_SHADETABLE_SHARED)) {
      if (hVObject->pShades[x] != NULL) {
        if (hVObject->pShades[x] == hVObject->p16BPPPalette)
          f16BitPal = TRUE;
        else
          f16BitPal = FALSE;

        MemFree(hVObject->pShades[x]);
        hVObject->pShades[x] = NULL;

        if (f16BitPal) hVObject->p16BPPPalette = NULL;
      }
    }
  }

  if (hVObject->p16BPPPalette != NULL) {
    MemFree(hVObject->p16BPPPalette);
    hVObject->p16BPPPalette = NULL;
  }

  hVObject->pShadeCurrent = NULL;
  hVObject->pGlow = NULL;

  return (TRUE);
}

uint16_t SetObjectShade(struct VObject *pObj, uint32_t uiShade) {
  Assert(pObj != NULL);
  Assert(uiShade >= 0);
  Assert(uiShade < HVOBJECT_SHADE_TABLES);

  if (pObj->pShades[uiShade] == NULL) {
    DebugMsg(TOPIC_VIDEOOBJECT, DBG_NORMAL, String("Attempt to set shade level to NULL table"));
    return (FALSE);
  }

  pObj->pShadeCurrent = pObj->pShades[uiShade];
  return (TRUE);
}

uint16_t SetObjectHandleShade(uint32_t uiHandle, uint32_t uiShade) {
  struct VObject *hObj;

  if (!GetVideoObject(&hObj, uiHandle)) {
    DebugMsg(TOPIC_VIDEOOBJECT, DBG_NORMAL,
             String("Invalid object handle for setting shade level"));
    return (FALSE);
  }
  return (SetObjectShade(hObj, uiShade));
}

/********************************************************************************************
        GetETRLEPixelValue

        Given a VOBJECT and ETRLE image index, retrieves the value of the pixel located at the
        given image coordinates. The value returned is an 8-bit palette index
********************************************************************************************/
BOOLEAN GetETRLEPixelValue(uint8_t *pDest, struct VObject *hVObject, uint16_t usETRLEIndex,
                           uint16_t usX, uint16_t usY) {
  uint8_t *pCurrent;
  uint16_t usLoopX = 0;
  uint16_t usLoopY = 0;
  uint16_t ubRunLength;
  ETRLEObject *pETRLEObject;

  // Do a bunch of checks
  if (!(hVObject != NULL)) {
    return FALSE;
  }
  if (!(usETRLEIndex < hVObject->usNumberOfObjects)) {
    return FALSE;
  }

  pETRLEObject = &(hVObject->pETRLEObject[usETRLEIndex]);

  if (!(usX < pETRLEObject->usWidth)) {
    return FALSE;
  }
  if (!(usY < pETRLEObject->usHeight)) {
    return FALSE;
  }

  // Assuming everything's okay, go ahead and look...
  pCurrent = &((uint8_t *)hVObject->pPixData)[pETRLEObject->uiDataOffset];

  // Skip past all uninteresting scanlines
  while (usLoopY < usY) {
    while (*pCurrent != 0) {
      if (*pCurrent & COMPRESS_TRANSPARENT) {
        pCurrent++;
      } else {
        pCurrent += *pCurrent & COMPRESS_RUN_MASK;
      }
    }
    usLoopY++;
  }

  // Now look in this scanline for the appropriate byte
  do {
    ubRunLength = *pCurrent & COMPRESS_RUN_MASK;

    if (*pCurrent & COMPRESS_TRANSPARENT) {
      if (usLoopX + ubRunLength >= usX) {
        *pDest = 0;
        return (TRUE);
      } else {
        pCurrent++;
      }
    } else {
      if (usLoopX + ubRunLength >= usX) {
        // skip to the correct byte; skip at least 1 to get past the byte defining the run
        pCurrent += (usX - usLoopX) + 1;
        *pDest = *pCurrent;
        return (TRUE);
      } else {
        pCurrent += ubRunLength + 1;
      }
    }
    usLoopX += ubRunLength;
  } while (usLoopX < usX);
  // huh???
  return (FALSE);
}

BOOLEAN GetVideoObjectETRLEProperties(struct VObject *hVObject, ETRLEObject *pETRLEObject,
                                      uint16_t usIndex) {
  if (!(usIndex >= 0)) {
    return FALSE;
  }
  if (!(usIndex < hVObject->usNumberOfObjects)) {
    return FALSE;
  }

  memcpy(pETRLEObject, &(hVObject->pETRLEObject[usIndex]), sizeof(ETRLEObject));

  return (TRUE);
}

BOOLEAN GetVideoObjectETRLESubregionProperties(uint32_t uiVideoObject, uint16_t usIndex,
                                               uint16_t *pusWidth, uint16_t *pusHeight) {
  struct VObject *hVObject;
  ETRLEObject ETRLEObject;

  // Get video object
  if (!(GetVideoObject(&hVObject, uiVideoObject))) {
    return FALSE;
  }

  if (!(GetVideoObjectETRLEProperties(hVObject, &ETRLEObject, usIndex))) {
    return FALSE;
  }

  *pusWidth = ETRLEObject.usWidth;
  *pusHeight = ETRLEObject.usHeight;

  return (TRUE);
}

BOOLEAN GetVideoObjectETRLEPropertiesFromIndex(uint32_t uiVideoObject, ETRLEObject *pETRLEObject,
                                               uint16_t usIndex) {
  struct VObject *hVObject;

  // Get video object
  if (!(GetVideoObject(&hVObject, uiVideoObject))) {
    return FALSE;
  }

  if (!(GetVideoObjectETRLEProperties(hVObject, pETRLEObject, usIndex))) {
    return FALSE;
  }

  return (TRUE);
}

BOOLEAN BltVideoObjectOutlineFromIndex(struct VSurface *dest, uint32_t uiSrcVObject,
                                       uint16_t usIndex, int32_t iDestX, int32_t iDestY,
                                       int16_t s16BPPColor, BOOLEAN fDoOutline) {
  uint16_t *pBuffer;
  uint32_t uiPitch;
  struct VObject *hSrcVObject;

  // Lock video surface
  pBuffer = (uint16_t *)VSurfaceLockOld(dest, &uiPitch);

  if (pBuffer == NULL) {
    return (FALSE);
  }

  // Get video object
  if (!(GetVideoObject(&hSrcVObject, uiSrcVObject))) {
    return FALSE;
  }

  if (BltIsClipped(hSrcVObject, iDestX, iDestY, usIndex, &ClippingRect)) {
    Blt8BPPDataTo16BPPBufferOutlineClip((uint16_t *)pBuffer, uiPitch, hSrcVObject, iDestX, iDestY,
                                        usIndex, s16BPPColor, fDoOutline, &ClippingRect);
  } else {
    Blt8BPPDataTo16BPPBufferOutline((uint16_t *)pBuffer, uiPitch, hSrcVObject, iDestX, iDestY,
                                    usIndex, s16BPPColor, fDoOutline);
  }

  VSurfaceUnlock(dest);
  return (TRUE);
}

BOOLEAN BltVideoObjectOutline(struct VSurface *dest, struct VObject *hSrcVObject, uint16_t usIndex,
                              int32_t iDestX, int32_t iDestY, int16_t s16BPPColor,
                              BOOLEAN fDoOutline) {
  uint16_t *pBuffer;
  uint32_t uiPitch;
  // Lock video surface
  pBuffer = (uint16_t *)VSurfaceLockOld(dest, &uiPitch);

  if (pBuffer == NULL) {
    return (FALSE);
  }

  if (BltIsClipped(hSrcVObject, iDestX, iDestY, usIndex, &ClippingRect)) {
    Blt8BPPDataTo16BPPBufferOutlineClip((uint16_t *)pBuffer, uiPitch, hSrcVObject, iDestX, iDestY,
                                        usIndex, s16BPPColor, fDoOutline, &ClippingRect);
  } else {
    Blt8BPPDataTo16BPPBufferOutline((uint16_t *)pBuffer, uiPitch, hSrcVObject, iDestX, iDestY,
                                    usIndex, s16BPPColor, fDoOutline);
  }
  VSurfaceUnlock(dest);
  return (TRUE);
}

BOOLEAN BltVideoObjectOutlineShadowFromIndex(struct VSurface *dest, uint32_t uiSrcVObject,
                                             uint16_t usIndex, int32_t iDestX, int32_t iDestY) {
  uint16_t *pBuffer;
  uint32_t uiPitch;
  struct VObject *hSrcVObject;

  // Lock video surface
  pBuffer = (uint16_t *)VSurfaceLockOld(dest, &uiPitch);

  if (pBuffer == NULL) {
    return (FALSE);
  }

  // Get video object
  if (!(GetVideoObject(&hSrcVObject, uiSrcVObject))) {
    return FALSE;
  }

  if (BltIsClipped(hSrcVObject, iDestX, iDestY, usIndex, &ClippingRect)) {
    Blt8BPPDataTo16BPPBufferOutlineShadowClip((uint16_t *)pBuffer, uiPitch, hSrcVObject, iDestX,
                                              iDestY, usIndex, &ClippingRect);
  } else {
    Blt8BPPDataTo16BPPBufferOutlineShadow((uint16_t *)pBuffer, uiPitch, hSrcVObject, iDestX, iDestY,
                                          usIndex);
  }

  VSurfaceUnlock(dest);
  return (TRUE);
}

void VObjectUpdateShade(struct VObject *obj, uint8_t shade_num, uint32_t rscale, uint32_t gscale,
                        uint32_t bscale, BOOLEAN mono) {
  obj->pShades[shade_num] =
      Create16BPPPaletteShaded(obj->pPaletteEntry, rscale, gscale, bscale, mono);
}
