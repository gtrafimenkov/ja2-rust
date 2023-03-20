#include "Tactical/Keys.h"

#include <memory.h>
#include <stdio.h>

#include "Editor/SmoothingUtils.h"
#include "SGP/Debug.h"
#include "SGP/Random.h"
#include "SGP/Types.h"
#include "SGP/WCheck.h"
#include "Soldier.h"
#include "Strategic/CampaignTypes.h"
#include "Strategic/GameClock.h"
#include "Strategic/MapScreen.h"
#include "Strategic/MapScreenInterface.h"
#include "Strategic/StrategicMap.h"
#include "Tactical/Campaign.h"
#include "Tactical/HandleDoors.h"
#include "Tactical/HandleItems.h"
#include "Tactical/Interface.h"
#include "Tactical/Items.h"
#include "Tactical/LOS.h"
#include "Tactical/OppList.h"
#include "Tactical/Overhead.h"
#include "Tactical/SkillCheck.h"
#include "Tactical/SoldierControl.h"
#include "Tactical/TacticalSave.h"
#include "Tactical/Weapons.h"
#include "TacticalAI/AI.h"
#include "TileEngine/ExplosionControl.h"
#include "TileEngine/IsometricUtils.h"
#include "TileEngine/RenderWorld.h"
#include "TileEngine/Structure.h"
#include "TileEngine/StructureInternals.h"
#include "TileEngine/TileDat.h"
#include "TileEngine/WorldMan.h"
#include "Town.h"
#include "Utils/Message.h"
#include "Utils/SoundControl.h"
#include "Utils/Text.h"
#include "platform.h"
#include "rust_fileman.h"

DOOR_STATUS *gpDoorStatus = NULL;
uint8_t gubNumDoorStatus = 0;
extern int8_t gbMercIsNewInThisSector[MAX_NUM_SOLDIERS];

BOOLEAN InternalIsPerceivedDifferentThanReality(DOOR_STATUS *pDoorStatus);
void InternalUpdateDoorGraphicFromStatus(DOOR_STATUS *pDoorStatus, BOOLEAN fUsePerceivedStatus,
                                         BOOLEAN fDirty);
void InternalUpdateDoorsPerceivedValue(DOOR_STATUS *pDoorStatus);
BOOLEAN InternalSetDoorPerceivedOpenStatus(DOOR_STATUS *pDoorStatus, BOOLEAN fPerceivedOpen);

KEY KeyTable[NUM_KEYS] = {
    // Item #			Flags		Sector, Date Found
    //
    {KEY_1, 0, 0, 0},  {KEY_2, 0, 0, 0},  {KEY_3, 0, 0, 0},  {KEY_4, 0, 0, 0},  {KEY_5, 0, 0, 0},
    {KEY_6, 0, 0, 0},  {KEY_7, 0, 0, 0},  {KEY_8, 0, 0, 0},  {KEY_9, 0, 0, 0},  {KEY_10, 0, 0, 0},

    {KEY_11, 0, 0, 0}, {KEY_12, 0, 0, 0}, {KEY_13, 0, 0, 0}, {KEY_14, 0, 0, 0}, {KEY_15, 0, 0, 0},
    {KEY_16, 0, 0, 0}, {KEY_17, 0, 0, 0}, {KEY_18, 0, 0, 0}, {KEY_19, 0, 0, 0}, {KEY_20, 0, 0, 0},

    {KEY_21, 0, 0, 0}, {KEY_22, 0, 0, 0}, {KEY_23, 0, 0, 0}, {KEY_24, 0, 0, 0}, {KEY_25, 0, 0, 0},
    {KEY_26, 0, 0, 0}, {KEY_27, 0, 0, 0}, {KEY_28, 0, 0, 0}, {KEY_29, 0, 0, 0}, {KEY_30, 0, 0, 0},

    {KEY_31, 0, 0, 0}, {KEY_32, 0, 0, 0}

};

// Current number of doors in world.
uint8_t gubNumDoors = 0;

// Current max number of doors.  This is only used by the editor.  When adding doors to the
// world, we may run out of space in the DoorTable, so we will allocate a new array with extra
// slots, then copy everything over again.  gubMaxDoors holds the arrays actual number of slots,
// even though the current number (gubNumDoors) will be <= to it.
uint8_t gubMaxDoors = 0;

LOCK LockTable[NUM_LOCKS] = {0};

/*
LOCK LockTable[NUM_LOCKS] =
{
        // Keys that will open the lock				Lock type
Pick diff			Smash diff { { NO_KEY, NO_KEY, NO_KEY, NO_KEY},	LOCK_REGULAR,
0,						0},
        { { 0,			NO_KEY, NO_KEY, NO_KEY},	LOCK_REGULAR,
-25,					-25},
        { { 1,			NO_KEY, NO_KEY, NO_KEY},	LOCK_REGULAR,
-60,					-55},
        { { 2,			NO_KEY, NO_KEY, NO_KEY},	LOCK_REGULAR,
-75,					-80},
        { { 3,			NO_KEY, NO_KEY, NO_KEY},	LOCK_REGULAR,
-35,					-45},
        { { 4,			NO_KEY, NO_KEY, NO_KEY},	LOCK_REGULAR,
-45,					-60},
        { { 5,			NO_KEY, NO_KEY, NO_KEY},	LOCK_REGULAR,
-65,					-90},
        { { 6,			NO_KEY, NO_KEY, NO_KEY},	LOCK_PADLOCK,
-60,					-70},
        { { 7,			NO_KEY, NO_KEY, NO_KEY},	LOCK_ELECTRONIC, -50,
-60}, { { 8,			NO_KEY, NO_KEY, NO_KEY},	LOCK_ELECTRONIC,		-75,
-80},
        { { 9,			NO_KEY, NO_KEY, NO_KEY},	LOCK_CARD,
-50,					-40},
        { { 10,			NO_KEY, NO_KEY, NO_KEY},	LOCK_CARD,
-85,					-80},
        { { 11,			NO_KEY, NO_KEY, NO_KEY},	LOCK_REGULAR,
-50,					-50}
};
*/

DOORTRAP DoorTrapTable[NUM_DOOR_TRAPS] = {
    {0},                                             // nothing
    {DOOR_TRAP_STOPS_ACTION},                        // explosion
    {DOOR_TRAP_STOPS_ACTION | DOOR_TRAP_RECURRING},  // electric
    {DOOR_TRAP_RECURRING},                           // siren
    {DOOR_TRAP_RECURRING | DOOR_TRAP_SILENT},        // silent alarm
    {DOOR_TRAP_RECURRING},                           // brothel siren
    {DOOR_TRAP_STOPS_ACTION | DOOR_TRAP_RECURRING},  // super electric
};

// Dynamic array of Doors.  For general game purposes, the doors that are locked and/or trapped
// are permanently saved within the map, and are loaded and allocated when the map is loaded.
// Because the editor allows more doors to be added, or removed, the actual size of the DoorTable
// may change.
DOOR *DoorTable = NULL;

BOOLEAN LoadLockTable(void) {
  uint32_t uiNumBytesRead = 0;
  uint32_t uiBytesToRead;
  char *pFileName = "BINARYDATA\\Locks.bin";
  FileID hFile = FILE_ID_ERR;

  // Load the Lock Table

  hFile = File_OpenForReading(pFileName);
  if (!hFile) {
    DebugMsg(TOPIC_JA2, DBG_INFO, String("FAILED to LoadLockTable from file %s", pFileName));
    return (FALSE);
  }

  uiBytesToRead = sizeof(LOCK) * NUM_LOCKS;
  File_Read(hFile, LockTable, uiBytesToRead, &uiNumBytesRead);

  File_Close(hFile);

  if (uiNumBytesRead != uiBytesToRead) {
    return (FALSE);
  }

  return (TRUE);
}

BOOLEAN SoldierHasKey(struct SOLDIERTYPE *pSoldier, uint8_t ubKeyID) {
  if (KeyExistsInKeyRing(pSoldier, ubKeyID, NULL) || KeyExistsInInventory(pSoldier, ubKeyID)) {
    return (TRUE);
  }

  return (FALSE);
}

BOOLEAN KeyExistsInKeyRing(struct SOLDIERTYPE *pSoldier, uint8_t ubKeyID, uint8_t *pubPos) {
  // returns the index into the key ring where the key can be found
  uint8_t ubLoop;

  if (!(pSoldier->pKeyRing)) {
    // no key ring!
    return (FALSE);
  }
  for (ubLoop = 0; ubLoop < NUM_KEYS; ubLoop++) {
    if (pSoldier->pKeyRing[ubLoop].ubNumber == 0) {
      continue;
    }
    if (pSoldier->pKeyRing[ubLoop].ubKeyID == ubKeyID || (ubKeyID == ANYKEY)) {
      // found it!
      if (pubPos) {
        *pubPos = ubLoop;
      }
      return (TRUE);
    }
  }
  // exhausted key ring
  return (FALSE);
}

BOOLEAN KeyExistsInInventory(struct SOLDIERTYPE *pSoldier, uint8_t ubKeyID) {
  uint8_t ubLoop;

  for (ubLoop = 0; ubLoop < NUM_INV_SLOTS; ubLoop++) {
    if (Item[pSoldier->inv[ubLoop].usItem].usItemClass == IC_KEY) {
      if ((pSoldier->inv[ubLoop].ubKeyID == ubKeyID) || (ubKeyID == ANYKEY)) {
        // there's the key we want!
        return (TRUE);
      }
    }
  }
  return (FALSE);
}

