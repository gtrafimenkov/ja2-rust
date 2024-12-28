// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include "TacticalAI/NPC.h"

#include "Laptop/Finances.h"
#include "SGP/Debug.h"
#include "SGP/Random.h"
#include "SGP/Types.h"
#include "SGP/WCheck.h"
#include "Soldier.h"
#include "Strategic/Assignments.h"
#include "Strategic/CampaignTypes.h"
#include "Strategic/GameClock.h"
#include "Strategic/Meanwhile.h"
#include "Strategic/Quests.h"
#include "Strategic/Scheduling.h"
#include "Strategic/StrategicMap.h"
#include "Strategic/StrategicTownLoyalty.h"
#include "Tactical/AnimationControl.h"
#include "Tactical/ArmsDealerInit.h"
#include "Tactical/DialogueControl.h"
#include "Tactical/Interface.h"
#include "Tactical/InterfaceDialogue.h"
#include "Tactical/InterfaceItems.h"
#include "Tactical/Items.h"
#include "Tactical/OppList.h"
#include "Tactical/Overhead.h"
#include "Tactical/SkillCheck.h"
#include "Tactical/SoldierAdd.h"
#include "Tactical/SoldierControl.h"
#include "Tactical/SoldierMacros.h"
#include "Tactical/SoldierProfile.h"
#include "Tactical/SoldierTile.h"
#include "Tactical/TacticalSave.h"
#include "Tactical/Weapons.h"
#include "TacticalAI/AI.h"
#include "TacticalAI/QuestDebug.h"
#include "TileEngine/IsometricUtils.h"
#include "TileEngine/RenderFun.h"
#include "TileEngine/WorldMan.h"
#include "Utils/Message.h"
#include "Utils/Text.h"
#include "Utils/TimerControl.h"
#include "rust_civ_groups.h"
#include "rust_fileman.h"

#ifdef JA2TESTVERSION
#include "Strategic/QuestDebugSystem.h"
#include "Strategic/QuestText.h"
#endif

#define NUM_CIVQUOTE_SECTORS 20
#define MINERS_CIV_QUOTE_INDEX 16

int16_t gsCivQuoteSector[NUM_CIVQUOTE_SECTORS][2] = {
    {2, MAP_ROW_A},  {2, MAP_ROW_B},  {13, MAP_ROW_B}, {13, MAP_ROW_C}, {13, MAP_ROW_D},
    {8, MAP_ROW_F},  {9, MAP_ROW_F},  {8, MAP_ROW_G},  {9, MAP_ROW_G},  {1, MAP_ROW_H},

    {2, MAP_ROW_H},  {3, MAP_ROW_H},  {8, MAP_ROW_H},  {13, MAP_ROW_H}, {14, MAP_ROW_I},
    {11, MAP_ROW_L}, {12, MAP_ROW_L}, {0, 0},  // THIS ONE USED NOW - FOR bSectorZ > 0.....
    {0, 0},          {0, 0},
};

#define NO_FACT (MAX_FACTS - 1)
#define NO_QUEST 255
#define QUEST_NOT_STARTED_NUM 100
#define QUEST_DONE_NUM 200
#define NO_QUOTE 255
#define IRRELEVANT 255
#define NO_MOVE 65535

NPCQuoteInfo *gpNPCQuoteInfoArray[NUM_PROFILES] = {NULL};
NPCQuoteInfo *gpBackupNPCQuoteInfoArray[NUM_PROFILES] = {NULL};
NPCQuoteInfo *gpCivQuoteInfoArray[NUM_CIVQUOTE_SECTORS] = {NULL};

#ifdef JA2TESTVERSION
// Warning: cheap hack approaching
BOOLEAN gfTriedToLoadQuoteInfoArray[NUM_PROFILES] = {FALSE};
#endif

uint8_t gubTeamPenalty;

BOOLEAN EnsureQuoteFileLoaded(uint8_t ubNPC);
NPCQuoteInfo *LoadQuoteFile(uint8_t ubNPC);
uint8_t NPCConsiderQuote(uint8_t ubNPC, uint8_t ubMerc, uint8_t ubApproach, uint8_t ubQuoteNum,
                         uint8_t ubTalkDesire, NPCQuoteInfo *pNPCQuoteInfoArray);
uint8_t NPCConsiderReceivingItemFromMerc(uint8_t ubNPC, uint8_t ubMerc, struct OBJECTTYPE *pObj,
                                         NPCQuoteInfo *pNPCQuoteInfoArray,
                                         NPCQuoteInfo **ppResultQuoteInfo, uint8_t *pubQuoteNum);
void PCsNearNPC(uint8_t ubNPC);
void TriggerClosestMercWhoCanSeeNPC(uint8_t ubNPC, NPCQuoteInfo *pQuotePtr);
BOOLEAN NPCHasUnusedRecordWithGivenApproach(uint8_t ubNPC, uint8_t ubApproach);

int8_t gbFirstApproachFlags[4] = {0x01, 0x02, 0x04, 0x08};

uint8_t gubAlternateNPCFileNumsForQueenMeanwhiles[] = {160, 161, 162, 163, 164, 165, 166, 167, 168,
                                                       169, 170, 171, 172, 173, 174, 175, 176};
uint8_t gubAlternateNPCFileNumsForElliotMeanwhiles[] = {180, 181, 182, 183, 184, 185, 186, 187, 188,
                                                        189, 190, 191, 192, 193, 194, 195, 196};

#ifdef JA2BETAVERSION
BOOLEAN gfDisplayScreenMsgOnRecordUsage = FALSE;
#endif

extern void PauseAITemporarily(void);
extern void PayOffSkyriderDebtIfAny();
//
// NPC QUOTE LOW LEVEL ROUTINES
//

NPCQuoteInfo *LoadQuoteFile(uint8_t ubNPC) {
  char zFileName[255];
  FileID hFile = FILE_ID_ERR;
  NPCQuoteInfo *pFileData;
  uint32_t uiBytesRead;
  uint32_t uiFileSize;

  if (ubNPC == PETER || ubNPC == ALBERTO || ubNPC == CARLO) {
    // use a copy of Herve's data file instead!
    sprintf(zFileName, "NPCData\\%03d.npc", HERVE);
  } else if (ubNPC < FIRST_RPC || (ubNPC < FIRST_NPC && gMercProfiles[ubNPC].ubMiscFlags &
                                                            PROFILE_MISC_FLAG_RECRUITED)) {
    sprintf(zFileName, "%s", "NPCData\\000.npc");
  } else {
    sprintf(zFileName, "NPCData\\%03d.npc", ubNPC);
  }

  // ATE: Put some stuff i here to use a different NPC file if we are in a meanwhile.....
  if (AreInMeanwhile()) {
    // If we are the queen....
    if (ubNPC == QUEEN) {
      sprintf(zFileName, "NPCData\\%03d.npc",
              gubAlternateNPCFileNumsForQueenMeanwhiles[GetMeanwhileID()]);
    }

    // If we are elliot....
    if (ubNPC == ELLIOT) {
      sprintf(zFileName, "NPCData\\%03d.npc",
              gubAlternateNPCFileNumsForElliotMeanwhiles[GetMeanwhileID()]);
    }
  }

  CHECKN(File_Exists(zFileName));

  hFile = File_OpenForReading(zFileName);
  CHECKN(hFile);

  uiFileSize = sizeof(NPCQuoteInfo) * NUM_NPC_QUOTE_RECORDS;
  pFileData = (NPCQuoteInfo *)MemAlloc(uiFileSize);
  if (pFileData) {
    if (!File_Read(hFile, pFileData, uiFileSize, &uiBytesRead) || uiBytesRead != uiFileSize) {
      MemFree(pFileData);
      pFileData = NULL;
    }
  }

  File_Close(hFile);

  return (pFileData);
}

void RevertToOriginalQuoteFile(uint8_t ubNPC) {
  if (gpBackupNPCQuoteInfoArray[ubNPC] && gpNPCQuoteInfoArray[ubNPC]) {
    MemFree(gpNPCQuoteInfoArray[ubNPC]);
    gpNPCQuoteInfoArray[ubNPC] = gpBackupNPCQuoteInfoArray[ubNPC];
    gpBackupNPCQuoteInfoArray[ubNPC] = NULL;
  }
}

void BackupOriginalQuoteFile(uint8_t ubNPC) {
  gpBackupNPCQuoteInfoArray[ubNPC] = gpNPCQuoteInfoArray[ubNPC];
  gpNPCQuoteInfoArray[ubNPC] = NULL;
}

BOOLEAN EnsureQuoteFileLoaded(uint8_t ubNPC) {
  BOOLEAN fLoadFile = FALSE;

  if (ubNPC == ROBOT) {
    return (FALSE);
  }

  if (gpNPCQuoteInfoArray[ubNPC] == NULL) {
    fLoadFile = TRUE;
  }

  if (ubNPC >= FIRST_RPC && ubNPC < FIRST_NPC) {
    if (gMercProfiles[ubNPC].ubMiscFlags & PROFILE_MISC_FLAG_RECRUITED) {
      // recruited
      if (gpBackupNPCQuoteInfoArray[ubNPC] == NULL) {
        // no backup stored of current script, so need to backup
        fLoadFile = TRUE;
        // set pointer to back up script!
        BackupOriginalQuoteFile(ubNPC);
      }
      // else have backup, are recruited, nothing special
    } else {
      // not recruited
      if (gpBackupNPCQuoteInfoArray[ubNPC] != NULL) {
        // backup stored, restore backup
        RevertToOriginalQuoteFile(ubNPC);
      }
      // else are no backup, nothing special
    }
  }

  if (fLoadFile) {
    gpNPCQuoteInfoArray[ubNPC] = LoadQuoteFile(ubNPC);
    if (gpNPCQuoteInfoArray[ubNPC] == NULL) {
#ifdef JA2TESTVERSION
      if (!gfTriedToLoadQuoteInfoArray[ubNPC])  // don't report the error a second time
      {
        ScreenMsg(MSG_FONT_RED, MSG_DEBUG, L"ERROR: NPC.C - NPC needs NPC file: %d.", ubNPC);
        gfTriedToLoadQuoteInfoArray[ubNPC] = TRUE;
      }
#endif
      // error message at this point!
      return (FALSE);
    }
  }

  return (TRUE);
}

BOOLEAN ReloadQuoteFile(uint8_t ubNPC) {
  if (gpNPCQuoteInfoArray[ubNPC] != NULL) {
    MemFree(gpNPCQuoteInfoArray[ubNPC]);
    gpNPCQuoteInfoArray[ubNPC] = NULL;
  }
  // zap backup if any
  if (gpBackupNPCQuoteInfoArray[ubNPC] != NULL) {
    MemFree(gpBackupNPCQuoteInfoArray[ubNPC]);
    gpBackupNPCQuoteInfoArray[ubNPC] = NULL;
  }
  return (EnsureQuoteFileLoaded(ubNPC));
}

BOOLEAN ReloadQuoteFileIfLoaded(uint8_t ubNPC) {
  if (gpNPCQuoteInfoArray[ubNPC] != NULL) {
    MemFree(gpNPCQuoteInfoArray[ubNPC]);
    gpNPCQuoteInfoArray[ubNPC] = NULL;
    return (EnsureQuoteFileLoaded(ubNPC));
  } else {
    return (TRUE);
  }
}

BOOLEAN RefreshNPCScriptRecord(uint8_t ubNPC, uint8_t ubRecord) {
  uint8_t ubLoop;
  NPCQuoteInfo *pNewArray;

  if (ubNPC == NO_PROFILE) {
    // we have some work to do...
    // loop through all PCs, and refresh their copy of this record
    for (ubLoop = 0; ubLoop < FIRST_RPC; ubLoop++)  // need more finesse here
    {
      RefreshNPCScriptRecord(ubLoop, ubRecord);
    }
    for (ubLoop = FIRST_RPC; ubLoop < FIRST_NPC; ubLoop++) {
      if (gMercProfiles[ubNPC].ubMiscFlags & PROFILE_MISC_FLAG_RECRUITED &&
          gpBackupNPCQuoteInfoArray[ubNPC] != NULL) {
        RefreshNPCScriptRecord(ubLoop, ubRecord);
      }
    }
    return (TRUE);
  }

  if (gpNPCQuoteInfoArray[ubNPC]) {
    if (CHECK_FLAG(gpNPCQuoteInfoArray[ubNPC][ubRecord].fFlags, QUOTE_FLAG_SAID)) {
      // already used so we don't have to refresh!
      return (TRUE);
    }

    pNewArray = LoadQuoteFile(ubNPC);
    if (pNewArray) {
      memcpy(&(gpNPCQuoteInfoArray[ubNPC][ubRecord]), &(pNewArray[ubRecord]), sizeof(NPCQuoteInfo));
      MemFree(pNewArray);
    }
  }
  return (TRUE);
}

//
// CIV QUOTE LOW LEVEL ROUTINES
//

NPCQuoteInfo *LoadCivQuoteFile(uint8_t ubIndex) {
  char zFileName[255];
  FileID hFile = FILE_ID_ERR;
  NPCQuoteInfo *pFileData;
  uint32_t uiBytesRead;
  uint32_t uiFileSize;

  if (ubIndex == MINERS_CIV_QUOTE_INDEX) {
    sprintf(zFileName, "NPCData\\miners.npc");
  } else {
    sprintf(zFileName, "NPCData\\%c%d.npc", 'A' + (gsCivQuoteSector[ubIndex][1] - 1),
            gsCivQuoteSector[ubIndex][0]);
  }

  CHECKN(File_Exists(zFileName));

  hFile = File_OpenForReading(zFileName);
  CHECKN(hFile);

  uiFileSize = sizeof(NPCQuoteInfo) * NUM_NPC_QUOTE_RECORDS;
  pFileData = (NPCQuoteInfo *)MemAlloc(uiFileSize);
  if (pFileData) {
    if (!File_Read(hFile, pFileData, uiFileSize, &uiBytesRead) || uiBytesRead != uiFileSize) {
      MemFree(pFileData);
      pFileData = NULL;
    }
  }

  File_Close(hFile);

  return (pFileData);
}

BOOLEAN EnsureCivQuoteFileLoaded(uint8_t ubIndex) {
  BOOLEAN fLoadFile = FALSE;

  if (gpCivQuoteInfoArray[ubIndex] == NULL) {
    fLoadFile = TRUE;
  }

  if (fLoadFile) {
    gpCivQuoteInfoArray[ubIndex] = LoadCivQuoteFile(ubIndex);
    if (gpCivQuoteInfoArray[ubIndex] == NULL) {
      return (FALSE);
    }
  }

  return (TRUE);
}

BOOLEAN ReloadCivQuoteFile(uint8_t ubIndex) {
  if (gpCivQuoteInfoArray[ubIndex] != NULL) {
    MemFree(gpCivQuoteInfoArray[ubIndex]);
    gpCivQuoteInfoArray[ubIndex] = NULL;
  }
  return (EnsureCivQuoteFileLoaded(ubIndex));
}

BOOLEAN ReloadCivQuoteFileIfLoaded(uint8_t ubIndex) {
  if (gpCivQuoteInfoArray[ubIndex] != NULL) {
    MemFree(gpCivQuoteInfoArray[ubIndex]);
    gpCivQuoteInfoArray[ubIndex] = NULL;
    return (EnsureCivQuoteFileLoaded(ubIndex));
  } else {
    return (TRUE);
  }
}

