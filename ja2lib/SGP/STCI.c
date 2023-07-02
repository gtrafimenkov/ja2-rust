#include <string.h>

#include "SGP/Debug.h"
#include "SGP/HImage.h"
#include "rust_debug.h"
#include "rust_fileman.h"
#include "rust_images.h"

BOOLEAN LoadSTCIFileToImage(const char *filePath, struct Image *hImage, bool loadAppData) {
  Assert(hImage != NULL);
  DebugLogWrite(filePath);

  if (!(File_Exists(filePath))) {
    return FALSE;
  }

  FileID hFile = File_OpenForReading(filePath);
  if (!(hFile)) {
    return FALSE;
  }

  struct STIImageLoaded sti = LoadSTIImage(hFile, loadAppData);
  if (!sti.success) {
    File_Close(hFile);
    return FALSE;
  }

  hImage->pImageData = sti.image_data;
  if (hImage->pImageData) {
    hImage->imageDataAllocatedInRust = true;
    hImage->fFlags |= IMAGE_BITMAPDATA;
  }

  hImage->pPalette = sti.palette;
  if (hImage->pPalette) {
    hImage->paletteAllocatedInRust = true;
    hImage->fFlags |= IMAGE_PALETTE;
  }

  hImage->pETRLEObject = sti.subimages;
  if (hImage->pETRLEObject) {
    hImage->usNumberOfObjects = sti.number_of_subimages;
    hImage->fFlags |= IMAGE_TRLECOMPRESSED;
    hImage->uiSizePixData = sti.image_data_size;
  }

  hImage->pAppData = sti.app_data;
  hImage->uiAppDataSize = sti.app_data_size;
  if (hImage->pAppData) {
    hImage->fFlags |= IMAGE_APPDATA;
  }

  if (sti.zlib_compressed) {
    hImage->fFlags |= IMAGE_COMPRESSED;
  }
  hImage->usWidth = sti.Width;
  hImage->usHeight = sti.Height;
  hImage->ubBitDepth = sti.pixel_depth;

  File_Close(hFile);
  return (TRUE);
}
