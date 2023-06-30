#if !defined(STCI_H)

#define STCI_H

/*
Sir-Tech's Crazy Image (STCI) file format specifications.  Each file is composed of:
- ImageFileHeader, uncompressed
- Palette (STCI_INDEXED, size = uiNumberOfColours * PALETTE_ELEMENT_SIZE), uncompressed
- SubRectInfo's (usNumberOfRects > 0, size = usNumberOfSubRects * sizeof(SubRectInfo)), uncompressed
- Bytes of image data, possibly compressed
*/

#include "SGP/Types.h"
#include "rust_images.h"

#define STCI_ID_STRING "STCI"

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
  struct STCIHeaderHead head;
  // 24 bytes
  union {
    struct STCIHeaderMiddleRGB RGB;
    struct STCIHeaderMiddleIndexed Indexed;
  };
  struct STCIHeaderEnd end;
} STCIHeader;

#define STCI_HEADER_SIZE 64

typedef struct {
  UINT32 uiDataOffset;
  UINT32 uiDataLength;
  INT16 sOffsetX;
  INT16 sOffsetY;
  UINT16 usHeight;
  UINT16 usWidth;
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