BOOLEAN ValidKey(DOOR *pDoor, uint8_t ubKeyID) { return (pDoor->ubLockID == ubKeyID); }

BOOLEAN DoLockDoor(DOOR *pDoor, uint8_t ubKeyID) {
  // if the door is unlocked and this is the right key, lock the door and
  // return true, otherwise return false
  if (!(pDoor->fLocked) && ValidKey(pDoor, ubKeyID)) {
    pDoor->fLocked = TRUE;
    return (TRUE);
  } else {
    return (FALSE);
  }
}

BOOLEAN DoUnlockDoor(DOOR *pDoor, uint8_t ubKeyID) {
  // if the door is locked and this is the right key, unlock the door and
  // return true, otherwise return false
  if ((pDoor->fLocked) && ValidKey(pDoor, ubKeyID)) {
    // Play lockpicking
    PlayJA2Sample(((uint8_t)UNLOCK_DOOR_1), RATE_11025, SoundVolume(MIDVOLUME, pDoor->sGridNo), 1,
                  SoundDir(pDoor->sGridNo));

    pDoor->fLocked = FALSE;
    return (TRUE);
  } else {
    return (FALSE);
  }
}

BOOLEAN AttemptToUnlockDoor(struct SOLDIERTYPE *pSoldier, DOOR *pDoor) {
  uint8_t ubLoop;
  uint8_t ubKeyID;

  for (ubLoop = 0; ubLoop < MAX_KEYS_PER_LOCK; ubLoop++) {
    ubKeyID = pDoor->ubLockID;
    if (KeyExistsInKeyRing(pSoldier, ubKeyID, NULL)) {
      // unlock door and move key to front of key ring!
      DoUnlockDoor(pDoor, ubKeyID);
      return (TRUE);
    } else if (KeyExistsInInventory(pSoldier, ubKeyID)) {
      // unlock door!
      DoUnlockDoor(pDoor, ubKeyID);
      return (TRUE);
    }
  }

  // drat, couldn't find the key
  PlayJA2Sample(KEY_FAILURE, RATE_11025, MIDVOLUME, 1, MIDDLEPAN);

  return (FALSE);
}

BOOLEAN AttemptToLockDoor(struct SOLDIERTYPE *pSoldier, DOOR *pDoor) {
  uint8_t ubLoop;
  uint8_t ubKeyID;

  for (ubLoop = 0; ubLoop < MAX_KEYS_PER_LOCK; ubLoop++) {
    ubKeyID = pDoor->ubLockID;
    if (KeyExistsInKeyRing(pSoldier, ubKeyID, NULL)) {
      // lock door and move key to front of key ring!
      DoLockDoor(pDoor, ubKeyID);
      return (TRUE);
    } else if (KeyExistsInInventory(pSoldier, ubKeyID)) {
      // lock door!
      DoLockDoor(pDoor, ubKeyID);
      return (TRUE);
    }
  }
  // drat, couldn't find the key
  return (FALSE);
}

BOOLEAN AttemptToCrowbarLock(struct SOLDIERTYPE *pSoldier, DOOR *pDoor) {
  int32_t iResult;
  int8_t bStress, bSlot;

  bSlot = FindUsableObj(pSoldier, CROWBAR);
  if (bSlot == ITEM_NOT_FOUND) {
    // error!
    return (FALSE);
  }

  // generate a noise for thumping on the door
  MakeNoise(pSoldier->ubID, pSoldier->sGridNo, pSoldier->bLevel,
            gpWorldLevelData[pSoldier->sGridNo].ubTerrainID, CROWBAR_DOOR_VOLUME,
            NOISE_DOOR_SMASHING);

  if (!pDoor->fLocked) {
    // auto success but no XP

    // succeeded! door can never be locked again, so remove from door list...
    RemoveDoorInfoFromTable(pDoor->sGridNo);
    // award experience points?

    // Play lock busted sound
    PlayJA2Sample(((uint8_t)BREAK_LOCK), RATE_11025, SoundVolume(MIDVOLUME, pSoldier->sGridNo), 1,
                  SoundDir(pSoldier->sGridNo));

    return (TRUE);
  }

  if (pDoor->ubLockID == LOCK_UNOPENABLE) {
    // auto failure!
    return (FALSE);
  }

  // possibly damage crowbar
  bStress = min(EffectiveStrength(pSoldier), LockTable[pDoor->ubLockID].ubSmashDifficulty + 30);
  // reduce crowbar status by random % between 0 and 5%
  DamageObj(&(pSoldier->inv[bSlot]), (int8_t)PreRandom(bStress / 20));

  // did we succeed?

  if (LockTable[pDoor->ubLockID].ubSmashDifficulty == OPENING_NOT_POSSIBLE) {
    // do this to get 'can't do this' messages
    iResult = SkillCheck(pSoldier, OPEN_WITH_CROWBAR, (int8_t)(-100));
    iResult = -100;
  } else {
    iResult = SkillCheck(
        pSoldier, OPEN_WITH_CROWBAR,
        (int8_t)(-(int8_t)(LockTable[pDoor->ubLockID].ubSmashDifficulty - pDoor->bLockDamage)));
  }

  if (iResult > 0) {
    // char* GAIN (20) - Pried open a lock
    StatChange(pSoldier, STRAMT, 20, FALSE);

    // succeeded! door can never be locked again, so remove from door list...
    RemoveDoorInfoFromTable(pDoor->sGridNo);

    // Play lock busted sound
    PlayJA2Sample(((uint8_t)BREAK_LOCK), RATE_11025, SoundVolume(MIDVOLUME, pSoldier->sGridNo), 1,
                  SoundDir(pSoldier->sGridNo));

    return (TRUE);
  } else {
    if (iResult > -10) {
      // char* GAIN - Damaged a lock by prying
      StatChange(pSoldier, STRAMT, 5, FALSE);

      // we came close... so do some damage to the lock
      pDoor->bLockDamage += (int8_t)(10 + iResult);
    } else if (iResult > -40 && pSoldier->sGridNo != pSoldier->sSkillCheckGridNo) {
      // give token point for effort :-)
      StatChange(pSoldier, STRAMT, 1, FALSE);
    }

    return (FALSE);
  }
}

BOOLEAN AttemptToSmashDoor(struct SOLDIERTYPE *pSoldier, DOOR *pDoor) {
  int32_t iResult;

  LOCK *pLock;

  // generate a noise for thumping on the door
  MakeNoise(pSoldier->ubID, pSoldier->sGridNo, pSoldier->bLevel,
            gpWorldLevelData[pSoldier->sGridNo].ubTerrainID, SMASHING_DOOR_VOLUME,
            NOISE_DOOR_SMASHING);

  if (!pDoor->fLocked) {
    // auto success but no XP

    // succeeded! door can never be locked again, so remove from door list...
    RemoveDoorInfoFromTable(pDoor->sGridNo);
    // award experience points?

    // Play lock busted sound
    PlayJA2Sample(((uint8_t)BREAK_LOCK), RATE_11025, SoundVolume(MIDVOLUME, pSoldier->sGridNo), 1,
                  SoundDir(pSoldier->sGridNo));

    return (TRUE);
  }

  if (pDoor->ubLockID == LOCK_UNOPENABLE) {
    // auto failure!
    return (FALSE);
  }

  pLock = &(LockTable[pDoor->ubLockID]);

  // did we succeed?
  if (pLock->ubSmashDifficulty == OPENING_NOT_POSSIBLE) {
    // do this to get 'can't do this' messages
    iResult = SkillCheck(pSoldier, SMASH_DOOR_CHECK, (int8_t)(-100));
    iResult = -100;
  } else {
    iResult = SkillCheck(
        pSoldier, SMASH_DOOR_CHECK,
        (int8_t)(-(int8_t)(LockTable[pDoor->ubLockID].ubSmashDifficulty - pDoor->bLockDamage)));
  }
  if (iResult > 0) {
    // char* GAIN (20) - Pried open a lock
    StatChange(pSoldier, STRAMT, 20, FALSE);

    // succeeded! door can never be locked again, so remove from door list...
    RemoveDoorInfoFromTable(pDoor->sGridNo);
    // award experience points?

    // Play lock busted sound
    PlayJA2Sample(((uint8_t)BREAK_LOCK), RATE_11025, SoundVolume(MIDVOLUME, pSoldier->sGridNo), 1,
                  SoundDir(pSoldier->sGridNo));

    return (TRUE);
  } else {
    if (iResult > -10) {
      // char* GAIN - Damaged a lock by prying
      StatChange(pSoldier, STRAMT, 5, FALSE);

      // we came close... so do some damage to the lock
      pDoor->bLockDamage += (int8_t)(10 + iResult);
    } else if (iResult > -40 && pSoldier->sGridNo != pSoldier->sSkillCheckGridNo) {
      // give token point for effort :-)
      StatChange(pSoldier, STRAMT, 1, FALSE);
    }
    return (FALSE);
  }
}

