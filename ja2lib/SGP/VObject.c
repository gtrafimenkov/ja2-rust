#include "SGP/VObject.h"

#include <stdio.h>
#include <string.h>

#include "Globals.h"
#include "SGP/Debug.h"
#include "SGP/HImage.h"
#include "SGP/Shading.h"
#include "SGP/Types.h"
#include "SGP/VObjectBlitters.h"
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
  UINT32 uiIndex;
  struct VOBJECT_NODE *next, *prev;
} VOBJECT_NODE;

VOBJECT_NODE *gpVObjectHead = NULL;
VOBJECT_NODE *gpVObjectTail = NULL;
UINT32 guiVObjectIndex = 1;
UINT32 guiVObjectSize = 0;
UINT32 guiVObjectTotalAdded = 0;

static BOOLEAN BltVideoObjectToBuffer(UINT16 *pBuffer, UINT32 uiDestPitchBYTES,
                                      struct VObject *hSrcVObject, UINT16 usIndex, INT32 iDestX,
                                      INT32 iDestY, INT32 fBltFlags, blt_fx *pBltFx);

// Sets struct VObject* palette, creates if nessessary. Also sets 16BPP palette
static BOOLEAN SetVideoObjectPalette(struct VObject *hVObject, struct SGPPaletteEntry *pSrcPalette);

// Sets Transparency color into struct VObject*
static BOOLEAN SetVideoObjectTransparencyColor(struct VObject *hVObject, COLORVAL TransColor);

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

UINT32 CountVideoObjectNodes() {
  VOBJECT_NODE *curr;
  UINT32 i = 0;
  curr = gpVObjectHead;
  while (curr) {
    i++;
    curr = curr->next;
  }
  return i;
}

