#ifndef __TACTICAL_PLACEMENT_GUI_H
#define __TACTICAL_PLACEMENT_GUI_H

#include "SGP/Types.h"

struct MOUSE_REGION;
struct SOLDIERTYPE;

void InitTacticalPlacementGUI();
void KillTacticalPlacementGUI();
void TacticalPlacementHandle();
void RenderTacticalPlacementGUI();

void HandleTacticalPlacementClicksInOverheadMap(struct MOUSE_REGION *reg, int32_t reason);

extern BOOLEAN gfTacticalPlacementGUIActive;
extern BOOLEAN gfEnterTacticalPlacementGUI;

extern struct SOLDIERTYPE *gpTacticalPlacementSelectedSoldier;
extern struct SOLDIERTYPE *gpTacticalPlacementHilightedSoldier;

// Saved value.  Contains the last choice for future battles.
extern uint8_t gubDefaultButton;

#endif
