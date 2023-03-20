#ifndef __VSURFACE_H
#define __VSURFACE_H

#include "SGP/Container.h"
#include "SGP/HImage.h"
#include "SGP/Types.h"

struct VObject;

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// Video Surface SGP Module
//
///////////////////////////////////////////////////////////////////////////////////////////////////

//
// Defines for special video object handles given to blit function
//

#define PRIMARY_SURFACE 0xFFFFFFF0
#define BACKBUFFER 0xFFFFFFF1
#define FRAME_BUFFER 0xFFFFFFF2
#define MOUSE_BUFFER 0xFFFFFFF3

//
// Defines for blitting
//

#define VS_BLT_COLORFILL 0x000000020
#define VS_BLT_USECOLORKEY 0x000000002
#define VS_BLT_USEDESTCOLORKEY 0x000000200
#define VS_BLT_FAST 0x000000004
#define VS_BLT_CLIPPED 0x000000008
#define VS_BLT_SRCREGION 0x000000010
#define VS_BLT_DESTREGION 0x000000080
#define VS_BLT_SRCSUBRECT 0x000000040
#define VS_BLT_COLORFILLRECT 0x000000100
#define VS_BLT_MIRROR_Y 0x000001000

//
// Effects structure for specialized blitting
//

typedef struct {
  COLORVAL ColorFill;  // Used for fill effect
  SGPRect SrcRect;     // Given SRC subrect instead of srcregion
  SGPRect FillRect;    // Given SRC subrect instead of srcregion
  UINT16 DestRegion;   // Given a DEST region for dest positions within the VO
} blt_vs_fx;

//
// Video Surface Flags
// Used to describe the memory usage of a video Surface
//

#define VSURFACE_DEFAULT_MEM_USAGE \
  0x00000001  // Default mem usage is same as DD, try video and then try system. Will usually work
#define VSURFACE_VIDEO_MEM_USAGE \
  0x00000002  // Will force surface into video memory and will fail if it can't
#define VSURFACE_SYSTEM_MEM_USAGE \
  0x00000004  // Will force surface into system memory and will fail if it can't
#define VSURFACE_RESERVED_SURFACE \
  0x00000100  // Reserved for special purposes, like a primary surface

//
// Video Surface creation flags
// Used in the VSurface_DESC structure to describe creation flags
//

#define VSURFACE_CREATE_DEFAULT \
  0x00000020  // Creates and empty Surface of given width, height and BPP
#define VSURFACE_CREATE_FROMFILE 0x00000040  // Creates a video Surface from a file ( using HIMAGE )

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
BOOLEAN AddVideoSurface(VSURFACE_DESC *VSurfaceDesc, UINT32 *uiIndex);

// Returns a HVSurface for the specified index
BOOLEAN GetVideoSurface(struct VSurface **hVSurface, UINT32 uiIndex);

BYTE *LockVideoSurface(UINT32 uiVSurface, UINT32 *uiPitch);
void UnLockVideoSurface(UINT32 uiVSurface);

// Blits a video Surface to another video Surface
BOOLEAN BltVideoSurface(UINT32 uiDestVSurface, UINT32 uiSrcVSurface, UINT16 usRegionIndex,
                        INT32 iDestX, INT32 iDestY, UINT32 fBltFlags, blt_vs_fx *pBltFx);

BOOLEAN ColorFillVideoSurfaceArea(UINT32 uiDestVSurface, INT32 iDestX1, INT32 iDestY1,
                                  INT32 iDestX2, INT32 iDestY2, UINT16 Color16BPP);

BOOLEAN ImageFillVideoSurfaceArea(UINT32 uiDestVSurface, INT32 iDestX1, INT32 iDestY1,
                                  INT32 iDestX2, INT32 iDestY2, struct VObject *BkgrndImg,
                                  UINT16 Index, INT16 Ox, INT16 Oy);

// Sets transparency
BOOLEAN SetVideoSurfaceTransparency(UINT32 uiIndex, COLORVAL TransColor);

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// Video Surface manipulation functions
//
///////////////////////////////////////////////////////////////////////////////////////////////////

// Created from a VSurface_DESC structure. Can be from a file via HIMAGE or empty.
struct VSurface *CreateVideoSurface(VSURFACE_DESC *VSurfaceDesc);

// Gets the RGB palette entry values
BOOLEAN GetVSurfacePaletteEntries(struct VSurface *hVSurface, struct SGPPaletteEntry *pPalette);

// Set data from HIMAGE.
BOOLEAN SetVideoSurfaceDataFromHImage(struct VSurface *hVSurface, HIMAGE hImage, UINT16 usX,
                                      UINT16 usY, SGPRect *pSrcRect);

// Sets Transparency color into HVSurface and the underlying DD surface
BOOLEAN SetVideoSurfaceTransparencyColor(struct VSurface *hVSurface, COLORVAL TransColor);

// Sets HVSurface palette, creates if nessessary. Also sets 16BPP palette
BOOLEAN SetVideoSurfacePalette(struct VSurface *hVSurface, struct SGPPaletteEntry *pSrcPalette);

// Deletes all data, including palettes, regions, DD Surfaces
BOOLEAN DeleteVideoSurface(struct VSurface *hVSurface);
BOOLEAN DeleteVideoSurfaceFromIndex(UINT32 uiIndex);

/////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Blt Functions
//
///////////////////////////////////////////////////////////////////////////////////////////////////

// These blitting functions more-or less encapsolate all of the functionality of DirectDraw
// Blitting, giving an API layer for portability.

BOOLEAN BltVideoSurfaceToVideoSurface(struct VSurface *hDestVSurface, struct VSurface *hSrcVSurface,
                                      UINT16 usIndex, INT32 iDestX, INT32 iDestY, INT32 fBltFlags,
                                      blt_vs_fx *pBltFx);

struct VSurface *GetPrimaryVideoSurface();
struct VSurface *GetBackBufferVideoSurface();

BOOLEAN ShadowVideoSurfaceRect(UINT32 uiDestVSurface, INT32 X1, INT32 Y1, INT32 X2, INT32 Y2);
BOOLEAN ShadowVideoSurfaceImage(UINT32 uiDestVSurface, struct VObject *hImageHandle, INT32 iPosX,
                                INT32 iPosY);

// If the Dest Rect and the source rect are not the same size, the source surface will be either
// enlraged or shunk.
BOOLEAN BltStretchVideoSurface(UINT32 uiDestVSurface, UINT32 uiSrcVSurface, INT32 iDestX,
                               INT32 iDestY, UINT32 fBltFlags, SGPRect *SrcRect, SGPRect *DestRect);

BOOLEAN ShadowVideoSurfaceRectUsingLowPercentTable(UINT32 uiDestVSurface, INT32 X1, INT32 Y1,
                                                   INT32 X2, INT32 Y2);

#endif
