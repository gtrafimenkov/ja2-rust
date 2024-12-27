#ifndef __WORDWRAP_H_
#define __WORDWRAP_H_

#include "Laptop/Email.h"
#include "Laptop/Files.h"
#include "SGP/Types.h"

// Flags for DrawTextToScreen()

// Defines for coded text For use with IanDisplayWrappedString()
#define TEXT_SPACE 32
#define TEXT_CODE_NEWLINE 177
#define TEXT_CODE_BOLD 178
#define TEXT_CODE_CENTER 179
#define TEXT_CODE_NEWCOLOR 180
#define TEXT_CODE_DEFCOLOR 181

uint16_t IanDisplayWrappedString(uint16_t usPosX, uint16_t usPosY, uint16_t usWidth, UINT8 ubGap,
                                 uint32_t uiFont, UINT8 ubColor, STR16 pString,
                                 UINT8 ubBackGroundColor, BOOLEAN fDirty, uint32_t uiFlags);

#define LEFT_JUSTIFIED 0x00000001
#define CENTER_JUSTIFIED 0x00000002
#define RIGHT_JUSTIFIED 0x00000004
#define TEXT_SHADOWED 0x00000008

#define INVALIDATE_TEXT 0x00000010
#define DONT_DISPLAY_TEXT \
  0x00000020  // Wont display the text.  Used if you just want to get how many lines will be
              // displayed

#define IAN_WRAP_NO_SHADOW 32

#define NEWLINE_CHAR 177

typedef struct _WRAPPEDSTRING {
  STR16 sString;
  struct _WRAPPEDSTRING *pNextWrappedString;
} WRAPPED_STRING;

WRAPPED_STRING *LineWrap(uint32_t ulFont, uint16_t usLineWidthPixels,
                         uint16_t *pusLineWidthIfWordIsWiderThenWidth, STR16 pString, ...);
uint16_t DisplayWrappedString(uint16_t usPosX, uint16_t usPosY, uint16_t usWidth, UINT8 ubGap,
                              uint32_t uiFont, UINT8 ubColor, STR16 pString,
                              UINT8 ubBackGroundColor, BOOLEAN fDirty, uint32_t ulFlags);
uint16_t DeleteWrappedString(WRAPPED_STRING *pWrappedString);
void CleanOutControlCodesFromString(STR16 pSourceString, STR16 pDestString);
INT16 IanDisplayWrappedStringToPages(uint16_t usPosX, uint16_t usPosY, uint16_t usWidth,
                                     uint16_t usPageHeight, uint16_t usTotalHeight,
                                     uint16_t usPageNumber, UINT8 ubGap, uint32_t uiFont,
                                     UINT8 ubColor, STR16 pString, UINT8 ubBackGroundColor,
                                     BOOLEAN fDirty, uint32_t uiFlags, BOOLEAN *fOnLastPageFlag);
BOOLEAN DrawTextToScreen(STR16 pStr, uint16_t LocX, uint16_t LocY, uint16_t usWidth,
                         uint32_t ulFont, UINT8 ubColor, UINT8 ubBackGroundColor, BOOLEAN fDirty,
                         uint32_t FLAGS);
uint16_t IanWrappedStringHeight(uint16_t usPosX, uint16_t usPosY, uint16_t usWidth, UINT8 ubGap,
                                uint32_t uiFont, UINT8 ubColor, STR16 pString,
                                UINT8 ubBackGroundColor, BOOLEAN fDirty, uint32_t uiFlags);

BOOLEAN WillThisStringGetCutOff(INT32 iCurrentYPosition, INT32 iBottomOfPage, INT32 iWrapWidth,
                                uint32_t uiFont, STR16 pString, INT32 iGap, INT32 iPage);
BOOLEAN IsThisStringBeforeTheCurrentPage(INT32 iTotalYPosition, INT32 iPageSize, INT32 iCurrentPage,
                                         INT32 iWrapWidth, uint32_t uiFont, STR16 pString,
                                         INT32 iGap);
INT32 GetNewTotalYPositionOfThisString(INT32 iTotalYPosition, INT32 iPageSize, INT32 iCurrentPage,
                                       INT32 iWrapWidth, uint32_t uiFont, STR16 pString,
                                       INT32 iGap);
RecordPtr GetFirstRecordOnThisPage(RecordPtr RecordList, uint32_t uiFont, uint16_t usWidth,
                                   UINT8 ubGap, INT32 iPage, INT32 iPageSize);
FileStringPtr GetFirstStringOnThisPage(FileStringPtr RecordList, uint32_t uiFont, uint16_t usWidth,
                                       UINT8 ubGap, INT32 iPage, INT32 iPageSize,
                                       FileRecordWidthPtr iWidthArray);

// Places a shadow the width an height of the string, to PosX, posY
void ShadowText(uint32_t uiDestVSurface, STR16 pString, uint32_t uiFont, uint16_t usPosX,
                uint16_t usPosY);

BOOLEAN ReduceStringLength(STR16 pString, size_t bufSize, uint32_t uiWidth, uint32_t uiFont);

void UseSingleCharWordsForWordWrap(BOOLEAN fUseSingleCharWords);
WRAPPED_STRING *LineWrapForSingleCharWords(uint32_t ulFont, uint16_t usLineWidthPixels,
                                           uint16_t *pusLineWidthIfWordIsWiderThenWidth,
                                           STR16 pString, ...);

#endif
