#ifndef __IMAGE_H
#define __IMAGE_H

#include "SGP/MemMan.h"
#include "SGP/Types.h"
#include "rust_images.h"

struct GRect;
struct Subimage;

// The struct Image* module provides a common interface for managing image data. This module
// includes:
// - A set of data structures representing image data. Data can be 8 or 16 bpp and/or
//   compressed
// - A set of file loaders which load specific file formats into the internal data format
// - A set of blitters which blt the data to memory
// - A comprehensive automatic blitter which blits the appropriate type based on the
//   image header.

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
  UINT16 number_of_subimages;
};

struct ImageDataParams {
  u16 width;
  u16 height;
  u16 *palette16bpp;
  void *data;
  u32 pitch;  // amount of bytes per line; it is at least widht * bytes_per_pixel, but can be more
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
BOOLEAN CopyImageToBuffer(struct Image *hImage, u8 bufferBitDepth, BYTE *pDestBuf,
                          UINT16 usDestWidth, UINT16 usDestHeight, UINT16 usX, UINT16 usY,
                          struct GRect *srcRect);

// The following blitters are used by the function above as well as clients

BOOLEAN Copy8BPPImageTo8BPPBuffer(struct Image *hImage, BYTE *pDestBuf, UINT16 usDestWidth,
                                  UINT16 usDestHeight, UINT16 usX, UINT16 usY,
                                  struct GRect *srcRect);
BOOLEAN Copy8BPPImageTo16BPPBuffer(struct Image *hImage, BYTE *pDestBuf, UINT16 usDestWidth,
                                   UINT16 usDestHeight, UINT16 usX, UINT16 usY,
                                   struct GRect *srcRect);
BOOLEAN Copy16BPPImageTo16BPPBuffer(struct Image *hImage, BYTE *pDestBuf, UINT16 usDestWidth,
                                    UINT16 usDestHeight, UINT16 usX, UINT16 usY,
                                    struct GRect *srcRect);

// This function will create a buffer in memory of ETRLE data, excluding palette
BOOLEAN CopyImageData(struct Image *hImage, struct ImageData *pBuffer);

// UTILITY FUNCTIONS

// Used to create a 16BPP Palette from an 8 bit palette, found in himage.c
UINT16 *Create16BPPPaletteShaded(struct SGPPaletteEntry *pPalette, UINT32 rscale, UINT32 gscale,
                                 UINT32 bscale, BOOLEAN mono);
UINT16 *Create16BPPPalette(struct SGPPaletteEntry *pPalette);
struct SGPPaletteEntry *ConvertRGBToPaletteEntry(UINT8 sbStart, UINT8 sbEnd, UINT8 *pOldPalette);

// Create a scaled down copy of an image.
struct Image *ScaleImageDown2x(struct Image *image);

#endif
