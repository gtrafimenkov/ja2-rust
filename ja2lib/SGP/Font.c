#include "SGP/Font.h"

#include <malloc.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>

#include "Globals.h"
#include "SGP/Debug.h"
#include "SGP/HImage.h"
#include "SGP/MemMan.h"
#include "SGP/PCX.h"
#include "SGP/Types.h"
#include "SGP/VObject.h"
#include "SGP/VObjectBlitters.h"
#include "SGP/VObjectInternal.h"
#include "SGP/VSurface.h"
#include "SGP/Video.h"
#include "rust_fileman.h"
#include "rust_images.h"

//*******************************************************
//
//   Defines
//
//*******************************************************

#define PALETTE_SIZE 768
#define STRING_DELIMITER 0
#define ID_BLACK 0
#define MAX_FONTS 25

//*******************************************************
//
//   Typedefs
//
//*******************************************************

typedef struct {
  UINT16 usDefaultPixelDepth;
  FontTranslationTable *pTranslationTable;
} FontManager;

FontManager *pFManager;
struct VObject *FontObjs[MAX_FONTS];
INT32 FontsLoaded = 0;

// Destination printing parameters
INT32 FontDefault = (-1);
struct VSurface *FontDestSurface = NULL;
UINT32 FontDestPitch = 640 * 2;
UINT32 FontDestBPP = 16;
SGPRect FontDestRegion = {0, 0, 640, 480};
BOOLEAN FontDestWrap = FALSE;
UINT16 FontForeground16 = 0;
UINT16 FontBackground16 = 0;
UINT16 FontShadow16 = DEFAULT_SHADOW;
UINT8 FontForeground8 = 0;
UINT8 FontBackground8 = 0;

// Temp, for saving printing parameters
INT32 SaveFontDefault = (-1);
struct VSurface *SaveFontDestSurface = NULL;
UINT32 SaveFontDestPitch = 640 * 2;
UINT32 SaveFontDestBPP = 16;
SGPRect SaveFontDestRegion = {0, 0, 640, 480};
BOOLEAN SaveFontDestWrap = FALSE;
UINT16 SaveFontForeground16 = 0;
UINT16 SaveFontShadow16 = 0;
UINT16 SaveFontBackground16 = 0;
UINT8 SaveFontForeground8 = 0;
UINT8 SaveFontBackground8 = 0;

//*****************************************************************************
// SetFontColors
//
//	Sets both the foreground and the background colors of the current font. The
// top byte of the parameter word is the background color, and the bottom byte
// is the foreground.
//
//*****************************************************************************
void SetFontColors(UINT16 usColors) {
  UINT8 ubForeground, ubBackground;

  ubForeground = (UINT8)(usColors & 0xff);
  ubBackground = (UINT8)((usColors & 0xff00) >> 8);

  SetFontForeground(ubForeground);
  SetFontBackground(ubBackground);
}

//*****************************************************************************
// SetFontForeground
//
//	Sets the foreground color of the currently selected font. The parameter is
// the index into the 8-bit palette. In 8BPP mode, that index number is used
// for the pixel value to be drawn for nontransparent pixels. In 16BPP mode,
// the RGB values from the palette are used to create the pixel color. Note
// that if you change fonts, the selected foreground/background colors will
// stay at what they are currently set to.
//
//*****************************************************************************
void SetFontForeground(UINT8 ubForeground) {
  UINT32 uiRed, uiGreen, uiBlue;

  if ((FontDefault < 0) || (FontDefault > MAX_FONTS)) return;

  FontForeground8 = ubForeground;

  uiRed = (UINT32)FontObjs[FontDefault]->pPaletteEntry[ubForeground].peRed;
  uiGreen = (UINT32)FontObjs[FontDefault]->pPaletteEntry[ubForeground].peGreen;
  uiBlue = (UINT32)FontObjs[FontDefault]->pPaletteEntry[ubForeground].peBlue;

  FontForeground16 = Get16BPPColor(FROMRGB(uiRed, uiGreen, uiBlue));
}

void SetFontShadow(UINT8 ubShadow) {
  UINT32 uiRed, uiGreen, uiBlue;

  if ((FontDefault < 0) || (FontDefault > MAX_FONTS)) return;

  // FontForeground8=ubForeground;

  uiRed = (UINT32)FontObjs[FontDefault]->pPaletteEntry[ubShadow].peRed;
  uiGreen = (UINT32)FontObjs[FontDefault]->pPaletteEntry[ubShadow].peGreen;
  uiBlue = (UINT32)FontObjs[FontDefault]->pPaletteEntry[ubShadow].peBlue;

  FontShadow16 = Get16BPPColor(FROMRGB(uiRed, uiGreen, uiBlue));

  if (ubShadow != 0) {
    if (FontShadow16 == 0) {
      FontShadow16 = 1;
    }
  }
}

//*****************************************************************************
// SetFontBackground
//
//	Sets the Background color of the currently selected font. The parameter is
// the index into the 8-bit palette. In 8BPP mode, that index number is used
// for the pixel value to be drawn for nontransparent pixels. In 16BPP mode,
// the RGB values from the palette are used to create the pixel color. If the
// background value is zero, the background of the font will be transparent.
// Note that if you change fonts, the selected foreground/background colors will
// stay at what they are currently set to.
//
//*****************************************************************************
void SetFontBackground(UINT8 ubBackground) {
  UINT32 uiRed, uiGreen, uiBlue;

  if ((FontDefault < 0) || (FontDefault > MAX_FONTS)) return;

  FontBackground8 = ubBackground;

  uiRed = (UINT32)FontObjs[FontDefault]->pPaletteEntry[ubBackground].peRed;
  uiGreen = (UINT32)FontObjs[FontDefault]->pPaletteEntry[ubBackground].peGreen;
  uiBlue = (UINT32)FontObjs[FontDefault]->pPaletteEntry[ubBackground].peBlue;

  FontBackground16 = Get16BPPColor(FROMRGB(uiRed, uiGreen, uiBlue));
}