BOOLEAN AttemptToPickLock(struct SOLDIERTYPE *pSoldier, DOOR *pDoor) {
  int32_t iResult;
  int8_t bReason;
  LOCK *pLock;

  if (pDoor->ubLockID == LOCK_UNOPENABLE) {
    // auto failure!
    return (FALSE);
  }

  pLock = &(LockTable[pDoor->ubLockID]);

  // look up the type of lock to see if it is electronic or not
  if (pLock->ubLockType == LOCK_CARD || pLock->ubLockType == LOCK_ELECTRONIC) {
    bReason = ELECTRONIC_LOCKPICKING_CHECK;
  } else {
    bReason = LOCKPICKING_CHECK;
  }

  // Play lockpicking
  // ATE: Moved to animation
  // PlayJA2Sample( ( (uint8_t)PICKING_LOCK ), RATE_11025, SoundVolume( MIDVOLUME, pSoldier->sGridNo
  // ), 1, SoundDir( pSoldier->sGridNo ) );

  // See if we measure up to the task.
  // The difficulty is negated here to make it a skill adjustment
  if (pLock->ubPickDifficulty == OPENING_NOT_POSSIBLE) {
    // do this to get 'can't do this' messages
    iResult = SkillCheck(pSoldier, bReason, (int8_t)(-100));
    iResult = -100;
  } else {
    iResult = SkillCheck(pSoldier, bReason, (int8_t)(-(int8_t)(pLock->ubPickDifficulty)));
  }
  if (iResult > 0) {
    // MECHANICAL GAIN:  Picked open a lock
    StatChange(pSoldier, MECHANAMT, (uint16_t)(pLock->ubPickDifficulty / 5), FALSE);

    // DEXTERITY GAIN:  Picked open a lock
    StatChange(pSoldier, DEXTAMT, (uint16_t)(pLock->ubPickDifficulty / 10), FALSE);

    // succeeded!
    pDoor->fLocked = FALSE;
    return (TRUE);
  } else {
    // NOTE: failures are not rewarded, since you can keep trying indefinitely...

    // check for traps
    return (FALSE);
  }
}

BOOLEAN AttemptToUntrapDoor(struct SOLDIERTYPE *pSoldier, DOOR *pDoor) {
  int32_t iResult;

  // See if we measure up to the task.
  if (pDoor->ubTrapID == EXPLOSION) {
    iResult = SkillCheck(pSoldier, DISARM_TRAP_CHECK, (int8_t)(pDoor->ubTrapLevel * 7));
  } else {
    iResult = SkillCheck(pSoldier, DISARM_ELECTRONIC_TRAP_CHECK, (int8_t)(pDoor->ubTrapLevel * 7));
  }

  if (iResult > 0) {
    // succeeded!
    pDoor->ubTrapLevel = 0;
    pDoor->ubTrapID = NO_TRAP;
    return (TRUE);
  } else {
    // trap should REALLY go off now!
    return (FALSE);
  }
}

BOOLEAN ExamineDoorForTraps(struct SOLDIERTYPE *pSoldier, DOOR *pDoor) {
  // Check to see if there is a trap or not on this door
  int8_t bDetectLevel;

  if (pDoor->ubTrapID == NO_TRAP) {
    // No trap!
    pDoor->bPerceivedTrapped = DOOR_PERCEIVED_UNTRAPPED;
  } else {
    if (pDoor->bPerceivedTrapped == DOOR_PERCEIVED_TRAPPED) {
      return (TRUE);
    } else {
      bDetectLevel = CalcTrapDetectLevel(pSoldier, TRUE);
      if (bDetectLevel < pDoor->ubTrapLevel) {
        pDoor->bPerceivedTrapped = DOOR_PERCEIVED_UNTRAPPED;
      } else {
        pDoor->bPerceivedTrapped = DOOR_PERCEIVED_TRAPPED;
        return (TRUE);
      }
    }
  }
  return (FALSE);
}

BOOLEAN HasDoorTrapGoneOff(struct SOLDIERTYPE *pSoldier, DOOR *pDoor) {
  // Check to see if the soldier causes the trap to go off
  int8_t bDetectLevel;

  if (pDoor->ubTrapID != NO_TRAP) {
    // one quick check to see if the guy sees the trap ahead of time!
    bDetectLevel = CalcTrapDetectLevel(pSoldier, FALSE);
    if (bDetectLevel < pDoor->ubTrapLevel) {
      // trap goes off!
      return (TRUE);
    }
  }
  return (FALSE);
}

void HandleDoorTrap(struct SOLDIERTYPE *pSoldier, DOOR *pDoor) {
  if (!(DoorTrapTable[pDoor->ubTrapID].fFlags & DOOR_TRAP_SILENT)) {
    switch (pDoor->ubTrapID) {
      case BROTHEL_SIREN:
        ScreenMsg(MSG_FONT_YELLOW, MSG_INTERFACE, TacticalStr[LOCK_TRAP_HAS_GONE_OFF_STR],
                  pDoorTrapStrings[SIREN]);
        break;
      case SUPER_ELECTRIC:
        ScreenMsg(MSG_FONT_YELLOW, MSG_INTERFACE, TacticalStr[LOCK_TRAP_HAS_GONE_OFF_STR],
                  pDoorTrapStrings[ELECTRIC]);
        break;
      default:
        ScreenMsg(MSG_FONT_YELLOW, MSG_INTERFACE, TacticalStr[LOCK_TRAP_HAS_GONE_OFF_STR],
                  pDoorTrapStrings[pDoor->ubTrapID]);
        break;
    }
  }

  // set trap off
  switch (pDoor->ubTrapID) {
    case EXPLOSION:
      // cause damage as a regular hand grenade
      IgniteExplosion(NOBODY, CenterX(pSoldier->sGridNo), (int16_t)CenterY(pSoldier->sGridNo), 25,
                      pSoldier->sGridNo, HAND_GRENADE, 0);
      break;

    case SIREN:
      // play siren sound effect but otherwise treat as silent alarm, calling
      // available enemies to this location
      PlayJA2Sample(KLAXON_ALARM, RATE_11025, SoundVolume(MIDVOLUME, pDoor->sGridNo), 5,
                    SoundDir(pDoor->sGridNo));
    case SILENT_ALARM:
      // Get all available enemies running here
      CallAvailableEnemiesTo(pDoor->sGridNo);
      break;

    case BROTHEL_SIREN:
      PlayJA2Sample(KLAXON_ALARM, RATE_11025, SoundVolume(MIDVOLUME, pDoor->sGridNo), 5,
                    SoundDir(pDoor->sGridNo));
      CallAvailableKingpinMenTo(pDoor->sGridNo);
      // no one is authorized any more!
      gMercProfiles[MADAME].bNPCData = 0;
      break;

    case ELECTRIC:
      // insert electrical sound effect here
      PlayJA2Sample(DOOR_ELECTRICITY, RATE_11025, SoundVolume(MIDVOLUME, pDoor->sGridNo), 1,
                    SoundDir(pDoor->sGridNo));

      // Set attacker's ID
      pSoldier->ubAttackerID = GetSolID(pSoldier);
      // Increment  being attacked count
      pSoldier->bBeingAttackedCount++;
      gTacticalStatus.ubAttackBusyCount++;
      DebugMsg(TOPIC_JA2, DBG_INFO,
               String("!!!!!!! Trap gone off %d", gTacticalStatus.ubAttackBusyCount));

      SoldierTakeDamage(pSoldier, 0, (uint16_t)(10 + PreRandom(10)),
                        (uint16_t)((3 + PreRandom(3) * 1000)), TAKE_DAMAGE_ELECTRICITY, NOBODY,
                        pDoor->sGridNo, 0, TRUE);
      break;

    case SUPER_ELECTRIC:
      // insert electrical sound effect here
      PlayJA2Sample(DOOR_ELECTRICITY, RATE_11025, SoundVolume(MIDVOLUME, pDoor->sGridNo), 1,
                    SoundDir(pDoor->sGridNo));

      // Set attacker's ID
      pSoldier->ubAttackerID = GetSolID(pSoldier);
      // Increment  being attacked count
      pSoldier->bBeingAttackedCount++;
      gTacticalStatus.ubAttackBusyCount++;
      DebugMsg(TOPIC_JA2, DBG_INFO,
               String("!!!!!!! Trap gone off %d", gTacticalStatus.ubAttackBusyCount));

      SoldierTakeDamage(pSoldier, 0, (uint16_t)(20 + PreRandom(20)),
                        (uint16_t)((6 + PreRandom(6) * 1000)), TAKE_DAMAGE_ELECTRICITY, NOBODY,
                        pDoor->sGridNo, 0, TRUE);
      break;

    default:
      // no trap
      break;
  }
}

