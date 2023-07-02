#ifndef __EDITORMAPINFO_H
#define __EDITORMAPINFO_H

#include "BuildDefines.h"
#include "SGP/HImage.h"
#include "SGP/Types.h"

void SetupTextInputForMapInfo();
void UpdateMapInfo();
void ExtractAndUpdateMapInfo();
BOOLEAN ApplyNewExitGridValuesToTextFields();
void UpdateMapInfoFields();

extern BOOLEAN gfEditorForceShadeTableRebuild;

void LocateNextExitGrid();

enum { PRIMETIME_LIGHT, NIGHTTIME_LIGHT, ALWAYSON_LIGHT };
void ChangeLightDefault(INT8 bLightType);
extern INT8 gbDefaultLightType;

#endif
