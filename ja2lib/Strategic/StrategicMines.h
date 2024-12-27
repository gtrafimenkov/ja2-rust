// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#ifndef __STRATEGIC_MINES_H
#define __STRATEGIC_MINES_H

#include "SGP/Types.h"
#include "Town.h"
#include "rust_fileman.h"

// the mines
enum {
  MINE_SAN_MONA = 0,
  MINE_DRASSEN,
  MINE_ALMA,
  MINE_CAMBRIA,
  MINE_CHITZENA,
  MINE_GRUMM,
  MAX_NUMBER_OF_MINES,
};

enum {
  MINER_FRED = 0,
  MINER_MATT,
  MINER_OSWALD,
  MINER_CALVIN,
  MINER_CARL,
  NUM_HEAD_MINERS,
};

// different types of mines
enum {
  SILVER_MINE = 0,
  GOLD_MINE,
  NUM_MINE_TYPES,
};

// monster infestatation level, as percieved by the villagers
enum {
  MINES_NO_MONSTERS = 0,    // nothing there at all
  MINES_TRACE_MONSTERS,     // monsters leave traces behind, but no one has seen them yet
  MINES_SOME_MONSTERS,      // there have been sightings but no one believes the witnesses
  MINES_MODERATE_MONSTERS,  // scattered reports of monsters in lower levels, leaves workers uneasy
  MINES_HIGH_MONSTERS,      // workers know they are there and all but a few refuse to work
  MINES_RAMPANT_MONSTERS,  // the few that go down don't seem to return, no one wants to work period
  MINES_MONSTERS_SURFACE,  // monsters are coming out of the mines into the town looking for any
                           // edibles
};

// head miner quote types
enum {
  HEAD_MINER_STRATEGIC_QUOTE_RUNNING_OUT = 0,
  HEAD_MINER_STRATEGIC_QUOTE_CREATURES_ATTACK,
  HEAD_MINER_STRATEGIC_QUOTE_CREATURES_GONE,
  HEAD_MINER_STRATEGIC_QUOTE_CREATURES_AGAIN,
  NUM_HEAD_MINER_STRATEGIC_QUOTES
};

// the strategic mine structures
typedef struct MINE_LOCATION_TYPE {
  uint8_t sSectorX;        // x value of sector mine is in
  uint8_t sSectorY;        // y value of sector mine is in
  int8_t bAssociatedTown;  // associated town of this mine

} MINE_LOCATION_TYPE;

typedef struct MINE_STATUS_TYPE {
  uint8_t ubMineType;  // type of mine (silver or gold)
  uint8_t filler1[3];
  uint32_t uiMaxRemovalRate;  // fastest rate we can move ore from this mine in period

  uint32_t uiRemainingOreSupply;  // the total value left to this mine (-1 means unlimited)
  uint32_t uiOreRunningOutPoint;  // when supply drop below this, workers tell player the mine is
                                  // running out of ore

  BOOLEAN fEmpty;       // whether no longer minable
  BOOLEAN fRunningOut;  // whether mine is beginning to run out
  BOOLEAN
  fWarnedOfRunningOut;  // whether mine foreman has already told player the mine's running out
  BOOLEAN fShutDownIsPermanent;    // means will never produce again in the game (head miner was
                                   // attacked & died/quit)
  BOOLEAN fShutDown;               // TRUE means mine production has been shut off
  BOOLEAN fPrevInvadedByMonsters;  // whether or not mine has been previously invaded by monsters
  BOOLEAN fSpokeToHeadMiner;  // player doesn't receive income from mine without speaking to the
                              // head miner first
  BOOLEAN fMineHasProducedForPlayer;  // player has earned income from this mine at least once

  BOOLEAN fQueenRetookProducingMine;  // whether or not queen ever retook a mine after a player had
                                      // produced from it
  BOOLEAN fAttackedHeadMiner;         // player has attacked the head miner, shutting down mine &
                                      // decreasing loyalty
  uint16_t usValidDayCreaturesCanInfest;   // Creatures will be permitted to spread if the game day
                                           // is greater than this value.
  uint32_t uiTimePlayerProductionStarted;  // time in minutes when 'fMineHasProducedForPlayer' was
                                           // first set

  uint8_t filler[11];  // reserved for expansion

} MINE_STATUS_TYPE;

