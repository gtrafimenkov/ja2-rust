#ifndef __SHADE_TABLE_UTIL_H
#define __SHADE_TABLE_UTIL_H

#include "TileEngine/TileDat.h"

struct VObject;

void DetermineRGBDistributionSettings();
BOOLEAN LoadShadeTable(struct VObject* pObj, UINT32 uiTileTypeIndex);
BOOLEAN SaveShadeTable(struct VObject* pObj, UINT32 uiTileTypeIndex);

extern CHAR8 TileSurfaceFilenames[NUMBEROFTILETYPES][32];
extern BOOLEAN gfForceBuildShadeTables;

BOOLEAN DeleteShadeTableDir();

#endif
