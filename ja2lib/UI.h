#ifndef __UI_H
#define __UI_H

#include "SGP/Types.h"

void MarkForRedrawalStrategicMap();

bool IsTacticalMode();

// Are we on the map screen?
bool IsMapScreen();

// Another check for Are we on the map screen?
// It is not clear how it is different from IsMapScreen().
bool IsMapScreen_2();

const SGPRect* GetMapCenteringRect();

// Get Merc associated with the context menu on tactical screen.
struct SOLDIERTYPE* GetTacticalContextMenuMerc();

void SwitchMapToMilitiaMode();

bool IsGoingToAutoresolve();

// Return index of the character selected for assignment
i8 GetCharForAssignmentIndex();

///////////////////////////////////////////////////////////////////
// message boxes
///////////////////////////////////////////////////////////////////

// Message box flags
#define MSG_BOX_FLAG_USE_CENTERING_RECT 0x0001     // Pass in a rect to center in
#define MSG_BOX_FLAG_OK 0x0002                     // Displays OK button
#define MSG_BOX_FLAG_YESNO 0x0004                  // Displays YES NO buttons
#define MSG_BOX_FLAG_CANCEL 0x0008                 // Displays YES NO buttons
#define MSG_BOX_FLAG_FOUR_NUMBERED_BUTTONS 0x0010  // Displays four numbered buttons, 1-4
#define MSG_BOX_FLAG_YESNOCONTRACT 0x0020          // yes no and contract buttons
#define MSG_BOX_FLAG_OKCONTRACT 0x0040             // ok and contract buttons
#define MSG_BOX_FLAG_YESNOLIE 0x0080               // ok and contract buttons
#define MSG_BOX_FLAG_CONTINUESTOP 0x0100           // continue stop box
#define MSG_BOX_FLAG_OKSKIP 0x0200                 // Displays ok or skip (meanwhile) buttons
#define MSG_BOX_FLAG_GENERICCONTRACT \
  0x0400                             // displays contract buttoin + 2 user-defined text buttons
#define MSG_BOX_FLAG_GENERIC 0x0800  // 2 user-defined text buttons

// message box return codes
#define MSG_BOX_RETURN_OK 1        // ENTER or on OK button
#define MSG_BOX_RETURN_YES 2       // ENTER or YES button
#define MSG_BOX_RETURN_NO 3        // ESC, Right Click or NO button
#define MSG_BOX_RETURN_CONTRACT 4  // contract button
#define MSG_BOX_RETURN_LIE 5       // LIE BUTTON

// message box style flags
enum {
  MSG_BOX_BASIC_STYLE = 0,  // We'll have other styles, like in laptop, etc
                            // Graphics are all that are required here...
  MSG_BOX_RED_ON_WHITE,
  MSG_BOX_BLUE_ON_GREY,
  MSG_BOX_BASIC_SMALL_BUTTONS,
  MSG_BOX_IMP_STYLE,
  MSG_BOX_LAPTOP_DEFAULT,
};

typedef void (*MSGBOX_CALLBACK)(uint8_t bExitValue);

int32_t DoMapMessageBox(uint8_t ubStyle, CHAR16* zString, uint32_t uiExitScreen, uint16_t usFlags,
                        MSGBOX_CALLBACK ReturnCallback);

void DoScreenIndependantMessageBox(CHAR16* zString, uint16_t usFlags,
                                   MSGBOX_CALLBACK ReturnCallback);
void DoLowerScreenIndependantMessageBox(CHAR16* zString, uint16_t usFlags,
                                        MSGBOX_CALLBACK ReturnCallback);
int32_t DoMessageBox(uint8_t ubStyle, CHAR16* zString, uint32_t uiExitScreen, uint16_t usFlags,
                     MSGBOX_CALLBACK ReturnCallback, const SGPRect* pCenteringRect);

///////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////

void StopTimeCompression(void);

///////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////

#endif
