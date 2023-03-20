#include "Utils/Utilities.h"

#include <stdio.h>
#include <string.h>
#include <time.h>

#include "SGP/Debug.h"
#include "SGP/Font.h"
#include "SGP/HImage.h"
#include "SGP/VObject.h"
#include "SGP/VSurface.h"
#include "SGP/Video.h"
#include "SGP/WCheck.h"
#include "Tactical/OverheadTypes.h"
#include "Tactical/SoldierControl.h"
#include "Utils/FontControl.h"
#include "rust_fileman.h"

void FilenameForBPP(STR pFilename, STR pDestination) { strcpy(pDestination, pFilename); }

BOOLEAN CreateSGPPaletteFromCOLFile(struct SGPPaletteEntry *pPalette, SGPFILENAME ColFile) {
  FileID hFileHandle = FILE_ID_ERR;
  BYTE bColHeader[8];
  UINT32 cnt;

  // See if files exists, if not, return error
  if (!File_Exists(ColFile)) {
    // Return FALSE w/ debug
    DebugMsg(TOPIC_JA2, DBG_INFO, "Cannot find COL file");
    return (FALSE);
  }

  // Open and read in the file
  if ((hFileHandle = File_OpenForReading(ColFile)) == 0) {
    // Return FALSE w/ debug
    DebugMsg(TOPIC_JA2, DBG_INFO, "Cannot open COL file");
    return (FALSE);
  }

  // Skip header
  File_Read(hFileHandle, bColHeader, sizeof(bColHeader), NULL);

  // Read in a palette entry at a time
  for (cnt = 0; cnt < 256; cnt++) {
    File_Read(hFileHandle, &pPalette[cnt].peRed, sizeof(UINT8), NULL);
    File_Read(hFileHandle, &pPalette[cnt].peGreen, sizeof(UINT8), NULL);
    File_Read(hFileHandle, &pPalette[cnt].peBlue, sizeof(UINT8), NULL);
  }

  // Close file
  File_Close(hFileHandle);

  return (TRUE);
}

BOOLEAN DisplayPaletteRep(PaletteRepID aPalRep, UINT8 ubXPos, UINT8 ubYPos, UINT32 uiDestSurface) {
  UINT16 us16BPPColor;
  UINT32 cnt1;
  UINT8 ubSize;
  INT16 sTLX, sTLY, sBRX, sBRY;
  UINT8 ubPaletteRep;

  // Create 16BPP Palette
  CHECKF(GetPaletteRepIndexFromID(aPalRep, &ubPaletteRep));

  SetFont(LARGEFONT1);

  ubSize = gpPalRep[ubPaletteRep].ubPaletteSize;

  for (cnt1 = 0; cnt1 < ubSize; cnt1++) {
    sTLX = ubXPos + (UINT16)((cnt1 % 16) * 20);
    sTLY = ubYPos + (UINT16)((cnt1 / 16) * 20);
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

BOOLEAN WrapString(STR16 pStr, STR16 pStr2, size_t buf2Size, UINT16 usWidth, INT32 uiFont) {
  UINT32 Cur, uiLet, uiNewLet, uiHyphenLet;
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

          pStr[uiNewLet] = (INT16)'\0';

          wcscpy(pStr2, &(pStr[uiNewLet + 1]));
        }

        if (fLineSplit) break;

        uiNewLet--;
        curletter--;
      }
      if (!fLineSplit) {
        // We completed the check for a space, but failed, so use the hyphen method.
        swprintf(pStr2, buf2Size, L"-%s", &(pStr[uiHyphenLet]));
        pStr[uiHyphenLet] = (INT16)'\0';
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
