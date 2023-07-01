#include "SGP/HImage.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "SGP/Debug.h"
#include "SGP/ImpTGA.h"
#include "SGP/PCX.h"
#include "SGP/PaletteEntry.h"
#include "SGP/STCI.h"
#include "SGP/Types.h"
#include "SGP/VObject.h"
#include "SGP/WCheck.h"
#include "StrUtils.h"
#include "platform_strings.h"
#include "rust_alloc.h"
#include "rust_fileman.h"

// This is the color substituted to keep a 24bpp -> 16bpp color
// from going transparent (0x0000) -- DB

#define BLACK_SUBSTITUTE 0x0001

UINT16 gusAlphaMask = 0;

// this funky union is used for fast 16-bit pixel format conversions
typedef union {
  struct {
    UINT16 usLower;
    UINT16 usHigher;
  };
  UINT32 uiValue;
} SplitUINT32;

// This function will attept to Load data from an existing image object's filename
// In this way, dynamic loading of image data can be done
static BOOLEAN LoadImageData(const char *filePath, u32 fileLoader, struct Image *hImage,
                             bool loadAppData);

struct Image *CreateImage(const char *ImageFile, bool loadAppData) {
  struct Image *hImage = NULL;
  SGPFILENAME Extension;
  CHAR8 ExtensionSep[] = ".";
  STR StrPtr;
  UINT32 iFileLoader;
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
  if (!File_Exists(imageFileCopy)) {
    DebugMsg(TOPIC_HIMAGE, DBG_NORMAL, String("Resource file %s does not exist.", imageFileCopy));
    return (NULL);
  }

  // Create memory for image structure
  hImage = (struct Image *)MemAlloc(sizeof(struct Image));

  AssertMsg(hImage, "Failed to allocate memory for hImage in CreateImage");
  // Initialize some values
  memset(hImage, 0, sizeof(struct Image));

  if (!LoadImageData(imageFileCopy, iFileLoader, hImage, loadAppData)) {
    return (NULL);
  }

  // All is fine, image is loaded and allocated, return pointer
  return (hImage);
}

BOOLEAN DestroyImage(struct Image *hImage) {
  Assert(hImage != NULL);

  // First delete contents
  ReleaseImageData(hImage);

  // Now free structure
  MemFree(hImage);

  return (TRUE);
}

void FreeImageData(struct Image *image) {
  if (image->imageDataAllocatedInRust) {
    RustDealloc(image->pImageData);
  } else {
    MemFree(image->pImageData);
  }
}