void ShutdownNPCQuotes(void) {
  uint8_t ubLoop;

  for (ubLoop = 0; ubLoop < NUM_PROFILES; ubLoop++) {
    if (gpNPCQuoteInfoArray[ubLoop]) {
      MemFree(gpNPCQuoteInfoArray[ubLoop]);
      gpNPCQuoteInfoArray[ubLoop] = NULL;
    }

    if (gpBackupNPCQuoteInfoArray[ubLoop] != NULL) {
      MemFree(gpBackupNPCQuoteInfoArray[ubLoop]);
      gpBackupNPCQuoteInfoArray[ubLoop] = NULL;
    }
  }

  for (ubLoop = 0; ubLoop < NUM_CIVQUOTE_SECTORS; ubLoop++) {
    if (gpCivQuoteInfoArray[ubLoop]) {
      MemFree(gpCivQuoteInfoArray[ubLoop]);
      gpCivQuoteInfoArray[ubLoop] = NULL;
    }
  }
}

//
// GENERAL LOW LEVEL ROUTINES
//

BOOLEAN ReloadAllQuoteFiles(void) {
  uint8_t ubProfile, ubLoop;

  for (ubProfile = FIRST_RPC; ubProfile < NUM_PROFILES; ubProfile++) {
    // zap backup if any
    if (gpBackupNPCQuoteInfoArray[ubProfile] != NULL) {
      MemFree(gpBackupNPCQuoteInfoArray[ubProfile]);
      gpBackupNPCQuoteInfoArray[ubProfile] = NULL;
    }
    ReloadQuoteFileIfLoaded(ubProfile);
  }
  // reload all civ quote files
  for (ubLoop = 0; ubLoop < NUM_CIVQUOTE_SECTORS; ubLoop++) {
    ReloadCivQuoteFileIfLoaded(ubLoop);
  }

  return (TRUE);
}

//
// THE REST
//

void SetQuoteRecordAsUsed(uint8_t ubNPC, uint8_t ubRecord) {
  if (EnsureQuoteFileLoaded(ubNPC)) {
    gpNPCQuoteInfoArray[ubNPC][ubRecord].fFlags |= QUOTE_FLAG_SAID;
  }
}

int32_t CalcThreateningEffectiveness(uint8_t ubMerc) {
  struct SOLDIERTYPE *pSoldier;
  int32_t iStrength, iDeadliness;

  // effective threat is 1/3 strength, 1/3 weapon deadliness, 1/3 leadership

  pSoldier = FindSoldierByProfileID(ubMerc, TRUE);

  if (!pSoldier) {
    return (0);
  }

  iStrength = EffectiveStrength(pSoldier);

  if (Item[pSoldier->inv[HANDPOS].usItem].usItemClass & IC_WEAPON) {
    iDeadliness = Weapon[pSoldier->inv[HANDPOS].usItem].ubDeadliness;
  } else {
    iDeadliness = 0;
  }

  if (iDeadliness == 0) {
    // penalize!
    iDeadliness = -30;
  }

  return ((EffectiveLeadership(pSoldier) + iStrength + iDeadliness) / 2);
}

uint8_t CalcDesireToTalk(uint8_t ubNPC, uint8_t ubMerc, int8_t bApproach) {
  int32_t iWillingness;
  int32_t iPersonalVal, iTownVal, iApproachVal;
  int32_t iEffectiveLeadership;
  MERCPROFILESTRUCT *pNPCProfile;
  MERCPROFILESTRUCT *pMercProfile;

  pNPCProfile = &(gMercProfiles[ubNPC]);
  pMercProfile = &(gMercProfiles[ubMerc]);

  iPersonalVal =
      50 +
      pNPCProfile
          ->bMercOpinion[ubMerc]; /* + pNPCProfile->bMercTownReputation[ pNPCProfile->bTown ] */
  ;

  // ARM: NOTE - for towns which don't use loyalty (San Mona, Estoni, Tixa, Orta )
  // loyalty will always remain 0 (this was OKed by Ian)
  iTownVal = GetTownLoyaltyRating(pNPCProfile->bTown);
  iTownVal = iTownVal * pNPCProfile->bTownAttachment / 100;

  if (bApproach == NPC_INITIATING_CONV || bApproach == APPROACH_GIVINGITEM) {
    iApproachVal = 100;
  } else if (bApproach == APPROACH_THREATEN) {
    iEffectiveLeadership =
        CalcThreateningEffectiveness(ubMerc) * pMercProfile->usApproachFactor[bApproach - 1] / 100;
    iApproachVal = pNPCProfile->ubApproachVal[bApproach - 1] * iEffectiveLeadership / 50;
  } else {
    iEffectiveLeadership =
        ((int32_t)pMercProfile->bLeadership) * pMercProfile->usApproachFactor[bApproach - 1] / 100;
    iApproachVal = pNPCProfile->ubApproachVal[bApproach - 1] * iEffectiveLeadership / 50;
  }
  // NB if town attachment is less than 100% then we should make personal value proportionately more
  // important!
  if (pNPCProfile->bTownAttachment < 100) {
    iPersonalVal = iPersonalVal * (100 + (100 - pNPCProfile->bTownAttachment)) / 100;
  }
  iWillingness = (iPersonalVal / 2 + iTownVal / 2) * iApproachVal / 100 - gubTeamPenalty;

  if (bApproach == NPC_INITIATING_CONV) {
    iWillingness -= INITIATING_FACTOR;
  }

  if (iWillingness < 0) {
    iWillingness = 0;
  }

  return ((uint8_t)iWillingness);
}

void ApproachedForFirstTime(MERCPROFILESTRUCT *pNPCProfile, int8_t bApproach) {
  uint8_t ubLoop;
  uint32_t uiTemp;

  pNPCProfile->bApproached |= gbFirstApproachFlags[bApproach - 1];
  for (ubLoop = 1; ubLoop <= NUM_REAL_APPROACHES; ubLoop++) {
    uiTemp = (uint32_t)pNPCProfile->ubApproachVal[ubLoop - 1] *
             (uint32_t)pNPCProfile->ubApproachMod[bApproach - 1][ubLoop - 1] / 100;
    if (uiTemp > 255) {
      uiTemp = 255;
    }
    pNPCProfile->ubApproachVal[ubLoop - 1] = (uint8_t)uiTemp;
  }
}

uint8_t NPCConsiderTalking(uint8_t ubNPC, uint8_t ubMerc, int8_t bApproach, uint8_t ubRecord,
                           NPCQuoteInfo *pNPCQuoteInfoArray, NPCQuoteInfo **ppResultQuoteInfo,
                           uint8_t *pubQuoteNum) {
  // This function returns the opinion level required of the "most difficult" quote
  // that the NPC is willing to say to the merc.  It can also provide the quote #.
  MERCPROFILESTRUCT *pNPCProfile = NULL;
  NPCQuoteInfo *pNPCQuoteInfo = NULL;
  uint8_t ubTalkDesire, ubLoop, ubQuote, ubHighestOpinionRequired = 0;
  BOOLEAN fQuoteFound = FALSE;
  uint8_t ubFirstQuoteRecord, ubLastQuoteRecord;
  struct SOLDIERTYPE *pSoldier = NULL;

  ubTalkDesire = ubQuote = 0;

  pSoldier = FindSoldierByProfileID(ubNPC, FALSE);
  if (pSoldier == NULL) {
    return (0);
  }

  if (ppResultQuoteInfo) {
    (*ppResultQuoteInfo) = NULL;
  }

  if (pubQuoteNum) {
    (*pubQuoteNum) = 0;
  }

  if (bApproach <= NUM_REAL_APPROACHES) {
    pNPCProfile = &(gMercProfiles[ubNPC]);
    // What's our willingness to divulge?
    ubTalkDesire = CalcDesireToTalk(ubNPC, ubMerc, bApproach);
    if (bApproach < NUM_REAL_APPROACHES &&
        !(pNPCProfile->bApproached & gbFirstApproachFlags[bApproach - 1])) {
      ApproachedForFirstTime(pNPCProfile, bApproach);
    }
  } else if (ubNPC == PABLO &&
             bApproach == APPROACH_SECTOR_NOT_SAFE)  // for Pablo, consider as threaten
  {
    pNPCProfile = &(gMercProfiles[ubNPC]);
    // What's our willingness to divulge?
    ubTalkDesire = CalcDesireToTalk(ubNPC, ubMerc, APPROACH_THREATEN);
    if (pNPCProfile->bApproached & gbFirstApproachFlags[APPROACH_THREATEN - 1]) {
      ApproachedForFirstTime(pNPCProfile, APPROACH_THREATEN);
    }
  }

  switch (bApproach) {
      /*
              case APPROACH_RECRUIT:
                      ubFirstQuoteRecord = 0;
                      ubLastQuoteRecord = 0;
                      break;
                      */
    case TRIGGER_NPC:
      ubFirstQuoteRecord = ubRecord;
      ubLastQuoteRecord = ubRecord;
      break;
    default:
      ubFirstQuoteRecord = 0;
      ubLastQuoteRecord = NUM_NPC_QUOTE_RECORDS - 1;
      break;
  }

  for (ubLoop = ubFirstQuoteRecord; ubLoop <= ubLastQuoteRecord; ubLoop++) {
    pNPCQuoteInfo = &(pNPCQuoteInfoArray[ubLoop]);

    // Check if we have the item / are in right spot
    if (pNPCQuoteInfo->sRequiredItem > 0) {
      if (!ObjectExistsInSoldierProfile(ubNPC, pNPCQuoteInfo->sRequiredItem)) {
        continue;
      }
    } else if (pNPCQuoteInfo->sRequiredGridno < 0) {
      if (pSoldier->sGridNo != -(pNPCQuoteInfo->sRequiredGridno)) {
        continue;
      }
    }

    if (NPCConsiderQuote(ubNPC, ubMerc, bApproach, ubLoop, ubTalkDesire, pNPCQuoteInfoArray)) {
      if (bApproach == NPC_INITIATING_CONV) {
        // want to find the quote with the highest required opinion rating that we're willing
        // to say
        if (pNPCQuoteInfo->ubOpinionRequired > ubHighestOpinionRequired) {
          fQuoteFound = TRUE;
          ubHighestOpinionRequired = pNPCQuoteInfo->ubOpinionRequired;
          ubQuote = pNPCQuoteInfo->ubQuoteNum;
        }
      } else {
        // we do have a quote to say, and we want to say this one right away!
        if (ppResultQuoteInfo) {
          (*ppResultQuoteInfo) = pNPCQuoteInfo;
        }
        if (pubQuoteNum) {
          (*pubQuoteNum) = ubLoop;
        }

        return (pNPCQuoteInfo->ubOpinionRequired);
      }
    }
  }

  // Whew, checked them all.  If we found a quote, return the appropriate values.
  if (fQuoteFound) {
    if (ppResultQuoteInfo) {
      (*ppResultQuoteInfo) = pNPCQuoteInfo;
    }
    if (pubQuoteNum) {
      (*pubQuoteNum) = ubQuote;
    }

    return (ubHighestOpinionRequired);
  } else {
    if (ppResultQuoteInfo) {
      (*ppResultQuoteInfo) = NULL;
    }
    if (pubQuoteNum) {
      (*pubQuoteNum) = 0;
    }
    return (0);
  }
}