// Kris:  These are new counterparts to the above functions.  They won't
//			 effect an 8BPP font, only 16.
void SetRGBFontForeground(UINT32 uiRed, UINT32 uiGreen, UINT32 uiBlue) {
  if ((FontDefault < 0) || (FontDefault > MAX_FONTS)) return;
  FontForeground16 = Get16BPPColor(FROMRGB(uiRed, uiGreen, uiBlue));
}

void SetRGBFontBackground(UINT32 uiRed, UINT32 uiGreen, UINT32 uiBlue) {
  if ((FontDefault < 0) || (FontDefault > MAX_FONTS)) return;
  FontBackground16 = Get16BPPColor(FROMRGB(uiRed, uiGreen, uiBlue));
}

void SetRGBFontShadow(UINT32 uiRed, UINT32 uiGreen, UINT32 uiBlue) {
  if ((FontDefault < 0) || (FontDefault > MAX_FONTS)) return;
  FontShadow16 = Get16BPPColor(FROMRGB(uiRed, uiGreen, uiBlue));
}
// end Kris

//*****************************************************************************
// ResetFontObjectPalette
//
//	Sets the palette of a font, using an 8 bit palette (which is converted to
// the appropriate 16-bit palette, and assigned to the struct VObject*).
//
//*****************************************************************************
BOOLEAN ResetFontObjectPalette(INT32 iFont) {
  Assert(iFont >= 0);
  Assert(iFont <= MAX_FONTS);
  Assert(FontObjs[iFont] != NULL);

  SetFontObjectPalette8BPP(iFont, FontObjs[iFont]->pPaletteEntry);

  return (TRUE);
}

//*****************************************************************************
// SetFontObjectPalette8BPP
//
//	Sets the palette of a font, using an 8 bit palette (which is converted to
// the appropriate 16-bit palette, and assigned to the struct VObject*).
//
//*****************************************************************************
UINT16 *SetFontObjectPalette8BPP(INT32 iFont, struct SGPPaletteEntry *pPal8) {
  UINT16 *pPal16;

  Assert(iFont >= 0);
  Assert(iFont <= MAX_FONTS);
  Assert(FontObjs[iFont] != NULL);

  if ((pPal16 = Create16BPPPalette(pPal8)) == NULL) return (NULL);

  FontObjs[iFont]->p16BPPPalette = pPal16;
  FontObjs[iFont]->pShadeCurrent = pPal16;

  return (pPal16);
}

//*****************************************************************************
// SetFontObjectPalette16BPP
//
//	Sets the palette of a font, using a 16 bit palette.
//
//*****************************************************************************
UINT16 *SetFontObjectPalette16BPP(INT32 iFont, UINT16 *pPal16) {
  Assert(iFont >= 0);
  Assert(iFont <= MAX_FONTS);
  Assert(FontObjs[iFont] != NULL);

  FontObjs[iFont]->p16BPPPalette = pPal16;
  FontObjs[iFont]->pShadeCurrent = pPal16;

  return (pPal16);
}

//*****************************************************************************
// GetFontObjectPalette16BPP
//
//	Sets the palette of a font, using a 16 bit palette.
//
//*****************************************************************************
UINT16 *GetFontObjectPalette16BPP(INT32 iFont) {
  Assert(iFont >= 0);
  Assert(iFont <= MAX_FONTS);
  Assert(FontObjs[iFont] != NULL);

  return (FontObjs[iFont]->p16BPPPalette);
}

//*****************************************************************************
// GetFontObject
//
//	Returns the VOBJECT pointer of a font.
//
//*****************************************************************************
struct VObject *GetFontObject(INT32 iFont) {
  Assert(iFont >= 0);
  Assert(iFont <= MAX_FONTS);
  Assert(FontObjs[iFont] != NULL);

  return (FontObjs[iFont]);
}

//*****************************************************************************
// FindFreeFont
//
//	Locates an empty slot in the font table.
//
//*****************************************************************************
INT32 FindFreeFont(void) {
  int count;

  for (count = 0; count < MAX_FONTS; count++)
    if (FontObjs[count] == NULL) return (count);

  return (-1);
}

//*****************************************************************************
// LoadFontFile
//
//	Loads a font from an ETRLE file, and inserts it into one of the font slots.
//  This function returns (-1) if it fails, and debug msgs for a reason.
//  Otherwise the font number is returned.
//*****************************************************************************
INT32 LoadFontFile(STR8 filename) {
  UINT32 LoadIndex;

  Assert(filename != NULL);
  Assert(strlen(filename));

  if ((LoadIndex = FindFreeFont()) == (-1)) {
    DebugMsg(TOPIC_FONT_HANDLER, DBG_ERROR, String("Out of font slots (%s)", filename));
    FatalError("Cannot init FONT file %s", filename);
    return (-1);
  }

  if ((FontObjs[LoadIndex] = CreateVObjectFromFile(filename)) == NULL) {
    DebugMsg(TOPIC_FONT_HANDLER, DBG_ERROR, String("Error creating VOBJECT (%s)", filename));
    FatalError("Cannot init FONT file %s", filename);
    return (-1);
  }

  if (FontDefault == (-1)) FontDefault = LoadIndex;

  return (LoadIndex);
}

