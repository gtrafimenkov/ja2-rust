#ifndef _VEHICLES_H
#define _VEHICLES_H

#include "SGP/Types.h"

struct GROUP;
struct SOLDIERTYPE;

#define MAX_VEHICLES 10

// type of vehicles
enum {
  ELDORADO_CAR = 0,
  HUMMER,
  ICE_CREAM_TRUCK,
  JEEP_CAR,
  TANK_CAR,
  HELICOPTER,
  NUMBER_OF_TYPES_OF_VEHICLES,
};

// external armor hit locations
enum {
  FRONT_EXTERNAL_HIT_LOCATION,
  LEFT_EXTERNAL_HIT_LOCATION,
  RIGHT_EXTERNAL_HIT_LOCATION,
  REAR_EXTERNAL_HIT_LOCATION,
  BOTTOM_EXTERNAL_HIT_LOCATION,
  TOP_EXTERNAL_HIT_LOCATION,
  NUMBER_OF_EXTERNAL_HIT_LOCATIONS_ON_VEHICLE,
};

// internal critical hit locations
enum {
  ENGINE_HIT_LOCATION,
  CREW_COMPARTMENT_HIT_LOCATION,
  RF_TIRE_HIT_LOCATION,
  LF_TIRE_HIT_LOCATION,
  RR_TIRE_HIT_LOCATION,
  LR_TIRE_HIT_LOCATION,
  GAS_TANK_HIT_LOCATION,
  NUMBER_OF_INTERNAL_HIT_LOCATIONS_IN_VEHICLE,
};

// extern wchar_t* sCritLocationStrings[];

// extern int8_t bInternalCritHitsByLocation[NUMBER_OF_EXTERNAL_HIT_LOCATIONS_ON_VEHICLE][
// NUMBER_OF_INTERNAL_HIT_LOCATIONS_IN_VEHICLE ];

extern int16_t sVehicleOrigArmorValues[NUMBER_OF_TYPES_OF_VEHICLES]
                                      [NUMBER_OF_INTERNAL_HIT_LOCATIONS_IN_VEHICLE];

// struct for vehicles
typedef struct {
  struct path *pMercPath;   // vehicle's stategic path list
  uint8_t ubMovementGroup;  // the movement group this vehicle belongs to
  uint8_t ubVehicleType;    // type of vehicle
  int16_t sSectorX;         // X position on the Stategic Map
  int16_t sSectorY;         // Y position on the Stategic Map
  int16_t sSectorZ;
  BOOLEAN fBetweenSectors;  // between sectors?
  int16_t sGridNo;          // location in tactical
  struct SOLDIERTYPE *pPassengers[10];
  uint8_t ubDriver;
  int16_t sInternalHitLocations[NUMBER_OF_EXTERNAL_HIT_LOCATIONS_ON_VEHICLE];
  int16_t sArmourType;
  int16_t sExternalArmorLocationsStatus[NUMBER_OF_EXTERNAL_HIT_LOCATIONS_ON_VEHICLE];
  int16_t sCriticalHits[NUMBER_OF_INTERNAL_HIT_LOCATIONS_IN_VEHICLE];
  int32_t iOnSound;
  int32_t iOffSound;
  int32_t iMoveSound;
  int32_t iOutOfSound;
  BOOLEAN fFunctional;
  BOOLEAN fDestroyed;
  int32_t iMovementSoundID;
  uint8_t ubProfileID;

  BOOLEAN fValid;

} VEHICLETYPE;

// the list of vehicles
extern VEHICLETYPE *pVehicleList;

// number of vehicles on the list
extern uint8_t ubNumberOfVehicles;

extern int32_t iMvtTypes[];

void SetVehicleValuesIntoSoldierType(struct SOLDIERTYPE *pVehicle);

// add vehicle to list and return id value
int32_t AddVehicleToList(uint8_t sMapX, uint8_t sMapY, int16_t sGridNo, uint8_t ubType);

// remove this vehicle from the list
BOOLEAN RemoveVehicleFromList(int32_t iId);

// clear out the vehicle list
void ClearOutVehicleList(void);

BOOLEAN AnyAccessibleVehiclesInSoldiersSector(struct SOLDIERTYPE *pSoldier);

// is this vehicle in the same sector (not between sectors), and accesible
BOOLEAN IsThisVehicleAccessibleToSoldier(struct SOLDIERTYPE *pSoldier, int32_t iId);

// add soldier to Vehicle
BOOLEAN AddSoldierToVehicle(struct SOLDIERTYPE *pSoldier, int32_t iId);

// remove soldier from vehicle
BOOLEAN RemoveSoldierFromVehicle(struct SOLDIERTYPE *pSoldier, int32_t iId);

// strategic mvt stuff
// move character path to the vehicle
BOOLEAN MoveCharactersPathToVehicle(struct SOLDIERTYPE *pSoldier);

// travel time at the startegic level
int32_t GetTravelTimeOfVehicle(int32_t iId);

// is this vehicle a valid one?
BOOLEAN VehicleIdIsValid(int32_t iId);

// set up vehicle mvt for this grunt involved
BOOLEAN SetUpMvtGroupForVehicle(struct SOLDIERTYPE *pSoldier);

// set up soldier mvt for vehicle
BOOLEAN CopyVehiclePathToSoldier(struct SOLDIERTYPE *pSoldier);

