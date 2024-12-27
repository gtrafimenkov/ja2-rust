#ifndef __SOLDIER_INIT_LIST_H
#define __SOLDIER_INIT_LIST_H

#include "Tactical/SoldierCreate.h"

typedef struct SOLDIERINITNODE {
  uint8_t ubNodeID;
  uint8_t ubSoldierID;
  BASIC_SOLDIERCREATE_STRUCT *pBasicPlacement;
  SOLDIERCREATE_STRUCT *pDetailedPlacement;
  struct SOLDIERTYPE *pSoldier;
  struct SOLDIERINITNODE *prev;
  struct SOLDIERINITNODE *next;
} SOLDIERINITNODE;

extern SOLDIERINITNODE *gSoldierInitHead;
extern SOLDIERINITNODE *gSoldierInitTail;

// These serialization functions are assuming the passing of a valid file
// pointer to the beginning of the save/load area, at the correct part of the
// map file.
BOOLEAN LoadSoldiersFromMap(INT8 **hBuffer);
BOOLEAN SaveSoldiersToMap(HWFILE fp);

// For the purpose of keeping track of which soldier belongs to which placement within the game,
// the only way we can do this properly is to save the soldier ID from the list and reconnect the
// soldier pointer whenever we load the game.
BOOLEAN SaveSoldierInitListLinks(HWFILE hfile);
BOOLEAN LoadSoldierInitListLinks(HWFILE hfile);
BOOLEAN NewWayOfLoadingEnemySoldierInitListLinks(HWFILE hfile);
BOOLEAN NewWayOfLoadingCivilianInitListLinks(HWFILE hfile);
BOOLEAN LookAtButDontProcessEnemySoldierInitListLinks(HWFILE hfile);

void InitSoldierInitList();
void KillSoldierInitList();
SOLDIERINITNODE *AddBasicPlacementToSoldierInitList(BASIC_SOLDIERCREATE_STRUCT *pBasicPlacement);
void RemoveSoldierNodeFromInitList(SOLDIERINITNODE *pNode);
SOLDIERINITNODE *FindSoldierInitNodeWithID(uint16_t usID);

uint8_t AddSoldierInitListTeamToWorld(INT8 bTeam, uint8_t ubMaxNum);
void AddSoldierInitListEnemyDefenceSoldiers(uint8_t ubTotalAdmin, uint8_t ubTotalTroops,
                                            uint8_t ubTotalElite);
void AddSoldierInitListCreatures(BOOLEAN fQueen, uint8_t ubNumLarvae, uint8_t ubNumInfants,
                                 uint8_t ubNumYoungMales, uint8_t ubNumYoungFemales,
                                 uint8_t ubNumAdultMales, uint8_t ubNumAdultFemales);
void AddSoldierInitListMilitia(uint8_t ubNumGreen, uint8_t ubNumReg, uint8_t ubNumElites);

void AddSoldierInitListBloodcats();

void UseEditorOriginalList();
void UseEditorAlternateList();
BOOLEAN AddPlacementToWorld(SOLDIERINITNODE *pNode);
void AddPlacementToWorldByProfileID(uint8_t ubProfile);

void EvaluateDeathEffectsToSoldierInitList(struct SOLDIERTYPE *pSoldier);
void RemoveDetailedPlacementInfo(uint8_t ubNodeID);

void AddProfilesUsingProfileInsertionData();
void AddProfilesNotUsingProfileInsertionData();

#endif
