#include "SGP/HImage.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "SGP/Debug.h"
#include "SGP/PCX.h"
#include "SGP/Types.h"
#include "SGP/VObject.h"
#include "SGP/WCheck.h"
#include "StrUtils.h"
#include "platform_strings.h"
#include "rust_alloc.h"
#include "rust_fileman.h"
#include "rust_images.h"

// This is the color substituted to keep a 24bpp -> 16bpp color
// from going transparent (0x0000) -- DB

#define BLACK_SUBSTITUTE 0x0001

// this funky union is used for fast 16-bit pixel format conversions
typedef union {
  struct {
    uint16_t usLower;
    uint16_t usHigher;
  };
  uint32_t uiValue;
} SplitUINT32;

struct Image *CreateImage(const char *ImageFile, bool loadAppData) {
  SGPFILENAME Extension;
  char ExtensionSep[] = ".";
  char *StrPtr;
  SGPFILENAME imageFileCopy;

  strcopy(imageFileCopy, ARR_SIZE(imageFileCopy), ImageFile);

  // Depending on extension of filename, use different image readers
  // Get extension
  StrPtr = strstr(imageFileCopy, ExtensionSep);

  if (StrPtr == NULL) {
    // No extension given, use default internal loader extension
    DebugMsg(TOPIC_HIMAGE, DBG_NORMAL, "No extension given, using default");
    strcat(imageFileCopy, ".PCX");
    strcpy(Extension, ".PCX");
  } else {
    strcpy(Extension, StrPtr + 1);
  }

  // Determine if resource exists before creating image structure
  if (!File_Exists(imageFileCopy)) {
    DebugMsg(TOPIC_HIMAGE, DBG_NORMAL, String("Resource file %s does not exist.", imageFileCopy));
    return (NULL);
  }

  if (strcasecmp(Extension, "PCX") == 0) {
    struct Image *hImage = (struct Image *)MemAlloc(sizeof(struct Image));
    memset(hImage, 0, sizeof(struct Image));
    if (!LoadPCXFileToImage(imageFileCopy, hImage)) {
      DebugLogWrite("failed to load PCX image");
      return NULL;
    }
    DebugLogWrite("PCX image loaded");
    return (hImage);
  } else if (strcasecmp(Extension, "STI") == 0) {
    return LoadSTCIFileToImage(imageFileCopy, loadAppData);
  } else {
    DebugMsg(TOPIC_HIMAGE, DBG_NORMAL, "Unknown image loader was specified.");
    return NULL;
  }
}

void DestroyImage(struct Image *image) {
  if (!image) {
    return;
  }

  MemFree(image->palette);
  MemFree(image->palette16bpp);
  MemFree(image->image_data);
  MemFree(image->subimages);
  MemFree(image->app_data);
  MemFree(image);
}

BOOLEAN CopyImageToBuffer(struct Image *hImage, uint8_t bufferBitDepth, uint8_t *pDestBuf,
                          uint16_t usDestWidth, uint16_t usDestHeight, uint16_t usX, uint16_t usY,
                          struct GRect *srcRect) {
  Assert(hImage != NULL);

  if (hImage->bit_depth == 8 && bufferBitDepth == 8) {
    DebugMsg(TOPIC_HIMAGE, DBG_NORMAL, "Copying 8 BPP Imagery.");
    return (
        Copy8BPPImageTo8BPPBuffer(hImage, pDestBuf, usDestWidth, usDestHeight, usX, usY, srcRect));
  }

  if (hImage->bit_depth == 8 && bufferBitDepth == 16) {
    DebugMsg(TOPIC_HIMAGE, DBG_INFO, "Copying 8 BPP Imagery to 16BPP Buffer.");
    return (
        Copy8BPPImageTo16BPPBuffer(hImage, pDestBuf, usDestWidth, usDestHeight, usX, usY, srcRect));
  }

  if (hImage->bit_depth == 16 && bufferBitDepth == 16) {
    DebugMsg(TOPIC_HIMAGE, DBG_INFO, "Automatically Copying 16 BPP Imagery.");
    return (Copy16BPPImageTo16BPPBuffer(hImage, pDestBuf, usDestWidth, usDestHeight, usX, usY,
                                        srcRect));
  }

  return (FALSE);
}

