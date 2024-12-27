#ifndef __ITEM_STATISTICS_H
#define __ITEM_STATISTICS_H

#include "BuildDefines.h"
#include "SGP/Types.h"
#include "Tactical/ItemTypes.h"

// Handles the dynamic changing of text input fields and button modes depending on the currently
// edited item.  Both the merc's inventory panel, and the items tab use the same code to accomplish
// this.

// Set if we are editing items from the items tab.  Otherwise, it is assumed that we are
// editing items from the merc's inventory panel.
extern BOOLEAN gfItemEditingMode;

// Set if we need to update the panel.
extern BOOLEAN gfRenderItemStatsPanel;

void SpecifyItemToEdit(struct OBJECTTYPE *pItem, INT32 iMapIndex);

void ShowItemStatsPanel();
void HideItemStatsPanel();
void EnableItemStatsPanel();
void DisableItemStatsPanel();

// called from the taskbar renderer.
void UpdateItemStatsPanel();

enum {
  ITEMSTATS_APPLY,
  ITEMSTATS_CANCEL,
  ITEMSTATS_DEFAULT,
  ITEMSTATS_DELETE,
  ITEMSTATS_HIDE,
  ITEMSTATS_SHOW,
};
void ExecuteItemStatsCmd(uint8_t ubAction);

extern struct OBJECTTYPE *gpItem;
extern INT16 gsItemGridNo;

// enumerations for all of the different action items.  Used by the popup menu for
// changing the type of action item.  When modified, an equivalent text array must be
// changed as well.
enum {
  ACTIONITEM_TRIP_KLAXON,
  ACTIONITEM_FLARE,
  ACTIONITEM_TEARGAS,
  ACTIONITEM_STUN,
  ACTIONITEM_SMOKE,
  ACTIONITEM_MUSTARD,
  ACTIONITEM_MINE,
  ACTIONITEM_OPEN,
  ACTIONITEM_CLOSE,
  ACTIONITEM_SMPIT,
  ACTIONITEM_LGPIT,
  ACTIONITEM_SMALL,   // grenade
  ACTIONITEM_MEDIUM,  // TNT
  ACTIONITEM_LARGE,   // C4
  ACTIONITEM_TOGGLE_DOOR,
  ACTIONITEM_TOGGLE_ACTION1,
  ACTIONITEM_TOGGLE_ACTION2,
  ACTIONITEM_TOGGLE_ACTION3,
  ACTIONITEM_TOGGLE_ACTION4,
  ACTIONITEM_ENTER_BROTHEL,
  ACTIONITEM_EXIT_BROTHEL,
  ACTIONITEM_KINGPIN_ALARM,
  ACTIONITEM_SEX,
  ACTIONITEM_REVEAL_ROOM,
  ACTIONITEM_LOCAL_ALARM,
  ACTIONITEM_GLOBAL_ALARM,
  ACTIONITEM_KLAXON,
  ACTIONITEM_UNLOCK_DOOR,
  ACTIONITEM_TOGGLE_LOCK,
  ACTIONITEM_UNTRAP_DOOR,
  ACTIONITEM_TOGGLE_PRESSURE_ITEMS,
  ACTIONITEM_MUSEUM_ALARM,
  ACTIONITEM_BLOODCAT_ALARM,
  ACTIONITEM_BIG_TEAR_GAS,
  //***22.05.2016***
  ACTIONITEM_ACTIVE_ENEMY_LOCAL,
  ACTIONITEM_ACTIVE_ENEMY_GLOBAL,
  ACTIONITEM_PASSIVE_ENEMY_LOCAL,
  ACTIONITEM_PASSIVE_ENEMY_GLOBAL,
  NUM_ACTIONITEMS
};
extern CHAR16 gszActionItemDesc[NUM_ACTIONITEMS][30];
// Returns a pointer to one of the above string array.
extern CHAR16 *GetActionItemName(struct OBJECTTYPE *pItem);
// Called by the popup menu, when a selection is made.
extern void UpdateActionItem(int8_t bActionItemIndex);
// Changes an action item into the type specified by the ACTIONITEM enumeration.
extern void ChangeActionItem(struct OBJECTTYPE *pItem, int8_t bActionItemIndex);
extern int8_t gbActionItemIndex;
extern int8_t gbDefaultBombTrapLevel;

extern void SetOwnershipGroup(uint8_t ubNewGroup);

#endif
