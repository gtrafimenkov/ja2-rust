#ifndef __IMAGE_H
#define __IMAGE_H

#include "SGP/MemMan.h"
#include "SGP/Types.h"

struct Subimage;

// The struct Image* module provides a common interface for managing image data. This module
// includes:
// - A set of data structures representing image data. Data can be 8 or 16 bpp and/or
//   compressed
// - A set of file loaders which load specific file formats into the internal data format
// - A set of blitters which blt the data to memory
// - A comprehensive automatic blitter which blits the appropriate type based on the
//   image header.

// Defines for buffer bit depth
#define BUFFER_8BPP 0x1
#define BUFFER_16BPP 0x2

// Palette structure, mimics that of Win32
struct SGPPaletteEntry;

#define AUX_FULL_TILE 0x01
#define AUX_ANIMATED_TILE 0x02
#define AUX_DYNAMIC_TILE 0x04
#define AUX_INTERACTIVE_TILE 0x08
#define AUX_IGNORES_HEIGHT 0x10
#define AUX_USES_LAND_Z 0x20

struct AuxObjectData {
  UINT8 ubWallOrientation;
  UINT8 ubNumberOfTiles;
  UINT16 usTileLocIndex;
  UINT8 ubUnused1[3];
  UINT8 ubCurrentFrame;
  UINT8 ubNumberOfFrames;
  UINT8 fFlags;
  UINT8 ubUnused[6];
};

// relative tile location
struct RelTileLoc {
  INT8 bTileOffsetX;
  INT8 bTileOffsetY;
};

struct ImageData {
  void *image_data;
  UINT32 image_data_size;
  struct Subimage *subimages;
  UINT16 usNumberOfObjects;
};

// Image header structure
struct Image {
  UINT16 usWidth;
  UINT16 usHeight;
  UINT8 ubBitDepth;
  struct SGPPaletteEntry *pPalette;
  UINT16 *pui16BPPPalette;
  UINT8 *pAppData;
  UINT32 uiAppDataSize;
  void *image_data;
  UINT32 image_data_size;
  struct Subimage *subimages;
  UINT16 usNumberOfObjects;
  bool imageDataAllocatedInRust;
  bool paletteAllocatedInRust;
};

#define SGPGetRValue(rgb) ((BYTE)(rgb))
#define SGPGetBValue(rgb) ((BYTE)((rgb) >> 16))
#define SGPGetGValue(rgb) ((BYTE)(((UINT16)(rgb)) >> 8))

// This function will return NULL if it fails, and call SetLastError() to set
// error information
struct Image *CreateImage(const char *ImageFile, bool loadAppData);

// This function destroys the struct Image* structure as well as its contents
void DestroyImage(struct Image *hImage);

BOOLEAN ReleaseImageData(struct Image *hImage);

// This function will run the appropriate copy function based on the type of struct Image*
// object
BOOLEAN CopyImageToBuffer(struct Image *hImage, UINT32 fBufferType, BYTE *pDestBuf,
                          UINT16 usDestWidth, UINT16 usDestHeight, UINT16 usX, UINT16 usY,
                          SGPRect *srcRect);

// The following blitters are used by the function above as well as clients

BOOLEAN Copy8BPPImageTo8BPPBuffer(struct Image *hImage, BYTE *pDestBuf, UINT16 usDestWidth,
                                  UINT16 usDestHeight, UINT16 usX, UINT16 usY, SGPRect *srcRect);
BOOLEAN Copy8BPPImageTo16BPPBuffer(struct Image *hImage, BYTE *pDestBuf, UINT16 usDestWidth,
                                   UINT16 usDestHeight, UINT16 usX, UINT16 usY, SGPRect *srcRect);
BOOLEAN Copy16BPPImageTo16BPPBuffer(struct Image *hImage, BYTE *pDestBuf, UINT16 usDestWidth,
                                    UINT16 usDestHeight, UINT16 usX, UINT16 usY, SGPRect *srcRect);

// This function will create a buffer in memory of ETRLE data, excluding palette
BOOLEAN CopyImageData(struct Image *hImage, struct ImageData *pBuffer);

// UTILITY FUNCTIONS

// Used to create a 16BPP Palette from an 8 bit palette, found in himage.c
UINT16 *Create16BPPPaletteShaded(struct SGPPaletteEntry *pPalette, UINT32 rscale, UINT32 gscale,
                                 UINT32 bscale, BOOLEAN mono);
UINT16 *Create16BPPPalette(struct SGPPaletteEntry *pPalette);
UINT16 Get16BPPColor(UINT32 RGBValue);
UINT32 GetRGBColor(UINT16 Value16BPP);
struct SGPPaletteEntry *ConvertRGBToPaletteEntry(UINT8 sbStart, UINT8 sbEnd, UINT8 *pOldPalette);

extern UINT16 gusAlphaMask;

// used to convert 565 RGB data into different bit-formats
void ConvertRGBDistribution565To555(UINT16 *p16BPPData, UINT32 uiNumberOfPixels);

#endif
