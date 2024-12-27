#ifndef _ASSIGNMENTS_H
#define _ASSIGNMENTS_H

// header for assignment manipulation/updating for characters

#include "SGP/Types.h"
#include "Town.h"

struct GROUP;
struct MOUSE_REGION;
struct SOLDIERTYPE;

// this distinguishes whether we're only looking for patients healable THIS HOUR (those that have
// been on their assignment long enough), or those that will be healable EVER (regardless of whether
// they're getting healed during this hour)
#define HEALABLE_EVER 0
#define HEALABLE_THIS_HOUR 1

// merc collapses from fatigue if max breath drops to this.  Can't go any lower!
#define BREATHMAX_ABSOLUTE_MINIMUM 10
#define BREATHMAX_GOTTA_STOP_MOVING 30
#define BREATHMAX_PRETTY_TIRED 50
#define BREATHMAX_CANCEL_COLLAPSE 60
#define BREATHMAX_CANCEL_TIRED 75
#define BREATHMAX_FULLY_RESTED 95

#define VEHICLE_REPAIR_POINTS_DIVISOR 10

// Assignments Defines
enum {
  SQUAD_1 = 0,
  SQUAD_2,
  SQUAD_3,
  SQUAD_4,
  SQUAD_5,
  SQUAD_6,
  SQUAD_7,
  SQUAD_8,
  SQUAD_9,
  SQUAD_10,
  SQUAD_11,
  SQUAD_12,
  SQUAD_13,
  SQUAD_14,
  SQUAD_15,
  SQUAD_16,
  SQUAD_17,
  SQUAD_18,
  SQUAD_19,
  SQUAD_20,
  ON_DUTY,
  DOCTOR,
  PATIENT,
  VEHICLE,
  IN_TRANSIT,
  REPAIR,
  TRAIN_SELF,
  TRAIN_TOWN,
  TRAIN_TEAMMATE,
  TRAIN_BY_OTHER,
  ASSIGNMENT_DEAD,
  ASSIGNMENT_UNCONCIOUS,  // unused
  ASSIGNMENT_POW,
  ASSIGNMENT_HOSPITAL,
  ASSIGNMENT_EMPTY,
};

#define NO_ASSIGNMENT 127  // used when no pSoldier->ubDesiredSquad

// Train stats defines (must match ATTRIB_MENU_ defines, and pAttributeMenuStrings )
enum {
  STRENGTH = 0,
  DEXTERITY,
  AGILITY,
  HEALTH,
  MARKSMANSHIP,
  MEDICAL,
  MECHANICAL,
  LEADERSHIP,
  EXPLOSIVE_ASSIGN,
  NUM_TRAINABLE_STATS
  // NOTE: Wisdom isn't trainable!
};

typedef struct TOWN_TRAINER_TYPE {
  struct SOLDIERTYPE *pSoldier;
  int16_t sTrainingPts;

} TOWN_TRAINER_TYPE;

void ResumeOldAssignment(struct SOLDIERTYPE *pSoldier);

// can character do this assignment?
// BOOLEAN CanSoldierAssignment( struct SOLDIERTYPE *pSoldier, int8_t bAssignment );

// can this character be assigned as a doctor?
BOOLEAN CanCharacterDoctor(struct SOLDIERTYPE *pCharacter);

// can this character be assigned as a repairman?
BOOLEAN CanCharacterRepair(struct SOLDIERTYPE *pCharacter);

// can character be patient?
BOOLEAN CanCharacterPatient(struct SOLDIERTYPE *pCharacter);

// can character train militia?
BOOLEAN CanCharacterTrainMilitia(struct SOLDIERTYPE *pCharacter);

// can character train stat?..as train self or as trainer?
BOOLEAN CanCharacterTrainStat(struct SOLDIERTYPE *pSoldier, int8_t bStat, BOOLEAN fTrainSelf,
                              BOOLEAN fTrainTeammate);

// is character capable of practising at all?
BOOLEAN CanCharacterPractise(struct SOLDIERTYPE *pCharacter);

// can this character train others?
BOOLEAN CanCharacterTrainTeammates(struct SOLDIERTYPE *pSoldier);

// put character on duty?
BOOLEAN CanCharacterOnDuty(struct SOLDIERTYPE *pCharacter);

// put character to sleep?
BOOLEAN CanCharacterSleep(struct SOLDIERTYPE *pCharacter, BOOLEAN fExplainWhyNot);

BOOLEAN CanCharacterBeAwakened(struct SOLDIERTYPE *pSoldier, BOOLEAN fExplainWhyNot);

// put character in vehicle?
BOOLEAN CanCharacterVehicle(struct SOLDIERTYPE *pCharacter);

