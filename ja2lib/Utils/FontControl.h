#ifndef __FONT_CONTROL_H
#define __FONT_CONTROL_H

#include "BuildDefines.h"
#include "SGP/Font.h"
#include "SGP/Types.h"

struct VObject;

// ATE: A few winfont wrappers..
uint16_t WFGetFontHeight(int32_t FontNum);
int16_t WFStringPixLength(wchar_t* string, int32_t UseFont);

// Global variables for video objects
extern int32_t gpLargeFontType1;
extern struct VObject* gvoLargeFontType1;

extern int32_t gpSmallFontType1;
extern struct VObject* gvoSmallFontType1;

extern int32_t gpTinyFontType1;
extern struct VObject* gvoTinyFontType1;

extern int32_t gp12PointFont1;
extern struct VObject* gvo12PointFont1;

extern int32_t gpClockFont;
extern struct VObject* gvoClockFont;

extern int32_t gpCompFont;
extern struct VObject* gvoCompFont;

extern int32_t gpSmallCompFont;
extern struct VObject* gvoSmallCompFont;

extern int32_t gp10PointRoman;
extern struct VObject* gvo10PointRoman;

extern int32_t gp12PointRoman;
extern struct VObject* gvo12PointRoman;

extern int32_t gp14PointSansSerif;
extern struct VObject* gvo14PointSansSerif;

// extern int32_t						gpMilitaryFont1;
// extern struct VObject*				gvoMilitaryFont1;

extern int32_t gp10PointArial;
extern struct VObject* gvo10PointArial;

extern int32_t gp14PointArial;
extern struct VObject* gvo14PointArial;

extern int32_t gp12PointArial;
extern struct VObject* gvo12PointArial;

extern int32_t gpBlockyFont;
extern struct VObject* gvoBlockyFont;

extern int32_t gpBlockyFont2;
extern struct VObject* gvoBlockyFont2;

extern int32_t gp10PointArialBold;
extern struct VObject* gvo10PointArialBold;

extern int32_t gp12PointArialFixedFont;
extern struct VObject* gvo12PointArialFixedFont;

extern int32_t gp16PointArial;
extern struct VObject* gvo16PointArial;

extern int32_t gpBlockFontNarrow;
extern struct VObject* gvoBlockFontNarrow;

extern int32_t gp14PointHumanist;
extern struct VObject* gvo14PointHumanist;

#ifdef JA2EDITOR
extern int32_t gpHugeFont;
extern struct VObject* gvoHugeFont;
#endif

extern int32_t giSubTitleWinFont;

extern BOOLEAN gfFontsInit;

// Defines
#define LARGEFONT1 gpLargeFontType1
#define SMALLFONT1 gpSmallFontType1
#define TINYFONT1 gpTinyFontType1
#define FONT12POINT1 gp12PointFont1
#define CLOCKFONT gpClockFont
#define COMPFONT gpCompFont
#define SMALLCOMPFONT gpSmallCompFont
#define FONT10ROMAN gp10PointRoman
#define FONT12ROMAN gp12PointRoman
#define FONT14SANSERIF gp14PointSansSerif
#define MILITARYFONT1 BLOCKFONT  // gpMilitaryFont1
#define FONT10ARIAL gp10PointArial
#define FONT14ARIAL gp14PointArial
#define FONT12ARIAL gp12PointArial
#define FONT10ARIALBOLD gp10PointArialBold
#define BLOCKFONT gpBlockyFont
#define BLOCKFONT2 gpBlockyFont2
#define FONT12ARIALFIXEDWIDTH gp12PointArialFixedFont
#define FONT16ARIAL gp16PointArial
#define BLOCKFONTNARROW gpBlockFontNarrow
#define FONT14HUMANIST gp14PointHumanist

#if defined(JA2EDITOR) && defined(ENGLISH)
#define HUGEFONT gpHugeFont
#else
#define HUGEFONT gp16PointArial
#endif

#define FONT_SHADE_RED 6
#define FONT_SHADE_BLUE 1
#define FONT_SHADE_GREEN 2
#define FONT_SHADE_YELLOW 3
#define FONT_SHADE_NEUTRAL 4
#define FONT_SHADE_WHITE 5

#define FONT_MCOLOR_BLACK 0
#define FONT_MCOLOR_WHITE 208
#define FONT_MCOLOR_DKWHITE 134
#define FONT_MCOLOR_DKWHITE2 134
#define FONT_MCOLOR_LTGRAY 134
#define FONT_MCOLOR_LTGRAY2 134
#define FONT_MCOLOR_DKGRAY 136
#define FONT_MCOLOR_LTBLUE 203
#define FONT_MCOLOR_LTRED 162
#define FONT_MCOLOR_RED 163
#define FONT_MCOLOR_DKRED 164
#define FONT_MCOLOR_LTGREEN 184
#define FONT_MCOLOR_LTYELLOW 144

// Grayscale font colors
#define FONT_WHITE 208  // lightest color
#define FONT_GRAY1 133
#define FONT_GRAY2 134  // light gray
#define FONT_GRAY3 135
#define FONT_GRAY4 136  // gray
#define FONT_GRAY5 137
#define FONT_GRAY6 138
#define FONT_GRAY7 139  // dark gray
#define FONT_GRAY8 140
#define FONT_NEARBLACK 141
#define FONT_BLACK 0  // darkest color
// Color font colors
#define FONT_LTRED 162
#define FONT_RED 163
#define FONT_DKRED 218
#define FONT_ORANGE 76
#define FONT_YELLOW 145
#define FONT_DKYELLOW 80
#define FONT_LTGREEN 184
#define FONT_GREEN 185
#define FONT_DKGREEN 186
#define FONT_LTBLUE 71
#define FONT_BLUE 203
#define FONT_DKBLUE 205

#define FONT_BEIGE 130
#define FONT_METALGRAY 94
#define FONT_BURGUNDY 172
#define FONT_LTKHAKI 88
#define FONT_KHAKI 198
#define FONT_DKKHAKI 201

BOOLEAN InitializeFonts();
void ShutdownFonts();

BOOLEAN SetFontShade(uint32_t uiFontID, int8_t bColorID);

#endif
