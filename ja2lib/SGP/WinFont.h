#ifndef __WINFONT_
#define __WINFONT_

#include "SGP/Types.h"

void InitWinFonts();
void ShutdownWinFonts();

void SetWinFontForeColor(int32_t iFont, COLORVAL* pColor);

void PrintWinFont(uint32_t uiDestBuf, int32_t iFont, int32_t x, int32_t y, wchar_t* pFontString,
                  ...);

int16_t WinFontStringPixLength(wchar_t* string, int32_t iFont);
int16_t GetWinFontHeight(wchar_t* string, int32_t iFont);
uint32_t WinFont_mprintf(int32_t iFont, int32_t x, int32_t y, wchar_t* pFontString, ...);

#endif
