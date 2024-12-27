// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#ifndef __FONT_H_
#define __FONT_H_

#include "SGP/HImage.h"
#include "SGP/Types.h"

struct VObject;

#define DEFAULT_SHADOW 2
#define MILITARY_SHADOW 67
#define NO_SHADOW 0

// these are bogus! No palette is set yet!
// font foreground color symbols
#define FONT_FCOLOR_WHITE 208
#define FONT_FCOLOR_RED 162
#define FONT_FCOLOR_NICERED 164
#define FONT_FCOLOR_BLUE 203
#define FONT_FCOLOR_GREEN 184
#define FONT_FCOLOR_YELLOW 144
#define FONT_FCOLOR_BROWN 184
#define FONT_FCOLOR_ORANGE 76
#define FONT_FCOLOR_PURPLE 160

// font background color symbols
#define FONT_BCOLOR_WHITE 208
#define FONT_BCOLOR_RED 162
#define FONT_BCOLOR_BLUE 203
#define FONT_BCOLOR_GREEN 184
#define FONT_BCOLOR_YELLOW 144
#define FONT_BCOLOR_BROWN 80
#define FONT_BCOLOR_ORANGE 76
#define FONT_BCOLOR_PURPLE 160

// typedefs

typedef struct {
  uint16_t usNumberOfSymbols;
  uint16_t *DynamicArrayOf16BitValues;

} FontTranslationTable;

extern int32_t FontDefault;
extern struct VSurface *FontDestSurface;
extern uint32_t FontDestPitch;
extern uint32_t FontDestBPP;
extern struct GRect FontDestRegion;
extern BOOLEAN FontDestWrap;

#define SetFontDestClip(x1, y1, x2, y2) (SetFontDest(FontDestSurface, x1, y1, x2, y2, FontDestWrap))
#define SetFontDestWrap(x)                                                                     \
  (SetFontDest(FontDestSurface, FontDestRegion.left, FontDestRegion.top, FontDestRegion.right, \
               FontDestRegion.bottom, x))

void SetFontColors(uint16_t usColors);
void SetFontForeground(uint8_t ubForeground);
void SetFontBackground(uint8_t ubBackground);
void SetFontShadow(uint8_t ubBackground);

// Kris:  added these
void SetRGBFontForeground(uint32_t uiRed, uint32_t uiGreen, uint32_t uiBlue);
void SetRGBFontBackground(uint32_t uiRed, uint32_t uiGreen, uint32_t uiBlue);
void SetRGBFontShadow(uint32_t uiRed, uint32_t uiGreen, uint32_t uiBlue);

BOOLEAN ResetFontObjectPalette(int32_t iFont);
uint16_t *SetFontObjectPalette8BPP(int32_t iFont, struct SGPPaletteEntry *pPal8);
uint16_t *SetFontObjectPalette16BPP(int32_t iFont, uint16_t *pPal16);
uint16_t *GetFontObjectPalette16BPP(int32_t iFont);

void DestroyEnglishTransTable(void);

extern struct VObject *GetFontObject(int32_t iFont);
extern uint32_t gprintf(int32_t x, int32_t y, wchar_t* pFontString, ...);
extern uint32_t gprintfDirty(int32_t x, int32_t y, wchar_t* pFontString, ...);
extern uint32_t mprintf(int32_t x, int32_t y, wchar_t* pFontString, ...);
extern uint32_t gprintf_buffer(uint8_t *pDestBuf, uint32_t uiDestPitchBYTES, uint32_t FontType, int32_t x,
                             int32_t y, wchar_t* pFontString, ...);
extern uint32_t mprintf_buffer(uint8_t *pDestBuf, uint32_t uiDestPitchBYTES, uint32_t FontType, int32_t x,
                             int32_t y, wchar_t* pFontString, ...);

// Function for displaying coded test. Since it's slower to do this, it's separate from  the normal
// fuctions
#define FONT_CODE_BEGINCOLOR 180
#define FONT_CODE_RESETCOLOR 181

uint32_t mprintf_buffer_coded(uint8_t *pDestBuf, uint32_t uiDestPitchBYTES, uint32_t FontType, int32_t x,
                            int32_t y, wchar_t* pFontString, ...);
uint32_t mprintf_coded(int32_t x, int32_t y, wchar_t* pFontString, ...);

BOOLEAN SetFontDest(struct VSurface *dest, i32 x1, i32 y1, i32 x2, i32 y2, BOOLEAN wrap);
extern BOOLEAN SetFont(int32_t iFontIndex);

extern int32_t LoadFontFile(char* pFileName);
extern uint16_t GetFontHeight(int32_t FontNum);
extern BOOLEAN InitializeFontManager(uint16_t usDefaultPixDepth, FontTranslationTable *pTransTable);
extern void ShutdownFontManager(void);
extern void UnloadFont(uint32_t FontIndex);

extern FontTranslationTable *CreateEnglishTransTable();

extern int16_t GetIndex(uint16_t siChar);
extern uint32_t GetWidth(struct VObject *hSrcVObject, int16_t ssIndex);

extern int16_t StringPixLengthArgFastHelp(int32_t usUseFont, int32_t usBoldFont, uint32_t uiCharCount,
                                        wchar_t *pFontString);
extern int16_t StringPixLengthArg(int32_t usUseFont, uint32_t uiCharCount, wchar_t *pFontString, ...);
extern int16_t StringPixLength(wchar_t *string, int32_t UseFont);
extern int16_t StringNPixLength(wchar_t *string, uint32_t uiMaxCount, int32_t UseFont);
extern void SaveFontSettings(void);
extern void RestoreFontSettings(void);

void VarFindFontRightCoordinates(int16_t sLeft, int16_t sTop, int16_t sWidth, int16_t sHeight,
                                 int32_t iFontIndex, int16_t *psNewX, int16_t *psNewY,
                                 wchar_t *pFontString, ...);
void VarFindFontCenterCoordinates(int16_t sLeft, int16_t sTop, int16_t sWidth, int16_t sHeight,
                                  int32_t iFontIndex, int16_t *psNewX, int16_t *psNewY,
                                  wchar_t *pFontString, ...);
void FindFontRightCoordinates(int16_t sLeft, int16_t sTop, int16_t sWidth, int16_t sHeight, wchar_t *pStr,
                              int32_t iFontIndex, int16_t *psNewX, int16_t *psNewY);
void FindFontCenterCoordinates(int16_t sLeft, int16_t sTop, int16_t sWidth, int16_t sHeight, wchar_t *pStr,
                               int32_t iFontIndex, int16_t *psNewX, int16_t *psNewY);

#endif
