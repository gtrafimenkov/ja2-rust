#include "Laptop/Personnel.h"

#include "GameLoop.h"
#include "Laptop/AIMMembers.h"
#include "Laptop/Finances.h"
#include "Laptop/Laptop.h"
#include "Laptop/LaptopSave.h"
#include "Money.h"
#include "Point.h"
#include "SGP/ButtonSystem.h"
#include "SGP/Debug.h"
#include "SGP/English.h"
#include "SGP/Input.h"
#include "SGP/Line.h"
#include "SGP/Random.h"
#include "SGP/VObject.h"
#include "SGP/VSurface.h"
#include "SGP/WCheck.h"
#include "Soldier.h"
#include "Strategic/Assignments.h"
#include "Strategic/GameClock.h"
#include "Strategic/MapScreen.h"
#include "Strategic/MapScreenInterfaceMap.h"
#include "Strategic/MercContract.h"
#include "Strategic/Strategic.h"
#include "Strategic/StrategicMap.h"
#include "Tactical/InterfaceItems.h"
#include "Tactical/Menptr.h"
#include "Tactical/Overhead.h"
#include "Tactical/SoldierAdd.h"
#include "Tactical/SoldierControl.h"
#include "Tactical/SoldierMacros.h"
#include "Tactical/SoldierProfile.h"
#include "Tactical/Weapons.h"
#include "Town.h"
#include "Utils/Cursors.h"
#include "Utils/EncryptedFile.h"
#include "Utils/Text.h"
#include "Utils/Utilities.h"
#include "Utils/WordWrap.h"

// struct POINT;

#define NUM_BACKGROUND_REPS 40
#define BACKGROUND_HEIGHT 10
#define BACKGROUND_WIDTH 125
#define IMAGE_BOX_X 395
#define IMAGE_BOX_Y LAPTOP_SCREEN_UL_Y + 24
#define IMAGE_BOX_WIDTH 112
#define IMAGE_BOX_WITH_NO_BORDERS 106
#define IMAGE_BOX_COUNT 4
#define IMAGE_NAME_WIDTH 106
#define IMAGE_FULL_NAME_OFFSET_Y 111
#define TEXT_BOX_WIDTH 160
#define TEXT_DELTA_OFFSET 9
#define TEXT_BOX_Y LAPTOP_SCREEN_UL_Y + 188
#define PAGE_BOX_X LAPTOP_SCREEN_UL_X + 250 - 10
#define PAGE_BOX_Y LAPTOP_SCREEN_UL_Y + 3
#define PAGE_BOX_WIDTH 58
#define PAGE_BOX_HEIGHT 24
#define MAX_SLOTS 4
#define PERS_CURR_TEAM_X LAPTOP_SCREEN_UL_X + 39 - 15
#define PERS_CURR_TEAM_Y LAPTOP_SCREEN_UL_Y + 218
#define PERS_DEPART_TEAM_Y LAPTOP_SCREEN_UL_Y + 247

#define BUTTON_Y LAPTOP_SCREEN_UL_Y + 34
#define LEFT_BUTTON_X LAPTOP_SCREEN_UL_X + 3 - 10
#define RIGHT_BUTTON_X LAPTOP_SCREEN_UL_X + 476 - 10
#define PERS_COUNT 15
#define MAX_STATS 20
#define PERS_FONT FONT10ARIAL
#define PERS_HEADER_FONT FONT14ARIAL
#define CHAR_NAME_FONT FONT12ARIAL
#define CHAR_NAME_Y 177
#define CHAR_LOC_Y 189
#define PERS_TEXT_FONT_COLOR FONT_WHITE  // 146
#define PERS_TEXT_FONT_ALTERNATE_COLOR FONT_YELLOW
#define PERS_FONT_COLOR FONT_WHITE
#define PAGE_X PAGE_BOX_X + 2 - 10
#define PAGE_Y PAGE_BOX_Y + 2

#define FACES_DIR "FACES\\BIGFACES\\"
#define SMALL_FACES_DIR "FACES\\"

#define NEXT_MERC_FACE_X LAPTOP_SCREEN_UL_X + 448
#define MERC_FACE_SCROLL_Y LAPTOP_SCREEN_UL_Y + 150
#define PREV_MERC_FACE_X LAPTOP_SCREEN_UL_X + 285

#define DEPARTED_X LAPTOP_SCREEN_UL_X + 29 - 10
#define DEPARTED_Y LAPTOP_SCREEN_UL_Y + 207

#define PERSONNEL_PORTRAIT_NUMBER 20
#define PERSONNEL_PORTRAIT_NUMBER_WIDTH 5

#define SMALL_PORTRAIT_WIDTH 46
#define SMALL_PORTRAIT_HEIGHT 42

#define SMALL_PORT_WIDTH 52
#define SMALL_PORT_HEIGHT 45

#define SMALL_PORTRAIT_WIDTH_NO_BORDERS 48

#define SMALL_PORTRAIT_START_X 141 - 10
#define SMALL_PORTRAIT_START_Y 53

#define PERS_CURR_TEAM_COST_X LAPTOP_SCREEN_UL_X + 150 - 10
#define PERS_CURR_TEAM_COST_Y LAPTOP_SCREEN_UL_Y + 218

#define PERS_CURR_TEAM_HIGHEST_Y PERS_CURR_TEAM_COST_Y + 15
#define PERS_CURR_TEAM_LOWEST_Y PERS_CURR_TEAM_HIGHEST_Y + 15

#define PERS_CURR_TEAM_WIDTH 286 - 160

#define PERS_DEPART_TEAM_WIDTH PERS_CURR_TEAM_WIDTH - 20

#define PERS_STAT_AVG_X LAPTOP_SCREEN_UL_X + 157 - 10
#define PERS_STAT_AVG_Y LAPTOP_SCREEN_UL_Y + 274
#define PERS_STAT_AVG_WIDTH 202 - 159
#define PERS_STAT_LOWEST_X LAPTOP_SCREEN_UL_X + 72 - 10
#define PERS_STAT_LOWEST_WIDTH 155 - 75
#define PERS_STAT_HIGHEST_X LAPTOP_SCREEN_UL_X + 205 - 10
#define PERS_STAT_LIST_X LAPTOP_SCREEN_UL_X + 33 - 10

#define PERS_TOGGLE_CUR_DEPART_WIDTH 106 - 35
#define PERS_TOGGLE_CUR_DEPART_HEIGHT 236 - 212

#define PERS_TOGGLE_CUR_DEPART_X LAPTOP_SCREEN_UL_X + 35 - 10
#define PERS_TOGGLE_CUR_Y LAPTOP_SCREEN_UL_Y + 208
#define PERS_TOGGLE_DEPART_Y LAPTOP_SCREEN_UL_Y + 238

#define PERS_DEPARTED_UP_X LAPTOP_SCREEN_UL_X + 265 - 10
#define PERS_DEPARTED_UP_Y LAPTOP_SCREEN_UL_Y + 210
#define PERS_DEPARTED_DOWN_Y LAPTOP_SCREEN_UL_Y + 237

#define PERS_TITLE_X 140
#define PERS_TITLE_Y 33

#define ATM_UL_X LAPTOP_SCREEN_UL_X + 397
#define ATM_UL_Y LAPTOP_SCREEN_UL_Y + 27

/// atm font
#define ATM_FONT PERS_FONT

// departed states
enum {
  DEPARTED_DEAD = 0,
  DEPARTED_FIRED,
  DEPARTED_OTHER,
  DEPARTED_MARRIED,
  DEPARTED_CONTRACT_EXPIRED,
  DEPARTED_QUIT,

};

// atm button positions
#define ATM_BUTTONS_START_Y 110
#define ATM_BUTTONS_START_X 510
#define ATM_BUTTON_WIDTH 15
#define ATM_BUTTON_HEIGHT 15
#define ATM_DISPLAY_X 509
#define ATM_DISPLAY_Y 58
#define ATM_DISPLAY_HEIGHT 10
#define ATM_DISPLAY_WIDTH 81

// the number of inventory items per personnel page
#define NUMBER_OF_INVENTORY_PERSONNEL 8
#define Y_SIZE_OF_PERSONNEL_SCROLL_REGION (422 - 219)
#define X_SIZE_OF_PERSONNEL_SCROLL_REGION (589 - 573)
#define Y_OF_PERSONNEL_SCROLL_REGION 219
#define X_OF_PERSONNEL_SCROLL_REGION 573
#define SIZE_OF_PERSONNEL_CURSOR 19

// number buttons
enum {
  OK_ATM = 0,
  DEPOSIT_ATM,
  WIDTHDRAWL_ATM,
  CANCEL_ATM,
  CLEAR_ATM,
  NUMBER_ATM_BUTTONS,
};

// enums for the buttons in the information side bar ( used with giPersonnelATMStartButton[] )
enum {
  PERSONNEL_STAT_BTN,
  PERSONNEL_EMPLOYMENT_BTN,
  PERSONNEL_INV_BTN,

  PERSONNEL_NUM_BTN,
};

// enums for the current state of the information side bar ( stat panel )
enum {
  PRSNL_STATS,
  PRSNL_EMPLOYMENT,
  PRSNL_INV,
};
uint8_t gubPersonnelInfoState = PRSNL_STATS;

// enums for the pPersonnelScreenStrings[]
enum {
  PRSNL_TXT_HEALTH,  // HEALTH OF MERC
  PRSNL_TXT_AGILITY,
  PRSNL_TXT_DEXTERITY,
  PRSNL_TXT_STRENGTH,
  PRSNL_TXT_LEADERSHIP,
  PRSNL_TXT_WISDOM,   // 5
  PRSNL_TXT_EXP_LVL,  // EXPERIENCE LEVEL
  PRSNL_TXT_MARKSMANSHIP,
  PRSNL_TXT_MECHANICAL,
  PRSNL_TXT_EXPLOSIVES,
  PRSNL_TXT_MEDICAL,           // 10
  PRSNL_TXT_MED_DEPOSIT,       // AMOUNT OF MEDICAL DEPOSIT PUT DOWN ON THE MERC
  PRSNL_TXT_CURRENT_CONTRACT,  // COST OF CURRENT CONTRACT
  PRSNL_TXT_KILLS,             // NUMBER OF KILLS BY MERC
  PRSNL_TXT_ASSISTS,           // NUMBER OF ASSISTS ON KILLS BY MERC
  PRSNL_TXT_DAILY_COST,        // DAILY COST OF MERC			//15
  PRSNL_TXT_TOTAL_COST,        // TOTAL COST OF MERC
  PRSNL_TXT_CONTRACT,          // COST OF CURRENT CONTRACT
  PRSNL_TXT_TOTAL_SERVICE,     // TOTAL SERVICE RENDERED BY MERC
  PRSNL_TXT_UNPAID_AMOUNT,     // AMOUNT LEFT ON MERC MERC TO BE PAID
  PRSNL_TXT_HIT_PERCENTAGE,    // PERCENTAGE OF SHOTS THAT HIT TARGET		//20
  PRSNL_TXT_BATTLES,           // NUMBER OF BATTLES FOUGHT
  PRSNL_TXT_TIMES_WOUNDED,     // NUMBER OF TIMES MERC HAS BEEN WOUNDED
  PRSNL_TXT_SKILLS,
  PRSNL_TXT_NOSKILLS,
};

extern BOOLEAN gfTemporaryDisablingOfLoadPendingFlag;
extern BOOLEAN fExitingLaptopFlag;
extern void HandleLapTopESCKey(void);
extern void HandleAltTabKeyInLaptop(void);
extern void HandleShiftAltTabKeyInLaptop(void);

// BOOLEAN fShowInventory = FALSE;
uint8_t uiCurrentInventoryIndex = 0;

uint32_t guiSliderPosition;

// the transfer funds string
CHAR16 sTransferString[32];

int32_t giPersonnelATMSideButton[NUMBER_ATM_BUTTONS];
int32_t giPersonnelATMSideButtonImage[NUMBER_ATM_BUTTONS];

int32_t iNumberPadButtons[10];
int32_t iNumberPadButtonsImages[10];

#define PrsnlOffSetX (-15)  //-20
#define Prsnl_DATA_OffSetX (36)
#define PrsnlOffSetY 10

struct Point pPersonnelScreenPoints[] = {
    {422 + PrsnlOffSetX, 205 + PrsnlOffSetY},
    {422 + PrsnlOffSetX, 215 + PrsnlOffSetY},
    {422 + PrsnlOffSetX, 225 + PrsnlOffSetY},
    {422 + PrsnlOffSetX, 235 + PrsnlOffSetY},
    {422 + PrsnlOffSetX, 245 + PrsnlOffSetY},
    {422 + PrsnlOffSetX, 255 + PrsnlOffSetY},
    {422 + PrsnlOffSetX, 315 + PrsnlOffSetY},
    {422 + PrsnlOffSetX, 270 + PrsnlOffSetY},
    {422 + PrsnlOffSetX, 280 + PrsnlOffSetY},
    {422 + PrsnlOffSetX, 290 + PrsnlOffSetY},
    {422 + PrsnlOffSetX, 300 + PrsnlOffSetY},  // 10
    {422 + PrsnlOffSetX, 395 + PrsnlOffSetY},
    {422 + PrsnlOffSetX, 385 + PrsnlOffSetY},
    {422 + PrsnlOffSetX, 415 + PrsnlOffSetY},
    {422 + PrsnlOffSetX, 425 + PrsnlOffSetY},
    {422 + PrsnlOffSetX, 445 + PrsnlOffSetY},
    {422 + PrsnlOffSetX, 380 + PrsnlOffSetY},  // for contract price
    {422 + PrsnlOffSetX, 435 + PrsnlOffSetY},
    {140, 33},  // Personnel Header
    {422 + PrsnlOffSetX, 330 + PrsnlOffSetY},
    {422 + PrsnlOffSetX, 340 + PrsnlOffSetY},  // 20
    {422 + PrsnlOffSetX, 355 + PrsnlOffSetY},
    {422 + PrsnlOffSetX, 365 + PrsnlOffSetY},
    {422 + PrsnlOffSetX, 375 + PrsnlOffSetY},
    {422 + PrsnlOffSetX, 385 + PrsnlOffSetY},
    {422 + PrsnlOffSetX, 395 + PrsnlOffSetY},
};

uint32_t guiSCREEN;
uint32_t guiFACE;
uint32_t guiDEPARTEDTEAM;
uint32_t guiCURRENTTEAM;
uint32_t guiPersonnelInventory;

int32_t giPersonnelButton[6];
int32_t giPersonnelButtonImage[6];
int32_t giPersonnelInventoryButtons[2];
int32_t giPersonnelInventoryButtonsImages[2];
int32_t iStartPersonId;  // iId of the person who is leftmost on the display
int32_t iLastPersonId;
int32_t giDepartedButtonImage[2];
int32_t giDepartedButton[2];

// buttons for ATM
int32_t giPersonnelATMStartButton[3];
int32_t giPersonnelATMStartButtonImage[3];
int32_t giPersonnelATMButton;
int32_t giPersonnelATMButtonImage;

BOOLEAN fATMFlags = 0;
BOOLEAN fOldATMFlags = 0;
// the past team of the player
// int16_t ubDeadCharactersList[ 256 ];
// int16_t ubLeftCharactersList[ 256 ];
// int16_t ubOtherCharactersList[ 256 ];

// the id of currently displayed merc in right half of screen
int32_t iCurrentPersonSelectedId = -1;

int32_t giCurrentUpperLeftPortraitNumber = 0;

// which mode are we showing?..current team?...or deadly departed?
BOOLEAN fCurrentTeamMode = TRUE;

// show the atm panel?
BOOLEAN fShowAtmPanel = FALSE;
BOOLEAN fShowAtmPanelStartButton = TRUE;

// create buttons for scrolling departures
BOOLEAN fCreatePeronnelDepartureButton = FALSE;

// waitr one frame
BOOLEAN fOneFrameDelayInPersonnel = FALSE;

// whther or not we are creating mouse regions to place over portraits
BOOLEAN fCreatePersonnelPortraitMouseRegions = FALSE;

// mouse regions
struct MOUSE_REGION gPortraitMouseRegions[20];

struct MOUSE_REGION gTogglePastCurrentTeam[2];

struct MOUSE_REGION gMouseScrollPersonnelINV;

int32_t iCurPortraitId = 0;

// create mouse regions for past/current toggles
BOOLEAN fCreateRegionsForPastCurrentToggle = FALSE;

BOOLEAN LoadPersonnelGraphics(void);
void RemovePersonnelGraphics(void);
void RenderPersonnel(void);
void RenderPersonnelStats(int32_t iId, int32_t iSlot);
BOOLEAN RenderPersonnelFace(int32_t iId, int32_t iSlot, BOOLEAN fDead, BOOLEAN fFired,
                            BOOLEAN fOther);
BOOLEAN RenderPersonnelPictures(void);
void LeftButtonCallBack(GUI_BUTTON *btn, int32_t reason);
void RightButtonCallBack(GUI_BUTTON *btn, int32_t reason);
void LeftFFButtonCallBack(GUI_BUTTON *btn, int32_t reason);
void RightFFButtonCallBack(GUI_BUTTON *btn, int32_t reason);
void PersonnelPortraitCallback(struct MOUSE_REGION *pRegion, int32_t iReason);
void CreatePersonnelButtons(void);
void DeletePersonnelButtons(void);
void DisplayHeader(void);
void DisplayCharName(int32_t iId, int32_t iSlot);
void DisplayCharStats(int32_t iId, int32_t iSlot);
int32_t GetLastMercId(void);
void DrawPageNumber(void);
void SetPersonnelButtonStates(void);
void CreateDestroyButtonsForPersonnelDepartures(void);
BOOLEAN LoadPersonnelScreenBackgroundGraphics(void);
void DeletePersonnelScreenBackgroundGraphics(void);
void RenderPersonnelScreenBackground(void);
int32_t GetNumberOfMercsOnPlayersTeam(void);
void CreateDestroyMouseRegionsForPersonnelPortraits(void);
BOOLEAN DisplayPicturesOfCurrentTeam(void);
void DisplayFaceOfDisplayedMerc();
void DisplayNumberOnCurrentTeam(void);
void DisplayNumberDeparted(void);
int32_t GetTotalDailyCostOfCurrentTeam(void);
void DisplayCostOfCurrentTeam(void);
int32_t GetLowestDailyCostOfCurrentTeam(void);
int32_t GetHighestDailyCostOfCurrentTeam(void);
int32_t GetAvgStatOfCurrentTeamStat(int32_t iStat);
void DisplayAverageStatValuesForCurrentTeam(void);
void DisplayLowestStatValuesForCurrentTeam(void);
void DisplayHighestStatValuesForCurrentTeam(void);
void DisplayPersonnelTeamStats(void);
void InitPastCharactersList(void);
int32_t GetNumberOfPastMercsOnPlayersTeam(void);
int32_t GetNumberOfOtherOnPastTeam(void);
int32_t GetNumberOfLeftOnPastTeam(void);
int32_t GetNumberOfDeadOnPastTeam(void);
void DisplayStateOfPastTeamMembers(void);
void CreateDestroyCurrentDepartedMouseRegions(void);
void PersonnelCurrentTeamCallback(struct MOUSE_REGION *pRegion, int32_t iReason);
void PersonnelDepartedTeamCallback(struct MOUSE_REGION *pRegion, int32_t iReason);
void CreateDestroyButtonsForDepartedTeamList(void);
void DepartedDownCallBack(GUI_BUTTON *btn, int32_t reason);
void DepartedUpCallBack(GUI_BUTTON *btn, int32_t reason);
void DisplayPastMercsPortraits(void);
BOOLEAN DisplayPortraitOfPastMerc(int32_t iId, int32_t iCounter, BOOLEAN fDead, BOOLEAN fFired,
                                  BOOLEAN fOther);
int32_t GetIdOfPastMercInSlot(int32_t iSlot);
void DisplayDepartedCharStats(int32_t iId, int32_t iSlot, int32_t iState);
void EnableDisableDeparturesButtons(void);
void DisplayDepartedCharName(int32_t iId, int32_t iSlot, int32_t iState);
int32_t GetTheStateOfDepartedMerc(int32_t iId);
void DisplayPersonnelTextOnTitleBar(void);
int32_t GetIdOfDepartedMercWithHighestStat(int32_t iStat);
int32_t GetIdOfDepartedMercWithLowestStat(int32_t iStat);
void RenderInventoryForCharacter(int32_t iId, int32_t iSlot);
void DisplayInventoryForSelectedChar(void);
int32_t GetNumberOfInventoryItemsOnCurrentMerc(void);
void CreateDestroyPersonnelInventoryScrollButtons(void);
void EnableDisableInventoryScrollButtons(void);
void PersonnelINVStartButtonCallback(GUI_BUTTON *btn, int32_t reason);
void EmployementInfoButtonCallback(GUI_BUTTON *btn, int32_t reason);
void PersonnelStatStartButtonCallback(GUI_BUTTON *btn, int32_t reason);
void HandleSliderBarClickCallback(struct MOUSE_REGION *pRegion, int32_t iReason);
int32_t GetNumberOfMercsDeadOrAliveOnPlayersTeam(void);

void RenderSliderBarForPersonnelInventory(void);
void FindPositionOfPersInvSlider(void);

// check if current guy can have atm
void UpDateStateOfStartButton(void);
void HandlePersonnelKeyboard(void);

void DisplayEmploymentinformation(int32_t iId, int32_t iSlot);

// AIM merc:  Returns the amount of time left on mercs contract
// MERC merc: Returns the amount of time the merc has worked
// IMP merc:	Returns the amount of time the merc has worked
// else:			returns -1
int32_t CalcTimeLeftOnMercContract(struct SOLDIERTYPE *pSoldier);

// what state is the past merc in?
BOOLEAN IsPastMercDead(int32_t iId);
BOOLEAN IsPastMercFired(int32_t iId);
BOOLEAN IsPastMercOther(int32_t iId);

// display box around currently selected merc
BOOLEAN DisplayHighLightBox(void);

// id of merc in this slot
int32_t GetIdOfThisSlot(int32_t iSlot);

// grab appropriate id of soldier first being displayed
int32_t GetIdOfFirstDisplayedMerc();

// get avg for this stat
int32_t GetAvgStatOfPastTeamStat(int32_t iStat);

// render atm panel
BOOLEAN RenderAtmPanel(void);
void DisplayATMAmount(void);

// create destroy ATM button
void CreateDestroyStartATMButton(void);
void ATMStartButtonCallback(GUI_BUTTON *btn, int32_t reason);
void ATMNumberButtonCallback(GUI_BUTTON *btn, int32_t reason);
void HandleStateOfATMButtons(void);
void ATMOtherButtonCallback(GUI_BUTTON *btn, int32_t reason);

// atm misc functions

void ATMOther2ButtonCallback(GUI_BUTTON *btn, int32_t reason);
void DisplayATMStrings(void);
void DisplayAmountOnCurrentMerc(void);
void RenderRectangleForPersonnelTransactionAmount(void);
void HandleTimedAtmModes(void);

void GameInitPersonnel(void) {
  // init past characters lists
  InitPastCharactersList();
}

void EnterPersonnel(void) {
  fReDrawScreenFlag = TRUE;
  iStartPersonId = -1;

  iCurrentPersonSelectedId = -1;

  uiCurrentInventoryIndex = 0;
  guiSliderPosition = 0;

  iCurPortraitId = 0;

  // load graphics for screen
  LoadPersonnelGraphics();

  // show atm panel
  fShowAtmPanelStartButton = TRUE;

  // create buttons needed
  CreateDestroyButtonsForPersonnelDepartures();

  // load personnel
  LoadPersonnelScreenBackgroundGraphics();

  // render screen
  RenderPersonnel();

  // how many people do we have?..if you have someone set default to 0
  if (GetNumberOfMercsDeadOrAliveOnPlayersTeam() > 0) {
    iCurrentPersonSelectedId = GetIdOfFirstDisplayedMerc();
  }

  fCreatePersonnelPortraitMouseRegions = TRUE;

  CreateDestroyMouseRegionsForPersonnelPortraits();
  // set states of en- dis able buttons
  // SetPersonnelButtonStates( );

  fCreateRegionsForPastCurrentToggle = TRUE;

  CreateDestroyCurrentDepartedMouseRegions();

  // create buttons for screen
  CreatePersonnelButtons();

  // set states of en- dis able buttons
  SetPersonnelButtonStates();

  return;
}

void ExitPersonnel(void) {
  if (fCurrentTeamMode == FALSE) {
    fCurrentTeamMode = TRUE;
    CreateDestroyButtonsForDepartedTeamList();
    fCurrentTeamMode = FALSE;
  }

  // get rid of atm panel buttons
  fShowAtmPanelStartButton = FALSE;
  fShowAtmPanel = FALSE;
  fATMFlags = 0;
  CreateDestroyStartATMButton();

  //	fShowInventory = FALSE;
  gubPersonnelInfoState = PRSNL_STATS;

  CreateDestroyPersonnelInventoryScrollButtons();

  // get rid of graphics
  RemovePersonnelGraphics();

  DeletePersonnelScreenBackgroundGraphics();

  CreateDestroyButtonsForPersonnelDepartures();

  // delete buttons
  DeletePersonnelButtons();

  fCreatePersonnelPortraitMouseRegions = FALSE;

  // delete mouse regions
  CreateDestroyMouseRegionsForPersonnelPortraits();

  fCreateRegionsForPastCurrentToggle = FALSE;

  CreateDestroyCurrentDepartedMouseRegions();

  return;
}

void HandlePersonnel(void) {
  // RenderButtonsFastHelp( );
  CreateDestroyButtonsForPersonnelDepartures();

  // create / destroy buttons for scrolling departed list
  CreateDestroyButtonsForDepartedTeamList();

  // enable / disable departures buttons
  EnableDisableDeparturesButtons();

  // create destroy inv buttons as needed
  CreateDestroyPersonnelInventoryScrollButtons();

  // enable disable buttons as needed
  EnableDisableInventoryScrollButtons();

  HandlePersonnelKeyboard();

  // handle timed modes for ATM
  HandleTimedAtmModes();

  return;
}

BOOLEAN LoadPersonnelGraphics(void) {
  // load graphics needed for personnel screen
  VOBJECT_DESC VObjectDesc;

  // load graphics

  // title bar
  VObjectDesc.fCreateFlags = VOBJECT_CREATE_FROMFILE;
  FilenameForBPP("LAPTOP\\programtitlebar.sti", VObjectDesc.ImageFile);
  CHECKF(AddVideoObject(&VObjectDesc, &guiTITLE));

  // the background grpahics
  VObjectDesc.fCreateFlags = VOBJECT_CREATE_FROMFILE;
  FilenameForBPP("LAPTOP\\personnelwindow.sti", VObjectDesc.ImageFile);
  CHECKF(AddVideoObject(&VObjectDesc, &guiSCREEN));

  VObjectDesc.fCreateFlags = VOBJECT_CREATE_FROMFILE;
  FilenameForBPP("LAPTOP\\personnel_inventory.sti", VObjectDesc.ImageFile);
  CHECKF(AddVideoObject(&VObjectDesc, &guiPersonnelInventory));

  return (TRUE);
}

void RemovePersonnelGraphics(void) {
  // delete graphics needed for personnel screen

  DeleteVideoObjectFromIndex(guiSCREEN);
  DeleteVideoObjectFromIndex(guiTITLE);
  DeleteVideoObjectFromIndex(guiPersonnelInventory);

  return;
}

void RenderPersonnel(void) {
  struct VObject *hHandle;
  // re-renders personnel screen
  // render main background

  // blit title
  GetVideoObject(&hHandle, guiTITLE);
  BltVideoObject(FRAME_BUFFER, hHandle, 0, LAPTOP_SCREEN_UL_X, LAPTOP_SCREEN_UL_Y - 2,
                 VO_BLT_SRCTRANSPARENCY, NULL);

  // blit screen
  GetVideoObject(&hHandle, guiSCREEN);
  BltVideoObject(FRAME_BUFFER, hHandle, 0, LAPTOP_SCREEN_UL_X, LAPTOP_SCREEN_UL_Y + 22,
                 VO_BLT_SRCTRANSPARENCY, NULL);

  // render pictures of mercs on scnree
  // RenderPersonnelPictures( );

  // display header for screen
  // DisplayHeader( );

  // what page are we on?..display it
  // DrawPageNumber( );

  // display border
  // GetVideoObject(&hHandle, guiLaptopBACKGROUND);
  // BltVideoObject(FRAME_BUFFER, hHandle, 0,108, 23, VO_BLT_SRCTRANSPARENCY,NULL);

  // invalidte the region we blitted to
  // InvalidateRegion(LAPTOP_SCREEN_UL_X,LAPTOP_SCREEN_UL_Y,LAPTOP_SCREEN_LR_X,LAPTOP_SCREEN_LR_Y);

  // render personnel screen background
  RenderPersonnelScreenBackground();

  // show team
  DisplayPicturesOfCurrentTeam();

  DisplayPastMercsPortraits();

  // show selected merc
  DisplayFaceOfDisplayedMerc();

  // show current team size
  DisplayNumberOnCurrentTeam();

  // show departed team size
  DisplayNumberDeparted();

  // list stats row headers for team stats list
  DisplayPersonnelTeamStats();

  // showinventory of selected guy if applicable
  DisplayInventoryForSelectedChar();

  // the average stats for the current team
  DisplayAverageStatValuesForCurrentTeam();

  // lowest stat values
  DisplayLowestStatValuesForCurrentTeam();

  // past team
  DisplayStateOfPastTeamMembers();

  // title bar
  BlitTitleBarIcons();

  // show text on titlebar
  DisplayPersonnelTextOnTitleBar();

  // the highest stats
  DisplayHighestStatValuesForCurrentTeam();

  // render the atm panel
  RenderAtmPanel();

  DisplayAmountOnCurrentMerc();

  // en-dis-able start button
  UpDateStateOfStartButton();

  return;
}

BOOLEAN RenderPersonnelPictures(void) {
  // will render portraits of personnel onscreen
  // find person with iStartPersonId, unless it is -1, then find first bActive Merc on Staff
  struct SOLDIERTYPE *pTeamSoldier, *pSoldier;
  BOOLEAN fFound = FALSE;
  int32_t iSlot = 0;
  int32_t cnt = 0;
  int32_t iCurrentId = 0;

  pSoldier = MercPtrs[cnt];
  pTeamSoldier = pSoldier;

  if (iStartPersonId == -1) {
    cnt = gTacticalStatus.Team[pSoldier->bTeam].bFirstID;
    for (pSoldier = MercPtrs[cnt]; cnt <= gTacticalStatus.Team[pSoldier->bTeam].bLastID;
         cnt++, pSoldier++) {
      if (pSoldier->bLife >= OKLIFE && IsSolActive(pSoldier)) {
        fFound = TRUE;
        iStartPersonId = cnt;
        break;
      }
    }
    if (!fFound) return (FALSE);
  } else {
    iCurrentId = iStartPersonId;
    fFound = TRUE;
    cnt = iCurrentId;
  }

  while (fFound) {
    // the soldier's ID is found
    // render Face
    fFound = FALSE;
    RenderPersonnelFace(iCurrentId, iSlot, FALSE, FALSE, FALSE);
    // draw stats
    RenderPersonnelStats(iCurrentId, iSlot);
    DisplayCharName(iCurrentId, iSlot);
    // find next guy
    pSoldier = MercPtrs[iCurrentId];
    cnt++;
    for (pTeamSoldier = MercPtrs[cnt]; cnt <= gTacticalStatus.Team[pSoldier->bTeam].bLastID;
         cnt++, pTeamSoldier++) {
      if (pTeamSoldier->bLife >= OKLIFE && pTeamSoldier->bActive) {
        if (pTeamSoldier->uiStatusFlags & SOLDIER_VEHICLE) {
          return (FALSE);
        }

        fFound = TRUE;
        iSlot++;
        break;
      }
    }
    if (iSlot >= MAX_SLOTS) fFound = FALSE;
    iCurrentId = cnt;
  }

  return (TRUE);
}

void RenderPersonnelStats(int32_t iId, int32_t iSlot) {
  // will render the stats of person iId in slot iSlot
  SetFont(PERS_FONT);
  SetFontForeground(PERS_TEXT_FONT_COLOR);
  SetFontBackground(FONT_BLACK);

  if (gubPersonnelInfoState == PERSONNEL_STAT_BTN) {
    DisplayCharStats(iId, iSlot);
  } else if (gubPersonnelInfoState == PERSONNEL_EMPLOYMENT_BTN) {
    DisplayEmploymentinformation(iId, iSlot);
  }
}

