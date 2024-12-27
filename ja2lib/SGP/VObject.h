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
#define VO_BLT_DESTTRANSPARENCY 0x000000120
#define VO_BLT_SHADOW 0x000000200
#define VO_BLT_TRANSSHADOW 0x000000003
#define VO_BLT_UNCOMPRESSED 0x000004000
#define VO_BLT_CLIP 0x000000001
#define VO_BLT_MIRROR_Y 0x000001000  // must be the same as VS_BLT_MIRROR_Y for Wiz!!!

// Defines for struct VObject* limits
#define HVOBJECT_SHADE_TABLES 48

#define HVOBJECT_GLOW_GREEN 0
#define HVOBJECT_GLOW_BLUE 1
#define HVOBJECT_GLOW_YELLOW 2
#define HVOBJECT_GLOW_RED 3

// Effects structure for specialized blitting
typedef struct {
  uint32_t uiShadowLevel;
  SGPRect ClipRect;

} blt_fx;

// Z-buffer info structure for properly assigning Z values
typedef struct {
  INT8 bInitialZChange;        // difference in Z value between the leftmost and base strips
  uint8_t ubFirstZStripWidth;  // # of pixels in the leftmost strip
  uint8_t ubNumberOfZChanges;  // number of strips (after the first)
  INT8 *pbZChange;             // change to the Z value in each strip (after the first)
} ZStripInfo;

typedef struct {
  uint16_t *p16BPPData;
  uint16_t usRegionIndex;
  uint8_t ubShadeLevel;
  uint16_t usWidth;
  uint16_t usHeight;
  INT16 sOffsetX;
  INT16 sOffsetY;
} SixteenBPPObjectInfo;

// This definition mimics what is found in WINDOWS.H ( for Direct Draw compatiblity )
// From RGB to COLORVAL
#define FROMRGB(r, g, b) \
  ((uint32_t)(((uint8_t)(r) | ((uint16_t)(g) << 8)) | (((uint32_t)(uint8_t)(b)) << 16)))

// Video object creation flags
// Used in the VOBJECT_DESC structure to describe creation flags

#define VOBJECT_CREATE_DEFAULT \
  0x00000020  // Creates and empty object of given width, height and BPP
#define VOBJECT_CREATE_FROMFILE 0x00000040    // Creates a video object from a file ( using HIMAGE )
#define VOBJECT_CREATE_FROMHIMAGE 0x00000080  // Creates a video object from a pre-loaded hImage

// VOBJECT FLAGS
#define VOBJECT_FLAG_SHADETABLE_SHARED 0x00000100

// This structure is a video object.
// The video object contains different data based on it's type, compressed or not
struct VObject {
  uint32_t fFlags;                        // Special flags
  uint32_t uiSizePixData;                 // ETRLE data size
  struct SGPPaletteEntry *pPaletteEntry;  // 8BPP Palette
  COLORVAL TransparentColor;              // Defaults to 0,0,0
  uint16_t *p16BPPPalette;                // A 16BPP palette used for 8->16 blits

  PTR pPixData;               // ETRLE pixel data
  ETRLEObject *pETRLEObject;  // Object offset data etc
  SixteenBPPObjectInfo *p16BPPObject;
  uint16_t *pShades[HVOBJECT_SHADE_TABLES];  // Shading tables
  uint16_t *pShadeCurrent;
  uint16_t *pGlow;            // glow highlight table
  uint8_t *pShade8;           // 8-bit shading index table
  uint8_t *pGlow8;            // 8-bit glow table
  ZStripInfo **ppZStripInfo;  // Z-value strip info arrays

  uint16_t usNumberOf16BPPObjects;
  uint16_t usNumberOfObjects;  // Total number of objects
  uint8_t ubBitDepth;          // BPP

  // Reserved for added room and 32-byte boundaries
  BYTE bReserved[1];
};

