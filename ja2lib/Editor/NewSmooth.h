#ifndef __NEWSMOOTH_H
#define __NEWSMOOTH_H

#include "BuildDefines.h"
#include "SGP/Types.h"

void AddBuildingSectionToWorld(struct GRect *pSelectRegion);
void RemoveBuildingSectionFromWorld(struct GRect *pSelectRegion);

void AddCaveSectionToWorld(struct GRect *pSelectRegion);
void RemoveCaveSectionFromWorld(struct GRect *pSelectRegion);

void EraseBuilding(UINT32 iMapIndex);
void RebuildRoof(UINT32 iMapIndex, UINT16 usRoofType);
void RebuildRoofUsingFloorInfo(INT32 iMapIndex, UINT16 usRoofType);

void AddCave(INT32 iMapIndex, UINT16 usIndex);

void AnalyseCaveMapForStructureInfo();

#endif
