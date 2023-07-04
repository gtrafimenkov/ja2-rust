#include "SGP/HImage.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "SGP/Debug.h"
#include "SGP/PCX.h"
#include "SGP/STCI.h"
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
    UINT16 usLower;
    UINT16 usHigher;
  };
  UINT32 uiValue;
} SplitUINT32;

struct Image *CreateImage(const char *ImageFile, bool loadAppData) {
  SGPFILENAME Extension;
  CHAR8 ExtensionSep[] = ".";
  STR StrPtr;
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

  if (image->paletteAllocatedInRust) {
    RustDealloc((uint8_t *)image->palette);
  } else {
    MemFree(image->palette);
  }

  MemFree(image->palette16bpp);

  if (image->imageDataAllocatedInRust) {
    RustDealloc((uint8_t *)image->image_data);
  } else {
    MemFree(image->image_data);
  }
  RustDealloc((uint8_t *)image->subimages);
  RustDealloc(image->app_data);

  MemFree(image);
}

BOOLEAN CopyImageToBuffer(struct Image *hImage, u8 bufferBitDepth, BYTE *pDestBuf,
                          UINT16 usDestWidth, UINT16 usDestHeight, UINT16 usX, UINT16 usY,
                          SGPRect *srcRect) {
  Assert(hImage != NULL);

  if (hImage->ubBitDepth == 8 && bufferBitDepth == 8) {
    DebugMsg(TOPIC_HIMAGE, DBG_NORMAL, "Copying 8 BPP Imagery.");
    return (
        Copy8BPPImageTo8BPPBuffer(hImage, pDestBuf, usDestWidth, usDestHeight, usX, usY, srcRect));
  }

  if (hImage->ubBitDepth == 8 && bufferBitDepth == 16) {
    DebugMsg(TOPIC_HIMAGE, DBG_INFO, "Copying 8 BPP Imagery to 16BPP Buffer.");
    return (
        Copy8BPPImageTo16BPPBuffer(hImage, pDestBuf, usDestWidth, usDestHeight, usX, usY, srcRect));
  }

  if (hImage->ubBitDepth == 16 && bufferBitDepth == 16) {
    DebugMsg(TOPIC_HIMAGE, DBG_INFO, "Automatically Copying 16 BPP Imagery.");
    return (Copy16BPPImageTo16BPPBuffer(hImage, pDestBuf, usDestWidth, usDestHeight, usX, usY,
                                        srcRect));
  }

  return (FALSE);
}

BOOLEAN Copy8BPPImageTo8BPPBuffer(struct Image *hImage, BYTE *pDestBuf, UINT16 usDestWidth,
                                  UINT16 usDestHeight, UINT16 usX, UINT16 usY, SGPRect *srcRect) {
  UINT32 uiSrcStart, uiDestStart, uiNumLines, uiLineSize;
  UINT32 cnt;
  UINT8 *pDest, *pSrc;

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
  uiSrcStart = srcRect->iTop * hImage->usWidth + srcRect->iLeft;
  uiDestStart = usY * usDestWidth + usX;
  uiNumLines = (srcRect->iBottom - srcRect->iTop) + 1;
  uiLineSize = (srcRect->iRight - srcRect->iLeft) + 1;

  Assert(usDestWidth >= uiLineSize);
  Assert(usDestHeight >= uiNumLines);

  // Copy line by line
  pDest = (UINT8 *)pDestBuf + uiDestStart;
  pSrc = (UINT8 *)hImage->image_data + uiSrcStart;

  for (cnt = 0; cnt < uiNumLines - 1; cnt++) {
    memcpy(pDest, pSrc, uiLineSize);
    pDest += usDestWidth;
    pSrc += hImage->usWidth;
  }
  // Do last line
  memcpy(pDest, pSrc, uiLineSize);

  return (TRUE);
}