//*****************************************************************************
// UnloadFont - Delete the font structure
//
//	Deletes the video object of a particular font. Frees up the memory and
// resources allocated for it.
//
//*****************************************************************************
void UnloadFont(UINT32 FontIndex) {
  Assert(FontIndex >= 0);
  Assert(FontIndex <= MAX_FONTS);
  Assert(FontObjs[FontIndex] != NULL);

  DeleteVideoObject(FontObjs[FontIndex]);
  FontObjs[FontIndex] = NULL;
}

//*****************************************************************************
// GetWidth
//
//	Returns the width of a given character in the font.
//
//*****************************************************************************
UINT32 GetWidth(struct VObject *hSrcVObject, INT16 ssIndex) {
  struct Subimage *pTrav;

  // Assertions
  Assert(hSrcVObject != NULL);

  // Get Offsets from Index into structure
  pTrav = &(hSrcVObject->subimages[ssIndex]);
  return ((UINT32)(pTrav->usWidth + pTrav->sOffsetX));
}

//*****************************************************************************
// StringPixLengthArg
//
//		Returns the length of a string with a variable number of arguments, in
// pixels, using the current font. Maximum length in characters the string can
// evaluate to is 512.
//    'uiCharCount' specifies how many characters of the string are counted.
//*****************************************************************************
INT16 StringPixLengthArg(INT32 usUseFont, UINT32 uiCharCount, STR16 pFontString, ...) {
  va_list argptr;
  wchar_t string[512];

  Assert(pFontString != NULL);

  va_start(argptr, pFontString);  // Set up variable argument pointer
  vswprintf(string, ARR_SIZE(string), pFontString,
            argptr);  // process gprintf string (get output str)
  va_end(argptr);

  // make sure the character count is legal
  if (uiCharCount > wcslen(string)) {
    uiCharCount = wcslen(string);
  } else {
    if (uiCharCount < wcslen(string)) {
      // less than the full string, so whack off the end of it (it's temporary anyway)
      string[uiCharCount] = '\0';
    }
  }

  return (StringPixLength(string, usUseFont));
}

//*****************************************************************************
// StringPixLengthArg
//
// Returns the length of a string with a variable number of arguments, in
// pixels, using the current font. Maximum length in characters the string can
// evaluate to is 512.  Because this is for fast help text, all '|' characters are ignored for the
// width calculation.
// 'uiCharCount' specifies how many characters of the string are counted.
// YOU HAVE TO PREBUILD THE FAST HELP STRING!
//*****************************************************************************
INT16 StringPixLengthArgFastHelp(INT32 usUseFont, INT32 usBoldFont, UINT32 uiCharCount,
                                 STR16 pFontString) {
  wchar_t string[512];
  UINT32 i, index;
  INT16 sBoldDiff = 0;
  CHAR16 str[2];

  Assert(pFontString != NULL);

  wcscpy(string, pFontString);

  // make sure the character count is legal
  if (uiCharCount > wcslen(string)) {
    uiCharCount = wcslen(string);
  } else {
    if (uiCharCount < wcslen(string)) {
      // less than the full string, so whack off the end of it (it's temporary anyway)
      string[uiCharCount] = '\0';
    }
  }
  // now eliminate all '|' characters from the string.
  i = 0;
  while (i < uiCharCount) {
    if (string[i] == '|') {
      for (index = i; index < uiCharCount; index++) {
        string[index] = string[index + 1];
      }
      uiCharCount--;
      // now we have eliminated the '|' character, so now calculate the size difference of the
      // bolded character.
      str[0] = string[i];
      str[1] = 0;
      sBoldDiff += StringPixLength(str, usBoldFont) - StringPixLength(str, usUseFont);
    }
    i++;
  }
  return StringPixLength(string, usUseFont) + sBoldDiff;
}

//*****************************************************************************************
//
//  StringNPixLength
//
//  Return the length of the of the string or count characters in the
//  string, which ever comes first.
//
//  Returns INT16
//
//  Created by:     Gilles Beauparlant
//  Created on:     12/1/99
//
//*****************************************************************************************
INT16 StringNPixLength(STR16 string, UINT32 uiMaxCount, INT32 UseFont) {
  UINT32 Cur, uiCharCount;
  STR16 curletter;
  INT16 transletter;

  Cur = 0;
  uiCharCount = 0;
  curletter = string;

  while ((*curletter) != L'\0' && uiCharCount < uiMaxCount) {
    transletter = GetIndex(*curletter++);
    Cur += GetWidth(FontObjs[UseFont], transletter);
    uiCharCount++;
  }
  return ((INT16)Cur);
}

//*****************************************************************************
//
// StringPixLength
//
//	Returns the length of a string in pixels, depending on the font given.
//
//*****************************************************************************
INT16 StringPixLength(STR16 string, INT32 UseFont) {
  UINT32 Cur;
  STR16 curletter;
  INT16 transletter;

  if (string == NULL) {
    return (0);
  }

  Cur = 0;
  curletter = string;

  while ((*curletter) != L'\0') {
    transletter = GetIndex(*curletter++);
    Cur += GetWidth(FontObjs[UseFont], transletter);
  }
  return ((INT16)Cur);
}

//*****************************************************************************
//
// SaveFontSettings
//
//	Saves the current font printing settings into temporary locations.
//
//*****************************************************************************
void SaveFontSettings(void) {
  SaveFontDefault = FontDefault;
  SaveFontDestSurface = FontDestSurface;
  SaveFontDestPitch = FontDestPitch;
  SaveFontDestBPP = FontDestBPP;
  SaveFontDestRegion = FontDestRegion;
  SaveFontDestWrap = FontDestWrap;
  SaveFontForeground16 = FontForeground16;
  SaveFontShadow16 = FontShadow16;
  SaveFontBackground16 = FontBackground16;
  SaveFontForeground8 = FontForeground8;
  SaveFontBackground8 = FontBackground8;
}