#define CHARACTER_CANT_JOIN_SQUAD_ALREADY_IN_IT -6
#define CHARACTER_CANT_JOIN_SQUAD_SQUAD_MOVING -5
#define CHARACTER_CANT_JOIN_SQUAD_MOVING -4
#define CHARACTER_CANT_JOIN_SQUAD_VEHICLE -3
#define CHARACTER_CANT_JOIN_SQUAD_TOO_FAR -2
#define CHARACTER_CANT_JOIN_SQUAD_FULL -1
#define CHARACTER_CANT_JOIN_SQUAD 0
#define CHARACTER_CAN_JOIN_SQUAD 1

// can character be added to squad
int8_t CanCharacterSquad(struct SOLDIERTYPE *pCharacter, int8_t bSquadValue);

// if merc could train militia here, do they have sufficient loyalty?
BOOLEAN DoesSectorMercIsInHaveSufficientLoyaltyToTrainMilitia(struct SOLDIERTYPE *pSoldier);
BOOLEAN DoesTownHaveRatingToTrainMilitia(TownID bTownId);

// is the character in transit?
BOOLEAN IsCharacterInTransit(struct SOLDIERTYPE *pCharacter);

// handler for assignments -- called once per hour via event
void UpdateAssignments();

// how many people in this secotr have this assignment?
uint8_t FindNumberInSectorWithAssignment(int16_t sX, int16_t sY, int8_t bAssignment);

void MakeSoldiersTacticalAnimationReflectAssignment(struct SOLDIERTYPE *pSoldier);

// build list of sectors with mercs
void BuildSectorsWithSoldiersList(void);

// init sectors with soldiers list
void InitSectorsWithSoldiersList(void);

// is there a soldier in this sector?..only use after BuildSectorsWithSoldiersList is called
BOOLEAN IsThereASoldierInThisSector(u8 sSectorX, u8 sSectorY, int8_t bSectorZ);

void CheckIfSoldierUnassigned(struct SOLDIERTYPE *pSoldier);

// figure out the assignment menu pop up box positions
void DetermineBoxPositions(void);

// set x,y position in tactical
void SetTacticalPopUpAssignmentBoxXY(void);

// get number of pts that are being used this strategic turn
int16_t GetTownTrainPtsForCharacter(struct SOLDIERTYPE *pTrainer, uint16_t *pusMaxPts);

// find number of healing pts
uint16_t CalculateHealingPointsForDoctor(struct SOLDIERTYPE *pSoldier, uint16_t *pusMaxPts,
                                         BOOLEAN fMakeSureKitIsInHand);

// find number of repair pts repairman has available
uint8_t CalculateRepairPointsForRepairman(struct SOLDIERTYPE *pSoldier, uint16_t *pusMaxPts,
                                          BOOLEAN fMakeSureKitIsInHand);

// get bonus tarining pts due to an instructor for this student
int16_t GetBonusTrainingPtsDueToInstructor(struct SOLDIERTYPE *pInstructor,
                                           struct SOLDIERTYPE *pStudent, int8_t bTrainStat,
                                           BOOLEAN fAtGunRange, uint16_t *pusMaxPts);

// get training pts for this soldier
int16_t GetSoldierTrainingPts(struct SOLDIERTYPE *pSoldier, int8_t bTrainStat, BOOLEAN fAtGunRange,
                              uint16_t *pusMaxPts);

// pts for being a student for this soldier
int16_t GetSoldierStudentPts(struct SOLDIERTYPE *pSoldier, int8_t bTrainStat, BOOLEAN fAtGunRange,
                             uint16_t *pusMaxPts);

// reset these soldiers
void ResetAssignmentsForAllSoldiersInSectorWhoAreTrainingTown(struct SOLDIERTYPE *pSoldier);

// Handle assignment done
void AssignmentDone(struct SOLDIERTYPE *pSoldier, BOOLEAN fSayQuote, BOOLEAN fMeToo);

extern INT32 ghAssignmentBox;
extern INT32 ghEpcBox;
extern INT32 ghSquadBox;
extern INT32 ghVehicleBox;
extern INT32 ghRepairBox;
extern INT32 ghTrainingBox;
extern INT32 ghAttributeBox;
extern INT32 ghRemoveMercAssignBox;
extern INT32 ghContractBox;
extern INT32 ghMoveBox;
// extern INT32 ghUpdateBox;

extern BOOLEAN fShownContractMenu;
extern BOOLEAN fShownAssignmentMenu;
extern BOOLEAN fShowRepairMenu;

extern BOOLEAN fFirstClickInAssignmentScreenMask;

extern void RestorePopUpBoxes(void);

extern BOOLEAN fGlowContractRegion;

extern BOOLEAN gfReEvaluateEveryonesNothingToDo;

// pop up menu mouse regions
void CreateDestroyMouseRegionsForAssignmentMenu(void);
void AssignmentMenuMvtCallBack(struct MOUSE_REGION *pRegion, INT32 iReason);
void AssignmentMenuBtnCallback(struct MOUSE_REGION *pRegion, INT32 iReason);