BOOLEAN AttemptToBlowUpLock(struct SOLDIERTYPE *pSoldier, DOOR *pDoor) {
  int32_t iResult;
  int8_t bSlot = NO_SLOT;

  bSlot = FindObj(pSoldier, SHAPED_CHARGE);
  if (bSlot == NO_SLOT) {
    return (FALSE);
  }

  iResult = SkillCheck(pSoldier, PLANTING_BOMB_CHECK, 0);
  if (iResult >= -20) {
    // Do explosive graphic....
    {
      ANITILE_PARAMS AniParams;
      int16_t sGridNo;
      int16_t sX, sY, sZ;

      // Get gridno
      sGridNo = pDoor->sGridNo;

      // Get sX, sy;
      sX = CenterX(sGridNo);
      sY = CenterY(sGridNo);

      // Get Z position, based on orientation....
      sZ = 20;

      AniParams.sGridNo = sGridNo;
      AniParams.ubLevelID = ANI_TOPMOST_LEVEL;
      AniParams.sDelay = (int16_t)(100);
      AniParams.sStartFrame = 0;
      AniParams.uiFlags = ANITILE_CACHEDTILE | ANITILE_FORWARD | ANITILE_ALWAYS_TRANSLUCENT;
      AniParams.sX = sX;
      AniParams.sY = sY;
      AniParams.sZ = sZ;

      strcpy(AniParams.zCachedFile, "TILECACHE\\MINIBOOM.STI");

      CreateAnimationTile(&AniParams);

      PlayJA2Sample(SMALL_EXPLODE_1, RATE_11025, SoundVolume((int8_t)HIGHVOLUME, sGridNo), 1,
                    SoundDir(sGridNo));

      // Remove the explosive.....
      bSlot = FindObj(pSoldier, SHAPED_CHARGE);
      if (bSlot != NO_SLOT) {
        RemoveObjs(&(pSoldier->inv[bSlot]), 1);
        DirtyMercPanelInterface(pSoldier, DIRTYLEVEL2);
      }
    }

    // Not sure if this makes sense, but the explosive is small.
    // Double the damage here as we are damaging a lock rather than a person
    pDoor->bLockDamage += Explosive[Item[SHAPED_CHARGE].ubClassIndex].ubDamage * 2;
    if (pDoor->bLockDamage > LockTable[pDoor->ubLockID].ubSmashDifficulty) {
      // succeeded! door can never be locked again, so remove from door list...
      RemoveDoorInfoFromTable(pDoor->sGridNo);
      // award experience points?
      return (TRUE);
    }
  } else {
    bSlot = FindObj(pSoldier, SHAPED_CHARGE);
    if (bSlot != NO_SLOT) {
      RemoveObjs(&(pSoldier->inv[bSlot]), 1);
      DirtyMercPanelInterface(pSoldier, DIRTYLEVEL2);
    }

    // OOPS! ... BOOM!
    IgniteExplosion(NOBODY, pSoldier->sX, pSoldier->sY,
                    (int16_t)(gpWorldLevelData[pSoldier->sGridNo].sHeight), pSoldier->sGridNo,
                    SHAPED_CHARGE, 0);
  }
  return (FALSE);
}

// File I/O for loading the door information from the map.  This automatically allocates
// the exact number of slots when loading.
void LoadDoorTableFromMap(int8_t **hBuffer) {
  int32_t cnt;

  TrashDoorTable();
  LOADDATA(&gubNumDoors, *hBuffer, 1);

  gubMaxDoors = gubNumDoors;
  DoorTable = (DOOR *)MemAlloc(sizeof(DOOR) * gubMaxDoors);

  LOADDATA(DoorTable, *hBuffer, sizeof(DOOR) * gubMaxDoors);

  // OK, reset perceived values to nothing...
  for (cnt = 0; cnt < gubNumDoors; cnt++) {
    DoorTable[cnt].bPerceivedLocked = DOOR_PERCEIVED_UNKNOWN;
    DoorTable[cnt].bPerceivedTrapped = DOOR_PERCEIVED_UNKNOWN;
  }
}

// Saves the existing door information to the map.  Before it actually saves, it'll verify that the
// door still exists.  Otherwise, it'll ignore it.  It is possible in the editor to delete doors in
// many different ways, so I opted to put it in the saving routine.
void SaveDoorTableToMap(FileID fp) {
  int32_t i = 0;
  uint32_t uiBytesWritten;

  while (i < gubNumDoors) {
    if (!OpenableAtGridNo(DoorTable[i].sGridNo))
      RemoveDoorInfoFromTable(DoorTable[i].sGridNo);
    else
      i++;
  }
  File_Write(fp, &gubNumDoors, 1, &uiBytesWritten);
  File_Write(fp, DoorTable, sizeof(DOOR) * gubNumDoors, &uiBytesWritten);
}

// The editor adds locks to the world.  If the gridno already exists, then the currently existing
// door information is overwritten.
void AddDoorInfoToTable(DOOR *pDoor) {
  int32_t i;
  for (i = 0; i < gubNumDoors; i++) {
    if (DoorTable[i].sGridNo == pDoor->sGridNo) {
      memcpy(&DoorTable[i], pDoor, sizeof(DOOR));
      return;
    }
  }
  // no existing door found, so add a new one.
  if (gubNumDoors < gubMaxDoors) {
    memcpy(&DoorTable[gubNumDoors], pDoor, sizeof(DOOR));
    gubNumDoors++;
  } else {  // we need to allocate more memory, so add ten more slots.
    DOOR *NewDoorTable;
    gubMaxDoors += 10;
    // Allocate new table with max+10 doors.
    NewDoorTable = (DOOR *)MemAlloc(sizeof(DOOR) * gubMaxDoors);
    // Copy contents of existing door table to new door table.
    memcpy(NewDoorTable, DoorTable, sizeof(DOOR) * gubNumDoors);
    // Deallocate the existing door table (possible to not have one).
    if (DoorTable) MemFree(DoorTable);
    // Assign the new door table as the existing door table
    DoorTable = NewDoorTable;
    // Add the new door info to the table.
    memcpy(&DoorTable[gubNumDoors], pDoor, sizeof(DOOR));
    gubNumDoors++;
  }
}

// When the editor removes a door from the world, this function looks for and removes accompanying
// door information.  If the entry is not the last entry, the last entry is move to it's current
// slot, to keep everything contiguous.
void RemoveDoorInfoFromTable(int32_t iMapIndex) {
  int32_t i;
  int32_t iNumDoorsToCopy;
  for (i = 0; i < gubNumDoors; i++) {
    if (DoorTable[i].sGridNo == iMapIndex) {
      iNumDoorsToCopy = gubNumDoors - i - 1;
      if (iNumDoorsToCopy) {
        memmove(&DoorTable[i], &DoorTable[i + 1], sizeof(DOOR) * iNumDoorsToCopy);
      }
      gubNumDoors--;
      return;
    }
  }
}

// This is the link to see if a door exists at a gridno.
DOOR *FindDoorInfoAtGridNo(int32_t iMapIndex) {
  int32_t i;
  for (i = 0; i < gubNumDoors; i++) {
    if (DoorTable[i].sGridNo == iMapIndex) return &DoorTable[i];
  }
  return NULL;
}

// Upon world deallocation, the door table needs to be deallocated.  Remember, this function
// resets the values, so make sure you do this before you change gubNumDoors or gubMaxDoors.
void TrashDoorTable() {
  if (DoorTable) MemFree(DoorTable);
  DoorTable = NULL;
  gubNumDoors = 0;
  gubMaxDoors = 0;
}

void UpdateDoorPerceivedValue(DOOR *pDoor) {
  if (pDoor->fLocked) {
    pDoor->bPerceivedLocked = DOOR_PERCEIVED_LOCKED;
  } else if (!pDoor->fLocked) {
    pDoor->bPerceivedLocked = DOOR_PERCEIVED_UNLOCKED;
  }

  if (pDoor->ubTrapID != NO_TRAP) {
    pDoor->bPerceivedTrapped = DOOR_PERCEIVED_TRAPPED;
  } else {
    pDoor->bPerceivedTrapped = DOOR_PERCEIVED_UNTRAPPED;
  }
}

BOOLEAN SaveDoorTableToDoorTableTempFile(uint8_t sSectorX, uint8_t sSectorY, int8_t bSectorZ) {
  uint32_t uiNumBytesWritten;
  uint32_t uiSizeToSave = 0;
  char zMapName[128];
  FileID hFile = FILE_ID_ERR;

  //	return( TRUE );

  uiSizeToSave = gubNumDoors * sizeof(DOOR);

  // Convert the current sector location into a file name
  //	GetMapFileName( sSectorX, sSectorY, bSectorZ, zTempName, FALSE );

  // add the 'd' for 'Door' to the front of the map name
  //	sprintf( zMapName, "%s\\d_%s", MAPS_DIR, zTempName);

  GetMapTempFileName(SF_DOOR_TABLE_TEMP_FILES_EXISTS, zMapName, sSectorX, sSectorY, bSectorZ);

  // if the file already exists, delete it
  if (File_Exists(zMapName)) {
    // We are going to be overwriting the file
    if (!Plat_DeleteFile(zMapName)) {
      return (FALSE);
    }
  }

  // Open the file for writing, Create it if it doesnt exist
  hFile = File_OpenForAppending(zMapName);
  if (hFile == 0) {
    // Error opening map modification file
    return (FALSE);
  }

  // Save the number of doors
  File_Write(hFile, &gubNumDoors, sizeof(uint8_t), &uiNumBytesWritten);
  if (uiNumBytesWritten != sizeof(uint8_t)) {
    File_Close(hFile);
    return (FALSE);
  }

  // if there are doors to save
  if (uiSizeToSave != 0) {
    // Save the door table
    File_Write(hFile, DoorTable, uiSizeToSave, &uiNumBytesWritten);
    if (uiNumBytesWritten != uiSizeToSave) {
      File_Close(hFile);
      return (FALSE);
    }
  }

  // Set the sector flag indicating that there is a Door table temp file present
  SetSectorFlag((uint8_t)gWorldSectorX, (uint8_t)gWorldSectorY, gbWorldSectorZ,
                SF_DOOR_TABLE_TEMP_FILES_EXISTS);

  File_Close(hFile);

  return (TRUE);
}

