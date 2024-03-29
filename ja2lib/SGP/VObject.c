#include "SGP/VObject.h"

#include <stdio.h>
#include <string.h>

#include "Globals.h"
#include "SGP/Debug.h"
#include "SGP/HImage.h"
#include "SGP/Shading.h"
#include "SGP/Types.h"
#include "SGP/VObjectBlitters.h"
#include "SGP/VObjectInternal.h"
#include "SGP/VSurface.h"
#include "SGP/Video.h"
#include "SGP/WCheck.h"
#include "platform_strings.h"
#include "rust_images.h"

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
  UINT32 uiIndex;
  struct VOBJECT_NODE *next, *prev;
} VOBJECT_NODE;

// TODO: rust
VOBJECT_NODE *gpVObjectHead = NULL;
VOBJECT_NODE *gpVObjectTail = NULL;
UINT32 guiVObjectIndex = 1;
UINT32 guiVObjectSize = 0;
UINT32 guiVObjectTotalAdded = 0;

static BOOLEAN BltVideoObjectToBuffer(UINT16 *pBuffer, UINT32 uiDestPitchBYTES,
                                      struct VObject *hSrcVObject, UINT16 usIndex, INT32 iDestX,
                                      INT32 iDestY);

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

// This structure describes the creation parameters for a Video Object
typedef struct {
  UINT32 fCreateFlags;  // Specifies creation flags like from file or not
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

// TODO: rust
// TODO: debug print how many video objects are there
// TODO: probably replace the list with array of fixed size
static BOOLEAN _AddVideoObject(struct VObject *hVObject, UINT32 *puiIndex) {
  Assert(puiIndex);
  Assert(hVObject);

  if (!hVObject) {
    // Video Object will set error condition.
    return FALSE;
  }

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

  return TRUE;
}

BOOLEAN AddVObjectFromFile(const char *path, UINT32 *puiIndex) {
  return _AddVideoObject(LoadVObjectFromFile(path), puiIndex);
}

// TODO: rust
BOOLEAN GetVideoObject(struct VObject **hVObject, UINT32 uiIndex) {
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

bool BltVObjectFromIndex(struct VSurface *dest, UINT32 uiSrcVObject, UINT16 usRegionIndex,
                         INT32 iDestX, INT32 iDestY) {
  UINT16 *pBuffer;
  UINT32 uiPitch;
  struct VObject *hSrcVObject;

  // Lock video surface
  pBuffer = (UINT16 *)VSurfaceLockOld(dest, &uiPitch);

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

BOOLEAN DeleteVideoObjectFromIndex(UINT32 uiVObject) {
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
      return TRUE;
    }
    curr = curr->next;
  }
  return FALSE;
}

bool BltVObject(struct VSurface *dest, struct VObject *vobj, u16 regionIndex, i32 x, i32 y) {
  bool res = false;
  if (dest) {
    struct BufferLockInfo lock = VSurfaceLock(dest);

    if (lock.dest == NULL) {
      return false;
    }

    res = BltVideoObjectToBuffer((u16 *)lock.dest, lock.pitch, vobj, regionIndex, x, y);
    VSurfaceUnlock(dest);
  }
  return res;
}

// *******************************************************************************
// Video Object Manipulation Functions
// *******************************************************************************

struct VObject *LoadVObjectFromFile(const char *path) {
  struct Image *image = CreateImage(path, false);
  struct VObject *vobject = CreateVObjectFromImage(image);
  DestroyImage(image);
  return vobject;
}

struct VObject *CreateVObjectFromImage(struct Image *hImage) {
  if (hImage == NULL) {
    DebugMsg(TOPIC_VIDEOOBJECT, DBG_NORMAL, "Invalid hImage pointer given");
    return (NULL);
  }

  // Check if returned himage is TRLE compressed - return error if not
  if (!hImage->subimages) {
    DebugMsg(TOPIC_VIDEOOBJECT, DBG_NORMAL, "Invalid Image format given.");
    return (NULL);
  }

  struct VObject *hVObject = (struct VObject *)zmalloc(sizeof(struct VObject));
  if (!(hVObject != NULL)) {
    return FALSE;
  }

  // Set values from himage
  hVObject->ubBitDepth = hImage->bit_depth;

  // Get TRLE data
  struct ImageData TempETRLEData;
  if (!(CopyImageData(hImage, &TempETRLEData))) {
    return FALSE;
  }

  // Set values
  hVObject->number_of_subimages = TempETRLEData.number_of_subimages;
  hVObject->subimages = TempETRLEData.subimages;
  hVObject->image_data = TempETRLEData.image_data;
  hVObject->image_data_size = TempETRLEData.image_data_size;

  // Set palette from himage
  if (hImage->bit_depth == 8) {
    SetVideoObjectPalette(hVObject, hImage->palette);
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

  return (TRUE);
}

// Deletes all palettes, surfaces and region data
BOOLEAN DeleteVideoObject(struct VObject *hVObject) {
  UINT16 usLoop;

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

  if (hVObject->image_data != NULL) {
    MemFree(hVObject->image_data);
    //		hVObject->image_data = NULL;
  }

  if (hVObject->subimages != NULL) {
    MemFree(hVObject->subimages);
    //		hVObject->subimages = NULL;
  }

  if (hVObject->ppZStripInfo != NULL) {
    for (usLoop = 0; usLoop < hVObject->number_of_subimages; usLoop++) {
      if (hVObject->ppZStripInfo[usLoop] != NULL) {
        MemFree(hVObject->ppZStripInfo[usLoop]->pbZChange);
        MemFree(hVObject->ppZStripInfo[usLoop]);
      }
    }
    MemFree(hVObject->ppZStripInfo);
  }

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

UINT16 CreateObjectPaletteTables(struct VObject *pObj, UINT32 uiType) {
  UINT32 count;

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
  for (count = 0; (count < HVOBJECT_SHADE_TABLES) && (pObj->pShades[count] != NULL); count++)
    ;

  // return the result of the check
  return (count == HVOBJECT_SHADE_TABLES);
}

// *******************************************************************
//
// Blitting Functions
//
// *******************************************************************

// High level blit function encapsolates ALL effects and BPP
static BOOLEAN BltVideoObjectToBuffer(UINT16 *pBuffer, UINT32 uiDestPitchBYTES,
                                      struct VObject *hSrcVObject, UINT16 usIndex, INT32 iDestX,
                                      INT32 iDestY) {
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
  UINT32 x;
  BOOLEAN f16BitPal;

  for (x = 0; x < HVOBJECT_SHADE_TABLES; x++) {
    if (!(hVObject->shared_shadetable)) {
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

UINT16 SetObjectShade(struct VObject *pObj, UINT32 uiShade) {
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

UINT16 SetObjectHandleShade(UINT32 uiHandle, UINT32 uiShade) {
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
BOOLEAN GetETRLEPixelValue(UINT8 *pDest, struct VObject *hVObject, UINT16 usETRLEIndex, UINT16 usX,
                           UINT16 usY) {
  UINT8 *pCurrent;
  UINT16 usLoopX = 0;
  UINT16 usLoopY = 0;
  UINT16 ubRunLength;
  struct Subimage *subimages;

  // Do a bunch of checks
  if (!(hVObject != NULL)) {
    return FALSE;
  }
  if (!(usETRLEIndex < hVObject->number_of_subimages)) {
    return FALSE;
  }

  subimages = &(hVObject->subimages[usETRLEIndex]);

  if (!(usX < subimages->width)) {
    return FALSE;
  }
  if (!(usY < subimages->height)) {
    return FALSE;
  }

  // Assuming everything's okay, go ahead and look...
  pCurrent = &((UINT8 *)hVObject->image_data)[subimages->data_offset];

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

BOOLEAN GetVideoObjectETRLEProperties(struct VObject *hVObject, struct Subimage *subimages,
                                      UINT16 usIndex) {
  if (!(usIndex >= 0)) {
    return FALSE;
  }
  if (!(usIndex < hVObject->number_of_subimages)) {
    return FALSE;
  }

  memcpy(subimages, &(hVObject->subimages[usIndex]), sizeof(struct Subimage));

  return (TRUE);
}

BOOLEAN GetVideoObjectETRLESubregionProperties(UINT32 uiVideoObject, UINT16 usIndex,
                                               UINT16 *pusWidth, UINT16 *pusHeight) {
  struct VObject *hVObject;
  struct Subimage ETRLEObject;

  // Get video object
  if (!(GetVideoObject(&hVObject, uiVideoObject))) {
    return FALSE;
  }

  if (!(GetVideoObjectETRLEProperties(hVObject, &ETRLEObject, usIndex))) {
    return FALSE;
  }

  *pusWidth = ETRLEObject.width;
  *pusHeight = ETRLEObject.height;

  return (TRUE);
}

BOOLEAN GetVideoObjectETRLEPropertiesFromIndex(UINT32 uiVideoObject, struct Subimage *subimages,
                                               UINT16 usIndex) {
  struct VObject *hVObject;

  // Get video object
  if (!(GetVideoObject(&hVObject, uiVideoObject))) {
    return FALSE;
  }

  if (!(GetVideoObjectETRLEProperties(hVObject, subimages, usIndex))) {
    return FALSE;
  }

  return (TRUE);
}

BOOLEAN BltVideoObjectOutlineFromIndex(struct VSurface *dest, UINT32 uiSrcVObject, UINT16 usIndex,
                                       INT32 iDestX, INT32 iDestY, INT16 s16BPPColor,
                                       BOOLEAN fDoOutline) {
  UINT16 *pBuffer;
  UINT32 uiPitch;
  struct VObject *hSrcVObject;

  // Lock video surface
  pBuffer = (UINT16 *)VSurfaceLockOld(dest, &uiPitch);

  if (pBuffer == NULL) {
    return (FALSE);
  }

  // Get video object
  if (!(GetVideoObject(&hSrcVObject, uiSrcVObject))) {
    return FALSE;
  }

  if (BltIsClipped(hSrcVObject, iDestX, iDestY, usIndex, &ClippingRect)) {
    Blt8BPPDataTo16BPPBufferOutlineClip((UINT16 *)pBuffer, uiPitch, hSrcVObject, iDestX, iDestY,
                                        usIndex, s16BPPColor, fDoOutline, &ClippingRect);
  } else {
    Blt8BPPDataTo16BPPBufferOutline((UINT16 *)pBuffer, uiPitch, hSrcVObject, iDestX, iDestY,
                                    usIndex, s16BPPColor, fDoOutline);
  }

  VSurfaceUnlock(dest);
  return (TRUE);
}

BOOLEAN BltVideoObjectOutline(struct VSurface *dest, struct VObject *hSrcVObject, UINT16 usIndex,
                              INT32 iDestX, INT32 iDestY, INT16 s16BPPColor, BOOLEAN fDoOutline) {
  UINT16 *pBuffer;
  UINT32 uiPitch;
  // Lock video surface
  pBuffer = (UINT16 *)VSurfaceLockOld(dest, &uiPitch);

  if (pBuffer == NULL) {
    return (FALSE);
  }

  if (BltIsClipped(hSrcVObject, iDestX, iDestY, usIndex, &ClippingRect)) {
    Blt8BPPDataTo16BPPBufferOutlineClip((UINT16 *)pBuffer, uiPitch, hSrcVObject, iDestX, iDestY,
                                        usIndex, s16BPPColor, fDoOutline, &ClippingRect);
  } else {
    Blt8BPPDataTo16BPPBufferOutline((UINT16 *)pBuffer, uiPitch, hSrcVObject, iDestX, iDestY,
                                    usIndex, s16BPPColor, fDoOutline);
  }
  VSurfaceUnlock(dest);
  return (TRUE);
}

BOOLEAN BltVideoObjectOutlineShadowFromIndex(struct VSurface *dest, UINT32 uiSrcVObject,
                                             UINT16 usIndex, INT32 iDestX, INT32 iDestY) {
  UINT16 *pBuffer;
  UINT32 uiPitch;
  struct VObject *hSrcVObject;

  // Lock video surface
  pBuffer = (UINT16 *)VSurfaceLockOld(dest, &uiPitch);

  if (pBuffer == NULL) {
    return (FALSE);
  }

  // Get video object
  if (!(GetVideoObject(&hSrcVObject, uiSrcVObject))) {
    return FALSE;
  }

  if (BltIsClipped(hSrcVObject, iDestX, iDestY, usIndex, &ClippingRect)) {
    Blt8BPPDataTo16BPPBufferOutlineShadowClip((UINT16 *)pBuffer, uiPitch, hSrcVObject, iDestX,
                                              iDestY, usIndex, &ClippingRect);
  } else {
    Blt8BPPDataTo16BPPBufferOutlineShadow((UINT16 *)pBuffer, uiPitch, hSrcVObject, iDestX, iDestY,
                                          usIndex);
  }

  VSurfaceUnlock(dest);
  return (TRUE);
}

void VObjectUpdateShade(struct VObject *obj, u8 shade_num, u32 rscale, u32 gscale, u32 bscale,
                        BOOLEAN mono) {
  obj->pShades[shade_num] =
      Create16BPPPaletteShaded(obj->pPaletteEntry, rscale, gscale, bscale, mono);
}
