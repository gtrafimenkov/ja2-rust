#include "SGP/HImage.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "SGP/Debug.h"
#include "SGP/FileMan.h"
#include "SGP/ImpTGA.h"
#include "SGP/PCX.h"
#include "SGP/STCI.h"
#include "SGP/Types.h"
#include "SGP/VObject.h"
#include "SGP/WCheck.h"
#include "StrUtils.h"
#include "platform_strings.h"

// This is the color substituted to keep a 24bpp -> 16bpp color
// from going transparent (0x0000) -- DB

#define BLACK_SUBSTITUTE 0x0001

uint16_t gusAlphaMask = 0;
uint16_t gusRedMask = 0;
uint16_t gusGreenMask = 0;
uint16_t gusBlueMask = 0;
int16_t gusRedShift = 0;
int16_t gusBlueShift = 0;
int16_t gusGreenShift = 0;

// this funky union is used for fast 16-bit pixel format conversions
typedef union {
  struct {
    uint16_t usLower;
    uint16_t usHigher;
  };
  uint32_t uiValue;
} SplitUINT32;

HIMAGE CreateImage(const char *ImageFile, uint16_t fContents) {
  HIMAGE hImage = NULL;
  SGPFILENAME Extension;
  CHAR8 ExtensionSep[] = ".";
  STR StrPtr;
  uint32_t iFileLoader;
  SGPFILENAME imageFileCopy;

  strcopy(imageFileCopy, ARR_SIZE(imageFileCopy), ImageFile);

  // Depending on extension of filename, use different image readers
  // Get extension
  StrPtr = strstr(imageFileCopy, ExtensionSep);

  if (StrPtr == NULL) {
    // No extension given, use default internal loader extension
    DbgMessage(TOPIC_HIMAGE, DBG_LEVEL_2, "No extension given, using default");
    strcat(imageFileCopy, ".PCX");
    strcpy(Extension, ".PCX");
  } else {
    strcpy(Extension, StrPtr + 1);
  }

  // Determine type from Extension
  do {
    iFileLoader = UNKNOWN_FILE_READER;

    if (strcasecmp(Extension, "PCX") == 0) {
      iFileLoader = PCX_FILE_READER;
      break;
    }

    if (strcasecmp(Extension, "TGA") == 0) {
      iFileLoader = TGA_FILE_READER;
      break;
    }

    if (strcasecmp(Extension, "STI") == 0) {
      iFileLoader = STCI_FILE_READER;
      break;
    }

  } while (FALSE);

  // Determine if resource exists before creating image structure
  if (!FileMan_Exists(imageFileCopy)) {
    DbgMessage(TOPIC_HIMAGE, DBG_LEVEL_2,
               String("Resource file %s does not exist.", imageFileCopy));
    return (NULL);
  }

  // Create memory for image structure
  hImage = (HIMAGE)MemAlloc(sizeof(image_type));

  AssertMsg(hImage, "Failed to allocate memory for hImage in CreateImage");
  // Initialize some values
  memset(hImage, 0, sizeof(image_type));

  // hImage->fFlags = 0;
  // Set data pointers to NULL
  // hImage->pImageData = NULL;
  // hImage->pPalette   = NULL;
  // hImage->pui16BPPPalette = NULL;

  // Set filename and loader
  strcpy(hImage->ImageFile, imageFileCopy);
  hImage->iFileLoader = iFileLoader;

  if (!LoadImageData(hImage, fContents)) {
    return (NULL);
  }

  // All is fine, image is loaded and allocated, return pointer
  return (hImage);
}

BOOLEAN DestroyImage(HIMAGE hImage) {
  Assert(hImage != NULL);

  // First delete contents
  ReleaseImageData(hImage, IMAGE_ALLDATA);  // hImage->fFlags );

  // Now free structure
  MemFree(hImage);

  return (TRUE);
}

