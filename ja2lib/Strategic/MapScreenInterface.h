#ifndef MAP_SCREEN_INTERFACE_H
#define MAP_SCREEN_INTERFACE_H

#include "MessageBoxScreen.h"
#include "SGP/Types.h"
#include "Tactical/SoldierControl.h"

typedef struct FASTHELPREGION {
  // the string
  wchar_t FastHelpText[256];

  // the x and y position values
  int32_t iX;
  int32_t iY;
  int32_t iW;

} FASTHELPREGION;

// String Lengths Defines
#define MAX_NAME_LENGTH 10
#define MAX_LOCATION_SIZE 8
#define MAX_DESTETA_SIZE 8
#define MAX_ASSIGN_SIZE 10
#define MAX_TIME_REMAINING_SIZE 8

// char breath and life position
#define BAR_INFO_X 66
#define BAR_INFO_Y 61

// merc icon position
#define CHAR_ICON_CONTRACT_Y 64
#define CHAR_ICON_X 187
#define CHAR_ICON_WIDTH 10
#define CHAR_ICON_HEIGHT 10
#define CHAR_ICON_SPACING 13

#define MAX_VEHICLE_COUNT 20

// map screen font
#define MAP_SCREEN_FONT BLOCKFONT2

// characterlist regions
#define Y_START 146
#define MAP_START_KEYRING_Y 107
#define Y_SIZE GetFontHeight(MAP_SCREEN_FONT)

// attribute menu defines (must match NUM_TRAINABLE_STATS defines, and pAttributeMenuStrings )
enum {
  ATTRIB_MENU_STR = 0,
  ATTRIB_MENU_DEX,
  ATTRIB_MENU_AGI,
  ATTRIB_MENU_HEA,
  ATTRIB_MENU_MARK,
  ATTRIB_MENU_MED,
  ATTRIB_MENU_MECH,
  ATTRIB_MENU_LEAD,
  ATTRIB_MENU_EXPLOS,
  ATTRIB_MENU_CANCEL,
  MAX_ATTRIBUTE_STRING_COUNT,
};

// the epc assignment menu
enum {
  EPC_MENU_ON_DUTY = 0,
  EPC_MENU_PATIENT,
  EPC_MENU_VEHICLE,
  EPC_MENU_REMOVE,
  EPC_MENU_CANCEL,
  MAX_EPC_MENU_STRING_COUNT,
};

// assignment menu defines
enum {
  ASSIGN_MENU_ON_DUTY = 0,
  ASSIGN_MENU_DOCTOR,
  ASSIGN_MENU_PATIENT,
  ASSIGN_MENU_VEHICLE,
  ASSIGN_MENU_REPAIR,
  ASSIGN_MENU_TRAIN,
  ASSIGN_MENU_CANCEL,
  MAX_ASSIGN_STRING_COUNT,
};

// training assignment menu defines
enum {
  TRAIN_MENU_SELF,
  TRAIN_MENU_TOWN,
  TRAIN_MENU_TEAMMATES,
  TRAIN_MENU_TRAIN_BY_OTHER,
  TRAIN_MENU_CANCEL,
  MAX_TRAIN_STRING_COUNT,
};

// the remove merc from team pop up box strings
enum {
  REMOVE_MERC = 0,
  REMOVE_MERC_CANCEL,
  MAX_REMOVE_MERC_COUNT,
};

// squad menu defines
enum {
  SQUAD_MENU_1,
  SQUAD_MENU_2,
  SQUAD_MENU_3,
  SQUAD_MENU_4,
  SQUAD_MENU_5,
  SQUAD_MENU_6,
  SQUAD_MENU_7,
  SQUAD_MENU_8,
  SQUAD_MENU_9,
  SQUAD_MENU_10,
  SQUAD_MENU_11,
  SQUAD_MENU_12,
  SQUAD_MENU_13,
  SQUAD_MENU_14,
  SQUAD_MENU_15,
  SQUAD_MENU_16,
  SQUAD_MENU_17,
  SQUAD_MENU_18,
  SQUAD_MENU_19,
  SQUAD_MENU_20,
  SQUAD_MENU_CANCEL,
  MAX_SQUAD_MENU_STRING_COUNT,
};

