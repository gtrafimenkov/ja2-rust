#ifndef __VSURFACE_H
#define __VSURFACE_H

#include "SGP/Container.h"
#include "SGP/HImage.h"
#include "SGP/Types.h"

struct VObject;
struct VSurface;

typedef uint32_t VSurfID;

extern struct VSurface *vsBB;
extern struct VSurface *vsFB;
extern struct VSurface *vsMouseCursor;
extern struct VSurface *vsMouseBuffer;
extern struct VSurface *vsPrimary;
extern struct VSurface *vsExtraBuffer;
extern struct VSurface *vsSaveBuffer;

//
// Defines for special video object handles given to blit function
//

#define BACKBUFFER 0xFFFFFFF1
#define FRAME_BUFFER 0xFFFFFFF2

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
// Video Surface creation flags
// Used in the VSurface_DESC structure to describe creation flags
//

#define VSURFACE_CREATE_DEFAULT \
  0x00000020  // Creates and empty Surface of given width, height and BPP
#define VSURFACE_CREATE_FROMFILE \
  0x00000040  // Creates a video Surface from a file ( using struct Image* )

uint16_t GetVSurfaceHeight(const struct VSurface *vs);
uint16_t GetVSurfaceWidth(const struct VSurface *vs);
uint16_t *GetVSurface16BPPPalette(struct VSurface *vs);
void SetVSurface16BPPPalette(struct VSurface *vs, uint16_t *palette);

//
// This structure describes the creation parameters for a Video Surface
//

typedef struct {
  uint32_t fCreateFlags;  // Specifies creation flags like from file or not
  SGPFILENAME ImageFile;  // Filename of image data to use
  uint16_t usWidth;       // Width, ignored if given from file
  uint16_t usHeight;      // Height, ignored if given from file
  uint8_t ubBitDepth;     // BPP, ignored if given from file
} VSURFACE_DESC;

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// Video Surface Manager Functions
//
///////////////////////////////////////////////////////////////////////////////////////////////////

// Creates a list to contain video Surfaces
BOOLEAN InitializeVideoSurfaceManager();

// Deletes any video Surface placed into list
BOOLEAN ShutdownVideoSurfaceManager();

// Creates and adds a video Surface to list
BOOLEAN AddVideoSurface(VSURFACE_DESC *VSurfaceDesc, VSurfID *uiIndex);

// Returns a HVSurface for the specified index
BOOLEAN GetVideoSurface(struct VSurface **hVSurface, VSurfID uiIndex);
struct VSurface *GetVSByID(VSurfID id);

BOOLEAN VSurfaceColorFill(struct VSurface *dest, int32_t x1, int32_t y1, int32_t x2, int32_t y2,
                          uint16_t Color16BPP);

// Sets transparency
BOOLEAN SetVideoSurfaceTransparency(uint32_t uiIndex, COLORVAL TransColor);

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// Video Surface manipulation functions
//
///////////////////////////////////////////////////////////////////////////////////////////////////

// Gets the RGB palette entry values
BOOLEAN GetVSurfacePaletteEntries(struct VSurface *hVSurface, struct SGPPaletteEntry *pPalette);

// Set data from struct Image*.
BOOLEAN SetVideoSurfaceDataFromHImage(struct VSurface *hVSurface, struct Image *hImage,
                                      uint16_t usX, uint16_t usY, SGPRect *pSrcRect);

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

BOOLEAN BltVideoSurface(struct VSurface *hDestVSurface, struct VSurface *hSrcVSurface,
                        int32_t iDestX, int32_t iDestY, int32_t fBltFlags, struct BltOpts *pBltFx);

BOOLEAN ShadowVideoSurfaceRect(struct VSurface *dest, int32_t X1, int32_t Y1, int32_t X2,
                               int32_t Y2);

// If the Dest Rect and the source rect are not the same size, the source surface will be either
// enlraged or shunk.
BOOLEAN BltStretchVideoSurface(struct VSurface *dest, struct VSurface *src, int32_t iDestX,
                               int32_t iDestY, uint32_t fBltFlags, SGPRect *SrcRect,
                               SGPRect *DestRect);

BOOLEAN ShadowVideoSurfaceRectUsingLowPercentTable(struct VSurface *dest, int32_t X1, int32_t Y1,
                                                   int32_t X2, int32_t Y2);

// The following structure is used to define a region of the video Surface
// These regions are stored via a HLIST
typedef struct {
  SGPRect RegionCoords;  // Rectangle describing coordinates of region
  SGPPoint Origin;       // Origin used for hot spots, etc
  uint8_t ubHitMask;     // Byte flags for hit detection
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
BYTE *VSurfaceLockOld(struct VSurface *vs, uint32_t *pitch);

struct VSurface *CreateVideoSurface(uint16_t width, uint16_t height, uint8_t bitDepth);
struct VSurface *CreateVideoSurfaceFromFile(const char *path);
struct VSurface *VSurfaceAdd(uint16_t width, uint16_t height, uint8_t bitDepth, VSurfID *puiIndex);

// Global variables for video objects
extern uint32_t guiRIGHTPANEL;
extern uint32_t guiSAVEBUFFER;

BOOLEAN InitializeSystemVideoObjects();
BOOLEAN InitializeGameVideoObjects();

#endif