typedef struct HEAD_MINER_TYPE {
  uint16_t usProfileId;
  int8_t bQuoteNum[NUM_HEAD_MINER_STRATEGIC_QUOTES];
  uint8_t ubExternalFace;
} HEAD_MINER_TYPE;

// init mines
void InitializeMines(void);

void HourlyMinesUpdate(void);

// get total left in this mine
int32_t GetTotalLeftInMine(int8_t bMineIndex);

// get max rates for this mine (per period, per day)
uint32_t GetMaxPeriodicRemovalFromMine(int8_t bMineIndex);
uint32_t GetMaxDailyRemovalFromMine(int8_t bMineIndex);

// which town does this mine belong to?
int8_t GetTownAssociatedWithMine(int8_t bMineIndex);

// which mine belongs tot his town
int8_t GetMineAssociatedWithThisTown(TownID bTownId);

// posts the actual mine production events daily
void PostEventsForMineProduction(void);

// the periodic checking for income from mines
void HandleIncomeFromMines(void);

// predict income from mines
int32_t PredictIncomeFromPlayerMines(void);

// predict income from a mine
uint32_t PredictDailyIncomeFromAMine(int8_t bMineIndex);

// calculate maximum possible daily income from all mines
int32_t CalcMaxPlayerIncomeFromMines(void);

// get index value of this mine in the mine list
int8_t GetMineIndexForSector(int16_t sX, int16_t sY);

// get the index of the mine associated with this town
int8_t GetMineIndexForTown(TownID bTownId);

// get the sector value for the mine associated with this town
int16_t GetMineSectorForTown(TownID bTownId);

// is there a mine here?
BOOLEAN IsThereAMineInThisSector(int16_t sX, int16_t sY);

// Save the mine status to the save game file
BOOLEAN SaveMineStatusToSaveGameFile(FileID hFile);

// Load the mine status from the saved game file
BOOLEAN LoadMineStatusFromSavedGameFile(FileID hFile);

// if the player controls a given mine
BOOLEAN PlayerControlsMine(int8_t bMineIndex);

void ShutOffMineProduction(int8_t bMineIndex);
void RestartMineProduction(int8_t bMineIndex);
void MineShutdownIsPermanent(int8_t bMineIndex);

BOOLEAN IsMineShutDown(int8_t bMineIndex);

uint8_t GetHeadMinerIndexForMine(int8_t bMineIndex);
uint16_t GetHeadMinerProfileIdForMine(int8_t bMineIndex);

// Find the sector location of a mine
void GetMineSector(uint8_t ubMineIndex, uint8_t* psX, uint8_t* psY);

void IssueHeadMinerQuote(int8_t bMineIndex, uint8_t ubQuoteType);

uint8_t GetHeadMinersMineIndex(uint8_t ubMinerProfileId);

void PlayerSpokeToHeadMiner(uint8_t ubMinerProfile);

BOOLEAN IsHisMineRunningOut(uint8_t ubMinerProfileId);
BOOLEAN IsHisMineEmpty(uint8_t ubMinerProfileId);
BOOLEAN IsHisMineDisloyal(uint8_t ubMinerProfileId);
BOOLEAN IsHisMineInfested(uint8_t ubMinerProfileId);
BOOLEAN IsHisMineLostAndRegained(uint8_t ubMinerProfileId);
BOOLEAN IsHisMineAtMaxProduction(uint8_t ubMinerProfileId);
void ResetQueenRetookMine(uint8_t ubMinerProfileId);

void QueenHasRegainedMineSector(int8_t bMineIndex);

BOOLEAN HasAnyMineBeenAttackedByMonsters(void);

void PlayerAttackedHeadMiner(uint8_t ubMinerProfileId);

BOOLEAN HasHisMineBeenProducingForPlayerForSomeTime(uint8_t ubMinerProfileId);

// given sector value, get mine id value
int8_t GetIdOfMineForSector(uint8_t sSectorX, uint8_t sSectorY, int8_t bSectorZ);

// use this for miner (civilian) quotes when *underground* in a mine
BOOLEAN PlayerForgotToTakeOverMine(uint8_t ubMineIndex);

// use this to determine whether or not to place miners into a underground mine level
BOOLEAN AreThereMinersInsideThisMine(uint8_t ubMineIndex);

// use this to determine whether or not the player has spoken to a head miner
BOOLEAN SpokenToHeadMiner(uint8_t ubMineIndex);

#endif
