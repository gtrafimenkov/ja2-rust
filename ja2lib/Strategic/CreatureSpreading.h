#ifndef __CREATURE_SPREADING_H
#define __CREATURE_SPREADING_H

#include "SGP/Types.h"

void InitCreatureQuest();
void SpreadCreatures();
void DecayCreatures();
void ChooseCreatureQuestStartDay();
void ClearCreatureQuest();
void DeleteCreatureDirectives();

BOOLEAN SaveCreatureDirectives(HWFILE hFile);
BOOLEAN LoadCreatureDirectives(HWFILE hFile, uint32_t uiSavedGameVersion);

uint8_t CreaturesInUndergroundSector(uint8_t ubSectorID, uint8_t ubSectorZ);
BOOLEAN PrepareCreaturesForBattle();
void CreatureNightPlanning();
void CreatureAttackTown(uint8_t ubSectorID, BOOLEAN fOverrideTest);

void CheckConditionsForTriggeringCreatureQuest(u8 sSectorX, u8 sSectorY, INT8 bSectorZ);

void ForceCreaturesToAvoidMineTemporarily(uint8_t ubMineIndex);

extern BOOLEAN gfUseCreatureMusic;

BOOLEAN MineClearOfMonsters(uint8_t ubMineIndex);

// Returns TRUE if valid and creature quest over, FALSE if creature quest active or not yet started
BOOLEAN GetWarpOutOfMineCodes(INT16 *psSectorX, INT16 *psSectorY, INT8 *pbSectorZ,
                              INT16 *psInsertionGridNo);

extern INT16 gsCreatureInsertionCode;
extern INT16 gsCreatureInsertionGridNo;
extern uint8_t gubNumCreaturesAttackingTown;
extern uint8_t gubYoungMalesAttackingTown;
extern uint8_t gubYoungFemalesAttackingTown;
extern uint8_t gubAdultMalesAttackingTown;
extern uint8_t gubAdultFemalesAttackingTown;
extern uint8_t gubSectorIDOfCreatureAttack;
enum {
  CREATURE_BATTLE_CODE_NONE,
  CREATURE_BATTLE_CODE_TACTICALLYADD,
  CREATURE_BATTLE_CODE_TACTICALLYADD_WITHFOV,
  CREATURE_BATTLE_CODE_PREBATTLEINTERFACE,
  CREATURE_BATTLE_CODE_AUTORESOLVE,
};
extern uint8_t gubCreatureBattleCode;

void DetermineCreatureTownComposition(uint8_t ubNumCreatures, uint8_t *pubNumYoungMales,
                                      uint8_t *pubNumYoungFemales, uint8_t *pubNumAdultMales,
                                      uint8_t *pubNumAdultFemales);

void DetermineCreatureTownCompositionBasedOnTacticalInformation(uint8_t *pubNumCreatures,
                                                                uint8_t *pubNumYoungMales,
                                                                uint8_t *pubNumYoungFemales,
                                                                uint8_t *pubNumAdultMales,
                                                                uint8_t *pubNumAdultFemales);

BOOLEAN PlayerGroupIsInACreatureInfestedMine();

#endif
