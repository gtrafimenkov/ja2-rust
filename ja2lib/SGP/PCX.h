#ifndef __PCX_
#define __PCX_

#include "SGP/HImage.h"
#include "SGP/Types.h"

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

BOOLEAN LoadPCXFileToImage(HIMAGE hImage, uint16_t fContents);
PcxObject *LoadPcx(STR8 pFilename);
BOOLEAN BlitPcxToBuffer(PcxObject *pCurrentPcxObject, uint8_t *pBuffer, uint16_t usBufferWidth,
                        uint16_t usBufferHeight, uint16_t usX, uint16_t usY, BOOLEAN fTransp);

#endif