// contract menu defines
enum {
  CONTRACT_MENU_CURRENT_FUNDS = 0,
  CONTRACT_MENU_SPACE,
  CONTRACT_MENU_DAY,
  CONTRACT_MENU_WEEK,
  CONTRACT_MENU_TWO_WEEKS,
  CONTRACT_MENU_TERMINATE,
  CONTRACT_MENU_CANCEL,
  MAX_CONTRACT_MENU_STRING_COUNT,
};

// enums for pre battle interface pop ups
enum { ASSIGNMENT_POPUP, DESTINATION_POPUP, CONTRACT_POPUP };

enum {
  NO_REASON_FOR_UPDATE = 0,
  CONTRACT_FINISHED_FOR_UPDATE,
  ASSIGNMENT_FINISHED_FOR_UPDATE,
  ASSIGNMENT_RETURNING_FOR_UPDATE,
  ASLEEP_GOING_AUTO_FOR_UPDATE,
  CONTRACT_EXPIRE_WARNING_REASON,
};

enum {
  START_RED_SECTOR_LOCATOR = 0,
  STOP_RED_SECTOR_LOCATOR,
  START_YELLOW_SECTOR_LOCATOR,
  STOP_YELLOW_SECTOR_LOCATOR,
};

// dimensions and offset for merc update box
#define UPDATE_MERC_FACE_X_WIDTH 50
#define UPDATE_MERC_FACE_X_HEIGHT 50
#define UPDATE_MERC_FACE_X_OFFSET 2
#define UPDATE_MERC_FACE_Y_OFFSET 2
#define WIDTH_OF_UPDATE_PANEL_BLOCKS 50
#define HEIGHT_OF_UPDATE_PANEL_BLOCKS 50
#define UPDATE_MERC_Y_OFFSET 4
#define UPDATE_MERC_X_OFFSET 4

// dimensions and offset for merc update box
#define TACT_UPDATE_MERC_FACE_X_WIDTH 70
#define TACT_UPDATE_MERC_FACE_X_HEIGHT 49
#define TACT_UPDATE_MERC_FACE_X_OFFSET 8
#define TACT_UPDATE_MERC_FACE_Y_OFFSET 6
#define TACT_WIDTH_OF_UPDATE_PANEL_BLOCKS 70
#define TACT_HEIGHT_OF_UPDATE_PANEL_BLOCKS 49
#define TACT_UPDATE_MERC_Y_OFFSET 4
#define TACT_UPDATE_MERC_X_OFFSET 4

// the first vehicle slot int he list
#define FIRST_VEHICLE 18

typedef struct MERC_LEAVE_ITEM {
  struct OBJECTTYPE o;
  struct MERC_LEAVE_ITEM *pNext;
} MERC_LEAVE_ITEM;

extern BOOLEAN fShowAssignmentMenu;
extern BOOLEAN fShowTrainingMenu;
extern BOOLEAN fShowAttributeMenu;
extern BOOLEAN fShowSquadMenu;
extern BOOLEAN fShowContractMenu;
extern BOOLEAN fShowRemoveMenu;

extern BOOLEAN fFirstTimeInMapScreen;
extern BOOLEAN fLockOutMapScreenInterface;

// The character data structure
typedef struct {
  uint16_t usSolID;  // soldier ID in MenPtrs
  BOOLEAN fValid;    // is the current soldier a valid soldier

} MapScreenCharacterSt;

// map screen character structure list, contrains soldier ids into menptr
extern MapScreenCharacterSt gCharactersList[];

extern BOOLEAN fShowMapScreenHelpText;

// map inventory pool inited
extern BOOLEAN fMapInventoryPoolInited;

// highlighted lines
extern int32_t giHighLine;
extern int32_t giAssignHighLine;
extern int32_t giDestHighLine;
extern int32_t giContractHighLine;
extern int32_t giSleepHighLine;

extern uint32_t guiUpdatePanel;
extern uint32_t guiUpdatePanelTactical;
extern BOOLEAN fShowUpdateBox;

extern SGPRect ContractDimensions;
extern SGPPoint ContractPosition;
extern SGPRect AttributeDimensions;
extern SGPPoint AttributePosition;
extern SGPRect TrainDimensions;
extern SGPPoint TrainPosition;
extern SGPRect VehicleDimensions;
extern SGPPoint VehiclePosition;
extern SGPRect AssignmentDimensions;
extern SGPPoint AssignmentPosition;
extern SGPPoint SquadPosition;
extern SGPRect SquadDimensions;

