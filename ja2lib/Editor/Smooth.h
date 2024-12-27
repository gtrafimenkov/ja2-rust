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
void SmoothTerrain(int gridno, int origType, UINT16 *piNewTile, BOOLEAN fForceSmooth);

void SmoothTerrainRadius(uint32_t iMapIndex, uint32_t uiCheckType, UINT8 ubRadius,
                         BOOLEAN fForceSmooth);
void SmoothTerrainWorld(uint32_t uiCheckType);
void SmoothWaterTerrain(int gridno, int origType, UINT16 *piNewTile, BOOLEAN fForceSmooth);
void SmoothAllTerrainTypeRadius(uint32_t iMapIndex, UINT8 ubRadius, BOOLEAN fForceSmooth);

void SmoothExitGrid(int gridno, UINT16 *piNewTile, BOOLEAN fForceSmooth);
void SmoothExitGridRadius(INT16 sMapIndex, UINT8 ubRadius);

#endif