uint8_t NPCConsiderReceivingItemFromMerc(uint8_t ubNPC, uint8_t ubMerc, struct OBJECTTYPE *pObj,
                                         NPCQuoteInfo *pNPCQuoteInfoArray,
                                         NPCQuoteInfo **ppResultQuoteInfo, uint8_t *pubQuoteNum) {
  // This function returns the opinion level required of the "most difficult" quote
  // that the NPC is willing to say to the merc.  It can also provide the quote #.
  NPCQuoteInfo *pNPCQuoteInfo;
  uint8_t ubTalkDesire, ubLoop;
  uint8_t ubFirstQuoteRecord, ubLastQuoteRecord;
  uint16_t usItemToConsider;

  (*ppResultQuoteInfo) = NULL;
  (*pubQuoteNum) = 0;

  if (CheckFact(FACT_NPC_IS_ENEMY, ubNPC) && ubNPC != JOE) {
    // don't accept any items when we are the player's enemy
    return (0);
  }

  // How much do we want to talk with this merc?

  ubTalkDesire = CalcDesireToTalk(ubNPC, ubMerc, APPROACH_GIVINGITEM);

  ubFirstQuoteRecord = 0;
  ubLastQuoteRecord = NUM_NPC_QUOTE_RECORDS - 1;

  usItemToConsider = pObj->usItem;
  if (Item[usItemToConsider].usItemClass == IC_GUN && usItemToConsider != ROCKET_LAUNCHER) {
    uint8_t ubWeaponClass;

    ubWeaponClass = Weapon[usItemToConsider].ubWeaponClass;
    if (ubWeaponClass == RIFLECLASS || ubWeaponClass == MGCLASS) {
      usItemToConsider = ANY_RIFLE;  // treat all rifles the same
    }
  }
  switch (usItemToConsider) {
    case HEAD_2:
    case HEAD_3:
    // case HEAD_4: // NOT Slay's head; it's different
    case HEAD_5:
    case HEAD_6:
    case HEAD_7:
      // all treated the same in the NPC code
      usItemToConsider = HEAD_2;
      break;
    case MONEY:
    case SILVER:
    case GOLD:
      if (pObj->uiMoneyAmount < LARGE_AMOUNT_MONEY) {
        SetFactTrue(FACT_SMALL_AMOUNT_OF_MONEY);
      } else {
        SetFactTrue(FACT_LARGE_AMOUNT_OF_MONEY);
      }
      usItemToConsider = MONEY;
      break;
    case WINE:
    case BEER:
      usItemToConsider = ALCOHOL;
      break;
    default:
      break;
  }

  if (pObj->bStatus[0] < 50) {
    SetFactTrue(FACT_ITEM_POOR_CONDITION);
  } else {
    SetFactFalse(FACT_ITEM_POOR_CONDITION);
  }

  for (ubLoop = ubFirstQuoteRecord; ubLoop <= ubLastQuoteRecord; ubLoop++) {
    pNPCQuoteInfo = &(pNPCQuoteInfoArray[ubLoop]);

    // First see if we want that item....
    if (pNPCQuoteInfo->sRequiredItem > 0 && (pNPCQuoteInfo->sRequiredItem == usItemToConsider ||
                                             pNPCQuoteInfo->sRequiredItem == ACCEPT_ANY_ITEM)) {
      // Now see if everyhting else is OK
      if (NPCConsiderQuote(ubNPC, ubMerc, APPROACH_GIVINGITEM, ubLoop, ubTalkDesire,
                           pNPCQuoteInfoArray)) {
        switch (ubNPC) {
          case DARREN:
            if (usItemToConsider == MONEY &&
                pNPCQuoteInfo->sActionData == NPC_ACTION_DARREN_GIVEN_CASH) {
              if (pObj->uiMoneyAmount < 1000) {
                // refuse, bet too low - record 15
                (*ppResultQuoteInfo) = &pNPCQuoteInfoArray[15];
                (*pubQuoteNum) = 15;
                return ((*ppResultQuoteInfo)->ubOpinionRequired);
              } else if (pObj->uiMoneyAmount > 5000) {
                // refuse, bet too high - record 16
                (*ppResultQuoteInfo) = &pNPCQuoteInfoArray[16];
                (*pubQuoteNum) = 16;
                return ((*ppResultQuoteInfo)->ubOpinionRequired);
              } else {
                // accept - record 17
                /*
                {

                        struct SOLDIERTYPE *					pSoldier;
                        int8_t bMoney; int8_t
                bEmptySlot;

                        pSoldier = FindSoldierByProfileID( DARREN, FALSE );
                        bMoney = FindObjWithin( pSoldier, MONEY, BIGPOCK1POS, SMALLPOCK8POS );
                        bEmptySlot = FindObjWithin( pSoldier, NOTHING, BIGPOCK1POS, SMALLPOCK8POS );
                }
                */

                // record amount of bet
                gMercProfiles[DARREN].iBalance = pObj->uiMoneyAmount;
                SetFactFalse(FACT_DARREN_EXPECTING_MONEY);

                // if never fought before, use record 17
                // if fought before, today, use record 31
                // else use record 18
                if (!(gpNPCQuoteInfoArray[DARREN][17].fFlags &
                      QUOTE_FLAG_SAID))  // record 17 not used
                {
                  (*ppResultQuoteInfo) = &pNPCQuoteInfoArray[17];
                  (*pubQuoteNum) = 17;
                } else {
                  // find Kingpin, if he's in his house, invoke the script to move him to the bar
                  struct SOLDIERTYPE *pKingpin;
                  uint8_t ubKingpinRoom;

                  pKingpin = FindSoldierByProfileID(KINGPIN, FALSE);
                  if (pKingpin && InARoom(pKingpin->sGridNo, &ubKingpinRoom)) {
                    if (IN_KINGPIN_HOUSE(ubKingpinRoom)) {
                      // first boxer, bring kingpin over
                      (*ppResultQuoteInfo) = &pNPCQuoteInfoArray[17];
                      (*pubQuoteNum) = 17;
                    } else {
                      (*ppResultQuoteInfo) = &pNPCQuoteInfoArray[31];
                      (*pubQuoteNum) = 31;
                    }
                  } else {
                    (*ppResultQuoteInfo) = &pNPCQuoteInfoArray[31];
                    (*pubQuoteNum) = 31;
                  }
                }

                return ((*ppResultQuoteInfo)->ubOpinionRequired);
              }
            }
            break;
          case ANGEL:
            if (usItemToConsider == MONEY &&
                pNPCQuoteInfo->sActionData == NPC_ACTION_ANGEL_GIVEN_CASH) {
              if (pObj->uiMoneyAmount < Item[LEATHER_JACKET_W_KEVLAR].usPrice) {
                // refuse, bet too low - record 8
                (*ppResultQuoteInfo) = &pNPCQuoteInfoArray[8];
                (*pubQuoteNum) = 8;
                return ((*ppResultQuoteInfo)->ubOpinionRequired);
              } else if (pObj->uiMoneyAmount > Item[LEATHER_JACKET_W_KEVLAR].usPrice) {
                // refuse, bet too high - record 9
                (*ppResultQuoteInfo) = &pNPCQuoteInfoArray[9];
                (*pubQuoteNum) = 9;
                return ((*ppResultQuoteInfo)->ubOpinionRequired);
              } else {
                // accept - record 10
                (*ppResultQuoteInfo) = &pNPCQuoteInfoArray[10];
                (*pubQuoteNum) = 10;
                return ((*ppResultQuoteInfo)->ubOpinionRequired);
              }
            }
            break;
          case MADAME:
            if (usItemToConsider == MONEY) {
              if (gMercProfiles[ubMerc].bSex == FEMALE) {
                // say quote about not catering to women!
                (*ppResultQuoteInfo) = &pNPCQuoteInfoArray[5];
                (*pubQuoteNum) = 5;
                return ((*ppResultQuoteInfo)->ubOpinionRequired);
              }
              switch (pObj->uiMoneyAmount) {
                case 100:
                case 200:  // Carla
                  if (CheckFact(FACT_CARLA_AVAILABLE, 0)) {
                    gMercProfiles[MADAME].bNPCData += (int8_t)(pObj->uiMoneyAmount / 100);
                    TriggerNPCRecord(MADAME, 16);
                  } else {
                    // see default case
                    (*ppResultQuoteInfo) = &pNPCQuoteInfoArray[25];
                    (*pubQuoteNum) = 25;
                    return ((*ppResultQuoteInfo)->ubOpinionRequired);
                  }
                  break;
                case 500:
                case 1000:  // Cindy
                  if (CheckFact(FACT_CINDY_AVAILABLE, 0)) {
                    gMercProfiles[MADAME].bNPCData += (int8_t)(pObj->uiMoneyAmount / 500);
                    TriggerNPCRecord(MADAME, 17);
                  } else {
                    // see default case
                    (*ppResultQuoteInfo) = &pNPCQuoteInfoArray[25];
                    (*pubQuoteNum) = 25;
                    return ((*ppResultQuoteInfo)->ubOpinionRequired);
                  }
                  break;
                case 300:
                case 600:  // Bambi
                  if (CheckFact(FACT_BAMBI_AVAILABLE, 0)) {
                    gMercProfiles[MADAME].bNPCData += (int8_t)(pObj->uiMoneyAmount / 300);
                    TriggerNPCRecord(MADAME, 18);
                  } else {
                    // see default case
                    (*ppResultQuoteInfo) = &pNPCQuoteInfoArray[25];
                    (*pubQuoteNum) = 25;
                    return ((*ppResultQuoteInfo)->ubOpinionRequired);
                  }
                  break;
                case 400:
                case 800:  // Maria?
                  if (gubQuest[QUEST_RESCUE_MARIA] == QUESTINPROGRESS) {
                    gMercProfiles[MADAME].bNPCData += (int8_t)(pObj->uiMoneyAmount / 400);
                    TriggerNPCRecord(MADAME, 19);
                    break;
                  } else {
                    // see default case
                    (*ppResultQuoteInfo) = &pNPCQuoteInfoArray[25];
                    (*pubQuoteNum) = 25;
                    return ((*ppResultQuoteInfo)->ubOpinionRequired);
                  }
                  break;
                default:
                  // play quotes 39-42 (plus 44 if quest 22 on) plus 43 if >1 PC
                  // and return money
                  (*ppResultQuoteInfo) = &pNPCQuoteInfoArray[25];
                  (*pubQuoteNum) = 25;
                  return ((*ppResultQuoteInfo)->ubOpinionRequired);
              }
            }
            break;
          case JOE:
            if (ubNPC == JOE && usItemToConsider == MONEY &&
                pNPCQuoteInfo->sActionData != NPC_ACTION_JOE_GIVEN_CASH) {
              break;
            }
            // else fall through
          case GERARD:
            if (ubNPC == GERARD && usItemToConsider == MONEY &&
                pNPCQuoteInfo->sActionData != NPC_ACTION_GERARD_GIVEN_CASH) {
              break;
            }
            // else fall through
          case STEVE:
          case VINCE:
          case WALTER:
          case FRANK:
            if (usItemToConsider == MONEY) {
              if (ubNPC == VINCE || ubNPC == STEVE) {
                if (CheckFact(FACT_VINCE_EXPECTING_MONEY, ubNPC) == FALSE &&
                    gMercProfiles[ubNPC].iBalance < 0 &&
                    pNPCQuoteInfo->sActionData != NPC_ACTION_DONT_ACCEPT_ITEM) {
                  // increment balance
                  gMercProfiles[ubNPC].iBalance += (int32_t)pObj->uiMoneyAmount;
                  gMercProfiles[ubNPC].uiTotalCostToDate += pObj->uiMoneyAmount;
                  if (gMercProfiles[ubNPC].iBalance > 0) {
                    gMercProfiles[ubNPC].iBalance = 0;
                  }
                  ScreenMsg(FONT_YELLOW, MSG_INTERFACE, TacticalStr[BALANCE_OWED_STR],
                            gMercProfiles[ubNPC].zNickname, -gMercProfiles[ubNPC].iBalance);
                } else if (CheckFact(FACT_VINCE_EXPECTING_MONEY, ubNPC) == FALSE &&
                           pNPCQuoteInfo->sActionData != NPC_ACTION_DONT_ACCEPT_ITEM) {
                  // just accept cash!
                  if (ubNPC == VINCE) {
                    (*ppResultQuoteInfo) = &pNPCQuoteInfoArray[8];
                  } else {
                    (*ppResultQuoteInfo) = &pNPCQuoteInfoArray[7];
                  }
                  return ((*ppResultQuoteInfo)->ubOpinionRequired);
                } else {
                  // handle the player giving NPC some money
                  HandleNPCBeingGivenMoneyByPlayer(ubNPC, pObj->uiMoneyAmount, pubQuoteNum);
                  (*ppResultQuoteInfo) = &pNPCQuoteInfoArray[*pubQuoteNum];
                  return ((*ppResultQuoteInfo)->ubOpinionRequired);
                }
              } else {
                // handle the player giving NPC some money
                HandleNPCBeingGivenMoneyByPlayer(ubNPC, pObj->uiMoneyAmount, pubQuoteNum);
                (*ppResultQuoteInfo) = &pNPCQuoteInfoArray[*pubQuoteNum];
                return ((*ppResultQuoteInfo)->ubOpinionRequired);
              }
            }
            break;
          case KINGPIN:
            if (usItemToConsider == MONEY && gubQuest[QUEST_KINGPIN_MONEY] == QUESTINPROGRESS) {
              HandleNPCBeingGivenMoneyByPlayer(ubNPC, pObj->uiMoneyAmount, pubQuoteNum);
              (*ppResultQuoteInfo) = &pNPCQuoteInfoArray[*pubQuoteNum];
              return ((*ppResultQuoteInfo)->ubOpinionRequired);
            }
            break;
          default:
            if (usItemToConsider == MONEY &&
                (ubNPC == SKYRIDER || (ubNPC >= FIRST_RPC && ubNPC < FIRST_NPC))) {
              if (gMercProfiles[ubNPC].iBalance < 0 &&
                  pNPCQuoteInfo->sActionData != NPC_ACTION_DONT_ACCEPT_ITEM) {
                // increment balance
                gMercProfiles[ubNPC].iBalance += (int32_t)pObj->uiMoneyAmount;
                gMercProfiles[ubNPC].uiTotalCostToDate += pObj->uiMoneyAmount;
                if (gMercProfiles[ubNPC].iBalance > 0) {
                  gMercProfiles[ubNPC].iBalance = 0;
                }
                ScreenMsg(FONT_YELLOW, MSG_INTERFACE, TacticalStr[BALANCE_OWED_STR],
                          gMercProfiles[ubNPC].zNickname, -gMercProfiles[ubNPC].iBalance);
              }
            }
            break;
        }
        // This is great!
        // Return desire value
        (*ppResultQuoteInfo) = pNPCQuoteInfo;
        (*pubQuoteNum) = ubLoop;

        return (pNPCQuoteInfo->ubOpinionRequired);
      }
    }
  }

  return (0);
}

BOOLEAN HandleNPCBeingGivenMoneyByPlayer(uint8_t ubNPC, uint32_t uiMoneyAmount,
                                         uint8_t *pQuoteValue) {
  switch (ubNPC) {
    // handle for STEVE and VINCE
    case STEVE:
    case VINCE: {
      int32_t iCost;

      iCost = (int32_t)CalcMedicalCost(ubNPC);

      // check amount of money
      if ((int32_t)uiMoneyAmount + giHospitalTempBalance + giHospitalRefund >= iCost) {
        // enough cash, check how much help is needed
        if (CheckFact(FACT_WOUNDED_MERCS_NEARBY, ubNPC)) {
          *pQuoteValue = 26;
        } else if (CheckFact(FACT_ONE_WOUNDED_MERC_NEARBY, ubNPC)) {
          *pQuoteValue = 25;
        }

        if (giHospitalRefund > 0) {
          giHospitalRefund = max(0, giHospitalRefund - iCost + uiMoneyAmount);
        }
        giHospitalTempBalance = 0;
      } else {
        wchar_t sTempString[100];

        swprintf(sTempString, ARR_SIZE(sTempString), L"%ld",
                 iCost - uiMoneyAmount - giHospitalTempBalance);
        InsertDollarSignInToString(sTempString);

        // not enough cash
        ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, Message[STR_NEED_TO_GIVE_MONEY],
                  gMercProfiles[ubNPC].zNickname, sTempString);
        *pQuoteValue = 27;
        giHospitalTempBalance += uiMoneyAmount;
      }
    } break;
    case KINGPIN:
      if ((int32_t)uiMoneyAmount < -gMercProfiles[KINGPIN].iBalance) {
        *pQuoteValue = 9;
      } else {
        *pQuoteValue = 10;
      }
      gMercProfiles[KINGPIN].iBalance += (int32_t)uiMoneyAmount;
      break;
    case WALTER:
      if (gMercProfiles[WALTER].iBalance == 0) {
        *pQuoteValue = 12;
      } else {
        *pQuoteValue = 13;
      }
      gMercProfiles[WALTER].iBalance += uiMoneyAmount;
      break;
    case FRANK:
      gArmsDealerStatus[ARMS_DEALER_FRANK].uiArmsDealersCash += uiMoneyAmount;
      break;
    case GERARD:
      gMercProfiles[GERARD].iBalance += uiMoneyAmount;
      if ((gMercProfiles[GERARD].iBalance) >= 10000) {
        *pQuoteValue = 12;
      } else {
        *pQuoteValue = 11;
      }
      break;
    case JOE:
      gMercProfiles[JOE].iBalance += uiMoneyAmount;
      if ((gMercProfiles[JOE].iBalance) >= 10000) {
        *pQuoteValue = 7;
      } else {
        *pQuoteValue = 6;
      }
      break;
  }

  return (TRUE);
}

