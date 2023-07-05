#ifndef __VSURFACE_H
#define __VSURFACE_H

#include "SGP/Container.h"
#include "SGP/HImage.h"
#include "SGP/Types.h"
#include "rust_geometry.h"

struct VObject;
struct VSurface;

typedef uint32_t VSurfID;

extern struct VSurface *vsBB;
extern struct VSurface *vsFB;
extern struct VSurface *vsMouseCursor;
extern struct VSurface *vsMouseBuffer;
extern struct VSurface *vsPrimary;
extern struct VSurface *vsExtraBuffer;
extern struct VSurface *vsSB;

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
  COLORVAL ColorFill;     // Used for fill effect
  struct GRect SrcRect;   // Given SRC subrect instead of srcregion
  struct GRect FillRect;  // Given SRC subrect instead of srcregion
};

uint16_t GetVSurfaceHeight(const struct VSurface *vs);
uint16_t GetVSurfaceWidth(const struct VSurface *vs);

//
// This structure describes the creation parameters for a Video Surface
//

typedef struct {
  uint16_t usWidth;
  uint16_t usHeight;
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

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// Video Surface manipulation functions
//
///////////////////////////////////////////////////////////////////////////////////////////////////

// Set data from struct Image*.
BOOLEAN SetVideoSurfaceDataFromHImage(struct VSurface *hVSurface, struct Image *hImage,
                                      uint16_t usX, uint16_t usY, struct GRect *pSrcRect);

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
                               int32_t iDestY, uint32_t fBltFlags, struct GRect *SrcRect,
                               struct GRect *DestRect);

BOOLEAN ShadowVideoSurfaceRectUsingLowPercentTable(struct VSurface *dest, int32_t X1, int32_t Y1,
                                                   int32_t X2, int32_t Y2);

// The following structure is used to define a region of the video Surface
// These regions are stored via a HLIST
typedef struct {
  struct GRect RegionCoords;  // Rectangle describing coordinates of region
  SGPPoint Origin;            // Origin used for hot spots, etc
  uint8_t ubHitMask;          // Byte flags for hit detection
} VSURFACE_REGION;

void SetClippingRect(struct GRect *clip);

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

struct VSurface *CreateVideoSurface(uint16_t width, uint16_t height);
struct VSurface *VSurfaceAdd(uint16_t width, uint16_t height, VSurfID *puiIndex);

// Global variables for video objects
extern uint32_t guiRIGHTPANEL;
extern uint32_t guiSAVEBUFFER;

BOOLEAN InitializeSystemVideoObjects();
BOOLEAN InitializeGameVideoObjects();

void BlitImageToSurface(struct Image *source, struct VSurface *dest, int32_t x, int32_t y);
void BlitImageToSurfaceRect(struct Image *source, struct VSurface *dest, int32_t x, int32_t y,
                            struct GRect sourceRect);

#endif
