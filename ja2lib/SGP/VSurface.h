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
  COLORVAL ColorFill;   // Used for fill effect
  SGPRect SrcRect;      // Given SRC subrect instead of srcregion
  SGPRect FillRect;     // Given SRC subrect instead of srcregion
  uint16_t DestRegion;  // Given a DEST region for dest positions within the VO

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
// The following structure is used to define a region of the video Surface
// These regions are stored via a HLIST
//

typedef struct {
  SGPRect RegionCoords;  // Rectangle describing coordinates of region
  SGPPoint Origin;       // Origin used for hot spots, etc
  uint8_t ubHitMask;     // Byte flags for hit detection

} VSURFACE_REGION;

//
// This structure is a video Surface. Contains a HLIST of regions
//

struct VSurface {
  uint16_t usHeight;       // Height of Video Surface
  uint16_t usWidth;        // Width of Video Surface
  uint8_t ubBitDepth;      // BPP ALWAYS 16!
  PTR pSurfaceData;        // A void pointer, but for this implementation, is really a
                           // lpDirectDrawSurface;
  PTR pSurfaceData1;       // Direct Draw One Interface
  PTR pSavedSurfaceData1;  // A void pointer, but for this implementation, is really a
                           // lpDirectDrawSurface; pSavedSurfaceData is used to hold all video
                           // memory Surfaces so that they my be restored
  PTR pSavedSurfaceData;   // A void pointer, but for this implementation, is really a
                          // lpDirectDrawSurface; pSavedSurfaceData is used to hold all video memory
                          // Surfaces so that they my be restored
  uint32_t fFlags;            // Used to describe memory usage, etc
  PTR pPalette;               // A void pointer, but for this implementation a DDPalette
  uint16_t *p16BPPPalette;    // A 16BPP palette used for 8->16 blits
  COLORVAL TransparentColor;  // Defaults to 0,0,0
  PTR pClipper;               // A void pointer encapsolated as a clipper Surface
  HLIST RegionList;           // A List of regions within the video Surface
};

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

extern INT32 giMemUsedInSurfaces;

// Creates a list to contain video Surfaces
BOOLEAN InitializeVideoSurfaceManager();

// Deletes any video Surface placed into list
BOOLEAN ShutdownVideoSurfaceManager();

// Restores all video Surfaces in list
BOOLEAN RestoreVideoSurfaces();

// Creates and adds a video Surface to list
#ifdef SGP_VIDEO_DEBUGGING
void DumpVSurfaceInfoIntoFile(CHAR8 *filename, BOOLEAN fAppend);
extern BOOLEAN _AddAndRecordVSurface(VSURFACE_DESC *VSurfaceDesc, uint32_t *uiIndex,
                                     uint32_t uiLineNum, CHAR8 *pSourceFile);
#define AddVideoSurface(a, b) _AddAndRecordVSurface(a, b, __LINE__, __FILE__)
#else
#define AddVideoSurface(a, b) AddStandardVideoSurface(a, b)
#endif

BOOLEAN AddStandardVideoSurface(VSURFACE_DESC *VSurfaceDesc, uint32_t *uiIndex);

// Returns a HVSurface for the specified index
BOOLEAN GetVideoSurface(struct VSurface **hVSurface, uint32_t uiIndex);

BYTE *LockVideoSurface(uint32_t uiVSurface, uint32_t *uiPitch);
void UnLockVideoSurface(uint32_t uiVSurface);

// Blits a video Surface to another video Surface
BOOLEAN BltVideoSurface(uint32_t uiDestVSurface, uint32_t uiSrcVSurface, uint16_t usRegionIndex,
                        INT32 iDestX, INT32 iDestY, uint32_t fBltFlags, blt_vs_fx *pBltFx);

BOOLEAN ColorFillVideoSurfaceArea(uint32_t uiDestVSurface, INT32 iDestX1, INT32 iDestY1,
                                  INT32 iDestX2, INT32 iDestY2, uint16_t Color16BPP);

BOOLEAN ImageFillVideoSurfaceArea(uint32_t uiDestVSurface, INT32 iDestX1, INT32 iDestY1,
                                  INT32 iDestX2, INT32 iDestY2, struct VObject *BkgrndImg,
                                  uint16_t Index, INT16 Ox, INT16 Oy);

// This function sets the global video Surfaces for primary and backbuffer
BOOLEAN SetPrimaryVideoSurfaces();

// Sets transparency
BOOLEAN SetVideoSurfaceTransparency(uint32_t uiIndex, COLORVAL TransColor);

// Adds a video Surface region
BOOLEAN AddVideoSurfaceRegion(uint32_t uiIndex, VSURFACE_REGION *pNewRegion);

// Gets width, hight, bpp information
BOOLEAN GetVideoSurfaceDescription(uint32_t uiIndex, uint16_t *usWidth, uint16_t *usHeight,
                                   uint8_t *ubBitDepth);

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// Video Surface manipulation functions
//
///////////////////////////////////////////////////////////////////////////////////////////////////

// Darkens a rectangular area on a surface for menus etc.
BOOLEAN PixelateVideoSurfaceRect(uint32_t uiDestVSurface, INT32 X1, INT32 Y1, INT32 X2, INT32 Y2);

