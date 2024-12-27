// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#ifndef __SHADE_TABLE_UTIL_H
#define __SHADE_TABLE_UTIL_H

#include "TileEngine/TileDat.h"

struct VObject;

void DetermineRGBDistributionSettings();
BOOLEAN LoadShadeTable(struct VObject* pObj, uint32_t uiTileTypeIndex);
BOOLEAN SaveShadeTable(struct VObject* pObj, uint32_t uiTileTypeIndex);

extern char TileSurfaceFilenames[NUMBEROFTILETYPES][32];
extern BOOLEAN gfForceBuildShadeTables;

BOOLEAN DeleteShadeTableDir();

#endif
