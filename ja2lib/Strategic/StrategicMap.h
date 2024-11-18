#ifndef __STRATMAP_H
#define __STRATMAP_H

#include "SGP/Types.h"
#include "Town.h"

struct SOLDIERTYPE;

// The maximum size for any team strategically speaking.  For example, we can't have more than 20
// enemies, militia, or creatures at a time.
#define MAX_STRATEGIC_TEAM_SIZE 20

// Codes for jumoing into adjacent sectors..
enum { JUMP_ALL_LOAD_NEW, JUMP_ALL_NO_LOAD, JUMP_SINGLE_LOAD_NEW, JUMP_SINGLE_NO_LOAD };

enum {
  CONTROLLED = 0,
  UNCONTROLLED,
};

// For speed, etc lets make these globals, forget the functions if you want
extern int16_t gWorldSectorX;
extern int16_t gWorldSectorY;
extern int8_t gbWorldSectorZ;

extern BOOLEAN fFoundOrta;

extern BOOLEAN gfUseAlternateMap;

// FUNCTIONS FOR DERTERMINING GOOD GetSectorID8 EXIT DATA
#define CHECK_DIR_X_DELTA (WORLD_TILE_X * 4)
#define CHECK_DIR_Y_DELTA (WORLD_TILE_Y * 10)

void GetCurrentWorldSector(u8 *psMapX, u8 *psMapY);
BOOLEAN SetCurrentWorldSector(u8 sMapX, u8 sMapY, i8 bMapZ);

BOOLEAN EnterSector(u8 sSectorX, u8 sSectorY, int8_t bSectorZ);
void UpdateMercsInSector(u8 sSectorX, u8 sSectorY, int8_t bSectorZ);
void UpdateMercInSector(struct SOLDIERTYPE *pSoldier, u8 sSectorX, u8 sSectorY, int8_t bSectorZ);

// get short sector name without town name
void GetShortSectorString(u8 sMapX, u8 sMapY, wchar_t* sString, size_t bufSize);

// NEW!
// Calculates the name of the sector based on the loaded sector values.
// Examples:		A9
//						A10_B1
//						J9_B2_A ( >= BETAVERSION ) else J9_B2 (release
// equivalent)
void GetLoadedSectorString(wchar_t* pString, size_t bufSize);

void GetMapFileName(u8 sMapX, u8 sMapY, int8_t bSectorZ, char* bString, BOOLEAN fUsePlaceholder,
                    BOOLEAN fAddAlternateMapLetter);

// Called from within tactical.....
void JumpIntoAdjacentSector(uint8_t ubDirection, uint8_t ubJumpCode, int16_t sAdditionalData);

BOOLEAN CanGoToTacticalInSector(int16_t sX, int16_t sY, uint8_t ubZ);

void UpdateAirspaceControl(void);

// init sam sites
void InitializeSAMSites(void);

// get town sector size
uint8_t GetTownSectorSize(TownID bTownId);

// get town sector size, that is under play control
uint8_t GetTownSectorsUnderControl(TownID bTownId);

BOOLEAN SoldierOKForSectorExit(struct SOLDIERTYPE *pSoldier, int8_t bExitDirection,
                               uint16_t usAdditionalData);
BOOLEAN OKForSectorExit(int8_t bExitDirection, uint16_t usAdditionalData,
                        uint32_t *puiTraverseTimeInMinutes);
void SetupNewStrategicGame();

BOOLEAN LoadStrategicInfoFromSavedFile(FileID hFile);
BOOLEAN SaveStrategicInfoToSavedFile(FileID hFile);

void AllMercsHaveWalkedOffSector();

void AdjustSoldierPathToGoOffEdge(struct SOLDIERTYPE *pSoldier, int16_t sEndGridNo,
                                  uint8_t ubTacticalDirection);

void AllMercsWalkedToExitGrid();

void PrepareLoadedSector();

// handle for slay...no better place to really put this stuff
void HandleSlayDailyEvent(void);

void HandleQuestCodeOnSectorEntry(u8 sNewSectorX, u8 sNewSectorY, int8_t bNewSectorZ);

// handle a soldier leaving thier squad behind, this sets them up for mvt and potential rejoining of
// group
void HandleSoldierLeavingSectorByThemSelf(struct SOLDIERTYPE *pSoldier);

BOOLEAN CheckAndHandleUnloadingOfCurrentWorld();

BOOLEAN IsSectorDesert(u8 sSectorX, u8 sSectorY);

void SetupProfileInsertionDataForSoldier(struct SOLDIERTYPE *pSoldier);

BOOLEAN HandlePotentialBringUpAutoresolveToFinishBattle();

// Used for determining the type of error message that comes up when you can't traverse to
// an adjacent sector.  THESE VALUES DO NOT NEED TO BE SAVED!
extern BOOLEAN gfInvalidTraversal;
extern BOOLEAN gfLoneEPCAttemptingTraversal;
extern BOOLEAN gfRobotWithoutControllerAttemptingTraversal;
extern uint8_t gubLoneMercAttemptingToAbandonEPCs;
extern int8_t gbPotentiallyAbandonedEPCSlotID;

#endif
