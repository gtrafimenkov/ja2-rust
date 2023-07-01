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
#include "SGP/PaletteEntry.h"
#include "SGP/Types.h"
#include "SGP/VObject.h"
#include "SGP/VObjectBlitters.h"
#include "SGP/VObjectInternal.h"
#include "SGP/VSurface.h"
#include "SGP/Video.h"
#include "rust_fileman.h"

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
  uint16_t usDefaultPixelDepth;
  FontTranslationTable *pTranslationTable;
} FontManager;

FontManager *pFManager;
struct VObject *FontObjs[MAX_FONTS];
int32_t FontsLoaded = 0;

// Destination printing parameters
int32_t FontDefault = (-1);
struct VSurface *FontDestSurface = NULL;
uint32_t FontDestPitch = 640 * 2;
uint32_t FontDestBPP = 16;
SGPRect FontDestRegion = {0, 0, 640, 480};
BOOLEAN FontDestWrap = FALSE;
uint16_t FontForeground16 = 0;
uint16_t FontBackground16 = 0;
uint16_t FontShadow16 = DEFAULT_SHADOW;
uint8_t FontForeground8 = 0;
uint8_t FontBackground8 = 0;

// Temp, for saving printing parameters
int32_t SaveFontDefault = (-1);
struct VSurface *SaveFontDestSurface = NULL;
uint32_t SaveFontDestPitch = 640 * 2;
uint32_t SaveFontDestBPP = 16;
SGPRect SaveFontDestRegion = {0, 0, 640, 480};
BOOLEAN SaveFontDestWrap = FALSE;
uint16_t SaveFontForeground16 = 0;
uint16_t SaveFontShadow16 = 0;
uint16_t SaveFontBackground16 = 0;
uint8_t SaveFontForeground8 = 0;
uint8_t SaveFontBackground8 = 0;

