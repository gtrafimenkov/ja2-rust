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

extern INT32 FontDefault;
extern uint32_t FontDestBuffer;
extern uint32_t FontDestPitch;
extern uint32_t FontDestBPP;
extern SGPRect FontDestRegion;
extern BOOLEAN FontDestWrap;

#define SetFontDestObject(x)                                                           \
  (SetFontDestBuffer(x, FontDestRegion.left, FontDestRegion.top, FontDestRegion.right, \
                     FontDestRegion.bottom, FontDestWrap))

#define SetFontDestClip(x1, y1, x2, y2) \
  (SetFontDestBuffer(FontDestBuffer, x1, y1, x2, y2, FontDestWrap))
#define SetFontDestWrap(x)                                                    \
  (SetFontDestBuffer(FontDestBuffer, FontDestRegion.left, FontDestRegion.top, \
                     FontDestRegion.right, FontDestRegion.bottom, x))
// functions

void SetFontColors(uint16_t usColors);
void SetFontForeground(uint8_t ubForeground);
void SetFontBackground(uint8_t ubBackground);
void SetFontShadow(uint8_t ubBackground);

// Kris:  added these
void SetRGBFontForeground(uint32_t uiRed, uint32_t uiGreen, uint32_t uiBlue);
void SetRGBFontBackground(uint32_t uiRed, uint32_t uiGreen, uint32_t uiBlue);
void SetRGBFontShadow(uint32_t uiRed, uint32_t uiGreen, uint32_t uiBlue);

BOOLEAN ResetFontObjectPalette(INT32 iFont);
uint16_t *SetFontObjectPalette8BPP(INT32 iFont, struct SGPPaletteEntry *pPal8);
uint16_t *SetFontObjectPalette16BPP(INT32 iFont, uint16_t *pPal16);
uint16_t *GetFontObjectPalette16BPP(INT32 iFont);

void DestroyEnglishTransTable(void);

extern struct VObject *GetFontObject(INT32 iFont);
extern uint32_t gprintf(INT32 x, INT32 y, STR16 pFontString, ...);
extern uint32_t gprintfDirty(INT32 x, INT32 y, STR16 pFontString, ...);
extern uint32_t mprintf(INT32 x, INT32 y, STR16 pFontString, ...);
extern uint32_t gprintf_buffer(uint8_t *pDestBuf, uint32_t uiDestPitchBYTES, uint32_t FontType,
                               INT32 x, INT32 y, STR16 pFontString, ...);
extern uint32_t mprintf_buffer(uint8_t *pDestBuf, uint32_t uiDestPitchBYTES, uint32_t FontType,
                               INT32 x, INT32 y, STR16 pFontString, ...);

// Function for displaying coded test. Since it's slower to do this, it's separate from  the normal
// fuctions
#define FONT_CODE_BEGINCOLOR 180
#define FONT_CODE_RESETCOLOR 181

uint32_t mprintf_buffer_coded(uint8_t *pDestBuf, uint32_t uiDestPitchBYTES, uint32_t FontType,
                              INT32 x, INT32 y, STR16 pFontString, ...);
uint32_t mprintf_coded(INT32 x, INT32 y, STR16 pFontString, ...);

extern BOOLEAN SetFontDestBuffer(uint32_t DestBuffer, INT32 x1, INT32 y1, INT32 x2, INT32 y2,
                                 BOOLEAN wrap);
extern BOOLEAN SetFont(INT32 iFontIndex);

extern INT32 LoadFontFile(STR8 pFileName);
extern uint16_t GetFontHeight(INT32 FontNum);
extern BOOLEAN InitializeFontManager(uint16_t usDefaultPixDepth, FontTranslationTable *pTransTable);
extern void ShutdownFontManager(void);
extern void UnloadFont(uint32_t FontIndex);

extern FontTranslationTable *CreateEnglishTransTable();

extern int16_t GetIndex(uint16_t siChar);
extern uint32_t GetWidth(struct VObject *hSrcVObject, int16_t ssIndex);

extern int16_t StringPixLengthArgFastHelp(INT32 usUseFont, INT32 usBoldFont, uint32_t uiCharCount,
                                          CHAR16 *pFontString);
extern int16_t StringPixLengthArg(INT32 usUseFont, uint32_t uiCharCount, CHAR16 *pFontString, ...);
extern int16_t StringPixLength(CHAR16 *string, INT32 UseFont);
extern int16_t StringNPixLength(CHAR16 *string, uint32_t uiMaxCount, INT32 UseFont);
extern void SaveFontSettings(void);
extern void RestoreFontSettings(void);

void VarFindFontRightCoordinates(int16_t sLeft, int16_t sTop, int16_t sWidth, int16_t sHeight,
                                 INT32 iFontIndex, int16_t *psNewX, int16_t *psNewY,
                                 CHAR16 *pFontString, ...);
void VarFindFontCenterCoordinates(int16_t sLeft, int16_t sTop, int16_t sWidth, int16_t sHeight,
                                  INT32 iFontIndex, int16_t *psNewX, int16_t *psNewY,
                                  CHAR16 *pFontString, ...);
void FindFontRightCoordinates(int16_t sLeft, int16_t sTop, int16_t sWidth, int16_t sHeight,
                              CHAR16 *pStr, INT32 iFontIndex, int16_t *psNewX, int16_t *psNewY);
void FindFontCenterCoordinates(int16_t sLeft, int16_t sTop, int16_t sWidth, int16_t sHeight,
                               CHAR16 *pStr, INT32 iFontIndex, int16_t *psNewX, int16_t *psNewY);

#endif
