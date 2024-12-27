#ifndef __SKILL_CHECK_H
#define __SKILL_CHECK_H

#include "SGP/Types.h"

struct SOLDIERTYPE;

extern void ReducePointsForFatigue(struct SOLDIERTYPE *pSoldier, uint16_t *pusPoints);
extern int32_t GetSkillCheckPenaltyForFatigue(struct SOLDIERTYPE *pSoldier, int32_t iSkill);
extern int32_t SkillCheck(struct SOLDIERTYPE *pSoldier, int8_t bReason, int8_t bDifficulty);
extern int8_t CalcTrapDetectLevel(struct SOLDIERTYPE *pSoldier, BOOLEAN fExamining);

extern int8_t EffectiveStrength(struct SOLDIERTYPE *pSoldier);
extern int8_t EffectiveWisdom(struct SOLDIERTYPE *pSoldier);
extern int8_t EffectiveAgility(struct SOLDIERTYPE *pSoldier);
extern int8_t EffectiveMechanical(struct SOLDIERTYPE *pSoldier);
extern int8_t EffectiveExplosive(struct SOLDIERTYPE *pSoldier);
extern int8_t EffectiveLeadership(struct SOLDIERTYPE *pSoldier);
extern int8_t EffectiveMarksmanship(struct SOLDIERTYPE *pSoldier);
extern int8_t EffectiveDexterity(struct SOLDIERTYPE *pSoldier);
extern int8_t EffectiveExpLevel(struct SOLDIERTYPE *pSoldier);
extern int8_t EffectiveMedical(struct SOLDIERTYPE *pSoldier);

typedef enum {
  NO_CHECK = 0,
  LOCKPICKING_CHECK,
  ELECTRONIC_LOCKPICKING_CHECK,
  ATTACHING_DETONATOR_CHECK,
  ATTACHING_REMOTE_DETONATOR_CHECK,
  PLANTING_BOMB_CHECK,
  PLANTING_REMOTE_BOMB_CHECK,
  OPEN_WITH_CROWBAR,
  SMASH_DOOR_CHECK,
  DISARM_TRAP_CHECK,
  UNJAM_GUN_CHECK,
  NOTICE_DART_CHECK,
  LIE_TO_QUEEN_CHECK,
  ATTACHING_SPECIAL_ITEM_CHECK,
  ATTACHING_SPECIAL_ELECTRONIC_ITEM_CHECK,
  DISARM_ELECTRONIC_TRAP_CHECK,
} SkillChecks;

#endif