//*****************************************************************************
//
// RestoreFontSettings
//
//	Restores the last saved font printing settings from the temporary lactions
//
//*****************************************************************************
void RestoreFontSettings(void) {
  FontDefault = SaveFontDefault;
  FontDestSurface = SaveFontDestSurface;
  FontDestPitch = SaveFontDestPitch;
  FontDestBPP = SaveFontDestBPP;
  FontDestRegion = SaveFontDestRegion;
  FontDestWrap = SaveFontDestWrap;
  FontForeground16 = SaveFontForeground16;
  FontShadow16 = SaveFontShadow16;
  FontBackground16 = SaveFontBackground16;
  FontForeground8 = SaveFontForeground8;
  FontBackground8 = SaveFontBackground8;
}

//*****************************************************************************
// GetHeight
//
//	Returns the height of a given character in the font.
//
//*****************************************************************************
UINT32 GetHeight(struct VObject *hSrcVObject, INT16 ssIndex) {
  struct Subimage *pTrav;

  // Assertions
  Assert(hSrcVObject != NULL);

  // Get Offsets from Index into structure
  pTrav = &(hSrcVObject->subimages[ssIndex]);
  return ((UINT32)(pTrav->usHeight + pTrav->sOffsetY));
}

//*****************************************************************************
//
// GetFontHeight
//
//	Returns the height of the first character in a font.
//
//*****************************************************************************
UINT16 GetFontHeight(INT32 FontNum) {
  Assert(FontNum >= 0);
  Assert(FontNum <= MAX_FONTS);
  Assert(FontObjs[FontNum] != NULL);

  return ((UINT16)GetHeight(FontObjs[FontNum], 0));
}

//*****************************************************************************
// GetIndex
//
//		Given a word-sized character, this function returns the index of the
//	cell in the font to print to the screen. The conversion table is built by
//	CreateEnglishTransTable()
//
//*****************************************************************************
INT16 GetIndex(UINT16 siChar) {
  UINT16 *pTrav;
  UINT16 ssCount = 0;
  UINT16 usNumberOfSymbols = pFManager->pTranslationTable->usNumberOfSymbols;

  // search the Translation Table and return the index for the font
  pTrav = pFManager->pTranslationTable->DynamicArrayOf16BitValues;
  while (ssCount < usNumberOfSymbols) {
    if (siChar == *pTrav) {
      return ssCount;
    }
    ssCount++;
    pTrav++;
  }

  // If here, present warning and give the first index
  DebugMsg(TOPIC_FONT_HANDLER, DBG_ERROR, String("Error: Invalid character given %d", siChar));

  // Return 0 here, NOT -1 - we should see A's here now...
  return 0;
}

//*****************************************************************************
// SetFont
//
//	Sets the current font number.
//
//*****************************************************************************
BOOLEAN SetFont(INT32 iFontIndex) {
  Assert(iFontIndex >= 0);
  Assert(iFontIndex <= MAX_FONTS);
  Assert(FontObjs[iFontIndex] != NULL);

  FontDefault = iFontIndex;
  return (TRUE);
}

BOOLEAN SetFontDest(struct VSurface *dest, i32 x1, i32 y1, i32 x2, i32 y2, BOOLEAN wrap) {
  Assert(x2 > x1);
  Assert(y2 > y1);

  FontDestSurface = dest;

  FontDestRegion.iLeft = x1;
  FontDestRegion.iTop = y1;
  FontDestRegion.iRight = x2;
  FontDestRegion.iBottom = y2;
  FontDestWrap = wrap;

  return (TRUE);
}

//*****************************************************************************
// mprintf
//
//	Prints to the currently selected destination buffer, at the X/Y coordinates
// specified, using the currently selected font. Other than the X/Y coordinates,
// the parameters are identical to printf. The resulting string may be no longer
// than 512 word-characters. Uses monochrome font color settings
//*****************************************************************************
UINT32 mprintf(INT32 x, INT32 y, STR16 pFontString, ...) {
  INT32 destx, desty;
  STR16 curletter;
  INT16 transletter;
  va_list argptr;
  wchar_t string[512];
  UINT32 uiDestPitchBYTES;
  UINT8 *pDestBuf;

  Assert(pFontString != NULL);

  va_start(argptr, pFontString);  // Set up variable argument pointer
  vswprintf(string, ARR_SIZE(string), pFontString,
            argptr);  // process gprintf string (get output str)
  va_end(argptr);

  curletter = string;

  destx = x;
  desty = y;

  // Lock the dest buffer
  pDestBuf = VSurfaceLockOld(FontDestSurface, &uiDestPitchBYTES);

  while ((*curletter) != 0) {
    transletter = GetIndex(*curletter++);

    if (FontDestWrap &&
        BltIsClipped(FontObjs[FontDefault], destx, desty, transletter, &FontDestRegion)) {
      destx = x;
      desty += GetHeight(FontObjs[FontDefault], transletter);
    }

    // Blit directly
    Blt8BPPDataTo16BPPBufferMonoShadowClip(
        (UINT16 *)pDestBuf, uiDestPitchBYTES, FontObjs[FontDefault], destx, desty, transletter,
        &FontDestRegion, FontForeground16, FontBackground16, FontShadow16);
    destx += GetWidth(FontObjs[FontDefault], transletter);
  }

  // Unlock buffer
  VSurfaceUnlock(FontDestSurface);

  return (0);
}