BOOLEAN ReleaseImageData(HIMAGE hImage, uint16_t fContents) {
  Assert(hImage != NULL);

  if ((fContents & IMAGE_PALETTE) && (hImage->fFlags & IMAGE_PALETTE)) {
    // Destroy palette
    if (hImage->pPalette != NULL) {
      MemFree(hImage->pPalette);
      hImage->pPalette = NULL;
    }

    if (hImage->pui16BPPPalette != NULL) {
      MemFree(hImage->pui16BPPPalette);
      hImage->pui16BPPPalette = NULL;
    }

    // Remove contents flag
    hImage->fFlags = hImage->fFlags ^ IMAGE_PALETTE;
  }

  if ((fContents & IMAGE_BITMAPDATA) && (hImage->fFlags & IMAGE_BITMAPDATA)) {
    // Destroy image data
    Assert(hImage->pImageData != NULL);
    MemFree(hImage->pImageData);
    hImage->pImageData = NULL;
    if (hImage->usNumberOfObjects > 0) {
      MemFree(hImage->pETRLEObject);
    }
    // Remove contents flag
    hImage->fFlags = hImage->fFlags ^ IMAGE_BITMAPDATA;
  }

  if ((fContents & IMAGE_APPDATA) && (hImage->fFlags & IMAGE_APPDATA)) {
    // get rid of the APP DATA
    if (hImage->pAppData != NULL) {
      MemFree(hImage->pAppData);
      hImage->fFlags &= (~IMAGE_APPDATA);
    }
  }

  return (TRUE);
}

BOOLEAN LoadImageData(HIMAGE hImage, uint16_t fContents) {
  BOOLEAN fReturnVal = FALSE;

  Assert(hImage != NULL);

  // Switch on file loader
  switch (hImage->iFileLoader) {
    case TGA_FILE_READER:

      fReturnVal = LoadTGAFileToImage(hImage, fContents);
      break;

    case PCX_FILE_READER:

      fReturnVal = LoadPCXFileToImage(hImage, fContents);
      break;

    case STCI_FILE_READER:
      fReturnVal = LoadSTCIFileToImage(hImage, fContents);
      break;

    default:

      DbgMessage(TOPIC_HIMAGE, DBG_LEVEL_2, "Unknown image loader was specified.");
  }

  if (!fReturnVal) {
    DbgMessage(TOPIC_HIMAGE, DBG_LEVEL_2, "Error occured while reading image data.");
  }

  return (fReturnVal);
}

BOOLEAN CopyImageToBuffer(HIMAGE hImage, uint32_t fBufferType, BYTE *pDestBuf, uint16_t usDestWidth,
                          uint16_t usDestHeight, uint16_t usX, uint16_t usY, SGPRect *srcRect) {
  // Use blitter based on type of image
  Assert(hImage != NULL);

  if (hImage->ubBitDepth == 8 && fBufferType == BUFFER_8BPP) {
    // Default do here
    DbgMessage(TOPIC_HIMAGE, DBG_LEVEL_2, "Copying 8 BPP Imagery.");
    return (
        Copy8BPPImageTo8BPPBuffer(hImage, pDestBuf, usDestWidth, usDestHeight, usX, usY, srcRect));
  }

  if (hImage->ubBitDepth == 8 && fBufferType == BUFFER_16BPP) {
    // Default do here
    DbgMessage(TOPIC_HIMAGE, DBG_LEVEL_3, "Copying 8 BPP Imagery to 16BPP Buffer.");
    return (
        Copy8BPPImageTo16BPPBuffer(hImage, pDestBuf, usDestWidth, usDestHeight, usX, usY, srcRect));
  }

  if (hImage->ubBitDepth == 16 && fBufferType == BUFFER_16BPP) {
    DbgMessage(TOPIC_HIMAGE, DBG_LEVEL_3, "Automatically Copying 16 BPP Imagery.");
    return (Copy16BPPImageTo16BPPBuffer(hImage, pDestBuf, usDestWidth, usDestHeight, usX, usY,
                                        srcRect));
  }

  return (FALSE);
}

