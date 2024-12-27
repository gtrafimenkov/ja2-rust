#ifndef MEANWHILE_H
#define MEANWHILE_H

#include "SGP/Types.h"

enum {
  END_OF_PLAYERS_FIRST_BATTLE,
  DRASSEN_LIBERATED,
  CAMBRIA_LIBERATED,
  ALMA_LIBERATED,
  GRUMM_LIBERATED,
  CHITZENA_LIBERATED,
  NW_SAM,
  NE_SAM,
  CENTRAL_SAM,
  FLOWERS,
  LOST_TOWN,
  INTERROGATION,
  CREATURES,
  KILL_CHOPPER,
  AWOL_SCIENTIST,
  OUTSKIRTS_MEDUNA,
  BALIME_LIBERATED,
  NUM_MEANWHILES
};

typedef struct {
  int16_t sSectorX;
  int16_t sSectorY;
  uint16_t usTriggerEvent;

  uint8_t ubMeanwhileID;
  uint8_t ubNPCNumber;

} MEANWHILE_DEFINITION;

void ScheduleMeanwhileEvent(MEANWHILE_DEFINITION *pMeanwhileDef, uint32_t uiTime);
void HandleFirstBattleVictory(void);

BOOLEAN BeginMeanwhile(uint8_t ubMeanwhileID);

void CheckForMeanwhileOKStart();
void EndMeanwhile();

BOOLEAN AreInMeanwhile();
uint8_t GetMeanwhileID();
BOOLEAN AreReloadingFromMeanwhile();

void LocateToMeanwhileCharacter();

// post meanwhile event for town liberation
void HandleMeanWhileEventPostingForTownLiberation(uint8_t bTownId);

// post meanwhile event for SAM liberation
void HandleMeanWhileEventPostingForSAMLiberation(int8_t bSAMId);

// trash world has been called, should we do the first meanwhile?
void HandleFirstMeanWhileSetUpWithTrashWorld(void);

// battle ended, check if we should set up a meanwhile?
void HandleFirstBattleEndingWhileInTown(u8 sSectorX, u8 sSectorY, int16_t bSectorZ,
                                        BOOLEAN fFromAutoResolve);

// lost an entire town to the enemy!
void HandleMeanWhileEventPostingForTownLoss(uint8_t bTownId);

// handle short cutting past a meanwhilescene while it is being handled
void HandleShortCuttingOfMeanwhileSceneByPlayer(uint8_t ubMeanwhileID, INT32 iLastProfileId,
                                                INT32 iLastProfileAction);

// handle release of creatures meanwhile
void HandleCreatureRelease(void);

// handle sending flowers to the queen
void HandleFlowersMeanwhileScene(int8_t bTimeCode);

// player reaches the outskirts of Meduna
void HandleOutskirtsOfMedunaMeanwhileScene(void);

// let player know about Madlab after certain status % reached
void HandleScientistAWOLMeanwhileScene(void);

// handle chopper used meanwhile
void HandleKillChopperMeanwhileScene(void);

// handle interrogation meanwhile
void HandleInterrogationMeanwhileScene(void);

extern MEANWHILE_DEFINITION gCurrentMeanwhileDef;
extern MEANWHILE_DEFINITION gMeanwhileDef[NUM_MEANWHILES];
extern BOOLEAN gfMeanwhileScheduled;
extern BOOLEAN gfMeanwhileTryingToStart;
extern BOOLEAN gfInMeanwhile;
extern uint32_t uiMeanWhileFlags;

#endif
