// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#ifndef __LAPTOP_H
#define __LAPTOP_H

#include "MessageBoxScreen.h"
#include "SGP/Types.h"
#include "Utils/FontControl.h"

extern uint32_t guiDropDownBorder;
extern struct MOUSE_REGION pScreenMask;

int32_t EnterLaptop();
void ExitLaptop();
void RenderLaptop();
uint32_t ExitLaptopMode(uint32_t uiMode);
void EnterNewLaptopMode();
uint32_t DrawLapTopIcons();
uint32_t DrawLapTopText();
void ReDrawHighLight();
void DrawButtonText();
void InitBookMarkList();
void SetBookMark(int32_t iBookId);
void RemoveBookmark(int32_t iBookId);
BOOLEAN LeaveLapTopScreen();
void SetLaptopExitScreen(uint32_t uiExitScreen);
void SetLaptopNewGameFlag();
uint32_t DrawLapTopIcons();
void LapTopScreenCallBack(struct MOUSE_REGION *pRegion, int32_t iReason);
void HandleRightButtonUpEvent(void);
BOOLEAN DoLapTopMessageBox(uint8_t ubStyle, wchar_t *zString, uint32_t uiExitScreen, uint8_t ubFlags,
                           MSGBOX_CALLBACK ReturnCallback);
void GoToWebPage(int32_t iPageId);
BOOLEAN WebPageTileBackground(uint8_t ubNumX, uint8_t ubNumY, uint16_t usWidth, uint16_t usHeight,
                              uint32_t uiBackGround);
void BlitTitleBarIcons(void);
void HandleKeyBoardShortCutsForLapTop(uint16_t usEvent, uint32_t usParam, uint16_t usKeyState);
BOOLEAN RenderWWWProgramTitleBar(void);
void DisplayProgramBoundingBox(BOOLEAN fMarkButtons);
BOOLEAN DoLapTopSystemMessageBox(uint8_t ubStyle, wchar_t *zString, uint32_t uiExitScreen,
                                 uint16_t usFlags, MSGBOX_CALLBACK ReturnCallback);
BOOLEAN RemoveBookMark(int32_t iBookId);
void CreateFileAndNewEmailIconFastHelpText(uint32_t uiHelpTextID, BOOLEAN fClearHelpText);
BOOLEAN InitLaptopAndLaptopScreens();

// clear out all tempf iles from laptop
void ClearOutTempLaptopFiles(void);

void HaventMadeImpMercEmailCallBack();

extern uint32_t guiCurrentLaptopMode;
extern uint32_t guiPreviousLaptopMode;
extern int32_t giCurrentSubPage;
extern uint32_t guiCurrentLapTopCursor;
extern uint32_t guiPreviousLapTopCursor;
extern BOOLEAN fMarkButtonsDirtyFlag;
extern BOOLEAN fNotVistedImpYet;
extern BOOLEAN fReDrawScreenFlag;
extern BOOLEAN fPausedReDrawScreenFlag;
extern BOOLEAN fIntermediateReDrawFlag;
extern BOOLEAN fLoadPendingFlag;
extern BOOLEAN fEscKeyHandled;
extern BOOLEAN fReDrawPostButtonRender;
extern BOOLEAN fCurrentlyInLaptop;
// extern BOOLEAN gfNewGameLaptop;
extern uint32_t guiLaptopBACKGROUND;
extern uint32_t guiTITLEBARICONS;
extern BOOLEAN fDoneLoadPending;
extern BOOLEAN fReConnectingFlag;
extern BOOLEAN fConnectingToSubPage;
extern BOOLEAN fFastLoadFlag;
extern BOOLEAN gfShowBookmarks;
extern BOOLEAN fShowBookmarkInfo;
extern BOOLEAN fReDrawBookMarkInfo;

extern uint32_t guiTITLE;
extern uint32_t guiTOP;
extern uint32_t guiLONGLINE;
extern uint32_t guiSHADELINE;

extern uint32_t guiBottomButton;
extern uint32_t guiBottomButton2;
extern uint8_t gubCurPageNum;

extern uint32_t guiContentButton;

