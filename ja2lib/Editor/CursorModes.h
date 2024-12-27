#ifndef __CURSOR_MODES_H
#define __CURSOR_MODES_H

#include "BuildDefines.h"
#include "SGP/Types.h"

enum {
  SMALLSELECTION,   // 1x1
  MEDIUMSELECTION,  // 3x3
  LARGESELECTION,   // 5x5
  XLARGESELECTION,  // 7x7
  LINESELECTION,    // v or h line with a specified width
  AREASELECTION,    // user controlled area
  NUMSELECTIONTYPES
};

extern int16_t sGridX, sGridY;

extern uint16_t gusSelectionType;
extern uint16_t gusSelectionWidth;
extern uint16_t gusPreserveSelectionWidth;
extern uint16_t gusSelectionDensity;
extern uint16_t gusSavedSelectionType;
extern uint16_t gusSavedBuildingSelectionType;

BOOLEAN PerformDensityTest();
void SetDrawingMode(int32_t iMode);
void UpdateCursorAreas();
void IncreaseSelectionDensity();
void DecreaseSelectionDensity();
void RemoveCursors();

extern CHAR16 wszSelType[6][16];

extern BOOLEAN gfCurrentSelectionWithRightButton;

#endif
