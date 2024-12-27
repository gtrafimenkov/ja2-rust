#ifndef __TRLE_H
#define __TRLE_H

#include "SGP/Types.h"

typedef struct {
  uint32_t uiOffset;
  uint32_t uiWidth;
  uint32_t uiOffLen;
  int16_t sOffsetX;
  int16_t sOffsetY;

} TRLEObject;

typedef struct {
  uint32_t uiHeightEach;
  uint32_t uiTotalElements;
  TRLEObject *pTRLEObject;
  PTR pPixData;
  uint32_t uiSizePixDataElem;

} TRLEData;

BOOLEAN GetTRLEObjectData(uint32_t uiTotalElements, TRLEObject *pTRLEObject, int16_t ssIndex,
                          uint32_t *pWidth, uint32_t *pOffset, uint32_t *pOffLen,
                          uint16_t *pOffsetX, uint16_t *pOffsetY);

BOOLEAN SetTRLEObjectOffset(uint32_t uiTotalElements, TRLEObject *pTRLEObject, int16_t ssIndex,
                            int16_t sOffsetX, int16_t sOffsetY);

#endif
