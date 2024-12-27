// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#ifndef _INTERFACE_H
#define _INTERFACE_H

#include "Tactical/HandleUI.h"

struct STRUCTURE;

#define MAX_UICOMPOSITES 4

#define INTERFACE_START_Y 360
#define INV_INTERFACE_START_Y 340

#define INTERFACE_START_X 0

// FLASH PORTRAIT CODES
#define FLASH_PORTRAIT_STOP 0
#define FLASH_PORTRAIT_START 1
#define FLASH_PORTRAIT_WAITING 2
#define FLASH_PORTRAIT_DELAY 150

// FLASH PORTRAIT PALETTE IDS
#define FLASH_PORTRAIT_NOSHADE 0
#define FLASH_PORTRAIT_STARTSHADE 1
#define FLASH_PORTRAIT_ENDSHADE 2
#define FLASH_PORTRAIT_DARKSHADE 3
#define FLASH_PORTRAIT_GRAYSHADE 4
#define FLASH_PORTRAIT_LITESHADE 5

// GLOBAL DEFINES FOR SOME UI FLAGS
#define ARROWS_HIDE_UP 0x00000002
#define ARROWS_HIDE_DOWN 0x00000004
#define ARROWS_SHOW_UP_BESIDE 0x00000008
#define ARROWS_SHOW_DOWN_BESIDE 0x00000020
#define ARROWS_SHOW_UP_ABOVE_Y 0x00000040
#define ARROWS_SHOW_DOWN_BELOW_Y 0x00000080
#define ARROWS_SHOW_DOWN_BELOW_G 0x00000200
#define ARROWS_SHOW_DOWN_BELOW_YG 0x00000400
#define ARROWS_SHOW_DOWN_BELOW_GG 0x00000800
#define ARROWS_SHOW_UP_ABOVE_G 0x00002000
#define ARROWS_SHOW_UP_ABOVE_YG 0x00004000
#define ARROWS_SHOW_UP_ABOVE_GG 0x00008000
#define ARROWS_SHOW_UP_ABOVE_YY 0x00020000
#define ARROWS_SHOW_DOWN_BELOW_YY 0x00040000
#define ARROWS_SHOW_UP_ABOVE_CLIMB 0x00080000
#define ARROWS_SHOW_UP_ABOVE_CLIMB2 0x00400000
#define ARROWS_SHOW_UP_ABOVE_CLIMB3 0x00800000
#define ARROWS_SHOW_DOWN_CLIMB 0x02000000

#define ROOF_LEVEL_HEIGHT 50

#define LOCATEANDSELECT_MERC 1
#define LOCATE_MERC_ONCE 2

// Interface level enums
enum { I_GROUND_LEVEL, I_ROOF_LEVEL, I_NUMLEVELS };

#define DRAW_RED_BAR 1
#define DRAW_BLUE_BAR 2
#define DRAW_ERASE_BAR 3

extern BOOLEAN gfSwitchPanel;
extern BOOLEAN gfUIStanceDifferent;
extern uint8_t gbNewPanel;
extern uint8_t gubNewPanelParam;
extern int16_t gsCurInterfacePanel;

extern uint32_t guiCLOSE;
extern uint32_t guiDEAD;
extern uint32_t guiHATCH;
extern uint32_t guiGUNSM;
extern uint32_t guiP1ITEMS;
extern uint32_t guiP2ITEMS;
extern uint32_t guiP3ITEMS;
extern uint32_t guiCOMPANEL;
extern uint32_t guiCOMPANELB;
extern uint32_t guiRADIO;
extern uint32_t guiPORTRAITICONS;
extern uint32_t guiBURSTACCUM;
extern uint32_t guiITEMPOINTERHATCHES;

extern struct MOUSE_REGION gViewportRegion;
extern struct MOUSE_REGION gRadarRegion;
extern struct MOUSE_REGION gBottomPanalRegion;

#define MOVEMENT_MENU_LOOK 1
#define MOVEMENT_MENU_ACTIONC 2
#define MOVEMENT_MENU_HAND 3
#define MOVEMENT_MENU_TALK 4
#define MOVEMENT_MENU_RUN 5
#define MOVEMENT_MENU_WALK 6
#define MOVEMENT_MENU_SWAT 7
#define MOVEMENT_MENU_PRONE 8

#define DIRTYLEVEL0 0
#define DIRTYLEVEL1 1
#define DIRTYLEVEL2 2

typedef enum {
  SM_PANEL,
  TEAM_PANEL,
  NUM_UI_PANELS

} InterfacePanelDefines;

