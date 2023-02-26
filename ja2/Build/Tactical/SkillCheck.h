#ifndef __SKILL_CHECK_H
#define __SKILL_CHECK_H

#include "SGP/Types.h"

struct SOLDIERTYPE;

extern void ReducePointsForFatigue(struct SOLDIERTYPE *pSoldier, UINT16 *pusPoints);
extern INT32 GetSkillCheckPenaltyForFatigue(struct SOLDIERTYPE *pSoldier, INT32 iSkill);
extern INT32 SkillCheck(struct SOLDIERTYPE *pSoldier, INT8 bReason, INT8 bDifficulty);
extern INT8 CalcTrapDetectLevel(struct SOLDIERTYPE *pSoldier, BOOLEAN fExamining);

extern INT8 EffectiveStrength(struct SOLDIERTYPE *pSoldier);
extern INT8 EffectiveWisdom(struct SOLDIERTYPE *pSoldier);
extern INT8 EffectiveAgility(struct SOLDIERTYPE *pSoldier);
extern INT8 EffectiveMechanical(struct SOLDIERTYPE *pSoldier);
extern INT8 EffectiveExplosive(struct SOLDIERTYPE *pSoldier);
extern INT8 EffectiveLeadership(struct SOLDIERTYPE *pSoldier);
extern INT8 EffectiveMarksmanship(struct SOLDIERTYPE *pSoldier);
extern INT8 EffectiveDexterity(struct SOLDIERTYPE *pSoldier);
extern INT8 EffectiveExpLevel(struct SOLDIERTYPE *pSoldier);
extern INT8 EffectiveMedical(struct SOLDIERTYPE *pSoldier);

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