BOOLEAN LoadDoorTableFromDoorTableTempFile() {
  uint32_t uiNumBytesRead;
  FileID hFile = FILE_ID_ERR;
  char zMapName[128];

  //	return( TRUE );

  // Convert the current sector location into a file name
  //	GetMapFileName( gWorldSectorX, gWorldSectorY, gbWorldSectorZ, zTempName, FALSE );

  // add the 'd' for 'Door' to the front of the map name
  //	sprintf( zMapName, "%s\\d_%s", MAPS_DIR, zTempName);

  GetMapTempFileName(SF_DOOR_TABLE_TEMP_FILES_EXISTS, zMapName, (uint8_t)gWorldSectorX,
                     (uint8_t)gWorldSectorY, gbWorldSectorZ);

  // Check to see if the file exists
  if (!File_Exists(zMapName)) {
    // If the file doesnt exists, its no problem.
    return (TRUE);
  }

  // Get rid of the existing door table
  TrashDoorTable();

  // Open the file for reading
  hFile = File_OpenForReading(zMapName);
  if (hFile == 0) {
    // Error opening map modification file,
    return (FALSE);
  }

  // Read in the number of doors
  File_Read(hFile, &gubMaxDoors, sizeof(uint8_t), &uiNumBytesRead);
  if (uiNumBytesRead != sizeof(uint8_t)) {
    File_Close(hFile);
    return (FALSE);
  }

  gubNumDoors = gubMaxDoors;

  // if there is no doors to load
  if (gubNumDoors != 0) {
    // Allocate space for the door table
    DoorTable = (DOOR *)MemAlloc(sizeof(DOOR) * gubMaxDoors);
    if (DoorTable == NULL) {
      File_Close(hFile);
      return (FALSE);
    }

    // Read in the number of doors
    File_Read(hFile, DoorTable, sizeof(DOOR) * gubMaxDoors, &uiNumBytesRead);
    if (uiNumBytesRead != sizeof(DOOR) * gubMaxDoors) {
      File_Close(hFile);
      return (FALSE);
    }
  }

  File_Close(hFile);

  return (TRUE);
}

// fOpen is True if the door is open, false if it is closed
BOOLEAN ModifyDoorStatus(int16_t sGridNo, BOOLEAN fOpen, BOOLEAN fPerceivedOpen) {
  uint8_t ubCnt;
  struct STRUCTURE *pStructure;
  struct STRUCTURE *pBaseStructure;

  // Set the gridno for the door

  // Find the base tile for the door structure and use that gridno
  pStructure = FindStructure(sGridNo, STRUCTURE_ANYDOOR);
  if (pStructure) {
    pBaseStructure = FindBaseStructure(pStructure);
  } else {
    pBaseStructure = NULL;
  }

  if (pBaseStructure == NULL) {
#if 0
		ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_BETAVERSION, L"Door structure data at %d was not found", sGridNo );
#endif
    return (FALSE);
  }

  // if there is an array
  if (gpDoorStatus) {
    // Check to see if the user is adding an existing door
    for (ubCnt = 0; ubCnt < gubNumDoorStatus; ubCnt++) {
      // if the door is already in the array
      if (gpDoorStatus[ubCnt].sGridNo == pBaseStructure->sGridNo) {
        // set the status
        // ATE: Don't set if set to DONTSET
        if (fPerceivedOpen != DONTSETDOORSTATUS) {
          if (fPerceivedOpen)
            gpDoorStatus[ubCnt].ubFlags |= DOOR_PERCEIVED_OPEN;
          else
            gpDoorStatus[ubCnt].ubFlags &= ~DOOR_PERCEIVED_OPEN;

          // Turn off perceived not set flag....
          gpDoorStatus[ubCnt].ubFlags &= ~DOOR_PERCEIVED_NOTSET;
        }

        if (fOpen != DONTSETDOORSTATUS) {
          if (fOpen)
            gpDoorStatus[ubCnt].ubFlags |= DOOR_OPEN;
          else
            gpDoorStatus[ubCnt].ubFlags &= ~DOOR_OPEN;
        }

        // Dont add it
        return (TRUE);
      }
    }
  }

  // add a new door status structure

  // if there is an array
  if (gpDoorStatus) {
    // Increment the number of doors
    gubNumDoorStatus++;

    // reallocate memory to hold the new door
    gpDoorStatus = (DOOR_STATUS *)MemRealloc(gpDoorStatus, sizeof(DOOR_STATUS) * gubNumDoorStatus);
    if (gpDoorStatus == NULL) return (FALSE);

  } else {
    // Set the initial number of doors
    gubNumDoorStatus = 1;

    gpDoorStatus = (DOOR_STATUS *)MemAlloc(sizeof(DOOR_STATUS));
    if (gpDoorStatus == NULL) return (FALSE);
  }

  gpDoorStatus[gubNumDoorStatus - 1].sGridNo = pBaseStructure->sGridNo;

  // Init the flags
  gpDoorStatus[gubNumDoorStatus - 1].ubFlags = 0;

  // If the door is to be initially open
  if (fOpen) gpDoorStatus[gubNumDoorStatus - 1].ubFlags |= DOOR_OPEN;

  // IF A NEW DOOR, USE SAME AS ACTUAL
  if (fPerceivedOpen != DONTSETDOORSTATUS) {
    if (fOpen) gpDoorStatus[gubNumDoorStatus - 1].ubFlags |= DOOR_PERCEIVED_OPEN;
  } else {
    gpDoorStatus[gubNumDoorStatus - 1].ubFlags |= DOOR_PERCEIVED_NOTSET;
  }

  // flag the tile as containing a door status
  gpWorldLevelData[pBaseStructure->sGridNo].ubExtFlags[0] |= MAPELEMENT_EXT_DOOR_STATUS_PRESENT;

  return (TRUE);
}

void TrashDoorStatusArray() {
  if (gpDoorStatus) {
    MemFree(gpDoorStatus);
    gpDoorStatus = NULL;
  }

  gubNumDoorStatus = 0;
}

BOOLEAN IsDoorOpen(int16_t sGridNo) {
  uint8_t ubCnt;
  struct STRUCTURE *pStructure;
  struct STRUCTURE *pBaseStructure;

  // Find the base tile for the door structure and use that gridno
  pStructure = FindStructure(sGridNo, STRUCTURE_ANYDOOR);
  if (pStructure) {
    pBaseStructure = FindBaseStructure(pStructure);
  } else {
    pBaseStructure = NULL;
  }

  if (pBaseStructure == NULL) {
#if 0
		ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_BETAVERSION, L"Door structure data at %d was not found", sGridNo );
#endif
    return (FALSE);
  }

  // if there is an array
  if (gpDoorStatus) {
    // Check to see if the user is adding an existing door
    for (ubCnt = 0; ubCnt < gubNumDoorStatus; ubCnt++) {
      // if this is the door
      if (gpDoorStatus[ubCnt].sGridNo == pBaseStructure->sGridNo) {
        if (gpDoorStatus[ubCnt].ubFlags & DOOR_OPEN)
          return (TRUE);
        else
          return (FALSE);
      }
    }
  }

#ifdef JA2TESTVERSION
  ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_TESTVERSION,
            L"WARNING! Failed to find the Door Open Status on Gridno %s", sGridNo);
#endif

  return (FALSE);
}

// Returns a doors status value, NULL if not found
DOOR_STATUS *GetDoorStatus(int16_t sGridNo) {
  uint8_t ubCnt;
  struct STRUCTURE *pStructure;
  struct STRUCTURE *pBaseStructure;

  // if there is an array
  if (gpDoorStatus) {
    // Find the base tile for the door structure and use that gridno
    pStructure = FindStructure(sGridNo, STRUCTURE_ANYDOOR);
    if (pStructure) {
      pBaseStructure = FindBaseStructure(pStructure);
    } else {
      pBaseStructure = NULL;
    }

    if (pBaseStructure == NULL) {
      return (NULL);
    }

    // Check to see if the user is adding an existing door
    for (ubCnt = 0; ubCnt < gubNumDoorStatus; ubCnt++) {
      // if this is the door
      if (gpDoorStatus[ubCnt].sGridNo == pBaseStructure->sGridNo) {
        return (&(gpDoorStatus[ubCnt]));
      }
    }
  }

  return (NULL);
}