BOOLEAN Copy16BPPImageTo16BPPBuffer(struct Image *hImage, BYTE *pDestBuf, UINT16 usDestWidth,
                                    UINT16 usDestHeight, UINT16 usX, UINT16 usY, SGPRect *srcRect) {
  UINT32 uiSrcStart, uiDestStart, uiNumLines, uiLineSize;
  UINT32 cnt;
  UINT16 *pDest, *pSrc;

  Assert(hImage != NULL);
  Assert(hImage->image_data != NULL);

  // Validations
  if (!(usX >= 0)) {
    return FALSE;
  }
  if (!(usX < hImage->usWidth)) {
    return FALSE;
  }
  if (!(usY >= 0)) {
    return FALSE;
  }
  if (!(usY < hImage->usHeight)) {
    return FALSE;
  }
  if (!(srcRect->iRight > srcRect->iLeft)) {
    return FALSE;
  }
  if (!(srcRect->iBottom > srcRect->iTop)) {
    return FALSE;
  }

  // Determine memcopy coordinates
  uiSrcStart = srcRect->iTop * hImage->usWidth + srcRect->iLeft;
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
  pDest = (UINT16 *)pDestBuf + uiDestStart;
  pSrc = (UINT16 *)hImage->image_data + uiSrcStart;

  for (cnt = 0; cnt < uiNumLines - 1; cnt++) {
    memcpy(pDest, pSrc, uiLineSize * 2);
    pDest += usDestWidth;
    pSrc += hImage->usWidth;
  }
  // Do last line
  memcpy(pDest, pSrc, uiLineSize * 2);

  return (TRUE);
}

BOOLEAN Copy8BPPImageTo16BPPBuffer(struct Image *hImage, BYTE *pDestBuf, UINT16 usDestWidth,
                                   UINT16 usDestHeight, UINT16 usX, UINT16 usY, SGPRect *srcRect) {
  UINT32 uiSrcStart, uiDestStart, uiNumLines, uiLineSize;
  UINT32 rows, cols;
  UINT8 *pSrc, *pSrcTemp;
  UINT16 *pDest, *pDestTemp;

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
  uiSrcStart = srcRect->iTop * hImage->usWidth + srcRect->iLeft;
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
  pDest = (UINT16 *)pDestBuf + uiDestStart;
  pSrc = (UINT8 *)hImage->image_data + uiSrcStart;
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
    pSrc += hImage->usWidth;
  }
  // Do last line
  DebugMsg(TOPIC_HIMAGE, DBG_INFO, String("End Copying at %p", pDest));

  return (TRUE);
}

