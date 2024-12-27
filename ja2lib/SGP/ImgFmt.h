#if !defined(STCI_H)

#define STCI_H

// Sir-Tech's Crazy Image (STCI) file format specifications.  Each file is composed of:
// 1		ImageFileHeader, uncompressed
// *		Palette (STCI_INDEXED, size = uiNumberOfColours * PALETTE_ELEMENT_SIZE),
// uncompressed *		SubRectInfo's (usNumberOfRects > 0, size = usNumberOfSubRects *
// sizeof(SubRectInfo) ), uncompressed *		Bytes of image data, possibly compressed

#include "SGP/Types.h"

#define STCI_ID_STRING "STCI"
#define STCI_ID_LEN 4

#define STCI_ETRLE_COMPRESSED 0x0020
#define STCI_ZLIB_COMPRESSED 0x0010
#define STCI_INDEXED 0x0008
#define STCI_RGB 0x0004
#define STCI_ALPHA 0x0002
#define STCI_TRANSPARENT 0x0001

// ETRLE defines
#define COMPRESS_TRANSPARENT 0x80
#define COMPRESS_NON_TRANSPARENT 0x00
#define COMPRESS_RUN_LIMIT 0x7F

// NB if you're going to change the header definition:
// - make sure that everything in this header is nicely aligned
// - don't exceed the 64-byte maximum
typedef struct {
  UINT8 cID[STCI_ID_LEN];
  uint32_t uiOriginalSize;
  uint32_t uiStoredSize;  // equal to uiOriginalSize if data uncompressed
  uint32_t uiTransparentValue;
  uint32_t fFlags;
  uint16_t usHeight;
  uint16_t usWidth;
  union {
    struct {
      uint32_t uiRedMask;
      uint32_t uiGreenMask;
      uint32_t uiBlueMask;
      uint32_t uiAlphaMask;
      UINT8 ubRedDepth;
      UINT8 ubGreenDepth;
      UINT8 ubBlueDepth;
      UINT8 ubAlphaDepth;
    } RGB;
    struct {  // For indexed files, the palette will contain 3 separate bytes for red, green, and
              // blue
      uint32_t uiNumberOfColours;
      uint16_t usNumberOfSubImages;
      UINT8 ubRedDepth;
      UINT8 ubGreenDepth;
      UINT8 ubBlueDepth;
      UINT8 cIndexedUnused[11];
    } Indexed;
  };
  UINT8 ubDepth;  // size in bits of one pixel as stored in the file
  uint32_t uiAppDataSize;
  UINT8 cUnused[15];
} STCIHeader;

#define STCI_HEADER_SIZE 64

typedef struct {
  uint32_t uiDataOffset;
  uint32_t uiDataLength;
  INT16 sOffsetX;
  INT16 sOffsetY;
  uint16_t usHeight;
  uint16_t usWidth;
} STCISubImage;

#define STCI_SUBIMAGE_SIZE 16

typedef struct {
  UINT8 ubRed;
  UINT8 ubGreen;
  UINT8 ubBlue;
} STCIPaletteElement;

#define STCI_PALETTE_ELEMENT_SIZE 3
#define STCI_8BIT_PALETTE_SIZE 768

#endif
