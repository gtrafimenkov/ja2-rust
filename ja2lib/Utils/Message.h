// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#ifndef __MESSAGE_H
#define __MESSAGE_H

#include "SGP/Font.h"
#include "SGP/Types.h"
#include "Utils/FontControl.h"

struct stringstruct {
  wchar_t* pString16;
  int32_t iVideoOverlay;
  uint32_t uiFont;
  uint16_t usColor;
  uint32_t uiFlags;
  BOOLEAN fBeginningOfNewString;
  uint32_t uiTimeOfLastUpdate;
  uint32_t uiPadding[5];
  struct stringstruct* pNext;
  struct stringstruct* pPrev;
};

#define MSG_INTERFACE 0
#define MSG_DIALOG 1
#define MSG_CHAT 2
#define MSG_DEBUG 3
#define MSG_UI_FEEDBACK 4
#define MSG_ERROR 5
#define MSG_BETAVERSION 6
#define MSG_TESTVERSION 7
#define MSG_MAP_UI_POSITION_MIDDLE 8
#define MSG_MAP_UI_POSITION_UPPER 9
#define MSG_MAP_UI_POSITION_LOWER 10
#define MSG_SKULL_UI_FEEDBACK 11

// These defines correlate to defines in font.h
#define MSG_FONT_RED FONT_MCOLOR_RED
#define MSG_FONT_YELLOW FONT_MCOLOR_LTYELLOW
#define MSG_FONT_WHITE FONT_MCOLOR_WHITE

typedef struct stringstruct ScrollStringSt;
typedef ScrollStringSt* ScrollStringStPtr;

extern ScrollStringStPtr pStringS;
extern uint32_t StringCount;
extern uint8_t gubCurrentMapMessageString;
extern BOOLEAN fDisableJustForIan;

// are we allowed to beep on message scroll in tactical
extern BOOLEAN fOkToBeepNewMessage;

void ScreenMsg(uint16_t usColor, uint8_t ubPriority, wchar_t* pStringA, ...);

// same as screen message, but only display to mapscreen message system, not tactical
void MapScreenMessage(uint16_t usColor, uint8_t ubPriority, wchar_t* pStringA, ...);

void ScrollString(void);
void DisplayStringsInMapScreenMessageList(void);

void InitGlobalMessageList(void);
void FreeGlobalMessageList(void);

uint8_t GetRangeOfMapScreenMessages(void);

void EnableDisableScrollStringVideoOverlay(BOOLEAN fEnable);

// will go and clear all displayed strings off the screen
void ClearDisplayedListOfTacticalStrings(void);

// clear ALL strings in the tactical Message Queue
void ClearTacticalMessageQueue(void);

BOOLEAN LoadMapScreenMessagesFromSaveGameFile(FileID hFile);
BOOLEAN SaveMapScreenMessagesToSaveGameFile(FileID hFile);

// use these if you are not Kris
void HideMessagesDuringNPCDialogue(void);
void UnHideMessagesDuringNPCDialogue(void);

// disable and enable scroll string, only to be used by Kris
void DisableScrollMessages(void);
void EnableScrollMessages(void);

/* unused functions, written by Mr. Carter, so don't expect these to work...
uint8_t GetTheRelativePositionOfCurrentMessage( void );
void MoveCurrentMessagePointerDownList( void );
void MoveCurrentMessagePointerUpList( void );
void ScrollToHereInMapScreenMessageList( uint8_t ubPosition );
BOOLEAN IsThereAnEmptySlotInTheMapScreenMessageList( void );
uint8_t GetFirstEmptySlotInTheMapScreenMessageList( void );
void RemoveMapScreenMessageListString( ScrollStringStPtr pStringSt );
BOOLEAN AreThereASetOfStringsAfterThisIndex( uint8_t ubMsgIndex, int32_t iNumberOfStrings );
uint8_t GetCurrentMessageValue( void );
uint8_t GetCurrentTempMessageValue( void );
uint8_t GetNewMessageValueGivenPosition( uint8_t ubPosition );
BOOLEAN IsThisTheLastMessageInTheList( void );
BOOLEAN IsThisTheFirstMessageInTheList( void );
void DisplayLastMessage( void );
*/

#endif