extern SGPPoint RepairPosition;
extern SGPRect RepairDimensions;

extern SGPPoint OrigContractPosition;
extern SGPPoint OrigAttributePosition;
extern SGPPoint OrigSquadPosition;
extern SGPPoint OrigAssignmentPosition;
extern SGPPoint OrigTrainPosition;
extern SGPPoint OrigVehiclePosition;

// disble team info panel due to showing of battle roster
extern BOOLEAN fDisableDueToBattleRoster;

extern BOOLEAN gfAtLeastOneMercWasHired;

// curtrent map sector z that is being displayed in the mapscreen
extern int32_t iCurrentMapSectorZ;

// y position of the pop up box
extern int32_t giBoxY;

// pop up box textures
extern uint32_t guiPOPUPTEX;
extern uint32_t guiPOPUPBORDERS;

// the level-changing markers on the map border
extern uint32_t guiLEVELMARKER;

// the currently selected character arrow
extern uint32_t guiSelectedCharArrow;

// sam and mine icons
extern uint32_t guiSAMICON;

extern BOOLEAN fShowMapScreenMovementList;

// do we need to rebuild the mapscreen characterlist?
extern BOOLEAN fReBuildCharacterList;

// restore glow rotation in contract region glow boxes
extern BOOLEAN fResetContractGlow;

// init vehicle and characters list
void InitalizeVehicleAndCharacterList(void);

// set this entry to as selected
void SetEntryInSelectedCharacterList(int8_t bEntry);
// set this entry to as unselected
void ResetEntryForSelectedList(int8_t bEntry);

// reset selected list
void ResetSelectedListForMapScreen();

// build a selected list from a to b, inclusive
void BuildSelectedListFromAToB(int8_t bA, int8_t bB);

// isa this entry int he selected character list set?
BOOLEAN IsEntryInSelectedListSet(int8_t bEntry);

// is there more than one person selected?
BOOLEAN MultipleCharacterListEntriesSelected(void);

// toggle this entry on or off
void ToggleEntryInSelectedList(int8_t bEntry);

void RestoreBackgroundForAssignmentGlowRegionList(void);
void RestoreBackgroundForDestinationGlowRegionList(void);
void RestoreBackgroundForContractGlowRegionList(void);
void RestoreBackgroundForSleepGlowRegionList(void);

// play click when we are entering a glow region
void PlayGlowRegionSound(void);

// is this character in the action of plotting a path?
int16_t CharacterIsGettingPathPlotted(int16_t sCharNumber);

// disable team info panels
void DisableTeamInfoPanels(void);

// enable team info panels
void EnableTeamInfoPanels(void);

// activate pop up for soldiers in the pre battle interface
void ActivateSoldierPopup(struct SOLDIERTYPE *pSoldier, uint8_t ubPopupType, int16_t xp,
                          int16_t yp);

// hop up one leve,l int he map screen level interface
void GoUpOneLevelInMap(void);

// go down one level in the mapscreen map interface
void GoDownOneLevelInMap(void);

// jump to this level on the map
void JumpToLevel(int32_t iLevel);

// check to see if we need to update the screen
void CheckAndUpdateBasedOnContractTimes(void);

// check if are just about to display this pop up or stopping display
void HandleDisplayOfItemPopUpForSector(int16_t sMapX, int16_t sMapY, int16_t sMapZ);

// display red arrow by name of selected merc
void HandleDisplayOfSelectedMercArrows(void);

// check which guys can move with this guy
void DeselectSelectedListMercsWhoCantMoveWithThisGuy(struct SOLDIERTYPE *pSoldier);

// get morale string for this grunt given this morale level
void GetMoraleString(struct SOLDIERTYPE *pSoldier, wchar_t *sString);

// handle leaving of equipment in sector
void HandleLeavingOfEquipmentInCurrentSector(uint32_t uiMercId);

// set up a linked list of items being dropped and post an event to later drop them
void HandleMercLeavingEquipmentInDrassen(uint32_t uiMercId);
void HandleMercLeavingEquipmentInOmerta(uint32_t uiMercId);

// actually drop the stored list of items
void HandleEquipmentLeftInOmerta(uint32_t uiSlotIndex);
void HandleEquipmentLeftInDrassen(uint32_t uiSlotIndex);

