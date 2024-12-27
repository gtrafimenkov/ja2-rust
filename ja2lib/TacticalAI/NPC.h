// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#ifndef NPC_H
#define NPC_H

#include "SGP/Types.h"
#include "Strategic/MapScreen.h"
#include "rust_fileman.h"

#define NUM_NPC_QUOTE_RECORDS 50

struct OBJECTTYPE;

// #define IRRELEVANT 255
// #define NO_QUEST 255
// #define NO_FACT 255
// #define NO_QUOTE 255
#define MUST_BE_NEW_DAY 254
#define INITIATING_FACTOR 30

#define TURN_FLAG_ON(a, b) (a |= b)
#define TURN_FLAG_OFF(a, b) (a &= ~(b))
#define CHECK_FLAG(a, b) (a & b)

#define QUOTE_FLAG_SAID 0x0001
#define QUOTE_FLAG_ERASE_ONCE_SAID 0x0002
#define QUOTE_FLAG_SAY_ONCE_PER_CONVO 0x0004

#define NPC_TALK_RADIUS 4

#define TURN_UI_OFF 65000
#define TURN_UI_ON 65001
#define SPECIAL_TURN_UI_OFF 65002
#define SPECIAL_TURN_UI_ON 65003

#define LARGE_AMOUNT_MONEY 1000

#define ACCEPT_ANY_ITEM 1000
#define ANY_RIFLE 1001

typedef struct {
#if defined(RUSSIAN)
  uint8_t ubIdentifier[4];
#endif

  uint16_t fFlags;

  // conditions
  union {
    int16_t sRequiredItem;    // item NPC must have to say quote
    int16_t sRequiredGridno;  // location for NPC req'd to say quote
  };
  uint16_t usFactMustBeTrue;   // ...before saying quote
  uint16_t usFactMustBeFalse;  // ...before saying quote
  uint8_t ubQuest;             // quest must be current to say quote
  uint8_t ubFirstDay;          // first day quote can be said
  uint8_t ubLastDay;           // last day quote can be said
  uint8_t ubApproachRequired;  // must use this approach to generate quote
  uint8_t ubOpinionRequired;   // opinion needed for this quote     13 bytes

  // quote to say (if any)
  uint8_t ubQuoteNum;   // this is the quote to say
  uint8_t ubNumQuotes;  // total # of quotes to say          15 bytes

  // actions
  uint8_t ubStartQuest;
  uint8_t ubEndQuest;
  uint8_t ubTriggerNPC;
  uint8_t ubTriggerNPCRec;
  uint8_t ubFiller;  //                                       20 bytes
  uint16_t usSetFactTrue;
  uint16_t usGiftItem;  // item NPC gives to merc after saying quote
  uint16_t usGoToGridno;
  int16_t sActionData;  // special action value

#if !defined(RUSSIAN)
  uint8_t ubUnused[4];
#endif

} NPCQuoteInfo;  // 32 bytes

typedef enum {
  APPROACH_FRIENDLY = 1,
  APPROACH_DIRECT,
  APPROACH_THREATEN,
  APPROACH_RECRUIT,
  APPROACH_REPEAT,

  APPROACH_GIVINGITEM,
  NPC_INITIATING_CONV,
  NPC_INITIAL_QUOTE,
  NPC_WHOAREYOU,
  TRIGGER_NPC,

  APPROACH_GIVEFIRSTAID,
  APPROACH_SPECIAL_INITIAL_QUOTE,
  APPROACH_ENEMY_NPC_QUOTE,
  APPROACH_DECLARATION_OF_HOSTILITY,
  APPROACH_EPC_IN_WRONG_SECTOR,

  APPROACH_EPC_WHO_IS_RECRUITED,
  APPROACH_INITIAL_QUOTE,
  APPROACH_CLOSING_SHOP,
  APPROACH_SECTOR_NOT_SAFE,
  APPROACH_DONE_SLAPPED,  // 20

  APPROACH_DONE_PUNCH_0,
  APPROACH_DONE_PUNCH_1,
  APPROACH_DONE_PUNCH_2,
  APPROACH_DONE_OPEN_STRUCTURE,
  APPROACH_DONE_GET_ITEM,  // 25

  APPROACH_DONE_GIVING_ITEM,
  APPROACH_DONE_TRAVERSAL,
  APPROACH_BUYSELL,
  APPROACH_ONE_OF_FOUR_STANDARD,
  APPROACH_FRIENDLY_DIRECT_OR_RECRUIT,  // 30
} Approaches;

typedef enum {
  QUOTE_INTRO = 0,
  QUOTE_SUBS_INTRO,
  QUOTE_FRIENDLY_DEFAULT1,
  QUOTE_FRIENDLY_DEFAULT2,
  QUOTE_GIVEITEM_NO,
  QUOTE_DIRECT_DEFAULT,
  QUOTE_THREATEN_DEFAULT,
  QUOTE_RECRUIT_NO,
  QUOTE_BYE,
  QUOTE_GETLOST

} StandardQuoteIDs;