// Created from a VSurface_DESC structure. Can be from a file via HIMAGE or empty.
struct VSurface *CreateVideoSurface(VSURFACE_DESC *VSurfaceDesc);

// Gets the RGB palette entry values
BOOLEAN GetVSurfacePaletteEntries(struct VSurface *hVSurface, struct SGPPaletteEntry *pPalette);

BOOLEAN RestoreVideoSurface(struct VSurface *hVSurface);

// Returns a flat pointer for direct manipulation of data
BYTE *LockVideoSurfaceBuffer(struct VSurface *hVSurface, uint32_t *pPitch);

// Must be called after Locking buffer call above
void UnLockVideoSurfaceBuffer(struct VSurface *hVSurface);

// Set data from HIMAGE.
BOOLEAN SetVideoSurfaceDataFromHImage(struct VSurface *hVSurface, HIMAGE hImage, uint16_t usX,
                                      uint16_t usY, SGPRect *pSrcRect);

// Sets Transparency color into HVSurface and the underlying DD surface
BOOLEAN SetVideoSurfaceTransparencyColor(struct VSurface *hVSurface, COLORVAL TransColor);

// Sets HVSurface palette, creates if nessessary. Also sets 16BPP palette
BOOLEAN SetVideoSurfacePalette(struct VSurface *hVSurface, struct SGPPaletteEntry *pSrcPalette);

// Used if it's in video memory, will re-load backup copy
// BOOLEAN RestoreVideoSurface( HVSurface hVSurface );

// Deletes all data, including palettes, regions, DD Surfaces
BOOLEAN DeleteVideoSurface(struct VSurface *hVSurface);
BOOLEAN DeleteVideoSurfaceFromIndex(uint32_t uiIndex);

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// Region manipulation functions
//
///////////////////////////////////////////////////////////////////////////////////////////////////

// Regions will allow creation of sections within the Surface to manipulate quickly and cleanly
// An example would be a cursor tileset
BOOLEAN AddVSurfaceRegion(struct VSurface *hVSurface, VSURFACE_REGION *pNewRegion);
BOOLEAN AddVSurfaceRegionAtIndex(struct VSurface *hVSurface, uint16_t usIndex,
                                 VSURFACE_REGION *pNewRegion);
BOOLEAN AddVSurfaceRegions(struct VSurface *hVSurface, VSURFACE_REGION **ppNewRegions,
                           uint16_t uiNumRegions);
BOOLEAN RemoveVSurfaceRegion(struct VSurface *hVSurface, uint16_t usIndex);
BOOLEAN ClearAllVSurfaceRegions(struct VSurface *hVSurface);
BOOLEAN GetVSurfaceRegion(struct VSurface *hVSurface, uint16_t usIndex, VSURFACE_REGION *aRegion);
BOOLEAN GetNumRegions(struct VSurface *hVSurface, uint32_t *puiNumRegions);
BOOLEAN ReplaceVSurfaceRegion(struct VSurface *hVSurface, uint16_t usIndex,
                              VSURFACE_REGION *aRegion);
BOOLEAN DeleteVideoSurfaceFromIndex(uint32_t uiIndex);

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// Clipper manipulation functions
//
///////////////////////////////////////////////////////////////////////////////////////////////////

BOOLEAN SetClipList(struct VSurface *hVSurface, SGPRect *RegionData, uint16_t usNumRegions);

/////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Blt Functions
//
///////////////////////////////////////////////////////////////////////////////////////////////////

// These blitting functions more-or less encapsolate all of the functionality of DirectDraw
// Blitting, giving an API layer for portability.

BOOLEAN BltVideoSurfaceToVideoSurface(struct VSurface *hDestVSurface, struct VSurface *hSrcVSurface,
                                      uint16_t usIndex, INT32 iDestX, INT32 iDestY, INT32 fBltFlags,
                                      blt_vs_fx *pBltFx);

struct VSurface *GetPrimaryVideoSurface();
struct VSurface *GetBackBufferVideoSurface();

BOOLEAN ShadowVideoSurfaceRect(uint32_t uiDestVSurface, INT32 X1, INT32 Y1, INT32 X2, INT32 Y2);
BOOLEAN ShadowVideoSurfaceImage(uint32_t uiDestVSurface, struct VObject *hImageHandle, INT32 iPosX,
                                INT32 iPosY);

// If the Dest Rect and the source rect are not the same size, the source surface will be either
// enlraged or shunk.
BOOLEAN BltStretchVideoSurface(uint32_t uiDestVSurface, uint32_t uiSrcVSurface, INT32 iDestX,
                               INT32 iDestY, uint32_t fBltFlags, SGPRect *SrcRect,
                               SGPRect *DestRect);

BOOLEAN MakeVSurfaceFromVObject(uint32_t uiVObject, uint16_t usSubIndex, uint32_t *puiVSurface);

BOOLEAN ShadowVideoSurfaceRectUsingLowPercentTable(uint32_t uiDestVSurface, INT32 X1, INT32 Y1,
                                                   INT32 X2, INT32 Y2);

#endif
