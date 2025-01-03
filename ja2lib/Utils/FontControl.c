// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include "Utils/FontControl.h"

#include <stdarg.h>
#include <stdio.h>
#include <time.h>

#include "SGP/HImage.h"
#include "SGP/Types.h"
#include "SGP/VObject.h"
#include "SGP/VObjectInternal.h"
#include "SGP/VSurface.h"
#include "SGP/WCheck.h"

// Global variables for video objects
int32_t gpLargeFontType1;
struct VObject* gvoLargeFontType1;

int32_t gpSmallFontType1;
struct VObject* gvoSmallFontType1;

int32_t gpTinyFontType1;
struct VObject* gvoTinyFontType1;

int32_t gp12PointFont1;
struct VObject* gvo12PointFont1;

int32_t gpClockFont;
struct VObject* gvoClockFont;

int32_t gpCompFont;
struct VObject* gvoCompFont;

int32_t gpSmallCompFont;
struct VObject* gvoSmallCompFont;

int32_t gp10PointRoman;
struct VObject* gvo10PointRoman;

int32_t gp12PointRoman;
struct VObject* gvo12PointRoman;

int32_t gp14PointSansSerif;
struct VObject* gvo14PointSansSerif;

// int32_t						gpMilitaryFont1;
// struct VObject*				gvoMilitaryFont1;

int32_t gp10PointArial;
struct VObject* gvo10PointArial;

int32_t gp10PointArialBold;
struct VObject* gvo10PointArialBold;

int32_t gp14PointArial;
struct VObject* gvo14PointArial;

int32_t gp12PointArial;
struct VObject* gvo12PointArial;

int32_t gpBlockyFont;
struct VObject* gvoBlockyFont;

int32_t gpBlockyFont2;
struct VObject* gvoBlockyFont2;

int32_t gp12PointArialFixedFont;
struct VObject* gvo12PointArialFixedFont;

int32_t gp16PointArial;
struct VObject* gvo16PointArial;

int32_t gpBlockFontNarrow;
struct VObject* gvoBlockFontNarrow;

int32_t gp14PointHumanist;
struct VObject* gvo14PointHumanist;

#if defined(JA2EDITOR) && defined(ENGLISH)
int32_t gpHugeFont;
struct VObject* gvoHugeFont;
#endif

int32_t giSubTitleWinFont;

BOOLEAN gfFontsInit = FALSE;

uint16_t CreateFontPaletteTables(struct VObject* pObj);

extern wchar_t gzFontName[32];

