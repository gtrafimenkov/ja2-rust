#include "SGP/PCX.h"

#include <stdarg.h>
#include <stdio.h>

#include "SGP/MemMan.h"
#include "rust_debug.h"
#include "rust_fileman.h"
#include "rust_images.h"

// Local typedefs

#define PCX_NORMAL 1
#define PCX_RLE 2
#define PCX_256COLOR 4
#define PCX_X_CLIPPING 64
#define PCX_Y_CLIPPING 128

typedef struct {
  uint8_t ubManufacturer;
  uint8_t ubVersion;
  uint8_t ubEncoding;
  uint8_t ubBitsPerPixel;
  uint16_t usLeft, usTop;
  uint16_t usRight, usBottom;
  uint16_t usHorRez, usVerRez;
  uint8_t ubEgaPalette[48];
  uint8_t ubReserved;
  uint8_t ubColorPlanes;
  uint16_t usBytesPerLine;
  uint16_t usPaletteType;
  uint8_t ubFiller[58];
} PcxHeader;

typedef struct {
  uint8_t *pPcxBuffer;
  uint8_t ubPalette[768];
  uint16_t usWidth, usHeight;
  uint32_t uiBufferSize;
  uint16_t usPcxFlags;
} PcxObject;

static BOOLEAN SetPcxPalette(PcxObject *pCurrentPcxObject, struct Image *hImage);
static BOOLEAN BlitPcxToBuffer(PcxObject *pCurrentPcxObject, uint8_t *pBuffer,
                               uint16_t usBufferWidth, uint16_t usBufferHeight, uint16_t usX,
                               uint16_t usY, BOOLEAN fTransp);
static PcxObject *LoadPcx(const char *pFilename);

BOOLEAN LoadPCXFileToImage(const char *filePath, struct Image *hImage) {
  PcxObject *pPcxObject;

  // First Load a PCX Image
  pPcxObject = LoadPcx(filePath);

  if (pPcxObject == NULL) {
    return (FALSE);
  }

  // Set some header information
  hImage->usWidth = pPcxObject->usWidth;
  hImage->usHeight = pPcxObject->usHeight;
  hImage->ubBitDepth = 8;

  hImage->image_data = MemAlloc(hImage->usWidth * hImage->usHeight);

  if (!BlitPcxToBuffer(pPcxObject, (uint8_t *)hImage->image_data, hImage->usWidth, hImage->usHeight,
                       0, 0, FALSE)) {
    MemFree(hImage->image_data);
    return (FALSE);
  }

  SetPcxPalette(pPcxObject, hImage);

  hImage->palette16bpp = Create16BPPPalette(hImage->palette);

  MemFree(pPcxObject->pPcxBuffer);
  MemFree(pPcxObject);

  // {
  //   char buf[256];
  //   snprintf(buf, ARR_SIZE(buf), "pcx %p, %d, %d", hImage->app_data, hImage->app_data_size,
  //            hImage->number_of_subimages);
  //   DebugLogWrite(buf);
  // }

  return (TRUE);
}

PcxObject *LoadPcx(const char *pFilename) {
  PcxHeader Header;
  PcxObject *pCurrentPcxObject;
  FileID hFileHandle = FILE_ID_ERR;
  uint32_t uiFileSize;
  uint8_t *pPcxBuffer;

  // Open and read in the file
  if ((hFileHandle = File_OpenForReading(pFilename)) == 0) {  // damn we failed to open the file
    return NULL;
  }

  uiFileSize = File_GetSize(hFileHandle);
  if (uiFileSize == 0) {  // we failed to size up the file
    return NULL;
  }

  // Create enw pCX object
  pCurrentPcxObject = (PcxObject *)MemAlloc(sizeof(PcxObject));

  if (pCurrentPcxObject == NULL) {
    return (NULL);
  }

  pCurrentPcxObject->pPcxBuffer = (uint8_t *)MemAlloc(uiFileSize - (sizeof(PcxHeader) + 768));

  if (pCurrentPcxObject->pPcxBuffer == NULL) {
    return (NULL);
  }

  // Ok we now have a file handle, so let's read in the data
  File_Read(hFileHandle, &Header, sizeof(PcxHeader), NULL);
  if ((Header.ubManufacturer != 10) || (Header.ubEncoding != 1)) {  // We have an invalid pcx format
    // Delete the object
    MemFree(pCurrentPcxObject->pPcxBuffer);
    MemFree(pCurrentPcxObject);
    return (NULL);
  }

  if (Header.ubBitsPerPixel == 8) {
    pCurrentPcxObject->usPcxFlags = PCX_256COLOR;
  } else {
    pCurrentPcxObject->usPcxFlags = 0;
  }

  pCurrentPcxObject->usWidth = 1 + (Header.usRight - Header.usLeft);
  pCurrentPcxObject->usHeight = 1 + (Header.usBottom - Header.usTop);
  pCurrentPcxObject->uiBufferSize = uiFileSize - 768 - sizeof(PcxHeader);

  // We are ready to read in the pcx buffer data. Therefore we must lock the buffer
  pPcxBuffer = pCurrentPcxObject->pPcxBuffer;

  File_Read(hFileHandle, pPcxBuffer, pCurrentPcxObject->uiBufferSize, NULL);

  // Read in the palette
  File_Read(hFileHandle, &(pCurrentPcxObject->ubPalette[0]), 768, NULL);

  // Close file
  File_Close(hFileHandle);

  return pCurrentPcxObject;
}