BOOLEAN Copy8BPPImageTo8BPPBuffer(struct Image *hImage, uint8_t *pDestBuf, uint16_t usDestWidth,
                                  uint16_t usDestHeight, uint16_t usX, uint16_t usY,
                                  struct GRect *srcRect) {
  uint32_t uiSrcStart, uiDestStart, uiNumLines, uiLineSize;
  uint32_t cnt;
  uint8_t *pDest, *pSrc;

  // Assertions
  Assert(hImage != NULL);
  Assert(hImage->image_data != NULL);

  // Validations
  if (!(usX >= 0)) {
    return FALSE;
  }
  if (!(usX < usDestWidth)) {
    return FALSE;
  }
  if (!(usY >= 0)) {
    return FALSE;
  }
  if (!(usY < usDestHeight)) {
    return FALSE;
  }
  if (!(srcRect->iRight > srcRect->iLeft)) {
    return FALSE;
  }
  if (!(srcRect->iBottom > srcRect->iTop)) {
    return FALSE;
  }

  // Determine memcopy coordinates
  uiSrcStart = srcRect->iTop * hImage->width + srcRect->iLeft;
  uiDestStart = usY * usDestWidth + usX;
  uiNumLines = (srcRect->iBottom - srcRect->iTop) + 1;
  uiLineSize = (srcRect->iRight - srcRect->iLeft) + 1;

  Assert(usDestWidth >= uiLineSize);
  Assert(usDestHeight >= uiNumLines);

  // Copy line by line
  pDest = (uint8_t *)pDestBuf + uiDestStart;
  pSrc = (uint8_t *)hImage->image_data + uiSrcStart;

  for (cnt = 0; cnt < uiNumLines - 1; cnt++) {
    memcpy(pDest, pSrc, uiLineSize);
    pDest += usDestWidth;
    pSrc += hImage->width;
  }
  // Do last line
  memcpy(pDest, pSrc, uiLineSize);

  return (TRUE);
}

BOOLEAN Copy16BPPImageTo16BPPBuffer(struct Image *hImage, uint8_t *pDestBuf, uint16_t usDestWidth,
                                    uint16_t usDestHeight, uint16_t usX, uint16_t usY,
                                    struct GRect *srcRect) {
  uint32_t uiSrcStart, uiDestStart, uiNumLines, uiLineSize;
  uint32_t cnt;
  uint16_t *pDest, *pSrc;

  Assert(hImage != NULL);
  Assert(hImage->image_data != NULL);

  // Validations
  if (!(usX >= 0)) {
    return FALSE;
  }
  if (!(usX < hImage->width)) {
    return FALSE;
  }
  if (!(usY >= 0)) {
    return FALSE;
  }
  if (!(usY < hImage->height)) {
    return FALSE;
  }
  if (!(srcRect->iRight > srcRect->iLeft)) {
    return FALSE;
  }
  if (!(srcRect->iBottom > srcRect->iTop)) {
    return FALSE;
  }

  // Determine memcopy coordinates
  uiSrcStart = srcRect->iTop * hImage->width + srcRect->iLeft;
  uiDestStart = usY * usDestWidth + usX;
  uiNumLines = (srcRect->iBottom - srcRect->iTop) + 1;
  uiLineSize = (srcRect->iRight - srcRect->iLeft) + 1;

  if (!(usDestWidth >= uiLineSize)) {
    return FALSE;
  }
  if (!(usDestHeight >= uiNumLines)) {
    return FALSE;
  }

  // Copy line by line
  pDest = (uint16_t *)pDestBuf + uiDestStart;
  pSrc = (uint16_t *)hImage->image_data + uiSrcStart;

  for (cnt = 0; cnt < uiNumLines - 1; cnt++) {
    memcpy(pDest, pSrc, uiLineSize * 2);
    pDest += usDestWidth;
    pSrc += hImage->width;
  }
  // Do last line
  memcpy(pDest, pSrc, uiLineSize * 2);

  return (TRUE);
}