void VarFindFontRightCoordinates(INT16 sLeft, INT16 sTop, INT16 sWidth, INT16 sHeight,
                                 INT32 iFontIndex, INT16 *psNewX, INT16 *psNewY, STR16 pFontString,
                                 ...) {
  wchar_t string[512];
  va_list argptr;

  va_start(argptr, pFontString);  // Set up variable argument pointer
  vswprintf(string, ARR_SIZE(string), pFontString,
            argptr);  // process gprintf string (get output str)
  va_end(argptr);

  FindFontRightCoordinates(sLeft, sTop, sWidth, sHeight, string, iFontIndex, psNewX, psNewY);
}

void VarFindFontCenterCoordinates(INT16 sLeft, INT16 sTop, INT16 sWidth, INT16 sHeight,
                                  INT32 iFontIndex, INT16 *psNewX, INT16 *psNewY, STR16 pFontString,
                                  ...) {
  wchar_t string[512];
  va_list argptr;

  va_start(argptr, pFontString);  // Set up variable argument pointer
  vswprintf(string, ARR_SIZE(string), pFontString,
            argptr);  // process gprintf string (get output str)
  va_end(argptr);

  FindFontCenterCoordinates(sLeft, sTop, sWidth, sHeight, string, iFontIndex, psNewX, psNewY);
}

void FindFontRightCoordinates(INT16 sLeft, INT16 sTop, INT16 sWidth, INT16 sHeight, STR16 pStr,
                              INT32 iFontIndex, INT16 *psNewX, INT16 *psNewY) {
  INT16 xp, yp;

  // Compute the coordinates to right justify the text
  xp = ((sWidth - StringPixLength(pStr, iFontIndex))) + sLeft;
  yp = ((sHeight - GetFontHeight(iFontIndex)) / 2) + sTop;

  *psNewX = xp;
  *psNewY = yp;
}

void FindFontCenterCoordinates(INT16 sLeft, INT16 sTop, INT16 sWidth, INT16 sHeight, STR16 pStr,
                               INT32 iFontIndex, INT16 *psNewX, INT16 *psNewY) {
  INT16 xp, yp;

  // Compute the coordinates to center the text
  xp = ((sWidth - StringPixLength(pStr, iFontIndex) + 1) / 2) + sLeft;
  yp = ((sHeight - GetFontHeight(iFontIndex)) / 2) + sTop;

  *psNewX = xp;
  *psNewY = yp;
}

//*****************************************************************************
// gprintf
//
//	Prints to the currently selected destination buffer, at the X/Y coordinates
// specified, using the currently selected font. Other than the X/Y coordinates,
// the parameters are identical to printf. The resulting string may be no longer
// than 512 word-characters.
//*****************************************************************************
UINT32 gprintf(INT32 x, INT32 y, STR16 pFontString, ...) {
  INT32 destx, desty;
  STR16 curletter;
  INT16 transletter;
  va_list argptr;
  wchar_t string[512];
  UINT32 uiDestPitchBYTES;
  UINT8 *pDestBuf;

  Assert(pFontString != NULL);

  va_start(argptr, pFontString);  // Set up variable argument pointer
  vswprintf(string, ARR_SIZE(string), pFontString,
            argptr);  // process gprintf string (get output str)
  va_end(argptr);

  curletter = string;

  destx = x;
  desty = y;

  // Lock the dest buffer
  pDestBuf = VSurfaceLockOld(FontDestSurface, &uiDestPitchBYTES);

  while ((*curletter) != 0) {
    transletter = GetIndex(*curletter++);

    if (FontDestWrap &&
        BltIsClipped(FontObjs[FontDefault], destx, desty, transletter, &FontDestRegion)) {
      destx = x;
      desty += GetHeight(FontObjs[FontDefault], transletter);
    }

    // Blit directly
    Blt8BPPDataTo16BPPBufferTransparentClip((UINT16 *)pDestBuf, uiDestPitchBYTES,
                                            FontObjs[FontDefault], destx, desty, transletter,
                                            &FontDestRegion);
    destx += GetWidth(FontObjs[FontDefault], transletter);
  }

  // Unlock buffer
  VSurfaceUnlock(FontDestSurface);

  return (0);
}

