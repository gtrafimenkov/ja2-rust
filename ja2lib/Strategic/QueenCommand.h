#ifndef __QUEEN_COMMAND_H
#define __QUEEN_COMMAND_H

#include "SGP/Types.h"
#include "Strategic/CampaignTypes.h"

struct GROUP;
struct SOLDIERTYPE;

extern BOOLEAN gfPendingEnemies;

uint8_t NumFreeEnemySlots();

uint8_t NumEnemiesInAnySector(uint8_t sSectorX, uint8_t sSectorY, int16_t sSectorZ);

uint8_t NumEnemiesInSector(uint8_t sSectorX, uint8_t sSectorY);
uint8_t NumStationaryEnemiesInSector(uint8_t sSectorX, uint8_t sSectorY);
uint8_t NumMobileEnemiesInSector(uint8_t sSectorX, uint8_t sSectorY);
void GetNumberOfMobileEnemiesInSector(uint8_t sSectorX, uint8_t sSectorY, uint8_t *pubNumAdmins,
                                      uint8_t *pubNumTroops, uint8_t *pubNumElites);
void GetNumberOfStationaryEnemiesInSector(uint8_t sSectorX, uint8_t sSectorY, uint8_t *pubNumAdmins,
                                          uint8_t *pubNumTroops, uint8_t *pubNumElites);
void GetNumberOfEnemiesInSector(uint8_t sSectorX, uint8_t sSectorY, uint8_t *pubNumAdmins,
                                uint8_t *pubNumTroops, uint8_t *pubNumElites);

// Called when entering a sector so the campaign AI can automatically insert the
// correct number of troops of each type based on the current number in the sector
// in global focus (gWorldSectorX/Y)
BOOLEAN PrepareEnemyForSectorBattle();
BOOLEAN PrepareEnemyForUndergroundBattle();

void AddEnemiesToBattle(struct GROUP *pGroup, uint8_t ubStrategicInsertionCode, uint8_t ubNumAdmins,
                        uint8_t ubNumTroops, uint8_t ubNumElites, BOOLEAN fMagicallyAppeared);
void AddPossiblePendingEnemiesToBattle();
void EndTacticalBattleForEnemy();

void ProcessQueenCmdImplicationsOfDeath(struct SOLDIERTYPE *pSoldier);

void HandleEnemyStatusInCurrentMapBeforeLoadingNewMap();
BOOLEAN SaveUnderGroundSectorInfoToSaveGame(HWFILE hFile);
BOOLEAN LoadUnderGroundSectorInfoFromSavedGame(HWFILE hFile);

// Finds and returns the specified underground structure ( DONT MODIFY IT ).  Else returns NULL
UNDERGROUND_SECTORINFO *FindUnderGroundSector(int16_t sMapX, int16_t sMapY, uint8_t bMapZ);

void EnemyCapturesPlayerSoldier(struct SOLDIERTYPE *pSoldier);
void BeginCaptureSquence();
void EndCaptureSequence();

BOOLEAN PlayerSectorDefended(uint8_t ubSectorID);

BOOLEAN OnlyHostileCivsInSector();

extern int16_t gsInterrogationGridNo[3];

#endif
