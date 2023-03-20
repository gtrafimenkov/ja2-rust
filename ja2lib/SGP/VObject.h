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

// This definition mimics what is found in WINDOWS.H ( for Direct Draw compatiblity )
// From RGB to COLORVAL
#define FROMRGB(r, g, b) \
  ((UINT32)(((UINT8)(r) | ((UINT16)(g) << 8)) | (((UINT32)(UINT8)(b)) << 16)))

// Video object creation flags
// Used in the VOBJECT_DESC structure to describe creation flags

// VOBJECT FLAGS
#define VOBJECT_FLAG_SHADETABLE_SHARED 0x00000100

// This structure describes the creation parameters for a Video Object
typedef struct {
  SGPFILENAME ImageFile;  // Filename of image data to use
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

BOOLEAN AddVObjectFromFile(const char *path, UINT32 *puiIndex);
BOOLEAN AddVObjectFromHImage(struct Image *hImage, UINT32 *puiIndex);

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
struct VObject *CreateVObjectFromHImage(struct Image *hImage);

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

UINT16 *VObjectUpdateShade(struct VObject *obj, UINT32 rscale, UINT32 gscale, UINT32 bscale,
                           BOOLEAN mono);

#endif
