#include "Utils/Utilities.h"

#include <stdio.h>
#include <string.h>
#include <time.h>

#include "SGP/Debug.h"
#include "SGP/FileMan.h"
#include "SGP/Font.h"
#include "SGP/HImage.h"
#include "SGP/VObject.h"
#include "SGP/VSurface.h"
#include "SGP/Video.h"
#include "SGP/WCheck.h"
#include "Tactical/OverheadTypes.h"
#include "Tactical/SoldierControl.h"
#include "Utils/FontControl.h"

void FilenameForBPP(STR pFilename, STR pDestination) { strcpy(pDestination, pFilename); }

BOOLEAN CreateSGPPaletteFromCOLFile(struct SGPPaletteEntry *pPalette, SGPFILENAME ColFile) {
  HWFILE hFileHandle;
  BYTE bColHeader[8];
  uint32_t cnt;

  // See if files exists, if not, return error
  if (!FileMan_Exists(ColFile)) {
    // Return FALSE w/ debug
    DebugMsg(TOPIC_JA2, DBG_LEVEL_3, "Cannot find COL file");
    return (FALSE);
  }

  // Open and read in the file
  if ((hFileHandle = FileMan_Open(ColFile, FILE_ACCESS_READ, FALSE)) == 0) {
    // Return FALSE w/ debug
    DebugMsg(TOPIC_JA2, DBG_LEVEL_3, "Cannot open COL file");
    return (FALSE);
  }

  // Skip header
  FileMan_Read(hFileHandle, bColHeader, sizeof(bColHeader), NULL);

  // Read in a palette entry at a time
  for (cnt = 0; cnt < 256; cnt++) {
    FileMan_Read(hFileHandle, &pPalette[cnt].peRed, sizeof(uint8_t), NULL);
    FileMan_Read(hFileHandle, &pPalette[cnt].peGreen, sizeof(uint8_t), NULL);
    FileMan_Read(hFileHandle, &pPalette[cnt].peBlue, sizeof(uint8_t), NULL);
  }

  // Close file
  FileMan_Close(hFileHandle);

  return (TRUE);
}

BOOLEAN DisplayPaletteRep(PaletteRepID aPalRep, uint8_t ubXPos, uint8_t ubYPos,
                          uint32_t uiDestSurface) {
  uint16_t us16BPPColor;
  uint32_t cnt1;
  uint8_t ubSize;
  int16_t sTLX, sTLY, sBRX, sBRY;
  uint8_t ubPaletteRep;

  // Create 16BPP Palette
  CHECKF(GetPaletteRepIndexFromID(aPalRep, &ubPaletteRep));

  SetFont(LARGEFONT1);

  ubSize = gpPalRep[ubPaletteRep].ubPaletteSize;

  for (cnt1 = 0; cnt1 < ubSize; cnt1++) {
    sTLX = ubXPos + (uint16_t)((cnt1 % 16) * 20);
    sTLY = ubYPos + (uint16_t)((cnt1 / 16) * 20);
    sBRX = sTLX + 20;
    sBRY = sTLY + 20;

    us16BPPColor =
        Get16BPPColor(FROMRGB(gpPalRep[ubPaletteRep].r[cnt1], gpPalRep[ubPaletteRep].g[cnt1],
                              gpPalRep[ubPaletteRep].b[cnt1]));

    ColorFillVideoSurfaceArea(uiDestSurface, sTLX, sTLY, sBRX, sBRY, us16BPPColor);
  }

  gprintf(ubXPos + (16 * 20), ubYPos, L"%S", gpPalRep[ubPaletteRep].ID);

  return (TRUE);
}

BOOLEAN WrapString(STR16 pStr, STR16 pStr2, size_t buf2Size, uint16_t usWidth, int32_t uiFont) {
  uint32_t Cur, uiLet, uiNewLet, uiHyphenLet;
  CHAR16 *curletter, transletter;
  BOOLEAN fLineSplit = FALSE;
  struct VObject *hFont;

  // CHECK FOR WRAP
  Cur = 0;
  uiLet = 0;
  curletter = pStr;

  // GET FONT
  hFont = GetFontObject(uiFont);

  // LOOP FORWARDS AND COUNT
  while ((*curletter) != 0) {
    transletter = GetIndex(*curletter);
    Cur += GetWidth(hFont, transletter);

    if (Cur > usWidth) {
      // We are here, loop backwards to find a space
      // Generate second string, and exit upon completion.
      uiHyphenLet = uiLet;  // Save the hyphen location as it won't change.
      uiNewLet = uiLet;
      while ((*curletter) != 0) {
        if ((*curletter) == 32) {
          // Split Line!
          fLineSplit = TRUE;

          pStr[uiNewLet] = (int16_t)'\0';

          wcscpy(pStr2, &(pStr[uiNewLet + 1]));
        }

        if (fLineSplit) break;

        uiNewLet--;
        curletter--;
      }
      if (!fLineSplit) {
        // We completed the check for a space, but failed, so use the hyphen method.
        swprintf(pStr2, buf2Size, L"-%s", &(pStr[uiHyphenLet]));
        pStr[uiHyphenLet] = (int16_t)'\0';
        fLineSplit = TRUE;  // hyphen method
        break;
      }
    }

    //		if ( fLineSplit )
    //			break;

    uiLet++;
    curletter++;
  }

  return (fLineSplit);
}
