#include "UI.h"

#include "JAScreens.h"
#include "ScreenIDs.h"
#include "Soldier.h"
#include "Strategic/MapScreen.h"
#include "Strategic/MapScreenInterface.h"
#include "Strategic/MapScreenInterfaceBorder.h"
#include "Strategic/MapScreenInterfaceMap.h"
#include "Strategic/PreBattleInterface.h"
#include "Tactical/Interface.h"
#include "Tactical/InterfaceControl.h"

bool IsTacticalMode() { return guiCurrentScreen == GAME_SCREEN; }

static SGPRect mapCenteringRect = {0, 0, 640, INV_INTERFACE_START_Y};

const SGPRect* GetMapCenteringRect() { return &mapCenteringRect; }

bool IsMapScreen() { return guiTacticalInterfaceFlags & INTERFACE_MAPSCREEN; }
bool IsMapScreen_2() { return guiCurrentScreen == MAP_SCREEN; }

// Get Merc associated with the context menu on tactical screen.
struct SOLDIERTYPE* GetTacticalContextMenuMerc() { return GetSoldierByID(gusUIFullTargetID); }

void SwitchMapToMilitiaMode() {
  if (!fShowMilitia) {
    ToggleShowMilitiaMode();
  }
}

bool IsGoingToAutoresolve() { return gfAutomaticallyStartAutoResolve; }

// Return index of the character selected for assignment
int8_t GetCharForAssignmentIndex() { return bSelectedAssignChar; }
