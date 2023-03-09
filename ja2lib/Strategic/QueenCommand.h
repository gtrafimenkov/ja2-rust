#ifndef __QUEEN_COMMAND_H
#define __QUEEN_COMMAND_H

#include "SGP/Types.h"
#include "Strategic/CampaignTypes.h"

struct GROUP;
struct SOLDIERTYPE;

extern BOOLEAN gfPendingEnemies;

UINT8 NumFreeEnemySlots();

UINT8 NumEnemiesInAnySector(u8 sSectorX, u8 sSectorY, i8 sSectorZ);

UINT8 NumEnemiesInSector(u8 sSectorX, u8 sSectorY);
UINT8 NumStationaryEnemiesInSector(u8 sSectorX, u8 sSectorY);
UINT8 NumMobileEnemiesInSector(u8 sSectorX, u8 sSectorY);
void GetNumberOfMobileEnemiesInSector(u8 sSectorX, u8 sSectorY, UINT8 *pubNumAdmins,
                                      UINT8 *pubNumTroops, UINT8 *pubNumElites);
void GetNumberOfStationaryEnemiesInSector(u8 sSectorX, u8 sSectorY, UINT8 *pubNumAdmins,
                                          UINT8 *pubNumTroops, UINT8 *pubNumElites);
void GetNumberOfEnemiesInSector(u8 sSectorX, u8 sSectorY, UINT8 *pubNumAdmins, UINT8 *pubNumTroops,
                                UINT8 *pubNumElites);

// Called when entering a sector so the campaign AI can automatically insert the
// correct number of troops of each type based on the current number in the sector
// in global focus (gWorldSectorX/Y)
BOOLEAN PrepareEnemyForSectorBattle();
BOOLEAN PrepareEnemyForUndergroundBattle();

void AddEnemiesToBattle(struct GROUP *pGroup, UINT8 ubStrategicInsertionCode, UINT8 ubNumAdmins,
                        UINT8 ubNumTroops, UINT8 ubNumElites, BOOLEAN fMagicallyAppeared);
void AddPossiblePendingEnemiesToBattle();
void EndTacticalBattleForEnemy();

void ProcessQueenCmdImplicationsOfDeath(struct SOLDIERTYPE *pSoldier);

void HandleEnemyStatusInCurrentMapBeforeLoadingNewMap();
BOOLEAN SaveUnderGroundSectorInfoToSaveGame(HWFILE hFile);
BOOLEAN LoadUnderGroundSectorInfoFromSavedGame(HWFILE hFile);

// Finds and returns the specified underground structure ( DONT MODIFY IT ).  Else returns NULL
UNDERGROUND_SECTORINFO *FindUnderGroundSector(INT16 sMapX, INT16 sMapY, UINT8 bMapZ);

void EnemyCapturesPlayerSoldier(struct SOLDIERTYPE *pSoldier);
void BeginCaptureSquence();
void EndCaptureSequence();

BOOLEAN PlayerSectorDefended(UINT8 ubSectorID);

BOOLEAN OnlyHostileCivsInSector();

extern INT16 gsInterrogationGridNo[3];

#endif
