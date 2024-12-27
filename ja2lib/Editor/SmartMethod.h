#ifndef __SMARTMETHOD_H
#define __SMARTMETHOD_H

#include "BuildDefines.h"
#include "SGP/Types.h"

void CalcSmartWallDefault(UINT16 *pusObjIndex, UINT16 *pusUseIndex);
void CalcSmartDoorDefault(UINT16 *pusObjIndex, UINT16 *pusUseIndex);
void CalcSmartWindowDefault(UINT16 *pusObjIndex, UINT16 *pusUseIndex);
void CalcSmartBrokenWallDefault(UINT16 *pusObjIndex, UINT16 *pusUseIndex);

UINT16 CalcSmartWindowIndex(UINT16 usWallOrientation);
UINT16 CalcSmartDoorIndex(UINT16 usWallOrientation);
UINT16 CalcSmartBrokenWallIndex(UINT16 usWallOrientation);
UINT16 CalcSmartDoorType();

void IncSmartWallUIValue();
void DecSmartWallUIValue();
void IncSmartDoorUIValue();
void DecSmartDoorUIValue();
void IncSmartWindowUIValue();
void DecSmartWindowUIValue();
void IncSmartBrokenWallUIValue();
void DecSmartBrokenWallUIValue();

BOOLEAN CalcWallInfoUsingSmartMethod(uint32_t iMapIndex, UINT16 *pusWallType, UINT16 *pusIndex);
BOOLEAN CalcDoorInfoUsingSmartMethod(uint32_t iMapIndex, UINT16 *pusDoorType, UINT16 *pusIndex);
BOOLEAN CalcWindowInfoUsingSmartMethod(uint32_t iMapIndex, UINT16 *pusWallType, UINT16 *pusIndex);
BOOLEAN CalcBrokenWallInfoUsingSmartMethod(uint32_t iMapIndex, UINT16 *pusWallType,
                                           UINT16 *pusIndex);

void PasteSmartWall(uint32_t iMapIndex);
void PasteSmartDoor(uint32_t iMapIndex);
void PasteSmartWindow(uint32_t iMapIndex);
void PasteSmartBrokenWall(uint32_t iMapIndex);

extern UINT8 gubDoorUIValue;
extern UINT8 gubWindowUIValue;
extern UINT8 gubWallUIValue;
extern UINT8 gubBrokenWallUIValue;

#endif
