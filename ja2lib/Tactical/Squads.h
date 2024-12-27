// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#ifndef _SQUADS_H
#define _SQUADS_H

#include "SGP/Types.h"

struct SOLDIERTYPE;

// header for squad management system
#define NUMBER_OF_SOLDIERS_PER_SQUAD 6

// enums for squads
enum {
  FIRST_SQUAD = 0,
  SECOND_SQUAD,
  THIRD_SQUAD,
  FOURTH_SQUAD,
  FIFTH_SQUAD,
  SIXTH_SQUAD,
  SEVENTH_SQUAD,
  EIGTH_SQUAD,
  NINTH_SQUAD,
  TENTH_SQUAD,
  ELEVENTH_SQUAD,
  TWELTH_SQUAD,
  THIRTEENTH_SQUAD,
  FOURTEENTH_SQUAD,
  FIFTHTEEN_SQUAD,
  SIXTEENTH_SQUAD,
  SEVENTEENTH_SQUAD,
  EIGTHTEENTH_SQUAD,
  NINTEENTH_SQUAD,
  TWENTYTH_SQUAD,
  NUMBER_OF_SQUADS,
};

// ATE: Added so we can have no current squad
// happens in we move off sector via tactical, but nobody is left!
#define NO_CURRENT_SQUAD NUMBER_OF_SQUADS

// ptrs to soldier types of squads and their members

// squads
extern struct SOLDIERTYPE *Squad[NUMBER_OF_SQUADS][NUMBER_OF_SOLDIERS_PER_SQUAD];

extern int32_t iCurrentTacticalSquad;

// will initialize the squad lists for game initalization
void InitSquads(void);

// add character to squad
BOOLEAN AddCharacterToSquad(struct SOLDIERTYPE *pCharacter, int8_t bSquadValue);

// find the first slot the guy will fit in, return true if he is in a squad or has been put in one
BOOLEAN AddCharacterToAnySquad(struct SOLDIERTYPE *pCharacter);

// remove character from squads
BOOLEAN RemoveCharacterFromSquads(struct SOLDIERTYPE *pCharacter);

// remove character from a squad
BOOLEAN RemoveCharacterFromASquad(struct SOLDIERTYPE *pCharacter, int8_t bSquadValue);

// check if character is in this squad
BOOLEAN IsCharacterInSquad(struct SOLDIERTYPE *pCharacter, int8_t bSquadValue);

// return which squad character is in, -1 if none found
int8_t SquadCharacterIsIn(struct SOLDIERTYPE *pCharacter);

// what slot is character in in this squad?..-1 if not found in squad
int8_t SlotCharacterIsInSquad(struct SOLDIERTYPE *pCharacter, int8_t bSquadValue);

// return number of people in this squad
int8_t NumberOfPeopleInSquad(int8_t bSquadValue);

int8_t NumberOfNonEPCsInSquad(int8_t bSquadValue);

BOOLEAN IsRobotControllerInSquad(int8_t bSquadValue);

int8_t NumberOfPlayerControllableMercsInSquad(int8_t bSquadValue);

// what sector is the squad currently in?..return if anyone in squad
BOOLEAN SectorSquadIsIn(int8_t bSquadValue, int16_t *sMapX, int16_t *sMapY, int16_t *sMapZ);

// rebuild current squad list
void RebuildCurrentSquad(void);

// copy path of squad to character
BOOLEAN CopyPathOfSquadToCharacter(struct SOLDIERTYPE *pCharacter, int8_t bSquadValue);

// copy path from character back to squad
BOOLEAN CopyPathOfCharacterToSquad(struct SOLDIERTYPE *pCharacter, int8_t bSquadValue);

// what is the id of the current squad?
int32_t CurrentSquad(void);

// add character to unique squad (returns the squad #, -1 if failed)
int8_t AddCharacterToUniqueSquad(struct SOLDIERTYPE *pCharacter);

// is this squad empty?
BOOLEAN SquadIsEmpty(int8_t bSquadValue);

// is this squad in the current tactical sector?
BOOLEAN IsSquadOnCurrentTacticalMap(int32_t iCurrentSquad);

// set this squad as the current tatcical squad
BOOLEAN SetCurrentSquad(int32_t iCurrentSquad, BOOLEAN fForce);

// set default squad in sector
void SetDefaultSquadOnSectorEntry(BOOLEAN fForce);

// get last squad that has active mercs
int32_t GetLastSquadActive(void);

// set squads between sector position
void SetSquadPositionBetweenSectors(uint8_t ubNextX, uint8_t ubNextY, uint8_t ubPrevX,
                                    uint8_t ubPrevY, uint32_t uiTraverseTime, uint32_t uiArriveTime,
                                    uint8_t ubSquadValue);

// get squads between sector positions and times
void GetSquadPosition(uint8_t *ubNextX, uint8_t *ubNextY, uint8_t *ubPrevX, uint8_t *ubPrevY,
                      uint32_t *uiTraverseTime, uint32_t *uiArriveTime, uint8_t ubSquadValue);

void ExamineCurrentSquadLights(void);

// get location of this squad
void GetLocationOfSquad(int16_t *sX, int16_t *sY, int8_t *bZ, int8_t bSquadValue);

// Save the squad information to the saved game file
BOOLEAN SaveSquadInfoToSavedGameFile(FileID hFile);

// Load all the squad info from the saved game file
BOOLEAN LoadSquadInfoFromSavedGameFile(FileID hFile);

// get squad id of first free squad
int8_t GetFirstEmptySquad(void);

// is this squad in the same sector as soldier?
BOOLEAN IsSquadInSector(struct SOLDIERTYPE *pSoldier, uint8_t ubSquad);

// is any merc on squad asleep?
BOOLEAN IsAnyMercOnSquadAsleep(uint8_t ubSquadValue);

// is therea  dead guy here
BOOLEAN IsDeadGuyInThisSquadSlot(int8_t bSlotId, int8_t bSquadValue,
                                 int8_t *bNumberOfDeadGuysSoFar);

// dead soldier was on squad
BOOLEAN SoldierIsDeadAndWasOnSquad(struct SOLDIERTYPE *pSoldier, int8_t bSquadValue);

// now reset the table for these mercs
BOOLEAN ResetDeadSquadMemberList(int32_t iSquadValue);

// this passed  soldier on the current squad int he tactical map
BOOLEAN IsMercOnCurrentSquad(struct SOLDIERTYPE *pSoldier);

// is this squad filled up?
BOOLEAN IsThisSquadFull(int8_t bSquadValue);

// is this squad moving?
BOOLEAN IsThisSquadOnTheMove(int8_t bSquadValue);

// is there a vehicle in this squad?
BOOLEAN DoesVehicleExistInSquad(int8_t bSquadValue);

// re-create any trashed squad movement groups
void CheckSquadMovementGroups(void);

#endif