BOOLEAN ReleaseImageData(struct Image *hImage) {
  Assert(hImage != NULL);

  if (hImage->fFlags & IMAGE_PALETTE) {
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

  if (hImage->fFlags & IMAGE_BITMAPDATA) {
    // Destroy image data
    Assert(hImage->pImageData != NULL);
    FreeImageData(hImage);
    hImage->pImageData = NULL;
    if (hImage->usNumberOfObjects > 0) {
      MemFree(hImage->pETRLEObject);
    }
    // Remove contents flag
    hImage->fFlags = hImage->fFlags ^ IMAGE_BITMAPDATA;
  }

  if (hImage->fFlags & IMAGE_APPDATA) {
    // get rid of the APP DATA
    if (hImage->pAppData != NULL) {
      MemFree(hImage->pAppData);
      hImage->fFlags &= (~IMAGE_APPDATA);
    }
  }

  return (TRUE);
}

static BOOLEAN LoadImageData(const char *filePath, u32 fileLoader, struct Image *hImage,
                             bool loadAppData) {
  BOOLEAN fReturnVal = FALSE;

  Assert(hImage != NULL);

  // Switch on file loader
  switch (fileLoader) {
    case TGA_FILE_READER:
      fReturnVal = LoadTGAFileToImage(filePath, hImage);
      break;

    case PCX_FILE_READER:
      fReturnVal = LoadPCXFileToImage(filePath, hImage);
      break;

    case STCI_FILE_READER:
      fReturnVal = LoadSTCIFileToImage(filePath, hImage, loadAppData);
      break;

    default:
      DebugMsg(TOPIC_HIMAGE, DBG_NORMAL, "Unknown image loader was specified.");
  }

  if (!fReturnVal) {
    DebugMsg(TOPIC_HIMAGE, DBG_NORMAL, "Error occured while reading image data.");
  }

  return (fReturnVal);
}

BOOLEAN CopyImageToBuffer(struct Image *hImage, UINT32 fBufferType, BYTE *pDestBuf,
                          UINT16 usDestWidth, UINT16 usDestHeight, UINT16 usX, UINT16 usY,
                          SGPRect *srcRect) {
  // Use blitter based on type of image
  Assert(hImage != NULL);

  if (hImage->ubBitDepth == 8 && fBufferType == BUFFER_8BPP) {
    // Default do here
    DebugMsg(TOPIC_HIMAGE, DBG_NORMAL, "Copying 8 BPP Imagery.");
    return (
        Copy8BPPImageTo8BPPBuffer(hImage, pDestBuf, usDestWidth, usDestHeight, usX, usY, srcRect));
  }

  if (hImage->ubBitDepth == 8 && fBufferType == BUFFER_16BPP) {
    // Default do here
    DebugMsg(TOPIC_HIMAGE, DBG_INFO, "Copying 8 BPP Imagery to 16BPP Buffer.");
    return (
        Copy8BPPImageTo16BPPBuffer(hImage, pDestBuf, usDestWidth, usDestHeight, usX, usY, srcRect));
  }

  if (hImage->ubBitDepth == 16 && fBufferType == BUFFER_16BPP) {
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
  Assert(hImage->p16BPPData != NULL);

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

BOOLEAN Copy16BPPImageTo16BPPBuffer(struct Image *hImage, BYTE *pDestBuf, UINT16 usDestWidth,
                                    UINT16 usDestHeight, UINT16 usX, UINT16 usY, SGPRect *srcRect) {
  UINT32 uiSrcStart, uiDestStart, uiNumLines, uiLineSize;
  UINT32 cnt;
  UINT16 *pDest, *pSrc;

  Assert(hImage != NULL);
  Assert(hImage->p16BPPData != NULL);

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

BOOLEAN Extract8BPPCompressedImageToBuffer(struct Image *hImage, BYTE *pDestBuf) { return (FALSE); }

BOOLEAN Extract16BPPCompressedImageToBuffer(struct Image *hImage, BYTE *pDestBuf) {
  return (FALSE);
}

BOOLEAN Copy8BPPImageTo16BPPBuffer(struct Image *hImage, BYTE *pDestBuf, UINT16 usDestWidth,
                                   UINT16 usDestHeight, UINT16 usX, UINT16 usY, SGPRect *srcRect) {
  UINT32 uiSrcStart, uiDestStart, uiNumLines, uiLineSize;
  UINT32 rows, cols;
  UINT8 *pSrc, *pSrcTemp;
  UINT16 *pDest, *pDestTemp;
  UINT16 *p16BPPPalette;

  p16BPPPalette = hImage->pui16BPPPalette;

  // Assertions
  Assert(p16BPPPalette != NULL);
  Assert(hImage != NULL);

  // Validations
  if (!(hImage->p16BPPData != NULL)) {
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
  pSrc = hImage->p8BPPData + uiSrcStart;
  DebugMsg(TOPIC_HIMAGE, DBG_INFO, String("Start Copying at %p", pDest));

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
    r = pPalette[cnt].peRed;
    g = pPalette[cnt].peGreen;
    b = pPalette[cnt].peBlue;

    r16 = ((UINT16)r << 8);
    g16 = ((UINT16)g << 3);
    b16 = ((UINT16)b >> 3);

    usColor = (r16 & 0xf800) | (g16 & 0x07e0) | (b16 & 0x001f);

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

    r = (UINT8)min(rmod, 255);
    g = (UINT8)min(gmod, 255);
    b = (UINT8)min(bmod, 255);

    r16 = ((UINT16)r << 8);
    g16 = ((UINT16)g << 3);
    b16 = ((UINT16)b >> 3);

    // Prevent creation of pure black color
    usColor = (r16 & 0xf800) | (g16 & 0x07e0) | (b16 & 0x001f);

    if (usColor == 0) {
      if ((r + g + b) != 0) usColor = BLACK_SUBSTITUTE | gusAlphaMask;
    } else
      usColor |= gusAlphaMask;

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
    if (RGBValue != 0) usColor = BLACK_SUBSTITUTE | gusAlphaMask;
  } else
    usColor |= gusAlphaMask;

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
    pPalEntry->peRed = *(pOldPalette + (Index * 3));
    pPalEntry->peGreen = *(pOldPalette + (Index * 3) + 1);
    pPalEntry->peBlue = *(pOldPalette + (Index * 3) + 2);
    pPalEntry->peFlags = 0;
    pPalEntry++;
  }
  return pInitEntry;
}

BOOLEAN GetETRLEImageData(struct Image *hImage, ETRLEData *pBuffer) {
  // Assertions
  Assert(hImage != NULL);
  Assert(pBuffer != NULL);

  // Create memory for data
  pBuffer->usNumberOfObjects = hImage->usNumberOfObjects;

  // Create buffer for objects
  pBuffer->pETRLEObject = (ETRLEObject *)MemAlloc(sizeof(ETRLEObject) * pBuffer->usNumberOfObjects);
  if (!(pBuffer->pETRLEObject != NULL)) {
    return FALSE;
  }

  // Copy into buffer
  memcpy(pBuffer->pETRLEObject, hImage->pETRLEObject,
         sizeof(ETRLEObject) * pBuffer->usNumberOfObjects);

  // Allocate memory for pixel data
  pBuffer->pPixData = MemAlloc(hImage->uiSizePixData);
  if (!(pBuffer->pPixData != NULL)) {
    return FALSE;
  }

  pBuffer->uiSizePixData = hImage->uiSizePixData;

  // Copy into buffer
  memcpy(pBuffer->pPixData, hImage->pPixData8, pBuffer->uiSizePixData);

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
      *pPixel = Pixel.usHigher | gusAlphaMask;
    }
    pPixel++;
  }
}
