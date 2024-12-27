#ifndef __OVERHEADMAP_H
#define __OVERHEADMAP_H

#include "SGP/Types.h"

struct MOUSE_REGION;

void InitNewOverheadDB(uint8_t ubTilesetID);
void RenderOverheadMap(int16_t sStartPointX_M, int16_t sStartPointY_M, int16_t sStartPointX_S,
                       int16_t sStartPointY_S, int16_t sEndXS, int16_t sEndYS,
                       BOOLEAN fFromMapUtility);

void HandleOverheadMap();
BOOLEAN InOverheadMap();
void GoIntoOverheadMap();
void HandleOverheadUI();
void KillOverheadMap();

void ClickOverheadRegionCallback(struct MOUSE_REGION *reg, int32_t reason);
void MoveInOverheadRegionCallback(struct MOUSE_REGION *reg, int32_t reason);

void CalculateRestrictedMapCoords(int8_t bDirection, int16_t *psX1, int16_t *psY1, int16_t *psX2,
                                  int16_t *psY2, int16_t sEndXS, int16_t sEndYS);
void CalculateRestrictedScaleFactors(int16_t *pScaleX, int16_t *pScaleY);

void TrashOverheadMap();

#endif
