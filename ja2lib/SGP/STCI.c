#include <string.h>

#include "SGP/Debug.h"
#include "SGP/HImage.h"
#include "SGP/ImgFmt.h"
#include "SGP/MemMan.h"
#include "SGP/PaletteEntry.h"
#include "SGP/Types.h"
#include "SGP/WCheck.h"
#include "rust_debug.h"
#include "rust_fileman.h"
#include "rust_images.h"

static BOOLEAN STCILoadRGB(struct Image *hImage, UINT16 fContents, FileID hFile,
                           struct STCIHeaderTmp *pHeader);
static BOOLEAN STCILoadIndexed(struct Image *hImage, UINT16 fContents, FileID hFile,
                               struct STCIHeaderTmp *pHeader);
static BOOLEAN STCISetPalette(PTR pSTCIPalette, struct Image *hImage);

BOOLEAN LoadSTCIFileToImage(const char *filePath, struct Image *hImage, UINT16 fContents) {
  FileID hFile = FILE_ID_ERR;
  UINT32 uiBytesRead;
  struct Image TempImage;

  // Check that hImage is valid, and that the file in question exists
  Assert(hImage != NULL);

  TempImage = *hImage;

  if (!(File_Exists(filePath))) {
    return FALSE;
  }

  // Open the file and read the header
  hFile = File_OpenForReading(filePath);
  if (!(hFile)) {
    return FALSE;
  }

  struct STCIHeaderTmp header;
  if (!ReadSTCIHeader(hFile, &header)) {
    DebugMsg(TOPIC_HIMAGE, DBG_INFO, "Problem reading STCI header.");
    File_Close(hFile);
    return (FALSE);
  }

  // Determine from the header the data stored in the file. and run the appropriate loader
  if (header.middle.tag == Rgb) {
    if (!STCILoadRGB(&TempImage, fContents, hFile, &header)) {
      DebugMsg(TOPIC_HIMAGE, DBG_INFO, "Problem loading RGB image.");
      File_Close(hFile);
      return (FALSE);
    }
  } else if (header.middle.tag == Indexed) {
    if (!STCILoadIndexed(&TempImage, fContents, hFile, &header)) {
      DebugMsg(TOPIC_HIMAGE, DBG_INFO, "Problem loading palettized image.");
      File_Close(hFile);
      return (FALSE);
    }
  } else {  // unsupported type of data, or the right flags weren't set!
    DebugMsg(TOPIC_HIMAGE, DBG_INFO, "Unknown data organization in STCI file.");
    File_Close(hFile);
    return (FALSE);
  }

  // Requested data loaded successfully.
  File_Close(hFile);

  // Set some more flags in the temporary image structure, copy it so that hImage points
  // to it, and return.
  if (header.head.Flags & STCI_ZLIB_COMPRESSED) {
    TempImage.fFlags |= IMAGE_COMPRESSED;
  }
  TempImage.usWidth = header.head.Width;
  TempImage.usHeight = header.head.Height;
  TempImage.ubBitDepth = header.end.Depth;
  *hImage = TempImage;

  return (TRUE);
}

static BOOLEAN STCILoadRGB(struct Image *hImage, UINT16 fContents, FileID hFile,
                           struct STCIHeaderTmp *pHeader) {
  UINT32 uiBytesRead;

  if (fContents & IMAGE_PALETTE &&
      !(fContents & IMAGE_ALLIMAGEDATA)) {  // RGB doesn't have a palette!
    return (FALSE);
  }

  if (fContents & IMAGE_BITMAPDATA) {
    // Allocate memory for the image data and read it in
    hImage->pImageData = MemAlloc(pHeader->head.StoredSize);
    if (hImage->pImageData == NULL) {
      return (FALSE);
    } else if (!File_Read(hFile, hImage->pImageData, pHeader->head.StoredSize, &uiBytesRead) ||
               uiBytesRead != pHeader->head.StoredSize) {
      MemFree(hImage->pImageData);
      return (FALSE);
    }

    hImage->fFlags |= IMAGE_BITMAPDATA;

    if (pHeader->end.Depth == 16) {
      // ASSUMPTION: file data is 565 R,G,B

      if (gusRedMask != (UINT16)pHeader->middle.rgb.uiRedMask ||
          gusGreenMask != (UINT16)pHeader->middle.rgb.uiGreenMask ||
          gusBlueMask != (UINT16)pHeader->middle.rgb.uiBlueMask) {
        // colour distribution of the file is different from hardware!  We have to change it!
        DebugMsg(TOPIC_HIMAGE, DBG_INFO, "Converting to current RGB distribution!");
        // Convert the image to the current hardware's specifications
        if (gusRedMask > gusGreenMask && gusGreenMask > gusBlueMask) {
          // hardware wants RGB!
          if (gusRedMask == 0x7C00 && gusGreenMask == 0x03E0 &&
              gusBlueMask == 0x001F) {  // hardware is 555
            ConvertRGBDistribution565To555(hImage->p16BPPData,
                                           pHeader->head.Width * pHeader->head.Height);
            return (TRUE);
          } else if (gusRedMask == 0xFC00 && gusGreenMask == 0x03E0 && gusBlueMask == 0x001F) {
            ConvertRGBDistribution565To655(hImage->p16BPPData,
                                           pHeader->head.Width * pHeader->head.Height);
            return (TRUE);
          } else if (gusRedMask == 0xF800 && gusGreenMask == 0x07C0 && gusBlueMask == 0x003F) {
            ConvertRGBDistribution565To556(hImage->p16BPPData,
                                           pHeader->head.Width * pHeader->head.Height);
            return (TRUE);
          } else {
            // take the long route
            ConvertRGBDistribution565ToAny(hImage->p16BPPData,
                                           pHeader->head.Width * pHeader->head.Height);
            return (TRUE);
          }
        } else {
          // hardware distribution is not R-G-B so we have to take the long route!
          ConvertRGBDistribution565ToAny(hImage->p16BPPData,
                                         pHeader->head.Width * pHeader->head.Height);
          return (TRUE);
        }
      }
    }
  }
  return (TRUE);
}

