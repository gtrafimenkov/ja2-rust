#include <string.h>

#include "SGP/Debug.h"
#include "SGP/FileMan.h"
#include "SGP/HImage.h"
#include "SGP/ImgFmt.h"
#include "SGP/MemMan.h"
#include "SGP/Types.h"
#include "SGP/WCheck.h"

BOOLEAN STCILoadRGB(HIMAGE hImage, uint16_t fContents, HWFILE hFile, STCIHeader *pHeader);
BOOLEAN STCILoadIndexed(HIMAGE hImage, uint16_t fContents, HWFILE hFile, STCIHeader *pHeader);
BOOLEAN STCISetPalette(void *pSTCIPalette, HIMAGE hImage);

BOOLEAN LoadSTCIFileToImage(HIMAGE hImage, uint16_t fContents) {
  HWFILE hFile;
  STCIHeader Header;
  uint32_t uiBytesRead;
  image_type TempImage;

  // Check that hImage is valid, and that the file in question exists
  Assert(hImage != NULL);

  TempImage = *hImage;

  CHECKF(FileMan_Exists(TempImage.ImageFile));

  // Open the file and read the header
  hFile = FileMan_Open(TempImage.ImageFile, FILE_ACCESS_READ, FALSE);
  CHECKF(hFile);

  if (!FileMan_Read(hFile, &Header, STCI_HEADER_SIZE, &uiBytesRead) ||
      uiBytesRead != STCI_HEADER_SIZE || memcmp(Header.cID, STCI_ID_STRING, STCI_ID_LEN) != 0) {
    DbgMessage(TOPIC_HIMAGE, DBG_LEVEL_3, "Problem reading STCI header.");
    FileMan_Close(hFile);
    return (FALSE);
  }

  // Determine from the header the data stored in the file. and run the appropriate loader
  if (Header.fFlags & STCI_RGB) {
    if (!STCILoadRGB(&TempImage, fContents, hFile, &Header)) {
      DbgMessage(TOPIC_HIMAGE, DBG_LEVEL_3, "Problem loading RGB image.");
      FileMan_Close(hFile);
      return (FALSE);
    }
  } else if (Header.fFlags & STCI_INDEXED) {
    if (!STCILoadIndexed(&TempImage, fContents, hFile, &Header)) {
      DbgMessage(TOPIC_HIMAGE, DBG_LEVEL_3, "Problem loading palettized image.");
      FileMan_Close(hFile);
      return (FALSE);
    }
  } else {  // unsupported type of data, or the right flags weren't set!
    DbgMessage(TOPIC_HIMAGE, DBG_LEVEL_3, "Unknown data organization in STCI file.");
    FileMan_Close(hFile);
    return (FALSE);
  }

  // Requested data loaded successfully.
  FileMan_Close(hFile);

  // Set some more flags in the temporary image structure, copy it so that hImage points
  // to it, and return.
  if (Header.fFlags & STCI_ZLIB_COMPRESSED) {
    TempImage.fFlags |= IMAGE_COMPRESSED;
  }
  TempImage.usWidth = Header.usWidth;
  TempImage.usHeight = Header.usHeight;
  TempImage.ubBitDepth = Header.ubDepth;
  *hImage = TempImage;

  return (TRUE);
}

