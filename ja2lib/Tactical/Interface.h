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
extern UINT8 gbNewPanel;
extern UINT8 gubNewPanelParam;
extern INT16 gsCurInterfacePanel;

extern UINT32 guiCLOSE;
extern UINT32 guiDEAD;
extern UINT32 guiHATCH;
extern UINT32 guiGUNSM;
extern UINT32 guiP1ITEMS;
extern UINT32 guiP2ITEMS;
extern UINT32 guiP3ITEMS;
extern UINT32 guiCOMPANEL;
extern UINT32 guiCOMPANELB;
extern UINT32 guiRADIO;
extern UINT32 guiPORTRAITICONS;
extern UINT32 guiBURSTACCUM;
extern UINT32 guiITEMPOINTERHATCHES;

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
extern INT16 gsInterfaceLevel;
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
                         UINT8 ubDirection, BOOLEAN fClosingDoor);
BOOLEAN HandleOpenDoorMenu();
void CancelOpenDoorMenu();

void HandleInterfaceBackgrounds();

void EndOverlayMessage();

void DrawSelectedUIAboveGuy(UINT16 usSoldierID);

void CreateCurrentTacticalPanelButtons();
void RemoveCurrentTacticalPanelButtons();
void SetCurrentTacticalPanelCurrentMerc(UINT8 ubID);
void SetCurrentInterfacePanel(UINT8 ubNewPanel);
BOOLEAN IsMercPortraitVisible(UINT8 ubSoldierID);

BOOLEAN InitializeCurrentPanel();
void ShutdownCurrentPanel();

void ClearInterface();
void RestoreInterface();

void RenderArrows();
void EraseRenderArrows();
void GetArrowsBackground();
void HandleUpDownArrowBackgrounds();

void EndDeadlockMsg();

void HandleLocateSelectMerc(UINT8 ubID, INT8 bFlag);

void DirtyMercPanelInterface(struct SOLDIERTYPE *pSoldier, UINT8 ubDirtyLevel);

void EndUIMessage();
void BeginUIMessage(CHAR16 *pFontString, ...);
void InternalBeginUIMessage(BOOLEAN fUseSkullIcon, CHAR16 *pFontString, ...);

// map screen version, for centering over the map area
void BeginMapUIMessage(UINT8 fPosition, CHAR16 *pFontString, ...);

extern UINT16 gusUIOldSelectedSoldier;
extern INT32 giUIMessageOverlay;
extern UINT32 guiUIMessageTime;

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
BOOLEAN AddTopMessage(UINT8 ubType, CHAR16 *pzString);
BOOLEAN InTopMessageBarAnimation();
void EndTopMessage();

void PauseRT(BOOLEAN fPause);

void InitEnemyUIBar(UINT8 ubNumEnemies, UINT8 ubDoneEnemies);

CHAR16 *GetSoldierHealthString(struct SOLDIERTYPE *pSoldier);

void GetLaunchItemParamsFromUI();
void RenderAimCubeUI();
void ResetAimCubeAI();
void SetupAimCubeAI();
void IncrementAimCubeUI();
void EndAimCubeUI();
void BeginAimCubeUI(struct SOLDIERTYPE *pSoldier, INT16 sGridNo, INT8 ubLevel, UINT8 bStartPower,
                    INT8 bStartHeight);
BOOLEAN AimCubeUIClick();

void ResetPhysicsTrajectoryUI();
void SetupPhysicsTrajectoryUI();
void EndPhysicsTrajectoryUI();
void BeginPhysicsTrajectoryUI(INT16 sGridNo, INT8 bLevel, BOOLEAN fBadCTGT);

void InitPlayerUIBar(BOOLEAN fInterrupt);

void ToggleTacticalPanels();

void DirtyTopMessage();

void BeginMultiPurposeLocator(INT16 sGridNo, INT8 bLevel, BOOLEAN fSlideTo);
void HandleMultiPurposeLocator();
void RenderTopmostMultiPurposeLocator();

#endif
