#if !defined(STCI_H)

#define STCI_H

#include "SGP/Types.h"
#include "rust_images.h"

#define STCI_ETRLE_COMPRESSED 0x0020
#define STCI_ZLIB_COMPRESSED 0x0010

// ETRLE defines
#define COMPRESS_TRANSPARENT 0x80
#define COMPRESS_NON_TRANSPARENT 0x00
#define COMPRESS_RUN_LIMIT 0x7F

typedef struct {
  uint32_t uiDataOffset;
  uint32_t uiDataLength;
  int16_t sOffsetX;
  int16_t sOffsetY;
  uint16_t usHeight;
  uint16_t usWidth;
} STCISubImage;

#define STCI_SUBIMAGE_SIZE 16

typedef struct {
  uint8_t ubRed;
  uint8_t ubGreen;
  uint8_t ubBlue;
} STCIPaletteElement;

#define STCI_PALETTE_ELEMENT_SIZE 3

#endif