BOOLEAN RenderPersonnelFace(int32_t iId, int32_t iSlot, BOOLEAN fDead, BOOLEAN fFired,
                            BOOLEAN fOther) {
  char sTemp[100];
  struct VObject *hFaceHandle;
  VOBJECT_DESC VObjectDesc;

  // draw face to soldier iId in slot iSlot

  // special case?..player generated merc
  if (fCurrentTeamMode == TRUE) {
    if ((50 < MercPtrs[iId]->ubProfile) && (57 > MercPtrs[iId]->ubProfile)) {
      sprintf(sTemp, "%s%03d.sti", FACES_DIR, gMercProfiles[MercPtrs[iId]->ubProfile].ubFaceIndex);
    } else {
      sprintf(sTemp, "%s%02d.sti", FACES_DIR, Menptr[iId].ubProfile);
    }
  } else {
    // if this is not a valid merc
    if (!fDead && !fFired && !fOther) {
      return (TRUE);
    }

    if ((50 < iId) && (57 > iId)) {
      sprintf(sTemp, "%s%03d.sti", FACES_DIR, gMercProfiles[iId].ubFaceIndex);
    } else {
      sprintf(sTemp, "%s%02d.sti", FACES_DIR, iId);
    }
  }

  if (fCurrentTeamMode == TRUE) {
    if (MercPtrs[iId]->uiStatusFlags & SOLDIER_VEHICLE) {
      return (TRUE);
    }
  }

  VObjectDesc.fCreateFlags = VOBJECT_CREATE_FROMFILE;
  FilenameForBPP(sTemp, VObjectDesc.ImageFile);
  CHECKF(AddVideoObject(&VObjectDesc, &guiFACE));

  // Blt face to screen to
  GetVideoObject(&hFaceHandle, guiFACE);

  if (fCurrentTeamMode == TRUE) {
    if (MercPtrs[iId]->bLife <= 0) {
      hFaceHandle->pShades[0] =
          Create16BPPPaletteShaded(hFaceHandle->pPaletteEntry, DEAD_MERC_COLOR_RED,
                                   DEAD_MERC_COLOR_GREEN, DEAD_MERC_COLOR_BLUE, TRUE);

      // set the red pallete to the face
      SetObjectHandleShade(guiFACE, 0);
    }
  } else {
    if (fDead == TRUE) {
      hFaceHandle->pShades[0] =
          Create16BPPPaletteShaded(hFaceHandle->pPaletteEntry, DEAD_MERC_COLOR_RED,
                                   DEAD_MERC_COLOR_GREEN, DEAD_MERC_COLOR_BLUE, TRUE);

      // set the red pallete to the face
      SetObjectHandleShade(guiFACE, 0);
    }
  }

  BltVideoObject(FRAME_BUFFER, hFaceHandle, 0, IMAGE_BOX_X + (iSlot * IMAGE_BOX_WIDTH), IMAGE_BOX_Y,
                 VO_BLT_SRCTRANSPARENCY, NULL);

  // if the merc is dead, display it
  if (!fCurrentTeamMode) {
    int32_t iHeightOfText;

    iHeightOfText = DisplayWrappedString(
        IMAGE_BOX_X, (uint16_t)(IMAGE_BOX_Y + IMAGE_FULL_NAME_OFFSET_Y), IMAGE_NAME_WIDTH, 1,
        PERS_FONT, PERS_FONT_COLOR, gMercProfiles[iId].zName, 0, FALSE,
        CENTER_JUSTIFIED | DONT_DISPLAY_TEXT);

    // if the string will rap
    if ((iHeightOfText - 2) > GetFontHeight(PERS_FONT)) {
      // raise where we display it, and rap it
      DisplayWrappedString(
          IMAGE_BOX_X,
          (uint16_t)(IMAGE_BOX_Y + IMAGE_FULL_NAME_OFFSET_Y - GetFontHeight(PERS_FONT)),
          IMAGE_NAME_WIDTH, 1, PERS_FONT, PERS_FONT_COLOR, gMercProfiles[iId].zName, 0, FALSE,
          CENTER_JUSTIFIED);
    } else {
      DrawTextToScreen(gMercProfiles[iId].zName, IMAGE_BOX_X,
                       (uint16_t)(IMAGE_BOX_Y + IMAGE_FULL_NAME_OFFSET_Y), IMAGE_NAME_WIDTH,
                       PERS_FONT, PERS_FONT_COLOR, 0, FALSE, CENTER_JUSTIFIED);
    }
    //		DrawTextToScreen(gMercProfiles[ iId  ].zName, ( int16_t ) (
    // IMAGE_BOX_X+(iSlot*IMAGE_BOX_WIDTH) ), ( int16_t ) ( IMAGE_BOX_Y + 107 ),
    // IMAGE_BOX_WITH_NO_BORDERS, PERS_FONT, PERS_FONT_COLOR, FONT_MCOLOR_BLACK, FALSE,
    // CENTER_JUSTIFIED	);
  }

  /*
  removed cause we already show this under the picture, instead display the mercs FULL name ( above
  ) if( fCurrentTeamMode == TRUE )
          {
                  if( Menptr[ iId ].bLife <= 0 )
                  {
                          //if the merc is dead, display it
                          DrawTextToScreen(pDepartedMercPortraitStrings[0], ( int16_t ) (
  IMAGE_BOX_X+(iSlot*IMAGE_BOX_WIDTH) ), ( int16_t ) ( IMAGE_BOX_Y + 107 ),
  IMAGE_BOX_WITH_NO_BORDERS, FONT14ARIAL, 145, FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED	);
                  }
          }
          else
          {
                  if( fDead )
                  {
                          //if the merc is dead, display it
                          DrawTextToScreen(pDepartedMercPortraitStrings[0], ( int16_t ) (
  IMAGE_BOX_X+(iSlot*IMAGE_BOX_WIDTH) ), ( int16_t ) ( IMAGE_BOX_Y + 107 ),
  IMAGE_BOX_WITH_NO_BORDERS, FONT14ARIAL, 145, FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED	);
  //			DrawTextToScreen( AimPopUpText[ AIM_MEMBER_DEAD ], ( int16_t ) (
  IMAGE_BOX_X+(iSlot*IMAGE_BOX_WIDTH) ), ( int16_t ) ( IMAGE_BOX_Y + 107 ),
  IMAGE_BOX_WITH_NO_BORDERS, FONT14ARIAL, 145, FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED	);
                  }
                  else if( fFired )
                  {
                          //if the merc is dead, display it
                          DrawTextToScreen(pDepartedMercPortraitStrings[1], ( int16_t ) (
  IMAGE_BOX_X+(iSlot*IMAGE_BOX_WIDTH) ), ( int16_t ) ( IMAGE_BOX_Y + 107 ),
  IMAGE_BOX_WITH_NO_BORDERS, FONT14ARIAL, 145, FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED	);
                  }
                  else if( fOther )
                  {
                          //if the merc is dead, display it
                          DrawTextToScreen(pDepartedMercPortraitStrings[2], ( int16_t ) (
  IMAGE_BOX_X+(iSlot*IMAGE_BOX_WIDTH) ), ( int16_t ) ( IMAGE_BOX_Y + 107 ),
  IMAGE_BOX_WITH_NO_BORDERS, FONT14ARIAL, 145, FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED	);
                  }
          }

  */

  DeleteVideoObjectFromIndex(guiFACE);

  return (TRUE);
}

BOOLEAN NextPersonnelFace(void) {
  if (iCurrentPersonSelectedId == -1) {
    return (TRUE);
  }

  if (fCurrentTeamMode == TRUE) {
    // wrap around?
    if (iCurrentPersonSelectedId == GetNumberOfMercsDeadOrAliveOnPlayersTeam() - 1) {
      iCurrentPersonSelectedId = 0;
      return (FALSE);  // def added 3/14/99 to enable disable buttons properly
    } else {
      iCurrentPersonSelectedId++;
    }
  } else {
    if (((iCurPortraitId + 1) == (GetNumberOfDeadOnPastTeam() + GetNumberOfLeftOnPastTeam() +
                                  GetNumberOfOtherOnPastTeam()) -
                                     giCurrentUpperLeftPortraitNumber)) {
      // about to go off the end
      giCurrentUpperLeftPortraitNumber = 0;
      iCurPortraitId = 0;

    } else if (iCurPortraitId == 19) {
      giCurrentUpperLeftPortraitNumber += 20;
      iCurPortraitId = 0;
    } else {
      iCurPortraitId++;
    }
    // get of this merc in this slot

    iCurrentPersonSelectedId = iCurPortraitId;
    fReDrawScreenFlag = TRUE;
  }

  return (TRUE);
}

BOOLEAN PrevPersonnelFace(void) {
  if (iCurrentPersonSelectedId == -1) {
    return (TRUE);
  }

  if (fCurrentTeamMode == TRUE) {
    // wrap around?
    if (iCurrentPersonSelectedId == 0) {
      iCurrentPersonSelectedId = GetNumberOfMercsDeadOrAliveOnPlayersTeam() - 1;

      if (iCurrentPersonSelectedId == 0) {
        return (FALSE);  // def added 3/14/99 to enable disable buttons properly
      }
    } else {
      iCurrentPersonSelectedId--;
    }
  } else {
    if ((iCurPortraitId == 0) && (giCurrentUpperLeftPortraitNumber == 0)) {
      // about to go off the end
      giCurrentUpperLeftPortraitNumber =
          (GetNumberOfDeadOnPastTeam() + GetNumberOfLeftOnPastTeam() +
           GetNumberOfOtherOnPastTeam()) -
          (GetNumberOfDeadOnPastTeam() + GetNumberOfLeftOnPastTeam() +
           GetNumberOfOtherOnPastTeam()) %
              20;
      iCurPortraitId = (GetNumberOfDeadOnPastTeam() + GetNumberOfLeftOnPastTeam() +
                        GetNumberOfOtherOnPastTeam()) %
                       20;
      iCurPortraitId--;

    } else if (iCurPortraitId == 0) {
      giCurrentUpperLeftPortraitNumber -= 20;
      iCurPortraitId = 19;
    } else {
      iCurPortraitId--;
    }
    // get of this merc in this slot

    iCurrentPersonSelectedId = iCurPortraitId;
    fReDrawScreenFlag = TRUE;
  }

  return (TRUE);
}

void CreatePersonnelButtons(void) {
  // left button
  giPersonnelButtonImage[0] = LoadButtonImage("LAPTOP\\personnelbuttons.sti", -1, 0, -1, 1, -1);
  giPersonnelButton[0] =
      QuickCreateButton(giPersonnelButtonImage[0], PREV_MERC_FACE_X, MERC_FACE_SCROLL_Y,
                        BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST - 1, BtnGenericMouseMoveButtonCallback,
                        (GUI_CALLBACK)LeftButtonCallBack);

  // right button
  giPersonnelButtonImage[1] = LoadButtonImage("LAPTOP\\personnelbuttons.sti", -1, 2, -1, 3, -1);
  giPersonnelButton[1] =
      QuickCreateButton(giPersonnelButtonImage[1], NEXT_MERC_FACE_X, MERC_FACE_SCROLL_Y,
                        BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST - 1, BtnGenericMouseMoveButtonCallback,
                        (GUI_CALLBACK)RightButtonCallBack);

  /*
  // left button
  giPersonnelButtonImage[0]=  LoadButtonImage( "LAPTOP\\arrows.sti" ,-1,0,-1,1,-1 );
  giPersonnelButton[0] = QuickCreateButton( giPersonnelButtonImage[0], LEFT_BUTTON_X, BUTTON_Y,
                                                                          BUTTON_TOGGLE,
  MSYS_PRIORITY_HIGHEST - 1, BtnGenericMouseMoveButtonCallback, (GUI_CALLBACK)LeftButtonCallBack);

  // right button
  giPersonnelButtonImage[1]=  LoadButtonImage( "LAPTOP\\arrows.sti" ,-1,6,-1,7,-1 );
  giPersonnelButton[1] = QuickCreateButton( giPersonnelButtonImage[1], RIGHT_BUTTON_X, BUTTON_Y,
                                                                          BUTTON_TOGGLE,
  MSYS_PRIORITY_HIGHEST - 1, BtnGenericMouseMoveButtonCallback, (GUI_CALLBACK)RightButtonCallBack);

  // left FF button
  giPersonnelButtonImage[2]=  LoadButtonImage( "LAPTOP\\arrows.sti" ,-1,3,-1,4,-1 );
  giPersonnelButton[2] = QuickCreateButton( giPersonnelButtonImage[2], LEFT_BUTTON_X, BUTTON_Y + 22,
                                                                          BUTTON_TOGGLE,
  MSYS_PRIORITY_HIGHEST - 1, BtnGenericMouseMoveButtonCallback, (GUI_CALLBACK)LeftFFButtonCallBack);

  // right ff button
  giPersonnelButtonImage[3]=  LoadButtonImage( "LAPTOP\\arrows.sti" ,-1,9,-1,10,-1 );
  giPersonnelButton[3] = QuickCreateButton( giPersonnelButtonImage[3], RIGHT_BUTTON_X, BUTTON_Y +
  22, BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST - 1, BtnGenericMouseMoveButtonCallback,
  (GUI_CALLBACK)RightFFButtonCallBack);
*/
  // set up cursors
  SetButtonCursor(giPersonnelButton[0], CURSOR_LAPTOP_SCREEN);
  SetButtonCursor(giPersonnelButton[1], CURSOR_LAPTOP_SCREEN);
  // SetButtonCursor(giPersonnelButton[2], CURSOR_LAPTOP_SCREEN);
  // SetButtonCursor(giPersonnelButton[3], CURSOR_LAPTOP_SCREEN);

  return;
}

void DeletePersonnelButtons(void) {
  RemoveButton(giPersonnelButton[0]);
  UnloadButtonImage(giPersonnelButtonImage[0]);
  RemoveButton(giPersonnelButton[1]);
  UnloadButtonImage(giPersonnelButtonImage[1]);
  /*RemoveButton(giPersonnelButton[2] );
  UnloadButtonImage( giPersonnelButtonImage[2] );
  RemoveButton(giPersonnelButton[3] );
  UnloadButtonImage( giPersonnelButtonImage[3] );
  */
  return;
}

void LeftButtonCallBack(GUI_BUTTON *btn, int32_t reason) {
  if (!(btn->uiFlags & BUTTON_ENABLED)) return;

  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    if (!(btn->uiFlags & BUTTON_CLICKED_ON)) {
      fReDrawScreenFlag = TRUE;
    }
    btn->uiFlags |= (BUTTON_CLICKED_ON);
  } else if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    if (btn->uiFlags & BUTTON_CLICKED_ON) {
      btn->uiFlags &= ~(BUTTON_CLICKED_ON);
      fReDrawScreenFlag = TRUE;
      PrevPersonnelFace();
      uiCurrentInventoryIndex = 0;
      guiSliderPosition = 0;
    }
  }
}

void LeftFFButtonCallBack(GUI_BUTTON *btn, int32_t reason) {
  if (!(btn->uiFlags & BUTTON_ENABLED)) return;

  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    if (!(btn->uiFlags & BUTTON_CLICKED_ON)) {
      fReDrawScreenFlag = TRUE;
    }
    btn->uiFlags |= (BUTTON_CLICKED_ON);
  } else if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    if (btn->uiFlags & BUTTON_CLICKED_ON) {
      btn->uiFlags &= ~(BUTTON_CLICKED_ON);
      fReDrawScreenFlag = TRUE;
      PrevPersonnelFace();
      PrevPersonnelFace();
      PrevPersonnelFace();
      PrevPersonnelFace();

      // set states
      SetPersonnelButtonStates();
    }
  }
}

void RightButtonCallBack(GUI_BUTTON *btn, int32_t reason) {
  if (!(btn->uiFlags & BUTTON_ENABLED)) return;

  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    if (!(btn->uiFlags & BUTTON_CLICKED_ON)) {
      fReDrawScreenFlag = TRUE;
    }
    btn->uiFlags |= (BUTTON_CLICKED_ON);
  } else if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    if (btn->uiFlags & BUTTON_CLICKED_ON) {
      btn->uiFlags &= ~(BUTTON_CLICKED_ON);
      fReDrawScreenFlag = TRUE;
      NextPersonnelFace();
      uiCurrentInventoryIndex = 0;
      guiSliderPosition = 0;
    }
  }
}

void RightFFButtonCallBack(GUI_BUTTON *btn, int32_t reason) {
  if (!(btn->uiFlags & BUTTON_ENABLED)) return;

  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    if (!(btn->uiFlags & BUTTON_CLICKED_ON)) {
      fReDrawScreenFlag = TRUE;
    }
    btn->uiFlags |= (BUTTON_CLICKED_ON);
  } else if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    if (btn->uiFlags & BUTTON_CLICKED_ON) {
      btn->uiFlags &= ~(BUTTON_CLICKED_ON);
      fReDrawScreenFlag = TRUE;
      NextPersonnelFace();
      NextPersonnelFace();
      NextPersonnelFace();
      NextPersonnelFace();

      // set states
      SetPersonnelButtonStates();
    }
  }
}

void DisplayHeader(void) {
  SetFont(PERS_HEADER_FONT);
  SetFontForeground(PERS_FONT_COLOR);
  SetFontBackground(0);

  mprintf(pPersonnelScreenPoints[18].x, pPersonnelScreenPoints[18].y, pPersonnelTitle[0]);

  return;
}

void DisplayCharName(int32_t iId, int32_t iSlot) {
  // get merc's nickName, assignment, and sector location info
  int16_t sX, sY;
  struct SOLDIERTYPE *pSoldier;
  CHAR16 sString[64];
  CHAR16 sTownName[256];
  TownID bTownId = -1;
  int32_t iHeightOfText;

  sTownName[0] = L'\0';

  pSoldier = MercPtrs[iId];

  SetFont(CHAR_NAME_FONT);
  SetFontForeground(PERS_TEXT_FONT_COLOR);
  SetFontBackground(FONT_BLACK);

  if (pSoldier->uiStatusFlags & SOLDIER_VEHICLE) {
    return;
  }

  if (Menptr[iId].bAssignment == ASSIGNMENT_POW) {
  } else if (Menptr[iId].bAssignment == IN_TRANSIT) {
  } else {
    // name of town, if any
    bTownId = GetTownIdForSector(Menptr[iId].sSectorX, Menptr[iId].sSectorY);

    if (bTownId != BLANK_SECTOR) {
      swprintf(sTownName, ARR_SIZE(sTownName), L"%s", pTownNames[bTownId]);
    }
  }

  if (sTownName[0] != L'\0') {
    // nick name - town name
    swprintf(sString, ARR_SIZE(sString), L"%s - %s", gMercProfiles[Menptr[iId].ubProfile].zNickname,
             sTownName);
  } else {
    // nick name
    swprintf(sString, ARR_SIZE(sString), L"%s", gMercProfiles[Menptr[iId].ubProfile].zNickname);
  }

  // nick name - assignment
  FindFontCenterCoordinates(IMAGE_BOX_X - 5, 0, IMAGE_BOX_WIDTH + 90, 0, sString, CHAR_NAME_FONT,
                            &sX, &sY);

  // check to see if we are going to go off the left edge
  if (sX < pPersonnelScreenPoints[0].x) {
    sX = (int16_t)pPersonnelScreenPoints[0].x;
  }

  // Display the mercs name
  mprintf(sX + iSlot * IMAGE_BOX_WIDTH, CHAR_NAME_Y, sString);

  swprintf(sString, ARR_SIZE(sString), L"%s", pPersonnelAssignmentStrings[Menptr[iId].bAssignment]);

  // nick name - assignment
  FindFontCenterCoordinates(IMAGE_BOX_X - 5, 0, IMAGE_BOX_WIDTH + 90, 0, sString, CHAR_NAME_FONT,
                            &sX, &sY);

  // check to see if we are going to go off the left edge
  if (sX < pPersonnelScreenPoints[0].x) {
    sX = (int16_t)pPersonnelScreenPoints[0].x;
  }

  mprintf(sX + iSlot * IMAGE_BOX_WIDTH, CHAR_LOC_Y, sString);

  //
  // Display the mercs FULL name over top of their portrait
  //

  // first get height of text to be displayed
  iHeightOfText = DisplayWrappedString(
      IMAGE_BOX_X, (uint16_t)(IMAGE_BOX_Y + IMAGE_FULL_NAME_OFFSET_Y), IMAGE_NAME_WIDTH, 1,
      PERS_FONT, PERS_FONT_COLOR, gMercProfiles[Menptr[iId].ubProfile].zName, 0, FALSE,
      CENTER_JUSTIFIED | DONT_DISPLAY_TEXT);

  // if the string will rap
  if ((iHeightOfText - 2) > GetFontHeight(PERS_FONT)) {
    // raise where we display it, and rap it
    DisplayWrappedString(
        IMAGE_BOX_X, (uint16_t)(IMAGE_BOX_Y + IMAGE_FULL_NAME_OFFSET_Y - GetFontHeight(PERS_FONT)),
        IMAGE_NAME_WIDTH, 1, PERS_FONT, PERS_FONT_COLOR, gMercProfiles[Menptr[iId].ubProfile].zName,
        0, FALSE, CENTER_JUSTIFIED);
  } else {
    DrawTextToScreen(gMercProfiles[Menptr[iId].ubProfile].zName, IMAGE_BOX_X,
                     (uint16_t)(IMAGE_BOX_Y + IMAGE_FULL_NAME_OFFSET_Y), IMAGE_NAME_WIDTH,
                     PERS_FONT, PERS_FONT_COLOR, 0, FALSE, CENTER_JUSTIFIED);
  }

  /*
  Moved so the name of the town will be in the same line as the name


          if( Menptr[iId].bAssignment == ASSIGNMENT_POW )
          {
  //		FindFontCenterCoordinates(IMAGE_BOX_X-5,0,IMAGE_BOX_WIDTH, 0,pPOWStrings[ 1
  ],CHAR_NAME_FONT, &sX, &sY );
  //	  mprintf(sX+iSlot*IMAGE_BOX_WIDTH, CHAR_NAME_Y+20,pPOWStrings[ 1 ] );
          }
          else if( Menptr[iId].bAssignment == IN_TRANSIT )
          {
                  return;
          }
          else
    {
                  // name of town, if any
                  bTownId = GetTownIdForSector( Menptr[iId].sSectorX, Menptr[iId].sSectorY );

                  if( bTownId != BLANK_SECTOR )
                  {
                          FindFontCenterCoordinates( IMAGE_BOX_X - 5, 0, IMAGE_BOX_WIDTH, 0,
  pTownNames[ bTownId ], CHAR_NAME_FONT, &sX, &sY ); mprintf( sX + ( iSlot * IMAGE_BOX_WIDTH ),
  CHAR_NAME_Y + 20, pTownNames[ bTownId ]);
                  }
          }
  */

  return;
}