// This structure describes the creation parameters for a Video Object
typedef struct {
  uint32_t fCreateFlags;  // Specifies creation flags like from file or not
  union {
    struct {
      SGPFILENAME ImageFile;  // Filename of image data to use
    };
    struct {
      HIMAGE hImage;
    };
  };
  uint8_t ubBitDepth;  // BPP, ignored if given from file
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
#ifdef SGP_VIDEO_DEBUGGING
void PerformVideoInfoDumpIntoFile(CHAR8 *filename, BOOLEAN fAppend);
void DumpVObjectInfoIntoFile(CHAR8 *filename, BOOLEAN fAppend);
BOOLEAN _AddAndRecordVObject(VOBJECT_DESC *VObjectDesc, uint32_t *uiIndex, uint32_t uiLineNum,
                             CHAR8 *pSourceFile);
#define AddVideoObject(a, b) _AddAndRecordVObject(a, (uint32_t *)b, __LINE__, __FILE__)
#else
#define AddVideoObject(a, b) AddStandardVideoObject(a, (uint32_t *)b)
#endif

BOOLEAN AddStandardVideoObject(VOBJECT_DESC *VObjectDesc, uint32_t *uiIndex);

// Removes a video object
BOOLEAN DeleteVideoObjectFromIndex(uint32_t uiVObject);

uint16_t CreateObjectPaletteTables(struct VObject *pObj, uint32_t uiType);

// Returns a struct VObject* for the specified index
BOOLEAN GetVideoObject(struct VObject **hVObject, uint32_t uiIndex);

// Blits a video object to another video object
BOOLEAN BltVideoObject(uint32_t uiDestVSurface, struct VObject *hVSrcObject, uint16_t usRegionIndex,
                       INT32 iDestX, INT32 iDestY, uint32_t fBltFlags, blt_fx *pBltFx);

BOOLEAN BltVideoObjectFromIndex(uint32_t uiDestVSurface, uint32_t uiSrcVObject,
                                uint16_t usRegionIndex, INT32 iDestX, INT32 iDestY,
                                uint32_t fBltFlags, blt_fx *pBltFx);

// Sets transparency
BOOLEAN SetVideoObjectTransparency(uint32_t uiIndex, COLORVAL TransColor);

// **********************************************************************************
//
// Video Object manipulation functions
//
// **********************************************************************************

// Created from a VOBJECT_DESC structure. Can be from a file via HIMAGE or empty.
struct VObject *CreateVideoObject(VOBJECT_DESC *VObjectDesc);

// Sets Transparency color into struct VObject*
BOOLEAN SetVideoObjectTransparencyColor(struct VObject *hVObject, COLORVAL TransColor);

// Sets struct VObject* palette, creates if nessessary. Also sets 16BPP palette
BOOLEAN SetVideoObjectPalette(struct VObject *hVObject, struct SGPPaletteEntry *pSrcPalette);

// Deletes all data
BOOLEAN DeleteVideoObject(struct VObject *hVObject);

// Deletes the 16-bit palette tables
BOOLEAN DestroyObjectPaletteTables(struct VObject *hVObject);

// Sets the current object shade table
uint16_t SetObjectShade(struct VObject *pObj, uint32_t uiShade);

// Sets the current object shade table using a vobject handle
uint16_t SetObjectHandleShade(uint32_t uiHandle, uint32_t uiShade);

// Fills a rectangular area of an object with a color
uint16_t FillObjectRect(uint32_t iObj, INT32 x1, INT32 y1, INT32 x2, INT32 y2, COLORVAL color32);

// Retrieves an struct VObject* pixel value
BOOLEAN GetETRLEPixelValue(uint8_t *pDest, struct VObject *hVObject, uint16_t usETLREIndex,
                           uint16_t usX, uint16_t usY);

// ****************************************************************************
//
// Globals
//
// ****************************************************************************
extern HLIST ghVideoObjects;

// ****************************************************************************
//
// Macros
//
// ****************************************************************************

extern BOOLEAN gfVideoObjectsInit;
#define VideoObjectsInitialized() (gfVideoObjectsInit)

// ****************************************************************************
//
// Blt Functions
//
// ****************************************************************************

// These blitting functions more-or less encapsolate all of the functionality of DirectDraw
// Blitting, giving an API layer for portability.

BOOLEAN BltVideoObjectToBuffer(uint16_t *pBuffer, uint32_t uiDestPitchBYTES,
                               struct VObject *hSrcVObject, uint16_t usIndex, INT32 iDestX,
                               INT32 iDestY, INT32 fBltFlags, blt_fx *pBltFx);

struct VObject *GetPrimaryVideoObject();
struct VObject *GetBackBufferVideoObject();

BOOLEAN GetVideoObjectETRLEProperties(struct VObject *hVObject, ETRLEObject *pETRLEObject,
                                      uint16_t usIndex);
BOOLEAN GetVideoObjectETRLEPropertiesFromIndex(uint32_t uiVideoObject, ETRLEObject *pETRLEObject,
                                               uint16_t usIndex);
BOOLEAN GetVideoObjectETRLESubregionProperties(uint32_t uiVideoObject, uint16_t usIndex,
                                               uint16_t *pusWidth, uint16_t *pusHeight);

BOOLEAN SetVideoObjectPalette8BPP(INT32 uiVideoObject, struct SGPPaletteEntry *pPal8);
BOOLEAN SetVideoObjectPalette16BPP(INT32 uiVideoObject, uint16_t *pPal16);
BOOLEAN GetVideoObjectPalette16BPP(INT32 uiVideoObject, uint16_t **ppPal16);
BOOLEAN CopyVideoObjectPalette16BPP(INT32 uiVideoObject, uint16_t *ppPal16);

BOOLEAN ConvertVObjectRegionTo16BPP(struct VObject *hVObject, uint16_t usRegionIndex,
                                    uint8_t ubShadeLevel);
BOOLEAN CheckFor16BPPRegion(struct VObject *hVObject, uint16_t usRegionIndex, uint8_t ubShadeLevel,
                            uint16_t *pusIndex);

BOOLEAN BltVideoObjectOutlineFromIndex(uint32_t uiDestVSurface, uint32_t uiSrcVObject,
                                       uint16_t usIndex, INT32 iDestX, INT32 iDestY,
                                       INT16 s16BPPColor, BOOLEAN fDoOutline);
BOOLEAN BltVideoObjectOutline(uint32_t uiDestVSurface, struct VObject *hSrcVObject,
                              uint16_t usIndex, INT32 iDestX, INT32 iDestY, INT16 s16BPPColor,
                              BOOLEAN fDoOutline);
BOOLEAN BltVideoObjectOutlineShadowFromIndex(uint32_t uiDestVSurface, uint32_t uiSrcVObject,
                                             uint16_t usIndex, INT32 iDestX, INT32 iDestY);
BOOLEAN BltVideoObjectOutlineShadow(uint32_t uiDestVSurface, struct VObject *hSrcVObject,
                                    uint16_t usIndex, INT32 iDestX, INT32 iDestY);
BOOLEAN PixelateVideoObjectRect(uint32_t uiDestVSurface, INT32 X1, INT32 Y1, INT32 X2, INT32 Y2);

#endif