uint8_t NPCConsiderQuote(uint8_t ubNPC, uint8_t ubMerc, uint8_t ubApproach, uint8_t ubQuoteNum,
                         uint8_t ubTalkDesire, NPCQuoteInfo *pNPCQuoteInfoArray) {
  // This function looks at a quote and determines if conditions for it have been met.
  // Returns 0 if none , 1 if one is found
  MERCPROFILESTRUCT *pNPCProfile;
  NPCQuoteInfo *pNPCQuoteInfo;
  uint32_t uiDay;
  BOOLEAN fTrue;

  if (ubNPC == NO_PROFILE) {
    pNPCProfile = NULL;
  } else {
    pNPCProfile = &(gMercProfiles[ubNPC]);
  }

  // How much do we want to talk with this merc?
  uiDay = GetGameTimeInDays();

  pNPCQuoteInfo = &(pNPCQuoteInfoArray[ubQuoteNum]);

#ifdef JA2TESTVERSION
  if (ubNPC != NO_PROFILE && ubMerc != NO_PROFILE) {
    NpcRecordLoggingInit(ubNPC, ubMerc, ubQuoteNum, ubApproach);
  }
#endif

  if (CHECK_FLAG(pNPCQuoteInfo->fFlags, QUOTE_FLAG_SAID)) {
#ifdef JA2TESTVERSION
    // Add entry to the quest debug file
    NpcRecordLogging(ubApproach, "Quote Already Said, leaving");
#endif
    // skip quotes already said
    return (FALSE);
  }

  // if the quote is quest-specific, is the player on that quest?
  if (pNPCQuoteInfo->ubQuest != NO_QUEST) {
#ifdef JA2TESTVERSION
    // Add entry to the quest debug file
    NpcRecordLogging(
        ubApproach, "Quest(%d:'%S') Must be in Progress, status is %d. %s", pNPCQuoteInfo->ubQuest,
        QuestDescText[pNPCQuoteInfo->ubQuest], gubQuest[pNPCQuoteInfo->ubQuest],
        (gubQuest[pNPCQuoteInfo->ubQuest] != QUESTINPROGRESS) ? "False, return" : "True");
#endif

    if (pNPCQuoteInfo->ubQuest > QUEST_DONE_NUM) {
      if (gubQuest[pNPCQuoteInfo->ubQuest - QUEST_DONE_NUM] != QUESTDONE) {
        return (FALSE);
      }
    } else if (pNPCQuoteInfo->ubQuest > QUEST_NOT_STARTED_NUM) {
      if (gubQuest[pNPCQuoteInfo->ubQuest - QUEST_NOT_STARTED_NUM] != QUESTNOTSTARTED) {
        return (FALSE);
      }
    } else {
      if (gubQuest[pNPCQuoteInfo->ubQuest] != QUESTINPROGRESS) {
        return (FALSE);
      }
    }
  }

  // if there are facts to be checked, check them
  if (pNPCQuoteInfo->usFactMustBeTrue != NO_FACT) {
    fTrue = CheckFact(pNPCQuoteInfo->usFactMustBeTrue, ubNPC);
#ifdef JA2TESTVERSION
    // Add entry to the quest debug file
    NpcRecordLogging(ubApproach, "Fact (%d:'%S') Must be True, status is %s",
                     pNPCQuoteInfo->usFactMustBeTrue, FactDescText[pNPCQuoteInfo->usFactMustBeTrue],
                     (fTrue == FALSE) ? "False, returning" : "True");
#endif
    if (fTrue == FALSE) {
      return (FALSE);
    }
  }

  if (pNPCQuoteInfo->usFactMustBeFalse != NO_FACT) {
    fTrue = CheckFact(pNPCQuoteInfo->usFactMustBeFalse, ubNPC);

#ifdef JA2TESTVERSION
    // Add entry to the quest debug file
    NpcRecordLogging(
        ubApproach, "Fact(%d:'%S') Must be False status is  %s", pNPCQuoteInfo->usFactMustBeFalse,
        FactDescText[pNPCQuoteInfo->usFactMustBeFalse], (fTrue == TRUE) ? "True, return" : "FALSE");
#endif

    if (fTrue == TRUE) {
      return (FALSE);
    }
  }

  // check for required approach
  // since the "I hate you" code triggers the record, triggering has to work properly
  // with the other value that is stored!
  if (pNPCQuoteInfo->ubApproachRequired ||
      !(ubApproach == APPROACH_FRIENDLY || ubApproach == APPROACH_DIRECT ||
        ubApproach == TRIGGER_NPC)) {
#ifdef JA2TESTVERSION
    // Add entry to the quest debug file
    NpcRecordLogging(ubApproach, "Approach Taken(%d) must equal required Approach(%d) = %s",
                     ubApproach, pNPCQuoteInfo->ubApproachRequired,
                     (ubApproach != pNPCQuoteInfo->ubApproachRequired) ? "TRUE, return" : "FALSE");
#endif

    if (pNPCQuoteInfo->ubApproachRequired == APPROACH_ONE_OF_FOUR_STANDARD) {
      // friendly to recruit will match
      if (ubApproach < APPROACH_FRIENDLY || ubApproach > APPROACH_RECRUIT) {
        return (FALSE);
      }
    } else if (pNPCQuoteInfo->ubApproachRequired == APPROACH_FRIENDLY_DIRECT_OR_RECRUIT) {
      if (ubApproach != APPROACH_FRIENDLY && ubApproach != APPROACH_DIRECT &&
          ubApproach != APPROACH_RECRUIT) {
        return (FALSE);
      }
    } else if (ubApproach != pNPCQuoteInfo->ubApproachRequired) {
      return (FALSE);
    }
  }

  // check time constraints on the quotes
  if (pNPCProfile != NULL && pNPCQuoteInfo->ubFirstDay == MUST_BE_NEW_DAY) {
#ifdef JA2TESTVERSION
    // Add entry to the quest debug file
    NpcRecordLogging(ubApproach,
                     "Time constraints. Current Day(%d) must <= Day last spoken too (%d) : %s",
                     uiDay, pNPCProfile->ubLastDateSpokenTo,
                     (uiDay <= pNPCProfile->ubLastDateSpokenTo) ? "TRUE, return" : "FALSE");
#endif

    if (uiDay <= pNPCProfile->ubLastDateSpokenTo) {
      // too early!
      return (FALSE);
    }
  } else if (uiDay < pNPCQuoteInfo->ubFirstDay) {
#ifdef JA2TESTVERSION
    // Add entry to the quest debug file
    NpcRecordLogging(ubApproach, "Current Day(%d) is before Required first day(%d) = %s", uiDay,
                     pNPCQuoteInfo->ubFirstDay,
                     (uiDay < pNPCQuoteInfo->ubFirstDay) ? "False, returning" : "True");
#endif
    // too early!
    return (FALSE);
  }

  if (uiDay > pNPCQuoteInfo->ubLastDay && uiDay < 255) {
#ifdef JA2TESTVERSION
    // Add entry to the quest debug file
    NpcRecordLogging(ubApproach, "Current Day(%d) is after Required first day(%d) = %s", uiDay,
                     pNPCQuoteInfo->ubFirstDay,
                     (uiDay > pNPCQuoteInfo->ubLastDay) ? "TRUE, returning" : "FALSE");
#endif

    // too late!
    return (FALSE);
  }

  // check opinion required
  if ((pNPCQuoteInfo->ubOpinionRequired != IRRELEVANT) && (ubApproach != TRIGGER_NPC)) {
#ifdef JA2TESTVERSION
    // Add entry to the quest debug file
    NpcRecordLogging(
        ubApproach, "Opinion Required.  Talk Desire (%d), Opinion Required(%d) : %s", ubTalkDesire,
        pNPCQuoteInfo->ubOpinionRequired,
        (ubTalkDesire < pNPCQuoteInfo->ubOpinionRequired) ? "False, return" : "False, continue");
#endif

    if (ubTalkDesire < pNPCQuoteInfo->ubOpinionRequired) {
      return (FALSE);
    }
  }

#ifdef JA2TESTVERSION
  // Add entry to the quest debug file
  NpcRecordLogging(ubApproach, "Return the quote opinion value! = TRUE");
#endif

  // Return the quote opinion value!
  return (TRUE);
}

void ReplaceLocationInNPCData(NPCQuoteInfo *pNPCQuoteInfoArray, int16_t sOldGridNo,
                              int16_t sNewGridNo) {
  uint8_t ubFirstQuoteRecord, ubLastQuoteRecord, ubLoop;
  NPCQuoteInfo *pNPCQuoteInfo;

  ubFirstQuoteRecord = 0;
  ubLastQuoteRecord = NUM_NPC_QUOTE_RECORDS - 1;
  for (ubLoop = ubFirstQuoteRecord; ubLoop <= ubLastQuoteRecord; ubLoop++) {
    pNPCQuoteInfo = &(pNPCQuoteInfoArray[ubLoop]);
    if (sOldGridNo == -pNPCQuoteInfo->sRequiredGridno) {
      pNPCQuoteInfo->sRequiredGridno = -sNewGridNo;
    }
    if (sOldGridNo == pNPCQuoteInfo->usGoToGridno) {
      pNPCQuoteInfo->usGoToGridno = sNewGridNo;
    }
  }
}

void ReplaceLocationInNPCDataFromProfileID(uint8_t ubNPC, int16_t sOldGridNo, int16_t sNewGridNo) {
  NPCQuoteInfo *pNPCQuoteInfoArray;

  if (EnsureQuoteFileLoaded(ubNPC) == FALSE) {
    // error!!!
    return;
  }

  pNPCQuoteInfoArray = gpNPCQuoteInfoArray[ubNPC];

  ReplaceLocationInNPCData(pNPCQuoteInfoArray, sOldGridNo, sNewGridNo);
}

void ResetOncePerConvoRecords(NPCQuoteInfo *pNPCQuoteInfoArray) {
  uint8_t ubLoop;

  for (ubLoop = 0; ubLoop < NUM_NPC_QUOTE_RECORDS; ubLoop++) {
    if (CHECK_FLAG(pNPCQuoteInfoArray[ubLoop].fFlags, QUOTE_FLAG_SAY_ONCE_PER_CONVO)) {
      TURN_FLAG_OFF(pNPCQuoteInfoArray[ubLoop].fFlags, QUOTE_FLAG_SAID);
    }
  }
}

void ResetOncePerConvoRecordsForNPC(uint8_t ubNPC) {
  if (EnsureQuoteFileLoaded(ubNPC) == FALSE) {
    // error!!!
    return;
  }
  ResetOncePerConvoRecords(gpNPCQuoteInfoArray[ubNPC]);
}

void ResetOncePerConvoRecordsForAllNPCsInLoadedSector(void) {
  uint8_t ubLoop;

  if (gWorldSectorX == 0 || gWorldSectorY == 0) {
    return;
  }

  for (ubLoop = FIRST_RPC; ubLoop < NUM_PROFILES; ubLoop++) {
    if (gMercProfiles[ubLoop].sSectorX == gWorldSectorX &&
        gMercProfiles[ubLoop].sSectorY == gWorldSectorY &&
        gMercProfiles[ubLoop].bSectorZ == gbWorldSectorZ && gpNPCQuoteInfoArray[ubLoop] != NULL) {
      ResetOncePerConvoRecordsForNPC(ubLoop);
    }
  }
}

void ReturnItemToPlayerIfNecessary(uint8_t ubMerc, int8_t bApproach, uintptr_t uiApproachData,
                                   NPCQuoteInfo *pQuotePtr) {
  struct OBJECTTYPE *pObj;
  struct SOLDIERTYPE *pSoldier;

  // if the approach was changed, always return the item
  // otherwise check to see if the record in question specified refusal
  if (bApproach != APPROACH_GIVINGITEM || (pQuotePtr == NULL) ||
      (pQuotePtr->sActionData == NPC_ACTION_DONT_ACCEPT_ITEM)) {
    pObj = (struct OBJECTTYPE *)uiApproachData;

    // Find the merc
    pSoldier = FindSoldierByProfileID(ubMerc, FALSE);

    // Try to auto place object and then if it fails, put into cursor
    if (!AutoPlaceObject(pSoldier, pObj, FALSE)) {
      InternalBeginItemPointer(pSoldier, pObj, NO_SLOT);
    }
    DirtyMercPanelInterface(pSoldier, DIRTYLEVEL2);
  }
}