BOOLEAN Copy8BPPImageTo8BPPBuffer(HIMAGE hImage, BYTE *pDestBuf, uint16_t usDestWidth,
                                  uint16_t usDestHeight, uint16_t usX, uint16_t usY,
                                  SGPRect *srcRect) {
  uint32_t uiSrcStart, uiDestStart, uiNumLines, uiLineSize;
  uint32_t cnt;
  uint8_t *pDest, *pSrc;

  // Assertions
  Assert(hImage != NULL);
  Assert(hImage->p16BPPData != NULL);

  // Validations
  CHECKF(usX >= 0);
  CHECKF(usX < usDestWidth);
  CHECKF(usY >= 0);
  CHECKF(usY < usDestHeight);
  CHECKF(srcRect->iRight > srcRect->iLeft);
  CHECKF(srcRect->iBottom > srcRect->iTop);

  // Determine memcopy coordinates
  uiSrcStart = srcRect->iTop * hImage->usWidth + srcRect->iLeft;
  uiDestStart = usY * usDestWidth + usX;
  uiNumLines = (srcRect->iBottom - srcRect->iTop) + 1;
  uiLineSize = (srcRect->iRight - srcRect->iLeft) + 1;

  Assert(usDestWidth >= uiLineSize);
  Assert(usDestHeight >= uiNumLines);

  // Copy line by line
  pDest = (uint8_t *)pDestBuf + uiDestStart;
  pSrc = hImage->p8BPPData + uiSrcStart;

  for (cnt = 0; cnt < uiNumLines - 1; cnt++) {
    memcpy(pDest, pSrc, uiLineSize);
    pDest += usDestWidth;
    pSrc += hImage->usWidth;
  }
  // Do last line
  memcpy(pDest, pSrc, uiLineSize);

  return (TRUE);
}

BOOLEAN Copy16BPPImageTo16BPPBuffer(HIMAGE hImage, BYTE *pDestBuf, uint16_t usDestWidth,
                                    uint16_t usDestHeight, uint16_t usX, uint16_t usY,
                                    SGPRect *srcRect) {
  uint32_t uiSrcStart, uiDestStart, uiNumLines, uiLineSize;
  uint32_t cnt;
  uint16_t *pDest, *pSrc;

  Assert(hImage != NULL);
  Assert(hImage->p16BPPData != NULL);

  // Validations
  CHECKF(usX >= 0);
  CHECKF(usX < hImage->usWidth);
  CHECKF(usY >= 0);
  CHECKF(usY < hImage->usHeight);
  CHECKF(srcRect->iRight > srcRect->iLeft);
  CHECKF(srcRect->iBottom > srcRect->iTop);

  // Determine memcopy coordinates
  uiSrcStart = srcRect->iTop * hImage->usWidth + srcRect->iLeft;
  uiDestStart = usY * usDestWidth + usX;
  uiNumLines = (srcRect->iBottom - srcRect->iTop) + 1;
  uiLineSize = (srcRect->iRight - srcRect->iLeft) + 1;

  CHECKF(usDestWidth >= uiLineSize);
  CHECKF(usDestHeight >= uiNumLines);

  // Copy line by line
  pDest = (uint16_t *)pDestBuf + uiDestStart;
  pSrc = hImage->p16BPPData + uiSrcStart;

  for (cnt = 0; cnt < uiNumLines - 1; cnt++) {
    memcpy(pDest, pSrc, uiLineSize * 2);
    pDest += usDestWidth;
    pSrc += hImage->usWidth;
  }
  // Do last line
  memcpy(pDest, pSrc, uiLineSize * 2);

  return (TRUE);
}

BOOLEAN Extract8BPPCompressedImageToBuffer(HIMAGE hImage, BYTE *pDestBuf) { return (FALSE); }

BOOLEAN Extract16BPPCompressedImageToBuffer(HIMAGE hImage, BYTE *pDestBuf) { return (FALSE); }