void DisplayCharStats(int32_t iId, int32_t iSlot) {
  int32_t iCounter = 0;
  wchar_t sString[50];
  //	wchar_t sStringA[ 50 ];
  int16_t sX, sY;
  uint32_t uiHits = 0;
  struct SOLDIERTYPE *pSoldier = GetSoldierByID(iId);
  BOOLEAN fAmIaRobot = AM_A_ROBOT(pSoldier);

  if (pSoldier->uiStatusFlags & SOLDIER_VEHICLE) {
    return;
  }

  // display the stats for a char
  for (iCounter = 0; iCounter < MAX_STATS; iCounter++) {
    switch (iCounter) {
      case 0:
        // health
        if (Menptr[iId].bAssignment != ASSIGNMENT_POW) {
          if (gMercProfiles[Menptr[iId].ubProfile].bLifeDelta > 0) {
            swprintf(sString, ARR_SIZE(sString), L"( %+d )",
                     gMercProfiles[Menptr[iId].ubProfile].bLifeDelta);
            FindFontRightCoordinates(
                (int16_t)(pPersonnelScreenPoints[iCounter].x + (iSlot * TEXT_BOX_WIDTH) +
                          TEXT_BOX_WIDTH - 20 + TEXT_DELTA_OFFSET),
                0, 30, 0, sString, PERS_FONT, &sX, &sY);
            mprintf(sX, pPersonnelScreenPoints[iCounter].y, sString);
          }

          // else
          //{
          swprintf(sString, ARR_SIZE(sString), L"%d/%d", Menptr[iId].bLife, Menptr[iId].bLifeMax);
          //}
        } else {
          swprintf(sString, ARR_SIZE(sString), pPOWStrings[1]);
        }

        mprintf((int16_t)(pPersonnelScreenPoints[iCounter].x + (iSlot * TEXT_BOX_WIDTH)),
                pPersonnelScreenPoints[iCounter].y, pPersonnelScreenStrings[PRSNL_TXT_HEALTH]);
        FindFontRightCoordinates(
            (int16_t)(pPersonnelScreenPoints[iCounter].x + (iSlot * TEXT_BOX_WIDTH)), 0,
            TEXT_BOX_WIDTH - 20, 0, sString, PERS_FONT, &sX, &sY);
        mprintf(sX, pPersonnelScreenPoints[iCounter].y, sString);
        break;
      case 1:
        // agility
        if (!fAmIaRobot) {
          if (gMercProfiles[Menptr[iId].ubProfile].bAgilityDelta > 0) {
            swprintf(sString, ARR_SIZE(sString), L"( %+d )",
                     gMercProfiles[Menptr[iId].ubProfile].bAgilityDelta);
            FindFontRightCoordinates(
                (int16_t)(pPersonnelScreenPoints[iCounter].x + (iSlot * TEXT_BOX_WIDTH) +
                          TEXT_BOX_WIDTH - 20 + TEXT_DELTA_OFFSET),
                0, 30, 0, sString, PERS_FONT, &sX, &sY);
            mprintf(sX, pPersonnelScreenPoints[iCounter].y, sString);
          }
          // else
          //{
          swprintf(sString, ARR_SIZE(sString), L"%d", Menptr[iId].bAgility);
          //}
        } else {
          swprintf(sString, ARR_SIZE(sString), L"%s",
                   gpStrategicString[STR_PB_NOTAPPLICABLE_ABBREVIATION]);
        }

        mprintf((int16_t)(pPersonnelScreenPoints[iCounter].x + (iSlot * TEXT_BOX_WIDTH)),
                pPersonnelScreenPoints[iCounter].y, pPersonnelScreenStrings[iCounter]);
        FindFontRightCoordinates(
            (int16_t)(pPersonnelScreenPoints[iCounter].x + (iSlot * TEXT_BOX_WIDTH)), 0,
            TEXT_BOX_WIDTH - 20, 0, sString, PERS_FONT, &sX, &sY);
        mprintf(sX, pPersonnelScreenPoints[iCounter].y, sString);
        break;
      case 2:
        // dexterity
        if (!fAmIaRobot) {
          if (gMercProfiles[Menptr[iId].ubProfile].bDexterityDelta > 0) {
            swprintf(sString, ARR_SIZE(sString), L"( %+d )",
                     gMercProfiles[Menptr[iId].ubProfile].bDexterityDelta);
            FindFontRightCoordinates(
                (int16_t)(pPersonnelScreenPoints[iCounter].x + (iSlot * TEXT_BOX_WIDTH) +
                          TEXT_BOX_WIDTH - 20 + TEXT_DELTA_OFFSET),
                0, 30, 0, sString, PERS_FONT, &sX, &sY);
            mprintf(sX, pPersonnelScreenPoints[iCounter].y, sString);
          }
          // else
          //{
          swprintf(sString, ARR_SIZE(sString), L"%d", Menptr[iId].bDexterity);
          //}
        } else {
          swprintf(sString, ARR_SIZE(sString), L"%s",
                   gpStrategicString[STR_PB_NOTAPPLICABLE_ABBREVIATION]);
        }

        mprintf((int16_t)(pPersonnelScreenPoints[iCounter].x + (iSlot * TEXT_BOX_WIDTH)),
                pPersonnelScreenPoints[iCounter].y, pPersonnelScreenStrings[iCounter]);
        FindFontRightCoordinates(
            (int16_t)(pPersonnelScreenPoints[iCounter].x + (iSlot * TEXT_BOX_WIDTH)), 0,
            TEXT_BOX_WIDTH - 20, 0, sString, PERS_FONT, &sX, &sY);
        mprintf(sX, pPersonnelScreenPoints[iCounter].y, sString);
        break;
      case 3:
        // strength
        if (!fAmIaRobot) {
          if (gMercProfiles[Menptr[iId].ubProfile].bStrengthDelta > 0) {
            swprintf(sString, ARR_SIZE(sString), L"( %+d )",
                     gMercProfiles[Menptr[iId].ubProfile].bStrengthDelta);
            FindFontRightCoordinates(
                (int16_t)(pPersonnelScreenPoints[iCounter].x + (iSlot * TEXT_BOX_WIDTH) +
                          TEXT_BOX_WIDTH - 20 + TEXT_DELTA_OFFSET),
                0, 30, 0, sString, PERS_FONT, &sX, &sY);
            mprintf(sX, pPersonnelScreenPoints[iCounter].y, sString);
          }
          // else
          //{
          swprintf(sString, ARR_SIZE(sString), L"%d", Menptr[iId].bStrength);
          //}
        } else {
          swprintf(sString, ARR_SIZE(sString), L"%s",
                   gpStrategicString[STR_PB_NOTAPPLICABLE_ABBREVIATION]);
        }

        mprintf((int16_t)(pPersonnelScreenPoints[iCounter].x + (iSlot * TEXT_BOX_WIDTH)),
                pPersonnelScreenPoints[iCounter].y, pPersonnelScreenStrings[iCounter]);
        FindFontRightCoordinates(
            (int16_t)(pPersonnelScreenPoints[iCounter].x + (iSlot * TEXT_BOX_WIDTH)), 0,
            TEXT_BOX_WIDTH - 20, 0, sString, PERS_FONT, &sX, &sY);
        mprintf(sX, pPersonnelScreenPoints[iCounter].y, sString);
        break;
      case 4:
        // leadership
        if (!fAmIaRobot) {
          if (gMercProfiles[Menptr[iId].ubProfile].bLeadershipDelta > 0) {
            swprintf(sString, ARR_SIZE(sString), L"( %+d )",
                     gMercProfiles[Menptr[iId].ubProfile].bLeadershipDelta);
            FindFontRightCoordinates(
                (int16_t)(pPersonnelScreenPoints[iCounter].x + (iSlot * TEXT_BOX_WIDTH) +
                          TEXT_BOX_WIDTH - 20 + TEXT_DELTA_OFFSET),
                0, 30, 0, sString, PERS_FONT, &sX, &sY);
            mprintf(sX, pPersonnelScreenPoints[iCounter].y, sString);
          }
          // else
          //{
          swprintf(sString, ARR_SIZE(sString), L"%d", Menptr[iId].bLeadership);
          //}
        } else {
          swprintf(sString, ARR_SIZE(sString), L"%s",
                   gpStrategicString[STR_PB_NOTAPPLICABLE_ABBREVIATION]);
        }

        mprintf((int16_t)(pPersonnelScreenPoints[iCounter].x + (iSlot * TEXT_BOX_WIDTH)),
                pPersonnelScreenPoints[iCounter].y, pPersonnelScreenStrings[iCounter]);
        FindFontRightCoordinates(
            (int16_t)(pPersonnelScreenPoints[iCounter].x + (iSlot * TEXT_BOX_WIDTH)), 0,
            TEXT_BOX_WIDTH - 20, 0, sString, PERS_FONT, &sX, &sY);
        mprintf(sX, pPersonnelScreenPoints[iCounter].y, sString);
        break;
      case 5:
        // wisdom
        if (!fAmIaRobot) {
          if (gMercProfiles[Menptr[iId].ubProfile].bWisdomDelta > 0) {
            swprintf(sString, ARR_SIZE(sString), L"( %+d )",
                     gMercProfiles[Menptr[iId].ubProfile].bWisdomDelta);
            FindFontRightCoordinates(
                (int16_t)(pPersonnelScreenPoints[iCounter].x + (iSlot * TEXT_BOX_WIDTH) +
                          TEXT_BOX_WIDTH - 20 + TEXT_DELTA_OFFSET),
                0, 30, 0, sString, PERS_FONT, &sX, &sY);
            mprintf(sX, pPersonnelScreenPoints[iCounter].y, sString);
          }
          // else
          //{
          swprintf(sString, ARR_SIZE(sString), L"%d", Menptr[iId].bWisdom);
          //}
        } else {
          swprintf(sString, ARR_SIZE(sString), L"%s",
                   gpStrategicString[STR_PB_NOTAPPLICABLE_ABBREVIATION]);
        }

        mprintf((int16_t)(pPersonnelScreenPoints[iCounter].x + (iSlot * TEXT_BOX_WIDTH)),
                pPersonnelScreenPoints[iCounter].y, pPersonnelScreenStrings[iCounter]);
        FindFontRightCoordinates(
            (int16_t)(pPersonnelScreenPoints[iCounter].x + (iSlot * TEXT_BOX_WIDTH)), 0,
            TEXT_BOX_WIDTH - 20, 0, sString, PERS_FONT, &sX, &sY);
        mprintf(sX, pPersonnelScreenPoints[iCounter].y, sString);
        break;
      case 6:
        // exper
        if (!fAmIaRobot) {
          if (gMercProfiles[Menptr[iId].ubProfile].bExpLevelDelta > 0) {
            swprintf(sString, ARR_SIZE(sString), L"( %+d )",
                     gMercProfiles[Menptr[iId].ubProfile].bExpLevelDelta);
            FindFontRightCoordinates(
                (int16_t)(pPersonnelScreenPoints[iCounter].x + (iSlot * TEXT_BOX_WIDTH) +
                          TEXT_BOX_WIDTH - 20 + TEXT_DELTA_OFFSET),
                0, 30, 0, sString, PERS_FONT, &sX, &sY);
            mprintf(sX, pPersonnelScreenPoints[iCounter].y, sString);
          }
          // else
          //{
          swprintf(sString, ARR_SIZE(sString), L"%d", Menptr[iId].bExpLevel);
          //}
        } else {
          swprintf(sString, ARR_SIZE(sString), L"%s",
                   gpStrategicString[STR_PB_NOTAPPLICABLE_ABBREVIATION]);
        }

        mprintf((int16_t)(pPersonnelScreenPoints[iCounter].x + (iSlot * TEXT_BOX_WIDTH)),
                pPersonnelScreenPoints[iCounter].y, pPersonnelScreenStrings[iCounter]);
        FindFontRightCoordinates(
            (int16_t)(pPersonnelScreenPoints[iCounter].x + (iSlot * TEXT_BOX_WIDTH)), 0,
            TEXT_BOX_WIDTH - 20, 0, sString, PERS_FONT, &sX, &sY);
        mprintf(sX, pPersonnelScreenPoints[iCounter].y, sString);
        break;
      case 7:
        // mrkmanship
        if (!fAmIaRobot) {
          if (gMercProfiles[Menptr[iId].ubProfile].bMarksmanshipDelta > 0) {
            swprintf(sString, ARR_SIZE(sString), L"( %+d )",
                     gMercProfiles[Menptr[iId].ubProfile].bMarksmanshipDelta);
            FindFontRightCoordinates(
                (int16_t)(pPersonnelScreenPoints[iCounter].x + (iSlot * TEXT_BOX_WIDTH) +
                          TEXT_BOX_WIDTH - 20 + TEXT_DELTA_OFFSET),
                0, 30, 0, sString, PERS_FONT, &sX, &sY);
            mprintf(sX, pPersonnelScreenPoints[iCounter].y, sString);
          }
          // else
          //{
          swprintf(sString, ARR_SIZE(sString), L"%d", Menptr[iId].bMarksmanship);
          //}
        } else {
          swprintf(sString, ARR_SIZE(sString), L"%s",
                   gpStrategicString[STR_PB_NOTAPPLICABLE_ABBREVIATION]);
        }

        mprintf((int16_t)(pPersonnelScreenPoints[iCounter].x + (iSlot * TEXT_BOX_WIDTH)),
                pPersonnelScreenPoints[iCounter].y, pPersonnelScreenStrings[iCounter]);
        FindFontRightCoordinates(
            (int16_t)(pPersonnelScreenPoints[iCounter].x + (iSlot * TEXT_BOX_WIDTH)), 0,
            TEXT_BOX_WIDTH - 20, 0, sString, PERS_FONT, &sX, &sY);
        mprintf(sX, pPersonnelScreenPoints[iCounter].y, sString);
        break;
      case 8:
        // mech
        if (!fAmIaRobot) {
          if (gMercProfiles[Menptr[iId].ubProfile].bMechanicDelta > 0) {
            swprintf(sString, ARR_SIZE(sString), L"( %+d )",
                     gMercProfiles[Menptr[iId].ubProfile].bMechanicDelta);
            FindFontRightCoordinates(
                (int16_t)(pPersonnelScreenPoints[iCounter].x + (iSlot * TEXT_BOX_WIDTH) +
                          TEXT_BOX_WIDTH - 20 + TEXT_DELTA_OFFSET),
                0, 30, 0, sString, PERS_FONT, &sX, &sY);
            mprintf(sX, pPersonnelScreenPoints[iCounter].y, sString);
          }
          // else
          //{
          swprintf(sString, ARR_SIZE(sString), L"%d", Menptr[iId].bMechanical);
          //}
        } else {
          swprintf(sString, ARR_SIZE(sString), L"%s",
                   gpStrategicString[STR_PB_NOTAPPLICABLE_ABBREVIATION]);
        }

        mprintf((int16_t)(pPersonnelScreenPoints[iCounter].x + (iSlot * TEXT_BOX_WIDTH)),
                pPersonnelScreenPoints[iCounter].y, pPersonnelScreenStrings[iCounter]);
        FindFontRightCoordinates(
            (int16_t)(pPersonnelScreenPoints[iCounter].x + (iSlot * TEXT_BOX_WIDTH)), 0,
            TEXT_BOX_WIDTH - 20, 0, sString, PERS_FONT, &sX, &sY);
        mprintf(sX, pPersonnelScreenPoints[iCounter].y, sString);
        break;
      case 9:
        // exp
        if (!fAmIaRobot) {
          if (gMercProfiles[Menptr[iId].ubProfile].bExplosivesDelta > 0) {
            swprintf(sString, ARR_SIZE(sString), L"( %+d )",
                     gMercProfiles[Menptr[iId].ubProfile].bExplosivesDelta);
            FindFontRightCoordinates(
                (int16_t)(pPersonnelScreenPoints[iCounter].x + (iSlot * TEXT_BOX_WIDTH) +
                          TEXT_BOX_WIDTH - 20 + TEXT_DELTA_OFFSET),
                0, 30, 0, sString, PERS_FONT, &sX, &sY);
            mprintf(sX, pPersonnelScreenPoints[iCounter].y, sString);
          }
          // else
          //{
          swprintf(sString, ARR_SIZE(sString), L"%d", Menptr[iId].bExplosive);
          //}

        } else {
          swprintf(sString, ARR_SIZE(sString), L"%s",
                   gpStrategicString[STR_PB_NOTAPPLICABLE_ABBREVIATION]);
        }

        mprintf((int16_t)(pPersonnelScreenPoints[iCounter].x + (iSlot * TEXT_BOX_WIDTH)),
                pPersonnelScreenPoints[iCounter].y, pPersonnelScreenStrings[iCounter]);
        FindFontRightCoordinates(
            (int16_t)(pPersonnelScreenPoints[iCounter].x + (iSlot * TEXT_BOX_WIDTH)), 0,
            TEXT_BOX_WIDTH - 20, 0, sString, PERS_FONT, &sX, &sY);
        mprintf(sX, pPersonnelScreenPoints[iCounter].y, sString);
        break;
      case 10:
        // med
        if (!fAmIaRobot) {
          if (gMercProfiles[Menptr[iId].ubProfile].bMedicalDelta > 0) {
            swprintf(sString, ARR_SIZE(sString), L"( %+d )",
                     gMercProfiles[Menptr[iId].ubProfile].bMedicalDelta);
            FindFontRightCoordinates(
                (int16_t)(pPersonnelScreenPoints[iCounter].x + (iSlot * TEXT_BOX_WIDTH) +
                          TEXT_BOX_WIDTH - 20 + TEXT_DELTA_OFFSET),
                0, 30, 0, sString, PERS_FONT, &sX, &sY);
            mprintf(sX, pPersonnelScreenPoints[iCounter].y, sString);
          }
          // else
          //{
          swprintf(sString, ARR_SIZE(sString), L"%d", Menptr[iId].bMedical);
          //}
        } else {
          swprintf(sString, ARR_SIZE(sString), L"%s",
                   gpStrategicString[STR_PB_NOTAPPLICABLE_ABBREVIATION]);
        }

        mprintf((int16_t)(pPersonnelScreenPoints[iCounter].x + (iSlot * TEXT_BOX_WIDTH)),
                pPersonnelScreenPoints[iCounter].y, pPersonnelScreenStrings[iCounter]);
        FindFontRightCoordinates(
            (int16_t)(pPersonnelScreenPoints[iCounter].x + (iSlot * TEXT_BOX_WIDTH)), 0,
            TEXT_BOX_WIDTH - 20, 0, sString, PERS_FONT, &sX, &sY);
        mprintf(sX, pPersonnelScreenPoints[iCounter].y, sString);
        break;

      case 14:
        // kills
        mprintf((int16_t)(pPersonnelScreenPoints[21].x + (iSlot * TEXT_BOX_WIDTH)),
                pPersonnelScreenPoints[21].y, pPersonnelScreenStrings[PRSNL_TXT_KILLS]);
        swprintf(sString, ARR_SIZE(sString), L"%d", gMercProfiles[Menptr[iId].ubProfile].usKills);
        FindFontRightCoordinates((int16_t)(pPersonnelScreenPoints[21].x + (iSlot * TEXT_BOX_WIDTH)),
                                 0, TEXT_BOX_WIDTH - 20, 0, sString, PERS_FONT, &sX, &sY);
        mprintf(sX, pPersonnelScreenPoints[21].y, sString);
        break;
      case 15:
        // assists
        mprintf((int16_t)(pPersonnelScreenPoints[22].x + (iSlot * TEXT_BOX_WIDTH)),
                pPersonnelScreenPoints[22].y, pPersonnelScreenStrings[PRSNL_TXT_ASSISTS]);
        swprintf(sString, ARR_SIZE(sString), L"%d", gMercProfiles[Menptr[iId].ubProfile].usAssists);
        FindFontRightCoordinates((int16_t)(pPersonnelScreenPoints[22].x + (iSlot * TEXT_BOX_WIDTH)),
                                 0, TEXT_BOX_WIDTH - 20, 0, sString, PERS_FONT, &sX, &sY);
        mprintf(sX, pPersonnelScreenPoints[22].y, sString);
        break;
      case 16:
        // shots/hits
        mprintf((int16_t)(pPersonnelScreenPoints[23].x + (iSlot * TEXT_BOX_WIDTH)),
                pPersonnelScreenPoints[23].y, pPersonnelScreenStrings[PRSNL_TXT_HIT_PERCENTAGE]);
        uiHits = (uint32_t)gMercProfiles[Menptr[iId].ubProfile].usShotsHit;
        uiHits *= 100;

        // check we have shot at least once
        if (gMercProfiles[Menptr[iId].ubProfile].usShotsFired > 0) {
          uiHits /= (uint32_t)gMercProfiles[Menptr[iId].ubProfile].usShotsFired;
        } else {
          // no, set hit % to 0
          uiHits = 0;
        }

        swprintf(sString, ARR_SIZE(sString), L"%d %%%%", uiHits);
        FindFontRightCoordinates((int16_t)(pPersonnelScreenPoints[23].x + (iSlot * TEXT_BOX_WIDTH)),
                                 0, TEXT_BOX_WIDTH - 20, 0, sString, PERS_FONT, &sX, &sY);
        sX += StringPixLength(L"%", PERS_FONT);
        mprintf(sX, pPersonnelScreenPoints[23].y, sString);
        break;
      case 17:
        // battles
        mprintf((int16_t)(pPersonnelScreenPoints[24].x + (iSlot * TEXT_BOX_WIDTH)),
                pPersonnelScreenPoints[24].y, pPersonnelScreenStrings[PRSNL_TXT_BATTLES]);
        swprintf(sString, ARR_SIZE(sString), L"%d",
                 gMercProfiles[Menptr[iId].ubProfile].usBattlesFought);
        FindFontRightCoordinates((int16_t)(pPersonnelScreenPoints[24].x + (iSlot * TEXT_BOX_WIDTH)),
                                 0, TEXT_BOX_WIDTH - 20, 0, sString, PERS_FONT, &sX, &sY);
        mprintf(sX, pPersonnelScreenPoints[24].y, sString);
        break;
      case 18:
        // wounds
        mprintf((int16_t)(pPersonnelScreenPoints[25].x + (iSlot * TEXT_BOX_WIDTH)),
                pPersonnelScreenPoints[25].y, pPersonnelScreenStrings[PRSNL_TXT_TIMES_WOUNDED]);
        swprintf(sString, ARR_SIZE(sString), L"%d",
                 gMercProfiles[Menptr[iId].ubProfile].usTimesWounded);
        FindFontRightCoordinates((int16_t)(pPersonnelScreenPoints[25].x + (iSlot * TEXT_BOX_WIDTH)),
                                 0, TEXT_BOX_WIDTH - 20, 0, sString, PERS_FONT, &sX, &sY);
        mprintf(sX, pPersonnelScreenPoints[25].y, sString);
        break;

      // The Mercs Skills
      case 19: {
        int32_t iWidth;
        int32_t iMinimumX;
        int8_t bScreenLocIndex = 19;  // if you change the '19', change it below in the if statement

        // Display the 'Skills' text
        mprintf((int16_t)(pPersonnelScreenPoints[bScreenLocIndex].x + (iSlot * TEXT_BOX_WIDTH)),
                pPersonnelScreenPoints[bScreenLocIndex].y,
                pPersonnelScreenStrings[PRSNL_TXT_SKILLS]);

        // KM: April 16, 1999
        // Added support for the German version, which has potential string overrun problems.  For
        // example, the text "Skills:" can overlap "NightOps (Expert)" because the German strings
        // are much longer.  In these cases, I ensure that the right justification of the traits
        // don't overlap.  If it would, I move it over to the right.
        iWidth = StringPixLength(pPersonnelScreenStrings[PRSNL_TXT_SKILLS], PERS_FONT);
        iMinimumX = iWidth + pPersonnelScreenPoints[bScreenLocIndex].x + iSlot * TEXT_BOX_WIDTH + 2;

        if (!fAmIaRobot) {
          int8_t bSkill1 = gMercProfiles[Menptr[iId].ubProfile].bSkillTrait;
          int8_t bSkill2 = gMercProfiles[Menptr[iId].ubProfile].bSkillTrait2;

          // if the 2 skills are the same, add the '(expert)' at the end
          if (bSkill1 == bSkill2 && bSkill1 != NO_SKILLTRAIT) {
            swprintf(sString, ARR_SIZE(sString), L"%s %s", gzMercSkillText[bSkill1],
                     gzMercSkillText[NUM_SKILLTRAITS]);

            FindFontRightCoordinates(
                (int16_t)(pPersonnelScreenPoints[bScreenLocIndex].x + (iSlot * TEXT_BOX_WIDTH)), 0,
                TEXT_BOX_WIDTH - 20, 0, sString, PERS_FONT, &sX, &sY);

            // KM: April 16, 1999
            // Perform the potential overrun check
            if (sX <= iMinimumX) {
              FindFontRightCoordinates(
                  (int16_t)(pPersonnelScreenPoints[iCounter].x + (iSlot * TEXT_BOX_WIDTH) +
                            TEXT_BOX_WIDTH - 20 + TEXT_DELTA_OFFSET),
                  0, 30, 0, sString, PERS_FONT, &sX, &sY);
              sX = (int16_t)max(sX, iMinimumX);
            }

            mprintf(sX, pPersonnelScreenPoints[bScreenLocIndex].y, sString);
          } else {
            // Display the first skill
            if (bSkill1 != NO_SKILLTRAIT) {
              swprintf(sString, ARR_SIZE(sString), L"%s", gzMercSkillText[bSkill1]);

              FindFontRightCoordinates(
                  (int16_t)(pPersonnelScreenPoints[bScreenLocIndex].x + (iSlot * TEXT_BOX_WIDTH)),
                  0, TEXT_BOX_WIDTH - 20, 0, sString, PERS_FONT, &sX, &sY);

              // KM: April 16, 1999
              // Perform the potential overrun check
              sX = (int16_t)max(sX, iMinimumX);

              mprintf(sX, pPersonnelScreenPoints[bScreenLocIndex].y, sString);

              bScreenLocIndex++;
            }

            // Display the second skill
            if (bSkill2 != NO_SKILLTRAIT) {
              swprintf(sString, ARR_SIZE(sString), L"%s", gzMercSkillText[bSkill2]);

              FindFontRightCoordinates(
                  (int16_t)(pPersonnelScreenPoints[bScreenLocIndex].x + (iSlot * TEXT_BOX_WIDTH)),
                  0, TEXT_BOX_WIDTH - 20, 0, sString, PERS_FONT, &sX, &sY);

              // KM: April 16, 1999
              // Perform the potential overrun check
              sX = (int16_t)max(sX, iMinimumX);

              mprintf(sX, pPersonnelScreenPoints[bScreenLocIndex].y, sString);

              bScreenLocIndex++;
            }

            // if no skill was displayed
            if (bScreenLocIndex == 19) {
              swprintf(sString, ARR_SIZE(sString), L"%s",
                       pPersonnelScreenStrings[PRSNL_TXT_NOSKILLS]);

              FindFontRightCoordinates(
                  (int16_t)(pPersonnelScreenPoints[bScreenLocIndex].x + (iSlot * TEXT_BOX_WIDTH)),
                  0, TEXT_BOX_WIDTH - 20, 0, sString, PERS_FONT, &sX, &sY);
              mprintf(sX, pPersonnelScreenPoints[bScreenLocIndex].y, sString);
            }
          }
        } else {
          swprintf(sString, ARR_SIZE(sString), L"%s",
                   gpStrategicString[STR_PB_NOTAPPLICABLE_ABBREVIATION]);
        }
      } break;
        /*
                         case 19:
                                 // total contract time served
                                mprintf((int16_t)(pPersonnelScreenPoints[24].x+(iSlot*TEXT_BOX_WIDTH)),pPersonnelScreenPoints[24].y,pPersonnelScreenStrings[18]);
                                if( gMercProfiles[Menptr[iId].ubProfile].usTotalDaysServed > 0 )
                                {
                                        swprintf(sString, ARR_SIZE(sString), L"%d
           %s",gMercProfiles[Menptr[iId].ubProfile].usTotalDaysServed - 1, gpStrategicString[
           STR_PB_DAYS_ABBREVIATION ] );
                                }
                                else
                                {
                                        swprintf(sString, ARR_SIZE(sString), L"%d
           %s",gMercProfiles[Menptr[iId].ubProfile].usTotalDaysServed, gpStrategicString[
           STR_PB_DAYS_ABBREVIATION ] );
                                }
              FindFontRightCoordinates((int16_t)(pPersonnelScreenPoints[24].x+(iSlot*TEXT_BOX_WIDTH)),0,TEXT_BOX_WIDTH-20,0,sString,
           PERS_FONT,  &sX, &sY); mprintf(sX,pPersonnelScreenPoints[24].y,sString); break;
        */
    }
  }
  return;
}

int32_t GetLastMercId(void) {
  // rolls through list of mercs and returns how many on team
  struct SOLDIERTYPE *pSoldier, *pTeamSoldier;
  int32_t cnt = 0;
  int32_t iCounter = 0;
  pSoldier = MercPtrs[0];

  for (pTeamSoldier = MercPtrs[cnt]; cnt <= gTacticalStatus.Team[pSoldier->bTeam].bLastID;
       cnt++, pTeamSoldier++) {
    if ((pTeamSoldier->bActive) && (pTeamSoldier->bLife > 0)) iCounter++;
  }
  return iCounter;
}

void DrawPageNumber(void) {
  // draws the page number

  wchar_t sString[10];
  int16_t sX, sY;
  int32_t iPageNumber, iLastPage;

  return;

  // get last page number, and current page too
  iLastPage = GetLastMercId() / MAX_SLOTS;
  iPageNumber = iStartPersonId / MAX_SLOTS;
  iPageNumber++;
  if (iLastPage == 0) iLastPage++;

  // get current and last pages
  swprintf(sString, ARR_SIZE(sString), L"%d/%d", iPageNumber, iLastPage);

  // set up font
  SetFont(PERS_FONT);
  SetFontForeground(FONT_BLACK);
  SetFontBackground(FONT_BLACK);
  SetFontShadow(NO_SHADOW);

  // center
  FindFontCenterCoordinates(PAGE_X, PAGE_Y, PAGE_BOX_WIDTH, PAGE_BOX_HEIGHT, sString, PERS_FONT,
                            &sX, &sY);

  // print page number
  mprintf(sX, sY, sString);

  // reset shadow
  SetFontShadow(DEFAULT_SHADOW);

  return;
}

void SetPersonnelButtonStates(void) {
  // this function will look at what page we are viewing, enable and disable buttons as needed

  if (!PrevPersonnelFace()) {
    // first page, disable left buttons

    //		DisableButton( 	giPersonnelButton[ 2 ] );
    DisableButton(giPersonnelButton[0]);
  } else {
    // enable buttons
    NextPersonnelFace();

    // enable buttons
    //		EnableButton( giPersonnelButton[ 2 ] );
    EnableButton(giPersonnelButton[0]);
  }

  if (!NextPersonnelFace()) {
    //		DisableButton( 	giPersonnelButton[ 3 ] );
    DisableButton(giPersonnelButton[1]);
  } else {
    // decrement page
    PrevPersonnelFace();
    // enable buttons
    //		EnableButton( giPersonnelButton[ 3 ] );
    EnableButton(giPersonnelButton[1]);
  }

  return;
}

void RenderPersonnelScreenBackground(void) {
  struct VObject *hHandle;

  // this fucntion will render the background for the personnel screen
  if (fCurrentTeamMode == TRUE) {
    // blit title
    GetVideoObject(&hHandle, guiCURRENTTEAM);

  } else {
    // blit title
    GetVideoObject(&hHandle, guiDEPARTEDTEAM);
  }

  BltVideoObject(FRAME_BUFFER, hHandle, 0, DEPARTED_X, DEPARTED_Y, VO_BLT_SRCTRANSPARENCY, NULL);

  return;
}

BOOLEAN LoadPersonnelScreenBackgroundGraphics(void) {
  // will load the graphics for the personeel screen background
  VOBJECT_DESC VObjectDesc;

  // departed bar
  VObjectDesc.fCreateFlags = VOBJECT_CREATE_FROMFILE;
  FilenameForBPP("LAPTOP\\departed.sti", VObjectDesc.ImageFile);
  CHECKF(AddVideoObject(&VObjectDesc, &guiDEPARTEDTEAM));

  // current bar
  VObjectDesc.fCreateFlags = VOBJECT_CREATE_FROMFILE;
  FilenameForBPP("LAPTOP\\CurrentTeam.sti", VObjectDesc.ImageFile);
  CHECKF(AddVideoObject(&VObjectDesc, &guiCURRENTTEAM));

  return (TRUE);
}

void DeletePersonnelScreenBackgroundGraphics(void) {
  // delete background V/O's

  DeleteVideoObjectFromIndex(guiCURRENTTEAM);
  DeleteVideoObjectFromIndex(guiDEPARTEDTEAM);
}

void CreateDestroyButtonsForPersonnelDepartures(void) {
  static BOOLEAN fCreated = FALSE;

  // create/ destroy personnel departures buttons as needed

  // create button?..if not created
  if ((fCreatePeronnelDepartureButton == TRUE) && (fCreated == FALSE)) {
    fCreated = TRUE;
  } else if ((fCreatePeronnelDepartureButton == FALSE) && (fCreated == TRUE)) {
    fCreated = FALSE;
  }

  return;
}

int32_t GetNumberOfMercsOnPlayersTeam(void) {
  struct SOLDIERTYPE *pTeamSoldier, *pSoldier;
  int32_t cnt = 0;
  int32_t iCounter = 0;

  // grab number on team
  pSoldier = MercPtrs[0];

  // no soldiers

  for (pTeamSoldier = MercPtrs[cnt]; cnt <= gTacticalStatus.Team[pSoldier->bTeam].bLastID;
       cnt++, pTeamSoldier++) {
    if ((pTeamSoldier->bActive) && !(pTeamSoldier->uiStatusFlags & SOLDIER_VEHICLE) &&
        (pTeamSoldier->bLife > 0))
      iCounter++;
  }

  return (iCounter);
}

int32_t GetNumberOfMercsDeadOrAliveOnPlayersTeam(void) {
  struct SOLDIERTYPE *pTeamSoldier, *pSoldier;
  int32_t cnt = 0;
  int32_t iCounter = 0;

  // grab number on team
  pSoldier = MercPtrs[0];

  // no soldiers

  for (pTeamSoldier = MercPtrs[cnt]; cnt <= gTacticalStatus.Team[pSoldier->bTeam].bLastID;
       cnt++, pTeamSoldier++) {
    if ((pTeamSoldier->bActive) && !(pTeamSoldier->uiStatusFlags & SOLDIER_VEHICLE)) iCounter++;
  }

  return (iCounter);
}

void CreateDestroyMouseRegionsForPersonnelPortraits(void) {
  // creates/ destroys mouse regions for portraits

  static BOOLEAN fCreated = FALSE;
  int16_t sCounter = 0;

  if ((fCreated == FALSE) && (fCreatePersonnelPortraitMouseRegions == TRUE)) {
    // create regions
    for (sCounter = 0; sCounter < PERSONNEL_PORTRAIT_NUMBER; sCounter++) {
      MSYS_DefineRegion(
          &gPortraitMouseRegions[sCounter],
          (int16_t)(SMALL_PORTRAIT_START_X +
                    (sCounter % PERSONNEL_PORTRAIT_NUMBER_WIDTH) * SMALL_PORT_WIDTH),
          (int16_t)(SMALL_PORTRAIT_START_Y +
                    (sCounter / PERSONNEL_PORTRAIT_NUMBER_WIDTH) * SMALL_PORT_HEIGHT),
          (int16_t)((SMALL_PORTRAIT_START_X) +
                    ((sCounter % PERSONNEL_PORTRAIT_NUMBER_WIDTH) * SMALL_PORT_WIDTH) +
                    SMALL_PORTRAIT_WIDTH),
          (int16_t)(SMALL_PORTRAIT_START_Y +
                    (sCounter / PERSONNEL_PORTRAIT_NUMBER_WIDTH) * SMALL_PORT_HEIGHT +
                    SMALL_PORTRAIT_HEIGHT),
          MSYS_PRIORITY_HIGHEST, CURSOR_LAPTOP_SCREEN, MSYS_NO_CALLBACK, PersonnelPortraitCallback);
      MSYS_SetRegionUserData(&gPortraitMouseRegions[sCounter], 0, sCounter);
      MSYS_AddRegion(&gPortraitMouseRegions[sCounter]);
    }

    fCreated = TRUE;

  } else if ((fCreated == TRUE) && (fCreatePersonnelPortraitMouseRegions == FALSE)) {
    // destroy regions
    for (sCounter = 0; sCounter < PERSONNEL_PORTRAIT_NUMBER; sCounter++) {
      MSYS_RemoveRegion(&gPortraitMouseRegions[sCounter]);
    }

    fCreated = FALSE;
  }
  return;
}

BOOLEAN DisplayPicturesOfCurrentTeam(void) {
  int32_t iCounter = 0;
  int32_t iTotalOnTeam = 0;
  char sTemp[100];
  struct VObject *hFaceHandle;
  VOBJECT_DESC VObjectDesc;
  struct SOLDIERTYPE *pSoldier;
  int32_t iId = 0;
  int32_t iCnt = 0;

  // will display the 20 small portraits of the current team

  // get number of mercs on team
  iTotalOnTeam = GetNumberOfMercsDeadOrAliveOnPlayersTeam();

  if ((iTotalOnTeam == 0) || (fCurrentTeamMode == FALSE)) {
    // nobody on team, leave
    return (TRUE);
  }

  pSoldier = MercPtrs[iCounter];

  // start id
  iId = gTacticalStatus.Team[pSoldier->bTeam].bFirstID;

  for (iCounter = 0; iCounter < iTotalOnTeam; iCnt++) {
    if ((MercPtrs[iId + iCnt]->bActive == TRUE)) {
      // found the next actual guy
      if ((50 < MercPtrs[iId + iCnt]->ubProfile) && (57 > MercPtrs[iId + iCnt]->ubProfile)) {
        sprintf(sTemp, "%s%03d.sti", SMALL_FACES_DIR,
                gMercProfiles[MercPtrs[iId + iCnt]->ubProfile].ubFaceIndex);
      } else {
        if (Menptr[iId + iCnt].ubProfile < 100) {
          sprintf(sTemp, "%s%02d.sti", SMALL_FACES_DIR, Menptr[iId + iCnt].ubProfile);
        } else {
          sprintf(sTemp, "%s%03d.sti", SMALL_FACES_DIR, Menptr[iId + iCnt].ubProfile);
        }
      }

      VObjectDesc.fCreateFlags = VOBJECT_CREATE_FROMFILE;
      FilenameForBPP(sTemp, VObjectDesc.ImageFile);
      CHECKF(AddVideoObject(&VObjectDesc, &guiFACE));

      // Blt face to screen to
      GetVideoObject(&hFaceHandle, guiFACE);

      if (Menptr[iId + iCnt].bLife <= 0) {
        hFaceHandle->pShades[0] =
            Create16BPPPaletteShaded(hFaceHandle->pPaletteEntry, DEAD_MERC_COLOR_RED,
                                     DEAD_MERC_COLOR_GREEN, DEAD_MERC_COLOR_BLUE, TRUE);

        // set the red pallete to the face
        SetObjectHandleShade(guiFACE, 0);
      }

      BltVideoObject(FRAME_BUFFER, hFaceHandle, 0,
                     (int16_t)(SMALL_PORTRAIT_START_X +
                               (iCounter % PERSONNEL_PORTRAIT_NUMBER_WIDTH) * SMALL_PORT_WIDTH),
                     (int16_t)(SMALL_PORTRAIT_START_Y +
                               (iCounter / PERSONNEL_PORTRAIT_NUMBER_WIDTH) * SMALL_PORT_HEIGHT),
                     VO_BLT_SRCTRANSPARENCY, NULL);

      if (Menptr[iId + iCnt].bLife <= 0) {
        // if the merc is dead, display it
        DrawTextToScreen(
            AimPopUpText[AIM_MEMBER_DEAD],
            (int16_t)(SMALL_PORTRAIT_START_X +
                      (iCounter % PERSONNEL_PORTRAIT_NUMBER_WIDTH) * SMALL_PORT_WIDTH),
            (int16_t)(SMALL_PORTRAIT_START_Y +
                      (iCounter / PERSONNEL_PORTRAIT_NUMBER_WIDTH) * SMALL_PORT_HEIGHT +
                      SMALL_PORT_HEIGHT / 2),
            SMALL_PORTRAIT_WIDTH_NO_BORDERS, FONT10ARIAL, 145, FONT_MCOLOR_BLACK, FALSE,
            CENTER_JUSTIFIED);
      }

      DeleteVideoObjectFromIndex(guiFACE);
      iCounter++;
    }
  }

  return (TRUE);
}