void Converse(uint8_t ubNPC, uint8_t ubMerc, int8_t bApproach, uintptr_t uiApproachData) {
  NPCQuoteInfo QuoteInfo;
  NPCQuoteInfo *pQuotePtr = &(QuoteInfo);
  NPCQuoteInfo *pNPCQuoteInfoArray = NULL;
  MERCPROFILESTRUCT *pProfile = NULL;
  uint8_t ubLoop, ubQuoteNum, ubRecordNum;
  struct SOLDIERTYPE *pSoldier = NULL;
  uint32_t uiDay;
  struct OBJECTTYPE *pObj = NULL;
  struct SOLDIERTYPE *pNPC;
  BOOLEAN fAttemptingToGiveItem;

  // we have to record whether an item is being given in order to determine whether,
  // in the case where the approach is overridden, we need to return the item to the
  // player
  fAttemptingToGiveItem = (bApproach == APPROACH_GIVINGITEM);

  pNPC = FindSoldierByProfileID(ubNPC, FALSE);
  if (pNPC) {
    // set delay for civ AI movement
    pNPC->uiTimeSinceLastSpoke = GetJA2Clock();

    if (CheckFact(FACT_CURRENT_SECTOR_IS_SAFE, ubNPC) == FALSE) {
      if (bApproach != TRIGGER_NPC && bApproach != APPROACH_GIVEFIRSTAID &&
          bApproach != APPROACH_DECLARATION_OF_HOSTILITY && bApproach != APPROACH_ENEMY_NPC_QUOTE) {
        if (NPCHasUnusedRecordWithGivenApproach(ubNPC, APPROACH_SECTOR_NOT_SAFE)) {
          // override with sector-not-safe approach
          bApproach = APPROACH_SECTOR_NOT_SAFE;
        }
      }
    }

    // make sure civ is awake now
    pNPC->fAIFlags &= (~AI_ASLEEP);
  }

  if (EnsureQuoteFileLoaded(ubNPC) == FALSE) {
    // error!!!

    if (fAttemptingToGiveItem) {
      ReturnItemToPlayerIfNecessary(ubMerc, bApproach, uiApproachData, NULL);
    }
    return;
  }
  pNPCQuoteInfoArray = gpNPCQuoteInfoArray[ubNPC];

  pProfile = &(gMercProfiles[ubNPC]);
  switch (bApproach) {
    case NPC_INITIAL_QUOTE:
      // reset stuff
      ResetOncePerConvoRecords(pNPCQuoteInfoArray);

      // CHEAP HACK
      // Since we don't have CONDITIONAL once-per-convo refreshes, do this in code
      // NB fact 281 is 'Darren has explained boxing rules'
      if (ubNPC == DARREN && CheckFact(281, DARREN) == FALSE) {
        TURN_FLAG_OFF(pNPCQuoteInfoArray[11].fFlags, QUOTE_FLAG_SAID);
      }

      // turn the NPC to face us
      // this '1' value is a dummy....
      NPCDoAction(ubNPC, NPC_ACTION_TURN_TO_FACE_NEAREST_MERC, 1);

      if (pProfile->ubLastDateSpokenTo > 0) {
        uiDay = GetGameTimeInDays();
        if (uiDay > pProfile->ubLastDateSpokenTo) {
          NPCConsiderTalking(ubNPC, ubMerc, APPROACH_SPECIAL_INITIAL_QUOTE, 0, pNPCQuoteInfoArray,
                             &pQuotePtr, &ubRecordNum);
          if (pQuotePtr != NULL) {
            // converse using this approach instead!
            if (fAttemptingToGiveItem) {
              ReturnItemToPlayerIfNecessary(ubMerc, bApproach, uiApproachData, NULL);
            }
            Converse(ubNPC, ubMerc, APPROACH_SPECIAL_INITIAL_QUOTE, 0);
            return;
          }
          // subsequent times approached intro
          ubQuoteNum = QUOTE_SUBS_INTRO;
        } else {
          // say nothing!
          if (fAttemptingToGiveItem) {
            ReturnItemToPlayerIfNecessary(ubMerc, bApproach, uiApproachData, NULL);
          }
          return;
        }
      } else {
        // try special initial quote first
        NPCConsiderTalking(ubNPC, ubMerc, APPROACH_SPECIAL_INITIAL_QUOTE, 0, pNPCQuoteInfoArray,
                           &pQuotePtr, &ubRecordNum);
        if (pQuotePtr != NULL) {
          // converse using this approach instead!
          if (fAttemptingToGiveItem) {
            ReturnItemToPlayerIfNecessary(ubMerc, bApproach, uiApproachData, NULL);
          }
          Converse(ubNPC, ubMerc, APPROACH_SPECIAL_INITIAL_QUOTE, 0);
          return;
        }

        NPCConsiderTalking(ubNPC, ubMerc, APPROACH_INITIAL_QUOTE, 0, pNPCQuoteInfoArray, &pQuotePtr,
                           &ubRecordNum);
        if (pQuotePtr != NULL) {
          // converse using this approach instead!
          if (fAttemptingToGiveItem) {
            ReturnItemToPlayerIfNecessary(ubMerc, bApproach, uiApproachData, NULL);
          }
          Converse(ubNPC, ubMerc, APPROACH_INITIAL_QUOTE, 0);
          return;
        }

        // first time approached intro
        ubQuoteNum = QUOTE_INTRO;
      }
      TalkingMenuDialogue(ubQuoteNum);
      pProfile->ubLastQuoteSaid = ubQuoteNum;
      pProfile->bLastQuoteSaidWasSpecial = FALSE;
      break;
    case NPC_WHOAREYOU:
      ubQuoteNum = QUOTE_INTRO;
      TalkingMenuDialogue(ubQuoteNum);
      // For now, DO NOT remember for 'Come again?'
      break;
    case APPROACH_REPEAT:
      if (pProfile->ubLastQuoteSaid == NO_QUOTE) {
        // this should never occur now!
        TalkingMenuDialogue(QUOTE_INTRO);
      } else {
        if (pProfile->bLastQuoteSaidWasSpecial) {
          pQuotePtr = &(pNPCQuoteInfoArray[pProfile->ubLastQuoteSaid]);
          // say quote and following consecutive quotes
          for (ubLoop = 0; ubLoop < pQuotePtr->ubNumQuotes; ubLoop++) {
            // say quote #(pQuotePtr->ubQuoteNum + ubLoop)
            TalkingMenuDialogue((uint8_t)(pQuotePtr->ubQuoteNum + ubLoop));
          }
        } else {
          TalkingMenuDialogue(pProfile->ubLastQuoteSaid);
        }
      }
      break;
    default:
      switch (bApproach) {
        case APPROACH_GIVINGITEM:
          // first start by triggering any introduction quote if there is one...
          if (pProfile->ubLastDateSpokenTo > 0) {
            uiDay = GetGameTimeInDays();
            if (uiDay > pProfile->ubLastDateSpokenTo) {
              NPCConsiderTalking(ubNPC, ubMerc, APPROACH_SPECIAL_INITIAL_QUOTE, 0,
                                 pNPCQuoteInfoArray, &pQuotePtr, &ubRecordNum);
              if (pQuotePtr != NULL) {
                // converse using this approach instead!
                Converse(ubNPC, ubMerc, APPROACH_SPECIAL_INITIAL_QUOTE, 0);

                if (ubNPC == DARREN) {
                  // then we have to make this give attempt fail
                  ReturnItemToPlayerIfNecessary(ubMerc, bApproach, uiApproachData, NULL);
                  return;
                }
              }
            }
          } else {
            NPCConsiderTalking(ubNPC, ubMerc, APPROACH_INITIAL_QUOTE, 0, pNPCQuoteInfoArray,
                               &pQuotePtr, &ubRecordNum);
            if (pQuotePtr != NULL) {
              // converse using this approach instead!
              Converse(ubNPC, ubMerc, APPROACH_INITIAL_QUOTE, 0);
            }
          }

          // If we are approaching because we want to give an item, do something different
          pObj = (struct OBJECTTYPE *)uiApproachData;
          NPCConsiderReceivingItemFromMerc(ubNPC, ubMerc, pObj, pNPCQuoteInfoArray, &pQuotePtr,
                                           &ubRecordNum);
          break;
        case TRIGGER_NPC:
          // if triggering, pass in the approach data as the record to consider
          DebugMsg(TOPIC_JA2, DBG_ERROR,
                   String("Handling trigger %S/%d at %ld", gMercProfiles[ubNPC].zNickname,
                          (uint8_t)uiApproachData, GetJA2Clock()));
          NPCConsiderTalking(ubNPC, ubMerc, bApproach, (uint8_t)uiApproachData, pNPCQuoteInfoArray,
                             &pQuotePtr, &ubRecordNum);
          break;
        default:
          NPCConsiderTalking(ubNPC, ubMerc, bApproach, 0, pNPCQuoteInfoArray, &pQuotePtr,
                             &ubRecordNum);
          break;
      }
      if (pQuotePtr == NULL) {
        // say random everyday quote
        // do NOT set last quote said!
        switch (bApproach) {
          case APPROACH_FRIENDLY:
            if (pProfile->bFriendlyOrDirectDefaultResponseUsedRecently) {
              ubQuoteNum = QUOTE_GETLOST;
            } else {
              ubQuoteNum = QUOTE_FRIENDLY_DEFAULT1 + (uint8_t)Random(2);
              pProfile->bFriendlyOrDirectDefaultResponseUsedRecently = TRUE;
            }
            break;
          case APPROACH_DIRECT:
            if (pProfile->bFriendlyOrDirectDefaultResponseUsedRecently) {
              ubQuoteNum = QUOTE_GETLOST;
            } else {
              ubQuoteNum = QUOTE_DIRECT_DEFAULT;
              pProfile->bFriendlyOrDirectDefaultResponseUsedRecently = TRUE;
            }
            break;
          case APPROACH_THREATEN:
            if (pProfile->bThreatenDefaultResponseUsedRecently) {
              ubQuoteNum = QUOTE_GETLOST;
            } else {
              ubQuoteNum = QUOTE_THREATEN_DEFAULT;
              pProfile->bThreatenDefaultResponseUsedRecently = TRUE;
            }
            break;
          case APPROACH_RECRUIT:
            if (pProfile->bRecruitDefaultResponseUsedRecently) {
              ubQuoteNum = QUOTE_GETLOST;
            } else {
              ubQuoteNum = QUOTE_RECRUIT_NO;
              pProfile->bRecruitDefaultResponseUsedRecently = TRUE;
            }
            break;
          case APPROACH_GIVINGITEM:
            ubQuoteNum = QUOTE_GIVEITEM_NO;

            /*
            CC - now handled below
            */
            break;
          case TRIGGER_NPC:
            // trigger did not succeed - abort!!
            return;
          default:
            ubQuoteNum = QUOTE_INTRO;
            break;
        }
        TalkingMenuDialogue(ubQuoteNum);
        pProfile->ubLastQuoteSaid = ubQuoteNum;
        pProfile->bLastQuoteSaidWasSpecial = FALSE;
        if (ubQuoteNum == QUOTE_GETLOST) {
          if (ubNPC == 70 || ubNPC == 120) {
            // becomes an enemy
            NPCDoAction(ubNPC, NPC_ACTION_BECOME_ENEMY, 0);
          }
          // close panel at end of speech
          NPCClosePanel();
        } else if (ubQuoteNum == QUOTE_GIVEITEM_NO) {
          // close panel at end of speech
          NPCClosePanel();
          if (pNPC) {
            switch (ubNPC) {
              case JIM:
              case JACK:
              case OLAF:
              case RAY:
              case OLGA:
              case TYRONE:
                // Start combat etc
                CancelAIAction(pNPC, TRUE);
                AddToShouldBecomeHostileOrSayQuoteList(pNPC->ubID);
              default:
                break;
            }
          }
        }
      } else {
#ifdef JA2BETAVERSION
        if (gfDisplayScreenMsgOnRecordUsage) {
          ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, L"Using record %d for %s", ubRecordNum,
                    gMercProfiles[ubNPC].zNickname);
        }
#endif

        // turn before speech?
        if (pQuotePtr->sActionData <= -NPC_ACTION_TURN_TO_FACE_NEAREST_MERC) {
          pSoldier = FindSoldierByProfileID(ubNPC, FALSE);
          ZEROTIMECOUNTER(pSoldier->AICounter);
          if (pSoldier->bNextAction == AI_ACTION_WAIT) {
            pSoldier->bNextAction = AI_ACTION_NONE;
            pSoldier->usNextActionData = 0;
          }
          NPCDoAction(ubNPC, (uint16_t) - (pQuotePtr->sActionData), ubRecordNum);
        }
        if (pQuotePtr->ubQuoteNum != NO_QUOTE) {
          // say quote and following consecutive quotes
          for (ubLoop = 0; ubLoop < pQuotePtr->ubNumQuotes; ubLoop++) {
            TalkingMenuDialogue((uint8_t)(pQuotePtr->ubQuoteNum + ubLoop));
          }
          pProfile->ubLastQuoteSaid = ubRecordNum;
          pProfile->bLastQuoteSaidWasSpecial = TRUE;
        }
        // set to "said" if we should do so
        if (pQuotePtr->fFlags & QUOTE_FLAG_ERASE_ONCE_SAID ||
            pQuotePtr->fFlags & QUOTE_FLAG_SAY_ONCE_PER_CONVO) {
          TURN_FLAG_ON(pQuotePtr->fFlags, QUOTE_FLAG_SAID);
        }

        // Carry out implications (actions) of this record

        // Give NPC item if appropriate
        if (bApproach == APPROACH_GIVINGITEM) {
          if (pQuotePtr->sActionData != NPC_ACTION_DONT_ACCEPT_ITEM) {
            PlaceObjectInSoldierProfile(ubNPC, pObj);

            // Find the GIVER....
            pSoldier = FindSoldierByProfileID(ubMerc, FALSE);

            // Is this one of us?
            if (pSoldier->bTeam == gbPlayerNum) {
              int8_t bSlot;

              bSlot = FindExactObj(pSoldier, pObj);
              if (bSlot != NO_SLOT) {
                RemoveObjs(&(pSoldier->inv[bSlot]), pObj->ubNumberOfObjects);
                DirtyMercPanelInterface(pSoldier, DIRTYLEVEL2);
              }
            } else {
              RemoveObjectFromSoldierProfile(ubMerc, pObj->usItem);
            }
          }
          // CC: now handled below
          /*
          else
          {
                  // ATE: Here, put back into inventory or place on ground....
                  {
                          // Find the merc
                          pSoldier = FindSoldierByProfileID( ubMerc, FALSE );

                          // Try to auto place object and then if it fails, put into cursor
                          if ( !AutoPlaceObject( pSoldier, pObj, FALSE ) )
                          {
                                  InternalBeginItemPointer( pSoldier, pObj, NO_SLOT );
                          }
                          DirtyMercPanelInterface( pSoldier, DIRTYLEVEL2 );

                  }
          }
          */
        } else if (bApproach == APPROACH_RECRUIT) {
          // the guy just joined our party
        }

        // Set things
        if (pQuotePtr->usSetFactTrue != NO_FACT) {
          SetFactTrue(pQuotePtr->usSetFactTrue);
        }
        if (pQuotePtr->ubEndQuest != NO_QUEST) {
          EndQuest(pQuotePtr->ubEndQuest, (uint8_t)gWorldSectorX, (uint8_t)gWorldSectorY);
        }
        if (pQuotePtr->ubStartQuest != NO_QUEST) {
          StartQuest(pQuotePtr->ubStartQuest, (uint8_t)gWorldSectorX, (uint8_t)gWorldSectorY);
        }

        // Give item to merc?
        if (pQuotePtr->usGiftItem >= TURN_UI_OFF) {
          switch (pQuotePtr->usGiftItem) {
            case TURN_UI_OFF:
              if (!(gTacticalStatus.uiFlags & INCOMBAT)) {
                gTacticalStatus.uiFlags |= ENGAGED_IN_CONV;
                // Increment refrence count...
                giNPCReferenceCount = 1;
              }
              break;
            case TURN_UI_ON:
              // while the special ref count is set, ignore standard off
              if (giNPCSpecialReferenceCount == 0) {
                gTacticalStatus.uiFlags &= ~ENGAGED_IN_CONV;
                // Decrement refrence count...
                giNPCReferenceCount = 0;
              }
              break;
            case SPECIAL_TURN_UI_OFF:
              if (!(gTacticalStatus.uiFlags & INCOMBAT)) {
                gTacticalStatus.uiFlags |= ENGAGED_IN_CONV;
                // Increment refrence count...
                giNPCReferenceCount = 1;
                if (giNPCSpecialReferenceCount < 0) {
                  // ???
                  giNPCSpecialReferenceCount = 0;
                }
                // increment SPECIAL reference count
                giNPCSpecialReferenceCount += 1;
              }
              break;
            case SPECIAL_TURN_UI_ON:
              // Decrement SPECIAL reference count
              giNPCSpecialReferenceCount -= 1;
              // if special count is now 0, turn reactivate UI
              if (giNPCSpecialReferenceCount == 0) {
                gTacticalStatus.uiFlags &= ~ENGAGED_IN_CONV;
                giNPCReferenceCount = 0;
              } else if (giNPCSpecialReferenceCount < 0) {
                // ???
                giNPCSpecialReferenceCount = 0;
              }
              break;
          }
        } else if (pQuotePtr->usGiftItem != 0) {
          {
            int8_t bInvPos;

            // Get soldier
            pSoldier = FindSoldierByProfileID(ubNPC, FALSE);

            // Look for item....
            bInvPos = FindObj(pSoldier, pQuotePtr->usGiftItem);

            AssertMsg(bInvPos != NO_SLOT, "NPC.C:  Gift item does not exist in NPC.");

            TalkingMenuGiveItem(ubNPC, &(pSoldier->inv[bInvPos]), bInvPos);
          }
        }
        // Action before movement?
        if (pQuotePtr->sActionData < 0 &&
            pQuotePtr->sActionData > -NPC_ACTION_TURN_TO_FACE_NEAREST_MERC) {
          pSoldier = FindSoldierByProfileID(ubNPC, FALSE);
          ZEROTIMECOUNTER(pSoldier->AICounter);
          if (pSoldier->bNextAction == AI_ACTION_WAIT) {
            pSoldier->bNextAction = AI_ACTION_NONE;
            pSoldier->usNextActionData = 0;
          }
          NPCDoAction(ubNPC, (uint16_t) - (pQuotePtr->sActionData), ubRecordNum);
        } else if (pQuotePtr->usGoToGridno == NO_MOVE && pQuotePtr->sActionData > 0) {
          pSoldier = FindSoldierByProfileID(ubNPC, FALSE);
          ZEROTIMECOUNTER(pSoldier->AICounter);
          if (pSoldier->bNextAction == AI_ACTION_WAIT) {
            pSoldier->bNextAction = AI_ACTION_NONE;
            pSoldier->usNextActionData = 0;
          }
          NPCDoAction(ubNPC, (uint16_t)(pQuotePtr->sActionData), ubRecordNum);
        }

        // Movement?
        if (pQuotePtr->usGoToGridno != NO_MOVE) {
          pSoldier = FindSoldierByProfileID(ubNPC, FALSE);

          // stupid hack CC
          if (pSoldier && ubNPC == KYLE) {
            // make sure he has keys
            pSoldier->bHasKeys = TRUE;
          }
          if (pSoldier && pSoldier->sGridNo == pQuotePtr->usGoToGridno) {
            // search for quotes to trigger immediately!
            pSoldier->ubQuoteRecord =
                ubRecordNum + 1;  // add 1 so that the value is guaranteed nonzero
            NPCReachedDestination(pSoldier, TRUE);
          } else {
            // turn off cowering
            if (pNPC->uiStatusFlags & SOLDIER_COWERING) {
              // pNPC->uiStatusFlags &= ~SOLDIER_COWERING;
              EVENT_InitNewSoldierAnim(pNPC, STANDING, 0, FALSE);
            }

            pSoldier->ubQuoteRecord =
                ubRecordNum + 1;  // add 1 so that the value is guaranteed nonzero

            if (pQuotePtr->sActionData == NPC_ACTION_TELEPORT_NPC) {
              BumpAnyExistingMerc(pQuotePtr->usGoToGridno);
              TeleportSoldier(pSoldier, pQuotePtr->usGoToGridno, FALSE);
              // search for quotes to trigger immediately!
              NPCReachedDestination(pSoldier, FALSE);
            } else {
              NPCGotoGridNo(ubNPC, pQuotePtr->usGoToGridno, ubRecordNum);
            }
          }
        }

        // Trigger other NPC?
        // ATE: Do all triggers last!
        if (pQuotePtr->ubTriggerNPC != IRRELEVANT) {
          // Check for special NPC trigger codes
          if (pQuotePtr->ubTriggerNPC == 0) {
            TriggerClosestMercWhoCanSeeNPC(ubNPC, pQuotePtr);
          } else if (pQuotePtr->ubTriggerNPC == 1) {
            // trigger self
            TriggerNPCRecord(ubNPC, pQuotePtr->ubTriggerNPCRec);
          } else {
            TriggerNPCRecord(pQuotePtr->ubTriggerNPC, pQuotePtr->ubTriggerNPCRec);
          }
        }

        // Ian says it is okay to take this out!
        /*
        if (bApproach == APPROACH_ENEMY_NPC_QUOTE)
        {
                NPCClosePanel();
        }
        */
      }
      break;
  }

  // Set last day spoken!
  switch (bApproach) {
    case APPROACH_FRIENDLY:
    case APPROACH_DIRECT:
    case APPROACH_THREATEN:
    case APPROACH_RECRUIT:
    case NPC_INITIATING_CONV:
    case NPC_INITIAL_QUOTE:
    case APPROACH_SPECIAL_INITIAL_QUOTE:
    case APPROACH_DECLARATION_OF_HOSTILITY:
    case APPROACH_INITIAL_QUOTE:
    case APPROACH_GIVINGITEM:
      pProfile->ubLastDateSpokenTo = (uint8_t)GetGameTimeInDays();
      break;
    default:
      break;
  }

  // return item?
  if (fAttemptingToGiveItem) {
    ReturnItemToPlayerIfNecessary(ubMerc, bApproach, uiApproachData, pQuotePtr);
  }
}