BOOLEAN InitializeFonts() {
  // Initialize fonts
  //	gpLargeFontType1  = LoadFontFile( "FONTS\\lfont1.sti" );
  gpLargeFontType1 = LoadFontFile("FONTS\\LARGEFONT1.sti");
  gvoLargeFontType1 = GetFontObject(gpLargeFontType1);
  if (!(CreateFontPaletteTables(gvoLargeFontType1))) {
    return FALSE;
  }

  //	gpSmallFontType1  = LoadFontFile( "FONTS\\6b-font.sti" );
  gpSmallFontType1 = LoadFontFile("FONTS\\SMALLFONT1.sti");
  gvoSmallFontType1 = GetFontObject(gpSmallFontType1);
  if (!(CreateFontPaletteTables(gvoSmallFontType1))) {
    return FALSE;
  }

  //	gpTinyFontType1  = LoadFontFile( "FONTS\\tfont1.sti" );
  gpTinyFontType1 = LoadFontFile("FONTS\\TINYFONT1.sti");
  gvoTinyFontType1 = GetFontObject(gpTinyFontType1);
  if (!(CreateFontPaletteTables(gvoTinyFontType1))) {
    return FALSE;
  }

  //	gp12PointFont1	= LoadFontFile( "FONTS\\font-12.sti" );
  gp12PointFont1 = LoadFontFile("FONTS\\FONT12POINT1.sti");
  gvo12PointFont1 = GetFontObject(gp12PointFont1);
  if (!(CreateFontPaletteTables(gvo12PointFont1))) {
    return FALSE;
  }

  //  gpClockFont  = LoadFontFile( "FONTS\\DIGI.sti" );
  gpClockFont = LoadFontFile("FONTS\\CLOCKFONT.sti");
  gvoClockFont = GetFontObject(gpClockFont);
  if (!(CreateFontPaletteTables(gvoClockFont))) {
    return FALSE;
  }

  //  gpCompFont  = LoadFontFile( "FONTS\\compfont.sti" );
  gpCompFont = LoadFontFile("FONTS\\COMPFONT.sti");
  gvoCompFont = GetFontObject(gpCompFont);
  if (!(CreateFontPaletteTables(gvoCompFont))) {
    return FALSE;
  }

  //  gpSmallCompFont  = LoadFontFile( "FONTS\\scfont.sti" );
  gpSmallCompFont = LoadFontFile("FONTS\\SMALLCOMPFONT.sti");
  gvoSmallCompFont = GetFontObject(gpSmallCompFont);
  if (!(CreateFontPaletteTables(gvoSmallCompFont))) {
    return FALSE;
  }

  //  gp10PointRoman  = LoadFontFile( "FONTS\\Roman10.sti" );
  gp10PointRoman = LoadFontFile("FONTS\\FONT10ROMAN.sti");
  gvo10PointRoman = GetFontObject(gp10PointRoman);
  if (!(CreateFontPaletteTables(gvo10PointRoman))) {
    return FALSE;
  }

  //  gp12PointRoman  = LoadFontFile( "FONTS\\Roman12.sti" );
  gp12PointRoman = LoadFontFile("FONTS\\FONT12ROMAN.sti");
  gvo12PointRoman = GetFontObject(gp12PointRoman);
  if (!(CreateFontPaletteTables(gvo12PointRoman))) {
    return FALSE;
  }

  //  gp14PointSansSerif  = LoadFontFile( "FONTS\\SansSerif14.sti" );
  gp14PointSansSerif = LoadFontFile("FONTS\\FONT14SANSERIF.sti");
  gvo14PointSansSerif = GetFontObject(gp14PointSansSerif);
  if (!(CreateFontPaletteTables(gvo14PointSansSerif))) {
    return FALSE;
  }

  //	DEF:	Removed.  Replaced with BLOCKFONT
  //  gpMilitaryFont1  = LoadFontFile( "FONTS\\milfont.sti" );
  //  gvoMilitaryFont1 = GetFontObject( gpMilitaryFont1);
  //  if (!( CreateFontPaletteTables( gvoMilitaryFont1) )) { return FALSE; }

  //  gp10PointArial  = LoadFontFile( "FONTS\\Arial10.sti" );
  gp10PointArial = LoadFontFile("FONTS\\FONT10ARIAL.sti");
  gvo10PointArial = GetFontObject(gp10PointArial);
  if (!(CreateFontPaletteTables(gvo10PointArial))) {
    return FALSE;
  }

  //  gp14PointArial  = LoadFontFile( "FONTS\\Arial14.sti" );
  gp14PointArial = LoadFontFile("FONTS\\FONT14ARIAL.sti");
  gvo14PointArial = GetFontObject(gp14PointArial);
  if (!(CreateFontPaletteTables(gvo14PointArial))) {
    return FALSE;
  }

  //  gp10PointArialBold  = LoadFontFile( "FONTS\\Arial10Bold2.sti" );
  gp10PointArialBold = LoadFontFile("FONTS\\FONT10ARIALBOLD.sti");
  gvo10PointArialBold = GetFontObject(gp10PointArialBold);
  if (!(CreateFontPaletteTables(gvo10PointArialBold))) {
    return FALSE;
  }

  //  gp12PointArial  = LoadFontFile( "FONTS\\Arial12.sti" );
  gp12PointArial = LoadFontFile("FONTS\\FONT12ARIAL.sti");
  gvo12PointArial = GetFontObject(gp12PointArial);
  if (!(CreateFontPaletteTables(gvo12PointArial))) {
    return FALSE;
  }

  //	gpBlockyFont  = LoadFontFile( "FONTS\\FONT2.sti" );
  gpBlockyFont = LoadFontFile("FONTS\\BLOCKFONT.sti");
  gvoBlockyFont = GetFontObject(gpBlockyFont);
  if (!(CreateFontPaletteTables(gvoBlockyFont))) {
    return FALSE;
  }

  //	gpBlockyFont2  = LoadFontFile( "FONTS\\interface_font.sti" );
  gpBlockyFont2 = LoadFontFile("FONTS\\BLOCKFONT2.sti");
  gvoBlockyFont2 = GetFontObject(gpBlockyFont2);
  if (!(CreateFontPaletteTables(gvoBlockyFont2))) {
    return FALSE;
  }

  //	gp12PointArialFixedFont = LoadFontFile( "FONTS\\Arial12FixedWidth.sti" );
  gp12PointArialFixedFont = LoadFontFile("FONTS\\FONT12ARIALFIXEDWIDTH.sti");
  gvo12PointArialFixedFont = GetFontObject(gp12PointArialFixedFont);
  if (!(CreateFontPaletteTables(gvo12PointArialFixedFont))) {
    return FALSE;
  }

  gp16PointArial = LoadFontFile("FONTS\\FONT16ARIAL.sti");
  gvo16PointArial = GetFontObject(gp16PointArial);
  if (!(CreateFontPaletteTables(gvo16PointArial))) {
    return FALSE;
  }

  gpBlockFontNarrow = LoadFontFile("FONTS\\BLOCKFONTNARROW.sti");
  gvoBlockFontNarrow = GetFontObject(gpBlockFontNarrow);
  if (!(CreateFontPaletteTables(gvoBlockFontNarrow))) {
    return FALSE;
  }

  gp14PointHumanist = LoadFontFile("FONTS\\FONT14HUMANIST.sti");
  gvo14PointHumanist = GetFontObject(gp14PointHumanist);
  if (!(CreateFontPaletteTables(gvo14PointHumanist))) {
    return FALSE;
  }

#if defined(JA2EDITOR) && defined(ENGLISH)
  gpHugeFont = LoadFontFile("FONTS\\HUGEFONT.sti");
  gvoHugeFont = GetFontObject(gpHugeFont);
  if (!(CreateFontPaletteTables(gvoHugeFont))) {
    return FALSE;
  }
#endif

  // Set default for font system
  SetFontDest(vsFB, 0, 0, 640, 480, FALSE);

  gfFontsInit = TRUE;

  return (TRUE);
}