BOOLEAN Copy8BPPImageTo16BPPBuffer(HIMAGE hImage, BYTE *pDestBuf, uint16_t usDestWidth,
                                   uint16_t usDestHeight, uint16_t usX, uint16_t usY,
                                   SGPRect *srcRect) {
  uint32_t uiSrcStart, uiDestStart, uiNumLines, uiLineSize;
  uint32_t rows, cols;
  uint8_t *pSrc, *pSrcTemp;
  uint16_t *pDest, *pDestTemp;
  uint16_t *p16BPPPalette;

  p16BPPPalette = hImage->pui16BPPPalette;

  // Assertions
  Assert(p16BPPPalette != NULL);
  Assert(hImage != NULL);

  // Validations
  CHECKF(hImage->p16BPPData != NULL);
  CHECKF(usX >= 0);
  CHECKF(usX < usDestWidth);
  CHECKF(usY >= 0);
  CHECKF(usY < usDestHeight);
  CHECKF(srcRect->iRight > srcRect->iLeft);
  CHECKF(srcRect->iBottom > srcRect->iTop);

  // Determine memcopy coordinates
  uiSrcStart = srcRect->iTop * hImage->usWidth + srcRect->iLeft;
  uiDestStart = usY * usDestWidth + usX;
  uiNumLines = (srcRect->iBottom - srcRect->iTop);
  uiLineSize = (srcRect->iRight - srcRect->iLeft);

  CHECKF(usDestWidth >= uiLineSize);
  CHECKF(usDestHeight >= uiNumLines);

  // Convert to Pixel specification
  pDest = (uint16_t *)pDestBuf + uiDestStart;
  pSrc = hImage->p8BPPData + uiSrcStart;
  DbgMessage(TOPIC_HIMAGE, DBG_LEVEL_3, String("Start Copying at %p", pDest));

  // For every entry, look up into 16BPP palette
  for (rows = 0; rows < uiNumLines - 1; rows++) {
    pDestTemp = pDest;
    pSrcTemp = pSrc;

    for (cols = 0; cols < uiLineSize; cols++) {
      *pDestTemp = p16BPPPalette[*pSrcTemp];
      pDestTemp++;
      pSrcTemp++;
    }

    pDest += usDestWidth;
    pSrc += hImage->usWidth;
  }
  // Do last line
  DbgMessage(TOPIC_HIMAGE, DBG_LEVEL_3, String("End Copying at %p", pDest));

  return (TRUE);
}

uint16_t *Create16BPPPalette(struct SGPPaletteEntry *pPalette) {
  uint16_t *p16BPPPalette, r16, g16, b16, usColor;
  uint32_t cnt;
  uint8_t r, g, b;

  Assert(pPalette != NULL);

  p16BPPPalette = (uint16_t *)MemAlloc(sizeof(uint16_t) * 256);

  for (cnt = 0; cnt < 256; cnt++) {
    r = pPalette[cnt].peRed;
    g = pPalette[cnt].peGreen;
    b = pPalette[cnt].peBlue;

    if (gusRedShift < 0)
      r16 = ((uint16_t)r >> abs(gusRedShift));
    else
      r16 = ((uint16_t)r << gusRedShift);

    if (gusGreenShift < 0)
      g16 = ((uint16_t)g >> abs(gusGreenShift));
    else
      g16 = ((uint16_t)g << gusGreenShift);

    if (gusBlueShift < 0)
      b16 = ((uint16_t)b >> abs(gusBlueShift));
    else
      b16 = ((uint16_t)b << gusBlueShift);

    usColor = (r16 & gusRedMask) | (g16 & gusGreenMask) | (b16 & gusBlueMask);

    if (usColor == 0) {
      if ((r + g + b) != 0) usColor = BLACK_SUBSTITUTE | gusAlphaMask;
    } else
      usColor |= gusAlphaMask;

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
      lumin = (pPalette[cnt].peRed * 299 / 1000) + (pPalette[cnt].peGreen * 587 / 1000) +
              (pPalette[cnt].peBlue * 114 / 1000);
      rmod = (rscale * lumin) / 256;
      gmod = (gscale * lumin) / 256;
      bmod = (bscale * lumin) / 256;
    } else {
      rmod = (rscale * pPalette[cnt].peRed / 256);
      gmod = (gscale * pPalette[cnt].peGreen / 256);
      bmod = (bscale * pPalette[cnt].peBlue / 256);
    }

    r = (uint8_t)min(rmod, 255);
    g = (uint8_t)min(gmod, 255);
    b = (uint8_t)min(bmod, 255);

    if (gusRedShift < 0)
      r16 = ((uint16_t)r >> (-gusRedShift));
    else
      r16 = ((uint16_t)r << gusRedShift);

    if (gusGreenShift < 0)
      g16 = ((uint16_t)g >> (-gusGreenShift));
    else
      g16 = ((uint16_t)g << gusGreenShift);

    if (gusBlueShift < 0)
      b16 = ((uint16_t)b >> (-gusBlueShift));
    else
      b16 = ((uint16_t)b << gusBlueShift);

    // Prevent creation of pure black color
    usColor = (r16 & gusRedMask) | (g16 & gusGreenMask) | (b16 & gusBlueMask);

    if (usColor == 0) {
      if ((r + g + b) != 0) usColor = BLACK_SUBSTITUTE | gusAlphaMask;
    } else
      usColor |= gusAlphaMask;

    p16BPPPalette[cnt] = usColor;
  }
  return (p16BPPPalette);
}

