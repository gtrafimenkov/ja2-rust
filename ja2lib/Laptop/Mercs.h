#ifndef __MERCS_H
#define __MERCS_H

#include "SGP/Types.h"

#define MERC_BUTTON_UP_COLOR FONT_MCOLOR_WHITE
#define MERC_BUTTON_DOWN_COLOR FONT_MCOLOR_DKWHITE

#define NUMBER_OF_MERCS 11
#define LAST_MERC_ID 10
#define NUMBER_OF_BAD_MERCS 5
#define NUMBER_MERCS_AFTER_FIRST_MERC_ARRIVES 6
#define NUMBER_MERCS_AFTER_SECOND_MERC_ARRIVES 8
#define NUMBER_MERCS_AFTER_THIRD_MERC_ARRIVES 9
#define NUMBER_MERCS_AFTER_FOURTH_MERC_ARRIVES 10

#define MERC_NUM_DAYS_TILL_FIRST_WARNING 7
#define MERC_NUM_DAYS_TILL_ACCOUNT_SUSPENDED 9
#define MERC_NUM_DAYS_TILL_ACCOUNT_INVALID 12

#define MERC_LARRY_ROACHBURN 7

#define DAYS_TIL_M_E_R_C_AVAIL 3

// The players account information for the MERC site
enum {
  MERC_NO_ACCOUNT,
  MERC_ACCOUNT_SUSPENDED,
  MERC_ACCOUNT_INVALID,
  MERC_ACCOUNT_VALID_FIRST_WARNING,
  MERC_ACCOUNT_VALID,
};
// extern	UINT8			gubPlayersMercAccountStatus;
// extern	UINT32		guiPlayersMercAccountNumber;

// The video conferencing for the merc page
#define MERC_VIDEO_SPECK_SPEECH_NOT_TALKING 65535
#define MERC_VIDEO_SPECK_HAS_TO_TALK_BUT_QUOTE_NOT_CHOSEN_YET 65534

// used with the gubArrivedFromMercSubSite variable to signify whcih page the player came from
enum {
  MERC_CAME_FROM_OTHER_PAGE,
  MERC_CAME_FROM_ACCOUNTS_PAGE,
  MERC_CAME_FROM_HIRE_PAGE,
};

void GameInitMercs();
BOOLEAN EnterMercs();
void ExitMercs();
void HandleMercs();
void RenderMercs();

BOOLEAN InitMercBackGround();
BOOLEAN DrawMecBackGround();
BOOLEAN RemoveMercBackGround();
void DailyUpdateOfMercSite(UINT16 usDate);
UINT8 GetMercIDFromMERCArray(UINT8 ubMercID);
void DisplayTextForSpeckVideoPopUp(STR16 pString);

BOOLEAN IsMercMercAvailable(UINT8 ubMercID);

void HandlePlayerHiringMerc(UINT8 ubHiredMercID);
void EnterInitMercSite();

void GetMercSiteBackOnline();

void DisableMercSiteButton();

extern UINT16 gusMercVideoSpeckSpeech;

extern UINT8 gubArrivedFromMercSubSite;

extern UINT8 gubMercArray[NUMBER_OF_MERCS];
extern UINT8 gubCurMercIndex;
// extern	UINT8			gubLastMercIndex;

// extern	UINT32		guiNumberOfMercPaymentsInDays;
// extern	UINT8			gubNumDaysTillFirstMercArrives;

extern BOOLEAN gfJustHiredAMercMerc;

void InitializeNumDaysMercArrive();

void NewMercsAvailableAtMercSiteCallBack();

void CalcAproximateAmountPaidToSpeck();

#endif
