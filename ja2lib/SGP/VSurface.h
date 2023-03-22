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
extern struct VSurface *vsMouseCursorOriginal;
extern struct VSurface *vsPrimary;
extern struct VSurface *vsExtraBuffer;
extern struct VSurface *vsSaveBuffer;

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
// Video Surface creation flags
// Used in the VSurface_DESC structure to describe creation flags
//

#define VSURFACE_CREATE_DEFAULT \
  0x00000020  // Creates and empty Surface of given width, height and BPP
#define VSURFACE_CREATE_FROMFILE \
  0x00000040  // Creates a video Surface from a file ( using struct Image* )

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

// Creates a list to contain video Surfaces
BOOLEAN InitializeVideoSurfaceManager();

// Deletes any video Surface placed into list
BOOLEAN ShutdownVideoSurfaceManager();

// Creates and adds a video Surface to list
BOOLEAN AddVideoSurface(VSURFACE_DESC *VSurfaceDesc, VSurfID *uiIndex);
// BOOLEAN AddVideoSurfaceEx(VSURFACE_DESC *VSurfaceDesc, VSurfID *uiIndex);

// Returns a HVSurface for the specified index
BOOLEAN GetVideoSurface(struct VSurface **hVSurface, VSurfID uiIndex);

// Blits a video Surface to another video Surface
BOOLEAN BltVideoSurface(VSurfID destSurface, VSurfID srcSurface, INT32 iDestX, INT32 iDestY,
                        UINT32 fBltFlags, struct BltOpts *pBltFx);

BOOLEAN VSurfaceColorFill(struct VSurface *dest, i32 x1, i32 y1, i32 x2, i32 y2, u16 Color16BPP);

BOOLEAN ColorFillVideoSurfaceArea(VSurfID destSurface, INT32 x1, INT32 y1, INT32 x2, INT32 y2,
                                  u16 Color16BPP);

BOOLEAN ImageFillVideoSurfaceArea(VSurfID destSurface, INT32 x1, INT32 y1, INT32 x2, INT32 y2,
                                  struct VObject *BkgrndImg, UINT16 Index, INT16 Ox, INT16 Oy);

// Sets transparency
BOOLEAN SetVideoSurfaceTransparency(UINT32 uiIndex, COLORVAL TransColor);

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// Video Surface manipulation functions
//
///////////////////////////////////////////////////////////////////////////////////////////////////

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

BYTE *VSurfaceLockOld(struct VSurface *vs, u32 *pitch);
BYTE *LockVideoSurface(VSurfID uiVSurface, u32 *pitch);
void UnLockVideoSurface(VSurfID uiVSurface);

struct VSurface *CreateVideoSurface(u16 width, u16 height, u8 bitDepth);
struct VSurface *CreateVideoSurfaceFromFile(const char *path);
struct VSurface *VSurfaceAdd(u16 width, u16 height, u8 bitDepth, VSurfID *puiIndex);

// Global variables for video objects
extern UINT32 guiBOTTOMPANEL;
extern UINT32 guiRIGHTPANEL;
extern UINT32 guiSAVEBUFFER;

BOOLEAN InitializeSystemVideoObjects();
BOOLEAN InitializeGameVideoObjects();

#endif