int16_t NPCConsiderInitiatingConv(struct SOLDIERTYPE *pNPC, uint8_t *pubDesiredMerc) {
  int16_t sMyGridNo, sDist, sDesiredMercDist = 100;
  uint8_t ubNPC, ubMerc, ubDesiredMerc = NOBODY;
  uint8_t ubTalkDesire, ubHighestTalkDesire = 0;
  struct SOLDIERTYPE *pMerc;
  struct SOLDIERTYPE *pDesiredMerc;
  NPCQuoteInfo *pNPCQuoteInfoArray;

  if (!(pubDesiredMerc)) {
    return FALSE;
  }
  sMyGridNo = pNPC->sGridNo;

  ubNPC = pNPC->ubProfile;
  if (EnsureQuoteFileLoaded(ubNPC) == FALSE) {
    // error!!!
    return (NOWHERE);
  }
  pNPCQuoteInfoArray = gpNPCQuoteInfoArray[ubNPC];

  // loop through all mercs
  for (ubMerc = 0; ubMerc < guiNumMercSlots; ubMerc++) {
    pMerc = MercSlots[ubMerc];
    if (pMerc != NULL) {
      // only look for mercs on the side of the player
      if (pMerc->bSide != gbPlayerNum) {
        continue;
      }

      // only look for active mercs
      if (pMerc->bAssignment >= ON_DUTY) {
        continue;
      }

      // if they're not visible, don't think about it
      if (pNPC->bOppList[ubMerc] != SEEN_CURRENTLY) {
        continue;
      }

      // what's the opinion required for the highest-opinion quote that we would
      // say to this merc
      ubTalkDesire = NPCConsiderTalking(pNPC->ubProfile, pMerc->ubProfile, NPC_INITIATING_CONV, 0,
                                        pNPCQuoteInfoArray, NULL, NULL);
      if (ubTalkDesire > 0) {
        if (ubTalkDesire > ubHighestTalkDesire) {
          ubHighestTalkDesire = ubTalkDesire;
          ubDesiredMerc = ubMerc;
          pDesiredMerc = MercPtrs[ubMerc];
          sDesiredMercDist = PythSpacesAway(sMyGridNo, pDesiredMerc->sGridNo);
        } else if (ubTalkDesire == ubHighestTalkDesire) {
          sDist = PythSpacesAway(sMyGridNo, MercPtrs[ubMerc]->sGridNo);
          if (sDist < sDesiredMercDist) {
            // we can say the same thing to this merc, and they're closer!
            ubDesiredMerc = ubMerc;
            sDesiredMercDist = sDist;
          }
        }
      }
    }
  }

  if (ubDesiredMerc == NOBODY) {
    return (NOWHERE);
  } else {
    *pubDesiredMerc = ubDesiredMerc;
    return (pDesiredMerc->sGridNo);
  }
}

uint8_t NPCTryToInitiateConv(
    struct SOLDIERTYPE *pNPC) {  // assumes current action is ACTION_APPROACH_MERC
  if (pNPC->bAction != AI_ACTION_APPROACH_MERC) {
    return (AI_ACTION_NONE);
  }
  if (PythSpacesAway(pNPC->sGridNo, MercPtrs[pNPC->usActionData]->sGridNo) < CONVO_DIST) {
    // initiate conversation!
    Converse(pNPC->ubProfile, MercPtrs[pNPC->usActionData]->ubProfile, NPC_INITIATING_CONV, 0);
    // after talking, wait a while before moving anywhere else
    return (AI_ACTION_WAIT);
  } else {
    // try to move towards that merc
    return (AI_ACTION_APPROACH_MERC);
  }
}

/*
BOOLEAN NPCOkToGiveItem( uint8_t ubNPC, uint8_t ubMerc, uint16_t usItem )
{
        // This function seems to be unused...

        NPCQuoteInfo					QuoteInfo;
        NPCQuoteInfo *				pQuotePtr = &(QuoteInfo);
        NPCQuoteInfo *				pNPCQuoteInfoArray;
        uint8_t									ubOpinionVal;
        uint8_t									ubQuoteNum;

        if (EnsureQuoteFileLoaded( ubNPC ) == FALSE)
        {
                // error!!!
                return( FALSE );
        }
        pNPCQuoteInfoArray = gpNPCQuoteInfoArray[ubNPC];

        ubOpinionVal =  NPCConsiderReceivingItemFromMerc( ubNPC, ubMerc, usItem, pNPCQuoteInfoArray,
&pQuotePtr, &ubQuoteNum );

        if ( ubOpinionVal )
        {
                return( TRUE );
        }
        else
        {
                return( FALSE );
        }
}
*/
void NPCReachedDestination(struct SOLDIERTYPE *pNPC, BOOLEAN fAlreadyThere) {
  // perform action or whatever after reaching our destination
  uint8_t ubNPC;
  NPCQuoteInfo *pQuotePtr;
  NPCQuoteInfo *pNPCQuoteInfoArray;
  uint8_t ubLoop;
  uint8_t ubQuoteRecord;

  if (pNPC->ubQuoteRecord == 0) {
    ubQuoteRecord = 0;
  } else {
    ubQuoteRecord = (uint8_t)(pNPC->ubQuoteRecord - 1);
  }

  // Clear values!
  pNPC->ubQuoteRecord = 0;
  if (pNPC->bTeam == gbPlayerNum) {
    // the "under ai control" flag was set temporarily; better turn it off now
    pNPC->uiStatusFlags &= (~SOLDIER_PCUNDERAICONTROL);
    // make damn sure the AI_HANDLE_EVERY_FRAME flag is turned off
    pNPC->fAIFlags &= (AI_HANDLE_EVERY_FRAME);
  }

  ubNPC = pNPC->ubProfile;
  if (EnsureQuoteFileLoaded(ubNPC) == FALSE) {
    // error!!!
    return;
  }

  pNPCQuoteInfoArray = gpNPCQuoteInfoArray[ubNPC];
  pQuotePtr = &(pNPCQuoteInfoArray[ubQuoteRecord]);
  // either we are supposed to consider a new quote record
  // (indicated by a negative gridno in the has-item field)
  // or an action to perform once we reached this gridno

  if (pNPC->sGridNo == pQuotePtr->usGoToGridno) {
    // check for an after-move action
    if (pQuotePtr->sActionData > 0) {
      NPCDoAction(ubNPC, (uint16_t)pQuotePtr->sActionData, ubQuoteRecord);
    }
  }

  for (ubLoop = 0; ubLoop < NUM_NPC_QUOTE_RECORDS; ubLoop++) {
    pQuotePtr = &(pNPCQuoteInfoArray[ubLoop]);
    if (pNPC->sGridNo == -(pQuotePtr->sRequiredGridno)) {
      if (NPCConsiderQuote(ubNPC, 0, TRIGGER_NPC, ubLoop, 0, pNPCQuoteInfoArray)) {
        if (fAlreadyThere) {
          TriggerNPCRecord(ubNPC, ubLoop);
        } else {
          // trigger this quote
          TriggerNPCRecordImmediately(ubNPC, ubLoop);
        }
        return;
      }
    }
  }
}

void TriggerNPCRecord(uint8_t ubTriggerNPC, uint8_t ubTriggerNPCRec) {
  // Check if we have a quote to trigger...
  NPCQuoteInfo *pQuotePtr;
  BOOLEAN fDisplayDialogue = TRUE;

  if (EnsureQuoteFileLoaded(ubTriggerNPC) == FALSE) {
    // error!!!
    return;
  }
  pQuotePtr = &(gpNPCQuoteInfoArray[ubTriggerNPC][ubTriggerNPCRec]);
  if (pQuotePtr->ubQuoteNum == IRRELEVANT) {
    fDisplayDialogue = FALSE;
  }

  if (NPCConsiderQuote(ubTriggerNPC, 0, TRIGGER_NPC, ubTriggerNPCRec, 0,
                       gpNPCQuoteInfoArray[ubTriggerNPC])) {
    NPCTriggerNPC(ubTriggerNPC, ubTriggerNPCRec, TRIGGER_NPC, fDisplayDialogue);
  } else {
    // don't do anything
    DebugMsg(TOPIC_JA2, DBG_INFO,
             String("WARNING: trigger of %d, record %d cannot proceed, possible error",
                    ubTriggerNPC, ubTriggerNPCRec));
  }
}

void TriggerNPCRecordImmediately(uint8_t ubTriggerNPC, uint8_t ubTriggerNPCRec) {
  // Check if we have a quote to trigger...
  NPCQuoteInfo *pQuotePtr;
  BOOLEAN fDisplayDialogue = TRUE;

  if (EnsureQuoteFileLoaded(ubTriggerNPC) == FALSE) {
    // error!!!
    return;
  }
  pQuotePtr = &(gpNPCQuoteInfoArray[ubTriggerNPC][ubTriggerNPCRec]);
  if (pQuotePtr->ubQuoteNum == IRRELEVANT) {
    fDisplayDialogue = FALSE;
  }

  if (NPCConsiderQuote(ubTriggerNPC, 0, TRIGGER_NPC, ubTriggerNPCRec, 0,
                       gpNPCQuoteInfoArray[ubTriggerNPC])) {
    // trigger IMMEDIATELY
    HandleNPCTriggerNPC(ubTriggerNPC, ubTriggerNPCRec, fDisplayDialogue, TRIGGER_NPC);
  } else {
    // don't do anything
    DebugMsg(TOPIC_JA2, DBG_INFO,
             String("WARNING: trigger of %d, record %d cannot proceed, possible error",
                    ubTriggerNPC, ubTriggerNPCRec));
  }
}

void PCsNearNPC(uint8_t ubNPC) {
  uint8_t ubLoop;
  NPCQuoteInfo *pNPCQuoteInfoArray;
  struct SOLDIERTYPE *pSoldier;
  NPCQuoteInfo *pQuotePtr;

  if (EnsureQuoteFileLoaded(ubNPC) == FALSE) {
    // error!!!
    return;
  }
  pNPCQuoteInfoArray = gpNPCQuoteInfoArray[ubNPC];

  // see what this triggers...
  SetFactTrue(FACT_PC_NEAR);

  // Clear values!
  // Get value for NPC
  pSoldier = FindSoldierByProfileID(ubNPC, FALSE);
  pSoldier->ubQuoteRecord = 0;

  for (ubLoop = 0; ubLoop < NUM_NPC_QUOTE_RECORDS; ubLoop++) {
    pQuotePtr = &(pNPCQuoteInfoArray[ubLoop]);
    if (pSoldier->sGridNo == -(pQuotePtr->sRequiredGridno)) {
      if (NPCConsiderQuote(ubNPC, 0, TRIGGER_NPC, ubLoop, 0, pNPCQuoteInfoArray)) {
        // trigger this quote IMMEDIATELY!
        TriggerNPCRecordImmediately(ubNPC, ubLoop);
        break;
      }
    }
  }

  // reset fact
  SetFactFalse(FACT_PC_NEAR);
}

BOOLEAN PCDoesFirstAidOnNPC(uint8_t ubNPC) {
  uint8_t ubLoop;
  NPCQuoteInfo *pNPCQuoteInfoArray;
  struct SOLDIERTYPE *pSoldier;
  NPCQuoteInfo *pQuotePtr;

  if (EnsureQuoteFileLoaded(ubNPC) == FALSE) {
    // error!!!
    return (FALSE);
  }
  pNPCQuoteInfoArray = gpNPCQuoteInfoArray[ubNPC];

  // Get ptr to NPC
  pSoldier = FindSoldierByProfileID(ubNPC, FALSE);
  // Clear values!
  pSoldier->ubQuoteRecord = 0;

  // Set flag...
  gMercProfiles[ubNPC].ubMiscFlags2 |= PROFILE_MISC_FLAG2_BANDAGED_TODAY;

  for (ubLoop = 0; ubLoop < NUM_NPC_QUOTE_RECORDS; ubLoop++) {
    pQuotePtr = &(pNPCQuoteInfoArray[ubLoop]);
    if (pQuotePtr->ubApproachRequired == APPROACH_GIVEFIRSTAID) {
      if (NPCConsiderQuote(ubNPC, 0, TRIGGER_NPC, ubLoop, 0, pNPCQuoteInfoArray)) {
        // trigger this quote IMMEDIATELY!
        TriggerNPCRecordImmediately(ubNPC, ubLoop);
        return (TRUE);
      }
    }
  }
  return (FALSE);
}

void TriggerClosestMercWhoCanSeeNPC(uint8_t ubNPC, NPCQuoteInfo *pQuotePtr) {
  // Loop through all mercs, gather closest mercs who can see and trigger one!
  uint8_t ubMercsInSector[40] = {0};
  uint8_t ubNumMercs = 0;
  uint8_t ubChosenMerc;
  struct SOLDIERTYPE *pTeamSoldier, *pSoldier;
  int32_t cnt;

  // First get pointer to NPC
  pSoldier = FindSoldierByProfileID(ubNPC, FALSE);

  // Loop through all our guys and randomly say one from someone in our sector

  // set up soldier ptr as first element in mercptrs list
  cnt = gTacticalStatus.Team[gbPlayerNum].bFirstID;

  // run through list
  for (pTeamSoldier = MercPtrs[cnt]; cnt <= gTacticalStatus.Team[gbPlayerNum].bLastID;
       cnt++, pTeamSoldier++) {
    // Add guy if he's a candidate...
    if (OK_INSECTOR_MERC(pTeamSoldier) &&
        pTeamSoldier->bOppList[pSoldier->ubID] == SEEN_CURRENTLY) {
      ubMercsInSector[ubNumMercs] = (uint8_t)cnt;
      ubNumMercs++;
    }
  }

  // If we are > 0
  if (ubNumMercs > 0) {
    ubChosenMerc = (uint8_t)Random(ubNumMercs);

    // Post action to close panel
    NPCClosePanel();

    // If 64, do something special
    if (pQuotePtr->ubTriggerNPCRec == QUOTE_RESPONSE_TO_MIGUEL_SLASH_QUOTE_MERC_OR_RPC_LETGO) {
      TacticalCharacterDialogueWithSpecialEvent(MercPtrs[ubMercsInSector[ubChosenMerc]],
                                                pQuotePtr->ubTriggerNPCRec,
                                                DIALOGUE_SPECIAL_EVENT_PCTRIGGERNPC, 57, 6);
    } else {
      TacticalCharacterDialogue(MercPtrs[ubMercsInSector[ubChosenMerc]],
                                pQuotePtr->ubTriggerNPCRec);
    }
  }
}

BOOLEAN TriggerNPCWithIHateYouQuote(uint8_t ubTriggerNPC) {
  // Check if we have a quote to trigger...
  NPCQuoteInfo *pNPCQuoteInfoArray;
  uint8_t ubLoop;

  if (EnsureQuoteFileLoaded(ubTriggerNPC) == FALSE) {
    // error!!!
    return (FALSE);
  }

  pNPCQuoteInfoArray = gpNPCQuoteInfoArray[ubTriggerNPC];

  for (ubLoop = 0; ubLoop < NUM_NPC_QUOTE_RECORDS; ubLoop++) {
    if (NPCConsiderQuote(ubTriggerNPC, 0, APPROACH_DECLARATION_OF_HOSTILITY, ubLoop, 0,
                         pNPCQuoteInfoArray)) {
      // trigger this quote!
      // reset approach required value so that we can trigger it
      // pQuotePtr->ubApproachRequired = TRIGGER_NPC;
      NPCTriggerNPC(ubTriggerNPC, ubLoop, APPROACH_DECLARATION_OF_HOSTILITY, TRUE);
      gMercProfiles[ubTriggerNPC].ubMiscFlags |= PROFILE_MISC_FLAG_SAID_HOSTILE_QUOTE;
      return (TRUE);
    }
  }
  return (FALSE);
}

