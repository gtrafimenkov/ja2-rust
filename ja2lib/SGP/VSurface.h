#ifndef __VSURFACE_H
#define __VSURFACE_H

#include "SGP/Container.h"
#include "SGP/HImage.h"
#include "SGP/Types.h"

struct VObject;
typedef uint32_t VSurfID;

//
// Defines for special video object handles given to blit function
//

#define BACKBUFFER 0xFFFFFFF1
#define FRAME_BUFFER 0xFFFFFFF2
#define MOUSE_BUFFER 0xFFFFFFF3

//
// Defines for blitting
//

#define VS_BLT_USECOLORKEY 0x000000002
#define VS_BLT_FAST 0x000000004
#define VS_BLT_SRCSUBRECT 0x000000040

//
// Effects structure for specialized blitting
//

struct BltOpts {
  COLORVAL ColorFill;  // Used for fill effect
  SGPRect SrcRect;     // Given SRC subrect instead of srcregion
  SGPRect FillRect;    // Given SRC subrect instead of srcregion
};

//
// Video Surface Flags
// Used to describe the memory usage of a video Surface
//

#define VSURFACE_VIDEO_MEM_USAGE \
  0x00000002  // Will force surface into video memory and will fail if it can't
#define VSURFACE_SYSTEM_MEM_USAGE \
  0x00000004  // Will force surface into system memory and will fail if it can't

//
// Video Surface creation flags
// Used in the VSurface_DESC structure to describe creation flags
//

#define VSURFACE_CREATE_DEFAULT \
  0x00000020  // Creates and empty Surface of given width, height and BPP
#define VSURFACE_CREATE_FROMFILE \
  0x00000040  // Creates a video Surface from a file ( using struct Image* )

//
// This structure is a video Surface. Contains a HLIST of regions
//

struct VSurface;

UINT16 GetVSurfaceHeight(const struct VSurface *vs);
UINT16 GetVSurfaceWidth(const struct VSurface *vs);
UINT16 *GetVSurface16BPPPalette(struct VSurface *vs);
void SetVSurface16BPPPalette(struct VSurface *vs, UINT16 *palette);

//
// This structure describes the creation parameters for a Video Surface
//

typedef struct {
  UINT32 fCreateFlags;    // Specifies creation flags like from file or not
  SGPFILENAME ImageFile;  // Filename of image data to use
  UINT16 usWidth;         // Width, ignored if given from file
  UINT16 usHeight;        // Height, ignored if given from file
  UINT8 ubBitDepth;       // BPP, ignored if given from file
} VSURFACE_DESC;

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// Video Surface Manager Functions
//
///////////////////////////////////////////////////////////////////////////////////////////////////

extern INT32 giMemUsedInSurfaces;

// Creates a list to contain video Surfaces
BOOLEAN InitializeVideoSurfaceManager();

// Deletes any video Surface placed into list
BOOLEAN ShutdownVideoSurfaceManager();

// Restores all video Surfaces in list
BOOLEAN RestoreVideoSurfaces();

// Creates and adds a video Surface to list
BOOLEAN AddVideoSurface(VSURFACE_DESC *VSurfaceDesc, VSurfID *uiIndex);

// Returns a HVSurface for the specified index
BOOLEAN GetVideoSurface(struct VSurface **hVSurface, VSurfID uiIndex);

BYTE *LockVideoSurface(VSurfID uiVSurface, UINT32 *uiPitch);
void UnLockVideoSurface(VSurfID uiVSurface);

// Blits a video Surface to another video Surface
BOOLEAN BltVideoSurface(VSurfID destSurface, VSurfID srcSurface, INT32 iDestX, INT32 iDestY,
                        UINT32 fBltFlags, struct BltOpts *pBltFx);

BOOLEAN ColorFillVideoSurfaceArea(VSurfID destSurface, INT32 iDestX1, INT32 iDestY1, INT32 iDestX2,
                                  INT32 iDestY2, UINT16 Color16BPP);