BOOLEAN AllMercsLookForDoor(int16_t sGridNo, BOOLEAN fUpdateValue) {
  int32_t cnt, cnt2;
  int8_t bDirs[8] = {NORTH, SOUTH, EAST, WEST, NORTHEAST, NORTHWEST, SOUTHEAST, SOUTHWEST};
  struct SOLDIERTYPE *pSoldier;
  int16_t sDistVisible;
  DOOR_STATUS *pDoorStatus;
  int16_t usNewGridNo;

  // Get door
  pDoorStatus = GetDoorStatus(sGridNo);

  if (pDoorStatus == NULL) {
    return (FALSE);
  }

  // IF IT'S THE SELECTED GUY, MAKE ANOTHER SELECTED!
  cnt = gTacticalStatus.Team[gbPlayerNum].bFirstID;

  // look for all mercs on the same team,
  for (pSoldier = MercPtrs[cnt]; cnt <= gTacticalStatus.Team[gbPlayerNum].bLastID;
       cnt++, pSoldier++) {
    // ATE: Ok, lets check for some basic things here!
    if (pSoldier->bLife >= OKLIFE && pSoldier->sGridNo != NOWHERE && IsSolActive(pSoldier) &&
        pSoldier->bInSector) {
      // is he close enough to see that gridno if he turns his head?
      sDistVisible =
          DistanceVisible(pSoldier, DIRECTION_IRRELEVANT, DIRECTION_IRRELEVANT, sGridNo, 0);

      if (PythSpacesAway(pSoldier->sGridNo, sGridNo) <= sDistVisible) {
        // and we can trace a line of sight to his x,y coordinates?
        // (taking into account we are definitely aware of this guy now)
        if (SoldierTo3DLocationLineOfSightTest(pSoldier, sGridNo, 0, 0, (uint8_t)sDistVisible,
                                               TRUE)) {
          // Update status...
          if (fUpdateValue) {
            InternalUpdateDoorsPerceivedValue(pDoorStatus);
          }
          return (TRUE);
        }
      }

      // Now try other adjacent gridnos...
      for (cnt2 = 0; cnt2 < 8; cnt2++) {
        usNewGridNo = NewGridNo(sGridNo, DirectionInc(bDirs[cnt2]));
        sDistVisible =
            DistanceVisible(pSoldier, DIRECTION_IRRELEVANT, DIRECTION_IRRELEVANT, usNewGridNo, 0);

        if (PythSpacesAway(pSoldier->sGridNo, usNewGridNo) <= sDistVisible) {
          // and we can trace a line of sight to his x,y coordinates?
          // (taking into account we are definitely aware of this guy now)
          if (SoldierTo3DLocationLineOfSightTest(pSoldier, usNewGridNo, 0, 0, (uint8_t)sDistVisible,
                                                 TRUE)) {
            // Update status...
            if (fUpdateValue) {
              InternalUpdateDoorsPerceivedValue(pDoorStatus);
            }
            return (TRUE);
          }
        }
      }
    }
  }

  return (FALSE);
}

BOOLEAN MercLooksForDoors(struct SOLDIERTYPE *pSoldier, BOOLEAN fUpdateValue) {
  int32_t cnt, cnt2;
  int16_t sDistVisible;
  int16_t sGridNo;
  DOOR_STATUS *pDoorStatus;
  int8_t bDirs[8] = {NORTH, SOUTH, EAST, WEST, NORTHEAST, NORTHWEST, SOUTHEAST, SOUTHWEST};
  int16_t usNewGridNo;

  // Loop through all corpses....
  for (cnt = 0; cnt < gubNumDoorStatus; cnt++) {
    pDoorStatus = &(gpDoorStatus[cnt]);

    if (!InternalIsPerceivedDifferentThanReality(pDoorStatus)) {
      continue;
    }

    sGridNo = pDoorStatus->sGridNo;

    // is he close enough to see that gridno if he turns his head?
    sDistVisible =
        DistanceVisible(pSoldier, DIRECTION_IRRELEVANT, DIRECTION_IRRELEVANT, sGridNo, 0);

    if (PythSpacesAway(pSoldier->sGridNo, sGridNo) <= sDistVisible) {
      // and we can trace a line of sight to his x,y coordinates?
      // (taking into account we are definitely aware of this guy now)
      if (SoldierTo3DLocationLineOfSightTest(pSoldier, sGridNo, 0, 0, (uint8_t)sDistVisible,
                                             TRUE)) {
        // OK, here... update perceived value....
        if (fUpdateValue) {
          InternalUpdateDoorsPerceivedValue(pDoorStatus);

          // Update graphic....
          InternalUpdateDoorGraphicFromStatus(pDoorStatus, TRUE, TRUE);
        }
        return (TRUE);
      }
    }

    // Now try other adjacent gridnos...
    for (cnt2 = 0; cnt2 < 8; cnt2++) {
      usNewGridNo = NewGridNo(sGridNo, DirectionInc(bDirs[cnt2]));

      if (PythSpacesAway(pSoldier->sGridNo, usNewGridNo) <= sDistVisible) {
        // and we can trace a line of sight to his x,y coordinates?
        // (taking into account we are definitely aware of this guy now)
        if (SoldierTo3DLocationLineOfSightTest(pSoldier, usNewGridNo, 0, 0, (uint8_t)sDistVisible,
                                               TRUE)) {
          // Update status...
          if (fUpdateValue) {
            InternalUpdateDoorsPerceivedValue(pDoorStatus);

            // Update graphic....
            InternalUpdateDoorGraphicFromStatus(pDoorStatus, TRUE, TRUE);
          }
          return (TRUE);
        }
      }
    }
  }

  return (FALSE);
}

void SyncronizeDoorStatusToStructureData(DOOR_STATUS *pDoorStatus) {
  struct STRUCTURE *pStructure, *pBaseStructure;
  struct LEVELNODE *pNode;
  int16_t sBaseGridNo = NOWHERE;

  // First look for a door structure here...
  pStructure = FindStructure(pDoorStatus->sGridNo, STRUCTURE_ANYDOOR);

  if (pStructure) {
    pBaseStructure = FindBaseStructure(pStructure);
    sBaseGridNo = pBaseStructure->sGridNo;
  } else {
    pBaseStructure = NULL;
  }

  if (pBaseStructure == NULL) {
#if 0
		ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_BETAVERSION, L"Door structure data at %d was not found", pDoorStatus->sGridNo );
#endif
    return;
  }

  pNode = FindLevelNodeBasedOnStructure(sBaseGridNo, pBaseStructure);
  if (!pNode) {
#ifdef JA2BETAVERSION
    ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_BETAVERSION,
              L"Could not find levelnode from door structure at %d", pDoorStatus->sGridNo);
#endif
    return;
  }

  // ATE: OK let me explain something here:
  // One of the purposes of this function is to MAKE sure the door status MATCHES
  // the struct data value - if not - change ( REGARDLESS of perceived being used or not... )
  //
  // Check for opened...
  if (pDoorStatus->ubFlags & DOOR_OPEN) {
    // IF closed.....
    if (!(pStructure->fFlags & STRUCTURE_OPEN)) {
      // Swap!
      SwapStructureForPartner(sBaseGridNo, pBaseStructure);
      RecompileLocalMovementCosts(sBaseGridNo);
    }
  } else {
    if ((pStructure->fFlags & STRUCTURE_OPEN)) {
      // Swap!
      SwapStructureForPartner(sBaseGridNo, pBaseStructure);
      RecompileLocalMovementCosts(sBaseGridNo);
    }
  }
}

void UpdateDoorGraphicsFromStatus(BOOLEAN fUsePerceivedStatus, BOOLEAN fDirty) {
  int32_t cnt;
  DOOR_STATUS *pDoorStatus;

  for (cnt = 0; cnt < gubNumDoorStatus; cnt++) {
    pDoorStatus = &(gpDoorStatus[cnt]);

    // ATE: Make sure door status flag and struct info are syncronized....
    SyncronizeDoorStatusToStructureData(pDoorStatus);

    InternalUpdateDoorGraphicFromStatus(pDoorStatus, fUsePerceivedStatus, fDirty);
  }
}

void InternalUpdateDoorGraphicFromStatus(DOOR_STATUS *pDoorStatus, BOOLEAN fUsePerceivedStatus,
                                         BOOLEAN fDirty) {
  struct STRUCTURE *pStructure, *pBaseStructure;
  int32_t cnt;
  BOOLEAN fOpenedGraphic = FALSE;
  struct LEVELNODE *pNode;
  BOOLEAN fWantToBeOpen = FALSE;
  BOOLEAN fDifferent = FALSE;
  int16_t sBaseGridNo = NOWHERE;

  // OK, look at perceived status and adjust graphic
  // First look for a door structure here...
  pStructure = FindStructure(pDoorStatus->sGridNo, STRUCTURE_ANYDOOR);

  if (pStructure) {
    pBaseStructure = FindBaseStructure(pStructure);
    sBaseGridNo = pBaseStructure->sGridNo;
  } else {
    pBaseStructure = NULL;
  }

  if (pBaseStructure == NULL) {
#if 0
		ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_BETAVERSION, L"Door structure data at %d was not found", pDoorStatus->sGridNo );
#endif
    return;
  }

  pNode = FindLevelNodeBasedOnStructure(sBaseGridNo, pBaseStructure);
  if (!pNode) {
#ifdef JA2BETAVERSION
    ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_BETAVERSION,
              L"Could not find levelnode from door structure at %d", pDoorStatus->sGridNo);
