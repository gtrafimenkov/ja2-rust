#ifndef __SMOOTH_H
#define __SMOOTH_H

#include "BuildDefines.h"
#include "SGP/Types.h"

#define ONELEVELTYPEONEROOF 1
#define ONELEVELTYPETWOROOF 2

//   Area (pointer to SGP rect) +
//      Location to check-+--|  |       |---- Check left and right edges -----|    |---- Check top
//      and bottom edges -----|
#define IsLocationInArea(x, y, r) \
  (((x) >= r->iLeft) && ((x) <= r->iRight) && ((y) >= r->iTop) && ((y) <= r->iBottom))

void SmoothAllTerrainWorld(void);
void SmoothTerrain(int gridno, int origType, uint16_t *piNewTile, BOOLEAN fForceSmooth);

void SmoothTerrainRadius(uint32_t iMapIndex, uint32_t uiCheckType, uint8_t ubRadius,
                         BOOLEAN fForceSmooth);
void SmoothTerrainWorld(uint32_t uiCheckType);
void SmoothWaterTerrain(int gridno, int origType, uint16_t *piNewTile, BOOLEAN fForceSmooth);
void SmoothAllTerrainTypeRadius(uint32_t iMapIndex, uint8_t ubRadius, BOOLEAN fForceSmooth);

void SmoothExitGrid(int gridno, uint16_t *piNewTile, BOOLEAN fForceSmooth);
void SmoothExitGridRadius(INT16 sMapIndex, uint8_t ubRadius);

#endif