// init/shutdown leave item lists
void InitLeaveList(void);
void ShutDownLeaveList(void);

// add item to leave equip index
BOOLEAN AddItemToLeaveIndex(struct OBJECTTYPE *o, uint32_t uiIndex);

// release memory for all items in this slot's leave item list
void FreeLeaveListSlot(uint32_t uiSlotIndex);

// first free slot in equip leave list
int32_t FindFreeSlotInLeaveList(void);

// set up drop list
int32_t SetUpDropItemListForMerc(uint32_t uiMercId);
// store owner's profile id for the items added to this leave slot index
void SetUpMercAboutToLeaveEquipment(uint32_t ubProfileId, uint32_t uiSlotIndex);

// remove item from leave index
// BOOLEAN RemoveItemFromLeaveIndex( MERC_LEAVE_ITEM *pItem, uint32_t uiIndex );

// handle a group about to arrive in a sector
void HandleGroupAboutToArrive(void);

// up arrow
void HandleMapScreenUpArrow(void);
void HandleMapScreenDownArrow(void);

// create and destroy the status bars mouse region
void CreateMapStatusBarsRegion(void);
void RemoveMapStatusBarsRegion(void);
void UpdateCharRegionHelpText(void);

// find this soldier in mapscreen character list and set as contract
void FindAndSetThisContractSoldier(struct SOLDIERTYPE *pSoldier);

// lose the cursor, re-render
void HandleMAPUILoseCursorFromOtherScreen(void);

void RenderMapRegionBackground(void);

// update mapscreen assignment positions
void UpdateMapScreenAssignmentPositions(void);

// get the umber of valid mercs in the mapscreen character list
int32_t GetNumberOfPeopleInCharacterList(void);

// the next and previous people in the mapscreen
void GoToPrevCharacterInList(void);
void GoToNextCharacterInList(void);

// this does the whole miner giving player info speil
void HandleMinerEvent(uint8_t bMinerNumber, u8 sSectorX, u8 sSectorY, int16_t sQuoteNumber,
                      BOOLEAN fForceMapscreen);

// set up the event of animating a mine sector
void SetUpAnimationOfMineSectors(int32_t iEvent);

// display map screen
void DisplayMapScreenFastHelpList(void);

// handle display of fast help
void HandleDisplayOfExitToTacticalMessageForFirstEntryToMapScreen(void);

// is the text up?
BOOLEAN IsMapScreenHelpTextUp(void);

// stop the help text in mapscreen
void StopMapScreenHelpText(void);

// set up the help text
void SetUpMapScreenFastHelpText(void);

void TurnOnSectorLocator(uint8_t ubProfileID);
void TurnOffSectorLocator();

extern int16_t gsSectorLocatorX;
extern int16_t gsSectorLocatorY;
extern uint8_t gubBlitSectorLocatorCode;

enum { LOCATOR_COLOR_NONE, LOCATOR_COLOR_RED, LOCATOR_COLOR_YELLOW };

extern uint32_t guiSectorLocatorGraphicID;
void HandleBlitOfSectorLocatorIcon(u8 sSectorX, u8 sSectorY, int16_t sSectorZ, uint8_t ubLocatorID);

// the tactical version

// handle the actual showingof the list
void HandleShowingOfTacticalInterfaceFastHelpText(void);

// start showing the list
void StartShowingInterfaceFastHelpText(void);

// stop showing the list
void StopShowingInterfaceFastHelpText(void);

// is the list active?
BOOLEAN IsTheInterfaceFastHelpTextActive(void);

// set up the tactical lists
BOOLEAN SetUpFastHelpListRegions(int32_t iXPosition[], int32_t iYPosition[], int32_t iWidth[],
                                 wchar_t *sString[], int32_t iSize);

// the alternate mapscreen movement system
void InitializeMovingLists(void);

