#ifndef __VOBJECT_H
#define __VOBJECT_H

#include "SGP/Container.h"
#include "SGP/HImage.h"
#include "SGP/Types.h"
#include "rust_geometry.h"

struct VSurface;
struct VObject;
typedef uint32_t VSurfID;

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
  uint32_t uiShadowLevel;
  struct GRect ClipRect;
} blt_fx;

// Z-buffer info structure for properly assigning Z values
typedef struct {
  int8_t bInitialZChange;      // difference in Z value between the leftmost and base strips
  uint8_t ubFirstZStripWidth;  // # of pixels in the leftmost strip
  uint8_t ubNumberOfZChanges;  // number of strips (after the first)
  int8_t *pbZChange;           // change to the Z value in each strip (after the first)
} ZStripInfo;

// This definition mimics what is found in WINDOWS.H ( for Direct Draw compatiblity )
// From RGB to COLORVAL
#define FROMRGB(r, g, b) \
  ((uint32_t)(((uint8_t)(r) | ((uint16_t)(g) << 8)) | (((uint32_t)(uint8_t)(b)) << 16)))

// **********************************************************************************
//
// Video Object Manager Functions
//
// **********************************************************************************

// Creates a list to contain video objects
BOOLEAN InitializeVideoObjectManager();

// Deletes any video object placed into list
BOOLEAN ShutdownVideoObjectManager();

BOOLEAN AddVObjectFromFile(const char *path, uint32_t *puiIndex);

// Removes a video object
BOOLEAN DeleteVideoObjectFromIndex(uint32_t uiVObject);

uint16_t CreateObjectPaletteTables(struct VObject *pObj, uint32_t uiType);

// Returns a struct VObject* for the specified index
BOOLEAN GetVideoObject(struct VObject **hVObject, uint32_t uiIndex);

// **********************************************************************************
//
// Video Object manipulation functions
//
// **********************************************************************************

struct VObject *CreateVObjectFromImage(struct Image *hImage);

// Deletes all data
BOOLEAN DeleteVideoObject(struct VObject *hVObject);

// Deletes the 16-bit palette tables
BOOLEAN DestroyObjectPaletteTables(struct VObject *hVObject);

// Sets the current object shade table
uint16_t SetObjectShade(struct VObject *pObj, uint32_t uiShade);

// Sets the current object shade table using a vobject handle
uint16_t SetObjectHandleShade(uint32_t uiHandle, uint32_t uiShade);

// Retrieves an struct VObject* pixel value
BOOLEAN GetETRLEPixelValue(uint8_t *pDest, struct VObject *hVObject, uint16_t usETLREIndex,
                           uint16_t usX, uint16_t usY);

// ****************************************************************************
//
// Blt Functions
//
// ****************************************************************************

struct VObject *GetPrimaryVideoObject();
struct VObject *GetBackBufferVideoObject();

BOOLEAN GetVideoObjectETRLEProperties(struct VObject *hVObject, struct Subimage *subimages,
                                      uint16_t usIndex);
BOOLEAN GetVideoObjectETRLEPropertiesFromIndex(uint32_t uiVideoObject, struct Subimage *subimages,
                                               uint16_t usIndex);
BOOLEAN GetVideoObjectETRLESubregionProperties(uint32_t uiVideoObject, uint16_t usIndex,
                                               uint16_t *pusWidth, uint16_t *pusHeight);

BOOLEAN BltVideoObjectOutlineFromIndex(struct VSurface *dest, uint32_t uiSrcVObject,
                                       uint16_t usIndex, int32_t iDestX, int32_t iDestY,
                                       int16_t s16BPPColor, BOOLEAN fDoOutline);
BOOLEAN BltVideoObjectOutline(struct VSurface *dest, struct VObject *hSrcVObject, uint16_t usIndex,
                              int32_t iDestX, int32_t iDestY, int16_t s16BPPColor,
                              BOOLEAN fDoOutline);
BOOLEAN BltVideoObjectOutlineShadowFromIndex(struct VSurface *dest, uint32_t uiSrcVObject,
                                             uint16_t usIndex, int32_t iDestX, int32_t iDestY);

void VObjectUpdateShade(struct VObject *obj, uint8_t shade_num, uint32_t rscale, uint32_t gscale,
                        uint32_t bscale, BOOLEAN mono);

// ****************************************************************************
//
//
//
// ****************************************************************************

struct VObject *LoadVObjectFromFile(const char *path);
bool BltVObject(struct VSurface *dest, struct VObject *vobj, uint16_t regionIndex, int32_t x,
                int32_t y);
bool BltVObjectFromIndex(struct VSurface *dest, uint32_t uiSrcVObject, uint16_t usRegionIndex,
                         int32_t iDestX, int32_t iDestY);

#endif