void PersonnelPortraitCallback(struct MOUSE_REGION *pRegion, int32_t iReason) {
  int32_t iPortraitId = 0;
  int32_t iOldPortraitId;

  iPortraitId = MSYS_GetRegionUserData(pRegion, 0);
  iOldPortraitId = iCurrentPersonSelectedId;

  // callback handler for the minize region that is attatched to the laptop program icon
  if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    // get id of portrait

    if (fCurrentTeamMode == TRUE) {
      // valid portrait, set up id
      if (iPortraitId >= GetNumberOfMercsDeadOrAliveOnPlayersTeam()) {
        // not a valid id, leave
        return;
      }

      iCurrentPersonSelectedId = iPortraitId;
      fReDrawScreenFlag = TRUE;
    } else {
      if (iPortraitId >= GetNumberOfPastMercsOnPlayersTeam()) {
        return;
      }
      iCurrentPersonSelectedId = iPortraitId;
      fReDrawScreenFlag = TRUE;
      iCurPortraitId = iPortraitId;
    }

    // if the selected merc is valid, and they are a POW, change to the inventory display
    if (iCurrentPersonSelectedId != -1 &&
        Menptr[GetIdOfThisSlot(iCurrentPersonSelectedId)].bAssignment == ASSIGNMENT_POW &&
        gubPersonnelInfoState == PERSONNEL_INV_BTN) {
      gubPersonnelInfoState = PERSONNEL_STAT_BTN;
    }

    if (iOldPortraitId != iPortraitId) {
      uiCurrentInventoryIndex = 0;
      guiSliderPosition = 0;
    }
  }

  if (iReason & MSYS_CALLBACK_REASON_RBUTTON_UP) {
    if (fCurrentTeamMode == TRUE) {
      // valid portrait, set up id
      if (iPortraitId >= GetNumberOfMercsDeadOrAliveOnPlayersTeam()) {
        // not a valid id, leave
        return;
      }

      // if the user is rigt clicking on the same face
      if (iCurrentPersonSelectedId == iPortraitId) {
        // increment the info page when the user right clicks
        if (gubPersonnelInfoState < PERSONNEL_NUM_BTN - 1)
          gubPersonnelInfoState++;
        else
          gubPersonnelInfoState = PERSONNEL_STAT_BTN;
      }

      iCurrentPersonSelectedId = iPortraitId;
      fReDrawScreenFlag = TRUE;

      uiCurrentInventoryIndex = 0;
      guiSliderPosition = 0;

      // if the selected merc is valid, and they are a POW, change to the inventory display
      if (iCurrentPersonSelectedId != -1 &&
          Menptr[GetIdOfThisSlot(iCurrentPersonSelectedId)].bAssignment == ASSIGNMENT_POW &&
          gubPersonnelInfoState == PERSONNEL_INV_BTN) {
        gubPersonnelInfoState = PERSONNEL_STAT_BTN;
      }
    }
  }
}

void DisplayFaceOfDisplayedMerc() {
  // valid person?, display

  if (iCurrentPersonSelectedId != -1) {
    // highlight it
    DisplayHighLightBox();

    // if showing inventory, leave

    if (fCurrentTeamMode == TRUE) {
      RenderPersonnelFace(GetIdOfThisSlot(iCurrentPersonSelectedId), 0, FALSE, FALSE, FALSE);
      DisplayCharName(GetIdOfThisSlot(iCurrentPersonSelectedId), 0);

      //			if( fShowInventory == TRUE )
      if (gubPersonnelInfoState == PRSNL_INV) {
        return;
      }

      RenderPersonnelStats(GetIdOfThisSlot(iCurrentPersonSelectedId), 0);

    } else {
      RenderPersonnelFace(GetIdOfPastMercInSlot(iCurrentPersonSelectedId), 0,
                          IsPastMercDead(iCurrentPersonSelectedId),
                          IsPastMercFired(iCurrentPersonSelectedId),
                          IsPastMercOther(iCurrentPersonSelectedId));
      DisplayDepartedCharName(
          GetIdOfPastMercInSlot(iCurrentPersonSelectedId), 0,
          GetTheStateOfDepartedMerc(GetIdOfPastMercInSlot(iCurrentPersonSelectedId)));

      //			if( fShowInventory == TRUE )
      if (gubPersonnelInfoState == PRSNL_INV) {
        return;
      }

      DisplayDepartedCharStats(
          GetIdOfPastMercInSlot(iCurrentPersonSelectedId), 0,
          GetTheStateOfDepartedMerc(GetIdOfPastMercInSlot(iCurrentPersonSelectedId)));
    }
  }

  return;
}

void DisplayInventoryForSelectedChar(void) {
  // display the inventory for this merc
  //	if( fShowInventory == FALSE )
  if (gubPersonnelInfoState != PRSNL_INV) {
    return;
  }

  CreateDestroyPersonnelInventoryScrollButtons();

  if (fCurrentTeamMode == TRUE) {
    RenderInventoryForCharacter(GetIdOfThisSlot(iCurrentPersonSelectedId), 0);
  } else {
    RenderInventoryForCharacter(GetIdOfPastMercInSlot(iCurrentPersonSelectedId), 0);
  }

  return;
}

void RenderInventoryForCharacter(int32_t iId, int32_t iSlot) {
  uint8_t ubCounter = 0;
  struct SOLDIERTYPE *pSoldier;
  int16_t sIndex;
  struct VObject *hHandle;
  ETRLEObject *pTrav;
  INVTYPE *pItem;
  int16_t PosX, PosY, sCenX, sCenY;
  uint32_t usHeight, usWidth;
  uint8_t ubItemCount = 0;
  uint8_t ubUpToCount = 0;
  int16_t sX, sY;
  CHAR16 sString[128];
  int32_t cnt = 0;
  int32_t iTotalAmmo = 0;

  GetVideoObject(&hHandle, guiPersonnelInventory);
  BltVideoObject(FRAME_BUFFER, hHandle, 0, (int16_t)(397), (int16_t)(200), VO_BLT_SRCTRANSPARENCY,
                 NULL);

  if (fCurrentTeamMode == FALSE) {
    return;
  }

  // render the bar for the character
  RenderSliderBarForPersonnelInventory();

  pSoldier = GetSoldierByID(iId);

  // if this is a robot, dont display any inventory
  if (AM_A_ROBOT(pSoldier)) {
    return;
  }

  for (ubCounter = 0; ubCounter < NUM_INV_SLOTS; ubCounter++) {
    PosX = 397 + 3;
    PosY = 200 + 8 + (ubItemCount * (29));

    // if the character is a robot, only display the inv for the hand pos
    if (GetSolProfile(pSoldier) == ROBOT && ubCounter != HANDPOS) {
      continue;
    }

    if (pSoldier->inv[ubCounter].ubNumberOfObjects) {
      if (uiCurrentInventoryIndex > ubUpToCount) {
        ubUpToCount++;
      } else {
        sIndex = (pSoldier->inv[ubCounter].usItem);
        pItem = &Item[sIndex];

        GetVideoObject(&hHandle, GetInterfaceGraphicForItem(pItem));
        pTrav = &(hHandle->pETRLEObject[pItem->ubGraphicNum]);

        usHeight = (uint32_t)pTrav->usHeight;
        usWidth = (uint32_t)pTrav->usWidth;

        sCenX = PosX + (abs((int32_t)(57 - usWidth)) / 2) - pTrav->sOffsetX;
        sCenY = PosY + (abs((int32_t)(22 - usHeight)) / 2) - pTrav->sOffsetY;

        // shadow
        // BltVideoObjectOutlineShadowFromIndex( FRAME_BUFFER, GetInterfaceGraphicForItem( pItem ),
        // pItem->ubGraphicNum, sCenX-2, sCenY+2);

        // blt the item
        BltVideoObjectOutlineFromIndex(FRAME_BUFFER, GetInterfaceGraphicForItem(pItem),
                                       pItem->ubGraphicNum, sCenX, sCenY, 0, FALSE);

        SetFont(FONT10ARIAL);
        SetFontForeground(FONT_WHITE);
        SetFontBackground(FONT_BLACK);
        SetFontDestBuffer(FRAME_BUFFER, 0, 0, 640, 480, FALSE);

        // grab item name
        LoadItemInfo(sIndex, sString, NULL);

        // shorten if needed
        if (StringPixLength(sString, FONT10ARIAL) > (171 - 75)) {
          ReduceStringLength(sString, ARR_SIZE(sString), (171 - 75), FONT10ARIAL);
        }

        // print name
        mprintf(PosX + 65, PosY + 3, sString);

        // condition
        if (Item[pSoldier->inv[ubCounter].usItem].usItemClass & IC_AMMO) {
          // Ammo
          iTotalAmmo = 0;
          if (pSoldier->inv[ubCounter].ubNumberOfObjects > 1) {
            for (cnt = 0; cnt < pSoldier->inv[ubCounter].ubNumberOfObjects; cnt++) {
              // get total ammo
              iTotalAmmo += pSoldier->inv[ubCounter].ubShotsLeft[cnt];
            }
          } else {
            iTotalAmmo = pSoldier->inv[ubCounter].ubShotsLeft[0];
          }

          swprintf(sString, ARR_SIZE(sString), L"%d/%d", iTotalAmmo,
                   (pSoldier->inv[ubCounter].ubNumberOfObjects *
                    Magazine[Item[pSoldier->inv[ubCounter].usItem].ubClassIndex].ubMagSize));
          FindFontRightCoordinates((int16_t)(PosX + 65), (int16_t)(PosY + 15), (int16_t)(171 - 75),
                                   (int16_t)(GetFontHeight(FONT10ARIAL)), sString, FONT10ARIAL, &sX,
                                   &sY);
        } else {
          swprintf(sString, ARR_SIZE(sString), L"%2d%%%%", pSoldier->inv[ubCounter].bStatus[0]);
          FindFontRightCoordinates((int16_t)(PosX + 65), (int16_t)(PosY + 15), (int16_t)(171 - 75),
                                   (int16_t)(GetFontHeight(FONT10ARIAL)), sString, FONT10ARIAL, &sX,
                                   &sY);

          sX += StringPixLength(L"%", FONT10ARIAL);
        }

        mprintf(sX, sY, sString);

        if (Item[pSoldier->inv[ubCounter].usItem].usItemClass & IC_GUN) {
          swprintf(
              sString, ARR_SIZE(sString), L"%s",
              AmmoCaliber[Weapon[Item[pSoldier->inv[ubCounter].usItem].ubClassIndex].ubCalibre]);

          // shorten if needed
          if (StringPixLength(sString, FONT10ARIAL) > (171 - 75)) {
            ReduceStringLength(sString, ARR_SIZE(sString), (171 - 75), FONT10ARIAL);
          }

          // print name
          mprintf(PosX + 65, PosY + 15, sString);
        }

        // if more than 1?
        if (pSoldier->inv[ubCounter].ubNumberOfObjects > 1) {
          swprintf(sString, ARR_SIZE(sString), L"x%d", pSoldier->inv[ubCounter].ubNumberOfObjects);
          FindFontRightCoordinates((int16_t)(PosX), (int16_t)(PosY + 15), (int16_t)(58),
                                   (int16_t)(GetFontHeight(FONT10ARIAL)), sString, FONT10ARIAL, &sX,
                                   &sY);
          mprintf(sX, sY, sString);
        }

        // display info about it

        ubItemCount++;
      }
    }

    if (ubItemCount == NUMBER_OF_INVENTORY_PERSONNEL) {
      ubCounter = NUM_INV_SLOTS;
    }
  }

  return;
}

void InventoryUpButtonCallback(GUI_BUTTON *btn, int32_t reason) {
  if (!(btn->uiFlags & BUTTON_ENABLED)) return;

  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    btn->uiFlags |= (BUTTON_CLICKED_ON);
  } else if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    if (btn->uiFlags & (BUTTON_CLICKED_ON)) {
      btn->uiFlags &= ~(BUTTON_CLICKED_ON);

      if (uiCurrentInventoryIndex == 0) {
        return;
      }

      // up one element
      uiCurrentInventoryIndex--;
      fReDrawScreenFlag = TRUE;

      FindPositionOfPersInvSlider();
    }
  } else if (reason & MSYS_CALLBACK_REASON_LBUTTON_REPEAT) {
    if (uiCurrentInventoryIndex == 0) {
      return;
    }

    // up one element
    uiCurrentInventoryIndex--;
    fReDrawScreenFlag = TRUE;
    FindPositionOfPersInvSlider();
  }
}

void InventoryDownButtonCallback(GUI_BUTTON *btn, int32_t reason) {
  if (!(btn->uiFlags & BUTTON_ENABLED)) return;

  if (reason & MSYS_CALLBACK_REASON_LBUTTON_REPEAT) {
    if ((int32_t)uiCurrentInventoryIndex >=
        (int32_t)(GetNumberOfInventoryItemsOnCurrentMerc() - NUMBER_OF_INVENTORY_PERSONNEL)) {
      return;
    }

    // up one element
    uiCurrentInventoryIndex++;
    fReDrawScreenFlag = TRUE;
    FindPositionOfPersInvSlider();

  } else if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    btn->uiFlags |= (BUTTON_CLICKED_ON);
  } else if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    if (btn->uiFlags & (BUTTON_CLICKED_ON)) {
      btn->uiFlags &= ~(BUTTON_CLICKED_ON);

      if ((int32_t)uiCurrentInventoryIndex >=
          (int32_t)(GetNumberOfInventoryItemsOnCurrentMerc() - NUMBER_OF_INVENTORY_PERSONNEL)) {
        return;
      }

      // up one element
      uiCurrentInventoryIndex++;
      fReDrawScreenFlag = TRUE;

      FindPositionOfPersInvSlider();
    }
  }
}

// decide which buttons can and can't be accessed based on what the current item is
void EnableDisableInventoryScrollButtons(void) {
  //	if( fShowInventory == FALSE )
  if (gubPersonnelInfoState != PRSNL_INV) {
    return;
  }

  if (uiCurrentInventoryIndex == 0) {
    ButtonList[giPersonnelInventoryButtons[0]]->uiFlags &= ~(BUTTON_CLICKED_ON);
    DisableButton(giPersonnelInventoryButtons[0]);
  } else {
    EnableButton(giPersonnelInventoryButtons[0]);
  }

  if ((int32_t)uiCurrentInventoryIndex >=
      (int32_t)(GetNumberOfInventoryItemsOnCurrentMerc() - NUMBER_OF_INVENTORY_PERSONNEL)) {
    ButtonList[giPersonnelInventoryButtons[1]]->uiFlags &= ~(BUTTON_CLICKED_ON);
    DisableButton(giPersonnelInventoryButtons[1]);
  } else {
    EnableButton(giPersonnelInventoryButtons[1]);
  }

  return;
}

int32_t GetNumberOfInventoryItemsOnCurrentMerc(void) {
  int32_t iId = 0;
  uint8_t ubCounter = 0;
  uint8_t ubCount = 0;
  struct SOLDIERTYPE *pSoldier;

  // in current team mode?..nope...move on
  if (fCurrentTeamMode == FALSE) {
    return (0);
  }

  iId = GetIdOfThisSlot(iCurrentPersonSelectedId);

  pSoldier = GetSoldierByID(iId);

  for (ubCounter = 0; ubCounter < NUM_INV_SLOTS; ubCounter++) {
    if ((pSoldier->inv[ubCounter].ubNumberOfObjects) && (pSoldier->inv[ubCounter].usItem)) {
      ubCount++;
    }
  }

  return (ubCount);
}

void CreateDestroyPersonnelInventoryScrollButtons(void) {
  static BOOLEAN fCreated = FALSE;

  //	if( ( fShowInventory == TRUE ) && ( fCreated == FALSE ) )
  if ((gubPersonnelInfoState == PRSNL_INV) && (fCreated == FALSE)) {
    // create buttons
    giPersonnelInventoryButtonsImages[0] =
        LoadButtonImage("LAPTOP\\personnel_inventory.sti", -1, 1, -1, 2, -1);
    giPersonnelInventoryButtons[0] =
        QuickCreateButton(giPersonnelInventoryButtonsImages[0], 176 + 397, 2 + 200, BUTTON_TOGGLE,
                          MSYS_PRIORITY_HIGHEST - 1, BtnGenericMouseMoveButtonCallback,
                          (GUI_CALLBACK)InventoryUpButtonCallback);

    giPersonnelInventoryButtonsImages[1] =
        LoadButtonImage("LAPTOP\\personnel_inventory.sti", -1, 3, -1, 4, -1);
    giPersonnelInventoryButtons[1] =
        QuickCreateButton(giPersonnelInventoryButtonsImages[1], 397 + 176, 200 + 223, BUTTON_TOGGLE,
                          MSYS_PRIORITY_HIGHEST - 1, BtnGenericMouseMoveButtonCallback,
                          (GUI_CALLBACK)InventoryDownButtonCallback);

    // set up cursors for these buttons
    SetButtonCursor(giPersonnelInventoryButtons[0], CURSOR_LAPTOP_SCREEN);
    SetButtonCursor(giPersonnelInventoryButtons[1], CURSOR_LAPTOP_SCREEN);

    MSYS_DefineRegion(
        &gMouseScrollPersonnelINV, X_OF_PERSONNEL_SCROLL_REGION, Y_OF_PERSONNEL_SCROLL_REGION,
        X_OF_PERSONNEL_SCROLL_REGION + X_SIZE_OF_PERSONNEL_SCROLL_REGION,
        Y_OF_PERSONNEL_SCROLL_REGION + Y_SIZE_OF_PERSONNEL_SCROLL_REGION, MSYS_PRIORITY_HIGHEST - 3,
        CURSOR_LAPTOP_SCREEN, MSYS_NO_CALLBACK, HandleSliderBarClickCallback);

    fCreated = TRUE;
  }
  //	else if( ( fCreated == TRUE ) && ( fShowInventory == FALSE ) )
  else if ((fCreated == TRUE) && (gubPersonnelInfoState != PERSONNEL_INV_BTN)) {
    // destroy buttons
    RemoveButton(giPersonnelInventoryButtons[0]);
    UnloadButtonImage(giPersonnelInventoryButtonsImages[0]);
    RemoveButton(giPersonnelInventoryButtons[1]);
    UnloadButtonImage(giPersonnelInventoryButtonsImages[1]);

    MSYS_RemoveRegion(&gMouseScrollPersonnelINV);

    fCreated = FALSE;
  }
}

void DisplayNumberOnCurrentTeam(void) {
  // display number on team
  CHAR16 sString[32];
  int16_t sX = 0, sY = 0;

  // font stuff
  SetFont(FONT10ARIAL);
  SetFontBackground(FONT_BLACK);
  SetFontForeground(PERS_TEXT_FONT_COLOR);

  if (fCurrentTeamMode == TRUE) {
    swprintf(sString, ARR_SIZE(sString), L"%s ( %d )", pPersonelTeamStrings[0],
             GetNumberOfMercsDeadOrAliveOnPlayersTeam());
    sX = PERS_CURR_TEAM_X;
  } else {
    swprintf(sString, ARR_SIZE(sString), L"%s", pPersonelTeamStrings[0]);
    FindFontCenterCoordinates(PERS_CURR_TEAM_X, 0, 65, 0, sString, FONT10ARIAL, &sX, &sY);
  }

  mprintf(sX, PERS_CURR_TEAM_Y, sString);

  // now the cost of the current team, if applicable
  DisplayCostOfCurrentTeam();

  return;
}

void DisplayNumberDeparted(void) {
  // display number departed from team
  CHAR16 sString[32];
  int16_t sX = 0, sY = 0;

  // font stuff
  SetFont(FONT10ARIAL);
  SetFontBackground(FONT_BLACK);
  SetFontForeground(PERS_TEXT_FONT_COLOR);

  if (fCurrentTeamMode == FALSE) {
    swprintf(sString, ARR_SIZE(sString), L"%s ( %d )", pPersonelTeamStrings[1],
             GetNumberOfPastMercsOnPlayersTeam());
    sX = PERS_CURR_TEAM_X;
  } else {
    swprintf(sString, ARR_SIZE(sString), L"%s", pPersonelTeamStrings[1]);
    FindFontCenterCoordinates(PERS_CURR_TEAM_X, 0, 65, 0, sString, FONT10ARIAL, &sX, &sY);
  }

  mprintf(sX, PERS_DEPART_TEAM_Y, sString);

  return;
}

int32_t GetTotalDailyCostOfCurrentTeam(void) {
  // will return the total daily cost of the current team

  struct SOLDIERTYPE *pSoldier;
  int32_t cnt = 0;
  int32_t iCostOfTeam = 0;

  // first grunt
  pSoldier = MercPtrs[0];

  // not active?..return cost of zero

  // run through active soldiers
  for (pSoldier = MercPtrs[0]; cnt <= gTacticalStatus.Team[OUR_TEAM].bLastID; cnt++) {
    pSoldier = MercPtrs[cnt];

    if ((IsSolActive(pSoldier)) && IsSolAlive(pSoldier)) {
      // valid soldier, get cost
      if (pSoldier->ubWhatKindOfMercAmI == MERC_TYPE__AIM_MERC) {
        // daily rate
        if (pSoldier->bTypeOfLastContract == CONTRACT_EXTEND_2_WEEK) {
          // 2 week contract
          iCostOfTeam += gMercProfiles[GetSolProfile(pSoldier)].uiBiWeeklySalary / 14;
        } else if (pSoldier->bTypeOfLastContract == CONTRACT_EXTEND_1_WEEK) {
          // 1 week contract
          iCostOfTeam += gMercProfiles[GetSolProfile(pSoldier)].uiWeeklySalary / 7;
        } else {
          iCostOfTeam += gMercProfiles[GetSolProfile(pSoldier)].sSalary;
        }
      } else if (pSoldier->ubWhatKindOfMercAmI == MERC_TYPE__MERC) {
        // MERC Merc
        iCostOfTeam += gMercProfiles[GetSolProfile(pSoldier)].sSalary;
      } else {
        // no cost
        iCostOfTeam += 0;
      }
    }
  }
  return iCostOfTeam;
}

int32_t GetLowestDailyCostOfCurrentTeam(void) {
  // will return the lowest daily cost of the current team

  struct SOLDIERTYPE *pSoldier;
  int32_t cnt = 0;
  int32_t iLowest = 999999;
  int32_t iCost = 0;

  // first grunt
  pSoldier = MercPtrs[0];

  // not active?..return cost of zero

  // run through active soldiers
  for (pSoldier = MercPtrs[0]; cnt <= gTacticalStatus.Team[OUR_TEAM].bLastID; cnt++) {
    pSoldier = MercPtrs[cnt];

    if ((IsSolActive(pSoldier)) && !(pSoldier->uiStatusFlags & SOLDIER_VEHICLE) &&
        IsSolAlive(pSoldier)) {
      // valid soldier, get cost
      if (pSoldier->ubWhatKindOfMercAmI == MERC_TYPE__AIM_MERC) {
        // daily rate
        if (pSoldier->bTypeOfLastContract == CONTRACT_EXTEND_2_WEEK) {
          // 2 week contract
          iCost = gMercProfiles[GetSolProfile(pSoldier)].uiBiWeeklySalary / 14;
        } else if (pSoldier->bTypeOfLastContract == CONTRACT_EXTEND_1_WEEK) {
          // 1 week contract
          iCost = gMercProfiles[GetSolProfile(pSoldier)].uiWeeklySalary / 7;
        } else {
          iCost = gMercProfiles[GetSolProfile(pSoldier)].sSalary;
        }
      } else if (pSoldier->ubWhatKindOfMercAmI == MERC_TYPE__MERC) {
        // MERC Merc
        iCost = gMercProfiles[GetSolProfile(pSoldier)].sSalary;
      } else {
        // no cost
        iCost = 0;
      }

      if (iCost <= iLowest) {
        iLowest = iCost;
      }
    }
  }

  // if no mercs, send 0
  if (iLowest == 999999) {
    iLowest = 0;
  }

  return iLowest;
}

int32_t GetHighestDailyCostOfCurrentTeam(void) {
  // will return the lowest daily cost of the current team

  struct SOLDIERTYPE *pSoldier;
  int32_t cnt = 0;
  int32_t iHighest = 0;
  int32_t iCost = 0;

  // first grunt
  pSoldier = MercPtrs[0];

  // not active?..return cost of zero

  // run through active soldiers
  for (pSoldier = MercPtrs[0]; cnt <= gTacticalStatus.Team[OUR_TEAM].bLastID; cnt++) {
    pSoldier = MercPtrs[cnt];

    if ((IsSolActive(pSoldier)) && !(pSoldier->uiStatusFlags & SOLDIER_VEHICLE) &&
        IsSolAlive(pSoldier)) {
      // valid soldier, get cost
      if (pSoldier->ubWhatKindOfMercAmI == MERC_TYPE__AIM_MERC) {
        // daily rate
        if (pSoldier->bTypeOfLastContract == CONTRACT_EXTEND_2_WEEK) {
          // 2 week contract
          iCost = gMercProfiles[GetSolProfile(pSoldier)].uiBiWeeklySalary / 14;
        } else if (pSoldier->bTypeOfLastContract == CONTRACT_EXTEND_1_WEEK) {
          // 1 week contract
          iCost = gMercProfiles[GetSolProfile(pSoldier)].uiWeeklySalary / 7;
        } else {
          iCost = gMercProfiles[GetSolProfile(pSoldier)].sSalary;
        }
      } else if (pSoldier->ubWhatKindOfMercAmI == MERC_TYPE__MERC) {
        // MERC Merc
        iCost = gMercProfiles[GetSolProfile(pSoldier)].sSalary;
      } else {
        // no cost
        iCost = 0;
      }

      if (iCost >= iHighest) {
        iHighest = iCost;
      }
    }
  }
  return iHighest;
}

void DisplayCostOfCurrentTeam(void) {
  // display number on team
  CHAR16 sString[32];
  int16_t sX, sY;

  // font stuff
  SetFont(FONT10ARIAL);
  SetFontBackground(FONT_BLACK);
  SetFontForeground(PERS_TEXT_FONT_COLOR);

  if (fCurrentTeamMode == TRUE) {
    // daily cost
    mprintf(PERS_CURR_TEAM_COST_X, PERS_CURR_TEAM_COST_Y, pPersonelTeamStrings[2]);

    swprintf(sString, ARR_SIZE(sString), L"%d", GetTotalDailyCostOfCurrentTeam());
    InsertCommasForDollarFigure(sString);
    InsertDollarSignInToString(sString);

    FindFontRightCoordinates((int16_t)(PERS_CURR_TEAM_COST_X), 0, PERS_CURR_TEAM_WIDTH, 0, sString,
                             PERS_FONT, &sX, &sY);

    mprintf(sX, PERS_CURR_TEAM_COST_Y, sString);

    // highest cost
    mprintf(PERS_CURR_TEAM_COST_X, PERS_CURR_TEAM_HIGHEST_Y, pPersonelTeamStrings[3]);

    swprintf(sString, ARR_SIZE(sString), L"%d", GetHighestDailyCostOfCurrentTeam());
    InsertCommasForDollarFigure(sString);
    InsertDollarSignInToString(sString);

    FindFontRightCoordinates((int16_t)(PERS_CURR_TEAM_COST_X), 0, PERS_CURR_TEAM_WIDTH, 0, sString,
                             PERS_FONT, &sX, &sY);

    mprintf(sX, PERS_CURR_TEAM_HIGHEST_Y, sString);

    // the lowest cost
    mprintf(PERS_CURR_TEAM_COST_X, PERS_CURR_TEAM_LOWEST_Y, pPersonelTeamStrings[4]);

    swprintf(sString, ARR_SIZE(sString), L"%d", GetLowestDailyCostOfCurrentTeam());
    InsertCommasForDollarFigure(sString);
    InsertDollarSignInToString(sString);

    FindFontRightCoordinates((int16_t)(PERS_CURR_TEAM_COST_X), 0, PERS_CURR_TEAM_WIDTH, 0, sString,
                             PERS_FONT, &sX, &sY);

    mprintf(sX, PERS_CURR_TEAM_LOWEST_Y, sString);

  } else {
    // do nothing
    return;
  }
}

int32_t GetIdOfDepartedMercWithHighestStat(int32_t iStat) {
  // will return the id value of the merc on the players team with highest in this stat
  // -1 means error
  int32_t iId = -1;
  int32_t iValue = 0;
  MERCPROFILESTRUCT *pTeamSoldier;
  int32_t cnt = 0;
  int8_t bCurrentList = 0;
  int16_t *bCurrentListValue = LaptopSaveInfo.ubDeadCharactersList;
  BOOLEAN fNotDone = TRUE;
  struct SOLDIERTYPE *pSoldier;
  uint32_t uiLoopCounter;

  // run through active soldiers
  //	while( fNotDone )
  for (uiLoopCounter = 0; fNotDone; uiLoopCounter++) {
    /*
                    // check if we are in fact not done
                    if( ( bCurrentList == 2 ) && ( *bCurrentListValue == -1 ) )
                    {
                            fNotDone = FALSE;
                            continue;
                    }
    */
    // if we are at the end of
    if (uiLoopCounter == 255 && bCurrentList == 2) {
      fNotDone = FALSE;
      continue;
    }

    // check if we need to move to the next list
    //		if( *bCurrentListValue == -1 )
    if (uiLoopCounter == 255) {
      if (bCurrentList == 0) {
        bCurrentList = 1;
        bCurrentListValue = LaptopSaveInfo.ubLeftCharactersList;
      } else if (bCurrentList == 1) {
        bCurrentList = 2;
        bCurrentListValue = LaptopSaveInfo.ubOtherCharactersList;
      }

      // reset the loop counter
      uiLoopCounter = 0;
    }

    // get the id of the grunt
    cnt = *bCurrentListValue;

    // do we need to reset the count?
    if (cnt == -1) {
      bCurrentListValue++;
      continue;
    }

    pTeamSoldier = &(gMercProfiles[cnt]);

    switch (iStat) {
      case 0:
        // health

        // if the soldier is a pow, dont use the health cause it aint known
        pSoldier = FindSoldierByProfileID((uint8_t)cnt, FALSE);
        if (pSoldier && GetSolAssignment(pSoldier) == ASSIGNMENT_POW) {
          continue;
        }

        if (pTeamSoldier->bLife >= iValue) {
          iId = cnt;
          iValue = pTeamSoldier->bLife;
        }
        break;
      case 1:
        // agility
        if (pTeamSoldier->bAgility >= iValue) {
          iId = cnt;
          iValue = pTeamSoldier->bAgility;
        }
        break;
      case 2:
        // dexterity
        if (pTeamSoldier->bDexterity >= iValue) {
          iId = cnt;
          iValue = pTeamSoldier->bDexterity;
        }
        break;
      case 3:
        // strength
        if (pTeamSoldier->bStrength >= iValue) {
          iId = cnt;
          iValue = pTeamSoldier->bStrength;
        }
        break;
      case 4:
        // leadership
        if (pTeamSoldier->bLeadership >= iValue) {
          iId = cnt;
          iValue = pTeamSoldier->bLeadership;
        }
        break;
      case 5:
        // wisdom
        if (pTeamSoldier->bWisdom >= iValue) {
          iId = cnt;
          iValue = pTeamSoldier->bWisdom;
        }
        break;
      case 6:
        // exper
        if (pTeamSoldier->bExpLevel >= iValue) {
          iId = cnt;
          iValue = pTeamSoldier->bExpLevel;
        }

        break;
      case 7:
        // mrkmanship
        if (pTeamSoldier->bMarksmanship >= iValue) {
          iId = cnt;
          iValue = pTeamSoldier->bMarksmanship;
        }

        break;
      case 8:
        // mech
        if (pTeamSoldier->bMechanical >= iValue) {
          iId = cnt;
          iValue = pTeamSoldier->bMechanical;
        }
        break;
      case 9:
        // exp
        if (pTeamSoldier->bExplosive >= iValue) {
          iId = cnt;
          iValue = pTeamSoldier->bExplosive;
        }
        break;
      case 10:
        // med
        if (pTeamSoldier->bMedical >= iValue) {
          iId = cnt;
          iValue = pTeamSoldier->bMedical;
        }
        break;
    }

    bCurrentListValue++;
  }

  return (iId);
}