enum {
  LAPTOP_MODE_NONE = 0,
  LAPTOP_MODE_FINANCES,
  LAPTOP_MODE_PERSONNEL,
  LAPTOP_MODE_HISTORY,
  LAPTOP_MODE_FILES,
  LAPTOP_MODE_FILES_ENRICO,
  LAPTOP_MODE_FILES_PLANS,
  LAPTOP_MODE_EMAIL,
  LAPTOP_MODE_EMAIL_NEW,
  LAPTOP_MODE_EMAIL_VIEW,
  LAPTOP_MODE_WWW,
  LAPTOP_MODE_AIM,
  LAPTOP_MODE_AIM_MEMBERS,
  LAPTOP_MODE_AIM_MEMBERS_FACIAL_INDEX,
  LAPTOP_MODE_AIM_MEMBERS_SORTED_FILES,
  LAPTOP_MODE_AIM_MEMBERS_SORTED_FILES_VIDEO,
  LAPTOP_MODE_AIM_MEMBERS_ARCHIVES,
  LAPTOP_MODE_AIM_POLICIES,
  LAPTOP_MODE_AIM_HISTORY,
  LAPTOP_MODE_AIM_LINKS,
  LAPTOP_MODE_MERC,
  LAPTOP_MODE_MERC_ACCOUNT,
  LAPTOP_MODE_MERC_NO_ACCOUNT,
  LAPTOP_MODE_MERC_FILES,
  LAPTOP_MODE_BOBBY_R,
  LAPTOP_MODE_BOBBY_R_GUNS,
  LAPTOP_MODE_BOBBY_R_AMMO,
  LAPTOP_MODE_BOBBY_R_ARMOR,
  LAPTOP_MODE_BOBBY_R_MISC,
  LAPTOP_MODE_BOBBY_R_USED,
  LAPTOP_MODE_BOBBY_R_MAILORDER,
  LAPTOP_MODE_CHAR_PROFILE,
  LAPTOP_MODE_CHAR_PROFILE_QUESTIONAIRE,
  LAPTOP_MODE_FLORIST,
  LAPTOP_MODE_FLORIST_FLOWER_GALLERY,
  LAPTOP_MODE_FLORIST_ORDERFORM,
  LAPTOP_MODE_FLORIST_CARD_GALLERY,
  LAPTOP_MODE_INSURANCE,
  LAPTOP_MODE_INSURANCE_INFO,
  LAPTOP_MODE_INSURANCE_CONTRACT,
  LAPTOP_MODE_INSURANCE_COMMENTS,
  LAPTOP_MODE_FUNERAL,
  LAPTOP_MODE_SIRTECH,
  LAPTOP_MODE_BROKEN_LINK,
  LAPTOP_MODE_BOBBYR_SHIPMENTS,
};

// bookamrks for WWW bookmark list

enum {
  FIRST_SIDE_PANEL = 1,
  SECOND_SIDE_PANEL,
};
enum {
  LAPTOP_NO_CURSOR = 0,
  LAPTOP_PANEL_CURSOR,
  LAPTOP_SCREEN_CURSOR,
  LAPTOP_WWW_CURSOR,
};

#define LAPTOP_SIDE_PANEL_X 0
#define LAPTOP_SIDE_PANEL_Y 0
#define LAPTOP_SIDE_PANEL_WIDTH 640
#define LAPTOP_SIDE_PANEL_HEIGHT 480
#define LAPTOP_X 0
#define LAPTOP_Y 0

#define LAPTOP_SCREEN_UL_X 111
#define LAPTOP_SCREEN_UL_Y 27
#define LAPTOP_SCREEN_LR_X 613
#define LAPTOP_SCREEN_LR_Y 427
#define LAPTOP_UL_X 24
#define LAPTOP_UL_Y 27
#define LAPTOP_SCREEN_WIDTH LAPTOP_SCREEN_LR_X - LAPTOP_SCREEN_UL_X
#define LAPTOP_SCREEN_HEIGHT LAPTOP_SCREEN_LR_Y - LAPTOP_SCREEN_UL_Y

// new positions for web browser
#define LAPTOP_SCREEN_WEB_UL_Y LAPTOP_SCREEN_UL_Y + 19
#define LAPTOP_SCREEN_WEB_LR_Y LAPTOP_SCREEN_WEB_UL_Y + LAPTOP_SCREEN_HEIGHT
#define LAPTOP_SCREEN_WEB_DELTA_Y LAPTOP_SCREEN_WEB_UL_Y - LAPTOP_SCREEN_UL_Y

// the laptop on/off button
#define ON_X 113
#define ON_Y 445

// the bookmark values, move cancel down as bookmarks added

enum {
  AIM_BOOKMARK = 0,
  BOBBYR_BOOKMARK,
  IMP_BOOKMARK,
  MERC_BOOKMARK,
  FUNERAL_BOOKMARK,
  FLORIST_BOOKMARK,
  INSURANCE_BOOKMARK,
  CANCEL_STRING,
};

extern uint32_t guiLaptopBACKGROUND;

#define DEAD_MERC_COLOR_RED 255
#define DEAD_MERC_COLOR_GREEN 55
#define DEAD_MERC_COLOR_BLUE 55

#endif
