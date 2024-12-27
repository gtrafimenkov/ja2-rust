// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#ifndef __NEWSMOOTH_H
#define __NEWSMOOTH_H

#include "BuildDefines.h"
#include "SGP/Types.h"

struct GRect;

void AddBuildingSectionToWorld(struct GRect *pSelectRegion);
void RemoveBuildingSectionFromWorld(struct GRect *pSelectRegion);

void AddCaveSectionToWorld(struct GRect *pSelectRegion);
void RemoveCaveSectionFromWorld(struct GRect *pSelectRegion);

void EraseBuilding(uint32_t iMapIndex);
void RebuildRoof(uint32_t iMapIndex, uint16_t usRoofType);
void RebuildRoofUsingFloorInfo(int32_t iMapIndex, uint16_t usRoofType);

void AddCave(int32_t iMapIndex, uint16_t usIndex);

void AnalyseCaveMapForStructureInfo();

#endif
