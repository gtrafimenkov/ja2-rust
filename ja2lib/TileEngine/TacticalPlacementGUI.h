#ifndef __TACTICAL_PLACEMENT_GUI_H
#define __TACTICAL_PLACEMENT_GUI_H

#include "SGP/Types.h"

struct MOUSE_REGION;
struct SOLDIERTYPE;

void InitTacticalPlacementGUI();
void KillTacticalPlacementGUI();
void TacticalPlacementHandle();
void RenderTacticalPlacementGUI();

void HandleTacticalPlacementClicksInOverheadMap(struct MOUSE_REGION *reg, INT32 reason);

extern BOOLEAN gfTacticalPlacementGUIActive;
extern BOOLEAN gfEnterTacticalPlacementGUI;

extern struct SOLDIERTYPE *gpTacticalPlacementSelectedSoldier;
extern struct SOLDIERTYPE *gpTacticalPlacementHilightedSoldier;

// Saved value.  Contains the last choice for future battles.
extern UINT8 gubDefaultButton;

#endif
