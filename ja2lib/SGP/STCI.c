#include <string.h>

#include "SGP/Debug.h"
#include "SGP/HImage.h"
#include "SGP/ImgFmt.h"
#include "SGP/MemMan.h"
#include "SGP/Types.h"
#include "SGP/WCheck.h"
#include "rust_debug.h"
#include "rust_fileman.h"
#include "rust_images.h"

static BOOLEAN STCILoadIndexed(struct Image *hImage, bool loadAppData, FileID hFile,
                               struct STCIHeader *pHeader);
static BOOLEAN STCISetPalette(PTR pSTCIPalette, struct Image *hImage);

BOOLEAN LoadSTCIFileToImage(const char *filePath, struct Image *hImage, bool loadAppData) {
  FileID hFile = FILE_ID_ERR;
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

  struct STCIHeader header;
  if (!ReadSTCIHeader(hFile, &header)) {
    DebugMsg(TOPIC_HIMAGE, DBG_INFO, "Problem reading STCI header.");
    File_Close(hFile);
    return (FALSE);
  }

  // Determine from the header the data stored in the file. and run the appropriate loader
  if (header.middle.tag == Rgb) {
    uint8_t *data = ReadSTCIRgbData(hFile, &header);
    if (!data) {
      File_Close(hFile);
      return (FALSE);
    }
    TempImage.pImageData = data;
    TempImage.imageDataAllocatedInRust = true;
    TempImage.fFlags |= IMAGE_BITMAPDATA;
  } else if (header.middle.tag == Indexed) {
    if (!STCILoadIndexed(&TempImage, loadAppData, hFile, &header)) {
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

static BOOLEAN STCILoadIndexed(struct Image *hImage, bool loadAppData, FileID hFile,
                               struct STCIHeader *pHeader) {
  hImage->pPalette = ReadSTCIPalette(hFile);
  if (!hImage->pPalette) {
    File_Close(hFile);
    return (FALSE);
  }
  hImage->paletteAllocatedInRust = true;
  hImage->fFlags |= IMAGE_PALETTE;

  // subimages
  {
    UINT32 uiBytesRead;
    if (pHeader->head.Flags & STCI_ETRLE_COMPRESSED) {
      // load data for the subimage (object) structures
      Assert(sizeof(struct ETRLEObject) == STCI_SUBIMAGE_SIZE);
      hImage->usNumberOfObjects = pHeader->middle.indexed.usNumberOfSubImages;
      UINT32 uiFileSectionSize = hImage->usNumberOfObjects * STCI_SUBIMAGE_SIZE;
      hImage->pETRLEObject = (struct ETRLEObject *)MemAlloc(uiFileSectionSize);
      if (hImage->pETRLEObject == NULL) {
        DebugMsg(TOPIC_HIMAGE, DBG_INFO, "Out of memory!");
        File_Close(hFile);
        FreeImagePalette(hImage);
        return (FALSE);
      }
      if (!File_Read(hFile, hImage->pETRLEObject, uiFileSectionSize, &uiBytesRead) ||
          uiBytesRead != uiFileSectionSize) {
        DebugMsg(TOPIC_HIMAGE, DBG_INFO, "Error loading subimage structures!");
        File_Close(hFile);
        FreeImagePalette(hImage);
        MemFree(hImage->pETRLEObject);
        return (FALSE);
      }
      hImage->uiSizePixData = pHeader->head.StoredSize;
      hImage->fFlags |= IMAGE_TRLECOMPRESSED;
    }
  }

  {
    UINT32 uiBytesRead;
    // allocate memory for and read in the image data
    hImage->pImageData = MemAlloc(pHeader->head.StoredSize);
    if (hImage->pImageData == NULL) {
      DebugMsg(TOPIC_HIMAGE, DBG_INFO, "Out of memory!");
      File_Close(hFile);
      FreeImagePalette(hImage);
      if (hImage->usNumberOfObjects > 0) {
        MemFree(hImage->pETRLEObject);
      }
      return (FALSE);
    } else if (!File_Read(hFile, hImage->pImageData, pHeader->head.StoredSize, &uiBytesRead) ||
               uiBytesRead != pHeader->head.StoredSize) {  // Problem reading in the image data!
      DebugMsg(TOPIC_HIMAGE, DBG_INFO, "Error loading image data!");
      File_Close(hFile);
      FreeImageData(hImage);
      FreeImagePalette(hImage);
      if (hImage->usNumberOfObjects > 0) {
        MemFree(hImage->pETRLEObject);
      }
      return (FALSE);
    }
    hImage->fFlags |= IMAGE_BITMAPDATA;
  }

  if (loadAppData && pHeader->end.AppDataSize > 0) {
    // load application-specific data
    hImage->pAppData = (UINT8 *)MemAlloc(pHeader->end.AppDataSize);
    if (hImage->pAppData == NULL) {
      DebugMsg(TOPIC_HIMAGE, DBG_INFO, "Out of memory!");
      File_Close(hFile);
      MemFree(hImage->pAppData);
      FreeImagePalette(hImage);
      FreeImageData(hImage);
      if (hImage->usNumberOfObjects > 0) {
        MemFree(hImage->pETRLEObject);
      }
      return (FALSE);
    }
    UINT32 uiBytesRead;
    if (!File_Read(hFile, hImage->pAppData, pHeader->end.AppDataSize, &uiBytesRead) ||
        uiBytesRead != pHeader->end.AppDataSize) {
      DebugMsg(TOPIC_HIMAGE, DBG_INFO, "Error loading application-specific data!");
      File_Close(hFile);
      MemFree(hImage->pAppData);
      FreeImagePalette(hImage);
      FreeImageData(hImage);
      if (hImage->usNumberOfObjects > 0) {
        MemFree(hImage->pETRLEObject);
      }
      return (FALSE);
    }
    hImage->uiAppDataSize = pHeader->end.AppDataSize;
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
    hImage->pPalette[usIndex]._unused = 0;
    pubPalette++;
  }
  return TRUE;
}