int32_t GetIdOfDepartedMercWithLowestStat(int32_t iStat) {
  // will return the id value of the merc on the players team with highest in this stat
  // -1 means error
  int32_t iId = -1;
  int32_t iValue = 9999999;
  MERCPROFILESTRUCT *pTeamSoldier;
  int32_t cnt = 0;
  int8_t bCurrentList = 0;
  int16_t *bCurrentListValue = LaptopSaveInfo.ubDeadCharactersList;
  BOOLEAN fNotDone = TRUE;
  struct SOLDIERTYPE *pSoldier;
  uint32_t uiLoopCounter;

  // run through active soldiers
  //	while( fNotDone )
  for (uiLoopCounter = 0; fNotDone; uiLoopCounter++) {
    /*
                    // check if we are in fact not done
                    if( ( bCurrentList == 2 ) && ( *bCurrentListValue == -1 ) )
                    {
                            fNotDone = FALSE;
                            continue;
                    }
    */
    // if we are at the end of
    if (uiLoopCounter == 255 && bCurrentList == 2) {
      fNotDone = FALSE;
      continue;
    }

    // check if we need to move to the next list
    //		if( *bCurrentListValue == -1 )
    if (uiLoopCounter == 255) {
      if (bCurrentList == 0) {
        bCurrentList = 1;
        bCurrentListValue = LaptopSaveInfo.ubLeftCharactersList;
      } else if (bCurrentList == 1) {
        bCurrentList = 2;
        bCurrentListValue = LaptopSaveInfo.ubOtherCharactersList;
      }

      // reset the loop counter
      uiLoopCounter = 0;
    }

    // get the id of the grunt
    cnt = *bCurrentListValue;

    // do we need to reset the count?
    if (cnt == -1) {
      bCurrentListValue++;
      continue;
    }

    pTeamSoldier = &(gMercProfiles[cnt]);

    switch (iStat) {
      case 0:
        // health

        pSoldier = FindSoldierByProfileID((uint8_t)cnt, FALSE);
        if (pSoldier && GetSolAssignment(pSoldier) == ASSIGNMENT_POW) {
          continue;
        }

        if (pTeamSoldier->bLife < iValue) {
          iId = cnt;
          iValue = pTeamSoldier->bLife;
        }
        break;
      case 1:
        // agility
        if (pTeamSoldier->bAgility < iValue) {
          iId = cnt;
          iValue = pTeamSoldier->bAgility;
        }
        break;
      case 2:
        // dexterity
        if (pTeamSoldier->bDexterity < iValue) {
          iId = cnt;
          iValue = pTeamSoldier->bDexterity;
        }
        break;
      case 3:
        // strength
        if (pTeamSoldier->bStrength < iValue) {
          iId = cnt;
          iValue = pTeamSoldier->bStrength;
        }
        break;
      case 4:
        // leadership
        if (pTeamSoldier->bLeadership < iValue) {
          iId = cnt;
          iValue = pTeamSoldier->bLeadership;
        }
        break;
      case 5:
        // wisdom
        if (pTeamSoldier->bWisdom < iValue) {
          iId = cnt;
          iValue = pTeamSoldier->bWisdom;
        }
        break;
      case 6:
        // exper
        if (pTeamSoldier->bExpLevel < iValue) {
          iId = cnt;
          iValue = pTeamSoldier->bExpLevel;
        }

        break;
      case 7:
        // mrkmanship
        if (pTeamSoldier->bMarksmanship < iValue) {
          iId = cnt;
          iValue = pTeamSoldier->bMarksmanship;
        }

        break;
      case 8:
        // mech
        if (pTeamSoldier->bMechanical < iValue) {
          iId = cnt;
          iValue = pTeamSoldier->bMechanical;
        }
        break;
      case 9:
        // exp
        if (pTeamSoldier->bExplosive < iValue) {
          iId = cnt;
          iValue = pTeamSoldier->bExplosive;
        }
        break;
      case 10:
        // med
        if (pTeamSoldier->bMedical < iValue) {
          iId = cnt;
          iValue = pTeamSoldier->bMedical;
        }
        break;
    }

    bCurrentListValue++;
  }

  return (iId);
}

int32_t GetIdOfMercWithHighestStat(int32_t iStat) {
  // will return the id value of the merc on the players team with highest in this stat
  // -1 means error
  int32_t iId = -1;
  int32_t iValue = 0;
  struct SOLDIERTYPE *pTeamSoldier, *pSoldier;
  int32_t cnt = 0;

  // first grunt
  pSoldier = MercPtrs[0];

  // run through active soldiers
  for (pTeamSoldier = MercPtrs[cnt]; cnt <= gTacticalStatus.Team[pSoldier->bTeam].bLastID;
       cnt++, pTeamSoldier++) {
    if ((pTeamSoldier->bActive) && !(pTeamSoldier->uiStatusFlags & SOLDIER_VEHICLE) &&
        (pTeamSoldier->bLife > 0) && !AM_A_ROBOT(pTeamSoldier)) {
      switch (iStat) {
        case 0:
          // health
          if (pTeamSoldier->bAssignment == ASSIGNMENT_POW) {
            continue;
          }

          if (pTeamSoldier->bLifeMax >= iValue) {
            iId = cnt;
            iValue = pTeamSoldier->bLifeMax;
          }
          break;
        case 1:
          // agility
          if (pTeamSoldier->bAgility >= iValue) {
            iId = cnt;
            iValue = pTeamSoldier->bAgility;
          }
          break;
        case 2:
          // dexterity
          if (pTeamSoldier->bDexterity >= iValue) {
            iId = cnt;
            iValue = pTeamSoldier->bDexterity;
          }
          break;
        case 3:
          // strength
          if (pTeamSoldier->bStrength >= iValue) {
            iId = cnt;
            iValue = pTeamSoldier->bStrength;
          }
          break;
        case 4:
          // leadership
          if (pTeamSoldier->bLeadership >= iValue) {
            iId = cnt;
            iValue = pTeamSoldier->bLeadership;
          }
          break;
        case 5:
          // wisdom
          if (pTeamSoldier->bWisdom >= iValue) {
            iId = cnt;
            iValue = pTeamSoldier->bWisdom;
          }
          break;
        case 6:
          // exper
          if (pTeamSoldier->bExpLevel >= iValue) {
            iId = cnt;
            iValue = pTeamSoldier->bExpLevel;
          }

          break;
        case 7:
          // mrkmanship
          if (pTeamSoldier->bMarksmanship >= iValue) {
            iId = cnt;
            iValue = pTeamSoldier->bMarksmanship;
          }

          break;
        case 8:
          // mech
          if (pTeamSoldier->bMechanical >= iValue) {
            iId = cnt;
            iValue = pTeamSoldier->bMechanical;
          }
          break;
        case 9:
          // exp
          if (pTeamSoldier->bExplosive >= iValue) {
            iId = cnt;
            iValue = pTeamSoldier->bExplosive;
          }
          break;
        case 10:
          // med
          if (pTeamSoldier->bMedical >= iValue) {
            iId = cnt;
            iValue = pTeamSoldier->bMedical;
          }
          break;
      }
    }
  }

  return (iId);
}

int32_t GetIdOfMercWithLowestStat(int32_t iStat) {
  // will return the id value of the merc on the players team with highest in this stat
  // -1 means error
  int32_t iId = -1;
  int32_t iValue = 999999;
  struct SOLDIERTYPE *pTeamSoldier, *pSoldier;
  int32_t cnt = 0;

  // first grunt
  pSoldier = MercPtrs[0];

  // run through active soldiers
  for (pTeamSoldier = MercPtrs[cnt]; cnt <= gTacticalStatus.Team[pSoldier->bTeam].bLastID;
       cnt++, pTeamSoldier++) {
    if ((pTeamSoldier->bActive) && !(pTeamSoldier->uiStatusFlags & SOLDIER_VEHICLE) &&
        (pTeamSoldier->bLife > 0) && !AM_A_ROBOT(pTeamSoldier)) {
      switch (iStat) {
        case 0:
          // health

          if (pTeamSoldier->bAssignment == ASSIGNMENT_POW) {
            continue;
          }

          if (pTeamSoldier->bLifeMax <= iValue) {
            iId = cnt;
            iValue = pTeamSoldier->bLifeMax;
          }
          break;
        case 1:
          // agility
          if (pTeamSoldier->bAgility <= iValue) {
            iId = cnt;
            iValue = pTeamSoldier->bAgility;
          }
          break;
        case 2:
          // dexterity
          if (pTeamSoldier->bDexterity <= iValue) {
            iId = cnt;
            iValue = pTeamSoldier->bDexterity;
          }
          break;
        case 3:
          // strength
          if (pTeamSoldier->bStrength <= iValue) {
            iId = cnt;
            iValue = pTeamSoldier->bStrength;
          }
          break;
        case 4:
          // leadership
          if (pTeamSoldier->bLeadership <= iValue) {
            iId = cnt;
            iValue = pTeamSoldier->bLeadership;
          }
          break;
        case 5:
          // wisdom
          if (pTeamSoldier->bWisdom <= iValue) {
            iId = cnt;
            iValue = pTeamSoldier->bWisdom;
          }
          break;
        case 6:
          // exper
          if (pTeamSoldier->bExpLevel <= iValue) {
            iId = cnt;
            iValue = pTeamSoldier->bExpLevel;
          }

          break;
        case 7:
          // mrkmanship
          if (pTeamSoldier->bMarksmanship <= iValue) {
            iId = cnt;
            iValue = pTeamSoldier->bMarksmanship;
          }

          break;
        case 8:
          // mech
          if (pTeamSoldier->bMechanical <= iValue) {
            iId = cnt;
            iValue = pTeamSoldier->bMechanical;
          }
          break;
        case 9:
          // exp
          if (pTeamSoldier->bExplosive <= iValue) {
            iId = cnt;
            iValue = pTeamSoldier->bExplosive;
          }
          break;
        case 10:
          // med
          if (pTeamSoldier->bMedical <= iValue) {
            iId = cnt;
            iValue = pTeamSoldier->bMedical;
          }
          break;
      }
    }
  }

  return (iId);
}

int32_t GetAvgStatOfCurrentTeamStat(int32_t iStat) {
  // will return the id value of the merc on the players team with highest in this stat
  // -1 means error
  struct SOLDIERTYPE *pTeamSoldier, *pSoldier;
  int32_t cnt = 0;
  int32_t iTotalStatValue = 0;
  int8_t bNumberOfPows = 0;
  uint8_t ubNumberOfMercsInCalculation = 0;

  // first grunt
  pSoldier = MercPtrs[0];

  // run through active soldiers
  for (pTeamSoldier = MercPtrs[cnt]; cnt <= gTacticalStatus.Team[pSoldier->bTeam].bLastID;
       cnt++, pTeamSoldier++) {
    if ((pTeamSoldier->bActive) && (pTeamSoldier->bLife > 0) && !AM_A_ROBOT(pTeamSoldier)) {
      switch (iStat) {
        case 0:
          // health

          // if this is a pow, dont count his stats
          if (pTeamSoldier->bAssignment == ASSIGNMENT_POW) {
            bNumberOfPows++;
            continue;
          }

          iTotalStatValue += pTeamSoldier->bLifeMax;

          break;
        case 1:
          // agility
          iTotalStatValue += pTeamSoldier->bAgility;

          break;
        case 2:
          // dexterity
          iTotalStatValue += pTeamSoldier->bDexterity;

          break;
        case 3:
          // strength
          iTotalStatValue += pTeamSoldier->bStrength;

          break;
        case 4:
          // leadership
          iTotalStatValue += pTeamSoldier->bLeadership;

          break;
        case 5:
          // wisdom

          iTotalStatValue += pTeamSoldier->bWisdom;
          break;
        case 6:
          // exper

          iTotalStatValue += pTeamSoldier->bExpLevel;

          break;
        case 7:
          // mrkmanship

          iTotalStatValue += pTeamSoldier->bMarksmanship;

          break;
        case 8:
          // mech

          iTotalStatValue += pTeamSoldier->bMechanical;
          break;
        case 9:
          // exp

          iTotalStatValue += pTeamSoldier->bExplosive;
          break;
        case 10:
          // med

          iTotalStatValue += pTeamSoldier->bMedical;
          break;
      }

      ubNumberOfMercsInCalculation++;
    }
  }

  // if the stat is health, and there are only pow's
  if (GetNumberOfMercsOnPlayersTeam() != 0 && GetNumberOfMercsOnPlayersTeam() == bNumberOfPows &&
      iStat == 0) {
    return (-1);
  } else if ((ubNumberOfMercsInCalculation - bNumberOfPows) > 0) {
    return (iTotalStatValue / (ubNumberOfMercsInCalculation - bNumberOfPows));
  } else {
    return (0);
  }
}

int32_t GetAvgStatOfPastTeamStat(int32_t iStat) {
  // will return the id value of the merc on the players team with highest in this stat
  // -1 means error
  int32_t cnt = 0;
  int32_t iTotalStatValue = 0;
  MERCPROFILESTRUCT *pTeamSoldier;
  int8_t bCurrentList = 0;
  int16_t *bCurrentListValue = LaptopSaveInfo.ubDeadCharactersList;
  BOOLEAN fNotDone = TRUE;
  uint32_t uiLoopCounter;

  // run through active soldiers

  // while( fNotDone )
  for (uiLoopCounter = 0; fNotDone; uiLoopCounter++) {
    /*
                    // check if we are in fact not done
                    if( ( bCurrentList == 2 ) && ( *bCurrentListValue == -1 ) )
                    {
                            fNotDone = FALSE;
                            continue;
                    }
    */

    // if we are at the end of
    if (uiLoopCounter == 255 && bCurrentList == 2) {
      fNotDone = FALSE;
      continue;
    }

    // check if we need to move to the next list
    //		if( *bCurrentListValue == -1 )
    if (uiLoopCounter == 255) {
      if (bCurrentList == 0) {
        bCurrentList = 1;
        bCurrentListValue = LaptopSaveInfo.ubLeftCharactersList;
      } else if (bCurrentList == 1) {
        bCurrentList = 2;
        bCurrentListValue = LaptopSaveInfo.ubOtherCharactersList;
      }

      // reset the loop counter
      uiLoopCounter = 0;
    }

    // get the id of the grunt
    cnt = *bCurrentListValue;

    // do we need to reset the count?
    if (cnt == -1) {
      bCurrentListValue++;
      continue;
    }

    pTeamSoldier = &(gMercProfiles[cnt]);

    switch (iStat) {
      case 0:
        // health

        iTotalStatValue += pTeamSoldier->bLife;

        break;
      case 1:
        // agility

        iTotalStatValue += pTeamSoldier->bAgility;

        break;
      case 2:
        // dexterity

        iTotalStatValue += pTeamSoldier->bDexterity;

        break;
      case 3:
        // strength

        iTotalStatValue += pTeamSoldier->bStrength;

        break;
      case 4:
        // leadership

        iTotalStatValue += pTeamSoldier->bLeadership;

        break;
      case 5:
        // wisdom

        iTotalStatValue += pTeamSoldier->bWisdom;

        break;
      case 6:
        // exper

        iTotalStatValue += pTeamSoldier->bExpLevel;

        break;
      case 7:
        // mrkmanship

        iTotalStatValue += pTeamSoldier->bMarksmanship;

        break;
      case 8:
        // mech

        iTotalStatValue += pTeamSoldier->bMechanical;

        break;
      case 9:
        // exp

        iTotalStatValue += pTeamSoldier->bExplosive;

        break;
      case 10:
        // med

        iTotalStatValue += pTeamSoldier->bMedical;
        break;
    }

    bCurrentListValue++;
  }

  if (GetNumberOfPastMercsOnPlayersTeam() > 0) {
    return (iTotalStatValue / GetNumberOfPastMercsOnPlayersTeam());
  } else {
    return (0);
  }
}

void DisplayAverageStatValuesForCurrentTeam(void) {
  // will display the average values for stats for the current team
  int16_t sX, sY;
  int32_t iCounter = 0;
  CHAR16 sString[32];

  // set up font
  SetFont(FONT10ARIAL);
  SetFontBackground(FONT_BLACK);
  SetFontForeground(PERS_TEXT_FONT_COLOR);

  // display header

  // center
  FindFontCenterCoordinates(PERS_STAT_AVG_X, 0, PERS_STAT_AVG_WIDTH, 0,
                            pPersonnelCurrentTeamStatsStrings[1], FONT10ARIAL, &sX, &sY);

  mprintf(sX, PERS_STAT_AVG_Y, pPersonnelCurrentTeamStatsStrings[1]);

  // nobody on team leave
  if ((GetNumberOfMercsDeadOrAliveOnPlayersTeam() == 0) && (fCurrentTeamMode == TRUE)) {
    return;
  }

  // check if in past team and nobody on past team
  if ((GetNumberOfPastMercsOnPlayersTeam() == 0) && (fCurrentTeamMode == FALSE)) {
    return;
  }

  for (iCounter = 0; iCounter < 11; iCounter++) {
    // even or odd?..color black or yellow?
    if (iCounter % 2 == 0) {
      SetFontForeground(PERS_TEXT_FONT_ALTERNATE_COLOR);
    } else {
      SetFontForeground(PERS_TEXT_FONT_COLOR);
    }

    if (fCurrentTeamMode == TRUE) {
      int32_t iValue = GetAvgStatOfCurrentTeamStat(iCounter);

      // if there are no values
      if (iValue == -1)
        swprintf(sString, ARR_SIZE(sString), L"%s", pPOWStrings[1]);
      else
        swprintf(sString, ARR_SIZE(sString), L"%d", iValue);

    } else {
      swprintf(sString, ARR_SIZE(sString), L"%d", GetAvgStatOfPastTeamStat(iCounter));
    }
    // center
    FindFontCenterCoordinates(PERS_STAT_AVG_X, 0, PERS_STAT_AVG_WIDTH, 0, sString, FONT10ARIAL, &sX,
                              &sY);

    mprintf(sX, PERS_STAT_AVG_Y + (iCounter + 1) * (GetFontHeight(FONT10ARIAL) + 3), sString);
  }

  return;
}

void DisplayLowestStatValuesForCurrentTeam(void) {
  // will display the average values for stats for the current team
  int16_t sX, sY;
  int32_t iCounter = 0;
  CHAR16 sString[32];
  int32_t iStat = 0;
  int32_t iDepartedId = 0;
  int32_t iId = 0;

  // set up font
  SetFont(FONT10ARIAL);
  SetFontBackground(FONT_BLACK);
  SetFontForeground(PERS_TEXT_FONT_COLOR);

  // display header

  // center
  FindFontCenterCoordinates(PERS_STAT_LOWEST_X, 0, PERS_STAT_LOWEST_WIDTH, 0,
                            pPersonnelCurrentTeamStatsStrings[0], FONT10ARIAL, &sX, &sY);

  mprintf(sX, PERS_STAT_AVG_Y, pPersonnelCurrentTeamStatsStrings[0]);

  // nobody on team leave
  if ((GetNumberOfMercsOnPlayersTeam() == 0) && (fCurrentTeamMode == TRUE)) {
    return;
  }

  if ((GetNumberOfPastMercsOnPlayersTeam() == 0) && (fCurrentTeamMode == FALSE)) {
    return;
  }

  for (iCounter = 0; iCounter < 11; iCounter++) {
    if (fCurrentTeamMode == TRUE) {
      iId = GetIdOfMercWithLowestStat(iCounter);
      //			if( iId == -1 )
      //				continue;
    } else {
      iDepartedId = GetIdOfDepartedMercWithLowestStat(iCounter);
      if (iDepartedId == -1) continue;
    }

    // even or odd?..color black or yellow?
    if (iCounter % 2 == 0) {
      SetFontForeground(PERS_TEXT_FONT_ALTERNATE_COLOR);
    } else {
      SetFontForeground(PERS_TEXT_FONT_COLOR);
    }

    if (fCurrentTeamMode == TRUE) {
      // get name
      if (iId == -1)
        swprintf(sString, ARR_SIZE(sString), L"%s", pPOWStrings[1]);
      else
        swprintf(sString, ARR_SIZE(sString), L"%s", MercPtrs[iId]->name);
    } else {
      // get name
      swprintf(sString, ARR_SIZE(sString), L"%s", gMercProfiles[iDepartedId].zNickname);
    }
    // print name
    mprintf(PERS_STAT_LOWEST_X, PERS_STAT_AVG_Y + (iCounter + 1) * (GetFontHeight(FONT10ARIAL) + 3),
            sString);

    switch (iCounter) {
      case 0:
        // health
        if (fCurrentTeamMode == TRUE) {
          if (iId == -1)
            iStat = -1;
          else
            iStat = MercPtrs[iId]->bLifeMax;
        } else {
          iStat = gMercProfiles[iDepartedId].bLife;
        }
        break;
      case 1:
        // agility
        if (fCurrentTeamMode == TRUE) {
          iStat = MercPtrs[iId]->bAgility;
        } else {
          iStat = gMercProfiles[iDepartedId].bAgility;
        }

        break;
      case 2:
        // dexterity
        if (fCurrentTeamMode == TRUE) {
          iStat = MercPtrs[iId]->bDexterity;
        } else {
          iStat = gMercProfiles[iDepartedId].bDexterity;
        }

        break;
      case 3:
        // strength
        if (fCurrentTeamMode == TRUE) {
          iStat = MercPtrs[iId]->bStrength;
        } else {
          iStat = gMercProfiles[iDepartedId].bStrength;
        }

        break;
      case 4:
        // leadership
        if (fCurrentTeamMode == TRUE) {
          iStat = MercPtrs[iId]->bLeadership;
        } else {
          iStat = gMercProfiles[iDepartedId].bLeadership;
        }
        break;
      case 5:
        // wisdom
        if (fCurrentTeamMode == TRUE) {
          iStat = MercPtrs[iId]->bWisdom;
        } else {
          iStat = gMercProfiles[iDepartedId].bWisdom;
        }
        break;
      case 6:
        // exper
        if (fCurrentTeamMode == TRUE) {
          iStat = MercPtrs[iId]->bExpLevel;
        } else {
          iStat = gMercProfiles[iDepartedId].bExpLevel;
        }
        break;
      case 7:
        // mrkmanship
        if (fCurrentTeamMode == TRUE) {
          iStat = MercPtrs[iId]->bMarksmanship;
        } else {
          iStat = gMercProfiles[iDepartedId].bMarksmanship;
        }
        break;
      case 8:
        // mech
        if (fCurrentTeamMode == TRUE) {
          iStat = MercPtrs[iId]->bMechanical;
        } else {
          iStat = gMercProfiles[iDepartedId].bMechanical;
        }
        break;
      case 9:
        // exp
        if (fCurrentTeamMode == TRUE) {
          iStat = MercPtrs[iId]->bExplosive;
        } else {
          iStat = gMercProfiles[iDepartedId].bExplosive;
        }
        break;
      case 10:
        // med
        if (fCurrentTeamMode == TRUE) {
          iStat = MercPtrs[iId]->bMedical;
        } else {
          iStat = gMercProfiles[iDepartedId].bMedical;
        }
        break;
    }

    if (iStat == -1)
      swprintf(sString, ARR_SIZE(sString), L"%s", pPOWStrings[1]);
    else
      swprintf(sString, ARR_SIZE(sString), L"%d", iStat);

    // right justify
    FindFontRightCoordinates(PERS_STAT_LOWEST_X, 0, PERS_STAT_LOWEST_WIDTH, 0, sString, FONT10ARIAL,
                             &sX, &sY);

    mprintf(sX, PERS_STAT_AVG_Y + (iCounter + 1) * (GetFontHeight(FONT10ARIAL) + 3), sString);
  }

  return;
}

void DisplayHighestStatValuesForCurrentTeam(void) {
  // will display the average values for stats for the current team
  int16_t sX, sY;
  int32_t iCounter = 0;
  CHAR16 sString[32];
  int32_t iStat = 0;
  int32_t iId = 0;

  // set up font
  SetFont(FONT10ARIAL);
  SetFontBackground(FONT_BLACK);
  SetFontForeground(PERS_TEXT_FONT_COLOR);

  // display header

  // center
  FindFontCenterCoordinates(PERS_STAT_HIGHEST_X, 0, PERS_STAT_LOWEST_WIDTH, 0,
                            pPersonnelCurrentTeamStatsStrings[2], FONT10ARIAL, &sX, &sY);

  mprintf(sX, PERS_STAT_AVG_Y, pPersonnelCurrentTeamStatsStrings[2]);

  // nobody on team leave
  if ((GetNumberOfMercsOnPlayersTeam() == 0) && (fCurrentTeamMode == TRUE)) {
    return;
  }

  if ((GetNumberOfPastMercsOnPlayersTeam() == 0) && (fCurrentTeamMode == FALSE)) {
    return;
  }

  for (iCounter = 0; iCounter < 11; iCounter++) {
    if (fCurrentTeamMode == TRUE)
      iId = GetIdOfMercWithHighestStat(iCounter);
    else
      iId = GetIdOfDepartedMercWithHighestStat(iCounter);

    //		if( iId == -1 )
    //			continue;

    // even or odd?..color black or yellow?
    if (iCounter % 2 == 0) {
      SetFontForeground(PERS_TEXT_FONT_ALTERNATE_COLOR);
    } else {
      SetFontForeground(PERS_TEXT_FONT_COLOR);
    }

    if (fCurrentTeamMode == TRUE) {
      // get name
      if (iId == -1)
        swprintf(sString, ARR_SIZE(sString), L"%s", pPOWStrings[1]);
      else
        swprintf(sString, ARR_SIZE(sString), L"%s", MercPtrs[iId]->name);
    } else {
      // get name
      swprintf(sString, ARR_SIZE(sString), L"%s", gMercProfiles[iId].zNickname);
    }
    // print name
    mprintf(PERS_STAT_HIGHEST_X,
            PERS_STAT_AVG_Y + (iCounter + 1) * (GetFontHeight(FONT10ARIAL) + 3), sString);

    switch (iCounter) {
      case 0:
        // health
        if (fCurrentTeamMode == TRUE) {
          if (iId == -1)
            iStat = -1;
          else
            iStat = MercPtrs[iId]->bLifeMax;
        } else {
          iStat = gMercProfiles[iId].bLife;
        }
        break;
      case 1:
        // agility
        if (fCurrentTeamMode == TRUE) {
          iStat = MercPtrs[iId]->bAgility;
        } else {
          iStat = gMercProfiles[iId].bAgility;
        }

        break;
      case 2:
        // dexterity
        if (fCurrentTeamMode == TRUE) {
          iStat = MercPtrs[iId]->bDexterity;
        } else {
          iStat = gMercProfiles[iId].bDexterity;
        }

        break;
      case 3:
        // strength
        if (fCurrentTeamMode == TRUE) {
          iStat = MercPtrs[iId]->bStrength;
        } else {
          iStat = gMercProfiles[iId].bStrength;
        }

        break;
      case 4:
        // leadership
        if (fCurrentTeamMode == TRUE) {
          iStat = MercPtrs[iId]->bLeadership;
        } else {
          iStat = gMercProfiles[iId].bLeadership;
        }
        break;
      case 5:
        // wisdom
        if (fCurrentTeamMode == TRUE) {
          iStat = MercPtrs[iId]->bWisdom;
        } else {
          iStat = gMercProfiles[iId].bWisdom;
        }
        break;
      case 6:
        // exper
        if (fCurrentTeamMode == TRUE) {
          iStat = MercPtrs[iId]->bExpLevel;
        } else {
          iStat = gMercProfiles[iId].bExpLevel;
        }
        break;
      case 7:
        // mrkmanship
        if (fCurrentTeamMode == TRUE) {
          iStat = MercPtrs[iId]->bMarksmanship;
        } else {
          iStat = gMercProfiles[iId].bMarksmanship;
        }
        break;
      case 8:
        // mech
        if (fCurrentTeamMode == TRUE) {
          iStat = MercPtrs[iId]->bMechanical;
        } else {
          iStat = gMercProfiles[iId].bMechanical;
        }
        break;
      case 9:
        // exp
        if (fCurrentTeamMode == TRUE) {
          iStat = MercPtrs[iId]->bExplosive;
        } else {
          iStat = gMercProfiles[iId].bExplosive;
        }
        break;
      case 10:
        // med
        if (fCurrentTeamMode == TRUE) {
          iStat = MercPtrs[iId]->bMedical;
        } else {
          iStat = gMercProfiles[iId].bMedical;
        }
        break;
    }

    if (iStat == -1)
      swprintf(sString, ARR_SIZE(sString), L"%s", pPOWStrings[1]);
    else
      swprintf(sString, ARR_SIZE(sString), L"%d", iStat);

    // right justify
    FindFontRightCoordinates(PERS_STAT_HIGHEST_X, 0, PERS_STAT_LOWEST_WIDTH, 0, sString,
                             FONT10ARIAL, &sX, &sY);

    mprintf(sX, PERS_STAT_AVG_Y + (iCounter + 1) * (GetFontHeight(FONT10ARIAL) + 3), sString);
  }

  return;
}

void DisplayPersonnelTeamStats(void) {
  // displays the stat title for each row in the team stat list
  int32_t iCounter = 0;

  // set up font
  SetFont(FONT10ARIAL);
  SetFontBackground(FONT_BLACK);
  SetFontForeground(FONT_WHITE);

  // display titles for each row
  for (iCounter = 0; iCounter < 11; iCounter++) {
    // even or odd?..color black or yellow?
    if (iCounter % 2 == 0) {
      SetFontForeground(PERS_TEXT_FONT_ALTERNATE_COLOR);
    } else {
      SetFontForeground(PERS_TEXT_FONT_COLOR);
    }

    mprintf(PERS_STAT_LIST_X, PERS_STAT_AVG_Y + (iCounter + 1) * (GetFontHeight(FONT10ARIAL) + 3),
            pPersonnelTeamStatsStrings[iCounter]);
  }

  return;
}

int32_t GetNumberOfPastMercsOnPlayersTeam(void) {
  int32_t iPastNumberOfMercs = 0;
  // will run through the alist of past mercs on the players team and return thier number

  // dead
  iPastNumberOfMercs += GetNumberOfDeadOnPastTeam();

  // left
  iPastNumberOfMercs += GetNumberOfLeftOnPastTeam();

  // other
  iPastNumberOfMercs += GetNumberOfOtherOnPastTeam();

  return iPastNumberOfMercs;
}

void InitPastCharactersList(void) {
  // inits the past characters list
  memset(&LaptopSaveInfo.ubDeadCharactersList, -1, sizeof(LaptopSaveInfo.ubDeadCharactersList));
  memset(&LaptopSaveInfo.ubLeftCharactersList, -1, sizeof(LaptopSaveInfo.ubLeftCharactersList));
  memset(&LaptopSaveInfo.ubOtherCharactersList, -1, sizeof(LaptopSaveInfo.ubOtherCharactersList));

  return;
}

int32_t GetNumberOfDeadOnPastTeam(void) {
  int32_t iNumberDead = 0;
  int32_t iCounter = 0;

  //	for( iCounter = 0; ( ( iCounter < 256) && ( LaptopSaveInfo.ubDeadCharactersList[ iCounter ]
  //!= -1 ) ) ; iCounter ++ )
  for (iCounter = 0; iCounter < 256; iCounter++) {
    if (LaptopSaveInfo.ubDeadCharactersList[iCounter] != -1) iNumberDead++;
  }

  return (iNumberDead);
}

int32_t GetNumberOfLeftOnPastTeam(void) {
  int32_t iNumberLeft = 0;
  int32_t iCounter = 0;

  for (iCounter = 0; iCounter < 256; iCounter++) {
    if (LaptopSaveInfo.ubLeftCharactersList[iCounter] != -1) iNumberLeft++;
  }

  return (iNumberLeft);
}

int32_t GetNumberOfOtherOnPastTeam(void) {
  int32_t iNumberOther = 0;
  int32_t iCounter = 0;

  for (iCounter = 0; iCounter < 256; iCounter++) {
    if (LaptopSaveInfo.ubOtherCharactersList[iCounter] != -1) iNumberOther++;
  }

  return (iNumberOther);
}

void DisplayStateOfPastTeamMembers(void) {
  int16_t sX, sY;
  CHAR16 sString[32];

  // font stuff
  SetFont(FONT10ARIAL);
  SetFontBackground(FONT_BLACK);
  SetFontForeground(PERS_TEXT_FONT_COLOR);

  // diplsya numbers fired, dead and othered
  if (fCurrentTeamMode == FALSE) {
    // dead
    mprintf(PERS_CURR_TEAM_COST_X, PERS_CURR_TEAM_COST_Y, pPersonelTeamStrings[5]);
    swprintf(sString, ARR_SIZE(sString), L"%d", GetNumberOfDeadOnPastTeam());

    FindFontRightCoordinates((int16_t)(PERS_CURR_TEAM_COST_X), 0, PERS_DEPART_TEAM_WIDTH, 0,
                             sString, PERS_FONT, &sX, &sY);

    mprintf(sX, PERS_CURR_TEAM_COST_Y, sString);

    // fired
    mprintf(PERS_CURR_TEAM_COST_X, PERS_CURR_TEAM_HIGHEST_Y, pPersonelTeamStrings[6]);
    swprintf(sString, ARR_SIZE(sString), L"%d", GetNumberOfLeftOnPastTeam());

    FindFontRightCoordinates((int16_t)(PERS_CURR_TEAM_COST_X), 0, PERS_DEPART_TEAM_WIDTH, 0,
                             sString, PERS_FONT, &sX, &sY);

    mprintf(sX, PERS_CURR_TEAM_HIGHEST_Y, sString);

    // other
    mprintf(PERS_CURR_TEAM_COST_X, PERS_CURR_TEAM_LOWEST_Y, pPersonelTeamStrings[7]);
    swprintf(sString, ARR_SIZE(sString), L"%d", GetNumberOfOtherOnPastTeam());

    FindFontRightCoordinates((int16_t)(PERS_CURR_TEAM_COST_X), 0, PERS_DEPART_TEAM_WIDTH, 0,
                             sString, PERS_FONT, &sX, &sY);

    mprintf(sX, PERS_CURR_TEAM_LOWEST_Y, sString);
  } else {
    // do nothing
  }
  return;
}