void ShutdownFonts() {
  UnloadFont(gpLargeFontType1);
  UnloadFont(gpSmallFontType1);
  UnloadFont(gpTinyFontType1);
  UnloadFont(gp12PointFont1);
  UnloadFont(gpClockFont);
  UnloadFont(gpCompFont);
  UnloadFont(gpSmallCompFont);
  UnloadFont(gp10PointRoman);
  UnloadFont(gp12PointRoman);
  UnloadFont(gp14PointSansSerif);
  //	UnloadFont( gpMilitaryFont1);
  UnloadFont(gp10PointArial);
  UnloadFont(gp10PointArialBold);
  UnloadFont(gp14PointArial);
  UnloadFont(gpBlockyFont);
  UnloadFont(gp12PointArialFixedFont);
#if defined(JA2EDITOR) && defined(ENGLISH)
  UnloadFont(gpHugeFont);
#endif
}

// Set shades for fonts
BOOLEAN SetFontShade(uint32_t uiFontID, int8_t bColorID) {
  struct VObject* pFont;

  if (!(bColorID > 0)) {
    return FALSE;
  }
  if (!(bColorID < 16)) {
    return FALSE;
  }

  pFont = GetFontObject(uiFontID);

  pFont->pShadeCurrent = pFont->pShades[bColorID];

  return (TRUE);
}

uint16_t CreateFontPaletteTables(struct VObject* pObj) {
  uint32_t count;

  for (count = 0; count < 16; count++) {
    if ((count == 4) && (pObj->p16BPPPalette == pObj->pShades[count]))
      pObj->pShades[count] = NULL;
    else if (pObj->pShades[count] != NULL) {
      MemFree(pObj->pShades[count]);
      pObj->pShades[count] = NULL;
    }
  }

  VObjectUpdateShade(pObj, FONT_SHADE_RED, 255, 0, 0, TRUE);
  VObjectUpdateShade(pObj, FONT_SHADE_BLUE, 0, 0, 255, TRUE);
  VObjectUpdateShade(pObj, FONT_SHADE_GREEN, 0, 255, 0, TRUE);
  VObjectUpdateShade(pObj, FONT_SHADE_YELLOW, 255, 255, 0, TRUE);
  VObjectUpdateShade(pObj, FONT_SHADE_NEUTRAL, 255, 255, 255, FALSE);

  VObjectUpdateShade(pObj, FONT_SHADE_WHITE, 255, 255, 255, TRUE);

  // the rest are darkening tables, right down to all-black.
  VObjectUpdateShade(pObj, 0, 165, 165, 165, FALSE);
  VObjectUpdateShade(pObj, 7, 135, 135, 135, FALSE);
  VObjectUpdateShade(pObj, 8, 105, 105, 105, FALSE);
  VObjectUpdateShade(pObj, 9, 75, 75, 75, FALSE);
  VObjectUpdateShade(pObj, 10, 45, 45, 45, FALSE);
  VObjectUpdateShade(pObj, 11, 36, 36, 36, FALSE);
  VObjectUpdateShade(pObj, 12, 27, 27, 27, FALSE);
  VObjectUpdateShade(pObj, 13, 18, 18, 18, FALSE);
  VObjectUpdateShade(pObj, 14, 9, 9, 9, FALSE);
  VObjectUpdateShade(pObj, 15, 0, 0, 0, FALSE);

  // Set current shade table to neutral color
  pObj->pShadeCurrent = pObj->pShades[4];

  return (TRUE);
}

uint16_t WFGetFontHeight(int32_t FontNum) { return (GetFontHeight(FontNum)); }

int16_t WFStringPixLength(wchar_t* string, int32_t UseFont) { return (StringPixLength(string, UseFont)); }