// Convert from RGB to 16 bit value
uint16_t Get16BPPColor(uint32_t RGBValue) {
  uint16_t r16, g16, b16, usColor;
  uint8_t r, g, b;

  r = SGPGetRValue(RGBValue);
  g = SGPGetGValue(RGBValue);
  b = SGPGetBValue(RGBValue);

  if (gusRedShift < 0)
    r16 = ((uint16_t)r >> abs(gusRedShift));
  else
    r16 = ((uint16_t)r << gusRedShift);

  if (gusGreenShift < 0)
    g16 = ((uint16_t)g >> abs(gusGreenShift));
  else
    g16 = ((uint16_t)g << gusGreenShift);

  if (gusBlueShift < 0)
    b16 = ((uint16_t)b >> abs(gusBlueShift));
  else
    b16 = ((uint16_t)b << gusBlueShift);

  usColor = (r16 & gusRedMask) | (g16 & gusGreenMask) | (b16 & gusBlueMask);

  // if our color worked out to absolute black, and the original wasn't
  // absolute black, convert it to a VERY dark grey to avoid transparency
  // problems

  if (usColor == 0) {
    if (RGBValue != 0) usColor = BLACK_SUBSTITUTE | gusAlphaMask;
  } else
    usColor |= gusAlphaMask;

  return (usColor);
}

// Convert from 16 BPP to RGBvalue
uint32_t GetRGBColor(uint16_t Value16BPP) {
  uint16_t r16, g16, b16;
  uint32_t r, g, b, val;

  r16 = Value16BPP & gusRedMask;
  g16 = Value16BPP & gusGreenMask;
  b16 = Value16BPP & gusBlueMask;

  if (gusRedShift < 0)
    r = ((uint32_t)r16 << abs(gusRedShift));
  else
    r = ((uint32_t)r16 >> gusRedShift);

  if (gusGreenShift < 0)
    g = ((uint32_t)g16 << abs(gusGreenShift));
  else
    g = ((uint32_t)g16 >> gusGreenShift);

  if (gusBlueShift < 0)
    b = ((uint32_t)b16 << abs(gusBlueShift));
  else
    b = ((uint32_t)b16 >> gusBlueShift);

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

struct SGPPaletteEntry *ConvertRGBToPaletteEntry(uint8_t sbStart, uint8_t sbEnd,
                                                 uint8_t *pOldPalette) {
  uint16_t Index;
  struct SGPPaletteEntry *pPalEntry;
  struct SGPPaletteEntry *pInitEntry;

  pPalEntry = (struct SGPPaletteEntry *)MemAlloc(sizeof(struct SGPPaletteEntry) * 256);
  pInitEntry = pPalEntry;
  DbgMessage(TOPIC_HIMAGE, DBG_LEVEL_0, "Converting RGB palette to struct SGPPaletteEntry");
  for (Index = 0; Index <= (sbEnd - sbStart); Index++) {
    pPalEntry->peRed = *(pOldPalette + (Index * 3));
    pPalEntry->peGreen = *(pOldPalette + (Index * 3) + 1);
    pPalEntry->peBlue = *(pOldPalette + (Index * 3) + 2);
    pPalEntry->peFlags = 0;
    pPalEntry++;
  }
  return pInitEntry;
}

BOOLEAN GetETRLEImageData(HIMAGE hImage, ETRLEData *pBuffer) {
  // Assertions
  Assert(hImage != NULL);
  Assert(pBuffer != NULL);

  // Create memory for data
  pBuffer->usNumberOfObjects = hImage->usNumberOfObjects;

  // Create buffer for objects
  pBuffer->pETRLEObject = (ETRLEObject *)MemAlloc(sizeof(ETRLEObject) * pBuffer->usNumberOfObjects);
  CHECKF(pBuffer->pETRLEObject != NULL);

  // Copy into buffer
  memcpy(pBuffer->pETRLEObject, hImage->pETRLEObject,
         sizeof(ETRLEObject) * pBuffer->usNumberOfObjects);

  // Allocate memory for pixel data
  pBuffer->pPixData = MemAlloc(hImage->uiSizePixData);
  CHECKF(pBuffer->pPixData != NULL);

  pBuffer->uiSizePixData = hImage->uiSizePixData;

  // Copy into buffer
  memcpy(pBuffer->pPixData, hImage->pPixData8, pBuffer->uiSizePixData);

  return (TRUE);
}

void ConvertRGBDistribution565To555(uint16_t *p16BPPData, uint32_t uiNumberOfPixels) {
  uint16_t *pPixel;
  uint32_t uiLoop;

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
      *pPixel = Pixel.usHigher | gusAlphaMask;
    }
    pPixel++;
  }
}