void CreateDestroyMouseRegionsForTrainingMenu(void);
void TrainingMenuMvtCallBack(struct MOUSE_REGION *pRegion, INT32 iReason);
void TrainingMenuBtnCallback(struct MOUSE_REGION *pRegion, INT32 iReason);

void CreateDestroyMouseRegionsForAttributeMenu(void);
void AttributeMenuMvtCallBack(struct MOUSE_REGION *pRegion, INT32 iReason);
void AttributesMenuBtnCallback(struct MOUSE_REGION *pRegion, INT32 iReason);

void CreateDestroyMouseRegionsForSquadMenu(BOOLEAN fPositionBox);
void SquadMenuMvtCallBack(struct MOUSE_REGION *pRegion, INT32 iReason);
void SquadMenuBtnCallback(struct MOUSE_REGION *pRegion, INT32 iReason);

// vehicle menu
void CreateDestroyMouseRegionForVehicleMenu(void);
void VehicleMenuMvtCallback(struct MOUSE_REGION *pRegion, INT32 iReason);
void VehicleMenuBtnCallback(struct MOUSE_REGION *pRegion, INT32 iReason);

// repair menu
void CreateDestroyMouseRegionForRepairMenu(void);
void RepairMenuMvtCallback(struct MOUSE_REGION *pRegion, INT32 iReason);
void RepairMenuBtnCallback(struct MOUSE_REGION *pRegion, INT32 iReason);

// contract menu
void CreateDestroyMouseRegionsForContractMenu(void);
void ContractMenuBtnCallback(struct MOUSE_REGION *pRegion, INT32 iReason);
void ContractMenuMvtCallback(struct MOUSE_REGION *pRegion, INT32 iReason);
void RebuildContractBoxForMerc(struct SOLDIERTYPE *pCharacter);

// remove merc from team menu callback
void RemoveMercMenuBtnCallback(struct MOUSE_REGION *pRegion, INT32 iReason);
void RemoveMercMenuMvtCallBack(struct MOUSE_REGION *pRegion, INT32 iReason);
void CreateDestroyMouseRegionsForRemoveMenu(void);

// misc assignment GUI functions
void HandleShadingOfLinesForAssignmentMenus(void);
BOOLEAN IsCharacterAliveAndConscious(struct SOLDIERTYPE *pCharacter);
void CreateDestroyScreenMaskForAssignmentAndContractMenus(void);

BOOLEAN CreateDestroyAssignmentPopUpBoxes(void);
void SetSoldierAssignment(struct SOLDIERTYPE *pSoldier, int8_t bAssignment, INT32 iParam1,
                          INT32 iParam2, INT32 iParam3);

// set merc asleep and awake under the new sleep system implemented June 29, 1998
// if give warning is false, the function can be used as an internal function
BOOLEAN SetMercAwake(struct SOLDIERTYPE *pSoldier, BOOLEAN fGiveWarning, BOOLEAN fForceHim);
BOOLEAN SetMercAsleep(struct SOLDIERTYPE *pSoldier, BOOLEAN fGiveWarning);
BOOLEAN PutMercInAsleepState(struct SOLDIERTYPE *pSoldier);
BOOLEAN PutMercInAwakeState(struct SOLDIERTYPE *pSoldier);

BOOLEAN AssignMercToAMovementGroup(struct SOLDIERTYPE *pSoldier);

// set what time this merc undertook this assignment
void SetTimeOfAssignmentChangeForMerc(struct SOLDIERTYPE *pSoldier);

// enough time on assignment for it to count?
BOOLEAN EnoughTimeOnAssignment(struct SOLDIERTYPE *pSoldier);

// check if any merc in group is too tired to keep moving
BOOLEAN AnyMercInGroupCantContinueMoving(struct GROUP *pGroup);

// handle selected group of mercs being put to sleep
BOOLEAN HandleSelectedMercsBeingPutAsleep(BOOLEAN fWakeUp, BOOLEAN fDisplayWarning);

// is any one on the team on this assignment?
BOOLEAN IsAnyOneOnPlayersTeamOnThisAssignment(int8_t bAssignment);

// rebuild assignments box
void RebuildAssignmentsBox(void);

void BandageBleedingDyingPatientsBeingTreated();

void ReEvaluateEveryonesNothingToDo();

// set assignment for list of characters
void SetAssignmentForList(int8_t bAssignment, int8_t bParam);

// is this area maxed out on militia?
BOOLEAN IsMilitiaTrainableFromSoldiersSectorMaxed(struct SOLDIERTYPE *pSoldier);

// function where we actually set someone's assignment so we can trap certain situations
void ChangeSoldiersAssignment(struct SOLDIERTYPE *pSoldier, int8_t bAssignment);

void UnEscortEPC(struct SOLDIERTYPE *pSoldier);

struct SOLDIERTYPE *AnyDoctorWhoCanHealThisPatient(struct SOLDIERTYPE *pPatient, BOOLEAN fThisHour);

#ifdef JA2BETAVERSION
void VerifyTownTrainingIsPaidFor(void);
#endif

#endif