static BOOLEAN BlitPcxToBuffer(PcxObject *pCurrentPcxObject, uint8_t *pBuffer,
                               uint16_t usBufferWidth, uint16_t usBufferHeight, uint16_t usX,
                               uint16_t usY, BOOLEAN fTransp) {
  uint8_t *pPcxBuffer;
  uint8_t ubRepCount;
  uint16_t usMaxX, usMaxY;
  uint32_t uiImageSize;
  uint8_t ubCurrentByte = 0;
  uint8_t ubMode;
  uint16_t usCurrentX, usCurrentY;
  uint32_t uiOffset, uiIndex;
  uint32_t uiNextLineOffset, uiStartOffset, uiCurrentOffset;

  pPcxBuffer = pCurrentPcxObject->pPcxBuffer;

  if (((pCurrentPcxObject->usWidth + usX) == usBufferWidth) &&
      ((pCurrentPcxObject->usHeight + usY) ==
       usBufferHeight)) {  // Pre-compute PCX blitting aspects.
    uiImageSize = usBufferWidth * usBufferHeight;
    ubMode = PCX_NORMAL;
    uiOffset = 0;
    ubRepCount = 0;

    // Blit Pcx object. Two main cases, one for transparency (0's are skipped and for without
    // transparency.
    if (fTransp == TRUE) {
      for (uiIndex = 0; uiIndex < uiImageSize; uiIndex++) {
        if (ubMode == PCX_NORMAL) {
          ubCurrentByte = *(pPcxBuffer + uiOffset++);
          if (ubCurrentByte > 0x0BF) {
            ubRepCount = ubCurrentByte & 0x03F;
            ubCurrentByte = *(pPcxBuffer + uiOffset++);
            if (--ubRepCount > 0) {
              ubMode = PCX_RLE;
            }
          }
        } else {
          if (--ubRepCount == 0) {
            ubMode = PCX_NORMAL;
          }
        }
        if (ubCurrentByte != 0) {
          *(pBuffer + uiIndex) = ubCurrentByte;
        }
      }
    } else {
      for (uiIndex = 0; uiIndex < uiImageSize; uiIndex++) {
        if (ubMode == PCX_NORMAL) {
          ubCurrentByte = *(pPcxBuffer + uiOffset++);
          if (ubCurrentByte > 0x0BF) {
            ubRepCount = ubCurrentByte & 0x03F;
            ubCurrentByte = *(pPcxBuffer + uiOffset++);
            if (--ubRepCount > 0) {
              ubMode = PCX_RLE;
            }
          }
        } else {
          if (--ubRepCount == 0) {
            ubMode = PCX_NORMAL;
          }
        }
        *(pBuffer + uiIndex) = ubCurrentByte;
      }
    }
  } else {  // Pre-compute PCX blitting aspects.
    if ((pCurrentPcxObject->usWidth + usX) >= usBufferWidth) {
      pCurrentPcxObject->usPcxFlags |= PCX_X_CLIPPING;
      usMaxX = usBufferWidth - 1;
    } else {
      usMaxX = pCurrentPcxObject->usWidth + usX;
    }

    if ((pCurrentPcxObject->usHeight + usY) >= usBufferHeight) {
      pCurrentPcxObject->usPcxFlags |= PCX_Y_CLIPPING;
      uiImageSize = pCurrentPcxObject->usWidth * (usBufferHeight - usY);
      usMaxY = usBufferHeight - 1;
    } else {
      uiImageSize = pCurrentPcxObject->usWidth * pCurrentPcxObject->usHeight;
      usMaxY = pCurrentPcxObject->usHeight + usY;
    }

    ubMode = PCX_NORMAL;
    uiOffset = 0;
    ubRepCount = 0;
    usCurrentX = usX;
    usCurrentY = usY;

    // Blit Pcx object. Two main cases, one for transparency (0's are skipped and for without
    // transparency.
    if (fTransp == TRUE) {
      for (uiIndex = 0; uiIndex < uiImageSize; uiIndex++) {
        if (ubMode == PCX_NORMAL) {
          ubCurrentByte = *(pPcxBuffer + uiOffset++);
          if (ubCurrentByte > 0x0BF) {
            ubRepCount = ubCurrentByte & 0x03F;
            ubCurrentByte = *(pPcxBuffer + uiOffset++);
            if (--ubRepCount > 0) {
              ubMode = PCX_RLE;
            }
          }
        } else {
          if (--ubRepCount == 0) {
            ubMode = PCX_NORMAL;
          }
        }
        if (ubCurrentByte != 0) {
          *(pBuffer + (usCurrentY * usBufferWidth) + usCurrentX) = ubCurrentByte;
        }
        usCurrentX++;
        if (usCurrentX > usMaxX) {
          usCurrentX = usX;
          usCurrentY++;
        }
      }
    } else {
      uiStartOffset = (usCurrentY * usBufferWidth) + usCurrentX;
      uiNextLineOffset = uiStartOffset + usBufferWidth;
      uiCurrentOffset = uiStartOffset;

      for (uiIndex = 0; uiIndex < uiImageSize; uiIndex++) {
        if (ubMode == PCX_NORMAL) {
          ubCurrentByte = *(pPcxBuffer + uiOffset++);
          if (ubCurrentByte > 0x0BF) {
            ubRepCount = ubCurrentByte & 0x03F;
            ubCurrentByte = *(pPcxBuffer + uiOffset++);
            if (--ubRepCount > 0) {
              ubMode = PCX_RLE;
            }
          }
        } else {
          if (--ubRepCount == 0) {
            ubMode = PCX_NORMAL;
          }
        }

        if (usCurrentX <
            usMaxX) {  // We are within the visible bounds so we write the byte to buffer
          *(pBuffer + uiCurrentOffset) = ubCurrentByte;
          uiCurrentOffset++;
          usCurrentX++;
        } else {
          if ((uiCurrentOffset + 1) < uiNextLineOffset) {  // Increment the uiCurrentOffset
            uiCurrentOffset++;
          } else {  // Go to next line
            usCurrentX = usX;
            usCurrentY++;
            if (usCurrentY > usMaxY) {
              break;
            }
            uiStartOffset = (usCurrentY * usBufferWidth) + usCurrentX;
            uiNextLineOffset = uiStartOffset + usBufferWidth;
            uiCurrentOffset = uiStartOffset;
          }
        }
      }
    }
  }

  return (TRUE);
}

static BOOLEAN SetPcxPalette(PcxObject *pCurrentPcxObject, struct Image *hImage) {
  uint16_t Index;
  uint8_t *pubPalette;

  pubPalette = &(pCurrentPcxObject->ubPalette[0]);

  // Allocate memory for palette
  hImage->palette = (struct SGPPaletteEntry *)MemAlloc(sizeof(struct SGPPaletteEntry) * 256);

  if (hImage->palette == NULL) {
    return (FALSE);
  }

  // Initialize the proper palette entries
  for (Index = 0; Index < 256; Index++) {
    hImage->palette[Index].red = *(pubPalette + (Index * 3));
    hImage->palette[Index].green = *(pubPalette + (Index * 3) + 1);
    hImage->palette[Index].blue = *(pubPalette + (Index * 3) + 2);
    hImage->palette[Index]._unused = 0;
  }

  return TRUE;
}
