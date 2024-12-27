#ifndef __IMAGE_H
#define __IMAGE_H

#include "SGP/ImgFmt.h"
#include "SGP/MemMan.h"
#include "SGP/Types.h"

// The HIMAGE module provides a common interface for managing image data. This module
// includes:
// - A set of data structures representing image data. Data can be 8 or 16 bpp and/or
//   compressed
// - A set of file loaders which load specific file formats into the internal data format
// - A set of blitters which blt the data to memory
// - A comprehensive automatic blitter which blits the appropriate type based on the
//   image header.

// Defines for type of file readers
#define PCX_FILE_READER 0x1
#define TGA_FILE_READER 0x2
#define STCI_FILE_READER 0x4
#define TRLE_FILE_READER 0x8
#define UNKNOWN_FILE_READER 0x200

// Defines for buffer bit depth
#define BUFFER_8BPP 0x1
#define BUFFER_16BPP 0x2

// Defines for image charactoristics
#define IMAGE_COMPRESSED 0x0001
#define IMAGE_TRLECOMPRESSED 0x0002
#define IMAGE_PALETTE 0x0004
#define IMAGE_BITMAPDATA 0x0008
#define IMAGE_APPDATA 0x0010
#define IMAGE_ALLIMAGEDATA 0x000C
#define IMAGE_ALLDATA 0x001C

// Palette structure, mimics that of Win32
struct SGPPaletteEntry {
  uint8_t peRed;
  uint8_t peGreen;
  uint8_t peBlue;
  uint8_t peFlags;
};

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

// TRLE subimage structure, mirroring that of ST(C)I
typedef struct tagETRLEObject {
  uint32_t uiDataOffset;
  uint32_t uiDataLength;
  INT16 sOffsetX;
  INT16 sOffsetY;
  uint16_t usHeight;
  uint16_t usWidth;
} ETRLEObject;

typedef struct tagETRLEData {
  PTR pPixData;
  uint32_t uiSizePixData;
  ETRLEObject *pETRLEObject;
  uint16_t usNumberOfObjects;
} ETRLEData;

// Image header structure
typedef struct {
  uint16_t usWidth;
  uint16_t usHeight;
  uint8_t ubBitDepth;
  uint16_t fFlags;
  SGPFILENAME ImageFile;
  uint32_t iFileLoader;
  struct SGPPaletteEntry *pPalette;
  uint16_t *pui16BPPPalette;
  uint8_t *pAppData;
  uint32_t uiAppDataSize;
  // This union is used to describe each data type and is flexible to include the
  // data strucutre of the compresssed format, once developed.
  union {
    struct {
      PTR pImageData;
    };
    struct {
      PTR pCompressedImageData;
    };
    struct {
      uint8_t *p8BPPData;
    };
    struct {
      uint16_t *p16BPPData;
    };
    struct {
      uint8_t *pPixData8;
      uint32_t uiSizePixData;
      ETRLEObject *pETRLEObject;
      uint16_t usNumberOfObjects;
    };
  };

} image_type, *HIMAGE;

#define SGPGetRValue(rgb) ((BYTE)(rgb))
#define SGPGetBValue(rgb) ((BYTE)((rgb) >> 16))
#define SGPGetGValue(rgb) ((BYTE)(((uint16_t)(rgb)) >> 8))

// This function will return NULL if it fails, and call SetLastError() to set
// error information
HIMAGE CreateImage(const char *ImageFile, uint16_t fContents);

// This function destroys the HIMAGE structure as well as its contents
BOOLEAN DestroyImage(HIMAGE hImage);

// This function releases data allocated to various parts of the image based
// on the contents flags passed as a parameter.  If a contents flag is given
// and the image does not contain that data, no error is raised
BOOLEAN ReleaseImageData(HIMAGE hImage, uint16_t fContents);

// This function will attept to Load data from an existing image object's filename
// In this way, dynamic loading of image data can be done
BOOLEAN LoadImageData(HIMAGE hImage, uint16_t fContents);

// This function will run the appropriate copy function based on the type of HIMAGE object
BOOLEAN CopyImageToBuffer(HIMAGE hImage, uint32_t fBufferType, BYTE *pDestBuf, uint16_t usDestWidth,
                          uint16_t usDestHeight, uint16_t usX, uint16_t usY, SGPRect *srcRect);

// The following blitters are used by the function above as well as clients

BOOLEAN Copy8BPPImageTo8BPPBuffer(HIMAGE hImage, BYTE *pDestBuf, uint16_t usDestWidth,
                                  uint16_t usDestHeight, uint16_t usX, uint16_t usY,
                                  SGPRect *srcRect);
BOOLEAN Copy8BPPImageTo16BPPBuffer(HIMAGE hImage, BYTE *pDestBuf, uint16_t usDestWidth,
                                   uint16_t usDestHeight, uint16_t usX, uint16_t usY,
                                   SGPRect *srcRect);
BOOLEAN Copy16BPPImageTo16BPPBuffer(HIMAGE hImage, BYTE *pDestBuf, uint16_t usDestWidth,
                                    uint16_t usDestHeight, uint16_t usX, uint16_t usY,
                                    SGPRect *srcRect);

// This function will create a buffer in memory of ETRLE data, excluding palette
BOOLEAN GetETRLEImageData(HIMAGE hImage, ETRLEData *pBuffer);

// UTILITY FUNCTIONS

// Used to create a 16BPP Palette from an 8 bit palette, found in himage.c
uint16_t *Create16BPPPaletteShaded(struct SGPPaletteEntry *pPalette, uint32_t rscale,
                                   uint32_t gscale, uint32_t bscale, BOOLEAN mono);
uint16_t *Create16BPPPalette(struct SGPPaletteEntry *pPalette);
uint16_t Get16BPPColor(uint32_t RGBValue);
uint32_t GetRGBColor(uint16_t Value16BPP);
struct SGPPaletteEntry *ConvertRGBToPaletteEntry(uint8_t sbStart, uint8_t sbEnd,
                                                 uint8_t *pOldPalette);

extern uint16_t gusAlphaMask;
extern uint16_t gusRedMask;
extern uint16_t gusGreenMask;
extern uint16_t gusBlueMask;
extern INT16 gusRedShift;
extern INT16 gusBlueShift;
extern INT16 gusGreenShift;

// used to convert 565 RGB data into different bit-formats
void ConvertRGBDistribution565To555(uint16_t *p16BPPData, uint32_t uiNumberOfPixels);
void ConvertRGBDistribution565To655(uint16_t *p16BPPData, uint32_t uiNumberOfPixels);
void ConvertRGBDistribution565To556(uint16_t *p16BPPData, uint32_t uiNumberOfPixels);
void ConvertRGBDistribution565ToAny(uint16_t *p16BPPData, uint32_t uiNumberOfPixels);

#endif