BOOLEAN Copy8BPPImageTo16BPPBuffer(struct Image *hImage, uint8_t *pDestBuf, uint16_t usDestWidth,
                                   uint16_t usDestHeight, uint16_t usX, uint16_t usY,
                                   struct GRect *srcRect) {
  uint32_t uiSrcStart, uiDestStart, uiNumLines, uiLineSize;
  uint32_t rows, cols;
  uint8_t *pSrc, *pSrcTemp;
  uint16_t *pDest, *pDestTemp;

  Assert(hImage->palette16bpp != NULL);
  Assert(hImage != NULL);

  // Validations
  if (!(hImage->image_data != NULL)) {
    return FALSE;
  }
  if (!(usX >= 0)) {
    return FALSE;
  }
  if (!(usX < usDestWidth)) {
    return FALSE;
  }
  if (!(usY >= 0)) {
    return FALSE;
  }
  if (!(usY < usDestHeight)) {
    return FALSE;
  }
  if (!(srcRect->iRight > srcRect->iLeft)) {
    return FALSE;
  }
  if (!(srcRect->iBottom > srcRect->iTop)) {
    return FALSE;
  }

  // Determine memcopy coordinates
  uiSrcStart = srcRect->iTop * hImage->width + srcRect->iLeft;
  uiDestStart = usY * usDestWidth + usX;
  uiNumLines = (srcRect->iBottom - srcRect->iTop);
  uiLineSize = (srcRect->iRight - srcRect->iLeft);

  if (!(usDestWidth >= uiLineSize)) {
    return FALSE;
  }
  if (!(usDestHeight >= uiNumLines)) {
    return FALSE;
  }

  // Convert to Pixel specification
  pDest = (uint16_t *)pDestBuf + uiDestStart;
  pSrc = (uint8_t *)hImage->image_data + uiSrcStart;
  DebugMsg(TOPIC_HIMAGE, DBG_INFO, String("Start Copying at %p", pDest));

  // For every entry, look up into 16BPP palette
  for (rows = 0; rows < uiNumLines - 1; rows++) {
    pDestTemp = pDest;
    pSrcTemp = pSrc;

    for (cols = 0; cols < uiLineSize; cols++) {
      *pDestTemp = hImage->palette16bpp[*pSrcTemp];
      pDestTemp++;
      pSrcTemp++;
    }

    pDest += usDestWidth;
    pSrc += hImage->width;
  }
  // Do last line
  DebugMsg(TOPIC_HIMAGE, DBG_INFO, String("End Copying at %p", pDest));

  return (TRUE);
}

uint16_t *Create16BPPPalette(struct SGPPaletteEntry *pPalette) {
  uint16_t *p16BPPPalette, r16, g16, b16, usColor;
  uint32_t cnt;
  uint8_t r, g, b;

  Assert(pPalette != NULL);

  p16BPPPalette = (uint16_t *)MemAlloc(sizeof(uint16_t) * 256);

  for (cnt = 0; cnt < 256; cnt++) {
    r = pPalette[cnt].red;
    g = pPalette[cnt].green;
    b = pPalette[cnt].blue;

    r16 = ((uint16_t)r << 8);
    g16 = ((uint16_t)g << 3);
    b16 = ((uint16_t)b >> 3);

    usColor = (r16 & 0xf800) | (g16 & 0x07e0) | (b16 & 0x001f);

    if (usColor == 0) {
      if ((r + g + b) != 0) usColor = BLACK_SUBSTITUTE;
    }

    p16BPPPalette[cnt] = usColor;
  }

  return (p16BPPPalette);
}

/**********************************************************************************************
 Create16BPPPaletteShaded

        Creates an 8 bit to 16 bit palette table, and modifies the colors as it builds.

        Parameters:
                rscale, gscale, bscale:
                                Color mode: Percentages (255=100%) of color to translate into
destination palette. Mono mode:  Color for monochrome palette. mono: TRUE or FALSE to create a
monochrome palette. In mono mode, Luminance values for colors are calculated, and the RGB color is
shaded according to each pixel's brightness.

        This can be used in several ways:

        1) To "brighten" a palette, pass down RGB values that are higher than 100% ( > 255) for all
                        three. mono=FALSE.
        2) To "darken" a palette, do the same with less than 100% ( < 255) values. mono=FALSE.

        3) To create a "glow" palette, select mono=TRUE, and pass the color in the RGB parameters.

        4) For gamma correction, pass in weighted values for each color.

**********************************************************************************************/
uint16_t *Create16BPPPaletteShaded(struct SGPPaletteEntry *pPalette, uint32_t rscale,
                                   uint32_t gscale, uint32_t bscale, BOOLEAN mono) {
  uint16_t *p16BPPPalette, r16, g16, b16, usColor;
  uint32_t cnt, lumin;
  uint32_t rmod, gmod, bmod;
  uint8_t r, g, b;

  Assert(pPalette != NULL);

  p16BPPPalette = (uint16_t *)MemAlloc(sizeof(uint16_t) * 256);

  for (cnt = 0; cnt < 256; cnt++) {
    if (mono) {
      lumin = (pPalette[cnt].red * 299 / 1000) + (pPalette[cnt].green * 587 / 1000) +
              (pPalette[cnt].blue * 114 / 1000);
      rmod = (rscale * lumin) / 256;
      gmod = (gscale * lumin) / 256;
      bmod = (bscale * lumin) / 256;
    } else {
      rmod = (rscale * pPalette[cnt].red / 256);
      gmod = (gscale * pPalette[cnt].green / 256);
      bmod = (bscale * pPalette[cnt].blue / 256);
    }

    r = (uint8_t)min(rmod, 255);
    g = (uint8_t)min(gmod, 255);
    b = (uint8_t)min(bmod, 255);

    r16 = ((uint16_t)r << 8);
    g16 = ((uint16_t)g << 3);
    b16 = ((uint16_t)b >> 3);

    // Prevent creation of pure black color
    usColor = (r16 & 0xf800) | (g16 & 0x07e0) | (b16 & 0x001f);

    if (usColor == 0) {
      if ((r + g + b) != 0) usColor = BLACK_SUBSTITUTE;
    }

    p16BPPPalette[cnt] = usColor;
  }
  return (p16BPPPalette);
}

