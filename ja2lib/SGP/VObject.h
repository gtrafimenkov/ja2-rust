#ifndef __VOBJECT_H
#define __VOBJECT_H

#include "SGP/Container.h"
#include "SGP/HImage.h"
#include "SGP/Types.h"

// ************************************************************************************
//
// Video Object SGP Module
//
// ************************************************************************************

// Defines for blitting
#define VO_BLT_SRCTRANSPARENCY 0x000000002
#define VO_BLT_SHADOW 0x000000200

// Defines for struct VObject* limits
#define HVOBJECT_SHADE_TABLES 48

#define HVOBJECT_GLOW_GREEN 0
#define HVOBJECT_GLOW_BLUE 1
#define HVOBJECT_GLOW_YELLOW 2
#define HVOBJECT_GLOW_RED 3

// Effects structure for specialized blitting
typedef struct {
  UINT32 uiShadowLevel;
  SGPRect ClipRect;
} blt_fx;

// Z-buffer info structure for properly assigning Z values
typedef struct {
  INT8 bInitialZChange;      // difference in Z value between the leftmost and base strips
  UINT8 ubFirstZStripWidth;  // # of pixels in the leftmost strip
  UINT8 ubNumberOfZChanges;  // number of strips (after the first)
  INT8 *pbZChange;           // change to the Z value in each strip (after the first)
} ZStripInfo;

typedef struct {
  UINT16 *p16BPPData;
  UINT16 usRegionIndex;
  UINT8 ubShadeLevel;
  UINT16 usWidth;
  UINT16 usHeight;
  INT16 sOffsetX;
  INT16 sOffsetY;
} SixteenBPPObjectInfo;

// This definition mimics what is found in WINDOWS.H ( for Direct Draw compatiblity )
// From RGB to COLORVAL
#define FROMRGB(r, g, b) \
  ((UINT32)(((UINT8)(r) | ((UINT16)(g) << 8)) | (((UINT32)(UINT8)(b)) << 16)))

// Video object creation flags
// Used in the VOBJECT_DESC structure to describe creation flags

#define VOBJECT_CREATE_FROMFILE 0x00000040    // Creates a video object from a file ( using HIMAGE )
#define VOBJECT_CREATE_FROMHIMAGE 0x00000080  // Creates a video object from a pre-loaded hImage

// VOBJECT FLAGS
#define VOBJECT_FLAG_SHADETABLE_SHARED 0x00000100

// This structure is a video object.
// The video object contains different data based on it's type, compressed or not
struct VObject {
  UINT32 fFlags;                          // Special flags
  UINT32 uiSizePixData;                   // ETRLE data size
  struct SGPPaletteEntry *pPaletteEntry;  // 8BPP Palette
  COLORVAL TransparentColor;              // Defaults to 0,0,0
  UINT16 *p16BPPPalette;                  // A 16BPP palette used for 8->16 blits

  PTR pPixData;               // ETRLE pixel data
  ETRLEObject *pETRLEObject;  // Object offset data etc
  SixteenBPPObjectInfo *p16BPPObject;
  UINT16 *pShades[HVOBJECT_SHADE_TABLES];  // Shading tables
  UINT16 *pShadeCurrent;
  UINT16 *pGlow;              // glow highlight table
  UINT8 *pShade8;             // 8-bit shading index table
  UINT8 *pGlow8;              // 8-bit glow table
  ZStripInfo **ppZStripInfo;  // Z-value strip info arrays

  UINT16 usNumberOf16BPPObjects;
  UINT16 usNumberOfObjects;  // Total number of objects
  UINT8 ubBitDepth;          // BPP

  // Reserved for added room and 32-byte boundaries
  BYTE bReserved[1];
};

// This structure describes the creation parameters for a Video Object
typedef struct {
  UINT32 fCreateFlags;  // Specifies creation flags like from file or not
  union {
    struct {
      SGPFILENAME ImageFile;  // Filename of image data to use
    };
    struct {
      HIMAGE hImage;
    };
  };
} VOBJECT_DESC;

// **********************************************************************************
//
// Video Object Manager Functions
//
// **********************************************************************************

// Creates a list to contain video objects
BOOLEAN InitializeVideoObjectManager();

// Deletes any video object placed into list
BOOLEAN ShutdownVideoObjectManager();

// Creates and adds a video object to list
BOOLEAN AddVideoObject(VOBJECT_DESC *VObjectDesc, UINT32 *uiIndex);

// Removes a video object
BOOLEAN DeleteVideoObjectFromIndex(UINT32 uiVObject);

UINT16 CreateObjectPaletteTables(struct VObject *pObj, UINT32 uiType);

// Returns a struct VObject* for the specified index
BOOLEAN GetVideoObject(struct VObject **hVObject, UINT32 uiIndex);

// Blits a video object to another video object
BOOLEAN BltVideoObject(UINT32 uiDestVSurface, struct VObject *hVSrcObject, UINT16 usRegionIndex,
                       INT32 iDestX, INT32 iDestY, UINT32 fBltFlags, blt_fx *pBltFx);

BOOLEAN BltVideoObjectFromIndex(UINT32 uiDestVSurface, UINT32 uiSrcVObject, UINT16 usRegionIndex,
                                INT32 iDestX, INT32 iDestY, UINT32 fBltFlags, blt_fx *pBltFx);

// **********************************************************************************
//
// Video Object manipulation functions
//
// **********************************************************************************

struct VObject *CreateVObjectFromFile(const char *path);
struct VObject *CreateVObjectFromHImage(HIMAGE hImage);

// Deletes all data
BOOLEAN DeleteVideoObject(struct VObject *hVObject);

// Deletes the 16-bit palette tables
BOOLEAN DestroyObjectPaletteTables(struct VObject *hVObject);

// Sets the current object shade table
UINT16 SetObjectShade(struct VObject *pObj, UINT32 uiShade);

// Sets the current object shade table using a vobject handle
UINT16 SetObjectHandleShade(UINT32 uiHandle, UINT32 uiShade);

// Retrieves an struct VObject* pixel value
BOOLEAN GetETRLEPixelValue(UINT8 *pDest, struct VObject *hVObject, UINT16 usETLREIndex, UINT16 usX,
                           UINT16 usY);

// ****************************************************************************
//
// Blt Functions
//
// ****************************************************************************

struct VObject *GetPrimaryVideoObject();
struct VObject *GetBackBufferVideoObject();

BOOLEAN GetVideoObjectETRLEProperties(struct VObject *hVObject, ETRLEObject *pETRLEObject,
                                      UINT16 usIndex);
BOOLEAN GetVideoObjectETRLEPropertiesFromIndex(UINT32 uiVideoObject, ETRLEObject *pETRLEObject,
                                               UINT16 usIndex);
BOOLEAN GetVideoObjectETRLESubregionProperties(UINT32 uiVideoObject, UINT16 usIndex,
                                               UINT16 *pusWidth, UINT16 *pusHeight);

BOOLEAN BltVideoObjectOutlineFromIndex(UINT32 uiDestVSurface, UINT32 uiSrcVObject, UINT16 usIndex,
                                       INT32 iDestX, INT32 iDestY, INT16 s16BPPColor,
                                       BOOLEAN fDoOutline);
BOOLEAN BltVideoObjectOutline(UINT32 uiDestVSurface, struct VObject *hSrcVObject, UINT16 usIndex,
                              INT32 iDestX, INT32 iDestY, INT16 s16BPPColor, BOOLEAN fDoOutline);
BOOLEAN BltVideoObjectOutlineShadowFromIndex(UINT32 uiDestVSurface, UINT32 uiSrcVObject,
                                             UINT16 usIndex, INT32 iDestX, INT32 iDestY);

#endif