UINT32 gprintfDirty(INT32 x, INT32 y, STR16 pFontString, ...) {
  INT32 destx, desty;
  STR16 curletter;
  INT16 transletter;
  va_list argptr;
  wchar_t string[512];
  UINT32 uiDestPitchBYTES;
  UINT8 *pDestBuf;

  Assert(pFontString != NULL);

  va_start(argptr, pFontString);  // Set up variable argument pointer
  vswprintf(string, ARR_SIZE(string), pFontString,
            argptr);  // process gprintf string (get output str)
  va_end(argptr);

  curletter = string;

  destx = x;
  desty = y;

  // Lock the dest buffer
  pDestBuf = VSurfaceLockOld(FontDestSurface, &uiDestPitchBYTES);

  while ((*curletter) != 0) {
    transletter = GetIndex(*curletter++);

    if (FontDestWrap &&
        BltIsClipped(FontObjs[FontDefault], destx, desty, transletter, &FontDestRegion)) {
      destx = x;
      desty += GetHeight(FontObjs[FontDefault], transletter);
    }

    // Blit directly
    Blt8BPPDataTo16BPPBufferTransparentClip((UINT16 *)pDestBuf, uiDestPitchBYTES,
                                            FontObjs[FontDefault], destx, desty, transletter,
                                            &FontDestRegion);
    destx += GetWidth(FontObjs[FontDefault], transletter);
  }

  // Unlock buffer
  VSurfaceUnlock(FontDestSurface);

  InvalidateRegion(x, y, x + StringPixLength(string, FontDefault), y + GetFontHeight(FontDefault));

  return (0);
}
//*****************************************************************************
// gprintf_buffer
//
//	Prints to the currently selected destination buffer, at the X/Y coordinates
// specified, using the currently selected font. Other than the X/Y coordinates,
// the parameters are identical to printf. The resulting string may be no longer
// than 512 word-characters.
//*****************************************************************************
UINT32 gprintf_buffer(UINT8 *pDestBuf, UINT32 uiDestPitchBYTES, UINT32 FontType, INT32 x, INT32 y,
                      STR16 pFontString, ...) {
  INT32 destx, desty;
  STR16 curletter;
  INT16 transletter;
  va_list argptr;
  wchar_t string[512];

  Assert(pFontString != NULL);

  va_start(argptr, pFontString);  // Set up variable argument pointer
  vswprintf(string, ARR_SIZE(string), pFontString,
            argptr);  // process gprintf string (get output str)
  va_end(argptr);

  curletter = string;

  destx = x;
  desty = y;

  while ((*curletter) != 0) {
    transletter = GetIndex(*curletter++);

    if (FontDestWrap &&
        BltIsClipped(FontObjs[FontType], destx, desty, transletter, &FontDestRegion)) {
      destx = x;
      desty += GetHeight(FontObjs[FontType], transletter);
    }

    // Blit directly
    Blt8BPPDataTo16BPPBufferTransparentClip((UINT16 *)pDestBuf, uiDestPitchBYTES,
                                            FontObjs[FontDefault], destx, desty, transletter,
                                            &FontDestRegion);

    destx += GetWidth(FontObjs[FontType], transletter);
  }

  return (0);
}

UINT32 mprintf_buffer(UINT8 *pDestBuf, UINT32 uiDestPitchBYTES, UINT32 FontType, INT32 x, INT32 y,
                      STR16 pFontString, ...) {
  INT32 destx, desty;
  STR16 curletter;
  INT16 transletter;
  va_list argptr;
  wchar_t string[512];

  Assert(pFontString != NULL);

  va_start(argptr, pFontString);  // Set up variable argument pointer
  vswprintf(string, ARR_SIZE(string), pFontString,
            argptr);  // process gprintf string (get output str)
  va_end(argptr);

  curletter = string;

  destx = x;
  desty = y;

  while ((*curletter) != 0) {
    transletter = GetIndex(*curletter++);

    if (FontDestWrap &&
        BltIsClipped(FontObjs[FontDefault], destx, desty, transletter, &FontDestRegion)) {
      destx = x;
      desty += GetHeight(FontObjs[FontDefault], transletter);
    }

    // Blit directly
    Blt8BPPDataTo16BPPBufferMonoShadowClip(
        (UINT16 *)pDestBuf, uiDestPitchBYTES, FontObjs[FontDefault], destx, desty, transletter,
        &FontDestRegion, FontForeground16, FontBackground16, FontShadow16);
    destx += GetWidth(FontObjs[FontDefault], transletter);
  }

  return (0);
}

UINT32 mprintf_buffer_coded(UINT8 *pDestBuf, UINT32 uiDestPitchBYTES, UINT32 FontType, INT32 x,
                            INT32 y, STR16 pFontString, ...) {
  INT32 destx, desty;
  STR16 curletter;
  INT16 transletter;
  va_list argptr;
  wchar_t string[512];
  UINT16 usOldForeColor;

  Assert(pFontString != NULL);

  va_start(argptr, pFontString);  // Set up variable argument pointer
  vswprintf(string, ARR_SIZE(string), pFontString,
            argptr);  // process gprintf string (get output str)
  va_end(argptr);

  curletter = string;

  destx = x;
  desty = y;

  usOldForeColor = FontForeground16;

  while ((*curletter) != 0) {
    if ((*curletter) == 180) {
      curletter++;
      SetFontForeground((UINT8)(*curletter));
      curletter++;
    } else if ((*curletter) == 181) {
      FontForeground16 = usOldForeColor;
      curletter++;
    }

    transletter = GetIndex(*curletter++);

    if (FontDestWrap &&
        BltIsClipped(FontObjs[FontDefault], destx, desty, transletter, &FontDestRegion)) {
      destx = x;
      desty += GetHeight(FontObjs[FontDefault], transletter);
    }

    // Blit directly
    Blt8BPPDataTo16BPPBufferMonoShadowClip(
        (UINT16 *)pDestBuf, uiDestPitchBYTES, FontObjs[FontDefault], destx, desty, transletter,
        &FontDestRegion, FontForeground16, FontBackground16, FontShadow16);
    destx += GetWidth(FontObjs[FontDefault], transletter);
  }

  return (0);
}

UINT32 mprintf_coded(INT32 x, INT32 y, STR16 pFontString, ...) {
  INT32 destx, desty;
  STR16 curletter;
  INT16 transletter;
  va_list argptr;
  wchar_t string[512];
  UINT16 usOldForeColor;
  UINT32 uiDestPitchBYTES;
  UINT8 *pDestBuf;

  Assert(pFontString != NULL);

  va_start(argptr, pFontString);  // Set up variable argument pointer
  vswprintf(string, ARR_SIZE(string), pFontString,
            argptr);  // process gprintf string (get output str)
  va_end(argptr);

  curletter = string;

  destx = x;
  desty = y;

  usOldForeColor = FontForeground16;

  // Lock the dest buffer
  pDestBuf = VSurfaceLockOld(FontDestSurface, &uiDestPitchBYTES);

  while ((*curletter) != 0) {
    if ((*curletter) == 180) {
      curletter++;
      SetFontForeground((UINT8)(*curletter));
      curletter++;
    } else if ((*curletter) == 181) {
      FontForeground16 = usOldForeColor;
      curletter++;
    }

    transletter = GetIndex(*curletter++);

    if (FontDestWrap &&
        BltIsClipped(FontObjs[FontDefault], destx, desty, transletter, &FontDestRegion)) {
      destx = x;
      desty += GetHeight(FontObjs[FontDefault], transletter);
    }

    // Blit directly
    Blt8BPPDataTo16BPPBufferMonoShadowClip(
        (UINT16 *)pDestBuf, uiDestPitchBYTES, FontObjs[FontDefault], destx, desty, transletter,
        &FontDestRegion, FontForeground16, FontBackground16, FontShadow16);
    destx += GetWidth(FontObjs[FontDefault], transletter);
  }

  // Unlock buffer
  VSurfaceUnlock(FontDestSurface);

  return (0);
}