BOOLEAN AddVideoObject(VOBJECT_DESC *pVObjectDesc, UINT32 *puiIndex) {
  struct VObject *hVObject;

  // Assertions
  Assert(puiIndex);
  Assert(pVObjectDesc);

  // Create video object
  hVObject = CreateVideoObject(pVObjectDesc);

  if (!hVObject) {
    // Video Object will set error condition.
    return FALSE;
  }

  // Set transparency to default
  SetVideoObjectTransparencyColor(hVObject, FROMRGB(0, 0, 0));

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

BOOLEAN SetVideoObjectTransparency(UINT32 uiIndex, COLORVAL TransColor) {
  struct VObject *hVObject;

  // Get video object
  CHECKF(GetVideoObject(&hVObject, uiIndex));

  // Set transparency
  SetVideoObjectTransparencyColor(hVObject, TransColor);

  return (TRUE);
}

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

BOOLEAN BltVideoObjectFromIndex(UINT32 uiDestVSurface, UINT32 uiSrcVObject, UINT16 usRegionIndex,
                                INT32 iDestX, INT32 iDestY, UINT32 fBltFlags, blt_fx *pBltFx) {
  UINT16 *pBuffer;
  UINT32 uiPitch;
  struct VObject *hSrcVObject;

  // Lock video surface
  pBuffer = (UINT16 *)LockVideoSurface(uiDestVSurface, &uiPitch);

  if (pBuffer == NULL) {
    return (FALSE);
  }

  // Get video object
  if (!GetVideoObject(&hSrcVObject, uiSrcVObject)) {
    UnLockVideoSurface(uiDestVSurface);
    return FALSE;
  }

  // Now we have the video object and surface, call the VO blitter function
  if (!BltVideoObjectToBuffer(pBuffer, uiPitch, hSrcVObject, usRegionIndex, iDestX, iDestY,
                              fBltFlags, pBltFx)) {
    UnLockVideoSurface(uiDestVSurface);
    // VO Blitter will set debug messages for error conditions
    return FALSE;
  }

  UnLockVideoSurface(uiDestVSurface);
  return (TRUE);
}

BOOLEAN DeleteVideoObjectFromIndex(UINT32 uiVObject) {
  VOBJECT_NODE *curr;

  curr = gpVObjectHead;
  while (curr) {
    if (curr->uiIndex == uiVObject) {  // Found the node, so detach it and delete it.

      // Deallocate the memory for the video object
      DeleteVideoObject(curr->hVObject);

      if (curr ==
          gpVObjectHead) {  // Advance the head, because we are going to remove the head node.
        gpVObjectHead = gpVObjectHead->next;
      }
      if (curr ==
          gpVObjectTail) {  // Back up the tail, because we are going to remove the tail node.
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

// Given an index to the dest and src vobject contained in ghVideoObjects
// Based on flags, blit accordingly
// There are two types, a BltFast and a Blt. BltFast is 10% faster, uses no
// clipping lists
BOOLEAN BltVideoObject(UINT32 uiDestVSurface, struct VObject *hSrcVObject, UINT16 usRegionIndex,
                       INT32 iDestX, INT32 iDestY, UINT32 fBltFlags, blt_fx *pBltFx) {
  UINT16 *pBuffer;
  UINT32 uiPitch;

  // Lock video surface
  pBuffer = (UINT16 *)LockVideoSurface(uiDestVSurface, &uiPitch);

  if (pBuffer == NULL) {
    return (FALSE);
  }

  // Now we have the video object and surface, call the VO blitter function
  if (!BltVideoObjectToBuffer(pBuffer, uiPitch, hSrcVObject, usRegionIndex, iDestX, iDestY,
                              fBltFlags, pBltFx)) {
    UnLockVideoSurface(uiDestVSurface);
    // VO Blitter will set debug messages for error conditions
    return (FALSE);
  }

  UnLockVideoSurface(uiDestVSurface);
  return (TRUE);
}

// *******************************************************************************
// Video Object Manipulation Functions
// *******************************************************************************

struct VObject *CreateVObjectFromFile(const char *path) {
  struct VObject *hVObject;
  HIMAGE hImage;
  ETRLEData TempETRLEData;

  // Allocate memory for video object data and initialize
  hVObject = (struct VObject *)MemAlloc(sizeof(struct VObject));
  CHECKF(hVObject != NULL);
  memset(hVObject, 0, sizeof(struct VObject));

  // Create himage object from file
  hImage = CreateImage(path, IMAGE_ALLIMAGEDATA);

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
  CHECKF(GetETRLEImageData(hImage, &TempETRLEData));

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

struct VObject *CreateVObjectFromHImage(HIMAGE hImage) {
  struct VObject *hVObject;
  ETRLEData TempETRLEData;

  // Allocate memory for video object data and initialize
  hVObject = (struct VObject *)MemAlloc(sizeof(struct VObject));
  CHECKF(hVObject != NULL);
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
  CHECKF(GetETRLEImageData(hImage, &TempETRLEData));

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

struct VObject *CreateVideoObject(VOBJECT_DESC *VObjectDesc) {
  if (VObjectDesc->fCreateFlags & VOBJECT_CREATE_FROMFILE) {
    return CreateVObjectFromFile(VObjectDesc->ImageFile);
  } else if (VObjectDesc->fCreateFlags & VOBJECT_CREATE_FROMHIMAGE) {
    return CreateVObjectFromHImage(VObjectDesc->hImage);
  } else {
    DebugMsg(TOPIC_VIDEOOBJECT, DBG_NORMAL, "Invalid VObject creation flags given.");
    return (NULL);
  }
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
    CHECKF(hVObject->pPaletteEntry != NULL);

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

// Transparency needs to take RGB value and find best fit and place it into DD Surface
// colorkey value.
static BOOLEAN SetVideoObjectTransparencyColor(struct VObject *hVObject, COLORVAL TransColor) {
  // Assertions
  Assert(hVObject != NULL);

  // Set trans color into video object
  hVObject->TransparentColor = TransColor;

  return (TRUE);
}

// Deletes all palettes, surfaces and region data
BOOLEAN DeleteVideoObject(struct VObject *hVObject) {
  UINT16 usLoop;

  // Assertions
  CHECKF(hVObject != NULL);

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
      pObj->pShades[0] = Create16BPPPaletteShaded(pObj->pPaletteEntry, 0, 255, 0, TRUE);
      break;
    case HVOBJECT_GLOW_BLUE:  // blue glow
      pObj->pShades[0] = Create16BPPPaletteShaded(pObj->pPaletteEntry, 0, 0, 255, TRUE);
      break;
    case HVOBJECT_GLOW_YELLOW:  // yellow glow
      pObj->pShades[0] = Create16BPPPaletteShaded(pObj->pPaletteEntry, 255, 255, 0, TRUE);
      break;
    case HVOBJECT_GLOW_RED:  // red glow
      pObj->pShades[0] = Create16BPPPaletteShaded(pObj->pPaletteEntry, 255, 0, 0, TRUE);
      break;
  }

  // these are the brightening tables, 115%-150% brighter than original
  pObj->pShades[1] = Create16BPPPaletteShaded(pObj->pPaletteEntry, 293, 293, 293, FALSE);
  pObj->pShades[2] = Create16BPPPaletteShaded(pObj->pPaletteEntry, 281, 281, 281, FALSE);
  pObj->pShades[3] = Create16BPPPaletteShaded(pObj->pPaletteEntry, 268, 268, 268, FALSE);

  // palette 4 is the non-modified palette.
  // if the standard one has already been made, we'll use it
  if (pObj->p16BPPPalette != NULL)
    pObj->pShades[4] = pObj->p16BPPPalette;
  else {
    // or create our own, and assign it to the standard one
    pObj->pShades[4] = Create16BPPPaletteShaded(pObj->pPaletteEntry, 255, 255, 255, FALSE);
    pObj->p16BPPPalette = pObj->pShades[4];
  }

  // the rest are darkening tables, right down to all-black.
  pObj->pShades[5] = Create16BPPPaletteShaded(pObj->pPaletteEntry, 195, 195, 195, FALSE);
  pObj->pShades[6] = Create16BPPPaletteShaded(pObj->pPaletteEntry, 165, 165, 165, FALSE);
  pObj->pShades[7] = Create16BPPPaletteShaded(pObj->pPaletteEntry, 135, 135, 135, FALSE);
  pObj->pShades[8] = Create16BPPPaletteShaded(pObj->pPaletteEntry, 105, 105, 105, FALSE);
  pObj->pShades[9] = Create16BPPPaletteShaded(pObj->pPaletteEntry, 75, 75, 75, FALSE);
  pObj->pShades[10] = Create16BPPPaletteShaded(pObj->pPaletteEntry, 45, 45, 45, FALSE);
  pObj->pShades[11] = Create16BPPPaletteShaded(pObj->pPaletteEntry, 36, 36, 36, FALSE);
  pObj->pShades[12] = Create16BPPPaletteShaded(pObj->pPaletteEntry, 27, 27, 27, FALSE);
  pObj->pShades[13] = Create16BPPPaletteShaded(pObj->pPaletteEntry, 18, 18, 18, FALSE);
  pObj->pShades[14] = Create16BPPPaletteShaded(pObj->pPaletteEntry, 9, 9, 9, FALSE);
  pObj->pShades[15] = Create16BPPPaletteShaded(pObj->pPaletteEntry, 0, 0, 0, FALSE);

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
                                      INT32 iDestY, INT32 fBltFlags, blt_fx *pBltFx) {
  // Assertions
  Assert(pBuffer != NULL);

  Assert(hSrcVObject != NULL);

  // Check For Flags and bit depths
  switch (hSrcVObject->ubBitDepth) {
    case 16:

      break;

    case 8:

      // Switch based on flags given
      do {
        if (fBltFlags & VO_BLT_SRCTRANSPARENCY) {
          if (BltIsClipped(hSrcVObject, iDestX, iDestY, usIndex, &ClippingRect))
            Blt8BPPDataTo16BPPBufferTransparentClip(pBuffer, uiDestPitchBYTES, hSrcVObject, iDestX,
                                                    iDestY, usIndex, &ClippingRect);
          else
            Blt8BPPDataTo16BPPBufferTransparent(pBuffer, uiDestPitchBYTES, hSrcVObject, iDestX,
                                                iDestY, usIndex);
          break;
        } else if (fBltFlags & VO_BLT_SHADOW) {
          if (BltIsClipped(hSrcVObject, iDestX, iDestY, usIndex, &ClippingRect))
            Blt8BPPDataTo16BPPBufferShadowClip(pBuffer, uiDestPitchBYTES, hSrcVObject, iDestX,
                                               iDestY, usIndex, &ClippingRect);
          else
            Blt8BPPDataTo16BPPBufferShadow(pBuffer, uiDestPitchBYTES, hSrcVObject, iDestX, iDestY,
                                           usIndex);
          break;
        }
        // Use default blitter here
        // Blt8BPPDataTo16BPPBuffer( hDestVObject, hSrcVObject, (UINT16)iDestX, (UINT16)iDestY,
        // (SGPRect*)&SrcRect );

      } while (FALSE);

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
  ETRLEObject *pETRLEObject;

  // Do a bunch of checks
  CHECKF(hVObject != NULL);
  CHECKF(usETRLEIndex < hVObject->usNumberOfObjects);

  pETRLEObject = &(hVObject->pETRLEObject[usETRLEIndex]);

  CHECKF(usX < pETRLEObject->usWidth);
  CHECKF(usY < pETRLEObject->usHeight);

  // Assuming everything's okay, go ahead and look...
  pCurrent = &((UINT8 *)hVObject->pPixData)[pETRLEObject->uiDataOffset];

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
                                      UINT16 usIndex) {
  CHECKF(usIndex >= 0);
  CHECKF(usIndex < hVObject->usNumberOfObjects);

  memcpy(pETRLEObject, &(hVObject->pETRLEObject[usIndex]), sizeof(ETRLEObject));

  return (TRUE);
}

BOOLEAN GetVideoObjectETRLESubregionProperties(UINT32 uiVideoObject, UINT16 usIndex,
                                               UINT16 *pusWidth, UINT16 *pusHeight) {
  struct VObject *hVObject;
  ETRLEObject ETRLEObject;

  // Get video object
  CHECKF(GetVideoObject(&hVObject, uiVideoObject));

  CHECKF(GetVideoObjectETRLEProperties(hVObject, &ETRLEObject, usIndex));

  *pusWidth = ETRLEObject.usWidth;
  *pusHeight = ETRLEObject.usHeight;

  return (TRUE);
}

BOOLEAN GetVideoObjectETRLEPropertiesFromIndex(UINT32 uiVideoObject, ETRLEObject *pETRLEObject,
                                               UINT16 usIndex) {
  struct VObject *hVObject;

  // Get video object
  CHECKF(GetVideoObject(&hVObject, uiVideoObject));

  CHECKF(GetVideoObjectETRLEProperties(hVObject, pETRLEObject, usIndex));

  return (TRUE);
}

BOOLEAN BltVideoObjectOutlineFromIndex(UINT32 uiDestVSurface, UINT32 uiSrcVObject, UINT16 usIndex,
                                       INT32 iDestX, INT32 iDestY, INT16 s16BPPColor,
                                       BOOLEAN fDoOutline) {
  UINT16 *pBuffer;
  UINT32 uiPitch;
  struct VObject *hSrcVObject;

  // Lock video surface
  pBuffer = (UINT16 *)LockVideoSurface(uiDestVSurface, &uiPitch);

  if (pBuffer == NULL) {
    return (FALSE);
  }

  // Get video object
  CHECKF(GetVideoObject(&hSrcVObject, uiSrcVObject));

  if (BltIsClipped(hSrcVObject, iDestX, iDestY, usIndex, &ClippingRect)) {
    Blt8BPPDataTo16BPPBufferOutlineClip((UINT16 *)pBuffer, uiPitch, hSrcVObject, iDestX, iDestY,
                                        usIndex, s16BPPColor, fDoOutline, &ClippingRect);
  } else {
    Blt8BPPDataTo16BPPBufferOutline((UINT16 *)pBuffer, uiPitch, hSrcVObject, iDestX, iDestY,
                                    usIndex, s16BPPColor, fDoOutline);
  }

  // Now we have the video object and surface, call the VO blitter function

  UnLockVideoSurface(uiDestVSurface);
  return (TRUE);
}

BOOLEAN BltVideoObjectOutline(UINT32 uiDestVSurface, struct VObject *hSrcVObject, UINT16 usIndex,
                              INT32 iDestX, INT32 iDestY, INT16 s16BPPColor, BOOLEAN fDoOutline) {
  UINT16 *pBuffer;
  UINT32 uiPitch;
  // Lock video surface
  pBuffer = (UINT16 *)LockVideoSurface(uiDestVSurface, &uiPitch);

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

  // Now we have the video object and surface, call the VO blitter function

  UnLockVideoSurface(uiDestVSurface);
  return (TRUE);
}

BOOLEAN BltVideoObjectOutlineShadowFromIndex(UINT32 uiDestVSurface, UINT32 uiSrcVObject,
                                             UINT16 usIndex, INT32 iDestX, INT32 iDestY) {
  UINT16 *pBuffer;
  UINT32 uiPitch;
  struct VObject *hSrcVObject;

  // Lock video surface
  pBuffer = (UINT16 *)LockVideoSurface(uiDestVSurface, &uiPitch);

  if (pBuffer == NULL) {
    return (FALSE);
  }

  // Get video object
  CHECKF(GetVideoObject(&hSrcVObject, uiSrcVObject));

  if (BltIsClipped(hSrcVObject, iDestX, iDestY, usIndex, &ClippingRect)) {
    Blt8BPPDataTo16BPPBufferOutlineShadowClip((UINT16 *)pBuffer, uiPitch, hSrcVObject, iDestX,
                                              iDestY, usIndex, &ClippingRect);
  } else {
    Blt8BPPDataTo16BPPBufferOutlineShadow((UINT16 *)pBuffer, uiPitch, hSrcVObject, iDestX, iDestY,
                                          usIndex);
  }

  // Now we have the video object and surface, call the VO blitter function

  UnLockVideoSurface(uiDestVSurface);
  return (TRUE);
}
