#include <string.h>

#include "SGP/Debug.h"
#include "SGP/HImage.h"
#include "rust_debug.h"
#include "rust_fileman.h"
#include "rust_images.h"

struct Image *LoadSTCIFileToImage(const char *filePath, bool loadAppData) {
  struct Image *hImage = (struct Image *)MemAlloc(sizeof(struct Image));
  if (!hImage) {
    return NULL;
  }
  memset(hImage, 0, sizeof(struct Image));

  struct STIImageLoaded sti = LoadSTIImageFromFile(filePath, loadAppData);
  if (!sti.success) {
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

  hImage->subimages = sti.subimages;
  if (hImage->subimages) {
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

  return hImage;
}
