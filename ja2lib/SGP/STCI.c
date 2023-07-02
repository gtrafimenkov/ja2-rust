#include <string.h>

#include "SGP/Debug.h"
#include "SGP/HImage.h"
#include "rust_debug.h"
#include "rust_fileman.h"
#include "rust_images.h"

BOOLEAN LoadSTCIFileToImage(const char *filePath, struct Image *hImage, bool loadAppData) {
  DebugLogWrite(filePath);
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
      TempImage.uiSizePixData = sti.StoredSize;
    }

    TempImage.pAppData = sti.app_data;
    TempImage.uiAppDataSize = sti.AppDataSize;
    if (TempImage.pAppData) {
      TempImage.fFlags |= IMAGE_APPDATA;
    }

    if (sti.compressed) {
      TempImage.fFlags |= IMAGE_COMPRESSED;
    }
    TempImage.usWidth = sti.Width;
    TempImage.usHeight = sti.Height;
    TempImage.ubBitDepth = sti.Depth;
  }

  File_Close(hFile);
  *hImage = TempImage;
  return (TRUE);
}