#endif
    return;
  }

  // Get status we want to chenge to.....
  if (fUsePerceivedStatus) {
    if (pDoorStatus->ubFlags & DOOR_PERCEIVED_OPEN) {
      fWantToBeOpen = TRUE;
    }
  } else {
    if (pDoorStatus->ubFlags & DOOR_OPEN) {
      fWantToBeOpen = TRUE;
    }
  }

  // First look for an opened door
  // get what it is now...
  cnt = 0;
  while (gClosedDoorList[cnt] != -1) {
    // IF WE ARE A SHADOW TYPE
    if (pNode->usIndex == gClosedDoorList[cnt]) {
      fOpenedGraphic = TRUE;
      break;
    }
    cnt++;
  };

  // OK, we either have an opened graphic, in which case we want to switch to the closed, or a
  // closed in which case we want to switch to opened... adjust o' graphic

  // OK, we now need to test these things against the true structure data
  // we may need to only adjust the graphic here....
  if (fWantToBeOpen && (pStructure->fFlags & STRUCTURE_OPEN)) {
    BOOLEAN fFound = FALSE;
    // Adjust graphic....

    // Loop through and and find opened graphic for the closed one....
    cnt = 0;
    while (gOpenDoorList[cnt] != -1) {
      // IF WE ARE A SHADOW TYPE
      if (pNode->usIndex == gOpenDoorList[cnt]) {
        fFound = TRUE;
        break;
      }
      cnt++;
    };

    // OK, now use opened graphic.
    if (fFound) {
      pNode->usIndex = gClosedDoorList[cnt];

      if (fDirty) {
        InvalidateWorldRedundency();
        SetRenderFlags(RENDER_FLAG_FULL);
      }
    }

    return;
  }

  // If we want to be closed but structure is closed
  if (!fWantToBeOpen && !(pStructure->fFlags & STRUCTURE_OPEN)) {
    BOOLEAN fFound = FALSE;
    // Adjust graphic....

    // Loop through and and find closed graphic for the opend one....
    cnt = 0;
    while (gClosedDoorList[cnt] != -1) {
      // IF WE ARE A SHADOW TYPE
      if (pNode->usIndex == gClosedDoorList[cnt]) {
        fFound = TRUE;
        break;
      }
      cnt++;
    };

    // OK, now use opened graphic.
    if (fFound) {
      pNode->usIndex = gOpenDoorList[cnt];

      if (fDirty) {
        InvalidateWorldRedundency();
        SetRenderFlags(RENDER_FLAG_FULL);
      }
    }

    return;
  }

  if (fOpenedGraphic && !fWantToBeOpen) {
    // Close the beast!
    fDifferent = TRUE;
    pNode->usIndex = gOpenDoorList[cnt];
  } else if (!fOpenedGraphic && fWantToBeOpen) {
    // Find the closed door graphic and adjust....
    cnt = 0;
    while (gOpenDoorList[cnt] != -1) {
      // IF WE ARE A SHADOW TYPE
      if (pNode->usIndex == gOpenDoorList[cnt]) {
        // Open the beast!
        fDifferent = TRUE;
        pNode->usIndex = gClosedDoorList[cnt];
        break;
      }
      cnt++;
    };
  }

  if (fDifferent) {
    SwapStructureForPartner(sBaseGridNo, pBaseStructure);

    RecompileLocalMovementCosts(sBaseGridNo);

    if (fDirty) {
      InvalidateWorldRedundency();
      SetRenderFlags(RENDER_FLAG_FULL);
    }
  }
}

BOOLEAN InternalIsPerceivedDifferentThanReality(DOOR_STATUS *pDoorStatus) {
  if ((pDoorStatus->ubFlags & DOOR_PERCEIVED_NOTSET)) {
    return (TRUE);
  }

  // Compare flags....
  if ((pDoorStatus->ubFlags & DOOR_OPEN && pDoorStatus->ubFlags & DOOR_PERCEIVED_OPEN) ||
      (!(pDoorStatus->ubFlags & DOOR_OPEN) && !(pDoorStatus->ubFlags & DOOR_PERCEIVED_OPEN))) {
    return (FALSE);
  }

  return (TRUE);
}

void InternalUpdateDoorsPerceivedValue(DOOR_STATUS *pDoorStatus) {
  // OK, look at door, set perceived value the same as actual....
  if (pDoorStatus->ubFlags & DOOR_OPEN) {
    InternalSetDoorPerceivedOpenStatus(pDoorStatus, TRUE);
  } else {
    InternalSetDoorPerceivedOpenStatus(pDoorStatus, FALSE);
  }
}

BOOLEAN UpdateDoorStatusPerceivedValue(int16_t sGridNo) {
  DOOR_STATUS *pDoorStatus = NULL;

  pDoorStatus = GetDoorStatus(sGridNo);
  CHECKF(pDoorStatus != NULL);

  InternalUpdateDoorsPerceivedValue(pDoorStatus);

  return (TRUE);
}

BOOLEAN IsDoorPerceivedOpen(int16_t sGridNo) {
  DOOR_STATUS *pDoorStatus;

  pDoorStatus = GetDoorStatus(sGridNo);

  if (pDoorStatus && pDoorStatus->ubFlags & DOOR_PERCEIVED_OPEN) {
    return (TRUE);
  } else {
#ifdef JA2TESTVERSION
    if (!pDoorStatus) {
      ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_TESTVERSION,
                L"WARNING! Failed to find the Perceived Open Door Status on Gridno %s", sGridNo);
    }
#endif

    return (FALSE);
  }
}

BOOLEAN InternalSetDoorPerceivedOpenStatus(DOOR_STATUS *pDoorStatus, BOOLEAN fPerceivedOpen) {
  if (fPerceivedOpen)
    pDoorStatus->ubFlags |= DOOR_PERCEIVED_OPEN;
  else
    pDoorStatus->ubFlags &= ~DOOR_PERCEIVED_OPEN;

  // Turn off perceived not set flag....
  pDoorStatus->ubFlags &= ~DOOR_PERCEIVED_NOTSET;

  return (TRUE);
}

BOOLEAN SetDoorPerceivedOpenStatus(int16_t sGridNo, BOOLEAN fPerceivedOpen) {
  DOOR_STATUS *pDoorStatus = NULL;

  pDoorStatus = GetDoorStatus(sGridNo);

  CHECKF(pDoorStatus != NULL);

  return (InternalSetDoorPerceivedOpenStatus(pDoorStatus, fPerceivedOpen));
}

BOOLEAN SetDoorOpenStatus(int16_t sGridNo, BOOLEAN fOpen) {
  DOOR_STATUS *pDoorStatus;

  pDoorStatus = GetDoorStatus(sGridNo);

  if (pDoorStatus) {
    if (fOpen) {
      pDoorStatus->ubFlags |= DOOR_OPEN;
    } else {
      pDoorStatus->ubFlags &= ~DOOR_OPEN;
    }
    return (TRUE);
  } else {
    return (FALSE);
  }
}

BOOLEAN SaveDoorStatusArrayToDoorStatusTempFile(uint8_t sSectorX, uint8_t sSectorY,
                                                int8_t bSectorZ) {
  char zMapName[128];
  FileID hFile = FILE_ID_ERR;
  uint32_t uiNumBytesWritten;
  uint8_t ubCnt;

  // Turn off any DOOR BUSY flags....
  for (ubCnt = 0; ubCnt < gubNumDoorStatus; ubCnt++) {
    gpDoorStatus[ubCnt].ubFlags &= (~DOOR_BUSY);
  }

  // Convert the current sector location into a file name
  //	GetMapFileName( sSectorX, sSectorY, bSectorZ, zTempName, FALSE );

  // add the 'm' for 'Modifed Map' to the front of the map name
  //	sprintf( zMapName, "%s\\ds_%s", MAPS_DIR, zTempName);

  GetMapTempFileName(SF_DOOR_STATUS_TEMP_FILE_EXISTS, zMapName, sSectorX, sSectorY, bSectorZ);

  // Open the file for writing, Create it if it doesnt exist
  hFile = File_OpenForAppending(zMapName);
  if (hFile == 0) {
    // Error opening map modification file
    return (FALSE);
  }

  // Save the number of elements in the door array
  File_Write(hFile, &gubNumDoorStatus, sizeof(uint8_t), &uiNumBytesWritten);
  if (uiNumBytesWritten != sizeof(uint8_t)) {
    // Error Writing size of array to disk
    File_Close(hFile);
    return (FALSE);
  }

  // if there is some to save
  if (gubNumDoorStatus != 0) {
    // Save the door array
    File_Write(hFile, gpDoorStatus, (sizeof(DOOR_STATUS) * gubNumDoorStatus), &uiNumBytesWritten);
    if (uiNumBytesWritten != (sizeof(DOOR_STATUS) * gubNumDoorStatus)) {
      // Error Writing size of array to disk
      File_Close(hFile);
      return (FALSE);
    }
  }

  File_Close(hFile);

  // Set the flag indicating that there is a door status array
  SetSectorFlag(sSectorX, sSectorY, bSectorZ, SF_DOOR_STATUS_TEMP_FILE_EXISTS);

  return (TRUE);
}

