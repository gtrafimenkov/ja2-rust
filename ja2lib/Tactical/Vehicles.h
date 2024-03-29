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

// extern STR16 sCritLocationStrings[];

// extern INT8 bInternalCritHitsByLocation[NUMBER_OF_EXTERNAL_HIT_LOCATIONS_ON_VEHICLE][
// NUMBER_OF_INTERNAL_HIT_LOCATIONS_IN_VEHICLE ];

extern INT16 sVehicleOrigArmorValues[NUMBER_OF_TYPES_OF_VEHICLES]
                                    [NUMBER_OF_INTERNAL_HIT_LOCATIONS_IN_VEHICLE];

// struct for vehicles
typedef struct {
  struct path *pMercPath;  // vehicle's stategic path list
  UINT8 ubMovementGroup;   // the movement group this vehicle belongs to
  UINT8 ubVehicleType;     // type of vehicle
  INT16 sSectorX;          // X position on the Stategic Map
  INT16 sSectorY;          // Y position on the Stategic Map
  INT16 sSectorZ;
  BOOLEAN fBetweenSectors;  // between sectors?
  INT16 sGridNo;            // location in tactical
  struct SOLDIERTYPE *pPassengers[10];
  UINT8 ubDriver;
  INT16 sInternalHitLocations[NUMBER_OF_EXTERNAL_HIT_LOCATIONS_ON_VEHICLE];
  INT16 sArmourType;
  INT16 sExternalArmorLocationsStatus[NUMBER_OF_EXTERNAL_HIT_LOCATIONS_ON_VEHICLE];
  INT16 sCriticalHits[NUMBER_OF_INTERNAL_HIT_LOCATIONS_IN_VEHICLE];
  INT32 iOnSound;
  INT32 iOffSound;
  INT32 iMoveSound;
  INT32 iOutOfSound;
  BOOLEAN fFunctional;
  BOOLEAN fDestroyed;
  INT32 iMovementSoundID;
  UINT8 ubProfileID;

  BOOLEAN fValid;

} VEHICLETYPE;

// the list of vehicles
extern VEHICLETYPE *pVehicleList;

// number of vehicles on the list
extern UINT8 ubNumberOfVehicles;

extern INT32 iMvtTypes[];

void SetVehicleValuesIntoSoldierType(struct SOLDIERTYPE *pVehicle);

// add vehicle to list and return id value
INT32 AddVehicleToList(u8 sMapX, u8 sMapY, INT16 sGridNo, UINT8 ubType);

// remove this vehicle from the list
BOOLEAN RemoveVehicleFromList(INT32 iId);

// clear out the vehicle list
void ClearOutVehicleList(void);

BOOLEAN AnyAccessibleVehiclesInSoldiersSector(struct SOLDIERTYPE *pSoldier);

// is this vehicle in the same sector (not between sectors), and accesible
BOOLEAN IsThisVehicleAccessibleToSoldier(struct SOLDIERTYPE *pSoldier, INT32 iId);

// add soldier to Vehicle
BOOLEAN AddSoldierToVehicle(struct SOLDIERTYPE *pSoldier, INT32 iId);

// remove soldier from vehicle
BOOLEAN RemoveSoldierFromVehicle(struct SOLDIERTYPE *pSoldier, INT32 iId);

// strategic mvt stuff
// move character path to the vehicle
BOOLEAN MoveCharactersPathToVehicle(struct SOLDIERTYPE *pSoldier);

// travel time at the startegic level
INT32 GetTravelTimeOfVehicle(INT32 iId);

// is this vehicle a valid one?
BOOLEAN VehicleIdIsValid(INT32 iId);

// set up vehicle mvt for this grunt involved
BOOLEAN SetUpMvtGroupForVehicle(struct SOLDIERTYPE *pSoldier);

// set up soldier mvt for vehicle
BOOLEAN CopyVehiclePathToSoldier(struct SOLDIERTYPE *pSoldier);

// update mercs position when vehicle arrives
void UpdatePositionOfMercsInVehicle(INT32 iId);

// find vehicle id of group with this vehicle
INT32 GivenMvtGroupIdFindVehicleId(UINT8 ubGroupId);

// given vehicle id, add all peopel in vehicle to mvt group, after clearing mvt group out
BOOLEAN AddVehicleMembersToMvtGroup(INT32 iId);

// injure this person in the vehicle
BOOLEAN InjurePersonInVehicle(INT32 iId, struct SOLDIERTYPE *pSoldier, UINT8 ubPointsOfDmg);

// kill this person in the vehicle
BOOLEAN KillPersonInVehicle(INT32 iId, struct SOLDIERTYPE *pSoldier);