void CreateDestroyCurrentDepartedMouseRegions(void) {
  static BOOLEAN fCreated = FALSE;

  // will arbitrate the creation/deletion of mouse regions for current/past team toggles

  if ((fCreateRegionsForPastCurrentToggle == TRUE) && (fCreated == FALSE)) {
    // not created, create
    MSYS_DefineRegion(&gTogglePastCurrentTeam[0], PERS_TOGGLE_CUR_DEPART_X, PERS_TOGGLE_CUR_Y,
                      PERS_TOGGLE_CUR_DEPART_X + PERS_TOGGLE_CUR_DEPART_WIDTH,
                      PERS_TOGGLE_CUR_Y + PERS_TOGGLE_CUR_DEPART_HEIGHT, MSYS_PRIORITY_HIGHEST - 3,
                      CURSOR_LAPTOP_SCREEN, MSYS_NO_CALLBACK, PersonnelCurrentTeamCallback);

    MSYS_AddRegion(&gTogglePastCurrentTeam[0]);

    MSYS_DefineRegion(&gTogglePastCurrentTeam[1], PERS_TOGGLE_CUR_DEPART_X, PERS_TOGGLE_DEPART_Y,
                      PERS_TOGGLE_CUR_DEPART_X + PERS_TOGGLE_CUR_DEPART_WIDTH,
                      PERS_TOGGLE_DEPART_Y + PERS_TOGGLE_CUR_DEPART_HEIGHT,
                      MSYS_PRIORITY_HIGHEST - 3, CURSOR_LAPTOP_SCREEN, MSYS_NO_CALLBACK,
                      PersonnelDepartedTeamCallback);

    MSYS_AddRegion(&gTogglePastCurrentTeam[1]);

    fCreated = TRUE;

  } else if ((fCreateRegionsForPastCurrentToggle == FALSE) && (fCreated == TRUE)) {
    // created, get rid of

    MSYS_RemoveRegion(&gTogglePastCurrentTeam[0]);
    MSYS_RemoveRegion(&gTogglePastCurrentTeam[1]);
    fCreated = FALSE;
  }

  return;
}

void PersonnelCurrentTeamCallback(struct MOUSE_REGION *pRegion, int32_t iReason) {
  if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    fCurrentTeamMode = TRUE;

    if (fCurrentTeamMode == TRUE) {
      iCurrentPersonSelectedId = -1;

      // how many people do we have?..if you have someone set default to 0
      if (GetNumberOfMercsDeadOrAliveOnPlayersTeam() > 0) {
        // get id of first merc in list

        iCurrentPersonSelectedId = GetIdOfFirstDisplayedMerc();
      }
    }

    fCurrentTeamMode = TRUE;
    fReDrawScreenFlag = TRUE;
  }
}

void PersonnelDepartedTeamCallback(struct MOUSE_REGION *pRegion, int32_t iReason) {
  if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    fCurrentTeamMode = FALSE;

    if (fCurrentTeamMode == FALSE) {
      iCurrentPersonSelectedId = -1;

      // how many departed people?
      if (GetNumberOfPastMercsOnPlayersTeam() > 0) {
        iCurrentPersonSelectedId = 0;
      }

      // Switch the panel on the right to be the stat panel
      gubPersonnelInfoState = PERSONNEL_STAT_BTN;
    }

    fReDrawScreenFlag = TRUE;
  }
}

void CreateDestroyButtonsForDepartedTeamList(void) {
  // creates/ destroys the buttons for cdeparted team list
  static BOOLEAN fCreated = FALSE;

  if ((fCurrentTeamMode == FALSE) && (fCreated == FALSE)) {
    // not created. create
    giPersonnelButtonImage[4] = LoadButtonImage("LAPTOP\\departuresbuttons.sti", -1, 0, -1, 2, -1);
    giPersonnelButton[4] =
        QuickCreateButton(giPersonnelButtonImage[4], PERS_DEPARTED_UP_X, PERS_DEPARTED_UP_Y,
                          BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST - 1,
                          BtnGenericMouseMoveButtonCallback, (GUI_CALLBACK)DepartedUpCallBack);

    // right button
    giPersonnelButtonImage[5] = LoadButtonImage("LAPTOP\\departuresbuttons.sti", -1, 1, -1, 3, -1);
    giPersonnelButton[5] =
        QuickCreateButton(giPersonnelButtonImage[5], PERS_DEPARTED_UP_X, PERS_DEPARTED_DOWN_Y,
                          BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST - 1,
                          BtnGenericMouseMoveButtonCallback, (GUI_CALLBACK)DepartedDownCallBack);

    // set up cursors for these buttons
    SetButtonCursor(giPersonnelButton[4], CURSOR_LAPTOP_SCREEN);
    SetButtonCursor(giPersonnelButton[5], CURSOR_LAPTOP_SCREEN);

    fCreated = TRUE;
  } else if ((fCurrentTeamMode == TRUE) && (fCreated == TRUE)) {
    // created. destroy
    RemoveButton(giPersonnelButton[4]);
    UnloadButtonImage(giPersonnelButtonImage[4]);
    RemoveButton(giPersonnelButton[5]);
    UnloadButtonImage(giPersonnelButtonImage[5]);
    fCreated = FALSE;
    fReDrawScreenFlag = TRUE;
  }
}

void DepartedUpCallBack(GUI_BUTTON *btn, int32_t reason) {
  if (!(btn->uiFlags & BUTTON_ENABLED)) return;

  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    if (!(btn->uiFlags & BUTTON_CLICKED_ON)) {
    }
    btn->uiFlags |= (BUTTON_CLICKED_ON);
  } else if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    if (btn->uiFlags & BUTTON_CLICKED_ON) {
      btn->uiFlags &= ~(BUTTON_CLICKED_ON);

      if (giCurrentUpperLeftPortraitNumber - 20 >= 0) {
        giCurrentUpperLeftPortraitNumber -= 20;
        fReDrawScreenFlag = TRUE;
      }
    }
  }
}

void DepartedDownCallBack(GUI_BUTTON *btn, int32_t reason) {
  if (!(btn->uiFlags & BUTTON_ENABLED)) return;

  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    if (!(btn->uiFlags & BUTTON_CLICKED_ON)) {
    }
    btn->uiFlags |= (BUTTON_CLICKED_ON);
  } else if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    if (btn->uiFlags & BUTTON_CLICKED_ON) {
      btn->uiFlags &= ~(BUTTON_CLICKED_ON);
      if ((giCurrentUpperLeftPortraitNumber + 20) <
          (GetNumberOfDeadOnPastTeam() + GetNumberOfLeftOnPastTeam() +
           GetNumberOfOtherOnPastTeam())) {
        giCurrentUpperLeftPortraitNumber += 20;
        fReDrawScreenFlag = TRUE;
      }
    }
  }
}

void DisplayPastMercsPortraits(void) {
  // display past mercs portraits, starting at giCurrentUpperLeftPortraitNumber and going up 20
  // mercs start at dead mercs, then fired, then other

  int32_t iCounter = 0;
  int32_t iCounterA = 0;
  int32_t iStartArray = 0;  // 0 = dead list, 1 = fired list, 2 = other list

  // not time to display
  if (fCurrentTeamMode == TRUE) {
    return;
  }

  // go through dead list
  //	for( iCounterA = 0; ( ( LaptopSaveInfo.ubDeadCharactersList[ iCounterA ] != -1 ) && (
  // iCounter < giCurrentUpperLeftPortraitNumber ) ); iCounter++, iCounterA++ );
  for (iCounterA = 0; (iCounter < giCurrentUpperLeftPortraitNumber); iCounterA++) {
    if (LaptopSaveInfo.ubDeadCharactersList[iCounterA] != -1) iCounter++;
  }

  if (iCounter < giCurrentUpperLeftPortraitNumber) {
    // now the fired list
    //			for( iCounterA = 0; ( ( LaptopSaveInfo.ubLeftCharactersList[ iCounterA ] !=
    //-1
    //)
    //&& ( iCounter < giCurrentUpperLeftPortraitNumber ) ); iCounter++, iCounterA++ );
    for (iCounterA = 0; ((iCounter < giCurrentUpperLeftPortraitNumber)); iCounterA++) {
      if (LaptopSaveInfo.ubLeftCharactersList[iCounterA] != -1) {
        iCounter++;
      }
    }

    if (iCounter < 20) {
      iStartArray = 0;
    } else {
      iStartArray = 1;
    }
  } else {
    iStartArray = 0;
  }

  if ((iCounter < giCurrentUpperLeftPortraitNumber) && (iStartArray != 0)) {
    // now the fired list
    //			for( iCounterA = 0; ( ( LaptopSaveInfo.ubOtherCharactersList[ iCounterA ] !=
    //-1
    //)
    //&& ( iCounter < giCurrentUpperLeftPortraitNumber ) ); iCounter++, iCounterA++ );
    for (iCounterA = 0; (iCounter < giCurrentUpperLeftPortraitNumber); iCounterA++) {
      if (LaptopSaveInfo.ubOtherCharactersList[iCounterA] != -1) iCounter++;
    }

    if (iCounter < 20) {
      iStartArray = 1;
    } else {
      iStartArray = 2;
    }
  } else if (iStartArray != 0) {
    iStartArray = 1;
  }

  //; we now have the array to start in, the position

  iCounter = 0;

  if (iStartArray == 0) {
    // run through list and display
    //	  for( iCounterA ; ( ( iCounter < 20 ) && ( LaptopSaveInfo.ubDeadCharactersList[ iCounterA ]
    //!= -1 ) ); iCounter++, iCounterA++ )
    for (int iCounterA = 0; iCounter < 20 && iCounterA < 256; iCounterA++) {
      // show dead pictures
      if (LaptopSaveInfo.ubDeadCharactersList[iCounterA] != -1) {
        DisplayPortraitOfPastMerc(LaptopSaveInfo.ubDeadCharactersList[iCounterA], iCounter, TRUE,
                                  FALSE, FALSE);
        iCounter++;
      }
    }

    // reset counter A for the next array, if applicable
    iCounterA = 0;
  }
  if (iStartArray <= 1) {
    for (int iCounterA = 0; (iCounter < 20 && iCounterA < 256); iCounterA++) {
      // show fired pics
      if (LaptopSaveInfo.ubLeftCharactersList[iCounterA] != -1) {
        DisplayPortraitOfPastMerc(LaptopSaveInfo.ubLeftCharactersList[iCounterA], iCounter, FALSE,
                                  TRUE, FALSE);
        iCounter++;
      }
    }
    // reset counter A for the next array, if applicable
    iCounterA = 0;
  }

  if (iStartArray <= 2) {
    for (int iCounterA = 0; (iCounter < 20 && iCounterA < 256); iCounterA++) {
      // show other pics
      if (LaptopSaveInfo.ubOtherCharactersList[iCounterA] != -1) {
        DisplayPortraitOfPastMerc(LaptopSaveInfo.ubOtherCharactersList[iCounterA], iCounter, FALSE,
                                  FALSE, TRUE);
        iCounter++;
      }
    }
    // reset counter A for the next array, if applicable
    iCounterA = 0;
  }

  return;
}

int32_t GetIdOfPastMercInSlot(int32_t iSlot) {
  int32_t iCounter = -1;
  int32_t iCounterA = 0;
  // returns ID of Merc in this slot

  // not time to display
  if (fCurrentTeamMode == TRUE) {
    return -1;
  }

  if (iSlot >
      ((GetNumberOfDeadOnPastTeam() + GetNumberOfLeftOnPastTeam() + GetNumberOfOtherOnPastTeam()) -
       giCurrentUpperLeftPortraitNumber)) {
    // invalid slot
    return iCurrentPersonSelectedId;
  }
  // go through dead list
  for (iCounterA = 0; ((iCounter) < iSlot + giCurrentUpperLeftPortraitNumber); iCounterA++) {
    if (LaptopSaveInfo.ubDeadCharactersList[iCounterA] != -1) iCounter++;
  }

  if (iSlot + giCurrentUpperLeftPortraitNumber == iCounter) {
    return (LaptopSaveInfo.ubDeadCharactersList[iCounterA - 1]);
  }

  // now the fired list
  iCounterA = 0;
  for (iCounterA = 0; (((iCounter) < iSlot + giCurrentUpperLeftPortraitNumber)); iCounterA++) {
    if (LaptopSaveInfo.ubLeftCharactersList[iCounterA] != -1) iCounter++;
  }

  if (iSlot + giCurrentUpperLeftPortraitNumber == iCounter) {
    return (LaptopSaveInfo.ubLeftCharactersList[iCounterA - 1]);
  }

  // now the fired list
  iCounterA = 0;
  for (iCounterA = 0; (((iCounter) < (iSlot + giCurrentUpperLeftPortraitNumber))); iCounterA++) {
    if (LaptopSaveInfo.ubOtherCharactersList[iCounterA] != -1) iCounter++;
  }

  return (LaptopSaveInfo.ubOtherCharactersList[iCounterA - 1]);
}

BOOLEAN DisplayPortraitOfPastMerc(int32_t iId, int32_t iCounter, BOOLEAN fDead, BOOLEAN fFired,
                                  BOOLEAN fOther) {
  char sTemp[100];
  struct VObject *hFaceHandle;
  VOBJECT_DESC VObjectDesc;

  if ((50 < iId) && (57 > iId)) {
    sprintf(sTemp, "%s%03d.sti", SMALL_FACES_DIR, gMercProfiles[iId].ubFaceIndex);
  } else {
    if (iId < 100) {
      sprintf(sTemp, "%s%02d.sti", SMALL_FACES_DIR, iId);
    } else {
      sprintf(sTemp, "%s%03d.sti", SMALL_FACES_DIR, iId);
    }
  }

  VObjectDesc.fCreateFlags = VOBJECT_CREATE_FROMFILE;
  FilenameForBPP(sTemp, VObjectDesc.ImageFile);
  CHECKF(AddVideoObject(&VObjectDesc, &guiFACE));

  // Blt face to screen to
  GetVideoObject(&hFaceHandle, guiFACE);

  if (fDead) {
    hFaceHandle->pShades[0] =
        Create16BPPPaletteShaded(hFaceHandle->pPaletteEntry, DEAD_MERC_COLOR_RED,
                                 DEAD_MERC_COLOR_GREEN, DEAD_MERC_COLOR_BLUE, TRUE);

    // set the red pallete to the face
    SetObjectHandleShade(guiFACE, 0);
  }

  BltVideoObject(FRAME_BUFFER, hFaceHandle, 0,
                 (int16_t)(SMALL_PORTRAIT_START_X +
                           (iCounter % PERSONNEL_PORTRAIT_NUMBER_WIDTH) * SMALL_PORT_WIDTH),
                 (int16_t)(SMALL_PORTRAIT_START_Y +
                           (iCounter / PERSONNEL_PORTRAIT_NUMBER_WIDTH) * SMALL_PORT_HEIGHT),
                 VO_BLT_SRCTRANSPARENCY, NULL);

  /*
   text on the Small portrait
          if( fDead )
          {
                  //if the merc is dead, display it
  //		DrawTextToScreen(pDepartedMercPortraitStrings[0], ( int16_t ) (
  SMALL_PORTRAIT_START_X+ ( iCounter % PERSONNEL_PORTRAIT_NUMBER_WIDTH ) * SMALL_PORT_WIDTH ), (
  int16_t ) ( SMALL_PORTRAIT_START_Y + ( iCounter / PERSONNEL_PORTRAIT_NUMBER_WIDTH ) *
  SMALL_PORT_HEIGHT + SMALL_PORT_HEIGHT / 2 ), SMALL_PORTRAIT_WIDTH_NO_BORDERS, FONT10ARIAL, 145,
  FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED	); DrawTextToScreen( AimPopUpText[ AIM_MEMBER_DEAD
  ], ( int16_t ) ( SMALL_PORTRAIT_START_X+ ( iCounter % PERSONNEL_PORTRAIT_NUMBER_WIDTH ) *
  SMALL_PORT_WIDTH ), ( int16_t ) ( SMALL_PORTRAIT_START_Y + ( iCounter /
  PERSONNEL_PORTRAIT_NUMBER_WIDTH ) * SMALL_PORT_HEIGHT + SMALL_PORT_HEIGHT / 2 ),
  SMALL_PORTRAIT_WIDTH_NO_BORDERS, FONT10ARIAL, 145, FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED	);
          }
          else if( fFired )
          {
                  DrawTextToScreen(pDepartedMercPortraitStrings[1], ( int16_t ) (
  SMALL_PORTRAIT_START_X+ ( iCounter % PERSONNEL_PORTRAIT_NUMBER_WIDTH ) * SMALL_PORT_WIDTH ), (
  int16_t ) ( SMALL_PORTRAIT_START_Y + ( iCounter / PERSONNEL_PORTRAIT_NUMBER_WIDTH ) *
  SMALL_PORT_HEIGHT + SMALL_PORT_HEIGHT / 2 ), SMALL_PORTRAIT_WIDTH_NO_BORDERS, FONT10ARIAL, 145,
  FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED	);
          }
          else if( fOther )
          {
                  DrawTextToScreen(pDepartedMercPortraitStrings[2], ( int16_t ) (
  SMALL_PORTRAIT_START_X+ ( iCounter % PERSONNEL_PORTRAIT_NUMBER_WIDTH ) * SMALL_PORT_WIDTH ), (
  int16_t ) ( SMALL_PORTRAIT_START_Y + ( iCounter / PERSONNEL_PORTRAIT_NUMBER_WIDTH ) *
  SMALL_PORT_HEIGHT + SMALL_PORT_HEIGHT / 2 ), SMALL_PORTRAIT_WIDTH_NO_BORDERS, FONT10ARIAL, 145,
  FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED	);
          }
  */

  DeleteVideoObjectFromIndex(guiFACE);

  return (TRUE);
}

void DisplayDepartedCharStats(int32_t iId, int32_t iSlot, int32_t iState) {
  int32_t iCounter = 0;
  wchar_t sString[50];
  int16_t sX, sY;
  uint32_t uiHits = 0;

  // font stuff
  SetFont(FONT10ARIAL);
  SetFontBackground(FONT_BLACK);
  SetFontForeground(PERS_TEXT_FONT_COLOR);

  // display the stats for a char
  for (iCounter = 0; iCounter < MAX_STATS; iCounter++) {
    switch (iCounter) {
      case 0:
        // health

        // dead?
        if (iState == 0) {
          swprintf(sString, ARR_SIZE(sString), L"%d/%d", 0, gMercProfiles[iId].bLife);
        } else {
          swprintf(sString, ARR_SIZE(sString), L"%d/%d", gMercProfiles[iId].bLife,
                   gMercProfiles[iId].bLife);
        }

        mprintf((int16_t)(pPersonnelScreenPoints[iCounter].x + (iSlot * TEXT_BOX_WIDTH)),
                pPersonnelScreenPoints[iCounter].y, pPersonnelScreenStrings[iCounter]);
        FindFontRightCoordinates(
            (int16_t)(pPersonnelScreenPoints[iCounter].x + (iSlot * TEXT_BOX_WIDTH)), 0,
            TEXT_BOX_WIDTH - 20, 0, sString, PERS_FONT, &sX, &sY);
        mprintf(sX, pPersonnelScreenPoints[iCounter].y, sString);
        break;
      case 1:
        // agility
        swprintf(sString, ARR_SIZE(sString), L"%d", gMercProfiles[iId].bAgility);
        mprintf((int16_t)(pPersonnelScreenPoints[iCounter].x + (iSlot * TEXT_BOX_WIDTH)),
                pPersonnelScreenPoints[iCounter].y, pPersonnelScreenStrings[iCounter]);
        FindFontRightCoordinates(
            (int16_t)(pPersonnelScreenPoints[iCounter].x + (iSlot * TEXT_BOX_WIDTH)), 0,
            TEXT_BOX_WIDTH - 20, 0, sString, PERS_FONT, &sX, &sY);
        mprintf(sX, pPersonnelScreenPoints[iCounter].y, sString);
        break;
      case 2:
        // dexterity
        swprintf(sString, ARR_SIZE(sString), L"%d", gMercProfiles[iId].bDexterity);
        mprintf((int16_t)(pPersonnelScreenPoints[iCounter].x + (iSlot * TEXT_BOX_WIDTH)),
                pPersonnelScreenPoints[iCounter].y, pPersonnelScreenStrings[iCounter]);
        FindFontRightCoordinates(
            (int16_t)(pPersonnelScreenPoints[iCounter].x + (iSlot * TEXT_BOX_WIDTH)), 0,
            TEXT_BOX_WIDTH - 20, 0, sString, PERS_FONT, &sX, &sY);
        mprintf(sX, pPersonnelScreenPoints[iCounter].y, sString);
        break;
      case 3:
        // strength
        swprintf(sString, ARR_SIZE(sString), L"%d", gMercProfiles[iId].bStrength);
        mprintf((int16_t)(pPersonnelScreenPoints[iCounter].x + (iSlot * TEXT_BOX_WIDTH)),
                pPersonnelScreenPoints[iCounter].y, pPersonnelScreenStrings[iCounter]);
        FindFontRightCoordinates(
            (int16_t)(pPersonnelScreenPoints[iCounter].x + (iSlot * TEXT_BOX_WIDTH)), 0,
            TEXT_BOX_WIDTH - 20, 0, sString, PERS_FONT, &sX, &sY);
        mprintf(sX, pPersonnelScreenPoints[iCounter].y, sString);
        break;
      case 4:
        // leadership
        swprintf(sString, ARR_SIZE(sString), L"%d", gMercProfiles[iId].bLeadership);
        mprintf((int16_t)(pPersonnelScreenPoints[iCounter].x + (iSlot * TEXT_BOX_WIDTH)),
                pPersonnelScreenPoints[iCounter].y, pPersonnelScreenStrings[iCounter]);
        FindFontRightCoordinates(
            (int16_t)(pPersonnelScreenPoints[iCounter].x + (iSlot * TEXT_BOX_WIDTH)), 0,
            TEXT_BOX_WIDTH - 20, 0, sString, PERS_FONT, &sX, &sY);
        mprintf(sX, pPersonnelScreenPoints[iCounter].y, sString);
        break;
      case 5:
        // wisdom
        swprintf(sString, ARR_SIZE(sString), L"%d", gMercProfiles[iId].bWisdom);
        mprintf((int16_t)(pPersonnelScreenPoints[iCounter].x + (iSlot * TEXT_BOX_WIDTH)),
                pPersonnelScreenPoints[iCounter].y, pPersonnelScreenStrings[iCounter]);
        FindFontRightCoordinates(
            (int16_t)(pPersonnelScreenPoints[iCounter].x + (iSlot * TEXT_BOX_WIDTH)), 0,
            TEXT_BOX_WIDTH - 20, 0, sString, PERS_FONT, &sX, &sY);
        mprintf(sX, pPersonnelScreenPoints[iCounter].y, sString);
        break;
      case 6:
        // exper
        swprintf(sString, ARR_SIZE(sString), L"%d", gMercProfiles[iId].bExpLevel);
        mprintf((int16_t)(pPersonnelScreenPoints[iCounter].x + (iSlot * TEXT_BOX_WIDTH)),
                pPersonnelScreenPoints[iCounter].y, pPersonnelScreenStrings[iCounter]);
        FindFontRightCoordinates(
            (int16_t)(pPersonnelScreenPoints[iCounter].x + (iSlot * TEXT_BOX_WIDTH)), 0,
            TEXT_BOX_WIDTH - 20, 0, sString, PERS_FONT, &sX, &sY);
        mprintf(sX, pPersonnelScreenPoints[iCounter].y, sString);
        break;
      case 7:
        // mrkmanship
        swprintf(sString, ARR_SIZE(sString), L"%d", gMercProfiles[iId].bMarksmanship);
        mprintf((int16_t)(pPersonnelScreenPoints[iCounter].x + (iSlot * TEXT_BOX_WIDTH)),
                pPersonnelScreenPoints[iCounter].y, pPersonnelScreenStrings[iCounter]);
        FindFontRightCoordinates(
            (int16_t)(pPersonnelScreenPoints[iCounter].x + (iSlot * TEXT_BOX_WIDTH)), 0,
            TEXT_BOX_WIDTH - 20, 0, sString, PERS_FONT, &sX, &sY);
        mprintf(sX, pPersonnelScreenPoints[iCounter].y, sString);
        break;
      case 8:
        // mech
        swprintf(sString, ARR_SIZE(sString), L"%d", gMercProfiles[iId].bMechanical);
        mprintf((int16_t)(pPersonnelScreenPoints[iCounter].x + (iSlot * TEXT_BOX_WIDTH)),
                pPersonnelScreenPoints[iCounter].y, pPersonnelScreenStrings[iCounter]);
        FindFontRightCoordinates(
            (int16_t)(pPersonnelScreenPoints[iCounter].x + (iSlot * TEXT_BOX_WIDTH)), 0,
            TEXT_BOX_WIDTH - 20, 0, sString, PERS_FONT, &sX, &sY);
        mprintf(sX, pPersonnelScreenPoints[iCounter].y, sString);
        break;
      case 9:
        // exp
        swprintf(sString, ARR_SIZE(sString), L"%d", gMercProfiles[iId].bExplosive);
        mprintf((int16_t)(pPersonnelScreenPoints[iCounter].x + (iSlot * TEXT_BOX_WIDTH)),
                pPersonnelScreenPoints[iCounter].y, pPersonnelScreenStrings[iCounter]);
        FindFontRightCoordinates(
            (int16_t)(pPersonnelScreenPoints[iCounter].x + (iSlot * TEXT_BOX_WIDTH)), 0,
            TEXT_BOX_WIDTH - 20, 0, sString, PERS_FONT, &sX, &sY);
        mprintf(sX, pPersonnelScreenPoints[iCounter].y, sString);
        break;
      case 10:
        // med
        mprintf((int16_t)(pPersonnelScreenPoints[iCounter].x + (iSlot * TEXT_BOX_WIDTH)),
                pPersonnelScreenPoints[iCounter].y, pPersonnelScreenStrings[iCounter]);

        swprintf(sString, ARR_SIZE(sString), L"%d", gMercProfiles[iId].bMedical);

        FindFontRightCoordinates(
            (int16_t)(pPersonnelScreenPoints[iCounter].x + (iSlot * TEXT_BOX_WIDTH)), 0,
            TEXT_BOX_WIDTH - 20, 0, sString, PERS_FONT, &sX, &sY);
        mprintf(sX, pPersonnelScreenPoints[iCounter].y, sString);
        break;

      case 14:
        // kills
        mprintf((int16_t)(pPersonnelScreenPoints[21].x + (iSlot * TEXT_BOX_WIDTH)),
                pPersonnelScreenPoints[21].y, pPersonnelScreenStrings[PRSNL_TXT_KILLS]);
        swprintf(sString, ARR_SIZE(sString), L"%d", gMercProfiles[iId].usKills);
        FindFontRightCoordinates((int16_t)(pPersonnelScreenPoints[21].x + (iSlot * TEXT_BOX_WIDTH)),
                                 0, TEXT_BOX_WIDTH - 20, 0, sString, PERS_FONT, &sX, &sY);
        mprintf(sX, pPersonnelScreenPoints[21].y, sString);
        break;
      case 15:
        // assists
        mprintf((int16_t)(pPersonnelScreenPoints[22].x + (iSlot * TEXT_BOX_WIDTH)),
                pPersonnelScreenPoints[22].y, pPersonnelScreenStrings[PRSNL_TXT_ASSISTS]);
        swprintf(sString, ARR_SIZE(sString), L"%d", gMercProfiles[iId].usAssists);
        FindFontRightCoordinates((int16_t)(pPersonnelScreenPoints[22].x + (iSlot * TEXT_BOX_WIDTH)),
                                 0, TEXT_BOX_WIDTH - 20, 0, sString, PERS_FONT, &sX, &sY);
        mprintf(sX, pPersonnelScreenPoints[22].y, sString);
        break;
      case 16:
        // shots/hits
        mprintf((int16_t)(pPersonnelScreenPoints[23].x + (iSlot * TEXT_BOX_WIDTH)),
                pPersonnelScreenPoints[23].y, pPersonnelScreenStrings[PRSNL_TXT_HIT_PERCENTAGE]);
        uiHits = (uint32_t)gMercProfiles[iId].usShotsHit;
        uiHits *= 100;

        // check we have shot at least once
        if (gMercProfiles[iId].usShotsFired > 0) {
          uiHits /= (uint32_t)gMercProfiles[iId].usShotsFired;
        } else {
          // no, set hit % to 0
          uiHits = 0;
        }

        swprintf(sString, ARR_SIZE(sString), L"%d %%%%", uiHits);
        FindFontRightCoordinates((int16_t)(pPersonnelScreenPoints[23].x + (iSlot * TEXT_BOX_WIDTH)),
                                 0, TEXT_BOX_WIDTH - 20, 0, sString, PERS_FONT, &sX, &sY);
        sX += StringPixLength(L"%", PERS_FONT);
        mprintf(sX, pPersonnelScreenPoints[23].y, sString);
        break;
      case 17:
        // battles
        mprintf((int16_t)(pPersonnelScreenPoints[24].x + (iSlot * TEXT_BOX_WIDTH)),
                pPersonnelScreenPoints[24].y, pPersonnelScreenStrings[PRSNL_TXT_BATTLES]);
        swprintf(sString, ARR_SIZE(sString), L"%d", gMercProfiles[iId].usBattlesFought);
        FindFontRightCoordinates((int16_t)(pPersonnelScreenPoints[24].x + (iSlot * TEXT_BOX_WIDTH)),
                                 0, TEXT_BOX_WIDTH - 20, 0, sString, PERS_FONT, &sX, &sY);
        mprintf(sX, pPersonnelScreenPoints[24].y, sString);
        break;
      case 18:
        // wounds
        mprintf((int16_t)(pPersonnelScreenPoints[25].x + (iSlot * TEXT_BOX_WIDTH)),
                pPersonnelScreenPoints[25].y, pPersonnelScreenStrings[PRSNL_TXT_TIMES_WOUNDED]);
        swprintf(sString, ARR_SIZE(sString), L"%d", gMercProfiles[iId].usTimesWounded);
        FindFontRightCoordinates((int16_t)(pPersonnelScreenPoints[25].x + (iSlot * TEXT_BOX_WIDTH)),
                                 0, TEXT_BOX_WIDTH - 20, 0, sString, PERS_FONT, &sX, &sY);
        mprintf(sX, pPersonnelScreenPoints[25].y, sString);
        break;
    }
  }

  return;
}

void EnableDisableDeparturesButtons(void) {
  // will enable or disable departures buttons based on upperleft picutre index value
  if ((fCurrentTeamMode == TRUE) || (fNewMailFlag == TRUE)) {
    return;
  }

  // disable both buttons
  DisableButton(giPersonnelButton[4]);
  DisableButton(giPersonnelButton[5]);

  if (giCurrentUpperLeftPortraitNumber != 0) {
    // enable up button
    EnableButton(giPersonnelButton[4]);
  }
  if ((GetNumberOfDeadOnPastTeam() + GetNumberOfLeftOnPastTeam() + GetNumberOfOtherOnPastTeam()) -
          giCurrentUpperLeftPortraitNumber >=
      20) {
    // enable down button
    EnableButton(giPersonnelButton[5]);
  }

  return;
}

