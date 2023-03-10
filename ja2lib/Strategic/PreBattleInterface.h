#ifndef __PREBATTLE_INTERFACE_H
#define __PREBATTLE_INTERFACE_H

#include "SGP/Types.h"
#include "Strategic/StrategicMovement.h"

void InitPreBattleInterface(struct GROUP *pBattleGroup, BOOLEAN fPersistantPBI);
void KillPreBattleInterface();
void RenderPreBattleInterface();

extern BOOLEAN gfPreBattleInterfaceActive;
extern BOOLEAN gfDisplayPotentialRetreatPaths;
extern BOOLEAN gfAutomaticallyStartAutoResolve;
extern BOOLEAN fDisableMapInterfaceDueToBattle;

extern struct GROUP *gpBattleGroup;

extern BOOLEAN gfTacticalTraversal;
extern struct GROUP *gpTacticalTraversalGroup;
extern struct SOLDIERTYPE *gpTacticalTraversalChosenSoldier;

extern BOOLEAN gfGotoSectorTransition;
extern BOOLEAN gfEnteringMapScreenToEnterPreBattleInterface;

enum {
  // General encounter codes (gubEnemyEncounterCode)
  NO_ENCOUNTER_CODE,  // when there is no encounter
  ENEMY_INVASION_CODE,
  ENEMY_ENCOUNTER_CODE,
  ENEMY_AMBUSH_CODE,
  ENTERING_ENEMY_SECTOR_CODE,
  CREATURE_ATTACK_CODE,

  BLOODCAT_AMBUSH_CODE,
  ENTERING_BLOODCAT_LAIR_CODE,

  // Explicit encounter codes only (gubExplicitEnemyEncounterCode -- a superset of
  // gubEnemyEncounterCode)
  FIGHTING_CREATURES_CODE,
  HOSTILE_CIVILIANS_CODE,
  HOSTILE_BLOODCATS_CODE,
};

extern BOOLEAN gfAutoAmbush;
extern BOOLEAN gfHighPotentialForAmbush;

extern void ActivatePreBattleAutoresolveAction();
extern void ActivatePreBattleEnterSectorAction();
extern void ActivatePreBattleRetreatAction();
extern void ActivateAutomaticAutoResolveStart();

void CalculateNonPersistantPBIInfo();
void ClearNonPersistantPBIInfo();

// SAVE START

// Using the ESC key in the PBI will get rid of the PBI and go back to mapscreen, but
// only if the PBI isn't persistant (!gfPersistantPBI).
extern BOOLEAN gfPersistantPBI;

// Contains general information about the type of encounter the player is faced with.  This
// determines whether or not you can autoresolve the battle or even retreat.  This code
// dictates the header that is used at the top of the PBI.
extern uint8_t gubEnemyEncounterCode;

// The autoresolve during tactical battle option needs more detailed information than the
// gubEnemyEncounterCode can provide.  The explicit version contains possibly unique codes
// for reasons not normally used in the PBI.  For example, if we were fighting the enemy
// in a normal situation, then shot at a civilian, the civilians associated with the victim
// would turn hostile, which would disable the ability to autoresolve the battle.
extern BOOLEAN gubExplicitEnemyEncounterCode;

// Location of the current battle (determines where the animated icon is blitted) and if the
// icon is to be blitted.
extern BOOLEAN gfBlitBattleSectorLocator;

extern uint8_t gubPBSectorX;
extern uint8_t gubPBSectorY;
extern uint8_t gubPBSectorZ;

extern BOOLEAN gfCantRetreatInPBI;
// SAVE END

void WakeUpAllMercsInSectorUnderAttack(void);

void ClearMovementForAllInvolvedPlayerGroups(void);
void RetreatAllInvolvedPlayerGroups(void);

BOOLEAN PlayerGroupInvolvedInThisCombat(struct GROUP *pGroup);
BOOLEAN PlayerMercInvolvedInThisCombat(struct SOLDIERTYPE *pSoldier);

BOOLEAN CurrentBattleSectorIs(uint8_t sSectorX, uint8_t sSectorY, int8_t sSectorZ);

enum {
  LOG_DEFEAT,
  LOG_VICTORY,
};
void LogBattleResults(uint8_t ubVictoryCode);

#endif