BOOLEAN NPCHasUnusedRecordWithGivenApproach(uint8_t ubNPC, uint8_t ubApproach) {
  // Check if we have a quote that could be used
  NPCQuoteInfo *pNPCQuoteInfoArray;
  uint8_t ubLoop;

  if (EnsureQuoteFileLoaded(ubNPC) == FALSE) {
    // error!!!
    return (FALSE);
  }

  pNPCQuoteInfoArray = gpNPCQuoteInfoArray[ubNPC];

  for (ubLoop = 0; ubLoop < NUM_NPC_QUOTE_RECORDS; ubLoop++) {
    if (NPCConsiderQuote(ubNPC, 0, ubApproach, ubLoop, 0, pNPCQuoteInfoArray)) {
      return (TRUE);
    }
  }
  return (FALSE);
}

BOOLEAN NPCHasUnusedHostileRecord(uint8_t ubNPC, uint8_t ubApproach) {
  // this is just like the standard check BUT we must skip any
  // records using fact 289 and print debug msg for any records which can't be marked as used
  // Check if we have a quote that could be used
  NPCQuoteInfo *pNPCQuoteInfoArray;
  NPCQuoteInfo *pQuotePtr;
  uint8_t ubLoop;

  if (EnsureQuoteFileLoaded(ubNPC) == FALSE) {
    // error!!!
    return (FALSE);
  }

  pNPCQuoteInfoArray = gpNPCQuoteInfoArray[ubNPC];

  for (ubLoop = 0; ubLoop < NUM_NPC_QUOTE_RECORDS; ubLoop++) {
    pQuotePtr = &(pNPCQuoteInfoArray[ubLoop]);
    if (NPCConsiderQuote(ubNPC, 0, ubApproach, ubLoop, 0, pNPCQuoteInfoArray)) {
      if (pQuotePtr->usFactMustBeTrue == FACT_NPC_HOSTILE_OR_PISSED_OFF) {
        continue;
      }
#ifdef JA2BETAVERSION
      if (!(pQuotePtr->fFlags & QUOTE_FLAG_ERASE_ONCE_SAID)) {
        ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK,
                  L"Warning: possible infinite quote loop to follow.");
      }
#endif
      return (TRUE);
    }
  }
  return (FALSE);
}

BOOLEAN NPCWillingToAcceptItem(uint8_t ubNPC, uint8_t ubMerc, struct OBJECTTYPE *pObj) {
  // Check if we have a quote that could be used, that applies to this item
  NPCQuoteInfo *pNPCQuoteInfoArray;
  NPCQuoteInfo *pQuotePtr;
  uint8_t ubQuoteNum;

  if (EnsureQuoteFileLoaded(ubNPC) == FALSE) {
    // error!!!
    return (FALSE);
  }

  pNPCQuoteInfoArray = gpNPCQuoteInfoArray[ubNPC];

  NPCConsiderReceivingItemFromMerc(ubNPC, ubMerc, pObj, pNPCQuoteInfoArray, &pQuotePtr,
                                   &ubQuoteNum);

  if (pQuotePtr) {
    return (TRUE);
  }

  return (FALSE);
}

BOOLEAN GetInfoForAbandoningEPC(uint8_t ubNPC, uint16_t *pusQuoteNum, uint16_t *pusFactToSetTrue) {
  // Check if we have a quote that could be used
  NPCQuoteInfo *pNPCQuoteInfoArray;
  uint8_t ubLoop;

  if (EnsureQuoteFileLoaded(ubNPC) == FALSE) {
    // error!!!
    return (FALSE);
  }

  pNPCQuoteInfoArray = gpNPCQuoteInfoArray[ubNPC];

  for (ubLoop = 0; ubLoop < NUM_NPC_QUOTE_RECORDS; ubLoop++) {
    if (NPCConsiderQuote(ubNPC, 0, APPROACH_EPC_IN_WRONG_SECTOR, ubLoop, 0, pNPCQuoteInfoArray)) {
      *pusQuoteNum = pNPCQuoteInfoArray[ubLoop].ubQuoteNum;
      *pusFactToSetTrue = pNPCQuoteInfoArray[ubLoop].usSetFactTrue;
      return (TRUE);
    }
  }
  return (FALSE);
}

BOOLEAN TriggerNPCWithGivenApproach(uint8_t ubTriggerNPC, uint8_t ubApproach, BOOLEAN fShowPanel) {
  // Check if we have a quote to trigger...
  NPCQuoteInfo *pNPCQuoteInfoArray;
  NPCQuoteInfo *pQuotePtr;
  uint8_t ubLoop;

  if (EnsureQuoteFileLoaded(ubTriggerNPC) == FALSE) {
    // error!!!
    return (FALSE);
  }

  pNPCQuoteInfoArray = gpNPCQuoteInfoArray[ubTriggerNPC];

  for (ubLoop = 0; ubLoop < NUM_NPC_QUOTE_RECORDS; ubLoop++) {
    pQuotePtr = &(pNPCQuoteInfoArray[ubLoop]);
    if (NPCConsiderQuote(ubTriggerNPC, 0, ubApproach, ubLoop, 0, pNPCQuoteInfoArray)) {
      if (pQuotePtr->ubQuoteNum == IRRELEVANT) {
        fShowPanel = FALSE;
      } else {
        fShowPanel = TRUE;
      }

      // trigger this quote!
      // reset approach required value so that we can trigger it
      // pQuotePtr->ubApproachRequired = TRIGGER_NPC;
      NPCTriggerNPC(ubTriggerNPC, ubLoop, ubApproach, fShowPanel);
      return (TRUE);
    }
  }
  return (FALSE);
}

BOOLEAN SaveNPCInfoToSaveGameFile(FileID hFile) {
  uint32_t uiNumBytesWritten = 0;
  uint32_t cnt;
  uint8_t ubOne = 1;
  uint8_t ubZero = 0;

  // Loop through all the NPC quotes
  for (cnt = 0; cnt < NUM_PROFILES; cnt++) {
    // if there is a npc qutoe
    if (gpNPCQuoteInfoArray[cnt]) {
      // save a byte specify that there is an npc quote saved
      File_Write(hFile, &ubOne, sizeof(uint8_t), &uiNumBytesWritten);
      if (uiNumBytesWritten != sizeof(uint8_t)) {
        return (FALSE);
      }

      // Save the NPC quote entry
      File_Write(hFile, gpNPCQuoteInfoArray[cnt], sizeof(NPCQuoteInfo) * NUM_NPC_QUOTE_RECORDS,
                 &uiNumBytesWritten);
      if (uiNumBytesWritten != sizeof(NPCQuoteInfo) * NUM_NPC_QUOTE_RECORDS) {
        return (FALSE);
      }
    } else {
      // save a byte specify that there is an npc quote saved
      File_Write(hFile, &ubZero, sizeof(uint8_t), &uiNumBytesWritten);
      if (uiNumBytesWritten != sizeof(uint8_t)) {
        return (FALSE);
      }
    }
  }

  for (cnt = 0; cnt < NUM_CIVQUOTE_SECTORS; cnt++) {
    // if there is a civ quote
    if (gpCivQuoteInfoArray[cnt]) {
      // save a byte specify that there is an npc quote saved
      File_Write(hFile, &ubOne, sizeof(uint8_t), &uiNumBytesWritten);
      if (uiNumBytesWritten != sizeof(uint8_t)) {
        return (FALSE);
      }

      // Save the NPC quote entry
      File_Write(hFile, gpCivQuoteInfoArray[cnt], sizeof(NPCQuoteInfo) * NUM_NPC_QUOTE_RECORDS,
                 &uiNumBytesWritten);
      if (uiNumBytesWritten != sizeof(NPCQuoteInfo) * NUM_NPC_QUOTE_RECORDS) {
        return (FALSE);
      }
    } else {
      // save a byte specify that there is an npc quote saved
      File_Write(hFile, &ubZero, sizeof(uint8_t), &uiNumBytesWritten);
      if (uiNumBytesWritten != sizeof(uint8_t)) {
        return (FALSE);
      }
    }
  }

  return (TRUE);
}

BOOLEAN LoadNPCInfoFromSavedGameFile(FileID hFile, uint32_t uiSaveGameVersion) {
  uint32_t uiNumBytesRead = 0;
  uint32_t cnt;
  uint8_t ubLoadQuote = 0;
  uint32_t uiNumberToLoad = 0;

  // If we are trying to restore a saved game prior to version 44, use the
  // MAX_NUM_SOLDIERS, else use NUM_PROFILES.  Dave used the wrong define!
  if (uiSaveGameVersion >= 44)
    uiNumberToLoad = NUM_PROFILES;
  else
    uiNumberToLoad = MAX_NUM_SOLDIERS;

  // Loop through all the NPC quotes
  for (cnt = 0; cnt < uiNumberToLoad; cnt++) {
    // Load a byte specify that there is an npc quote Loadd
    File_Read(hFile, &ubLoadQuote, sizeof(uint8_t), &uiNumBytesRead);
    if (uiNumBytesRead != sizeof(uint8_t)) {
      return (FALSE);
    }

    // if there is an existing quote
    if (gpNPCQuoteInfoArray[cnt]) {
      // delete it
      MemFree(gpNPCQuoteInfoArray[cnt]);
      gpNPCQuoteInfoArray[cnt] = NULL;
    }

    // if there is a npc quote
    if (ubLoadQuote) {
      // if there is no memory allocated
      if (gpNPCQuoteInfoArray[cnt] == NULL) {
        // allocate memory for the quote
        gpNPCQuoteInfoArray[cnt] =
            (NPCQuoteInfo *)MemAlloc(sizeof(NPCQuoteInfo) * NUM_NPC_QUOTE_RECORDS);
        if (gpNPCQuoteInfoArray[cnt] == NULL) return (FALSE);
        memset(gpNPCQuoteInfoArray[cnt], 0, sizeof(NPCQuoteInfo) * NUM_NPC_QUOTE_RECORDS);
      }

      // Load the NPC quote entry
      File_Read(hFile, gpNPCQuoteInfoArray[cnt], sizeof(NPCQuoteInfo) * NUM_NPC_QUOTE_RECORDS,
                &uiNumBytesRead);
      if (uiNumBytesRead != sizeof(NPCQuoteInfo) * NUM_NPC_QUOTE_RECORDS) {
        return (FALSE);
      }
    } else {
    }
  }

  if (uiSaveGameVersion >= 56) {
    for (cnt = 0; cnt < NUM_CIVQUOTE_SECTORS; cnt++) {
      File_Read(hFile, &ubLoadQuote, sizeof(uint8_t), &uiNumBytesRead);
      if (uiNumBytesRead != sizeof(uint8_t)) {
        return (FALSE);
      }

      // if there is an existing quote
      if (gpCivQuoteInfoArray[cnt]) {
        // delete it
        MemFree(gpCivQuoteInfoArray[cnt]);
        gpCivQuoteInfoArray[cnt] = NULL;
      }

      // if there is a civ quote file
      if (ubLoadQuote) {
        // if there is no memory allocated
        if (gpCivQuoteInfoArray[cnt] == NULL) {
          // allocate memory for the quote
          gpCivQuoteInfoArray[cnt] =
              (NPCQuoteInfo *)MemAlloc(sizeof(NPCQuoteInfo) * NUM_NPC_QUOTE_RECORDS);
          if (gpCivQuoteInfoArray[cnt] == NULL) return (FALSE);
          memset(gpCivQuoteInfoArray[cnt], 0, sizeof(NPCQuoteInfo) * NUM_NPC_QUOTE_RECORDS);
        }

        // Load the civ quote entry
        File_Read(hFile, gpCivQuoteInfoArray[cnt], sizeof(NPCQuoteInfo) * NUM_NPC_QUOTE_RECORDS,
                  &uiNumBytesRead);
        if (uiNumBytesRead != sizeof(NPCQuoteInfo) * NUM_NPC_QUOTE_RECORDS) {
          return (FALSE);
        }
      }
    }
  }

  if (uiSaveGameVersion < 88) {
    RefreshNPCScriptRecord(NO_PROFILE, 5);  // special pass-in value for "replace PC scripts"
    RefreshNPCScriptRecord(DARYL, 11);
    RefreshNPCScriptRecord(DARYL, 14);
    RefreshNPCScriptRecord(DARYL, 15);
  }
  if (uiSaveGameVersion < 89) {
    RefreshNPCScriptRecord(KINGPIN, 23);
    RefreshNPCScriptRecord(KINGPIN, 27);
  }
  if (uiSaveGameVersion < 90) {
    RefreshNPCScriptRecord(KINGPIN, 25);
    RefreshNPCScriptRecord(KINGPIN, 26);
  }
  if (uiSaveGameVersion < 92) {
    RefreshNPCScriptRecord(MATT, 14);
    RefreshNPCScriptRecord(AUNTIE, 8);
  }
  if (uiSaveGameVersion < 93) {
    RefreshNPCScriptRecord(JENNY, 7);
    RefreshNPCScriptRecord(JENNY, 8);
    RefreshNPCScriptRecord(FRANK, 7);
    RefreshNPCScriptRecord(FRANK, 8);
    RefreshNPCScriptRecord(FATHER, 12);
    RefreshNPCScriptRecord(FATHER, 13);
  }
  if (uiSaveGameVersion < 94) {
    RefreshNPCScriptRecord(CONRAD, 0);
    RefreshNPCScriptRecord(CONRAD, 2);
    RefreshNPCScriptRecord(CONRAD, 9);
  }
  if (uiSaveGameVersion < 95) {
    RefreshNPCScriptRecord(WALDO, 6);
    RefreshNPCScriptRecord(WALDO, 7);
    RefreshNPCScriptRecord(WALDO, 10);
    RefreshNPCScriptRecord(WALDO, 11);
    RefreshNPCScriptRecord(WALDO, 12);
  }
  if (uiSaveGameVersion < 96) {
    RefreshNPCScriptRecord(HANS, 18);
    RefreshNPCScriptRecord(ARMAND, 13);
    RefreshNPCScriptRecord(DARREN, 4);
    RefreshNPCScriptRecord(DARREN, 5);
  }
  if (uiSaveGameVersion < 97) {
    RefreshNPCScriptRecord(JOHN, 22);
    RefreshNPCScriptRecord(JOHN, 23);
    RefreshNPCScriptRecord(SKYRIDER, 19);
    RefreshNPCScriptRecord(SKYRIDER, 21);
    RefreshNPCScriptRecord(SKYRIDER, 22);
  }

  if (uiSaveGameVersion < 98) {
    RefreshNPCScriptRecord(SKYRIDER, 19);
    RefreshNPCScriptRecord(SKYRIDER, 21);
    RefreshNPCScriptRecord(SKYRIDER, 22);
  }

  return (TRUE);
}