BOOLEAN LoadDoorStatusArrayFromDoorStatusTempFile() {
  char zMapName[128];
  FileID hFile = FILE_ID_ERR;
  uint32_t uiNumBytesRead;
  uint8_t ubLoop;

  // Convert the current sector location into a file name
  //	GetMapFileName( gWorldSectorX, gWorldSectorY, gbWorldSectorZ, zTempName, FALSE );

  // add the 'm' for 'Modifed Map' to the front of the map name
  //	sprintf( zMapName, "%s\\ds_%s", MAPS_DIR, zTempName);

  GetMapTempFileName(SF_DOOR_STATUS_TEMP_FILE_EXISTS, zMapName, (uint8_t)gWorldSectorX,
                     (uint8_t)gWorldSectorY, gbWorldSectorZ);

  // Get rid of the existing door array
  TrashDoorStatusArray();

  // Open the file for reading
  hFile = File_OpenForReading(zMapName);
  if (hFile == 0) {
    // Error opening map modification file,
    return (FALSE);
  }

  // Load the number of elements in the door status array
  File_Read(hFile, &gubNumDoorStatus, sizeof(uint8_t), &uiNumBytesRead);
  if (uiNumBytesRead != sizeof(uint8_t)) {
    File_Close(hFile);
    return (FALSE);
  }

  if (gubNumDoorStatus == 0) {
    File_Close(hFile);
    return (TRUE);
  }

  // Allocate space for the door status array
  gpDoorStatus = (DOOR_STATUS *)MemAlloc(sizeof(DOOR_STATUS) * gubNumDoorStatus);
  if (gpDoorStatus == NULL) AssertMsg(0, "Error Allocating memory for the gpDoorStatus");
  memset(gpDoorStatus, 0, sizeof(DOOR_STATUS) * gubNumDoorStatus);

  // Load the number of elements in the door status array
  File_Read(hFile, gpDoorStatus, (sizeof(DOOR_STATUS) * gubNumDoorStatus), &uiNumBytesRead);
  if (uiNumBytesRead != (sizeof(DOOR_STATUS) * gubNumDoorStatus)) {
    File_Close(hFile);
    return (FALSE);
  }

  File_Close(hFile);

  // the graphics will be updated later in the loading process.

  // set flags in map for containing a door status
  for (ubLoop = 0; ubLoop < gubNumDoorStatus; ubLoop++) {
    gpWorldLevelData[gpDoorStatus[ubLoop].sGridNo].ubExtFlags[0] |=
        MAPELEMENT_EXT_DOOR_STATUS_PRESENT;
  }

  UpdateDoorGraphicsFromStatus(TRUE, FALSE);

  return (TRUE);
}

BOOLEAN SaveKeyTableToSaveGameFile(FileID hFile) {
  uint32_t uiNumBytesWritten = 0;

  // Save the KeyTable
  File_Write(hFile, KeyTable, sizeof(KEY) * NUM_KEYS, &uiNumBytesWritten);
  if (uiNumBytesWritten != sizeof(KEY) * NUM_KEYS) {
    return (FALSE);
  }

  return (TRUE);
}

BOOLEAN LoadKeyTableFromSaveedGameFile(FileID hFile) {
  uint32_t uiNumBytesRead = 0;

  // Load the KeyTable
  File_Read(hFile, KeyTable, sizeof(KEY) * NUM_KEYS, &uiNumBytesRead);
  if (uiNumBytesRead != sizeof(KEY) * NUM_KEYS) {
    return (FALSE);
  }

  return (TRUE);
}

void ExamineDoorsOnEnteringSector() {
  int32_t cnt;
  DOOR_STATUS *pDoorStatus;
  struct SOLDIERTYPE *pSoldier;
  BOOLEAN fOK = FALSE;
  TownID bTownId;

  // OK, only do this if conditions are met....
  // If this is any omerta tow, don't do it...
  bTownId = GetTownIdForSector((uint8_t)gWorldSectorX, (uint8_t)gWorldSectorY);

  if (bTownId == OMERTA) {
    return;
  }

  // Check time...
  if ((GetWorldTotalMin() - gTacticalStatus.uiTimeSinceLastInTactical) < 30) {
    return;
  }

  // there is at least one human being in that sector.
  // check for civ
  cnt = gTacticalStatus.Team[ENEMY_TEAM].bFirstID;
  // look for all mercs on the same team,
  for (pSoldier = MercPtrs[cnt]; cnt <= gTacticalStatus.Team[LAST_TEAM].bLastID;
       cnt++, pSoldier++) {
    if (IsSolActive(pSoldier)) {
      if (IsSolInSector(pSoldier)) {
        fOK = TRUE;
        break;
      }
    }
  }

  // Let's do it!
  if (fOK) {
    for (cnt = 0; cnt < gubNumDoorStatus; cnt++) {
      pDoorStatus = &(gpDoorStatus[cnt]);

      // Get status of door....
      if (pDoorStatus->ubFlags & DOOR_OPEN) {
        // If open, close!
        HandleDoorChangeFromGridNo(NULL, pDoorStatus->sGridNo, TRUE);
      }
    }
  }
}

void HandleDoorsChangeWhenEnteringSectorCurrentlyLoaded() {
  int32_t cnt;
  DOOR_STATUS *pDoorStatus;
  struct SOLDIERTYPE *pSoldier;
  BOOLEAN fOK = FALSE;
  int32_t iNumNewMercs = 0;
  TownID bTownId;

  // OK, only do this if conditions are met....

  // If this is any omerta tow, don't do it...
  bTownId = GetTownIdForSector((uint8_t)gWorldSectorX, (uint8_t)gWorldSectorY);

  if (bTownId == OMERTA) {
    return;
  }

  // 1 ) there is at least one human being in that sector.
  // check for civ
  cnt = gTacticalStatus.Team[ENEMY_TEAM].bFirstID;

  // Check time...
  if ((GetWorldTotalMin() - gTacticalStatus.uiTimeSinceLastInTactical) < 30) {
    return;
  }

  // look for all mercs on the same team,
  for (pSoldier = MercPtrs[cnt]; cnt <= gTacticalStatus.Team[LAST_TEAM].bLastID;
       cnt++, pSoldier++) {
    if (IsSolActive(pSoldier) && pSoldier->bInSector) {
      fOK = TRUE;
      break;
    }
  }

  // Loop through our team now....
  cnt = gTacticalStatus.Team[gbPlayerNum].bFirstID;
  for (pSoldier = MercPtrs[cnt]; cnt <= gTacticalStatus.Team[gbPlayerNum].bLastID;
       cnt++, pSoldier++) {
    if (IsSolActive(pSoldier) && pSoldier->bInSector && gbMercIsNewInThisSector[cnt]) {
      iNumNewMercs++;
    }
  }

  // ATE: Only do for newly added mercs....
  if (iNumNewMercs == 0) {
    return;
  }

  // Let's do it!
  if (fOK) {
    for (cnt = 0; cnt < gubNumDoorStatus; cnt++) {
      pDoorStatus = &(gpDoorStatus[cnt]);

      // Get status of door....
      if (pDoorStatus->ubFlags & DOOR_OPEN) {
        // If open, close!
        gfSetPerceivedDoorState = TRUE;

        HandleDoorChangeFromGridNo(NULL, pDoorStatus->sGridNo, TRUE);

        gfSetPerceivedDoorState = FALSE;

        AllMercsLookForDoor(pDoorStatus->sGridNo, TRUE);

        InternalUpdateDoorGraphicFromStatus(pDoorStatus, TRUE, TRUE);
      }
    }
  }
}

void DropKeysInKeyRing(struct SOLDIERTYPE *pSoldier, int16_t sGridNo, int8_t bLevel,
                       int8_t bVisible, BOOLEAN fAddToDropList, int32_t iDropListSlot,
                       BOOLEAN fUseUnLoaded) {
  uint8_t ubLoop;
  uint8_t ubItem;
  struct OBJECTTYPE Object;

  if (!(pSoldier->pKeyRing)) {
    // no key ring!
    return;
  }
  for (ubLoop = 0; ubLoop < NUM_KEYS; ubLoop++) {
    ubItem = pSoldier->pKeyRing[ubLoop].ubKeyID;

    if (pSoldier->pKeyRing[ubLoop].ubNumber > 0) {
      CreateKeyObject(&Object, pSoldier->pKeyRing[ubLoop].ubNumber, ubItem);

      // Zero out entry
      pSoldier->pKeyRing[ubLoop].ubNumber = 0;
      pSoldier->pKeyRing[ubLoop].ubKeyID = INVALID_KEY_NUMBER;

      if (fAddToDropList) {
        AddItemToLeaveIndex(&Object, iDropListSlot);
      } else {
        if (GetSolSectorX(pSoldier) != gWorldSectorX || GetSolSectorY(pSoldier) != gWorldSectorY ||
            GetSolSectorZ(pSoldier) != gbWorldSectorZ || fUseUnLoaded) {
          // Set flag for item...
          AddItemsToUnLoadedSector(GetSolSectorX(pSoldier), GetSolSectorY(pSoldier),
                                   pSoldier->bSectorZ, sGridNo, 1, &Object, bLevel,
                                   WOLRD_ITEM_FIND_SWEETSPOT_FROM_GRIDNO | WORLD_ITEM_REACHABLE, 0,
                                   bVisible, FALSE);
        } else {
          // Add to pool
          AddItemToPool(sGridNo, &Object, bVisible, bLevel, 0, 0);
        }
      }
    }
  }
}