BOOLEAN STCILoadRGB(HIMAGE hImage, uint16_t fContents, HWFILE hFile, STCIHeader *pHeader) {
  uint32_t uiBytesRead;

  if (fContents & IMAGE_PALETTE &&
      !(fContents & IMAGE_ALLIMAGEDATA)) {  // RGB doesn't have a palette!
    return (FALSE);
  }

  if (fContents & IMAGE_BITMAPDATA) {
    // Allocate memory for the image data and read it in
    hImage->pImageData = MemAlloc(pHeader->uiStoredSize);
    if (hImage->pImageData == NULL) {
      return (FALSE);
    } else if (!FileMan_Read(hFile, hImage->pImageData, pHeader->uiStoredSize, &uiBytesRead) ||
               uiBytesRead != pHeader->uiStoredSize) {
      MemFree(hImage->pImageData);
      return (FALSE);
    }

    hImage->fFlags |= IMAGE_BITMAPDATA;

    if (pHeader->ubDepth == 16) {
      // ASSUMPTION: file data is 565 R,G,B

      if (gusRedMask != (uint16_t)pHeader->RGB.uiRedMask ||
          gusGreenMask != (uint16_t)pHeader->RGB.uiGreenMask ||
          gusBlueMask != (uint16_t)pHeader->RGB.uiBlueMask) {
        // colour distribution of the file is different from hardware!  We have to change it!
        DbgMessage(TOPIC_HIMAGE, DBG_LEVEL_3, "Converting to current RGB distribution!");
        // Convert the image to the current hardware's specifications
        if (gusRedMask > gusGreenMask && gusGreenMask > gusBlueMask) {
          // hardware wants RGB!
          if (gusRedMask == 0x7C00 && gusGreenMask == 0x03E0 &&
              gusBlueMask == 0x001F) {  // hardware is 555
            ConvertRGBDistribution565To555(hImage->p16BPPData,
                                           pHeader->usWidth * pHeader->usHeight);
            return (TRUE);
          } else if (gusRedMask == 0xFC00 && gusGreenMask == 0x03E0 && gusBlueMask == 0x001F) {
            ConvertRGBDistribution565To655(hImage->p16BPPData,
                                           pHeader->usWidth * pHeader->usHeight);
            return (TRUE);
          } else if (gusRedMask == 0xF800 && gusGreenMask == 0x07C0 && gusBlueMask == 0x003F) {
            ConvertRGBDistribution565To556(hImage->p16BPPData,
                                           pHeader->usWidth * pHeader->usHeight);
            return (TRUE);
          } else {
            // take the long route
            ConvertRGBDistribution565ToAny(hImage->p16BPPData,
                                           pHeader->usWidth * pHeader->usHeight);
            return (TRUE);
          }
        } else {
          // hardware distribution is not R-G-B so we have to take the long route!
          ConvertRGBDistribution565ToAny(hImage->p16BPPData, pHeader->usWidth * pHeader->usHeight);
          return (TRUE);
        }
      }
    }
  }
  return (TRUE);
}

