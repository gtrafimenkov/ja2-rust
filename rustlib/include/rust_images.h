#pragma once

/* Warning, this file is autogenerated by cbindgen. Don't modify this manually. */

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#define STCI_ID_LEN 4

/**
 * Structure that describes one image from an indexed STCI file
 */
struct ETRLEObject {
  uint32_t uiDataOffset;
  uint32_t uiDataLength;
  int16_t sOffsetX;
  int16_t sOffsetY;
  uint16_t usHeight;
  uint16_t usWidth;
};

/**
 * First part of STCI image header
 */
struct STCIHeaderHead {
  uint8_t ID[STCI_ID_LEN];
  uint32_t OriginalSize;
  uint32_t StoredSize;
  uint32_t TransparentValue;
  uint32_t Flags;
  uint16_t Height;
  uint16_t Width;
};

/**
 * Middle part of STCI image header describing RGB image
 */
struct STCIHeaderMiddleRGB {
  uint32_t uiRedMask;
  uint32_t uiGreenMask;
  uint32_t uiBlueMask;
  uint32_t uiAlphaMask;
  uint8_t ubRedDepth;
  uint8_t ubGreenDepth;
  uint8_t ubBlueDepth;
  uint8_t ubAlphaDepth;
};

/**
 * Middle part of STCI image header describing RGB image
 */
struct STCIHeaderMiddleIndexed {
  uint32_t uiNumberOfColours;
  uint16_t usNumberOfSubImages;
  uint8_t ubRedDepth;
  uint8_t ubGreenDepth;
  uint8_t ubBlueDepth;
  uint8_t cIndexedUnused[11];
};

/**
 * Middle part of STCI image header
 */
enum STCIHeaderMiddle_Tag {
  Rgb,
  Indexed,
};

struct STCIHeaderMiddle {
  enum STCIHeaderMiddle_Tag tag;
  union {
    struct {
      struct STCIHeaderMiddleRGB rgb;
    };
    struct {
      struct STCIHeaderMiddleIndexed indexed;
    };
  };
};

/**
 * Last part of STCI image header
 */
struct STCIHeaderEnd {
  uint8_t Depth;
  uint8_t unused1;
  uint8_t unused2;
  uint8_t unused3;
  uint32_t AppDataSize;
  uint8_t Unused[12];
};

/**
 * Last part of STCI image header
 */
struct STCIHeader {
  struct STCIHeaderHead head;
  struct STCIHeaderMiddle middle;
  struct STCIHeaderEnd end;
};

/**
 * Palette structure, mimics that of Win32
 */
struct SGPPaletteEntry {
  uint8_t peRed;
  uint8_t peGreen;
  uint8_t peBlue;
  uint8_t _unused;
};

/**
 * Results of loading STI image.
 */
struct STIImageLoaded {
  bool success;
  uint32_t StoredSize;
  uint16_t Height;
  uint16_t Width;
  uint16_t usNumberOfSubImages;
  uint32_t AppDataSize;
  uint8_t *image_data;
  bool indexed;
  struct SGPPaletteEntry *palette;
  struct ETRLEObject *subimages;
  uint8_t *app_data;
};

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

void TmpImageFunc(struct ETRLEObject _pe);

bool ReadSTCIHeader(FileID file_id, struct STCIHeader *data);

struct STIImageLoaded LoadSTIImage(FileID file_id);

uint8_t *ReadSTCIImageData(FileID file_id, const struct STCIHeader *header);

/**
 * Read STCI indexed image palette from file and return it as SGPPaletteEntry[256] array.
 * If NULL is returned, there was an error reading data from file.
 * Memory must be freed afterwards using RustDealloc function.
 */
struct SGPPaletteEntry *ReadSTCIPalette(FileID file_id);

/**
 * Read STCI indexed image subimages info from the file and return it as ETRLEObject[num_subimages] array.
 * If NULL is returned, there was an error reading data from file.
 * Memory must be freed afterwards using RustDealloc function.
 */
struct ETRLEObject *ReadSTCISubimages(FileID file_id,
                                      uintptr_t num_subimages);

uint8_t *ReadSTCIAppData(FileID file_id, const struct STCIHeader *header);

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus
