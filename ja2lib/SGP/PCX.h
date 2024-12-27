#ifndef __PCX_
#define __PCX_

#include "SGP/HImage.h"
#include "SGP/Types.h"

typedef struct {
  UINT8 ubManufacturer;
  UINT8 ubVersion;
  UINT8 ubEncoding;
  UINT8 ubBitsPerPixel;
  uint16_t usLeft, usTop;
  uint16_t usRight, usBottom;
  uint16_t usHorRez, usVerRez;
  UINT8 ubEgaPalette[48];
  UINT8 ubReserved;
  UINT8 ubColorPlanes;
  uint16_t usBytesPerLine;
  uint16_t usPaletteType;
  UINT8 ubFiller[58];

} PcxHeader;

typedef struct {
  UINT8 *pPcxBuffer;
  UINT8 ubPalette[768];
  uint16_t usWidth, usHeight;
  uint32_t uiBufferSize;
  uint16_t usPcxFlags;

} PcxObject;

BOOLEAN LoadPCXFileToImage(HIMAGE hImage, uint16_t fContents);
PcxObject *LoadPcx(STR8 pFilename);
BOOLEAN BlitPcxToBuffer(PcxObject *pCurrentPcxObject, UINT8 *pBuffer, uint16_t usBufferWidth,
                        uint16_t usBufferHeight, uint16_t usX, uint16_t usY, BOOLEAN fTransp);

#endif
