#ifndef __WINFONT_
#define __WINFONT_

#include "SGP/Types.h"
#include "SGP/VObject.h"

void InitWinFonts();
void ShutdownWinFonts();

void SetWinFontForeColor(INT32 iFont, COLORVAL *pColor);

void PrintWinFont(UINT32 uiDestBuf, INT32 iFont, INT32 x, INT32 y, STR16 pFontString, ...);

INT16 WinFontStringPixLength(STR16 string, INT32 iFont);
INT16 GetWinFontHeight(STR16 string, INT32 iFont);
UINT32 WinFont_mprintf(INT32 iFont, INT32 x, INT32 y, STR16 pFontString, ...);

#endif
