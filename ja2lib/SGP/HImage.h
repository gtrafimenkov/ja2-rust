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
  uint8_t ubWallOrientation;
  uint8_t ubNumberOfTiles;
  uint16_t usTileLocIndex;
  uint8_t ubUnused1[3];
  uint8_t ubCurrentFrame;
  uint8_t ubNumberOfFrames;
  uint8_t fFlags;
  uint8_t ubUnused[6];
};

// relative tile location
struct RelTileLoc {
  int8_t bTileOffsetX;
  int8_t bTileOffsetY;
};

struct ImageData {
  void *image_data;
  uint32_t image_data_size;
  struct Subimage *subimages;
  uint16_t number_of_subimages;
};

struct ImageDataParams {
  uint16_t width;
  uint16_t height;
  uint16_t *palette16bpp;
  void *data;
  uint32_t
      pitch;  // amount of bytes per line; it is at least widht * bytes_per_pixel, but can be more
};

#define SGPGetRValue(rgb) ((BYTE)(rgb))
#define SGPGetBValue(rgb) ((BYTE)((rgb) >> 16))
#define SGPGetGValue(rgb) ((BYTE)(((uint16_t)(rgb)) >> 8))

// This function will return NULL if it fails, and call SetLastError() to set
// error information
struct Image *CreateImage(const char *ImageFile, bool loadAppData);

// This function destroys the struct Image* structure as well as its contents
void DestroyImage(struct Image *hImage);

BOOLEAN ReleaseImageData(struct Image *hImage);

// This function will run the appropriate copy function based on the type of struct Image*
// object
BOOLEAN CopyImageToBuffer(struct Image *hImage, uint8_t bufferBitDepth, BYTE *pDestBuf,
                          uint16_t usDestWidth, uint16_t usDestHeight, uint16_t usX, uint16_t usY,
                          struct GRect *srcRect);

// The following blitters are used by the function above as well as clients

BOOLEAN Copy8BPPImageTo8BPPBuffer(struct Image *hImage, BYTE *pDestBuf, uint16_t usDestWidth,
                                  uint16_t usDestHeight, uint16_t usX, uint16_t usY,
                                  struct GRect *srcRect);
BOOLEAN Copy8BPPImageTo16BPPBuffer(struct Image *hImage, BYTE *pDestBuf, uint16_t usDestWidth,
                                   uint16_t usDestHeight, uint16_t usX, uint16_t usY,
                                   struct GRect *srcRect);
BOOLEAN Copy16BPPImageTo16BPPBuffer(struct Image *hImage, BYTE *pDestBuf, uint16_t usDestWidth,
                                    uint16_t usDestHeight, uint16_t usX, uint16_t usY,
                                    struct GRect *srcRect);

// This function will create a buffer in memory of ETRLE data, excluding palette
BOOLEAN CopyImageData(struct Image *hImage, struct ImageData *pBuffer);

// UTILITY FUNCTIONS

// Used to create a 16BPP Palette from an 8 bit palette, found in himage.c
uint16_t *Create16BPPPaletteShaded(struct SGPPaletteEntry *pPalette, uint32_t rscale,
                                   uint32_t gscale, uint32_t bscale, BOOLEAN mono);
uint16_t *Create16BPPPalette(struct SGPPaletteEntry *pPalette);
struct SGPPaletteEntry *ConvertRGBToPaletteEntry(uint8_t sbStart, uint8_t sbEnd,
                                                 uint8_t *pOldPalette);

// Create a scaled down copy of an image.
struct Image *ScaleImageDown2x(struct Image *image);

#endif