BOOLEAN STCILoadIndexed(HIMAGE hImage, uint16_t fContents, HWFILE hFile, STCIHeader *pHeader) {
  uint32_t uiFileSectionSize;
  uint32_t uiBytesRead;
  void *pSTCIPalette;

  if (fContents & IMAGE_PALETTE) {  // Allocate memory for reading in the palette
    if (pHeader->Indexed.uiNumberOfColours != 256) {
      DbgMessage(TOPIC_HIMAGE, DBG_LEVEL_3, "Palettized image has bad palette size.");
      return (FALSE);
    }
    uiFileSectionSize = pHeader->Indexed.uiNumberOfColours * STCI_PALETTE_ELEMENT_SIZE;
    pSTCIPalette = MemAlloc(uiFileSectionSize);
    if (pSTCIPalette == NULL) {
      DbgMessage(TOPIC_HIMAGE, DBG_LEVEL_3, "Out of memory!");
      FileMan_Close(hFile);
      return (FALSE);
    }

    // ATE: Memset: Jan 16/99
    memset(pSTCIPalette, 0, uiFileSectionSize);

    // Read in the palette
    if (!FileMan_Read(hFile, pSTCIPalette, uiFileSectionSize, &uiBytesRead) ||
        uiBytesRead != uiFileSectionSize) {
      DbgMessage(TOPIC_HIMAGE, DBG_LEVEL_3, "Problem loading palette!");
      FileMan_Close(hFile);
      MemFree(pSTCIPalette);
      return (FALSE);
    } else if (!STCISetPalette(pSTCIPalette, hImage)) {
      DbgMessage(TOPIC_HIMAGE, DBG_LEVEL_3, "Problem setting hImage-format palette!");
      FileMan_Close(hFile);
      MemFree(pSTCIPalette);
      return (FALSE);
    }
    hImage->fFlags |= IMAGE_PALETTE;
    // Free the temporary buffer
    MemFree(pSTCIPalette);
  } else if (fContents & (IMAGE_BITMAPDATA | IMAGE_APPDATA)) {  // seek past the palette
    uiFileSectionSize = pHeader->Indexed.uiNumberOfColours * STCI_PALETTE_ELEMENT_SIZE;
    if (FileMan_Seek(hFile, uiFileSectionSize, FILE_SEEK_FROM_CURRENT) == FALSE) {
      DbgMessage(TOPIC_HIMAGE, DBG_LEVEL_3, "Problem seeking past palette!");
      FileMan_Close(hFile);
      return (FALSE);
    }
  }
  if (fContents & IMAGE_BITMAPDATA) {
    if (pHeader->fFlags & STCI_ETRLE_COMPRESSED) {
      // load data for the subimage (object) structures
      Assert(sizeof(ETRLEObject) == STCI_SUBIMAGE_SIZE);
      hImage->usNumberOfObjects = pHeader->Indexed.usNumberOfSubImages;
      uiFileSectionSize = hImage->usNumberOfObjects * STCI_SUBIMAGE_SIZE;
      hImage->pETRLEObject = (ETRLEObject *)MemAlloc(uiFileSectionSize);
      if (hImage->pETRLEObject == NULL) {
        DbgMessage(TOPIC_HIMAGE, DBG_LEVEL_3, "Out of memory!");
        FileMan_Close(hFile);
        if (fContents & IMAGE_PALETTE) {
          MemFree(hImage->pPalette);
        }
        return (FALSE);
      }
      if (!FileMan_Read(hFile, hImage->pETRLEObject, uiFileSectionSize, &uiBytesRead) ||
          uiBytesRead != uiFileSectionSize) {
        DbgMessage(TOPIC_HIMAGE, DBG_LEVEL_3, "Error loading subimage structures!");
        FileMan_Close(hFile);
        if (fContents & IMAGE_PALETTE) {
          MemFree(hImage->pPalette);
        }
        MemFree(hImage->pETRLEObject);
        return (FALSE);
      }
      hImage->uiSizePixData = pHeader->uiStoredSize;
      hImage->fFlags |= IMAGE_TRLECOMPRESSED;
    }
    // allocate memory for and read in the image data
    hImage->pImageData = MemAlloc(pHeader->uiStoredSize);
    if (hImage->pImageData == NULL) {
      DbgMessage(TOPIC_HIMAGE, DBG_LEVEL_3, "Out of memory!");
      FileMan_Close(hFile);
      if (fContents & IMAGE_PALETTE) {
        MemFree(hImage->pPalette);
      }
      if (hImage->usNumberOfObjects > 0) {
        MemFree(hImage->pETRLEObject);
      }
      return (FALSE);
    } else if (!FileMan_Read(hFile, hImage->pImageData, pHeader->uiStoredSize, &uiBytesRead) ||
               uiBytesRead != pHeader->uiStoredSize) {  // Problem reading in the image data!
      DbgMessage(TOPIC_HIMAGE, DBG_LEVEL_3, "Error loading image data!");
      FileMan_Close(hFile);
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
    if (FileMan_Seek(hFile, pHeader->uiStoredSize, FILE_SEEK_FROM_CURRENT) == FALSE) {
      DbgMessage(TOPIC_HIMAGE, DBG_LEVEL_3, "Problem seeking past image data!");
      FileMan_Close(hFile);
      return (FALSE);
    }
  }

  if (fContents & IMAGE_APPDATA && pHeader->uiAppDataSize > 0) {
    // load application-specific data
    hImage->pAppData = (uint8_t *)MemAlloc(pHeader->uiAppDataSize);
    if (hImage->pAppData == NULL) {
      DbgMessage(TOPIC_HIMAGE, DBG_LEVEL_3, "Out of memory!");
      FileMan_Close(hFile);
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
    if (!FileMan_Read(hFile, hImage->pAppData, pHeader->uiAppDataSize, &uiBytesRead) ||
        uiBytesRead != pHeader->uiAppDataSize) {
      DbgMessage(TOPIC_HIMAGE, DBG_LEVEL_3, "Error loading application-specific data!");
      FileMan_Close(hFile);
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
    hImage->uiAppDataSize = pHeader->uiAppDataSize;
    ;
    hImage->fFlags |= IMAGE_APPDATA;
  } else {
    hImage->pAppData = NULL;
    hImage->uiAppDataSize = 0;
  }
  return (TRUE);
}

BOOLEAN STCISetPalette(void *pSTCIPalette, HIMAGE hImage) {
  uint16_t usIndex;
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

BOOLEAN IsSTCIETRLEFile(char *ImageFile) {
  HWFILE hFile;
  STCIHeader Header;
  uint32_t uiBytesRead;

  CHECKF(FileMan_Exists(ImageFile));

  // Open the file and read the header
  hFile = FileMan_Open(ImageFile, FILE_ACCESS_READ, FALSE);
  CHECKF(hFile);

  if (!FileMan_Read(hFile, &Header, STCI_HEADER_SIZE, &uiBytesRead) ||
      uiBytesRead != STCI_HEADER_SIZE || memcmp(Header.cID, STCI_ID_STRING, STCI_ID_LEN) != 0) {
    DbgMessage(TOPIC_HIMAGE, DBG_LEVEL_3, "Problem reading STCI header.");
    FileMan_Close(hFile);
    return (FALSE);
  }
  FileMan_Close(hFile);
  if (Header.fFlags & STCI_ETRLE_COMPRESSED) {
    return (TRUE);
  } else {
    return (FALSE);
  }
}