BOOLEAN InitializeFontManager(UINT16 usDefaultPixelDepth, FontTranslationTable *pTransTable) {
  FontTranslationTable *pTransTab;
  int count;
  UINT16 uiRight, uiBottom;

  FontDefault = (-1);
  FontDestSurface = vsBB;
  FontDestPitch = 0;

  //	FontDestBPP=0;

  GetCurrentVideoSettings(&uiRight, &uiBottom);
  FontDestRegion.iLeft = 0;
  FontDestRegion.iTop = 0;
  FontDestRegion.iRight = (INT32)uiRight;
  FontDestRegion.iBottom = (INT32)uiBottom;
  FontDestBPP = 16;

  FontDestWrap = FALSE;

  // register the appropriate debug topics
  if (pTransTable == NULL) {
    return FALSE;
  }

  if ((pFManager = (FontManager *)MemAlloc(sizeof(FontManager))) == NULL) {
    return FALSE;
  }

  if ((pTransTab = (FontTranslationTable *)MemAlloc(sizeof(FontTranslationTable))) == NULL) {
    return FALSE;
  }

  pFManager->pTranslationTable = pTransTab;
  pFManager->usDefaultPixelDepth = usDefaultPixelDepth;
  pTransTab->usNumberOfSymbols = pTransTable->usNumberOfSymbols;
  pTransTab->DynamicArrayOf16BitValues = pTransTable->DynamicArrayOf16BitValues;

  // Mark all font slots as empty
  for (count = 0; count < MAX_FONTS; count++) FontObjs[count] = NULL;

  return TRUE;
}

//*****************************************************************************
// ShutdownFontManager
//
//	Shuts down, and deallocates all fonts.
//*****************************************************************************
void ShutdownFontManager(void) {
  INT32 count;

  if (pFManager) MemFree(pFManager);

  for (count = 0; count < MAX_FONTS; count++) {
    if (FontObjs[count] != NULL) UnloadFont(count);
  }
}

//*****************************************************************************
// DestroyEnglishTransTable
//
// Destroys the English text->font map table.
//*****************************************************************************
void DestroyEnglishTransTable(void) {
  if (pFManager) {
    if (pFManager->pTranslationTable != NULL) {
      if (pFManager->pTranslationTable->DynamicArrayOf16BitValues != NULL) {
        MemFree(pFManager->pTranslationTable->DynamicArrayOf16BitValues);
      }

      MemFree(pFManager->pTranslationTable);

      pFManager->pTranslationTable = NULL;
    }
  }
}