// update mercs position when vehicle arrives
void UpdatePositionOfMercsInVehicle(int32_t iId);

// find vehicle id of group with this vehicle
int32_t GivenMvtGroupIdFindVehicleId(uint8_t ubGroupId);

// given vehicle id, add all peopel in vehicle to mvt group, after clearing mvt group out
BOOLEAN AddVehicleMembersToMvtGroup(int32_t iId);

// injure this person in the vehicle
BOOLEAN InjurePersonInVehicle(int32_t iId, struct SOLDIERTYPE *pSoldier, uint8_t ubPointsOfDmg);

// kill this person in the vehicle
BOOLEAN KillPersonInVehicle(int32_t iId, struct SOLDIERTYPE *pSoldier);

// kill everyone in vehicle
BOOLEAN KillAllInVehicle(int32_t iId);

#ifdef JA2TESTVERSION
// test vehicle stuff
void VehicleTest(void);
#endif

// grab number of occupants in vehicles
int32_t GetNumberInVehicle(int32_t iId);

// grab # in vehicle skipping EPCs (who aren't allowed to drive :-)
int32_t GetNumberOfNonEPCsInVehicle(int32_t iId);

BOOLEAN EnterVehicle(struct SOLDIERTYPE *pVehicle, struct SOLDIERTYPE *pSoldier);

struct SOLDIERTYPE *GetDriver(int32_t iID);

void SetVehicleName(struct SOLDIERTYPE *pVehicle);

BOOLEAN ExitVehicle(struct SOLDIERTYPE *pSoldier);

void AddPassangersToTeamPanel(int32_t iId);

void VehicleTakeDamage(uint8_t ubID, uint8_t ubReason, int16_t sDamage, int16_t sGridNo,
                       uint8_t ubAttackerID);

// the soldiertype containing this tactical incarnation of this vehicle
struct SOLDIERTYPE *GetSoldierStructureForVehicle(int32_t iId);

void AdjustVehicleAPs(struct SOLDIERTYPE *pSoldier, uint8_t *pubPoints);

// get orig armor values for vehicle in this location
// int16_t GetOrigInternalArmorValueForVehicleInLocation( uint8_t ubID, uint8_t ubLocation );

// handle crit hit to vehicle in this location
void HandleCriticalHitForVehicleInLocation(uint8_t ubID, int16_t sDmg, int16_t sGridNo,
                                           uint8_t ubAttackerID);

// ste up armor values for this vehicle
void SetUpArmorForVehicle(uint8_t ubID);

// does it need fixing?
BOOLEAN DoesVehicleNeedAnyRepairs(int32_t iVehicleId);

// repair the vehicle
int8_t RepairVehicle(int32_t iVehicleId, int8_t bTotalPts, BOOLEAN *pfNothingToRepair);

// Save all the vehicle information to the saved game file
BOOLEAN SaveVehicleInformationToSaveGameFile(HWFILE hFile);

// Load all the vehicle information From the saved game file
BOOLEAN LoadVehicleInformationFromSavedGameFile(HWFILE hFile, uint32_t uiSavedGameVersion);

// take soldier out of vehicle
BOOLEAN TakeSoldierOutOfVehicle(struct SOLDIERTYPE *pSoldier);

// put soldier in vehicle
BOOLEAN PutSoldierInVehicle(struct SOLDIERTYPE *pSoldier, int8_t bVehicleId);

void SetVehicleSectorValues(int32_t iVehId, uint8_t ubSectorX, uint8_t ubSectorY);

void UpdateAllVehiclePassengersGridNo(struct SOLDIERTYPE *pSoldier);

BOOLEAN SaveVehicleMovementInfoToSavedGameFile(HWFILE hFile);
BOOLEAN LoadVehicleMovementInfoFromSavedGameFile(HWFILE hFile);
BOOLEAN NewSaveVehicleMovementInfoToSavedGameFile(HWFILE hFile);
BOOLEAN NewLoadVehicleMovementInfoFromSavedGameFile(HWFILE hFile);

BOOLEAN OKUseVehicle(uint8_t ubProfile);

BOOLEAN IsRobotControllerInVehicle(int32_t iId);

void AddVehicleFuelToSave();

BOOLEAN CanSoldierDriveVehicle(struct SOLDIERTYPE *pSoldier, int32_t iVehicleId,
                               BOOLEAN fIgnoreAsleep);
BOOLEAN SoldierMustDriveVehicle(struct SOLDIERTYPE *pSoldier, int32_t iVehicleId,
                                BOOLEAN fTryingToTravel);
BOOLEAN OnlyThisSoldierCanDriveVehicle(struct SOLDIERTYPE *pSoldier, int32_t iVehicleId);

BOOLEAN IsEnoughSpaceInVehicle(int32_t iID);

BOOLEAN IsSoldierInThisVehicleSquad(struct SOLDIERTYPE *pSoldier, int8_t bSquadNumber);

struct SOLDIERTYPE *PickRandomPassengerFromVehicle(struct SOLDIERTYPE *pSoldier);

BOOLEAN DoesVehicleHaveAnyPassengers(int32_t iVehicleID);
BOOLEAN DoesVehicleGroupHaveAnyPassengers(struct GROUP *pGroup);

void SetSoldierExitVehicleInsertionData(struct SOLDIERTYPE *pSoldier, int32_t iId);

#endif
