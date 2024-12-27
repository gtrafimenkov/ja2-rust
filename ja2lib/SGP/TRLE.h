#ifndef __TRLE_H
#define __TRLE_H

#include "SGP/Types.h"

typedef struct {
  uint32_t uiOffset;
  uint32_t uiWidth;
  uint32_t uiOffLen;
  INT16 sOffsetX;
  INT16 sOffsetY;

} TRLEObject;

typedef struct {
  uint32_t uiHeightEach;
  uint32_t uiTotalElements;
  TRLEObject *pTRLEObject;
  PTR pPixData;
  uint32_t uiSizePixDataElem;

} TRLEData;

BOOLEAN GetTRLEObjectData(uint32_t uiTotalElements, TRLEObject *pTRLEObject, INT16 ssIndex,
                          uint32_t *pWidth, uint32_t *pOffset, uint32_t *pOffLen,
                          uint16_t *pOffsetX, uint16_t *pOffsetY);

BOOLEAN SetTRLEObjectOffset(uint32_t uiTotalElements, TRLEObject *pTRLEObject, INT16 ssIndex,
                            INT16 sOffsetX, INT16 sOffsetY);

#endif