// the sector move box
void DeselectSquadForMovement(int32_t iSquadNumber);
void SelectedSquadForMovement(int32_t iSquadNumber);
void DeselectSoldierForMovement(struct SOLDIERTYPE *pSoldier);
void SelectSoldierForMovement(struct SOLDIERTYPE *pSoldier);
void SelectVehicleForMovement(int32_t iVehicleId, BOOLEAN fAndAllOnBoard);
void DeselectVehicleForMovement(int32_t iVehicleId);
void AddVehicleToMovingLists(int32_t iVehicleId);
void AddSquadToMovingLists(int32_t iSquadNumber);
void AddSoldierToMovingLists(struct SOLDIERTYPE *pSoldier);
void CreateDestroyMovementBox(u8 sSectorX, u8 sSectorY, int16_t sSectorZ);
void SetUpMovingListsForSector(u8 sSectorX, u8 sSectorY, int16_t sSectorZ);
void ReBuildMoveBox(void);
BOOLEAN IsCharacterSelectedForAssignment(int16_t sCharNumber);
BOOLEAN IsCharacterSelectedForSleep(int16_t sCharNumber);

// the update box
void CreateDestroyTheUpdateBox(void);
void SetSoldierUpdateBoxReason(int32_t iReason);
void AddSoldierToUpdateBox(struct SOLDIERTYPE *pSoldier);
void ResetSoldierUpdateBox(void);
void DisplaySoldierUpdateBox();
BOOLEAN IsThePopUpBoxEmpty(void);

// unmarking buttons dirty for dialogue
void UpdateButtonsDuringCharacterDialogue(void);
void UpdateButtonsDuringCharacterDialogueSubTitles(void);
void SetUpdateBoxFlag(BOOLEAN fFlag);

/// set the town of Tixa as found by the player
void SetTixaAsFound(void);

// set the town of Orta as found by the player
void SetOrtaAsFound(void);

// set this SAM site as being found by the player
void SetSAMSiteAsFound(uint8_t uiSamIndex);

// init time menus
void InitTimersForMoveMenuMouseRegions(void);

// the screen mask
void CreateScreenMaskForMoveBox(void);
void RemoveScreenMaskForMoveBox(void);

// help text to show user merc has insurance
void UpdateHelpTextForMapScreenMercIcons(void);
void CreateDestroyInsuranceMouseRegionForMercs(BOOLEAN fCreate);

// stuff to deal with player just starting the game
BOOLEAN HandleTimeCompressWithTeamJackedInAndGearedToGo(void);
// void HandlePlayerEnteringMapScreenBeforeGoingToTactical( void );

// handle sector being taken over uncontested
BOOLEAN NotifyPlayerWhenEnemyTakesControlOfImportantSector(u8 sSectorX, u8 sSectorY,
                                                           int8_t bSectorZ, BOOLEAN fContested);

// handle notifying player of invasion by enemy
void NotifyPlayerOfInvasionByEnemyForces(u8 sSectorX, u8 sSectorY, int8_t bSectorZ,
                                         MSGBOX_CALLBACK ReturnCallback);

void ShutDownUserDefineHelpTextRegions(void);

// shwo the update box
void ShowUpdateBox(void);

// add special events
void AddSoldierToWaitingListQueue(struct SOLDIERTYPE *pSoldier);
void AddReasonToWaitingListQueue(int32_t iReason);
void AddDisplayBoxToWaitingQueue(void);

// can this group move it out
BOOLEAN CanEntireMovementGroupMercIsInMove(struct SOLDIERTYPE *pSoldier, int8_t *pbErrorNumber);
void ReportMapScreenMovementError(int8_t bErrorNumber);

void HandleRebuildingOfMapScreenCharacterList(void);

void RequestToggleTimeCompression(void);
void RequestIncreaseInTimeCompression(void);
void RequestDecreaseInTimeCompression(void);

void SelectUnselectedMercsWhoMustMoveWithThisGuy(void);
BOOLEAN AnyMercInSameSquadOrVehicleIsSelected(struct SOLDIERTYPE *pSoldier);

BOOLEAN LoadLeaveItemList(HWFILE hFile);
BOOLEAN SaveLeaveItemList(HWFILE hFile);

BOOLEAN CheckIfSalaryIncreasedAndSayQuote(struct SOLDIERTYPE *pSoldier,
                                          BOOLEAN fTriggerContractMenu);

void EndUpdateBox(BOOLEAN fContinueTimeCompression);

extern BOOLEAN CanCharacterMoveInStrategic(struct SOLDIERTYPE *pSoldier, int8_t *pbErrorNumber);
extern BOOLEAN MapscreenCanPassItemToCharNum(int32_t iNewCharSlot);

#endif