static BOOLEAN STCILoadIndexed(struct Image *hImage, UINT16 fContents, FileID hFile,
                               struct STCIHeaderTmp *pHeader) {
  UINT32 uiFileSectionSize;
  UINT32 uiBytesRead;
  PTR pSTCIPalette;

  if (fContents & IMAGE_PALETTE) {  // Allocate memory for reading in the palette
    if (pHeader->middle.indexed.uiNumberOfColours != 256) {
      DebugMsg(TOPIC_HIMAGE, DBG_INFO, "Palettized image has bad palette size.");
      return (FALSE);
    }
    uiFileSectionSize = pHeader->middle.indexed.uiNumberOfColours * STCI_PALETTE_ELEMENT_SIZE;
    pSTCIPalette = MemAlloc(uiFileSectionSize);
    if (pSTCIPalette == NULL) {
      DebugMsg(TOPIC_HIMAGE, DBG_INFO, "Out of memory!");
      File_Close(hFile);
      return (FALSE);
    }

    // ATE: Memset: Jan 16/99
    memset(pSTCIPalette, 0, uiFileSectionSize);

    // Read in the palette
    if (!File_Read(hFile, pSTCIPalette, uiFileSectionSize, &uiBytesRead) ||
        uiBytesRead != uiFileSectionSize) {
      DebugMsg(TOPIC_HIMAGE, DBG_INFO, "Problem loading palette!");
      File_Close(hFile);
      MemFree(pSTCIPalette);
      return (FALSE);
    } else if (!STCISetPalette(pSTCIPalette, hImage)) {
      DebugMsg(TOPIC_HIMAGE, DBG_INFO, "Problem setting hImage-format palette!");
      File_Close(hFile);
      MemFree(pSTCIPalette);
      return (FALSE);
    }
    hImage->fFlags |= IMAGE_PALETTE;
    // Free the temporary buffer
    MemFree(pSTCIPalette);
  } else if (fContents & (IMAGE_BITMAPDATA | IMAGE_APPDATA)) {  // seek past the palette
    uiFileSectionSize = pHeader->middle.indexed.uiNumberOfColours * STCI_PALETTE_ELEMENT_SIZE;
    if (File_Seek(hFile, uiFileSectionSize, FILE_SEEK_CURRENT) == FALSE) {
      DebugMsg(TOPIC_HIMAGE, DBG_INFO, "Problem seeking past palette!");
      File_Close(hFile);
      return (FALSE);
    }
  }
  if (fContents & IMAGE_BITMAPDATA) {
    if (pHeader->head.Flags & STCI_ETRLE_COMPRESSED) {
      // load data for the subimage (object) structures
      Assert(sizeof(ETRLEObject) == STCI_SUBIMAGE_SIZE);
      hImage->usNumberOfObjects = pHeader->middle.indexed.usNumberOfSubImages;
      uiFileSectionSize = hImage->usNumberOfObjects * STCI_SUBIMAGE_SIZE;
      hImage->pETRLEObject = (ETRLEObject *)MemAlloc(uiFileSectionSize);
      if (hImage->pETRLEObject == NULL) {
        DebugMsg(TOPIC_HIMAGE, DBG_INFO, "Out of memory!");
        File_Close(hFile);
        if (fContents & IMAGE_PALETTE) {
          MemFree(hImage->pPalette);
        }
        return (FALSE);
      }
      if (!File_Read(hFile, hImage->pETRLEObject, uiFileSectionSize, &uiBytesRead) ||
          uiBytesRead != uiFileSectionSize) {
        DebugMsg(TOPIC_HIMAGE, DBG_INFO, "Error loading subimage structures!");
        File_Close(hFile);
        if (fContents & IMAGE_PALETTE) {
          MemFree(hImage->pPalette);
        }
        MemFree(hImage->pETRLEObject);
        return (FALSE);
      }
      hImage->uiSizePixData = pHeader->head.StoredSize;
      hImage->fFlags |= IMAGE_TRLECOMPRESSED;
    }
    // allocate memory for and read in the image data
    hImage->pImageData = MemAlloc(pHeader->head.StoredSize);
    if (hImage->pImageData == NULL) {
      DebugMsg(TOPIC_HIMAGE, DBG_INFO, "Out of memory!");
      File_Close(hFile);
      if (fContents & IMAGE_PALETTE) {
        MemFree(hImage->pPalette);
      }
      if (hImage->usNumberOfObjects > 0) {
        MemFree(hImage->pETRLEObject);
      }
      return (FALSE);
    } else if (!File_Read(hFile, hImage->pImageData, pHeader->head.StoredSize, &uiBytesRead) ||
               uiBytesRead != pHeader->head.StoredSize) {  // Problem reading in the image data!
      DebugMsg(TOPIC_HIMAGE, DBG_INFO, "Error loading image data!");
      File_Close(hFile);
      MemFree(hImage->pImageData);
      if (fContents & IMAGE_PALETTE) {
        MemFree(hImage->pPalette);
      }
      if (hImage->usNumberOfObjects > 0) {
        MemFree(hImage->pETRLEObject);
      }
      return (FALSE);
    }
    hImage->fFlags |= IMAGE_BITMAPDATA;
  } else if (fContents & IMAGE_APPDATA)  // then there's a point in seeking ahead
  {
    if (File_Seek(hFile, pHeader->head.StoredSize, FILE_SEEK_CURRENT) == FALSE) {
      DebugMsg(TOPIC_HIMAGE, DBG_INFO, "Problem seeking past image data!");
      File_Close(hFile);
      return (FALSE);
    }
  }

  if (fContents & IMAGE_APPDATA && pHeader->end.AppDataSize > 0) {
    // load application-specific data
    hImage->pAppData = (UINT8 *)MemAlloc(pHeader->end.AppDataSize);
    if (hImage->pAppData == NULL) {
      DebugMsg(TOPIC_HIMAGE, DBG_INFO, "Out of memory!");
      File_Close(hFile);
      MemFree(hImage->pAppData);
      if (fContents & IMAGE_PALETTE) {
        MemFree(hImage->pPalette);
      }
      if (fContents & IMAGE_BITMAPDATA) {
        MemFree(hImage->pImageData);
      }
      if (hImage->usNumberOfObjects > 0) {
        MemFree(hImage->pETRLEObject);
      }
      return (FALSE);
    }
    if (!File_Read(hFile, hImage->pAppData, pHeader->end.AppDataSize, &uiBytesRead) ||
        uiBytesRead != pHeader->end.AppDataSize) {
      DebugMsg(TOPIC_HIMAGE, DBG_INFO, "Error loading application-specific data!");
      File_Close(hFile);
      MemFree(hImage->pAppData);
      if (fContents & IMAGE_PALETTE) {
        MemFree(hImage->pPalette);
      }
      if (fContents & IMAGE_BITMAPDATA) {
        MemFree(hImage->pImageData);
      }
      if (hImage->usNumberOfObjects > 0) {
        MemFree(hImage->pETRLEObject);
      }
      return (FALSE);
    }
    hImage->uiAppDataSize = pHeader->end.AppDataSize;
    ;
    hImage->fFlags |= IMAGE_APPDATA;
  } else {
    hImage->pAppData = NULL;
    hImage->uiAppDataSize = 0;
  }
  return (TRUE);
}

static BOOLEAN STCISetPalette(PTR pSTCIPalette, struct Image *hImage) {
  UINT16 usIndex;
  STCIPaletteElement *pubPalette;

  pubPalette = (STCIPaletteElement *)pSTCIPalette;

  // Allocate memory for palette
  hImage->pPalette = (struct SGPPaletteEntry *)MemAlloc(sizeof(struct SGPPaletteEntry) * 256);
  memset(hImage->pPalette, 0, (sizeof(struct SGPPaletteEntry) * 256));

  if (hImage->pPalette == NULL) {
    return (FALSE);
  }

  // Initialize the proper palette entries
  for (usIndex = 0; usIndex < 256; usIndex++) {
    hImage->pPalette[usIndex].peRed = pubPalette->ubRed;
    hImage->pPalette[usIndex].peGreen = pubPalette->ubGreen;
    hImage->pPalette[usIndex].peBlue = pubPalette->ubBlue;
    hImage->pPalette[usIndex].peFlags = 0;
    pubPalette++;
  }
  return TRUE;
}