BOOLEAN SaveBackupNPCInfoToSaveGameFile(FileID hFile) {
  uint32_t uiNumBytesWritten = 0;
  uint32_t cnt;
  uint8_t ubOne = 1;
  uint8_t ubZero = 0;

  // Loop through all the NPC quotes
  for (cnt = 0; cnt < NUM_PROFILES; cnt++) {
    // if there is a npc qutoe
    if (gpBackupNPCQuoteInfoArray[cnt]) {
      // save a byte specify that there is an npc quote saved
      File_Write(hFile, &ubOne, sizeof(uint8_t), &uiNumBytesWritten);
      if (uiNumBytesWritten != sizeof(uint8_t)) {
        return (FALSE);
      }

      // Save the NPC quote entry
      File_Write(hFile, gpBackupNPCQuoteInfoArray[cnt],
                 sizeof(NPCQuoteInfo) * NUM_NPC_QUOTE_RECORDS, &uiNumBytesWritten);
      if (uiNumBytesWritten != sizeof(NPCQuoteInfo) * NUM_NPC_QUOTE_RECORDS) {
        return (FALSE);
      }
    } else {
      // save a byte specify that there is an npc quote saved
      File_Write(hFile, &ubZero, sizeof(uint8_t), &uiNumBytesWritten);
      if (uiNumBytesWritten != sizeof(uint8_t)) {
        return (FALSE);
      }
    }
  }

  return (TRUE);
}

BOOLEAN LoadBackupNPCInfoFromSavedGameFile(FileID hFile, uint32_t uiSaveGameVersion) {
  uint32_t uiNumBytesRead = 0;
  uint32_t cnt;
  uint8_t ubLoadQuote = 0;
  uint32_t uiNumberOfProfilesToLoad = 0;

  uiNumberOfProfilesToLoad = NUM_PROFILES;

  // Loop through all the NPC quotes
  for (cnt = 0; cnt < uiNumberOfProfilesToLoad; cnt++) {
    // Load a byte specify that there is an npc quote Loadd
    File_Read(hFile, &ubLoadQuote, sizeof(uint8_t), &uiNumBytesRead);
    if (uiNumBytesRead != sizeof(uint8_t)) {
      return (FALSE);
    }

    // if there is an existing quote
    if (gpBackupNPCQuoteInfoArray[cnt]) {
      // delete it
      MemFree(gpBackupNPCQuoteInfoArray[cnt]);
      gpBackupNPCQuoteInfoArray[cnt] = NULL;
    }

    // if there is a npc quote
    if (ubLoadQuote) {
      // if there is no memory allocated
      if (gpBackupNPCQuoteInfoArray[cnt] == NULL) {
        // allocate memory for the quote
        gpBackupNPCQuoteInfoArray[cnt] =
            (NPCQuoteInfo *)MemAlloc(sizeof(NPCQuoteInfo) * NUM_NPC_QUOTE_RECORDS);
        if (gpBackupNPCQuoteInfoArray[cnt] == NULL) return (FALSE);
        memset(gpBackupNPCQuoteInfoArray[cnt], 0, sizeof(NPCQuoteInfo) * NUM_NPC_QUOTE_RECORDS);
      }

      // Load the NPC quote entry
      File_Read(hFile, gpBackupNPCQuoteInfoArray[cnt], sizeof(NPCQuoteInfo) * NUM_NPC_QUOTE_RECORDS,
                &uiNumBytesRead);
      if (uiNumBytesRead != sizeof(NPCQuoteInfo) * NUM_NPC_QUOTE_RECORDS) {
        return (FALSE);
      }
    } else {
    }
  }

  return (TRUE);
}

void TriggerFriendWithHostileQuote(uint8_t ubNPC) {
  uint8_t ubMercsAvailable[40] = {0};
  uint8_t ubNumMercsAvailable = 0, ubChosenMerc;
  struct SOLDIERTYPE *pTeamSoldier;
  struct SOLDIERTYPE *pSoldier;
  int32_t cnt;
  int8_t bTeam;

  // First get pointer to NPC
  pSoldier = FindSoldierByProfileID(ubNPC, FALSE);
  if (!pSoldier) {
    return;
  }
  bTeam = pSoldier->bTeam;

  // Loop through all our guys and find one to yell

  // set up soldier ptr as first element in mercptrs list
  cnt = gTacticalStatus.Team[bTeam].bFirstID;

  // run through list
  for (pTeamSoldier = MercPtrs[cnt]; cnt <= gTacticalStatus.Team[bTeam].bLastID;
       cnt++, pTeamSoldier++) {
    // Add guy if he's a candidate...
    if (pTeamSoldier->bActive && pSoldier->bInSector && pTeamSoldier->bLife >= OKLIFE &&
        pTeamSoldier->bBreath >= OKBREATH && pTeamSoldier->bOppCnt > 0 &&
        pTeamSoldier->ubProfile != NO_PROFILE) {
      if (bTeam == CIV_TEAM && pSoldier->ubCivilianGroup != NON_CIV_GROUP &&
          pTeamSoldier->ubCivilianGroup != pSoldier->ubCivilianGroup) {
        continue;
      }

      if (!(gMercProfiles[pTeamSoldier->ubProfile].ubMiscFlags &
            PROFILE_MISC_FLAG_SAID_HOSTILE_QUOTE)) {
        ubMercsAvailable[ubNumMercsAvailable] = pTeamSoldier->ubProfile;
        ubNumMercsAvailable++;
      }
    }
  }

  if (bTeam == CIV_TEAM && pSoldier->ubCivilianGroup != NON_CIV_GROUP &&
      GetCivGroupHostility(pSoldier->ubCivilianGroup) == CIV_GROUP_NEUTRAL) {
    CivilianGroupMemberChangesSides(pSoldier);
  }

  if (ubNumMercsAvailable > 0) {
    PauseAITemporarily();
    ubChosenMerc = (uint8_t)Random(ubNumMercsAvailable);
    TriggerNPCWithIHateYouQuote(ubMercsAvailable[ubChosenMerc]);
  } else {
    // done... we should enter combat mode with this soldier's team starting,
    // after all the dialogue is completed
    NPCDoAction(ubNPC, NPC_ACTION_ENTER_COMBAT, 0);
  }
}

uint8_t ActionIDForMovementRecord(uint8_t ubNPC, uint8_t ubRecord) {
  // Check if we have a quote to trigger...
  NPCQuoteInfo *pNPCQuoteInfoArray;
  NPCQuoteInfo *pQuotePtr;

  if (EnsureQuoteFileLoaded(ubNPC) == FALSE) {
    // error!!!
    return (FALSE);
  }

  pNPCQuoteInfoArray = gpNPCQuoteInfoArray[ubNPC];

  pQuotePtr = &(pNPCQuoteInfoArray[ubRecord]);

  switch (pQuotePtr->sActionData) {
    case NPC_ACTION_TRAVERSE_MAP_EAST:
      return (QUOTE_ACTION_ID_TRAVERSE_EAST);

    case NPC_ACTION_TRAVERSE_MAP_SOUTH:
      return (QUOTE_ACTION_ID_TRAVERSE_SOUTH);

    case NPC_ACTION_TRAVERSE_MAP_WEST:
      return (QUOTE_ACTION_ID_TRAVERSE_WEST);

    case NPC_ACTION_TRAVERSE_MAP_NORTH:
      return (QUOTE_ACTION_ID_TRAVERSE_NORTH);

    default:
      return (QUOTE_ACTION_ID_CHECKFORDEST);
  }
}

void HandleNPCChangesForTacticalTraversal(struct SOLDIERTYPE *pSoldier) {
  if (!pSoldier || GetSolProfile(pSoldier) == NO_PROFILE ||
      (pSoldier->fAIFlags & AI_CHECK_SCHEDULE)) {
    return;
  }

  switch (pSoldier->ubQuoteActionID) {
    case QUOTE_ACTION_ID_TRAVERSE_EAST:
      gMercProfiles[GetSolProfile(pSoldier)].sSectorX++;

      // Call to change the NPC's Sector Location
      ChangeNpcToDifferentSector(GetSolProfile(pSoldier),
                                 (uint8_t)gMercProfiles[GetSolProfile(pSoldier)].sSectorX,
                                 (uint8_t)gMercProfiles[GetSolProfile(pSoldier)].sSectorY,
                                 gMercProfiles[GetSolProfile(pSoldier)].bSectorZ);
      break;
    case QUOTE_ACTION_ID_TRAVERSE_SOUTH:
      gMercProfiles[GetSolProfile(pSoldier)].sSectorY++;

      // Call to change the NPC's Sector Location
      ChangeNpcToDifferentSector(GetSolProfile(pSoldier),
                                 (uint8_t)gMercProfiles[GetSolProfile(pSoldier)].sSectorX,
                                 (uint8_t)gMercProfiles[GetSolProfile(pSoldier)].sSectorY,
                                 gMercProfiles[GetSolProfile(pSoldier)].bSectorZ);
      break;
    case QUOTE_ACTION_ID_TRAVERSE_WEST:
      gMercProfiles[GetSolProfile(pSoldier)].sSectorX--;

      // Call to change the NPC's Sector Location
      ChangeNpcToDifferentSector(GetSolProfile(pSoldier),
                                 (uint8_t)gMercProfiles[GetSolProfile(pSoldier)].sSectorX,
                                 (uint8_t)gMercProfiles[GetSolProfile(pSoldier)].sSectorY,
                                 gMercProfiles[GetSolProfile(pSoldier)].bSectorZ);
      break;
    case QUOTE_ACTION_ID_TRAVERSE_NORTH:
      gMercProfiles[GetSolProfile(pSoldier)].sSectorY--;

      // Call to change the NPC's Sector Location
      ChangeNpcToDifferentSector(GetSolProfile(pSoldier),
                                 (uint8_t)gMercProfiles[GetSolProfile(pSoldier)].sSectorX,
                                 (uint8_t)gMercProfiles[GetSolProfile(pSoldier)].sSectorY,
                                 gMercProfiles[GetSolProfile(pSoldier)].bSectorZ);
      break;
    default:
      break;
  }
}

void HandleVictoryInNPCSector(uint8_t sSectorX, uint8_t sSectorY, int8_t sSectorZ) {
  // handle special cases of victory in certain sector
  int16_t sSector = 0;

  // not the surface?..leave
  if (sSectorZ != 0) {
    return;
  }

  // grab sector value
  sSector = GetSectorID8(sSectorX, sSectorY);

  switch (sSector) {
    case (SEC_F10): {
      // we won over the hillbillies
      // set fact they are dead
      if (CheckFact(FACT_HILLBILLIES_KILLED, KEITH) == FALSE) {
        SetFactTrue(FACT_HILLBILLIES_KILLED);
      }

      // check if keith is out of business
      if (CheckFact(FACT_KEITH_OUT_OF_BUSINESS, KEITH) == TRUE) {
        SetFactFalse(FACT_KEITH_OUT_OF_BUSINESS);
      }
    }
  }

  return;
}

BOOLEAN HandleShopKeepHasBeenShutDown(uint8_t ubCharNum) {
  // check if shopkeep has been shutdown, if so handle
  switch (ubCharNum) {
    case (KEITH): {
      // if keith out of business, do action and leave
      if (CheckFact(FACT_KEITH_OUT_OF_BUSINESS, KEITH) == TRUE) {
        TriggerNPCRecord(KEITH, 11);

        return (TRUE);
      } else if (CheckFact(FACT_LOYALTY_LOW, KEITH) == TRUE) {
        // loyalty is too low
        TriggerNPCRecord(KEITH, 7);

        return (TRUE);
      }
    }
  }

  return (FALSE);
}

#ifdef JA2BETAVERSION
void ToggleNPCRecordDisplay(void) {
  if (gfDisplayScreenMsgOnRecordUsage) {
    gfDisplayScreenMsgOnRecordUsage = FALSE;
    ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, L"Turning record reporting OFF");
  } else {
    gfDisplayScreenMsgOnRecordUsage = TRUE;
    ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, L"Turning record reporting ON");
  }
}
#endif

void UpdateDarrelScriptToGoTo(struct SOLDIERTYPE *pSoldier) {
  // change destination in Darrel record 10 to go to a gridno adjacent to the
  // soldier's gridno, and destination in record 11
  int16_t sAdjustedGridNo;
  uint8_t ubDummyDirection;
  struct SOLDIERTYPE *pDarrel;

  pDarrel = FindSoldierByProfileID(DARREL, FALSE);
  if (!pDarrel) {
    return;
  }

  // find a spot to an alternate location nearby
  sAdjustedGridNo =
      FindGridNoFromSweetSpotExcludingSweetSpot(pDarrel, pSoldier->sGridNo, 5, &ubDummyDirection);
  if (sAdjustedGridNo == NOWHERE) {
    // yikes! try again with a bigger radius!
    sAdjustedGridNo = FindGridNoFromSweetSpotExcludingSweetSpot(pDarrel, pSoldier->sGridNo, 10,
                                                                &ubDummyDirection);
    if (sAdjustedGridNo == NOWHERE) {
      // ok, now we're completely foobar
      return;
    }
  }

  EnsureQuoteFileLoaded(DARREL);
  gpNPCQuoteInfoArray[DARREL][10].usGoToGridno = sAdjustedGridNo;
  gpNPCQuoteInfoArray[DARREL][11].sRequiredGridno = -(sAdjustedGridNo);
  gpNPCQuoteInfoArray[DARREL][11].ubTriggerNPC = GetSolProfile(pSoldier);
}

BOOLEAN RecordHasDialogue(uint8_t ubNPC, uint8_t ubRecord) {
  if (EnsureQuoteFileLoaded(ubNPC) == FALSE) {
    // error!!!
    return (FALSE);
  }

  if (gpNPCQuoteInfoArray[ubNPC][ubRecord].ubQuoteNum != NO_QUOTE &&
      gpNPCQuoteInfoArray[ubNPC][ubRecord].ubQuoteNum != 0) {
    return (TRUE);
  } else {
    return (FALSE);
  }
}

int8_t FindCivQuoteFileIndex(uint8_t sSectorX, uint8_t sSectorY, int8_t sSectorZ) {
  uint8_t ubLoop;

  if (sSectorZ > 0) {
    return (MINERS_CIV_QUOTE_INDEX);
  } else {
    for (ubLoop = 0; ubLoop < NUM_CIVQUOTE_SECTORS; ubLoop++) {
      if (gsCivQuoteSector[ubLoop][0] == sSectorX && gsCivQuoteSector[ubLoop][1] == sSectorY) {
        return (ubLoop);
      }
    }
  }
  return (-1);
}

int8_t ConsiderCivilianQuotes(uint8_t sSectorX, uint8_t sSectorY, int8_t sSectorZ,
                              BOOLEAN fSetAsUsed) {
  int8_t bLoop, bCivQuoteSectorIndex;
  NPCQuoteInfo *pCivQuoteInfoArray;

  bCivQuoteSectorIndex = FindCivQuoteFileIndex(sSectorX, sSectorY, sSectorZ);
  if (bCivQuoteSectorIndex == -1) {
    // no hints for this sector!
    return (-1);
  }

  if (EnsureCivQuoteFileLoaded(bCivQuoteSectorIndex) == FALSE) {
    // error!!!
    return (-1);
  }

  pCivQuoteInfoArray = gpCivQuoteInfoArray[bCivQuoteSectorIndex];

  for (bLoop = 0; bLoop < NUM_NPC_QUOTE_RECORDS; bLoop++) {
    if (NPCConsiderQuote(NO_PROFILE, NO_PROFILE, 0, bLoop, 0, pCivQuoteInfoArray)) {
      if (fSetAsUsed) {
        TURN_FLAG_ON(pCivQuoteInfoArray[bLoop].fFlags, QUOTE_FLAG_SAID);
      }

      if (pCivQuoteInfoArray[bLoop].ubStartQuest != NO_QUEST) {
        StartQuest(pCivQuoteInfoArray[bLoop].ubStartQuest, (uint8_t)gWorldSectorX,
                   (uint8_t)gWorldSectorY);
      }

      // return quote #
      return (pCivQuoteInfoArray[bLoop].ubQuoteNum);
    }
  }

  return (-1);
}
