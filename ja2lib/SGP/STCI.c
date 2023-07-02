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

  {
    struct STIImageLoaded sti = LoadSTIImage(hFile, loadAppData);
    if (!sti.success) {
      File_Close(hFile);
      return FALSE;
    }
    TempImage.pImageData = sti.image_data;
    if (TempImage.pImageData) {
      TempImage.imageDataAllocatedInRust = true;
      TempImage.fFlags |= IMAGE_BITMAPDATA;
    }

    TempImage.pPalette = sti.palette;
    if (TempImage.pPalette) {
      TempImage.paletteAllocatedInRust = true;
      TempImage.fFlags |= IMAGE_PALETTE;
    }

    TempImage.pETRLEObject = sti.subimages;
    if (TempImage.pETRLEObject) {
      TempImage.usNumberOfObjects = sti.usNumberOfSubImages;
      TempImage.fFlags |= IMAGE_TRLECOMPRESSED;
    }
    TempImage.uiSizePixData = sti.StoredSize;

    TempImage.pAppData = sti.app_data;
    if (TempImage.pAppData) {
      TempImage.uiAppDataSize = sti.AppDataSize;
      TempImage.fFlags |= IMAGE_APPDATA;
    } else {
      TempImage.pAppData = NULL;
      TempImage.uiAppDataSize = 0;
    }

    if (sti.compressed) {
      TempImage.fFlags |= IMAGE_COMPRESSED;
    }
    TempImage.usWidth = sti.Width;
    TempImage.usHeight = sti.Height;
    TempImage.ubBitDepth = header.end.Depth;
  }

  // struct STCIHeader header;
  // if (!ReadSTCIHeader(hFile, &header)) {
  //   DebugMsg(TOPIC_HIMAGE, DBG_INFO, "Problem reading STCI header.");
  //   File_Close(hFile);
  //   return (FALSE);
  // }

  // // Determine from the header the data stored in the file. and run the appropriate loader
  // if (header.middle.tag == Rgb) {
  //   TempImage.pImageData = ReadSTCIImageData(hFile, &header);
  //   if (!TempImage.pImageData) {
  //     File_Close(hFile);
  //     return (FALSE);
  //   }
  //   TempImage.imageDataAllocatedInRust = true;
  //   TempImage.fFlags |= IMAGE_BITMAPDATA;
  // } else if (header.middle.tag == Indexed) {
  //   if (!STCILoadIndexed(&TempImage, loadAppData, hFile, &header)) {
  //     DebugMsg(TOPIC_HIMAGE, DBG_INFO, "Problem loading palettized image.");
  //     File_Close(hFile);
  //     return (FALSE);
  //   }
  // } else {  // unsupported type of data, or the right flags weren't set!
  //   DebugMsg(TOPIC_HIMAGE, DBG_INFO, "Unknown data organization in STCI file.");
  //   File_Close(hFile);
  //   return (FALSE);
  // }

  // Requested data loaded successfully.
  File_Close(hFile);

  // Set some more flags in the temporary image structure, copy it so that hImage points
  // to it, and return.
  // if (header.head.Flags & STCI_ZLIB_COMPRESSED) {
  //   TempImage.fFlags |= IMAGE_COMPRESSED;
  // }
  // TempImage.usWidth = header.head.Width;
  // TempImage.usHeight = header.head.Height;
  // TempImage.ubBitDepth = header.end.Depth;
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
    if (pHeader->head.Flags & STCI_ETRLE_COMPRESSED) {
      hImage->pETRLEObject = ReadSTCISubimages(hFile, pHeader->middle.indexed.usNumberOfSubImages);
      if (!hImage->pETRLEObject) {
        File_Close(hFile);
        FreeImagePalette(hImage);
        return (FALSE);
      }
      hImage->usNumberOfObjects = pHeader->middle.indexed.usNumberOfSubImages;
      hImage->uiSizePixData = pHeader->head.StoredSize;
      hImage->fFlags |= IMAGE_TRLECOMPRESSED;
    }
  }

  // image data
  {
    UINT32 uiBytesRead;
    // allocate memory for and read in the image data
    hImage->pImageData = ReadSTCIImageData(hFile, pHeader);
    if (!hImage->pImageData) {
      File_Close(hFile);
      FreeImagePalette(hImage);
      FreeImageSubimages(hImage);
      return (FALSE);
    }
    hImage->imageDataAllocatedInRust = true;
    hImage->fFlags |= IMAGE_BITMAPDATA;
  }

  if (loadAppData && pHeader->end.AppDataSize > 0) {
    // load application-specific data
    hImage->pAppData = ReadSTCIAppData(hFile, pHeader);
    if (!hImage->pAppData) {
      File_Close(hFile);
      FreeImagePalette(hImage);
      FreeImageData(hImage);
      FreeImageSubimages(hImage);
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