// kill everyone in vehicle
BOOLEAN KillAllInVehicle(INT32 iId);

#ifdef JA2TESTVERSION
// test vehicle stuff
void VehicleTest(void);
#endif

// grab number of occupants in vehicles
INT32 GetNumberInVehicle(INT32 iId);

// grab # in vehicle skipping EPCs (who aren't allowed to drive :-)
INT32 GetNumberOfNonEPCsInVehicle(INT32 iId);

BOOLEAN EnterVehicle(struct SOLDIERTYPE *pVehicle, struct SOLDIERTYPE *pSoldier);

struct SOLDIERTYPE *GetDriver(INT32 iID);

void SetVehicleName(struct SOLDIERTYPE *pVehicle);

BOOLEAN ExitVehicle(struct SOLDIERTYPE *pSoldier);

void AddPassangersToTeamPanel(INT32 iId);

void VehicleTakeDamage(UINT8 ubID, UINT8 ubReason, INT16 sDamage, INT16 sGridNo,
                       UINT8 ubAttackerID);

// the soldiertype containing this tactical incarnation of this vehicle
struct SOLDIERTYPE *GetSoldierStructureForVehicle(INT32 iId);

void AdjustVehicleAPs(struct SOLDIERTYPE *pSoldier, UINT8 *pubPoints);

// get orig armor values for vehicle in this location
// INT16 GetOrigInternalArmorValueForVehicleInLocation( UINT8 ubID, UINT8 ubLocation );

// handle crit hit to vehicle in this location
void HandleCriticalHitForVehicleInLocation(UINT8 ubID, INT16 sDmg, INT16 sGridNo,
                                           UINT8 ubAttackerID);

// ste up armor values for this vehicle
void SetUpArmorForVehicle(UINT8 ubID);

// does it need fixing?
BOOLEAN DoesVehicleNeedAnyRepairs(INT32 iVehicleId);

// repair the vehicle
INT8 RepairVehicle(INT32 iVehicleId, INT8 bTotalPts, BOOLEAN *pfNothingToRepair);

// Save all the vehicle information to the saved game file
BOOLEAN SaveVehicleInformationToSaveGameFile(FileID hFile);

// Load all the vehicle information From the saved game file
BOOLEAN LoadVehicleInformationFromSavedGameFile(FileID hFile, UINT32 uiSavedGameVersion);

// take soldier out of vehicle
BOOLEAN TakeSoldierOutOfVehicle(struct SOLDIERTYPE *pSoldier);

// put soldier in vehicle
BOOLEAN PutSoldierInVehicle(struct SOLDIERTYPE *pSoldier, INT8 bVehicleId);

void SetVehicleSectorValues(INT32 iVehId, UINT8 ubSectorX, UINT8 ubSectorY);

void UpdateAllVehiclePassengersGridNo(struct SOLDIERTYPE *pSoldier);

BOOLEAN SaveVehicleMovementInfoToSavedGameFile(FileID hFile);
BOOLEAN LoadVehicleMovementInfoFromSavedGameFile(FileID hFile);
BOOLEAN NewSaveVehicleMovementInfoToSavedGameFile(FileID hFile);
BOOLEAN NewLoadVehicleMovementInfoFromSavedGameFile(FileID hFile);

BOOLEAN OKUseVehicle(UINT8 ubProfile);

BOOLEAN IsRobotControllerInVehicle(INT32 iId);

void AddVehicleFuelToSave();

BOOLEAN CanSoldierDriveVehicle(struct SOLDIERTYPE *pSoldier, INT32 iVehicleId,
                               BOOLEAN fIgnoreAsleep);
BOOLEAN SoldierMustDriveVehicle(struct SOLDIERTYPE *pSoldier, INT32 iVehicleId,
                                BOOLEAN fTryingToTravel);
BOOLEAN OnlyThisSoldierCanDriveVehicle(struct SOLDIERTYPE *pSoldier, INT32 iVehicleId);

BOOLEAN IsEnoughSpaceInVehicle(INT32 iID);

BOOLEAN IsSoldierInThisVehicleSquad(struct SOLDIERTYPE *pSoldier, INT8 bSquadNumber);

struct SOLDIERTYPE *PickRandomPassengerFromVehicle(struct SOLDIERTYPE *pSoldier);

BOOLEAN DoesVehicleHaveAnyPassengers(INT32 iVehicleID);
BOOLEAN DoesVehicleGroupHaveAnyPassengers(struct GROUP *pGroup);

void SetSoldierExitVehicleInsertionData(struct SOLDIERTYPE *pSoldier, INT32 iId);

#endif
