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

uint16_t IanDisplayWrappedString(uint16_t usPosX, uint16_t usPosY, uint16_t usWidth, uint8_t ubGap,
                                 uint32_t uiFont, uint8_t ubColor, wchar_t* pString,
                                 uint8_t ubBackGroundColor, BOOLEAN fDirty, uint32_t uiFlags);

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
  wchar_t* sString;
  struct _WRAPPEDSTRING* pNextWrappedString;
} WRAPPED_STRING;

WRAPPED_STRING* LineWrap(uint32_t ulFont, uint16_t usLineWidthPixels,
                         uint16_t* pusLineWidthIfWordIsWiderThenWidth, wchar_t* pString, ...);
uint16_t DisplayWrappedString(uint16_t usPosX, uint16_t usPosY, uint16_t usWidth, uint8_t ubGap,
                              uint32_t uiFont, uint8_t ubColor, wchar_t* pString,
                              uint8_t ubBackGroundColor, BOOLEAN fDirty, uint32_t ulFlags);
uint16_t DeleteWrappedString(WRAPPED_STRING* pWrappedString);
void CleanOutControlCodesFromString(wchar_t* pSourceString, wchar_t* pDestString);
int16_t IanDisplayWrappedStringToPages(uint16_t usPosX, uint16_t usPosY, uint16_t usWidth,
                                       uint16_t usPageHeight, uint16_t usTotalHeight,
                                       uint16_t usPageNumber, uint8_t ubGap, uint32_t uiFont,
                                       uint8_t ubColor, wchar_t* pString, uint8_t ubBackGroundColor,
                                       BOOLEAN fDirty, uint32_t uiFlags, BOOLEAN* fOnLastPageFlag);
BOOLEAN DrawTextToScreen(wchar_t* pStr, uint16_t LocX, uint16_t LocY, uint16_t usWidth,
                         uint32_t ulFont, uint8_t ubColor, uint8_t ubBackGroundColor,
                         BOOLEAN fDirty, uint32_t FLAGS);
uint16_t IanWrappedStringHeight(uint16_t usPosX, uint16_t usPosY, uint16_t usWidth, uint8_t ubGap,
                                uint32_t uiFont, uint8_t ubColor, wchar_t* pString,
                                uint8_t ubBackGroundColor, BOOLEAN fDirty, uint32_t uiFlags);

BOOLEAN WillThisStringGetCutOff(int32_t iCurrentYPosition, int32_t iBottomOfPage,
                                int32_t iWrapWidth, uint32_t uiFont, wchar_t* pString, int32_t iGap,
                                int32_t iPage);
BOOLEAN IsThisStringBeforeTheCurrentPage(int32_t iTotalYPosition, int32_t iPageSize,
                                         int32_t iCurrentPage, int32_t iWrapWidth, uint32_t uiFont,
                                         wchar_t* pString, int32_t iGap);
int32_t GetNewTotalYPositionOfThisString(int32_t iTotalYPosition, int32_t iPageSize,
                                         int32_t iCurrentPage, int32_t iWrapWidth, uint32_t uiFont,
                                         wchar_t* pString, int32_t iGap);
RecordPtr GetFirstRecordOnThisPage(RecordPtr RecordList, uint32_t uiFont, uint16_t usWidth,
                                   uint8_t ubGap, int32_t iPage, int32_t iPageSize);
FileStringPtr GetFirstStringOnThisPage(FileStringPtr RecordList, uint32_t uiFont, uint16_t usWidth,
                                       uint8_t ubGap, int32_t iPage, int32_t iPageSize,
                                       FileRecordWidthPtr iWidthArray);

// Places a shadow the width an height of the string, to PosX, posY
void ShadowText(uint32_t uiDestVSurface, wchar_t* pString, uint32_t uiFont, uint16_t usPosX,
                uint16_t usPosY);

BOOLEAN ReduceStringLength(wchar_t* pString, size_t bufSize, uint32_t uiWidth, uint32_t uiFont);

void UseSingleCharWordsForWordWrap(BOOLEAN fUseSingleCharWords);
WRAPPED_STRING* LineWrapForSingleCharWords(uint32_t ulFont, uint16_t usLineWidthPixels,
                                           uint16_t* pusLineWidthIfWordIsWiderThenWidth,
                                           wchar_t* pString, ...);

#endif