//*****************************************************************************
//
// ConvertToPaletteEntry
//
// Parameter List : Converts from RGB to struct SGPPaletteEntry
//
// Return Value  pointer to the struct SGPPaletteEntry
//
// Modification History :
// Dec 15th 1996 -> modified for use by Wizardry
//
//*****************************************************************************

struct SGPPaletteEntry *ConvertRGBToPaletteEntry(uint8_t sbStart, uint8_t sbEnd,
                                                 uint8_t *pOldPalette) {
  uint16_t Index;
  struct SGPPaletteEntry *pPalEntry;
  struct SGPPaletteEntry *pInitEntry;

  pPalEntry = (struct SGPPaletteEntry *)MemAlloc(sizeof(struct SGPPaletteEntry) * 256);
  pInitEntry = pPalEntry;
  DebugMsg(TOPIC_HIMAGE, DBG_ERROR, "Converting RGB palette to struct SGPPaletteEntry");
  for (Index = 0; Index <= (sbEnd - sbStart); Index++) {
    pPalEntry->red = *(pOldPalette + (Index * 3));
    pPalEntry->green = *(pOldPalette + (Index * 3) + 1);
    pPalEntry->blue = *(pOldPalette + (Index * 3) + 2);
    pPalEntry->_unused = 0;
    pPalEntry++;
  }
  return pInitEntry;
}

BOOLEAN CopyImageData(struct Image *hImage, struct ImageData *pBuffer) {
  Assert(hImage != NULL);
  Assert(pBuffer != NULL);

  // Create memory for data
  pBuffer->number_of_subimages = hImage->number_of_subimages;

  // Create buffer for objects
  pBuffer->subimages =
      (struct Subimage *)MemAlloc(sizeof(struct Subimage) * pBuffer->number_of_subimages);
  if (!(pBuffer->subimages != NULL)) {
    return FALSE;
  }

  // Copy into buffer
  memcpy(pBuffer->subimages, hImage->subimages,
         sizeof(struct Subimage) * pBuffer->number_of_subimages);

  // Allocate memory for pixel data
  pBuffer->image_data = MemAlloc(hImage->image_data_size);
  if (!(pBuffer->image_data != NULL)) {
    return FALSE;
  }

  pBuffer->image_data_size = hImage->image_data_size;

  // Copy into buffer
  memcpy(pBuffer->image_data, hImage->image_data, pBuffer->image_data_size);

  return (TRUE);
}

// Create a scaled down copy of an image.
struct Image *ScaleImageDown2x(struct Image *image) {
  // not all image types are supported
  bool supported =
      image->bit_depth == 8 && image->app_data_size == 0 && image->number_of_subimages == 0;
  if (!supported) {
    return NULL;
  }
  struct Image *res = zmalloc(sizeof(struct Image));
  if (!res) {
    return NULL;
  }

  res->width = image->width / 2;
  res->height = image->height / 2;
  res->bit_depth = image->bit_depth;

  uint32_t palette_size = sizeof(struct SGPPaletteEntry) * 256;
  res->palette = zmalloc(palette_size);
  if (!res->palette) {
    MemFree(res);
    return NULL;
  }
  memcpy(res->palette, image->palette, palette_size);

  res->image_data_size = res->width * res->height;
  res->image_data = zmalloc(res->image_data_size);
  if (!res->image_data) {
    MemFree(res->palette);
    MemFree(res);
    return NULL;
  }

  uint8_t *data = res->image_data;
  for (uint16_t y = 0; y < image->height; y += 2) {
    for (uint16_t x = 0; x < image->width; x += 2) {
      *data++ = ((uint8_t *)image->image_data)[y * image->width + x];
    }
  }

  return res;
}
