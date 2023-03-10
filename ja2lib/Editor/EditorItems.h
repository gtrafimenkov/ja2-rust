#ifndef __EDITORITEMS_H
#define __EDITORITEMS_H

#include "BuildDefines.h"
#include "SGP/Types.h"

typedef struct {
  BOOLEAN fGameInit;      // Used for initializing save variables the first time.
                          // This flag is initialize at
  BOOLEAN fKill;          // flagged for deallocation.
  BOOLEAN fActive;        // currently active
  UINT16 *pusItemIndex;   // a dynamic array of Item indices
  UINT32 uiBuffer;        // index of buffer
  UINT32 uiItemType;      // Weapons, ammo, armour, explosives, equipment
  INT16 sWidth, sHeight;  // width and height of buffer
  INT16 sNumItems;        // total number of items in the current class of item.
  INT16 sSelItemIndex;    // currently selected item index.
  INT16 sHilitedItemIndex;
  INT16 sScrollIndex;  // current scroll index (0 is far left, 1 is next tile to the right, ...)
  INT16 sSaveSelWeaponsIndex, sSaveSelAmmoIndex, sSaveSelArmourIndex, sSaveSelExplosivesIndex,
      sSaveSelEquipment1Index, sSaveSelEquipment2Index, sSaveSelEquipment3Index,
      sSaveSelTriggersIndex, sSaveSelKeysIndex;
  INT16 sSaveWeaponsScrollIndex, sSaveAmmoScrollIndex, sSaveArmourScrollIndex,
      sSaveExplosivesScrollIndex, sSaveEquipment1ScrollIndex, sSaveEquipment2ScrollIndex,
      sSaveEquipment3ScrollIndex, sSaveTriggersScrollIndex, sSaveKeysScrollIndex;
  INT16 sNumWeapons, sNumAmmo, sNumArmour, sNumExplosives, sNumEquipment1, sNumEquipment2,
      sNumEquipment3, sNumTriggers, sNumKeys;
} EditorItemsInfo;

extern EditorItemsInfo eInfo;

void InitEditorItemsToolbar();
void EntryInitEditorItemsInfo();
void InitEditorItemsInfo(UINT32 uiItemType);
void RenderEditorItemsInfo();
void ClearEditorItemsInfo();
void DisplayItemStatistics();
void DetermineItemsScrolling();

// User actions
void AddSelectedItemToWorld(INT16 sGridNo);
void HandleRightClickOnItem(INT16 sGridNo);
void DeleteSelectedItem();
void ShowSelectedItem();
void HideSelectedItem();
void SelectNextItemPool();
void SelectNextItemInPool();
void SelectPrevItemInPool();

void KillItemPoolList();
void BuildItemPoolList();

void HideItemCursor(INT32 iMapIndex);
void ShowItemCursor(INT32 iMapIndex);

void SetEditorItemsTaskbarMode(UINT16 usNewMode);

void HandleItemsPanel(UINT16 usScreenX, UINT16 usScreenY, INT8 bEvent);

extern INT32 giDefaultExistChance;

#endif