#define NUM_REAL_APPROACHES APPROACH_RECRUIT

#define CONVO_DIST 5

extern int8_t gbFirstApproachFlags[4];

extern uint8_t gubTeamPenalty;

extern void ShutdownNPCQuotes(void);

extern void SetQuoteRecordAsUsed(uint8_t ubNPC, uint8_t ubRecord);

// uiApproachData is used for approach things like giving items, etc.
extern uint8_t CalcDesireToTalk(uint8_t ubNPC, uint8_t ubMerc, int8_t bApproach);
extern void Converse(uint8_t ubNPC, uint8_t ubMerc, int8_t bApproach, uintptr_t uiApproachData);

extern BOOLEAN NPCOkToGiveItem(uint8_t ubNPC, uint8_t ubMerc, uint16_t usItem);
extern void NPCReachedDestination(struct SOLDIERTYPE *pNPC, BOOLEAN fAlreadyThere);
extern void PCsNearNPC(uint8_t ubNPC);
extern BOOLEAN PCDoesFirstAidOnNPC(uint8_t ubNPC);
extern void TriggerNPCRecord(uint8_t ubTriggerNPC, uint8_t ubTriggerNPCRec);
extern BOOLEAN TriggerNPCWithIHateYouQuote(uint8_t ubTriggerNPC);

extern void TriggerNPCRecordImmediately(uint8_t ubTriggerNPC, uint8_t ubTriggerNPCRec);

extern BOOLEAN TriggerNPCWithGivenApproach(uint8_t ubTriggerNPC, uint8_t ubApproach,
                                           BOOLEAN fShowPanel);

extern BOOLEAN ReloadQuoteFile(uint8_t ubNPC);
extern BOOLEAN ReloadAllQuoteFiles(void);

// Save and loads the npc info to a saved game file
extern BOOLEAN SaveNPCInfoToSaveGameFile(FileID hFile);
BOOLEAN LoadNPCInfoFromSavedGameFile(FileID hFile, uint32_t uiSaveGameVersion);

extern void TriggerFriendWithHostileQuote(uint8_t ubNPC);

extern void ReplaceLocationInNPCDataFromProfileID(uint8_t ubNPC, int16_t sOldGridNo,
                                                  int16_t sNewGridNo);

extern uint8_t ActionIDForMovementRecord(uint8_t ubNPC, uint8_t ubRecord);

// total amount given to doctors
extern uint32_t uiTotalAmountGivenToDoctors;

// handle money being npc being
extern BOOLEAN HandleNPCBeingGivenMoneyByPlayer(uint8_t ubNPCId, uint32_t uiMoneyAmount,
                                                uint8_t *pQuoteValue);

// given a victory in this sector, handle specific facts
void HandleVictoryInNPCSector(uint8_t sSectorX, uint8_t sSectorY, int8_t sSectorZ);

// check if this shopkeep has been shutdown, if so do soething and return the fact
BOOLEAN HandleShopKeepHasBeenShutDown(uint8_t ubCharNum);

BOOLEAN NPCHasUnusedRecordWithGivenApproach(uint8_t ubNPC, uint8_t ubApproach);
BOOLEAN NPCWillingToAcceptItem(uint8_t ubNPC, uint8_t ubMerc, struct OBJECTTYPE *pObj);

BOOLEAN SaveBackupNPCInfoToSaveGameFile(FileID hFile);
BOOLEAN LoadBackupNPCInfoFromSavedGameFile(FileID hFile, uint32_t uiSaveGameVersion);

#ifdef JA2BETAVERSION
void ToggleNPCRecordDisplay(void);
#endif

void UpdateDarrelScriptToGoTo(struct SOLDIERTYPE *pSoldier);

#define WALTER_BRIBE_AMOUNT 20000

BOOLEAN GetInfoForAbandoningEPC(uint8_t ubNPC, uint16_t *pusQuoteNum, uint16_t *pusFactToSetTrue);

BOOLEAN RecordHasDialogue(uint8_t ubNPC, uint8_t ubRecord);

int8_t ConsiderCivilianQuotes(uint8_t sSectorX, uint8_t sSectorY, int8_t sSectorZ,
                              BOOLEAN fSetAsUsed);

void ResetOncePerConvoRecordsForNPC(uint8_t ubNPC);

void HandleNPCChangesForTacticalTraversal(struct SOLDIERTYPE *pSoldier);

BOOLEAN NPCHasUnusedHostileRecord(uint8_t ubNPC, uint8_t ubApproach);

void ResetOncePerConvoRecordsForAllNPCsInLoadedSector(void);

#endif