void ConvertRGBDistribution565To655(uint16_t *p16BPPData, uint32_t uiNumberOfPixels) {
  uint16_t *pPixel;
  uint32_t uiLoop;

  SplitUINT32 Pixel;

  pPixel = p16BPPData;
  for (uiLoop = 0; uiLoop < uiNumberOfPixels; uiLoop++) {
    // we put the 16 pixel bits in the UPPER word of uiPixel, so that we can
    // right shift the blue value (at the bottom) into the LOWER word to protect it
    Pixel.usHigher = *pPixel;
    Pixel.uiValue >>= 5;
    // get rid of the least significant bit of green
    Pixel.usHigher >>= 1;
    // shift to the right some more...
    Pixel.uiValue >>= 5;
    // so we can left-shift the red value alone to give it an extra bit
    Pixel.usHigher <<= 1;
    // now shift back and copy
    Pixel.uiValue <<= 10;
    *pPixel = Pixel.usHigher;
    pPixel++;
  }
}

void ConvertRGBDistribution565To556(uint16_t *p16BPPData, uint32_t uiNumberOfPixels) {
  uint16_t *pPixel;
  uint32_t uiLoop;

  SplitUINT32 Pixel;

  pPixel = p16BPPData;
  for (uiLoop = 0; uiLoop < uiNumberOfPixels; uiLoop++) {
    // we put the 16 pixel bits in the UPPER word of uiPixel, so that we can
    // right shift the blue value (at the bottom) into the LOWER word to protect it
    Pixel.usHigher = *pPixel;
    Pixel.uiValue >>= 5;
    // get rid of the least significant bit of green
    Pixel.usHigher >>= 1;
    // shift back into the upper word
    Pixel.uiValue <<= 5;
    // give blue an extra bit (blank in the least significant spot)
    Pixel.usHigher <<= 1;
    // copy back
    *pPixel = Pixel.usHigher;
    pPixel++;
  }
}

void ConvertRGBDistribution565ToAny(uint16_t *p16BPPData, uint32_t uiNumberOfPixels) {
  uint16_t *pPixel;
  uint32_t uiRed, uiGreen, uiBlue, uiTemp, uiLoop;

  pPixel = p16BPPData;
  for (uiLoop = 0; uiLoop < uiNumberOfPixels; uiLoop++) {
    // put the 565 RGB 16-bit value into a 32-bit RGB value
    uiRed = (*pPixel) >> 11;
    uiGreen = (*pPixel & 0x07E0) >> 5;
    uiBlue = (*pPixel & 0x001F);
    uiTemp = FROMRGB(uiRed, uiGreen, uiBlue);
    // then convert the 32-bit RGB value to whatever 16 bit format is used
    *pPixel = Get16BPPColor(uiTemp);
    pPixel++;
  }
}