BOOLEAN ImageFillVideoSurfaceArea(VSurfID destSurface, INT32 iDestX1, INT32 iDestY1, INT32 iDestX2,
                                  INT32 iDestY2, struct VObject *BkgrndImg, UINT16 Index, INT16 Ox,
                                  INT16 Oy);

// Sets transparency
BOOLEAN SetVideoSurfaceTransparency(UINT32 uiIndex, COLORVAL TransColor);

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// Video Surface manipulation functions
//
///////////////////////////////////////////////////////////////////////////////////////////////////

// Created from a VSurface_DESC structure. Can be from a file via struct Image* or empty.
struct VSurface *CreateVideoSurface(VSURFACE_DESC *VSurfaceDesc);

// Gets the RGB palette entry values
BOOLEAN GetVSurfacePaletteEntries(struct VSurface *hVSurface, struct SGPPaletteEntry *pPalette);

// Set data from struct Image*.
BOOLEAN SetVideoSurfaceDataFromHImage(struct VSurface *hVSurface, struct Image *hImage, UINT16 usX,
                                      UINT16 usY, SGPRect *pSrcRect);

// Sets Transparency color into HVSurface and the underlying DD surface
BOOLEAN SetVideoSurfaceTransparencyColor(struct VSurface *hVSurface, COLORVAL TransColor);

// Sets HVSurface palette, creates if nessessary. Also sets 16BPP palette
BOOLEAN SetVideoSurfacePalette(struct VSurface *hVSurface, struct SGPPaletteEntry *pSrcPalette);

// Deletes all data, including palettes, regions, DD Surfaces
BOOLEAN DeleteVideoSurface(struct VSurface *hVSurface);
BOOLEAN DeleteVideoSurfaceFromIndex(VSurfID uiIndex);

/////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Blt Functions
//
///////////////////////////////////////////////////////////////////////////////////////////////////

// These blitting functions more-or less encapsolate all of the functionality of DirectDraw
// Blitting, giving an API layer for portability.

BOOLEAN BltVideoSurfaceToVideoSurface(struct VSurface *hDestVSurface, struct VSurface *hSrcVSurface,
                                      INT32 iDestX, INT32 iDestY, INT32 fBltFlags,
                                      struct BltOpts *pBltFx);

BOOLEAN ShadowVideoSurfaceRect(VSurfID destSurface, INT32 X1, INT32 Y1, INT32 X2, INT32 Y2);
BOOLEAN ShadowVideoSurfaceImage(VSurfID destSurface, struct VObject *hImageHandle, INT32 iPosX,
                                INT32 iPosY);

// If the Dest Rect and the source rect are not the same size, the source surface will be either
// enlraged or shunk.
BOOLEAN BltStretchVideoSurface(VSurfID destSurface, VSurfID srcSurface, INT32 iDestX, INT32 iDestY,
                               UINT32 fBltFlags, SGPRect *SrcRect, SGPRect *DestRect);

BOOLEAN ShadowVideoSurfaceRectUsingLowPercentTable(VSurfID destSurface, INT32 X1, INT32 Y1,
                                                   INT32 X2, INT32 Y2);

// The following structure is used to define a region of the video Surface
// These regions are stored via a HLIST
typedef struct {
  SGPRect RegionCoords;  // Rectangle describing coordinates of region
  SGPPoint Origin;       // Origin used for hot spots, etc
  UINT8 ubHitMask;       // Byte flags for hit detection
} VSURFACE_REGION;

void SetClippingRect(SGPRect *clip);

// Allocate a new empty instance of VSurface
struct VSurface *VSurfaceNew();

// This information can be used to write into video surface buffer.
struct BufferLockInfo {
  uint8_t *dest;   // address to write
  uint32_t pitch;  // size of one line of pixels in bytes
};

struct BufferLockInfo VSurfaceLock(struct VSurface *vs);
void VSurfaceUnlock(struct VSurface *vs);

#endif
