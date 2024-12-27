// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#ifndef __EDITORITEMS_H
#define __EDITORITEMS_H

#include "BuildDefines.h"
#include "SGP/Types.h"

typedef struct {
  BOOLEAN fGameInit;      // Used for initializing save variables the first time.
                          // This flag is initialize at
  BOOLEAN fKill;          // flagged for deallocation.
  BOOLEAN fActive;        // currently active
  uint16_t *pusItemIndex;   // a dynamic array of Item indices
  uint32_t uiBuffer;        // index of buffer
  uint32_t uiItemType;      // Weapons, ammo, armour, explosives, equipment
  int16_t sWidth, sHeight;  // width and height of buffer
  int16_t sNumItems;        // total number of items in the current class of item.
  int16_t sSelItemIndex;    // currently selected item index.
  int16_t sHilitedItemIndex;
  int16_t sScrollIndex;  // current scroll index (0 is far left, 1 is next tile to the right, ...)
  int16_t sSaveSelWeaponsIndex, sSaveSelAmmoIndex, sSaveSelArmourIndex, sSaveSelExplosivesIndex,
      sSaveSelEquipment1Index, sSaveSelEquipment2Index, sSaveSelEquipment3Index,
      sSaveSelTriggersIndex, sSaveSelKeysIndex;
  int16_t sSaveWeaponsScrollIndex, sSaveAmmoScrollIndex, sSaveArmourScrollIndex,
      sSaveExplosivesScrollIndex, sSaveEquipment1ScrollIndex, sSaveEquipment2ScrollIndex,
      sSaveEquipment3ScrollIndex, sSaveTriggersScrollIndex, sSaveKeysScrollIndex;
  int16_t sNumWeapons, sNumAmmo, sNumArmour, sNumExplosives, sNumEquipment1, sNumEquipment2,
      sNumEquipment3, sNumTriggers, sNumKeys;
} EditorItemsInfo;

extern EditorItemsInfo eInfo;

void InitEditorItemsToolbar();
void EntryInitEditorItemsInfo();
void InitEditorItemsInfo(uint32_t uiItemType);
void RenderEditorItemsInfo();
void ClearEditorItemsInfo();
void DisplayItemStatistics();
void DetermineItemsScrolling();

// User actions
void AddSelectedItemToWorld(int16_t sGridNo);
void HandleRightClickOnItem(int16_t sGridNo);
void DeleteSelectedItem();
void ShowSelectedItem();
void HideSelectedItem();
void SelectNextItemPool();
void SelectNextItemInPool();
void SelectPrevItemInPool();

void KillItemPoolList();
void BuildItemPoolList();

void HideItemCursor(int32_t iMapIndex);
void ShowItemCursor(int32_t iMapIndex);

void SetEditorItemsTaskbarMode(uint16_t usNewMode);

void HandleItemsPanel(uint16_t usScreenX, uint16_t usScreenY, int8_t bEvent);

extern int32_t giDefaultExistChance;

#endif