//*****************************************************************************
// CreateEnglishTransTable
//
// Creates the English text->font map table.
//*****************************************************************************
FontTranslationTable *CreateEnglishTransTable() {
  FontTranslationTable *pTable = NULL;
  UINT16 *temp;

  pTable = (FontTranslationTable *)MemAlloc(sizeof(FontTranslationTable));
  pTable->usNumberOfSymbols = 172;
  pTable->DynamicArrayOf16BitValues = (UINT16 *)MemAlloc(pTable->usNumberOfSymbols * 2);
  temp = pTable->DynamicArrayOf16BitValues;

  *temp = 'A';
  temp++;
  *temp = 'B';
  temp++;
  *temp = 'C';
  temp++;
  *temp = 'D';
  temp++;
  *temp = 'E';
  temp++;
  *temp = 'F';
  temp++;
  *temp = 'G';
  temp++;
  *temp = 'H';
  temp++;
  *temp = 'I';
  temp++;
  *temp = 'J';
  temp++;
  *temp = 'K';
  temp++;
  *temp = 'L';
  temp++;
  *temp = 'M';
  temp++;
  *temp = 'N';
  temp++;
  *temp = 'O';
  temp++;
  *temp = 'P';
  temp++;
  *temp = 'Q';
  temp++;
  *temp = 'R';
  temp++;
  *temp = 'S';
  temp++;
  *temp = 'T';
  temp++;
  *temp = 'U';
  temp++;
  *temp = 'V';
  temp++;
  *temp = 'W';
  temp++;
  *temp = 'X';
  temp++;
  *temp = 'Y';
  temp++;
  *temp = 'Z';
  temp++;
  *temp = 'a';
  temp++;
  *temp = 'b';
  temp++;
  *temp = 'c';
  temp++;
  *temp = 'd';
  temp++;
  *temp = 'e';
  temp++;
  *temp = 'f';
  temp++;
  *temp = 'g';
  temp++;
  *temp = 'h';
  temp++;
  *temp = 'i';
  temp++;
  *temp = 'j';
  temp++;
  *temp = 'k';
  temp++;
  *temp = 'l';
  temp++;
  *temp = 'm';
  temp++;
  *temp = 'n';
  temp++;
  *temp = 'o';
  temp++;
  *temp = 'p';
  temp++;
  *temp = 'q';
  temp++;
  *temp = 'r';
  temp++;
  *temp = 's';
  temp++;
  *temp = 't';
  temp++;
  *temp = 'u';
  temp++;
  *temp = 'v';
  temp++;
  *temp = 'w';
  temp++;
  *temp = 'x';
  temp++;
  *temp = 'y';
  temp++;
  *temp = 'z';
  temp++;
  *temp = '0';
  temp++;
  *temp = '1';
  temp++;
  *temp = '2';
  temp++;
  *temp = '3';
  temp++;
  *temp = '4';
  temp++;
  *temp = '5';
  temp++;
  *temp = '6';
  temp++;
  *temp = '7';
  temp++;
  *temp = '8';
  temp++;
  *temp = '9';
  temp++;
  *temp = '!';
  temp++;
  *temp = '@';
  temp++;
  *temp = '#';
  temp++;
  *temp = '$';
  temp++;
  *temp = '%';
  temp++;
  *temp = '^';
  temp++;
  *temp = '&';
  temp++;
  *temp = '*';
  temp++;
  *temp = '(';
  temp++;
  *temp = ')';
  temp++;
  *temp = '-';
  temp++;
  *temp = '_';
  temp++;
  *temp = '+';
  temp++;
  *temp = '=';
  temp++;
  *temp = '|';
  temp++;
  *temp = '\\';
  temp++;
  *temp = '{';
  temp++;
  *temp = '}';  // 80
  temp++;
  *temp = '[';
  temp++;
  *temp = ']';
  temp++;
  *temp = ':';
  temp++;
  *temp = ';';
  temp++;
  *temp = '"';
  temp++;
  *temp = '\'';
  temp++;
  *temp = '<';
  temp++;
  *temp = '>';
  temp++;
  *temp = ',';
  temp++;
  *temp = '.';
  temp++;
  *temp = '?';
  temp++;
  *temp = '/';
  temp++;
  *temp = ' ';  // 93
  temp++;

  *temp = 196;  // "A" umlaut
  temp++;
  *temp = 214;  // "O" umlaut
  temp++;
  *temp = 220;  // "U" umlaut
  temp++;
  *temp = 228;  // "a" umlaut
  temp++;
  *temp = 246;  // "o" umlaut
  temp++;
  *temp = 252;  // "u" umlaut
  temp++;
  *temp = 223;  // double-s that looks like a beta/B  // 100
  temp++;
  // START OF FUNKY RUSSIAN STUFF
  *temp = 1101;
  temp++;
  *temp = 1102;
  temp++;
  *temp = 1103;
  temp++;
  *temp = 1104;
  temp++;
  *temp = 1105;
  temp++;
  *temp = 1106;
  temp++;
  *temp = 1107;
  temp++;
  *temp = 1108;
  temp++;
  *temp = 1109;
  temp++;
  *temp = 1110;
  temp++;
  *temp = 1111;
  temp++;
  *temp = 1112;
  temp++;
  *temp = 1113;
  temp++;
  *temp = 1114;
  temp++;
  *temp = 1115;
  temp++;
  *temp = 1116;
  temp++;
  *temp = 1117;
  temp++;
  *temp = 1118;
  temp++;
  *temp = 1119;
  temp++;
  *temp = 1120;
  temp++;
  *temp = 1121;
  temp++;
  *temp = 1122;
  temp++;
  *temp = 1123;
  temp++;
  *temp = 1124;
  temp++;
  *temp = 1125;
  temp++;
  *temp = 1126;
  temp++;
  *temp = 1127;
  temp++;
  *temp = 1128;
  temp++;
  *temp = 1129;
  temp++;
  *temp = 1130;  // 130
  temp++;
  *temp = 1131;
  temp++;
  *temp = 1132;
  temp++;
  // END OF FUNKY RUSSIAN STUFF
  *temp = 196;  // �
  temp++;
  *temp = 192;  // �
  temp++;
  *temp = 193;  // �
  temp++;
  *temp = 194;  // �
  temp++;
  *temp = 199;  // �
  temp++;
  *temp = 203;  // �
  temp++;
  *temp = 200;  // �
  temp++;
  *temp = 201;  // �				140
  temp++;
  *temp = 202;  // �
  temp++;
  *temp = 207;  // �
  temp++;
  *temp = 214;  // �
  temp++;
  *temp = 210;  // �
  temp++;
  *temp = 211;  // �
  temp++;
  *temp = 212;  // �
  temp++;
  *temp = 220;  // �
  temp++;
  *temp = 217;  // �
  temp++;
  *temp = 218;  // �
  temp++;
  *temp = 219;  // �				150
  temp++;

  *temp = 228;  // �
  temp++;
  *temp = 224;  // �
  temp++;
  *temp = 225;  // �
  temp++;
  *temp = 226;  // �
  temp++;
  *temp = 231;  // �
  temp++;
  *temp = 235;  // �
  temp++;
  *temp = 232;  // �
  temp++;
  *temp = 233;  // �
  temp++;
  *temp = 234;  // �
  temp++;
  *temp = 239;  // �				160
  temp++;
  *temp = 246;  // �
  temp++;
  *temp = 242;  // �
  temp++;
  *temp = 243;  // �
  temp++;
  *temp = 244;  // �
  temp++;
  *temp = 252;  // �
  temp++;
  *temp = 249;  // �
  temp++;
  *temp = 250;  // �
  temp++;
  *temp = 251;  // �
  temp++;
  *temp = 204;  // �
  temp++;
  *temp = 206;  // �				170
  temp++;
  *temp = 236;  // �
  temp++;
  *temp = 238;  // �
  temp++;

  return pTable;
}
