#ifndef __OVERHEADMAP_H
#define __OVERHEADMAP_H

#include "SGP/Types.h"

struct MOUSE_REGION;

void InitNewOverheadDB(UINT8 ubTilesetID);
void RenderOverheadMap(INT16 sStartPointX_M, INT16 sStartPointY_M, INT16 sStartPointX_S,
                       INT16 sStartPointY_S, INT16 sEndXS, INT16 sEndYS);

void HandleOverheadMap();
BOOLEAN InOverheadMap();
void GoIntoOverheadMap();
void HandleOverheadUI();
void KillOverheadMap();

void ClickOverheadRegionCallback(struct MOUSE_REGION *reg, INT32 reason);
void MoveInOverheadRegionCallback(struct MOUSE_REGION *reg, INT32 reason);

void CalculateRestrictedMapCoords(INT8 bDirection, INT16 *psX1, INT16 *psY1, INT16 *psX2,
                                  INT16 *psY2, INT16 sEndXS, INT16 sEndYS);
void CalculateRestrictedScaleFactors(INT16 *pScaleX, INT16 *pScaleY);

void TrashOverheadMap();

#endif