BOOLEAN InitializeTacticalInterface();
BOOLEAN ShutdownTacticalInterface();
extern BOOLEAN fInterfacePanelDirty;
extern BOOLEAN gfPausedTacticalRenderFlags;
extern BOOLEAN gfPausedTacticalRenderInterfaceFlags;
extern int16_t gsInterfaceLevel;
extern BOOLEAN gfInMovementMenu;

void PopupPositionMenu(UI_EVENT *pUIEvent);
void PopDownPositionMenu();
void PopupMovementMenu(UI_EVENT *pUIEvent);
void PopDownMovementMenu();
void RenderMovementMenu();
void CancelMovementMenu();

void PopDownOpenDoorMenu();
void RenderOpenDoorMenu();
BOOLEAN InitDoorOpenMenu(struct SOLDIERTYPE *pSoldier, struct STRUCTURE *pStructure,
                         uint8_t ubDirection, BOOLEAN fClosingDoor);
BOOLEAN HandleOpenDoorMenu();
void CancelOpenDoorMenu();

void HandleInterfaceBackgrounds();

void EndOverlayMessage();

void DrawSelectedUIAboveGuy(uint16_t usSoldierID);

void CreateCurrentTacticalPanelButtons();
void RemoveCurrentTacticalPanelButtons();
void SetCurrentTacticalPanelCurrentMerc(uint8_t ubID);
void SetCurrentInterfacePanel(uint8_t ubNewPanel);
BOOLEAN IsMercPortraitVisible(uint8_t ubSoldierID);

BOOLEAN InitializeCurrentPanel();
void ShutdownCurrentPanel();

void ClearInterface();
void RestoreInterface();

void RenderArrows();
void EraseRenderArrows();
void GetArrowsBackground();
void HandleUpDownArrowBackgrounds();

void EndDeadlockMsg();

void HandleLocateSelectMerc(uint8_t ubID, int8_t bFlag);

void DirtyMercPanelInterface(struct SOLDIERTYPE *pSoldier, uint8_t ubDirtyLevel);

void EndUIMessage();
void BeginUIMessage(wchar_t *pFontString, ...);
void InternalBeginUIMessage(BOOLEAN fUseSkullIcon, wchar_t *pFontString, ...);

// map screen version, for centering over the map area
void BeginMapUIMessage(uint8_t fPosition, wchar_t *pFontString, ...);

extern uint16_t gusUIOldSelectedSoldier;
extern int32_t giUIMessageOverlay;
extern uint32_t guiUIMessageTime;

typedef enum {
  NO_MESSAGE,
  COMPUTER_TURN_MESSAGE,
  COMPUTER_INTERRUPT_MESSAGE,
  PLAYER_INTERRUPT_MESSAGE,
  MILITIA_INTERRUPT_MESSAGE,
  AIR_RAID_TURN_MESSAGE,
  PLAYER_TURN_MESSAGE,

} MESSAGE_TYPES;

void HandleTopMessages();
BOOLEAN AddTopMessage(uint8_t ubType, wchar_t *pzString);
BOOLEAN InTopMessageBarAnimation();
void EndTopMessage();

void PauseRT(BOOLEAN fPause);

void InitEnemyUIBar(uint8_t ubNumEnemies, uint8_t ubDoneEnemies);

wchar_t *GetSoldierHealthString(struct SOLDIERTYPE *pSoldier);

void GetLaunchItemParamsFromUI();
void RenderAimCubeUI();
void ResetAimCubeAI();
void SetupAimCubeAI();
void IncrementAimCubeUI();
void EndAimCubeUI();
void BeginAimCubeUI(struct SOLDIERTYPE *pSoldier, int16_t sGridNo, int8_t ubLevel, uint8_t bStartPower,
                    int8_t bStartHeight);
BOOLEAN AimCubeUIClick();

void ResetPhysicsTrajectoryUI();
void SetupPhysicsTrajectoryUI();
void EndPhysicsTrajectoryUI();
void BeginPhysicsTrajectoryUI(int16_t sGridNo, int8_t bLevel, BOOLEAN fBadCTGT);

void InitPlayerUIBar(BOOLEAN fInterrupt);

void ToggleTacticalPanels();

void DirtyTopMessage();

void BeginMultiPurposeLocator(int16_t sGridNo, int8_t bLevel, BOOLEAN fSlideTo);
void HandleMultiPurposeLocator();
void RenderTopmostMultiPurposeLocator();

#endif
