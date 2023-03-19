#ifndef __SOLDIER_INIT_LIST_H
#define __SOLDIER_INIT_LIST_H

#include "Tactical/SoldierCreate.h"

typedef struct SOLDIERINITNODE {
  UINT8 ubNodeID;
  UINT8 ubSoldierID;
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
BOOLEAN SaveSoldiersToMap(FileID fp);

// For the purpose of keeping track of which soldier belongs to which placement within the game,
// the only way we can do this properly is to save the soldier ID from the list and reconnect the
// soldier pointer whenever we load the game.
BOOLEAN SaveSoldierInitListLinks(FileID hfile);
BOOLEAN LoadSoldierInitListLinks(FileID hfile);
BOOLEAN NewWayOfLoadingEnemySoldierInitListLinks(FileID hfile);
BOOLEAN NewWayOfLoadingCivilianInitListLinks(FileID hfile);
BOOLEAN LookAtButDontProcessEnemySoldierInitListLinks(FileID hfile);

void InitSoldierInitList();
void KillSoldierInitList();
SOLDIERINITNODE *AddBasicPlacementToSoldierInitList(BASIC_SOLDIERCREATE_STRUCT *pBasicPlacement);
void RemoveSoldierNodeFromInitList(SOLDIERINITNODE *pNode);
SOLDIERINITNODE *FindSoldierInitNodeWithID(UINT16 usID);

UINT8 AddSoldierInitListTeamToWorld(INT8 bTeam, UINT8 ubMaxNum);
void AddSoldierInitListEnemyDefenceSoldiers(UINT8 ubTotalAdmin, UINT8 ubTotalTroops,
                                            UINT8 ubTotalElite);
void AddSoldierInitListCreatures(BOOLEAN fQueen, UINT8 ubNumLarvae, UINT8 ubNumInfants,
                                 UINT8 ubNumYoungMales, UINT8 ubNumYoungFemales,
                                 UINT8 ubNumAdultMales, UINT8 ubNumAdultFemales);
void AddSoldierInitListMilitia(UINT8 ubNumGreen, UINT8 ubNumReg, UINT8 ubNumElites);

void AddSoldierInitListBloodcats();

void UseEditorOriginalList();
void UseEditorAlternateList();
BOOLEAN AddPlacementToWorld(SOLDIERINITNODE *pNode);
void AddPlacementToWorldByProfileID(UINT8 ubProfile);

void EvaluateDeathEffectsToSoldierInitList(struct SOLDIERTYPE *pSoldier);
void RemoveDetailedPlacementInfo(UINT8 ubNodeID);

void AddProfilesUsingProfileInsertionData();
void AddProfilesNotUsingProfileInsertionData();

#endif
