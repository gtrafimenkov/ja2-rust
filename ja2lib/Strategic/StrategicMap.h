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
extern INT16 gWorldSectorX;
extern INT16 gWorldSectorY;
extern INT8 gbWorldSectorZ;

extern BOOLEAN fFoundOrta;

extern BOOLEAN gfUseAlternateMap;

// FUNCTIONS FOR DERTERMINING GOOD GetSectorID8 EXIT DATA
#define CHECK_DIR_X_DELTA (WORLD_TILE_X * 4)
#define CHECK_DIR_Y_DELTA (WORLD_TILE_Y * 10)

void GetCurrentWorldSector(u8 *psMapX, u8 *psMapY);
BOOLEAN SetCurrentWorldSector(u8 sMapX, u8 sMapY, i8 bMapZ);

BOOLEAN EnterSector(u8 sSectorX, u8 sSectorY, INT8 bSectorZ);
void UpdateMercsInSector(u8 sSectorX, u8 sSectorY, INT8 bSectorZ);
void UpdateMercInSector(struct SOLDIERTYPE *pSoldier, u8 sSectorX, u8 sSectorY, INT8 bSectorZ);

// get short sector name without town name
void GetShortSectorString(u8 sMapX, u8 sMapY, STR16 sString, size_t bufSize);

// NEW!
// Calculates the name of the sector based on the loaded sector values.
// Examples:		A9
//						A10_B1
//						J9_B2_A ( >= BETAVERSION ) else J9_B2 (release
// equivalent)
void GetLoadedSectorString(STR16 pString, size_t bufSize);

void GetMapFileName(u8 sMapX, u8 sMapY, INT8 bSectorZ, STR8 bString, BOOLEAN fUsePlaceholder,
                    BOOLEAN fAddAlternateMapLetter);

// Called from within tactical.....
void JumpIntoAdjacentSector(UINT8 ubDirection, UINT8 ubJumpCode, INT16 sAdditionalData);

BOOLEAN CanGoToTacticalInSector(INT16 sX, INT16 sY, UINT8 ubZ);

void UpdateAirspaceControl(void);

// init sam sites
void InitializeSAMSites(void);

// get town sector size
UINT8 GetTownSectorSize(TownID bTownId);

// get town sector size, that is under play control
UINT8 GetTownSectorsUnderControl(TownID bTownId);

BOOLEAN SoldierOKForSectorExit(struct SOLDIERTYPE *pSoldier, INT8 bExitDirection,
                               UINT16 usAdditionalData);
BOOLEAN OKForSectorExit(INT8 bExitDirection, UINT16 usAdditionalData,
                        UINT32 *puiTraverseTimeInMinutes);
void SetupNewStrategicGame();

BOOLEAN LoadStrategicInfoFromSavedFile(FileID hFile);
BOOLEAN SaveStrategicInfoToSavedFile(FileID hFile);

void AllMercsHaveWalkedOffSector();

void AdjustSoldierPathToGoOffEdge(struct SOLDIERTYPE *pSoldier, INT16 sEndGridNo,
                                  UINT8 ubTacticalDirection);

void AllMercsWalkedToExitGrid();

void PrepareLoadedSector();

// handle for slay...no better place to really put this stuff
void HandleSlayDailyEvent(void);

void HandleQuestCodeOnSectorEntry(u8 sNewSectorX, u8 sNewSectorY, INT8 bNewSectorZ);

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
extern UINT8 gubLoneMercAttemptingToAbandonEPCs;
extern INT8 gbPotentiallyAbandonedEPCSlotID;

#endif