//*****************************************************************************
// SetFontColors
//
//	Sets both the foreground and the background colors of the current font. The
// top byte of the parameter word is the background color, and the bottom byte
// is the foreground.
//
//*****************************************************************************
void SetFontColors(uint16_t usColors) {
  uint8_t ubForeground, ubBackground;

  ubForeground = (uint8_t)(usColors & 0xff);
  ubBackground = (uint8_t)((usColors & 0xff00) >> 8);

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
void SetFontForeground(uint8_t ubForeground) {
  uint32_t uiRed, uiGreen, uiBlue;

  if ((FontDefault < 0) || (FontDefault > MAX_FONTS)) return;

  FontForeground8 = ubForeground;

  uiRed = (uint32_t)FontObjs[FontDefault]->pPaletteEntry[ubForeground].peRed;
  uiGreen = (uint32_t)FontObjs[FontDefault]->pPaletteEntry[ubForeground].peGreen;
  uiBlue = (uint32_t)FontObjs[FontDefault]->pPaletteEntry[ubForeground].peBlue;

  FontForeground16 = Get16BPPColor(FROMRGB(uiRed, uiGreen, uiBlue));
}

void SetFontShadow(uint8_t ubShadow) {
  uint32_t uiRed, uiGreen, uiBlue;

  if ((FontDefault < 0) || (FontDefault > MAX_FONTS)) return;

  // FontForeground8=ubForeground;

  uiRed = (uint32_t)FontObjs[FontDefault]->pPaletteEntry[ubShadow].peRed;
  uiGreen = (uint32_t)FontObjs[FontDefault]->pPaletteEntry[ubShadow].peGreen;
  uiBlue = (uint32_t)FontObjs[FontDefault]->pPaletteEntry[ubShadow].peBlue;

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
void SetFontBackground(uint8_t ubBackground) {
  uint32_t uiRed, uiGreen, uiBlue;

  if ((FontDefault < 0) || (FontDefault > MAX_FONTS)) return;

  FontBackground8 = ubBackground;

  uiRed = (uint32_t)FontObjs[FontDefault]->pPaletteEntry[ubBackground].peRed;
  uiGreen = (uint32_t)FontObjs[FontDefault]->pPaletteEntry[ubBackground].peGreen;
  uiBlue = (uint32_t)FontObjs[FontDefault]->pPaletteEntry[ubBackground].peBlue;

  FontBackground16 = Get16BPPColor(FROMRGB(uiRed, uiGreen, uiBlue));
}

// Kris:  These are new counterparts to the above functions.  They won't
//			 effect an 8BPP font, only 16.
void SetRGBFontForeground(uint32_t uiRed, uint32_t uiGreen, uint32_t uiBlue) {
  if ((FontDefault < 0) || (FontDefault > MAX_FONTS)) return;
  FontForeground16 = Get16BPPColor(FROMRGB(uiRed, uiGreen, uiBlue));
}

void SetRGBFontBackground(uint32_t uiRed, uint32_t uiGreen, uint32_t uiBlue) {
  if ((FontDefault < 0) || (FontDefault > MAX_FONTS)) return;
  FontBackground16 = Get16BPPColor(FROMRGB(uiRed, uiGreen, uiBlue));
}

void SetRGBFontShadow(uint32_t uiRed, uint32_t uiGreen, uint32_t uiBlue) {
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
BOOLEAN ResetFontObjectPalette(int32_t iFont) {
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
uint16_t *SetFontObjectPalette8BPP(int32_t iFont, struct SGPPaletteEntry *pPal8) {
  uint16_t *pPal16;

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
uint16_t *SetFontObjectPalette16BPP(int32_t iFont, uint16_t *pPal16) {
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
uint16_t *GetFontObjectPalette16BPP(int32_t iFont) {
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
struct VObject *GetFontObject(int32_t iFont) {
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
int32_t FindFreeFont(void) {
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
int32_t LoadFontFile(char *filename) {
  uint32_t LoadIndex;

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
void UnloadFont(uint32_t FontIndex) {
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
uint32_t GetWidth(struct VObject *hSrcVObject, int16_t ssIndex) {
  ETRLEObject *pTrav;

  // Assertions
  Assert(hSrcVObject != NULL);

  // Get Offsets from Index into structure
  pTrav = &(hSrcVObject->pETRLEObject[ssIndex]);
  return ((uint32_t)(pTrav->usWidth + pTrav->sOffsetX));
}

//*****************************************************************************
// StringPixLengthArg
//
//		Returns the length of a string with a variable number of arguments, in
// pixels, using the current font. Maximum length in characters the string can
// evaluate to is 512.
//    'uiCharCount' specifies how many characters of the string are counted.
//*****************************************************************************
int16_t StringPixLengthArg(int32_t usUseFont, uint32_t uiCharCount, wchar_t *pFontString, ...) {
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
int16_t StringPixLengthArgFastHelp(int32_t usUseFont, int32_t usBoldFont, uint32_t uiCharCount,
                                   wchar_t *pFontString) {
  wchar_t string[512];
  uint32_t i, index;
  int16_t sBoldDiff = 0;
  wchar_t str[2];

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
//  Returns int16_t
//
//  Created by:     Gilles Beauparlant
//  Created on:     12/1/99
//
//*****************************************************************************************
int16_t StringNPixLength(wchar_t *string, uint32_t uiMaxCount, int32_t UseFont) {
  uint32_t Cur, uiCharCount;
  wchar_t *curletter;
  int16_t transletter;

  Cur = 0;
  uiCharCount = 0;
  curletter = string;

  while ((*curletter) != L'\0' && uiCharCount < uiMaxCount) {
    transletter = GetIndex(*curletter++);
    Cur += GetWidth(FontObjs[UseFont], transletter);
    uiCharCount++;
  }
  return ((int16_t)Cur);
}

//*****************************************************************************
//
// StringPixLength
//
//	Returns the length of a string in pixels, depending on the font given.
//
//*****************************************************************************
int16_t StringPixLength(wchar_t *string, int32_t UseFont) {
  uint32_t Cur;
  wchar_t *curletter;
  int16_t transletter;

  if (string == NULL) {
    return (0);
  }

  Cur = 0;
  curletter = string;

  while ((*curletter) != L'\0') {
    transletter = GetIndex(*curletter++);
    Cur += GetWidth(FontObjs[UseFont], transletter);
  }
  return ((int16_t)Cur);
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
uint32_t GetHeight(struct VObject *hSrcVObject, int16_t ssIndex) {
  ETRLEObject *pTrav;

  // Assertions
  Assert(hSrcVObject != NULL);

  // Get Offsets from Index into structure
  pTrav = &(hSrcVObject->pETRLEObject[ssIndex]);
  return ((uint32_t)(pTrav->usHeight + pTrav->sOffsetY));
}

//*****************************************************************************
//
// GetFontHeight
//
//	Returns the height of the first character in a font.
//
//*****************************************************************************
uint16_t GetFontHeight(int32_t FontNum) {
  Assert(FontNum >= 0);
  Assert(FontNum <= MAX_FONTS);
  Assert(FontObjs[FontNum] != NULL);

  return ((uint16_t)GetHeight(FontObjs[FontNum], 0));
}

//*****************************************************************************
// GetIndex
//
//		Given a word-sized character, this function returns the index of the
//	cell in the font to print to the screen. The conversion table is built by
//	CreateEnglishTransTable()
//
//*****************************************************************************
int16_t GetIndex(uint16_t siChar) {
  uint16_t *pTrav;
  uint16_t ssCount = 0;
  uint16_t usNumberOfSymbols = pFManager->pTranslationTable->usNumberOfSymbols;

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
BOOLEAN SetFont(int32_t iFontIndex) {
  Assert(iFontIndex >= 0);
  Assert(iFontIndex <= MAX_FONTS);
  Assert(FontObjs[iFontIndex] != NULL);

  FontDefault = iFontIndex;
  return (TRUE);
}

BOOLEAN SetFontDest(struct VSurface *dest, int32_t x1, int32_t y1, int32_t x2, int32_t y2,
                    BOOLEAN wrap) {
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
uint32_t mprintf(int32_t x, int32_t y, wchar_t *pFontString, ...) {
  int32_t destx, desty;
  wchar_t *curletter;
  int16_t transletter;
  va_list argptr;
  wchar_t string[512];
  uint32_t uiDestPitchBYTES;
  uint8_t *pDestBuf;

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
        (uint16_t *)pDestBuf, uiDestPitchBYTES, FontObjs[FontDefault], destx, desty, transletter,
        &FontDestRegion, FontForeground16, FontBackground16, FontShadow16);
    destx += GetWidth(FontObjs[FontDefault], transletter);
  }

  // Unlock buffer
  VSurfaceUnlock(FontDestSurface);

  return (0);
}

void VarFindFontRightCoordinates(int16_t sLeft, int16_t sTop, int16_t sWidth, int16_t sHeight,
                                 int32_t iFontIndex, int16_t *psNewX, int16_t *psNewY,
                                 wchar_t *pFontString, ...) {
  wchar_t string[512];
  va_list argptr;

  va_start(argptr, pFontString);  // Set up variable argument pointer
  vswprintf(string, ARR_SIZE(string), pFontString,
            argptr);  // process gprintf string (get output str)
  va_end(argptr);

  FindFontRightCoordinates(sLeft, sTop, sWidth, sHeight, string, iFontIndex, psNewX, psNewY);
}

void VarFindFontCenterCoordinates(int16_t sLeft, int16_t sTop, int16_t sWidth, int16_t sHeight,
                                  int32_t iFontIndex, int16_t *psNewX, int16_t *psNewY,
                                  wchar_t *pFontString, ...) {
  wchar_t string[512];
  va_list argptr;

  va_start(argptr, pFontString);  // Set up variable argument pointer
  vswprintf(string, ARR_SIZE(string), pFontString,
            argptr);  // process gprintf string (get output str)
  va_end(argptr);

  FindFontCenterCoordinates(sLeft, sTop, sWidth, sHeight, string, iFontIndex, psNewX, psNewY);
}

void FindFontRightCoordinates(int16_t sLeft, int16_t sTop, int16_t sWidth, int16_t sHeight,
                              wchar_t *pStr, int32_t iFontIndex, int16_t *psNewX, int16_t *psNewY) {
  int16_t xp, yp;

  // Compute the coordinates to right justify the text
  xp = ((sWidth - StringPixLength(pStr, iFontIndex))) + sLeft;
  yp = ((sHeight - GetFontHeight(iFontIndex)) / 2) + sTop;

  *psNewX = xp;
  *psNewY = yp;
}

void FindFontCenterCoordinates(int16_t sLeft, int16_t sTop, int16_t sWidth, int16_t sHeight,
                               wchar_t *pStr, int32_t iFontIndex, int16_t *psNewX,
                               int16_t *psNewY) {
  int16_t xp, yp;

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
uint32_t gprintf(int32_t x, int32_t y, wchar_t *pFontString, ...) {
  int32_t destx, desty;
  wchar_t *curletter;
  int16_t transletter;
  va_list argptr;
  wchar_t string[512];
  uint32_t uiDestPitchBYTES;
  uint8_t *pDestBuf;

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
    Blt8BPPDataTo16BPPBufferTransparentClip((uint16_t *)pDestBuf, uiDestPitchBYTES,
                                            FontObjs[FontDefault], destx, desty, transletter,
                                            &FontDestRegion);
    destx += GetWidth(FontObjs[FontDefault], transletter);
  }

  // Unlock buffer
  VSurfaceUnlock(FontDestSurface);

  return (0);
}

uint32_t gprintfDirty(int32_t x, int32_t y, wchar_t *pFontString, ...) {
  int32_t destx, desty;
  wchar_t *curletter;
  int16_t transletter;
  va_list argptr;
  wchar_t string[512];
  uint32_t uiDestPitchBYTES;
  uint8_t *pDestBuf;

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
    Blt8BPPDataTo16BPPBufferTransparentClip((uint16_t *)pDestBuf, uiDestPitchBYTES,
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
uint32_t gprintf_buffer(uint8_t *pDestBuf, uint32_t uiDestPitchBYTES, uint32_t FontType, int32_t x,
                        int32_t y, wchar_t *pFontString, ...) {
  int32_t destx, desty;
  wchar_t *curletter;
  int16_t transletter;
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
    Blt8BPPDataTo16BPPBufferTransparentClip((uint16_t *)pDestBuf, uiDestPitchBYTES,
                                            FontObjs[FontDefault], destx, desty, transletter,
                                            &FontDestRegion);

    destx += GetWidth(FontObjs[FontType], transletter);
  }

  return (0);
}

uint32_t mprintf_buffer(uint8_t *pDestBuf, uint32_t uiDestPitchBYTES, uint32_t FontType, int32_t x,
                        int32_t y, wchar_t *pFontString, ...) {
  int32_t destx, desty;
  wchar_t *curletter;
  int16_t transletter;
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
        (uint16_t *)pDestBuf, uiDestPitchBYTES, FontObjs[FontDefault], destx, desty, transletter,
        &FontDestRegion, FontForeground16, FontBackground16, FontShadow16);
    destx += GetWidth(FontObjs[FontDefault], transletter);
  }

  return (0);
}

uint32_t mprintf_buffer_coded(uint8_t *pDestBuf, uint32_t uiDestPitchBYTES, uint32_t FontType,
                              int32_t x, int32_t y, wchar_t *pFontString, ...) {
  int32_t destx, desty;
  wchar_t *curletter;
  int16_t transletter;
  va_list argptr;
  wchar_t string[512];
  uint16_t usOldForeColor;

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
      SetFontForeground((uint8_t)(*curletter));
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
        (uint16_t *)pDestBuf, uiDestPitchBYTES, FontObjs[FontDefault], destx, desty, transletter,
        &FontDestRegion, FontForeground16, FontBackground16, FontShadow16);
    destx += GetWidth(FontObjs[FontDefault], transletter);
  }

  return (0);
}

uint32_t mprintf_coded(int32_t x, int32_t y, wchar_t *pFontString, ...) {
  int32_t destx, desty;
  wchar_t *curletter;
  int16_t transletter;
  va_list argptr;
  wchar_t string[512];
  uint16_t usOldForeColor;
  uint32_t uiDestPitchBYTES;
  uint8_t *pDestBuf;

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
      SetFontForeground((uint8_t)(*curletter));
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
        (uint16_t *)pDestBuf, uiDestPitchBYTES, FontObjs[FontDefault], destx, desty, transletter,
        &FontDestRegion, FontForeground16, FontBackground16, FontShadow16);
    destx += GetWidth(FontObjs[FontDefault], transletter);
  }

  // Unlock buffer
  VSurfaceUnlock(FontDestSurface);

  return (0);
}

BOOLEAN InitializeFontManager(uint16_t usDefaultPixelDepth, FontTranslationTable *pTransTable) {
  FontTranslationTable *pTransTab;
  int count;
  uint16_t uiRight, uiBottom;

  FontDefault = (-1);
  FontDestSurface = vsBB;
  FontDestPitch = 0;

  //	FontDestBPP=0;

  GetCurrentVideoSettings(&uiRight, &uiBottom);
  FontDestRegion.iLeft = 0;
  FontDestRegion.iTop = 0;
  FontDestRegion.iRight = (int32_t)uiRight;
  FontDestRegion.iBottom = (int32_t)uiBottom;
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
  int32_t count;

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
  uint16_t *temp;

  pTable = (FontTranslationTable *)MemAlloc(sizeof(FontTranslationTable));
  pTable->usNumberOfSymbols = 172;
  pTable->DynamicArrayOf16BitValues = (uint16_t *)MemAlloc(pTable->usNumberOfSymbols * 2);
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

//*****************************************************************************
//
// LoadFontFile
//
// Parameter List : filename - File created by the utility tool to open
//
// Return Value  pointer to the base structure
//
// Modification History :
// Dec 15th 1996 -> modified for use by Wizardry
//
//*****************************************************************************

/*FontBase *LoadFontFile(char* pFilename)
{
FileID           hFileHandle = FILE_ID_ERR;
  uint32_t           uiFileSize;
  uint32_t           uiHeightEach;
  uint32_t           uiTotalSymbol;
  uint32_t           uiNewoffst, uiOldoffst;
  FontBase        *pFontBase;
  struct SGPPaletteEntry *pNewPalette;
  uint8_t           *pPalette;

  if (pFManager == NULL)
  {
    DebugMsg(TOPIC_FONT_HANDLER, DBG_ERROR, "Did not Initialize Font Manager");
    return NULL;
  }

  // Open and read in the file
  if ((hFileHandle = File_OpenForReading(pFilename)) == 0)
  { // damn we failed to open the file
    DebugMsg(TOPIC_FONT_HANDLER, DBG_ERROR, "Cannot open font file");
    return NULL;
  }

  uiFileSize = File_GetSize(hFileHandle);
  if (uiFileSize == 0)
  { // we failed to size up the file
    DebugMsg(TOPIC_FONT_HANDLER, DBG_ERROR, "Font file is empty");
    File_Close(hFileHandle);
    return NULL;
  }

  // Allocate memory for the font header file
  if ((pFontBase = (FontBase *)MemAlloc(sizeof(FontBase))) == NULL)
  {
    DebugMsg(TOPIC_FONT_HANDLER, DBG_ERROR, "Could not malloc memory");
          File_Close(hFileHandle);
  }

  // read in these values from the file
  if (File_Read(hFileHandle, &uiHeightEach, sizeof(uint32_t), NULL) == FALSE)
  {
          DebugMsg(TOPIC_FONT_HANDLER, DBG_ERROR, "Could not read Height from File");
          File_Close(hFileHandle);
          return NULL;
  }

  if (File_Read(hFileHandle, &uiTotalSymbol, sizeof(uint32_t), NULL) == FALSE)
  {
          DebugMsg(TOPIC_FONT_HANDLER, DBG_ERROR, "Could not read Total Symbol from File");
          File_Close(hFileHandle);
          return NULL;
  }

  // Assign the proper values to the Base structure
  pFontBase->uiHeightEach = uiHeightEach;
  pFontBase->uiTotalElements = uiTotalSymbol;
  pFontBase->pFontObject = (FontObject *)MemAlloc(uiTotalSymbol * sizeof(FontHeader));
  pPalette = (uint8_t *)MemAlloc(PALETTE_SIZE);
  uiOldoffst = (sizeof(FontHeader) + sizeof(FontObject)*pFontBase->uiTotalElements);
  uiNewoffst = uiFileSize - uiOldoffst;
  pFontBase->pPixData8 = (uint8_t *)MemAlloc(uiNewoffst);

  //seek past the FontHeader
  if (File_Seek(hFileHandle, sizeof(FontHeader), FILE_SEEK_START) == FALSE)
  {
          DebugMsg(TOPIC_FONT_HANDLER, DBG_ERROR, "Could not seek FileHeader");
          File_Close(hFileHandle);
          return NULL;
  }

  //read in the FontObject
  if (File_Read(hFileHandle, pFontBase->pFontObject, (uiTotalSymbol)*sizeof(FontHeader), NULL) ==
FALSE)
  {
          DebugMsg(TOPIC_FONT_HANDLER, DBG_ERROR, "Could not seek Font Objects");
          File_Close(hFileHandle);
          return NULL;
  }

  if (File_Seek(hFileHandle, uiOldoffst, FILE_SEEK_START) == FALSE)
  {
          DebugMsg(TOPIC_FONT_HANDLER, DBG_ERROR, "Could not seek Old offset");
          File_Close(hFileHandle);
          return NULL;
  }

  // read in the Pixel data
  if (File_Read(hFileHandle, pFontBase->pPixData8, uiNewoffst, NULL) == FALSE)
  {
          DebugMsg(TOPIC_FONT_HANDLER, DBG_ERROR, "Could not seek Pixel data");
          File_Close(hFileHandle);
          return NULL;
  }

  // seek proper position to read in Palette
  if (File_Seek(hFileHandle, sizeof(uint32_t)*3, FILE_SEEK_START) == FALSE)
  {
          DebugMsg(TOPIC_FONT_HANDLER, DBG_ERROR, "Could not seek Palette Start");
          File_Close(hFileHandle);
          return NULL;
  }

  // read in Palette
  if (File_Read(hFileHandle, pPalette, PALETTE_SIZE, NULL) == FALSE)
  {
          DebugMsg(TOPIC_FONT_HANDLER, DBG_ERROR, "Could not read Palette");
          File_Close(hFileHandle);
          return NULL;
  }

  // set the default pixel depth
  pFontBase->siPixelDepth = pFManager->usDefaultPixelDepth;
  File_Close(hFileHandle);

  // convert from RGB to struct SGPPaletteEntry
  pNewPalette = ConvertToPaletteEntry(0, 255, pPalette);
  pFontBase->pPalette = pNewPalette;

  // create the 16BPer Pixel palette
  if ((pFontBase->pPalet16 = Create16BPPPalette(pNewPalette)) == NULL)
  {
          DebugMsg(TOPIC_FONT_HANDLER, DBG_ERROR, "Could not create 16 bit palette");
          return NULL;
  }
  // return the FontBase structure
  return pFontBase;
}	*/

/*void UnloadFont(FontBase *pFontBase)
{
        // free allocated memory in FontBase
        if(pFontBase!=NULL)
        {
                if(pFontBase->pPalette!=NULL)
                        MemFree(pFontBase->pPalette);
                if(pFontBase->pPalet16!=NULL)
                        MemFree(pFontBase->pPalet16);
                if(pFontBase->pFontObject!=NULL)
                        MemFree(pFontBase->pFontObject);
                if(pFontBase->pPixData8!=NULL)
                        MemFree(pFontBase->pPixData8);
                if(pFontBase->pPixData16!=NULL)
                        MemFree(pFontBase->pPixData16);
                MemFree(pFontBase);
        }
}	*/

//*****************************************************************************
//
// GetMaxFontWidth - Gets the maximum font width
//
// Parameter List : pointer to the base structure
//
// Return Value  Maximum font width
//
// Modification History :
// Dec 15th 1996 -> modified for use by Wizardry
//
//*****************************************************************************

/*uint16_t GetMaxFontWidth(FontBase *pFontBase)
{
        FontObject *pWidth;
        uint32_t siBiggest = 0;
        uint16_t siCount;

  Assert(pFontBase != NULL);
        pWidth = pFontBase->pFontObject;
        // traverse the FontObject structure to find the biggest width
        for(siCount = 0; siCount < pFontBase->uiTotalElements; siCount++)
        {
                if( pWidth->uiFontWidth > siBiggest)
                {
      siBiggest = pWidth->uiFontWidth;
    }
                pWidth++;
        }
        // return the max width
        return (uint16_t)siBiggest;
} */

//*****************************************************************************
//
// ConvertToPaletteEntry
//
// Parameter List : Converts from RGB to struct SGPPaletteEntry
//
// Return Value  pointer to the struct SGPPaletteEntry
//
// Modification History :
// Dec 15th 1996 -> modified for use by Wizardry
//
//*****************************************************************************

/*
struct SGPPaletteEntry *ConvertToPaletteEntry(uint8_t sbStart, uint8_t sbEnd, uint8_t *pOldPalette)
{
        uint16_t Index;
  struct SGPPaletteEntry *pPalEntry;
        struct SGPPaletteEntry *pInitEntry;

        pPalEntry = (struct SGPPaletteEntry *)MemAlloc(sizeof(struct SGPPaletteEntry) * 256);
        pInitEntry = pPalEntry;
  DebugMsg(TOPIC_FONT_HANDLER, DBG_ERROR, "Converting RGB palette to struct SGPPaletteEntry");
  for(Index=0; Index <= (sbEnd-sbStart);Index++)
  {
    pPalEntry->peRed = *(pOldPalette + (Index*3));
          pPalEntry->peGreen = *(pOldPalette + (Index*3) + 1);
          pPalEntry->peBlue = *(pOldPalette + (Index*3) + 2);
    pPalEntry->peFlags = 0;
          pPalEntry++;
  }
  return pInitEntry;
} */

//*****************************************************************************
//
// SetFontPalette - Sets the Palette
//
// Parameter List : pointer to the base structure
//                  new pixel depth
//                  new Palette size
//                  pointer to palette data
//
// Return Value  BOOLEAN
//
// Modification History :
// Dec 15th 1996 -> modified for use by Wizardry
//
//*****************************************************************************

/*BOOLEAN SetFontPalette(FontBase *pFontBase, uint16_t siPixelDepth, struct SGPPaletteEntry
*pPalData)
{
        Assert(pFontBase != NULL);
        Assert(pPalData != NULL);
        MemFree(pFontBase->pPalette);

        // assign the new palette to the Base structure
        pFontBase->pPalette = pPalData;
        pFontBase->siPixelDepth = siPixelDepth;
        return TRUE;
}	*/

//*****************************************************************************
//
// SetFont16BitData - Sets the font structure to hold 16 bit data
//
// Parameter List : pointer to the base structure
//                  pointer to new 16 bit data
//
// Return Value  BOOLEAN
//
// Modification History :
// Dec 15th 1996 -> modified for use by Wizardry
//
//*****************************************************************************

/*BOOLEAN SetFont16BitData(FontBase *pFontBase, uint16_t *pData16)
{
        Assert(pFontBase != NULL);
        Assert(pData16 != NULL);
        MemFree(pFontBase->pPixData16);
        pFontBase->pPixData16 = pData16;
        return TRUE;
}	*/

//*****************************************************************************
//
// Blt8Imageto16Dest
//
// Parameter List : Start offset
//                  End Offset
//                  Dest x, y
//                  Font Width
//                  Pointer to Base structure
//                  Pointer to destination buffer
//                  Destination Pitch
//                  Height of Each element
//
// Return Value  : BOOLEAN
//
// Modification History :
// Nov 26th 1996 -> modified for use by Wizardry
//
//*****************************************************************************

/*BOOLEAN Blt8Imageto16Dest(uint32_t uiOffStart, uint32_t uiOffEnd, uint16_t siX, uint16_t siY,
uint32_t uiWidth, FontBase *pFontBase, uint8_t *pFrameBuffer, uint16_t siDestPitch, uint16_t
siHeightEach)
{
        uint8_t  *pTrav;
        uint16_t *pFrameTrav;
        uint16_t *p16BPPPalette;
        uint16_t  usEffectiveWidth;
        uint32_t  uiFrameCount;
        uint8_t   amount;
        uint32_t  row, count;
    uint16_t  modamount, divamount;
        uint32_t trace,modtrace;
        uint8_t sub=0;


        pTrav = pFontBase->pPixData8;
        pFrameTrav = (uint16_t *)pFrameBuffer;
        p16BPPPalette = pFontBase->pPalet16;
    trace = 0;
        modtrace = 0;
        // effective width is pitch/2 as 16 bits per pixel
        usEffectiveWidth = (uint16_t)(siDestPitch / 2);
        uiFrameCount = siY*usEffectiveWidth + siX;
        trace += uiFrameCount;
        modtrace = trace % 640;
        pFrameTrav += uiFrameCount;
        pTrav += uiOffStart;

        count = 0;
        row = 0;
        amount = 0;
        while (count < (uiOffEnd-uiOffStart))
        {
          amount = 0;
    if (*pTrav == ID_BLACK)
          {
                  pTrav++;
                  count++;
                  amount = *pTrav;
                  modamount = (uint8_t)(amount) % (uint8_t) uiWidth;
                  divamount = (uint8_t)(amount) / (uint8_t) uiWidth;
      if ((divamount == 0) && ((row+amount) < (uint16_t)uiWidth))
                  {
                          pFrameTrav += amount;
                          trace += amount;
                  modtrace = trace % 640;
                          row += amount;
                          row++;
                  }
                  else
                  {
        if (((row+amount) >= (uint16_t)uiWidth) && (divamount ==0))
                    {
                pFrameTrav -= row;
                                trace -= row;
                    modtrace = trace % 640;
                            row = amount-((uint16_t)uiWidth-row);
                            pFrameTrav += usEffectiveWidth+row;
                                trace += usEffectiveWidth+row;
                                modtrace = trace % 640;
                            row++;
                    }
                    else
                    {
                            pFrameTrav += (divamount*usEffectiveWidth);
                                trace += (divamount*usEffectiveWidth);
                                modtrace = trace % 640;
                                if(row+modamount > uiWidth)
                                {
                                        sub = (uint8_t)((row+modamount) % uiWidth);
                                        pFrameTrav -= row;
                                    trace -= row;
                                    modtrace = trace % 640;
                                        pFrameTrav += usEffectiveWidth+sub;
                                    trace += usEffectiveWidth + sub;
                                    modtrace = trace % 640;
                                row = sub;
                                row++;
                                }else
                                {
                                        pFrameTrav += modamount;
                                    trace += modamount;
                                    modtrace = trace % 640;
                                row = modamount;
                                row++;
                                }
                    }
      }
          } else
          {
                  if(row >= uiWidth)
                  {
            pFrameTrav += (usEffectiveWidth-uiWidth);
                        trace += (usEffectiveWidth-uiWidth);
                        modtrace = trace % 640;
            *pFrameTrav = p16BPPPalette[*pTrav];
                    row = 1;
                  }
                  else
                  {
            *pFrameTrav = p16BPPPalette[*pTrav];
                    row++;
                  }
    }

    pFrameTrav++;
        trace++;
        modtrace = trace % 640;
    pTrav++;
    count++;
        }

        return TRUE;
}	*/

//*****************************************************************************
//
// Blt8Imageto8Dest
//
// Parameter List : Start offset
//                  End Offset
//                  Dest x, y
//                  Font Width
//                  Pointer to Base structure
//                  Pointer to destination buffer
//                  Destination Pitch
//                  Height of Each element
//
// Return Value  : BOOLEAN
//
// Modification History :
// Nov 26th 1996 -> modified for use by Wizardry
//
//*****************************************************************************

/*BOOLEAN Blt8Imageto8Dest(uint32_t uiOffStart, uint32_t uiOffEnd, uint16_t siX, uint16_t siY,
uint32_t uiWidth, FontBase *pFontBase, uint8_t *pFrameBuffer, uint16_t siDestPitch, uint16_t
siHeightEach)
{
        uint8_t  *pTrav;
        uint32_t  uiFrameCount;
        uint8_t  *pFrameTrav;
        uint8_t   amount;
        uint32_t  row,count;
  uint16_t  modamount,divamount;

        DebugMsg(TOPIC_FONT_HANDLER, DBG_ERROR, "Blitting 8 to 8");
  Assert(pFontBase != NULL);
        Assert(pFrameBuffer != NULL);

        // get the pointers
        pTrav = pFontBase->pPixData8;
        pFrameTrav = (uint8_t *)pFrameBuffer;

        uiFrameCount = siY*siDestPitch + siX;
        pFrameTrav +=uiFrameCount;
        pTrav += uiOffStart;
  // perform blitting

        count=0;
        row = 0;
        amount = 0;
        while (count < (uiOffEnd-uiOffStart))
        {
          amount = 0;
    if (*pTrav == ID_BLACK)
          {
                  pTrav++;
                  count++;
                  amount = *pTrav;
                  modamount = amount % (uint8_t) uiWidth;
                  divamount = amount / (uint8_t) uiWidth;
      if ((divamount == 0) && ((row+amount) < (uint16_t)uiWidth))
                  {
                          pFrameTrav += amount;
                          row += amount;
                          row++;
                  }
                  else
                  {
        if (((row+amount) >= (uint16_t)uiWidth) && (divamount ==0))
                    {
          pFrameTrav -= row;
                            row = amount-((uint16_t)uiWidth-row);
                            pFrameTrav += siDestPitch+row;
                            row++;
                    }
                    else
                    {
                            pFrameTrav += (divamount*siDestPitch)+modamount;
                            row = modamount;
                            row++;
                    }
      }
          } else
          {
                  if (row >= uiWidth)
                  {
        pFrameTrav += (siDestPitch-uiWidth);
       *pFrameTrav = *pTrav;
                    row = 1;
                  }
                  else
                  {
       *pFrameTrav = *pTrav;
                    row++;
                  }
    }

    pFrameTrav++;
    pTrav++;
                count++;
        }

        return TRUE;
} */

//*****************************************************************************
//
// Blt16Imageto16Dest
//
// Parameter List : Start offset
//                  End Offset
//                  Dest x, y
//                  Font Width
//                  Pointer to Base structure
//                  Pointer to destination buffer
//                  Destination Pitch
//                  Height of Each element
//
// Return Value  : BOOLEAN
//
// Modification History :
// Nov 26th 1996 -> modified for use by Wizardry
//
//*****************************************************************************
/*BOOLEAN Blt16Imageto16Dest(uint32_t uiOffStart, uint32_t uiOffEnd, uint16_t siX, uint16_t siY,
uint32_t uiWidth, FontBase *pFontBase, uint8_t *pFrameBuffer, uint16_t siDestPitch, uint16_t
siHeightEach)
{
        uint16_t *pTrav;
        uint32_t  uiFrameCount;
        uint16_t *pFrameTrav;
        uint16_t  amount;
        uint32_t  row,count;
  uint16_t  modamount,divamount;
        uint16_t  usEffectiveWidth;

        DebugMsg(TOPIC_FONT_HANDLER, DBG_ERROR, "Blitting 16 to 16");
  Assert(pFontBase != NULL);
        Assert(pFrameBuffer != NULL);

        //get the pointers
        pTrav = pFontBase->pPixData16;
        pFrameTrav = (uint16_t *)pFrameBuffer;

        // effective width is pitch/2 as 16 bits per pixel
        usEffectiveWidth = (uint16_t)(siDestPitch / 2);
        uiFrameCount = siY*usEffectiveWidth + siX;
        pFrameTrav +=uiFrameCount;
        pTrav += uiOffStart;

        count=0;
        row = 0;
        amount = 0;
        while (count < (uiOffEnd-uiOffStart))
        {
    amount = 0;
    if (*pTrav == ID_BLACK)
          {
                  pTrav++;
                  count++;
                  amount = *pTrav;
                  modamount = amount % (uint8_t) uiWidth;
                  divamount = amount / (uint8_t) uiWidth;
      if ((divamount == 0) && ((row+amount) < (uint16_t)uiWidth))
                  {
                          pFrameTrav += amount;
                          row += amount;
                          row++;
                  }
                  else
                  {
        if (((row+amount) >= (uint16_t)uiWidth) && (divamount ==0))
                    {
          pFrameTrav -= row;
                            row = amount-((uint16_t)uiWidth-row);
                            pFrameTrav += usEffectiveWidth+row;
                            row++;
                    }
                    else
                    {
                            pFrameTrav += (divamount*usEffectiveWidth)+modamount;
                            row = modamount;
                            row++;
                    }
      }
          } else
          {
                  if(row >= uiWidth)
                  {
        pFrameTrav += (usEffectiveWidth-uiWidth);
        *pFrameTrav = *pTrav;
                    row = 1;
                  }
                  else
                  {
        *pFrameTrav = *pTrav;
                    row++;
                  }
    }

    pFrameTrav++;
                pTrav++;
                count++;
        }

        return TRUE;
}	*/

//*****************************************************************************
//
// GetOffset
//
// Parameter List : Given the index, gets the corresponding offset
//
// Return Value  : offset
//
// Modification History :
// Nov 26th 1996 -> modified for use by Wizardry
//
//*****************************************************************************

/*uint32_t GetOffset(FontBase *pFontBase, int16_t ssIndex)
{
  FontObject *pTrav;
  uint16_t siCount=0;

  Assert(pFontBase != NULL);
  // gets the offset based on the index
  if (((uint32_t)ssIndex > pFontBase->uiTotalElements) || (ssIndex < 0))
  {
          DebugMsg(TOPIC_FONT_HANDLER, DBG_ERROR, "Incorrect index value passed");
    return 0;
  }
  pTrav = pFontBase->pFontObject;
  while (siCount != ssIndex)
  {
          siCount++;
          pTrav++;
  }

  return pTrav->uiFontOffset;
} */

//*****************************************************************************
//
// GetOffLen
//
// Parameter List : Given the index, gets the corresponding offset
// length which is the number of compressed pixels
//
// Return Value  : offset
//
// Modification History :
// Nov 26th 1996 -> modified for use by Wizardry
//
//*****************************************************************************
/*uint32_t GetOffLen(FontBase *pFontBase, int16_t ssIndex)
{
  FontObject *pTrav;
  uint16_t siCount=0;

  Assert(pFontBase != NULL);
  // gets the offset based on the index
  if (((uint32_t)ssIndex > pFontBase->uiTotalElements) || (ssIndex < 0))
  {
          DebugMsg(TOPIC_FONT_HANDLER, DBG_ERROR, "Incorrect index value passed");
    return 0;
  }
  pTrav = pFontBase->pFontObject;
  while(siCount != ssIndex)
  {
          siCount++;
          pTrav++;
  }

  return pTrav->uiOffLen;
} */