void DisplayDepartedCharName(int32_t iId, int32_t iSlot, int32_t iState) {
  // get merc's nickName, assignment, and sector location info
  int16_t sX, sY;
  CHAR16 sString[32];

  SetFont(CHAR_NAME_FONT);
  SetFontForeground(PERS_TEXT_FONT_COLOR);
  SetFontBackground(FONT_BLACK);

  if ((iState == -1) || (iId == -1)) {
    return;
  }

  swprintf(sString, ARR_SIZE(sString), L"%s", gMercProfiles[iId].zNickname);

  // nick name - assignment
  FindFontCenterCoordinates(IMAGE_BOX_X - 5, 0, IMAGE_BOX_WIDTH + 90, 0, sString, CHAR_NAME_FONT,
                            &sX, &sY);

  // cehck to se eif we are going to go off the left edge
  if (sX < pPersonnelScreenPoints[0].x) {
    sX = (int16_t)pPersonnelScreenPoints[0].x;
  }

  mprintf(sX + iSlot * IMAGE_BOX_WIDTH, CHAR_NAME_Y, sString);

  // state
  if (gMercProfiles[iId].ubMiscFlags2 & PROFILE_MISC_FLAG2_MARRIED_TO_HICKS) {
    // displaye 'married'
    swprintf(sString, ARR_SIZE(sString), L"%s", pPersonnelDepartedStateStrings[DEPARTED_MARRIED]);
  } else if (iState == DEPARTED_DEAD) {
    swprintf(sString, ARR_SIZE(sString), L"%s", pPersonnelDepartedStateStrings[DEPARTED_DEAD]);
  }

  // if the merc is an AIM merc
  else if (iId < BIFF) {
    // if dismissed
    if (iState == DEPARTED_FIRED)
      swprintf(sString, ARR_SIZE(sString), L"%s", pPersonnelDepartedStateStrings[DEPARTED_FIRED]);
    else
      swprintf(sString, ARR_SIZE(sString), L"%s",
               pPersonnelDepartedStateStrings[DEPARTED_CONTRACT_EXPIRED]);
  }

  // else if its a MERC merc
  else if (iId >= BIFF && iId <= BUBBA) {
    if (iState == DEPARTED_FIRED)
      swprintf(sString, ARR_SIZE(sString), L"%s", pPersonnelDepartedStateStrings[DEPARTED_FIRED]);
    else
      swprintf(sString, ARR_SIZE(sString), L"%s", pPersonnelDepartedStateStrings[DEPARTED_QUIT]);
  }
  // must be a RPC
  else {
    if (iState == DEPARTED_FIRED)
      swprintf(sString, ARR_SIZE(sString), L"%s", pPersonnelDepartedStateStrings[DEPARTED_FIRED]);
    else
      swprintf(sString, ARR_SIZE(sString), L"%s", pPersonnelDepartedStateStrings[DEPARTED_QUIT]);
  }

  // nick name - assignment
  FindFontCenterCoordinates(IMAGE_BOX_X - 5, 0, IMAGE_BOX_WIDTH + 90, 0, sString, CHAR_NAME_FONT,
                            &sX, &sY);

  // cehck to se eif we are going to go off the left edge
  if (sX < pPersonnelScreenPoints[0].x) {
    sX = (int16_t)pPersonnelScreenPoints[0].x;
  }

  mprintf(sX + iSlot * IMAGE_BOX_WIDTH, CHAR_NAME_Y + 10, sString);

  return;
}

int32_t GetTheStateOfDepartedMerc(int32_t iId) {
  int32_t iCounter = 0;
  // will runt hrough each list until merc is found, if not a -1 is returned

  for (iCounter = 0; iCounter < 256; iCounter++) {
    if (LaptopSaveInfo.ubDeadCharactersList[iCounter] == iId) {
      return (DEPARTED_DEAD);
    }
  }

  for (iCounter = 0; iCounter < 256; iCounter++) {
    if (LaptopSaveInfo.ubLeftCharactersList[iCounter] == iId) {
      return (DEPARTED_FIRED);
    }
  }

  for (iCounter = 0; iCounter < 256; iCounter++) {
    if (LaptopSaveInfo.ubOtherCharactersList[iCounter] == iId) {
      return (DEPARTED_OTHER);
    }
  }

  return (-1);
}

void DisplayPersonnelTextOnTitleBar(void) {
  // draw email screen title text

  // font stuff
  SetFont(FONT14ARIAL);
  SetFontForeground(FONT_WHITE);
  SetFontBackground(FONT_BLACK);

  // printf the title
  mprintf(PERS_TITLE_X, PERS_TITLE_Y, pPersTitleText[0]);

  // reset the shadow
}

BOOLEAN DisplayHighLightBox(void) {
  // will display highlight box around selected merc
  VOBJECT_DESC VObjectDesc;
  uint32_t uiBox = 0;
  struct VObject *hHandle;

  // load graphics

  // is the current selected face valid?
  if (iCurrentPersonSelectedId == -1) {
    // no, leave
    return (FALSE);
  }

  // bounding
  VObjectDesc.fCreateFlags = VOBJECT_CREATE_FROMFILE;
  FilenameForBPP("LAPTOP\\PicBorde.sti", VObjectDesc.ImageFile);
  CHECKF(AddVideoObject(&VObjectDesc, &uiBox));

  // blit it
  GetVideoObject(&hHandle, uiBox);
  BltVideoObject(
      FRAME_BUFFER, hHandle, 0,
      (int16_t)(SMALL_PORTRAIT_START_X +
                (iCurrentPersonSelectedId % PERSONNEL_PORTRAIT_NUMBER_WIDTH) * SMALL_PORT_WIDTH -
                2),
      (int16_t)(SMALL_PORTRAIT_START_Y +
                (iCurrentPersonSelectedId / PERSONNEL_PORTRAIT_NUMBER_WIDTH) * SMALL_PORT_HEIGHT -
                3),
      VO_BLT_SRCTRANSPARENCY, NULL);

  // deleteit
  DeleteVideoObjectFromIndex(uiBox);

  return (TRUE);
}

// add to dead list
void AddCharacterToDeadList(struct SOLDIERTYPE *pSoldier) {
  int32_t iCounter = 0;

  for (iCounter = 0; iCounter < 256; iCounter++) {
    if (LaptopSaveInfo.ubDeadCharactersList[iCounter] == -1) {
      // valid slot, merc not found yet, inset here
      LaptopSaveInfo.ubDeadCharactersList[iCounter] = GetSolProfile(pSoldier);

      // leave
      return;
    }

    // are they already in the list?
    if (LaptopSaveInfo.ubDeadCharactersList[iCounter] == GetSolProfile(pSoldier)) {
      return;
    }
  }
}

void AddCharacterToFiredList(struct SOLDIERTYPE *pSoldier) {
  int32_t iCounter = 0;

  for (iCounter = 0; iCounter < 256; iCounter++) {
    if (LaptopSaveInfo.ubLeftCharactersList[iCounter] == -1) {
      // valid slot, merc not found yet, inset here
      LaptopSaveInfo.ubLeftCharactersList[iCounter] = GetSolProfile(pSoldier);

      // leave
      return;
    }

    // are they already in the list?
    if (LaptopSaveInfo.ubLeftCharactersList[iCounter] == GetSolProfile(pSoldier)) {
      return;
    }
  }
}

void AddCharacterToOtherList(struct SOLDIERTYPE *pSoldier) {
  int32_t iCounter = 0;

  for (iCounter = 0; iCounter < 256; iCounter++) {
    if (LaptopSaveInfo.ubOtherCharactersList[iCounter] == -1) {
      // valid slot, merc not found yet, inset here
      LaptopSaveInfo.ubOtherCharactersList[iCounter] = GetSolProfile(pSoldier);

      // leave
      return;
    }

    // are they already in the list?
    if (LaptopSaveInfo.ubOtherCharactersList[iCounter] == GetSolProfile(pSoldier)) {
      return;
    }
  }
}

// If you have hired a merc before, then the they left for whatever reason, and now you are hiring
// them again, we must get rid of them from the departed section in the personnel screen.  ( wouldnt
// make sense for them
// to be on your team list, and departed list )
BOOLEAN RemoveNewlyHiredMercFromPersonnelDepartedList(uint8_t ubProfile) {
  int32_t iCounter = 0;

  for (iCounter = 0; iCounter < 256; iCounter++) {
    // are they already in the Dead list?
    if (LaptopSaveInfo.ubDeadCharactersList[iCounter] == ubProfile) {
      // Reset the fact that they were once hired
      LaptopSaveInfo.ubDeadCharactersList[iCounter] = -1;
      return (TRUE);
    }

    // are they already in the other list?
    if (LaptopSaveInfo.ubLeftCharactersList[iCounter] == ubProfile) {
      // Reset the fact that they were once hired
      LaptopSaveInfo.ubLeftCharactersList[iCounter] = -1;
      return (TRUE);
    }

    // are they already in the list?
    if (LaptopSaveInfo.ubOtherCharactersList[iCounter] == ubProfile) {
      // Reset the fact that they were once hired
      LaptopSaveInfo.ubOtherCharactersList[iCounter] = -1;
      return (TRUE);
    }
  }

  return (FALSE);
}

// grab the id of the first merc being displayed
int32_t GetIdOfFirstDisplayedMerc() {
  struct SOLDIERTYPE *pSoldier;
  int32_t cnt = 0;

  // set current soldier
  pSoldier = MercPtrs[cnt];

  if (fCurrentTeamMode == TRUE) {
    // run through list of soldiers on players current team
    // cnt = gTacticalStatus.Team[ pSoldier->bTeam ].bFirstID;
    for (pSoldier = MercPtrs[cnt]; cnt <= gTacticalStatus.Team[pSoldier->bTeam].bLastID;
         cnt++, pSoldier++) {
      if ((IsSolActive(pSoldier)) && IsSolAlive(pSoldier)) {
        return (0);
      }
    }
    return (-1);
  } else {
    // run through list of soldier on players old team...the slot id will be translated
    return 0;
  }
}

int32_t GetIdOfThisSlot(int32_t iSlot) {
  struct SOLDIERTYPE *pSoldier;
  int32_t cnt = 0;
  int32_t iCounter = 0;

  // set current soldier
  pSoldier = MercPtrs[cnt];

  if (fCurrentTeamMode == TRUE) {
    // run through list of soldiers on players current team
    cnt = gTacticalStatus.Team[pSoldier->bTeam].bFirstID;
    for (pSoldier = MercPtrs[cnt]; cnt <= gTacticalStatus.Team[pSoldier->bTeam].bLastID;
         cnt++, pSoldier++) {
      if ((IsSolActive(pSoldier))) {
        // same character as slot, return this value
        if (iCounter == iSlot) {
          return (cnt);
        }

        // found another soldier
        iCounter++;
      }
    }
  } else {
    // run through list of soldier on players old team...the slot id will be translated
    return iSlot;
  }

  return (0);
}

BOOLEAN RenderAtmPanel(void) {
  VOBJECT_DESC VObjectDesc;
  uint32_t uiBox = 0;
  struct VObject *hHandle;

  // render the ATM panel
  if (fShowAtmPanel) {
    VObjectDesc.fCreateFlags = VOBJECT_CREATE_FROMFILE;
    FilenameForBPP("LAPTOP\\AtmButtons.sti", VObjectDesc.ImageFile);
    CHECKF(AddVideoObject(&VObjectDesc, &uiBox));

    // blit it
    GetVideoObject(&hHandle, uiBox);
    BltVideoObject(FRAME_BUFFER, hHandle, 0, (int16_t)(ATM_UL_X), (int16_t)(ATM_UL_Y),
                   VO_BLT_SRCTRANSPARENCY, NULL);

    DeleteVideoObjectFromIndex(uiBox);

    // show amount
    DisplayATMAmount();
    RenderRectangleForPersonnelTransactionAmount();

    // create destroy
    CreateDestroyStartATMButton();

    // display strings for ATM
    DisplayATMStrings();

    // handle states
    HandleStateOfATMButtons();

    // DisplayAmountOnCurrentMerc( );

  } else {
    // just show basic panel
    // bounding
    VObjectDesc.fCreateFlags = VOBJECT_CREATE_FROMFILE;
    FilenameForBPP("LAPTOP\\AtmButtons.sti", VObjectDesc.ImageFile);
    CHECKF(AddVideoObject(&VObjectDesc, &uiBox));

    GetVideoObject(&hHandle, uiBox);
    BltVideoObject(FRAME_BUFFER, hHandle, 0, (int16_t)(ATM_UL_X), (int16_t)(ATM_UL_Y),
                   VO_BLT_SRCTRANSPARENCY, NULL);

    // blit it
    GetVideoObject(&hHandle, uiBox);
    BltVideoObject(FRAME_BUFFER, hHandle, 1, (int16_t)(ATM_UL_X + 1), (int16_t)(ATM_UL_Y + 18),
                   VO_BLT_SRCTRANSPARENCY, NULL);

    DeleteVideoObjectFromIndex(uiBox);

    // display strings for ATM
    DisplayATMStrings();

    // DisplayAmountOnCurrentMerc( );

    // create destroy
    CreateDestroyStartATMButton();
  }
  return (TRUE);
}

void CreateDestroyStartATMButton(void) {
  static BOOLEAN fCreated = FALSE;
  // create/destroy atm start button as needed

  if ((fCreated == FALSE) && (fShowAtmPanelStartButton == TRUE)) {
    // not created, must create

    /*
    // the ATM start button
    giPersonnelATMStartButtonImage[ 0 ]=  LoadButtonImage( "LAPTOP\\AtmButtons.sti" ,-1,2,-1,3,-1 );
    giPersonnelATMStartButton[ 0 ] = QuickCreateButton( giPersonnelATMStartButtonImage[ 0 ] ,
    519,87, BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST - 1, BtnGenericMouseMoveButtonCallback,
    (GUI_CALLBACK)ATMStartButtonCallback );

    // set text and what not
    SpecifyButtonText( giPersonnelATMStartButton[ 0 ] ,gsAtmStartButtonText[ 0 ] );
    SpecifyButtonUpTextColors( giPersonnelATMStartButton[ 0 ], FONT_BLACK, FONT_BLACK );
    SpecifyButtonFont( giPersonnelATMStartButton[ 0 ], PERS_FONT );
    SetButtonCursor(giPersonnelATMStartButton[ 0 ], CURSOR_LAPTOP_SCREEN);
*/
    // the stats button
    giPersonnelATMStartButtonImage[PERSONNEL_STAT_BTN] =
        LoadButtonImage("LAPTOP\\AtmButtons.sti", -1, 2, -1, 3, -1);
    giPersonnelATMStartButton[PERSONNEL_STAT_BTN] =
        QuickCreateButton(giPersonnelATMStartButtonImage[PERSONNEL_STAT_BTN], 519, 80,
                          BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST - 1, MSYS_NO_CALLBACK,
                          (GUI_CALLBACK)PersonnelStatStartButtonCallback);

    // set text and what not
    SpecifyButtonText(giPersonnelATMStartButton[PERSONNEL_STAT_BTN], gsAtmStartButtonText[1]);
    SpecifyButtonUpTextColors(giPersonnelATMStartButton[PERSONNEL_STAT_BTN], FONT_BLACK,
                              FONT_BLACK);
    SpecifyButtonFont(giPersonnelATMStartButton[PERSONNEL_STAT_BTN], PERS_FONT);
    SetButtonCursor(giPersonnelATMStartButton[PERSONNEL_STAT_BTN], CURSOR_LAPTOP_SCREEN);

    // the Employment selection button
    giPersonnelATMStartButtonImage[PERSONNEL_EMPLOYMENT_BTN] =
        LoadButtonImage("LAPTOP\\AtmButtons.sti", -1, 2, -1, 3, -1);
    giPersonnelATMStartButton[PERSONNEL_EMPLOYMENT_BTN] = QuickCreateButton(
        giPersonnelATMStartButtonImage[PERSONNEL_EMPLOYMENT_BTN], 519, 110, BUTTON_TOGGLE,
        MSYS_PRIORITY_HIGHEST - 1, MSYS_NO_CALLBACK, (GUI_CALLBACK)EmployementInfoButtonCallback);

    // set text and what not
    SpecifyButtonText(giPersonnelATMStartButton[PERSONNEL_EMPLOYMENT_BTN], gsAtmStartButtonText[3]);
    SpecifyButtonUpTextColors(giPersonnelATMStartButton[PERSONNEL_EMPLOYMENT_BTN], FONT_BLACK,
                              FONT_BLACK);
    SpecifyButtonFont(giPersonnelATMStartButton[PERSONNEL_EMPLOYMENT_BTN], PERS_FONT);
    SetButtonCursor(giPersonnelATMStartButton[PERSONNEL_EMPLOYMENT_BTN], CURSOR_LAPTOP_SCREEN);

    // the inventory selection button
    giPersonnelATMStartButtonImage[PERSONNEL_INV_BTN] =
        LoadButtonImage("LAPTOP\\AtmButtons.sti", -1, 2, -1, 3, -1);
    giPersonnelATMStartButton[PERSONNEL_INV_BTN] = QuickCreateButton(
        giPersonnelATMStartButtonImage[PERSONNEL_INV_BTN], 519, 140, BUTTON_TOGGLE,
        MSYS_PRIORITY_HIGHEST - 1, MSYS_NO_CALLBACK, (GUI_CALLBACK)PersonnelINVStartButtonCallback);

    // set text and what not
    SpecifyButtonText(giPersonnelATMStartButton[PERSONNEL_INV_BTN], gsAtmStartButtonText[2]);
    SpecifyButtonUpTextColors(giPersonnelATMStartButton[PERSONNEL_INV_BTN], FONT_BLACK, FONT_BLACK);
    SpecifyButtonFont(giPersonnelATMStartButton[PERSONNEL_INV_BTN], PERS_FONT);
    SetButtonCursor(giPersonnelATMStartButton[PERSONNEL_INV_BTN], CURSOR_LAPTOP_SCREEN);

    fCreated = TRUE;
  } else if ((fCreated == TRUE) && (fShowAtmPanelStartButton == FALSE)) {
    // stop showing
    /*
    RemoveButton( giPersonnelATMStartButton[ 0 ] );
    UnloadButtonImage( giPersonnelATMStartButtonImage[ 0 ] );
    */
    RemoveButton(giPersonnelATMStartButton[PERSONNEL_STAT_BTN]);
    UnloadButtonImage(giPersonnelATMStartButtonImage[PERSONNEL_STAT_BTN]);
    RemoveButton(giPersonnelATMStartButton[PERSONNEL_EMPLOYMENT_BTN]);
    UnloadButtonImage(giPersonnelATMStartButtonImage[PERSONNEL_EMPLOYMENT_BTN]);
    RemoveButton(giPersonnelATMStartButton[PERSONNEL_INV_BTN]);
    UnloadButtonImage(giPersonnelATMStartButtonImage[PERSONNEL_INV_BTN]);

    fCreated = FALSE;
  }
}

void FindPositionOfPersInvSlider(void) {
  int32_t iValue = 0;
  int32_t iNumberOfItems = 0;
  int16_t sSizeOfEachSubRegion = 0;

  // find out how many there are
  iValue = (int32_t)(GetNumberOfInventoryItemsOnCurrentMerc());

  // otherwise there are more than one item
  iNumberOfItems = iValue - NUMBER_OF_INVENTORY_PERSONNEL;

  if (iValue <= 0) {
    iValue = 1;
  }

  // get the subregion sizes
  sSizeOfEachSubRegion =
      (int16_t)((int32_t)(Y_SIZE_OF_PERSONNEL_SCROLL_REGION - SIZE_OF_PERSONNEL_CURSOR) /
                (int32_t)(iNumberOfItems));

  // get slider position
  guiSliderPosition = uiCurrentInventoryIndex * sSizeOfEachSubRegion;
}

void HandleSliderBarClickCallback(struct MOUSE_REGION *pRegion, int32_t iReason) {
  int32_t iValue = 0;
  int32_t iNumberOfItems = 0;
  int16_t sSizeOfEachSubRegion = 0;
  int16_t sYPositionOnBar = 0;
  int16_t iCurrentItemValue = 0;

  if ((iReason & MSYS_CALLBACK_REASON_LBUTTON_DWN) ||
      (iReason & MSYS_CALLBACK_REASON_LBUTTON_REPEAT)) {
    // find out how many there are
    iValue = (int32_t)(GetNumberOfInventoryItemsOnCurrentMerc());

    // make sure there are more than one page
    if ((int32_t)uiCurrentInventoryIndex >= iValue - NUMBER_OF_INVENTORY_PERSONNEL + 1) {
      return;
    }

    // otherwise there are more than one item
    iNumberOfItems = iValue - NUMBER_OF_INVENTORY_PERSONNEL;

    // number of items is 0
    if (iNumberOfItems == 0) {
      return;
    }

    // find the x,y on the slider bar
    struct Point MousePos = GetMousePoint();

    // get the subregion sizes
    sSizeOfEachSubRegion =
        (int16_t)((int32_t)(Y_SIZE_OF_PERSONNEL_SCROLL_REGION - SIZE_OF_PERSONNEL_CURSOR) /
                  (int32_t)(iNumberOfItems));

    // get the cursor placement
    sYPositionOnBar = (int16_t)(MousePos.y - Y_OF_PERSONNEL_SCROLL_REGION);

    if (sSizeOfEachSubRegion == 0) {
      return;
    }

    // get the actual item position
    iCurrentItemValue = sYPositionOnBar / sSizeOfEachSubRegion;

    if (uiCurrentInventoryIndex != iCurrentItemValue) {
      // get slider position
      guiSliderPosition = iCurrentItemValue * sSizeOfEachSubRegion;

      // set current inventory value
      uiCurrentInventoryIndex = (uint8_t)iCurrentItemValue;

      // force update
      fReDrawScreenFlag = TRUE;
    }
  }
}

void RenderSliderBarForPersonnelInventory(void) {
  struct VObject *hHandle;

  // render slider bar for personnel
  GetVideoObject(&hHandle, guiPersonnelInventory);
  BltVideoObject(FRAME_BUFFER, hHandle, 5, (int16_t)(X_OF_PERSONNEL_SCROLL_REGION),
                 (int16_t)(guiSliderPosition + Y_OF_PERSONNEL_SCROLL_REGION),
                 VO_BLT_SRCTRANSPARENCY, NULL);
}

void ATMStartButtonCallback(GUI_BUTTON *btn, int32_t reason) {
  if (!(btn->uiFlags & BUTTON_ENABLED)) return;

  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    if (!(btn->uiFlags & BUTTON_CLICKED_ON)) {
      fReDrawScreenFlag = TRUE;
    }
    btn->uiFlags |= (BUTTON_CLICKED_ON);
  } else if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    if (btn->uiFlags & BUTTON_CLICKED_ON) {
      btn->uiFlags &= ~(BUTTON_CLICKED_ON);
      fReDrawScreenFlag = TRUE;
      fShowAtmPanel = TRUE;
      fShowAtmPanelStartButton = FALSE;
      fATMFlags = 0;
    }
  }
}

void PersonnelINVStartButtonCallback(GUI_BUTTON *btn, int32_t reason) {
  if (!(btn->uiFlags & BUTTON_ENABLED)) return;

  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    fReDrawScreenFlag = TRUE;
    btn->uiFlags |= (BUTTON_CLICKED_ON);
    ButtonList[giPersonnelATMStartButton[PERSONNEL_STAT_BTN]]->uiFlags &= ~(BUTTON_CLICKED_ON);
    ButtonList[giPersonnelATMStartButton[PERSONNEL_EMPLOYMENT_BTN]]->uiFlags &=
        ~(BUTTON_CLICKED_ON);
    //		fShowInventory = TRUE;
    gubPersonnelInfoState = PRSNL_INV;
  }
}

void PersonnelStatStartButtonCallback(GUI_BUTTON *btn, int32_t reason) {
  if (!(btn->uiFlags & BUTTON_ENABLED)) return;

  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    fReDrawScreenFlag = TRUE;
    btn->uiFlags |= BUTTON_CLICKED_ON;
    ButtonList[giPersonnelATMStartButton[PERSONNEL_EMPLOYMENT_BTN]]->uiFlags &=
        ~(BUTTON_CLICKED_ON);
    ButtonList[giPersonnelATMStartButton[PERSONNEL_INV_BTN]]->uiFlags &= ~(BUTTON_CLICKED_ON);
    //		fShowInventory = FALSE;
    gubPersonnelInfoState = PRSNL_STATS;
  }
}

void EmployementInfoButtonCallback(GUI_BUTTON *btn, int32_t reason) {
  if (!(btn->uiFlags & BUTTON_ENABLED)) return;

  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    fReDrawScreenFlag = TRUE;
    btn->uiFlags |= BUTTON_CLICKED_ON;
    ButtonList[giPersonnelATMStartButton[PERSONNEL_INV_BTN]]->uiFlags &= ~(BUTTON_CLICKED_ON);
    ButtonList[giPersonnelATMStartButton[PERSONNEL_STAT_BTN]]->uiFlags &= ~(BUTTON_CLICKED_ON);
    gubPersonnelInfoState = PRSNL_EMPLOYMENT;
  }
}

void ATMOther2ButtonCallback(GUI_BUTTON *btn, int32_t reason) {
  int32_t iValue = 0;

  if (!(btn->uiFlags & BUTTON_ENABLED)) return;

  iValue = MSYS_GetBtnUserData(btn, 0);

  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    if (!(btn->uiFlags & BUTTON_CLICKED_ON)) {
      fReDrawScreenFlag = TRUE;
    }
    btn->uiFlags |= (BUTTON_CLICKED_ON);

    switch (iValue) {
      case (DEPOSIT_ATM):
        fATMFlags = 2;
        fReDrawScreenFlag = TRUE;
        ButtonList[giPersonnelATMSideButton[WIDTHDRAWL_ATM]]->uiFlags &= ~(BUTTON_CLICKED_ON);
        break;
      case (WIDTHDRAWL_ATM):
        fATMFlags = 3;
        fReDrawScreenFlag = TRUE;
        ButtonList[giPersonnelATMSideButton[DEPOSIT_ATM]]->uiFlags &= ~(BUTTON_CLICKED_ON);
        break;
    }
  }
}

void ATMOtherButtonCallback(GUI_BUTTON *btn, int32_t reason) {
  int32_t iValue = 0;
  struct SOLDIERTYPE *pSoldier = MercPtrs[0];
  int32_t iId = 0;

  if (!(btn->uiFlags & BUTTON_ENABLED)) return;

  iValue = MSYS_GetBtnUserData(btn, 0);

  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    if (!(btn->uiFlags & BUTTON_CLICKED_ON)) {
      fReDrawScreenFlag = TRUE;
    }
    btn->uiFlags |= (BUTTON_CLICKED_ON);
  } else if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    if (btn->uiFlags & BUTTON_CLICKED_ON) {
      btn->uiFlags &= ~(BUTTON_CLICKED_ON);

      if (iCurrentPersonSelectedId != -1) {
        if (fCurrentTeamMode == TRUE) {
          iId = GetIdOfThisSlot(iCurrentPersonSelectedId);

          // set soldier
          pSoldier = MercPtrs[iId];

          switch (iValue) {
            case (OK_ATM):
              if (fATMFlags == 0) {
                fATMFlags = 1;
                fReDrawScreenFlag = TRUE;
                fOneFrameDelayInPersonnel = TRUE;
              } else if (fATMFlags == 2) {
                // deposit from merc to account
                if (GetFundsOnMerc(pSoldier) >= wcstol(sTransferString, NULL, 10)) {
                  if ((wcstol(sTransferString, NULL, 10) % 10) != 0) {
                    fOldATMFlags = fATMFlags;
                    fATMFlags = 5;

                    iValue = (wcstol(sTransferString, NULL, 10) -
                              (wcstol(sTransferString, NULL, 10) % 10));
                    swprintf(sTransferString, ARR_SIZE(sTransferString), L"%d", iValue);
                    fReDrawScreenFlag = TRUE;
                  } else {
                    // transfer
                    TransferFundsFromMercToBank(pSoldier, wcstol(sTransferString, NULL, 10));
                    sTransferString[0] = 0;
                    fReDrawScreenFlag = TRUE;
                  }
                } else {
                  fOldATMFlags = fATMFlags;
                  fATMFlags = 4;
                  iValue = GetFundsOnMerc(pSoldier);
                  swprintf(sTransferString, ARR_SIZE(sTransferString), L"%d", iValue);
                  fReDrawScreenFlag = TRUE;
                }
              } else if (fATMFlags == 3) {
                // deposit from merc to account
                if (MoneyGetBalance() >= wcstol(sTransferString, NULL, 10)) {
                  if ((wcstol(sTransferString, NULL, 10) % 10) != 0) {
                    fOldATMFlags = fATMFlags;
                    fATMFlags = 5;

                    iValue = (wcstol(sTransferString, NULL, 10) -
                              (wcstol(sTransferString, NULL, 10) % 10));
                    swprintf(sTransferString, ARR_SIZE(sTransferString), L"%d", iValue);
                    fReDrawScreenFlag = TRUE;
                  } else {
                    // transfer
                    TransferFundsFromBankToMerc(pSoldier, wcstol(sTransferString, NULL, 10));
                    sTransferString[0] = 0;
                    fReDrawScreenFlag = TRUE;
                  }
                } else {
                  fOldATMFlags = fATMFlags;
                  fATMFlags = 4;
                  iValue = MoneyGetBalance();
                  swprintf(sTransferString, ARR_SIZE(sTransferString), L"%d", iValue);
                  fReDrawScreenFlag = TRUE;
                }
              } else if (fATMFlags == 4) {
                fATMFlags = fOldATMFlags;
                fReDrawScreenFlag = TRUE;
              }
              break;
            case (DEPOSIT_ATM):
              fATMFlags = 2;
              fReDrawScreenFlag = TRUE;

              break;
            case (WIDTHDRAWL_ATM):
              fATMFlags = 3;
              fReDrawScreenFlag = TRUE;
              break;
            case (CANCEL_ATM):
              if (sTransferString[0] != 0) {
                sTransferString[0] = 0;
              } else if (fATMFlags != 0) {
                fATMFlags = 0;
                ButtonList[giPersonnelATMSideButton[WIDTHDRAWL_ATM]]->uiFlags &=
                    ~(BUTTON_CLICKED_ON);
                ButtonList[giPersonnelATMSideButton[DEPOSIT_ATM]]->uiFlags &= ~(BUTTON_CLICKED_ON);
              } else {
                fShowAtmPanel = FALSE;
                fShowAtmPanelStartButton = TRUE;
              }
              fReDrawScreenFlag = TRUE;
              break;
            case (CLEAR_ATM):
              sTransferString[0] = 0;
              fReDrawScreenFlag = TRUE;
              break;
          }
        }
      }
    }
  }
}

void ATMNumberButtonCallback(GUI_BUTTON *btn, int32_t reason) {
  int32_t iValue = 0;
  int32_t iCounter = 0;
  CHAR16 sZero[2] = L"0";

  if (!(btn->uiFlags & BUTTON_ENABLED)) return;

  iValue = MSYS_GetBtnUserData(btn, 0);

  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    if (!(btn->uiFlags & BUTTON_CLICKED_ON)) {
      fReDrawScreenFlag = TRUE;
    }
    btn->uiFlags |= (BUTTON_CLICKED_ON);
  } else if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    if (btn->uiFlags & BUTTON_CLICKED_ON) {
      btn->uiFlags &= ~(BUTTON_CLICKED_ON);
      // find position in value string, append character at end
      for (iCounter = 0; iCounter < (int32_t)wcslen(sTransferString); iCounter++);
      sTransferString[iCounter] = (sZero[0] + (uint16_t)iValue);
      sTransferString[iCounter + 1] = 0;
      fReDrawScreenFlag = TRUE;

      // gone too far
      if (StringPixLength(sTransferString, ATM_FONT) >= ATM_DISPLAY_WIDTH - 10) {
        sTransferString[iCounter] = 0;
      }
    }
  }
}

void DisplayATMAmount(void) {
  int16_t sX = 0, sY = 0;
  CHAR16 sTempString[32];
  int32_t iCounter = 0;

  if (fShowAtmPanel == FALSE) {
    return;
  }

  wcscpy(sTempString, sTransferString);

  if ((sTempString[0] == 48) && (sTempString[1] != 0)) {
    // strip the zero from the beginning
    for (iCounter = 1; iCounter < (int32_t)wcslen(sTempString); iCounter++) {
      sTempString[iCounter - 1] = sTempString[iCounter];
    }
  }

  // insert commas and dollar sign
  InsertCommasForDollarFigure(sTempString);
  InsertDollarSignInToString(sTempString);

  // set font
  SetFont(ATM_FONT);

  // set back and foreground
  SetFontForeground(FONT_WHITE);
  SetFontBackground(FONT_BLACK);

  // right justify
  FindFontRightCoordinates(ATM_DISPLAY_X, ATM_DISPLAY_Y + 37, ATM_DISPLAY_WIDTH, ATM_DISPLAY_HEIGHT,
                           sTempString, ATM_FONT, &sX, &sY);

  // print string
  mprintf(sX, sY, sTempString);

  return;
}

