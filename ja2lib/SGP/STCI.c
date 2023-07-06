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

  hImage->image_data = sti.image_data;

  hImage->palette = sti.palette;

  hImage->subimages = sti.subimages;
  hImage->number_of_subimages = sti.number_of_subimages;
  hImage->image_data_size = sti.image_data_size;

  hImage->app_data = sti.app_data;
  hImage->app_data_size = sti.app_data_size;

  hImage->width = sti.Width;
  hImage->height = sti.Height;
  hImage->bit_depth = sti.pixel_depth;

  return hImage;
}