UINT16 *Create16BPPPalette(struct SGPPaletteEntry *pPalette) {
  UINT16 *p16BPPPalette, r16, g16, b16, usColor;
  UINT32 cnt;
  UINT8 r, g, b;

  Assert(pPalette != NULL);

  p16BPPPalette = (UINT16 *)MemAlloc(sizeof(UINT16) * 256);

  for (cnt = 0; cnt < 256; cnt++) {
    r = pPalette[cnt].red;
    g = pPalette[cnt].green;
    b = pPalette[cnt].blue;

    r16 = ((UINT16)r << 8);
    g16 = ((UINT16)g << 3);
    b16 = ((UINT16)b >> 3);

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
UINT16 *Create16BPPPaletteShaded(struct SGPPaletteEntry *pPalette, UINT32 rscale, UINT32 gscale,
                                 UINT32 bscale, BOOLEAN mono) {
  UINT16 *p16BPPPalette, r16, g16, b16, usColor;
  UINT32 cnt, lumin;
  UINT32 rmod, gmod, bmod;
  UINT8 r, g, b;

  Assert(pPalette != NULL);

  p16BPPPalette = (UINT16 *)MemAlloc(sizeof(UINT16) * 256);

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

    r = (UINT8)min(rmod, 255);
    g = (UINT8)min(gmod, 255);
    b = (UINT8)min(bmod, 255);

    r16 = ((UINT16)r << 8);
    g16 = ((UINT16)g << 3);
    b16 = ((UINT16)b >> 3);

    // Prevent creation of pure black color
    usColor = (r16 & 0xf800) | (g16 & 0x07e0) | (b16 & 0x001f);

    if (usColor == 0) {
      if ((r + g + b) != 0) usColor = BLACK_SUBSTITUTE;
    }

    p16BPPPalette[cnt] = usColor;
  }
  return (p16BPPPalette);
}

// Convert from RGB to 16 bit value
UINT16 Get16BPPColor(UINT32 RGBValue) {
  UINT16 r16, g16, b16, usColor;
  UINT8 r, g, b;

  r = SGPGetRValue(RGBValue);
  g = SGPGetGValue(RGBValue);
  b = SGPGetBValue(RGBValue);

  r16 = ((UINT16)r << 8);
  g16 = ((UINT16)g << 3);
  b16 = ((UINT16)b >> 3);

  usColor = (r16 & 0xf800) | (g16 & 0x07e0) | (b16 & 0x001f);

  // if our color worked out to absolute black, and the original wasn't
  // absolute black, convert it to a VERY dark grey to avoid transparency
  // problems

  if (usColor == 0) {
    if (RGBValue != 0) usColor = BLACK_SUBSTITUTE;
  }
  return (usColor);
}

// Convert from 16 BPP to RGBvalue
UINT32 GetRGBColor(UINT16 Value16BPP) {
  UINT16 r16, g16, b16;
  UINT32 r, g, b, val;

  r16 = Value16BPP & 0xf800;
  g16 = Value16BPP & 0x07e0;
  b16 = Value16BPP & 0x001f;

  r = ((UINT32)r16 >> 8);

  g = ((UINT32)g16 >> 3);

  b = ((UINT32)b16 << 3);

  r &= 0x000000ff;
  g &= 0x000000ff;
  b &= 0x000000ff;

  val = FROMRGB(r, g, b);

  return (val);
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

struct SGPPaletteEntry *ConvertRGBToPaletteEntry(UINT8 sbStart, UINT8 sbEnd, UINT8 *pOldPalette) {
  UINT16 Index;
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

void ConvertRGBDistribution565To555(UINT16 *p16BPPData, UINT32 uiNumberOfPixels) {
  UINT16 *pPixel;
  UINT32 uiLoop;

  SplitUINT32 Pixel;

  pPixel = p16BPPData;
  for (uiLoop = 0; uiLoop < uiNumberOfPixels; uiLoop++) {
    // If the pixel is completely black, don't bother converting it -- DB
    if (*pPixel != 0) {
      // we put the 16 pixel bits in the UPPER word of uiPixel, so that we can
      // right shift the blue value (at the bottom) into the LOWER word to protect it
      Pixel.usHigher = *pPixel;
      Pixel.uiValue >>= 5;
      // get rid of the least significant bit of green
      Pixel.usHigher >>= 1;
      // now shift back into the upper word
      Pixel.uiValue <<= 5;
      // and copy back
      *pPixel = Pixel.usHigher;
    }
    pPixel++;
  }
}

// Create a scaled down copy of an image.
struct Image *ScaleImageDown2x(struct Image *image) {
  // not all image types are supported
  bool supported =
      image->ubBitDepth == 8 && image->app_data_size == 0 && image->number_of_subimages == 0;
  if (!supported) {
    return NULL;
  }
  struct Image *res = zmalloc(sizeof(struct Image));
  if (!res) {
    return NULL;
  }

  res->usWidth = image->usWidth / 2;
  res->usHeight = image->usHeight / 2;
  res->ubBitDepth = image->ubBitDepth;

  u32 palette_size = sizeof(struct SGPPaletteEntry) * 256;
  res->palette = zmalloc(palette_size);
  if (!res->palette) {
    free(res);
    return NULL;
  }
  memcpy(res->palette, image->palette, palette_size);

  res->image_data_size = res->usWidth * res->usHeight;
  res->image_data = zmalloc(res->image_data_size);
  if (!res->image_data) {
    free(res->palette);
    free(res);
    return NULL;
  }

  u8 *data = res->image_data;
  for (u16 y = 0; y < image->usHeight; y += 2) {
    for (u16 x = 0; x < image->usWidth; x += 2) {
      *data++ = ((u8 *)image->image_data)[y * image->usWidth + x];
    }
  }

  return res;
}