void HandleStateOfATMButtons(void) {
  int32_t iCounter = 0;

  // disable buttons based on state
  if ((fATMFlags == 0)) {
    for (iCounter = 0; iCounter < 10; iCounter++) {
      DisableButton(iNumberPadButtons[iCounter]);
    }

    for (iCounter = 0; iCounter < NUMBER_ATM_BUTTONS; iCounter++) {
      if ((iCounter != DEPOSIT_ATM) && (iCounter != WIDTHDRAWL_ATM) && (iCounter != CANCEL_ATM)) {
        DisableButton(giPersonnelATMSideButton[iCounter]);
      }
    }
  } else {
    for (iCounter = 0; iCounter < 10; iCounter++) {
      EnableButton(iNumberPadButtons[iCounter]);
    }

    for (iCounter = 0; iCounter < NUMBER_ATM_BUTTONS; iCounter++) {
      EnableButton(giPersonnelATMSideButton[iCounter]);
    }
  }
}

int32_t GetFundsOnMerc(struct SOLDIERTYPE *pSoldier) {
  int32_t iCurrentAmount = 0;
  int32_t iCurrentPocket = 0;
  // run through mercs pockets, if any money in them, add to total

  // error check
  if (pSoldier == NULL) {
    return 0;
  }

  // run through grunts pockets and count all the spare change
  for (iCurrentPocket = 0; iCurrentPocket < NUM_INV_SLOTS; iCurrentPocket++) {
    if (Item[pSoldier->inv[iCurrentPocket].usItem].usItemClass == IC_MONEY) {
      iCurrentAmount += pSoldier->inv[iCurrentPocket].uiMoneyAmount;
    }
  }

  return iCurrentAmount;
}

BOOLEAN TransferFundsFromMercToBank(struct SOLDIERTYPE *pSoldier, int32_t iCurrentBalance) {
  int32_t iCurrentPocket = 0;
  int32_t iAmountLeftToTake = iCurrentBalance;
  struct OBJECTTYPE ObjectToRemove;

  // move this amount of money from the grunt to the bank
  // error check
  if (pSoldier == NULL) {
    return FALSE;
  }

  // run through grunts pockets and count all the spare change
  for (iCurrentPocket = 0; iCurrentPocket < NUM_INV_SLOTS; iCurrentPocket++) {
    if (Item[pSoldier->inv[iCurrentPocket].usItem].usItemClass == IC_MONEY) {
      // is there more left to go, or does this pocket finish it off?
      if (pSoldier->inv[iCurrentPocket].uiMoneyAmount > (uint32_t)iAmountLeftToTake) {
        pSoldier->inv[iCurrentPocket].uiMoneyAmount -= iAmountLeftToTake;
        iAmountLeftToTake = 0;
      } else {
        iAmountLeftToTake -= pSoldier->inv[iCurrentPocket].uiMoneyAmount;
        pSoldier->inv[iCurrentPocket].uiMoneyAmount = 0;

        // Remove the item out off the merc
        RemoveObjectFromSlot(pSoldier, (int8_t)iCurrentPocket, &ObjectToRemove);
      }
    }
  }

  if (iAmountLeftToTake != 0) {
    // something wrong
    AddTransactionToPlayersBook(TRANSFER_FUNDS_FROM_MERC, GetSolProfile(pSoldier),
                                (iCurrentBalance - iAmountLeftToTake));
    return (FALSE);
  } else {
    // everything ok
    AddTransactionToPlayersBook(TRANSFER_FUNDS_FROM_MERC, GetSolProfile(pSoldier),
                                (iCurrentBalance));
    return (TRUE);
  }
}

BOOLEAN TransferFundsFromBankToMerc(struct SOLDIERTYPE *pSoldier, int32_t iCurrentBalance) {
  struct OBJECTTYPE pMoneyObject;

  // move this amount of money from the grunt to the bank
  // error check
  if (pSoldier == NULL) {
    return FALSE;
  }

  // make sure we are giving them some money
  if (iCurrentBalance <= 0) {
    return (FALSE);
  }

  // current balance
  if (iCurrentBalance > MoneyGetBalance()) {
    iCurrentBalance = MoneyGetBalance();
  }

  // set up object
  memset(&(pMoneyObject), 0, sizeof(struct OBJECTTYPE));

  // set up money object
  pMoneyObject.usItem = MONEY;
  pMoneyObject.ubNumberOfObjects = 1;
  pMoneyObject.bMoneyStatus = 100;
  pMoneyObject.bStatus[0] = 100;
  pMoneyObject.uiMoneyAmount = iCurrentBalance;

  // now auto place money object
  if (AutoPlaceObject(pSoldier, &(pMoneyObject), TRUE) == TRUE) {
    // now place transaction
    AddTransactionToPlayersBook(TRANSFER_FUNDS_TO_MERC, GetSolProfile(pSoldier),
                                -(iCurrentBalance));
  } else {
    // error, notify player that merc doesn't have the spce for this much cash
  }

  return (TRUE);
}

void DisplayATMStrings(void) {
  // display strings for ATM

  switch (fATMFlags) {
    case (0):
      if (fShowAtmPanelStartButton == FALSE) {
        DisplayWrappedString(509, (int16_t)(80), 81, 2, ATM_FONT, FONT_WHITE, sATMText[3],
                             FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED);
      }
      break;
    case (2):
      if (sTransferString[0] != 0) {
        DisplayWrappedString(509, 80, 81, 2, ATM_FONT, FONT_WHITE, sATMText[0], FONT_MCOLOR_BLACK,
                             FALSE, CENTER_JUSTIFIED);
        // DisplayWrappedString(509, ( int16_t )( 80 + GetFontHeight( ATM_FONT ) ), 81, 2, ATM_FONT,
        // FONT_WHITE, sATMText[ 1 ], FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED );

      } else {
        DisplayWrappedString(509, 80, 81, 2, ATM_FONT, FONT_WHITE, sATMText[2], FONT_MCOLOR_BLACK,
                             FALSE, CENTER_JUSTIFIED);
      }
      break;
    case (3):
      if (sTransferString[0] != 0) {
        DisplayWrappedString(509, 80, 81, 2, ATM_FONT, FONT_WHITE, sATMText[0], FONT_MCOLOR_BLACK,
                             FALSE, CENTER_JUSTIFIED);
        // DisplayWrappedString(509, ( int16_t )( 80 + GetFontHeight( ATM_FONT ) ), 81, 2, ATM_FONT,
        // FONT_WHITE, sATMText[ 1 ], FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED );
      } else {
        DisplayWrappedString(509, 80, 81, 2, ATM_FONT, FONT_WHITE, sATMText[2], FONT_MCOLOR_BLACK,
                             FALSE, CENTER_JUSTIFIED);
      }
      break;
    case (4):
      // not enough money
      DisplayWrappedString(509, 80, 81, 2, ATM_FONT, FONT_WHITE, sATMText[4], FONT_MCOLOR_BLACK,
                           FALSE, CENTER_JUSTIFIED);
      break;
    case (5):
      // not enough money
      DisplayWrappedString(509, 73, 81, 2, ATM_FONT, FONT_WHITE, sATMText[5], FONT_MCOLOR_BLACK,
                           FALSE, CENTER_JUSTIFIED);
      break;
  }
}

void UpDateStateOfStartButton(void) {
  int32_t iId = 0;

  // start button being shown?
  if (fShowAtmPanelStartButton == FALSE) {
    return;
  }

  //	if( fShowInventory == TRUE )
  if (gubPersonnelInfoState == PRSNL_INV) {
    ButtonList[giPersonnelATMStartButton[PERSONNEL_INV_BTN]]->uiFlags |= BUTTON_CLICKED_ON;
    ButtonList[giPersonnelATMStartButton[PERSONNEL_STAT_BTN]]->uiFlags &= ~(BUTTON_CLICKED_ON);
    ButtonList[giPersonnelATMStartButton[PERSONNEL_EMPLOYMENT_BTN]]->uiFlags &=
        ~(BUTTON_CLICKED_ON);
  } else if (gubPersonnelInfoState == PRSNL_STATS) {
    ButtonList[giPersonnelATMStartButton[PERSONNEL_INV_BTN]]->uiFlags &= ~BUTTON_CLICKED_ON;
    ButtonList[giPersonnelATMStartButton[PERSONNEL_STAT_BTN]]->uiFlags |= BUTTON_CLICKED_ON;
    ButtonList[giPersonnelATMStartButton[PERSONNEL_EMPLOYMENT_BTN]]->uiFlags &=
        ~(BUTTON_CLICKED_ON);
  } else {
    ButtonList[giPersonnelATMStartButton[PERSONNEL_STAT_BTN]]->uiFlags &= ~BUTTON_CLICKED_ON;
    ButtonList[giPersonnelATMStartButton[PERSONNEL_INV_BTN]]->uiFlags &= ~(BUTTON_CLICKED_ON);
    ButtonList[giPersonnelATMStartButton[PERSONNEL_EMPLOYMENT_BTN]]->uiFlags |= BUTTON_CLICKED_ON;
  }

  // if in current mercs and the currently selected guy is valid, enable button, else disable it
  if (fCurrentTeamMode == TRUE) {
    // is the current guy valid
    if (GetNumberOfMercsDeadOrAliveOnPlayersTeam() > 0) {
      // EnableButton( giPersonnelATMStartButton[ 0 ] );
      EnableButton(giPersonnelATMStartButton[PERSONNEL_STAT_BTN]);
      EnableButton(giPersonnelATMStartButton[PERSONNEL_INV_BTN]);
      EnableButton(giPersonnelATMStartButton[PERSONNEL_EMPLOYMENT_BTN]);

      iId = GetIdOfThisSlot(iCurrentPersonSelectedId);

      if (iId != -1) {
        if (Menptr[iId].bAssignment == ASSIGNMENT_POW) {
          // DisableButton( giPersonnelATMStartButton[ 0 ] );
          DisableButton(giPersonnelATMStartButton[PERSONNEL_INV_BTN]);

          //					if( fShowInventory == TRUE )
          if (gubPersonnelInfoState == PRSNL_INV) {
            //						fShowInventory = FALSE;
            gubPersonnelInfoState = PRSNL_STATS;

            fPausedReDrawScreenFlag = TRUE;
          }

          if (fATMFlags) {
            fATMFlags = 0;
            fPausedReDrawScreenFlag = TRUE;
          }
        }
      }

    } else {
      // not valid, disable
      // DisableButton( giPersonnelATMStartButton[ 0 ] );
      DisableButton(giPersonnelATMStartButton[PERSONNEL_STAT_BTN]);
      DisableButton(giPersonnelATMStartButton[PERSONNEL_INV_BTN]);
      DisableButton(giPersonnelATMStartButton[PERSONNEL_EMPLOYMENT_BTN]);
    }

  } else {
    // disable button
    // DisableButton( giPersonnelATMStartButton[ 0 ] );
    EnableButton(giPersonnelATMStartButton[PERSONNEL_STAT_BTN]);
    DisableButton(giPersonnelATMStartButton[PERSONNEL_INV_BTN]);
    DisableButton(giPersonnelATMStartButton[PERSONNEL_EMPLOYMENT_BTN]);
  }
}

void DisplayAmountOnCurrentMerc(void) {
  // will display the amount that the merc is carrying on him or herself
  int32_t iId;
  struct SOLDIERTYPE *pSoldier = NULL;
  int32_t iFunds;
  CHAR16 sString[64];
  int16_t sX, sY;

  iId = GetIdOfThisSlot(iCurrentPersonSelectedId);

  if (iId == -1) {
    pSoldier = NULL;
  } else {
    // set soldier
    pSoldier = MercPtrs[iId];
  }

  iFunds = GetFundsOnMerc(pSoldier);

  swprintf(sString, ARR_SIZE(sString), L"%d", iFunds);

  // insert commas and dollar sign
  InsertCommasForDollarFigure(sString);
  InsertDollarSignInToString(sString);

  // set font
  SetFont(ATM_FONT);

  // set back and foreground
  SetFontForeground(FONT_WHITE);
  SetFontBackground(FONT_BLACK);

  // right justify
  FindFontRightCoordinates(ATM_DISPLAY_X, ATM_DISPLAY_Y, ATM_DISPLAY_WIDTH, ATM_DISPLAY_HEIGHT,
                           sString, ATM_FONT, &sX, &sY);

  // print string
  mprintf(sX, sY, sString);

  return;
}

void HandlePersonnelKeyboard(void) {
  int32_t iCounter = 0;
  int32_t iValue = 0;
  CHAR16 sZero[2] = L"0";

  InputAtom InputEvent;

  while (DequeueEvent(&InputEvent) == TRUE) {
    if ((InputEvent.usEvent == KEY_DOWN) && (InputEvent.usParam >= '0') &&
        (InputEvent.usParam <= '9')) {
      if ((fShowAtmPanel) && (fATMFlags != 0)) {
        iValue = (int32_t)(InputEvent.usParam - '0');

        for (iCounter = 0; iCounter < (int32_t)wcslen(sTransferString); iCounter++);
        sTransferString[iCounter] = (sZero[0] + (uint16_t)iValue);
        sTransferString[iCounter + 1] = 0;
        fPausedReDrawScreenFlag = TRUE;

        // gone too far
        if (StringPixLength(sTransferString, ATM_FONT) >= ATM_DISPLAY_WIDTH - 10) {
          sTransferString[iCounter] = 0;
        }
      }
    }

    HandleKeyBoardShortCutsForLapTop(InputEvent.usEvent, InputEvent.usParam, InputEvent.usKeyState);
  }
}

void RenderRectangleForPersonnelTransactionAmount(void) {
  int32_t iLength = 0;
  int32_t iHeight = GetFontHeight(ATM_FONT);
  uint32_t uiDestPitchBYTES;
  uint8_t *pDestBuf;
  CHAR16 sTempString[32];
  int32_t iCounter = 0;

  wcscpy(sTempString, sTransferString);

  if ((sTempString[0] == 48) && (sTempString[1] != 0)) {
    // strip the zero from the beginning
    for (iCounter = 1; iCounter < (int32_t)wcslen(sTempString); iCounter++) {
      sTempString[iCounter - 1] = sTempString[iCounter];
    }
  }

  // insert commas and dollar sign
  InsertCommasForDollarFigure(sTempString);
  InsertDollarSignInToString(sTempString);

  // string not worth worrying about?
  if (wcslen(sTempString) < 2) {
    return;
  }

  // grab total length
  iLength = StringPixLength(sTempString, ATM_FONT);

  pDestBuf = LockVideoSurface(FRAME_BUFFER, &uiDestPitchBYTES);
  RestoreClipRegionToFullScreenForRectangle(uiDestPitchBYTES);
  RectangleDraw(TRUE, (ATM_DISPLAY_X + ATM_DISPLAY_WIDTH) - iLength - 2, ATM_DISPLAY_Y + 35,
                ATM_DISPLAY_X + ATM_DISPLAY_WIDTH + 1, ATM_DISPLAY_Y + iHeight + 36,
                Get16BPPColor(FROMRGB(255, 255, 255)), pDestBuf);
  UnLockVideoSurface(FRAME_BUFFER);
}

void HandleTimedAtmModes(void) {
  static BOOLEAN fOldAtmMode = 0;
  static uint32_t uiBaseTime = 0;

  if (fShowAtmPanel == FALSE) {
    return;
  }

  // update based on modes
  if (fATMFlags != fOldAtmMode) {
    uiBaseTime = GetJA2Clock();
    fOldAtmMode = fATMFlags;
    fPausedReDrawScreenFlag = TRUE;
  }

  if ((GetJA2Clock() - uiBaseTime) > DELAY_PER_MODE_CHANGE_IN_ATM) {
    switch (fATMFlags) {
      case (4):
      case (5):
        // insufficient funds ended
        fATMFlags = fOldATMFlags;
        fPausedReDrawScreenFlag = TRUE;
        break;
    }
  }
}

BOOLEAN IsPastMercDead(int32_t iId) {
  if (GetTheStateOfDepartedMerc(GetIdOfPastMercInSlot(iId)) == DEPARTED_DEAD) {
    return (TRUE);
  } else {
    return (FALSE);
  }
}

BOOLEAN IsPastMercFired(int32_t iId) {
  if (GetTheStateOfDepartedMerc(GetIdOfPastMercInSlot(iId)) == DEPARTED_FIRED) {
    return (TRUE);
  } else {
    return (FALSE);
  }
}

BOOLEAN IsPastMercOther(int32_t iId) {
  if (GetTheStateOfDepartedMerc(GetIdOfPastMercInSlot(iId)) == DEPARTED_OTHER) {
    return (TRUE);
  } else {
    return (FALSE);
  }
}

void DisplayEmploymentinformation(int32_t iId, int32_t iSlot) {
  int32_t iCounter = 0;
  wchar_t sString[50];
  wchar_t sStringA[50];
  int16_t sX, sY;

  if (Menptr[iId].uiStatusFlags & SOLDIER_VEHICLE) {
    return;
  }

  // display the stats for a char
  for (iCounter = 0; iCounter < MAX_STATS; iCounter++) {
    switch (iCounter) {
        //		 case 12:

      // Remaining Contract:
      case 0: {
        uint32_t uiMinutesInDay = 24 * 60;

        if (Menptr[iId].ubWhatKindOfMercAmI == MERC_TYPE__AIM_MERC ||
            Menptr[iId].ubProfile == SLAY) {
          int32_t iTimeLeftOnContract = CalcTimeLeftOnMercContract(GetSoldierByID(iId));

          // if the merc is in transit
          if (Menptr[iId].bAssignment == IN_TRANSIT) {
            // and if the ttime left on the cotract is greater then the contract time
            if (iTimeLeftOnContract >
                (int32_t)(Menptr[iId].iTotalContractLength * uiMinutesInDay)) {
              iTimeLeftOnContract = (Menptr[iId].iTotalContractLength * uiMinutesInDay);
            }
          }
          // if there is going to be a both days and hours left on the contract
          if (iTimeLeftOnContract / uiMinutesInDay) {
            swprintf(sString, ARR_SIZE(sString), L"%d%s %d%s / %d%s",
                     (iTimeLeftOnContract / uiMinutesInDay),
                     gpStrategicString[STR_PB_DAYS_ABBREVIATION],
                     (iTimeLeftOnContract % uiMinutesInDay) / 60,
                     gpStrategicString[STR_PB_HOURS_ABBREVIATION], Menptr[iId].iTotalContractLength,
                     gpStrategicString[STR_PB_DAYS_ABBREVIATION]);
            mprintf((int16_t)(pPersonnelScreenPoints[iCounter].x + (iSlot * TEXT_BOX_WIDTH)),
                    pPersonnelScreenPoints[iCounter].y,
                    pPersonnelScreenStrings[PRSNL_TXT_CURRENT_CONTRACT]);
          }

          // else there is under a day left
          else {
            // DEF: removed 2/7/99
            swprintf(sString, ARR_SIZE(sString), L"%d%s / %d%s",
                     (iTimeLeftOnContract % uiMinutesInDay) / 60,
                     gpStrategicString[STR_PB_HOURS_ABBREVIATION], Menptr[iId].iTotalContractLength,
                     gpStrategicString[STR_PB_DAYS_ABBREVIATION]);
            mprintf((int16_t)(pPersonnelScreenPoints[iCounter].x + (iSlot * TEXT_BOX_WIDTH)),
                    pPersonnelScreenPoints[iCounter].y,
                    pPersonnelScreenStrings[PRSNL_TXT_CURRENT_CONTRACT]);
          }

        } else if (Menptr[iId].ubWhatKindOfMercAmI == MERC_TYPE__MERC) {
          //					swprintf(sString, ARR_SIZE(sString), L"%d%s /
          //%d%s",Menptr[iId].iTotalContractLength, gpStrategicString[ STR_PB_DAYS_ABBREVIATION ], (
          // GetWorldTotalMin( ) -Menptr[iId].iStartContractTime ) / ( 24 * 60 ), gpStrategicString[
          // STR_PB_DAYS_ABBREVIATION ] );

          wcscpy(sString, gpStrategicString[STR_PB_NOTAPPLICABLE_ABBREVIATION]);
          mprintf((int16_t)(pPersonnelScreenPoints[iCounter].x + (iSlot * TEXT_BOX_WIDTH)),
                  pPersonnelScreenPoints[iCounter].y,
                  pPersonnelScreenStrings[PRSNL_TXT_CURRENT_CONTRACT]);
        } else {
          wcscpy(sString, gpStrategicString[STR_PB_NOTAPPLICABLE_ABBREVIATION]);
          mprintf((int16_t)(pPersonnelScreenPoints[iCounter].x + (iSlot * TEXT_BOX_WIDTH)),
                  pPersonnelScreenPoints[iCounter].y,
                  pPersonnelScreenStrings[PRSNL_TXT_CURRENT_CONTRACT]);
        }

        FindFontRightCoordinates((int16_t)(pPersonnelScreenPoints[iCounter].x +
                                           (iSlot * TEXT_BOX_WIDTH) + Prsnl_DATA_OffSetX),
                                 0, TEXT_BOX_WIDTH - 20, 0, sString, PERS_FONT, &sX, &sY);
        mprintf(sX, pPersonnelScreenPoints[iCounter].y, sString);
      } break;

        //		 case 11:
        //		 case 19:
      case 1:

        // total contract time served
        mprintf((int16_t)(pPersonnelScreenPoints[iCounter].x + (iSlot * TEXT_BOX_WIDTH)),
                pPersonnelScreenPoints[iCounter].y,
                pPersonnelScreenStrings[PRSNL_TXT_TOTAL_SERVICE]);

        //./DEF 2/4/99: total service days used to be calced as 'days -1'

        swprintf(sString, ARR_SIZE(sString), L"%d %s",
                 gMercProfiles[Menptr[iId].ubProfile].usTotalDaysServed,
                 gpStrategicString[STR_PB_DAYS_ABBREVIATION]);

        FindFontRightCoordinates((int16_t)(pPersonnelScreenPoints[iCounter].x +
                                           (iSlot * TEXT_BOX_WIDTH) + Prsnl_DATA_OffSetX),
                                 0, TEXT_BOX_WIDTH - 20, 0, sString, PERS_FONT, &sX, &sY);
        mprintf(sX, pPersonnelScreenPoints[iCounter].y, sString);
        break;

        //		 case 13:
      case 3:
        // cost (PRSNL_TXT_TOTAL_COST)

        /*
                                 if( Menptr[iId].ubWhatKindOfMercAmI == MERC_TYPE__AIM_MERC)
                                 {
                                         uint32_t uiDailyCost = 0;

                                         if( Menptr[iId].bTypeOfLastContract ==
        CONTRACT_EXTEND_2_WEEK )
                                         {
                                                 // 2 week contract
                                                 uiDailyCost = gMercProfiles[ Menptr[ iId
        ].ubProfile ].uiBiWeeklySalary / 14;
                                         }
                                         else if( Menptr[iId].bTypeOfLastContract ==
        CONTRACT_EXTEND_1_WEEK )
                                         {
                                                 // 1 week contract
                                                 uiDailyCost = gMercProfiles[ Menptr[ iId
        ].ubProfile ].uiWeeklySalary / 7;
                                         }
                                         else
                                         {
                                                 uiDailyCost = gMercProfiles[ Menptr[ iId
        ].ubProfile ].sSalary;
                                         }

        //				 swprintf( sString, L"%d",uiDailyCost * Menptr[ iId
        ].iTotalContractLength ); swprintf( sString, L"%d", gMercProfiles[ Menptr[ iId ].ubProfile
        ].uiTotalCostToDate );
                                 }
                                 else if( Menptr[iId].ubWhatKindOfMercAmI == MERC_TYPE__MERC)
                                 {
        //					swprintf( sString, L"%d",gMercProfiles[ Menptr[ iId
        ].ubProfile ].sSalary * gMercProfiles[ Menptr[ iId ].ubProfile ].iMercMercContractLength );
                                                swprintf( sString, L"%d", gMercProfiles[ Menptr[ iId
        ].ubProfile ].uiTotalCostToDate );
                                 }
                                 else
                                 {
                                         //Display a $0 amount
        //				 swprintf( sString, L"0" );

                                         swprintf( sString, L"%d", gMercProfiles[ Menptr[ iId
        ].ubProfile ].uiTotalCostToDate );
                                 }
        */
        swprintf(sString, ARR_SIZE(sString), L"%d",
                 gMercProfiles[Menptr[iId].ubProfile].uiTotalCostToDate);

        // insert commas and dollar sign
        InsertCommasForDollarFigure(sString);
        InsertDollarSignInToString(sString);

        /*
        DEF:3/19/99:
                                 if( Menptr[iId].ubWhatKindOfMercAmI == MERC_TYPE__MERC )
                                 {
                                   swprintf( sStringA, L"%s", pPersonnelScreenStrings[
        PRSNL_TXT_UNPAID_AMOUNT ] );
                                 }
                                 else
        */
        {
          swprintf(sStringA, ARR_SIZE(sStringA), L"%s",
                   pPersonnelScreenStrings[PRSNL_TXT_TOTAL_COST]);
        }

        FindFontRightCoordinates((int16_t)(pPersonnelScreenPoints[iCounter].x +
                                           (iSlot * TEXT_BOX_WIDTH) + Prsnl_DATA_OffSetX),
                                 0, TEXT_BOX_WIDTH - 20, 0, sString, PERS_FONT, &sX, &sY);
        mprintf((int16_t)(pPersonnelScreenPoints[iCounter].x + (iSlot * TEXT_BOX_WIDTH)),
                pPersonnelScreenPoints[iCounter].y, sStringA);

        // print contract cost
        mprintf((int16_t)(sX), pPersonnelScreenPoints[iCounter].y, sString);

        if (Menptr[iId].ubWhatKindOfMercAmI == MERC_TYPE__AIM_MERC) {
          // daily rate
          if (Menptr[iId].bTypeOfLastContract == CONTRACT_EXTEND_2_WEEK) {
            // 2 week contract
            swprintf(sStringA, ARR_SIZE(sStringA), L"%d",
                     gMercProfiles[Menptr[iId].ubProfile].uiBiWeeklySalary / 14);
            InsertCommasForDollarFigure(sStringA);
            InsertDollarSignInToString(sStringA);
            swprintf(sString, ARR_SIZE(sString), L"%s", sStringA);
          } else if (Menptr[iId].bTypeOfLastContract == CONTRACT_EXTEND_1_WEEK) {
            // 1 week contract
            swprintf(sStringA, ARR_SIZE(sStringA), L"%d",
                     gMercProfiles[Menptr[iId].ubProfile].uiWeeklySalary / 7);
            InsertCommasForDollarFigure(sStringA);
            InsertDollarSignInToString(sStringA);
            swprintf(sString, ARR_SIZE(sString), L"%s", sStringA);
          } else {
            swprintf(sStringA, ARR_SIZE(sStringA), L"%d",
                     gMercProfiles[Menptr[iId].ubProfile].sSalary);
            InsertCommasForDollarFigure(sStringA);
            InsertDollarSignInToString(sStringA);
            swprintf(sString, ARR_SIZE(sString), L"%s", sStringA);
          }
        } else if (Menptr[iId].ubWhatKindOfMercAmI == MERC_TYPE__MERC) {
          // DEF: 99/2/7
          //				 swprintf( sStringA, L"%d", gMercProfiles[Menptr[ iId
          //].ubProfile].sSalary * Menptr[ iId ].iTotalContractLength);
          swprintf(sStringA, ARR_SIZE(sStringA), L"%d",
                   gMercProfiles[Menptr[iId].ubProfile].sSalary);
          InsertCommasForDollarFigure(sStringA);
          InsertDollarSignInToString(sStringA);
          swprintf(sString, ARR_SIZE(sString), L"%s", sStringA);
        }

        else {
          // Display a $0 amount
          //				 swprintf( sString, L"0" );
          //				 InsertDollarSignInToString( sString );
          swprintf(sStringA, ARR_SIZE(sStringA), L"%d",
                   gMercProfiles[Menptr[iId].ubProfile].sSalary);
          InsertCommasForDollarFigure(sStringA);
          InsertDollarSignInToString(sStringA);
          swprintf(sString, ARR_SIZE(sString), L"%s", sStringA);
        }

        FindFontRightCoordinates((int16_t)(pPersonnelScreenPoints[iCounter].x +
                                           (iSlot * TEXT_BOX_WIDTH) + Prsnl_DATA_OffSetX),
                                 0, TEXT_BOX_WIDTH - 20, 0, sString, PERS_FONT, &sX, &sY);

        //			 iCounter++;
        iCounter++;

        // now print daily rate
        mprintf((int16_t)(sX), pPersonnelScreenPoints[iCounter + 1].y, sString);
        mprintf((int16_t)(pPersonnelScreenPoints[iCounter + 1].x + (iSlot * TEXT_BOX_WIDTH)),
                pPersonnelScreenPoints[iCounter + 1].y,
                pPersonnelScreenStrings[PRSNL_TXT_DAILY_COST]);

        break;

      case 5:
        // medical deposit

        // if its a merc merc, display the salary oweing
        if (Menptr[iId].ubWhatKindOfMercAmI == MERC_TYPE__MERC) {
          mprintf((int16_t)(pPersonnelScreenPoints[iCounter - 1].x + (iSlot * TEXT_BOX_WIDTH)),
                  pPersonnelScreenPoints[iCounter - 1].y,
                  pPersonnelScreenStrings[PRSNL_TXT_UNPAID_AMOUNT]);

          swprintf(sString, ARR_SIZE(sString), L"%d",
                   gMercProfiles[Menptr[iId].ubProfile].sSalary *
                       gMercProfiles[Menptr[iId].ubProfile].iMercMercContractLength);
          InsertCommasForDollarFigure(sString);
          InsertDollarSignInToString(sString);

          FindFontRightCoordinates((int16_t)(pPersonnelScreenPoints[iCounter - 1].x +
                                             (iSlot * TEXT_BOX_WIDTH) + Prsnl_DATA_OffSetX),
                                   0, TEXT_BOX_WIDTH - 20, 0, sString, PERS_FONT, &sX, &sY);
          mprintf(sX, pPersonnelScreenPoints[iCounter - 1].y, sString);
        } else {
          mprintf((int16_t)(pPersonnelScreenPoints[iCounter - 1].x + (iSlot * TEXT_BOX_WIDTH)),
                  pPersonnelScreenPoints[iCounter - 1].y,
                  pPersonnelScreenStrings[PRSNL_TXT_MED_DEPOSIT]);

          swprintf(sString, ARR_SIZE(sString), L"%d",
                   gMercProfiles[Menptr[iId].ubProfile].sMedicalDepositAmount);

          // insert commas and dollar sign
          InsertCommasForDollarFigure(sString);
          InsertDollarSignInToString(sString);

          FindFontRightCoordinates((int16_t)(pPersonnelScreenPoints[iCounter - 1].x +
                                             (iSlot * TEXT_BOX_WIDTH) + Prsnl_DATA_OffSetX),
                                   0, TEXT_BOX_WIDTH - 20, 0, sString, PERS_FONT, &sX, &sY);
          mprintf(sX, pPersonnelScreenPoints[iCounter - 1].y, sString);
        }

        break;
    }
  }
}

// AIM merc:  Returns the amount of time left on mercs contract
// MERC merc: Returns the amount of time the merc has worked
// IMP merc:	Returns the amount of time the merc has worked
// else:			returns -1
int32_t CalcTimeLeftOnMercContract(struct SOLDIERTYPE *pSoldier) {
  int32_t iTimeLeftOnContract = -1;

  if (pSoldier->ubWhatKindOfMercAmI == MERC_TYPE__AIM_MERC) {
    iTimeLeftOnContract = pSoldier->iEndofContractTime - GetWorldTotalMin();

    if (iTimeLeftOnContract < 0) iTimeLeftOnContract = 0;
  } else if (pSoldier->ubWhatKindOfMercAmI == MERC_TYPE__MERC) {
    iTimeLeftOnContract = gMercProfiles[GetSolProfile(pSoldier)].iMercMercContractLength;
  }

  else if (pSoldier->ubWhatKindOfMercAmI == MERC_TYPE__PLAYER_CHARACTER) {
    iTimeLeftOnContract = pSoldier->iTotalContractLength;
  }

  else {
    iTimeLeftOnContract = -1;
  }

  return (iTimeLeftOnContract);
}
