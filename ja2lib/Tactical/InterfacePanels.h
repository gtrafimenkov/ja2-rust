// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#ifndef __INTERFACE_PANELS
#define __INTERFACE_PANELS

#include "SGP/Types.h"
#include "Tactical/SoldierControl.h"

struct MOUSE_REGION;

enum {
  STANCEUP_BUTTON = 0,
  UPDOWN_BUTTON,
  CLIMB_BUTTON,
  STANCEDOWN_BUTTON,
  HANDCURSOR_BUTTON,
  PREVMERC_BUTTON,
  NEXTMERC_BUTTON,
  OPTIONS_BUTTON,
  BURSTMODE_BUTTON,
  LOOK_BUTTON,
  TALK_BUTTON,
  MUTE_BUTTON,
  SM_DONE_BUTTON,
  SM_MAP_SCREEN_BUTTON,
  NUM_SM_BUTTONS
};

enum { TEAM_DONE_BUTTON = 0, TEAM_MAP_SCREEN_BUTTON, CHANGE_SQUAD_BUTTON, NUM_TEAM_BUTTONS };

#define NEW_ITEM_CYCLE_COUNT 19
#define NEW_ITEM_CYCLES 4
#define NUM_TEAM_SLOTS 6

#define PASSING_ITEM_DISTANCE_OKLIFE 3
#define PASSING_ITEM_DISTANCE_NOTOKLIFE 2

#define SHOW_LOCATOR_NORMAL 1
#define SHOW_LOCATOR_FAST 2

BOOLEAN CreateSMPanelButtons();
void RemoveSMPanelButtons();
BOOLEAN InitializeSMPanel();
BOOLEAN ShutdownSMPanel();
void RenderSMPanel(BOOLEAN *pfDirty);
void EnableSMPanelButtons(BOOLEAN fEnable, BOOLEAN fFromItemPickup);

BOOLEAN CreateTEAMPanelButtons();
void RemoveTEAMPanelButtons();
BOOLEAN InitializeTEAMPanel();
BOOLEAN ShutdownTEAMPanel();
void RenderTEAMPanel(BOOLEAN fDirty);
void UpdateTEAMPanel();

void SetSMPanelCurrentMerc(uint8_t ubNewID);
void SetTEAMPanelCurrentMerc(uint8_t ubNewID);
uint16_t GetSMPanelCurrentMerc();
void UpdateSMPanel();

BOOLEAN InitTEAMSlots();
void AddPlayerToInterfaceTeamSlot(uint8_t ubID);
BOOLEAN RemovePlayerFromInterfaceTeamSlot(uint8_t ubID);
BOOLEAN GetPlayerIDFromInterfaceTeamSlot(uint8_t ubPanelSlot, uint8_t *pubID);
void RemoveAllPlayersFromSlot();
BOOLEAN PlayerExistsInSlot(uint8_t ubID);
BOOLEAN RemovePlayerFromTeamSlotGivenMercID(uint8_t ubMercID);
void CheckForAndAddMercToTeamPanel(struct SOLDIERTYPE *pSoldier);

void DisableTacticalTeamPanelButtons(BOOLEAN fDisable);
void RenderTownIDString();
void KeyRingSlotInvClickCallback(struct MOUSE_REGION *pRegion, int32_t iReason);

extern int8_t gbNewItem[NUM_INV_SLOTS];
extern int8_t gbNewItemCycle[NUM_INV_SLOTS];
extern uint8_t gubNewItemMerc;

void ShowRadioLocator(uint8_t ubID, uint8_t ubLocatorSpeed);
void EndRadioLocator(uint8_t ubID);

extern struct MOUSE_REGION gSMPanelRegion;

extern uint32_t guiSecItemHiddenVO;

extern BOOLEAN gfDisableTacticalPanelButtons;

typedef struct {
  uint8_t ubID;
  BOOLEAN fOccupied;

} TEAM_PANEL_SLOTS_TYPE;

extern TEAM_PANEL_SLOTS_TYPE gTeamPanel[NUM_TEAM_SLOTS];

// Used when the shop keeper interface is active
void DisableSMPpanelButtonsWhenInShopKeeperInterface(BOOLEAN fDontDrawButtons);
// void DisableSMPpanelButtonsWhenInShopKeeperInterface( );

//
void ReEvaluateDisabledINVPanelButtons();
void CheckForReEvaluateDisabledINVPanelButtons();

void CheckForDisabledForGiveItem();
void ReevaluateItemHatches(struct SOLDIERTYPE *pSoldier, BOOLEAN fEnable);

void HandlePanelFaceAnimations(struct SOLDIERTYPE *pSoldier);

void GoToMapScreenFromTactical(void);

void HandleTacticalEffectsOfEquipmentChange(struct SOLDIERTYPE *pSoldier, uint32_t uiInvPos,
                                            uint16_t usOldItem, uint16_t usNewItem);

void FinishAnySkullPanelAnimations();

uint8_t FindNextMercInTeamPanel(struct SOLDIERTYPE *pSoldier, BOOLEAN fGoodForLessOKLife,
                                BOOLEAN fOnlyRegularMercs);

#endif
