#ifndef __CREATURE_SPREADING_H
#define __CREATURE_SPREADING_H

#include "SGP/Types.h"

void InitCreatureQuest();
void SpreadCreatures();
void DecayCreatures();
void ChooseCreatureQuestStartDay();
void ClearCreatureQuest();
void DeleteCreatureDirectives();

BOOLEAN SaveCreatureDirectives(FileID hFile);
BOOLEAN LoadCreatureDirectives(FileID hFile, UINT32 uiSavedGameVersion);

UINT8 CreaturesInUndergroundSector(UINT8 ubSectorID, UINT8 ubSectorZ);
BOOLEAN PrepareCreaturesForBattle();
void CreatureNightPlanning();
void CreatureAttackTown(UINT8 ubSectorID, BOOLEAN fOverrideTest);

void CheckConditionsForTriggeringCreatureQuest(u8 sSectorX, u8 sSectorY, INT8 bSectorZ);

void ForceCreaturesToAvoidMineTemporarily(UINT8 ubMineIndex);

extern BOOLEAN gfUseCreatureMusic;

BOOLEAN MineClearOfMonsters(UINT8 ubMineIndex);

// Returns TRUE if valid and creature quest over, FALSE if creature quest active or not yet started
BOOLEAN GetWarpOutOfMineCodes(u8 *psSectorX, u8 *psSectorY, INT8 *pbSectorZ,
                              INT16 *psInsertionGridNo);

extern INT16 gsCreatureInsertionCode;
extern INT16 gsCreatureInsertionGridNo;
extern UINT8 gubNumCreaturesAttackingTown;
extern UINT8 gubYoungMalesAttackingTown;
extern UINT8 gubYoungFemalesAttackingTown;
extern UINT8 gubAdultMalesAttackingTown;
extern UINT8 gubAdultFemalesAttackingTown;
extern UINT8 gubSectorIDOfCreatureAttack;
enum {
  CREATURE_BATTLE_CODE_NONE,
  CREATURE_BATTLE_CODE_TACTICALLYADD,
  CREATURE_BATTLE_CODE_TACTICALLYADD_WITHFOV,
  CREATURE_BATTLE_CODE_PREBATTLEINTERFACE,
  CREATURE_BATTLE_CODE_AUTORESOLVE,
};
extern UINT8 gubCreatureBattleCode;

void DetermineCreatureTownComposition(UINT8 ubNumCreatures, UINT8 *pubNumYoungMales,
                                      UINT8 *pubNumYoungFemales, UINT8 *pubNumAdultMales,
                                      UINT8 *pubNumAdultFemales);

void DetermineCreatureTownCompositionBasedOnTacticalInformation(UINT8 *pubNumCreatures,
                                                                UINT8 *pubNumYoungMales,
                                                                UINT8 *pubNumYoungFemales,
                                                                UINT8 *pubNumAdultMales,
                                                                UINT8 *pubNumAdultFemales);

BOOLEAN PlayerGroupIsInACreatureInfestedMine();

#endif
