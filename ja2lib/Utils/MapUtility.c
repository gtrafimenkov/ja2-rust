#include "Utils/MapUtility.h"

#include "Editor/LoadScreen.h"
#include "Globals.h"
#include "SGP/English.h"
#include "SGP/FileMan.h"
#include "SGP/Line.h"
#include "SGP/Types.h"
#include "SGP/VObject.h"
#include "SGP/VObjectBlitters.h"
#include "SGP/VSurface.h"
#include "SGP/Video.h"
#include "Screens.h"
#include "Tactical/MapInformation.h"
#include "Tactical/Overhead.h"
#include "TileEngine/OverheadMap.h"
#include "TileEngine/RadarScreen.h"
#include "TileEngine/WorldDat.h"
#include "TileEngine/WorldDef.h"
#include "Utils/FontControl.h"
#include "Utils/STIConvert.h"
#include "platform.h"

#ifdef JA2EDITOR

#include "Utils/QuantizeWrap.h"

#define MINIMAP_X_SIZE 88
#define MINIMAP_Y_SIZE 44

#define WINDOW_SIZE 2

FLOAT gdXStep, gdYStep;
UINT32 giMiniMap, gi8BitMiniMap;
struct VSurface *ghvSurface;

extern BOOLEAN gfOverheadMapDirty;

// Utililty file for sub-sampling/creating our radar screen maps
// Loops though our maps directory and reads all .map files, subsamples an area, color
// quantizes it into an 8-bit image ans writes it to an sti file in radarmaps.

typedef struct {
  INT8 r;
  INT8 g;
  INT8 b;

} RGBValues;

UINT32 MapUtilScreenInit() { return (TRUE); }

UINT32 MapUtilScreenHandle() {
  static INT16 fNewMap = TRUE;
  static INT16 sFileNum = 0;
  InputAtom InputEvent;
  struct GetFile FileInfo;
  static struct FileDialogList *FListNode;
  static INT16 sFiles = 0, sCurFile = 0;
  static struct FileDialogList *FileList = NULL;
  CHAR8 zFilename[260], zFilename2[260];
  VSURFACE_DESC vs_desc;
  UINT16 usWidth;
  UINT16 usHeight;
  UINT8 ubBitDepth;
  UINT32 uiDestPitchBYTES, uiSrcPitchBYTES;
  UINT16 *pDestBuf, *pSrcBuf;
  UINT8 *pDataPtr;

  static UINT8 *p24BitDest = NULL;
  static RGBValues *p24BitValues = NULL;

  UINT32 uiRGBColor;

  UINT32 bR, bG, bB, bAvR, bAvG, bAvB;
  INT16 s16BPPSrc, sDest16BPPColor;
  INT32 cnt;

  INT16 sX1, sX2, sY1, sY2, sTop, sBottom, sLeft, sRight;

  FLOAT dX, dY, dStartX, dStartY;
  INT32 iX, iY, iSubX1, iSubY1, iSubX2, iSubY2, iWindowX, iWindowY, iCount;
  struct SGPPaletteEntry pPalette[256];

  sDest16BPPColor = -1;
  bAvR = bAvG = bAvB = 0;

  // Zero out area!
  ColorFillVideoSurfaceArea(FRAME_BUFFER, 0, 0, (INT16)(640), (INT16)(480),
                            Get16BPPColor(FROMRGB(0, 0, 0)));

  if (fNewMap) {
    fNewMap = FALSE;

    // Create render buffer
    GetCurrentVideoSettings(&usWidth, &usHeight, &ubBitDepth);
    vs_desc.fCreateFlags = VSURFACE_CREATE_DEFAULT | VSURFACE_SYSTEM_MEM_USAGE;
    vs_desc.usWidth = 88;
    vs_desc.usHeight = 44;
    vs_desc.ubBitDepth = ubBitDepth;

    if (AddVideoSurface(&vs_desc, &giMiniMap) == FALSE) {
      return (ERROR_SCREEN);
    }

    // USING BRET's STUFF FOR LOOPING FILES/CREATING LIST, hence AddToFDlgList.....
    if (Plat_GetFileFirst("MAPS\\*.dat", &FileInfo)) {
      FileList = AddToFDlgList(FileList, &FileInfo);
      sFiles++;
      while (Plat_GetFileNext(&FileInfo)) {
        FileList = AddToFDlgList(FileList, &FileInfo);
        sFiles++;
      }
      Plat_GetFileClose(&FileInfo);
    }

    FListNode = FileList;

    // Allocate 24 bit Surface
    p24BitValues = (RGBValues *)MemAlloc(MINIMAP_X_SIZE * MINIMAP_Y_SIZE * sizeof(RGBValues));
    p24BitDest = (UINT8 *)p24BitValues;

    // Allocate 8-bit surface
    vs_desc.fCreateFlags = VSURFACE_CREATE_DEFAULT | VSURFACE_SYSTEM_MEM_USAGE;
    vs_desc.usWidth = 88;
    vs_desc.usHeight = 44;
    vs_desc.ubBitDepth = 8;

    if (AddVideoSurface(&vs_desc, &gi8BitMiniMap) == FALSE) {
      return (ERROR_SCREEN);
    }
    GetVideoSurface(&ghvSurface, gi8BitMiniMap);
  }

  // OK, we are here, now loop through files
  if (sCurFile == sFiles || FListNode == NULL) {
    gfProgramIsRunning = FALSE;
    return (MAPUTILITY_SCREEN);
  }

  sprintf(zFilename, "%s", FListNode->FileInfo.zFileName);

  // OK, load maps and do overhead shrinkage of them...
  if (!LoadWorld(zFilename)) {
    return (ERROR_SCREEN);
  }

  // Render small map
  InitNewOverheadDB((UINT8)giCurrentTilesetID);

  gfOverheadMapDirty = TRUE;

  RenderOverheadMap(0, (WORLD_COLS / 2), 0, 0, 640, 320, TRUE);

  TrashOverheadMap();

  // OK, NOW PROCESS OVERHEAD MAP ( SHOUIDL BE ON THE FRAMEBUFFER )
  gdXStep = (float)640 / (float)88;
  gdYStep = (float)320 / (float)44;
  dStartX = dStartY = 0;

  // Adjust if we are using a restricted map...
  if (gMapInformation.ubRestrictedScrollID != 0) {
    CalculateRestrictedMapCoords(NORTH, &sX1, &sY1, &sX2, &sTop, 640, 320);
    CalculateRestrictedMapCoords(SOUTH, &sX1, &sBottom, &sX2, &sY2, 640, 320);
    CalculateRestrictedMapCoords(WEST, &sX1, &sY1, &sLeft, &sY2, 640, 320);
    CalculateRestrictedMapCoords(EAST, &sRight, &sY1, &sX2, &sY2, 640, 320);

    gdXStep = (float)(sRight - sLeft) / (float)88;
    gdYStep = (float)(sBottom - sTop) / (float)44;

    dStartX = sLeft;
    dStartY = sTop;
  }

  // LOCK BUFFERS

  dX = dStartX;
  dY = dStartY;

  pDestBuf = (UINT16 *)LockVideoSurface(giMiniMap, &uiDestPitchBYTES);
  pSrcBuf = (UINT16 *)LockVideoSurface(FRAME_BUFFER, &uiSrcPitchBYTES);

  for (iX = 0; iX < 88; iX++) {
    dY = dStartY;

    for (iY = 0; iY < 44; iY++) {
      // OK, AVERAGE PIXELS
      iSubX1 = (INT32)dX - WINDOW_SIZE;

      iSubX2 = (INT32)dX + WINDOW_SIZE;

      iSubY1 = (INT32)dY - WINDOW_SIZE;

      iSubY2 = (INT32)dY + WINDOW_SIZE;

      iCount = 0;
      bR = bG = bB = 0;

      for (iWindowX = iSubX1; iWindowX < iSubX2; iWindowX++) {
        for (iWindowY = iSubY1; iWindowY < iSubY2; iWindowY++) {
          if (iWindowX >= 0 && iWindowX < 640 && iWindowY >= 0 && iWindowY < 320) {
            s16BPPSrc = pSrcBuf[(iWindowY * (uiSrcPitchBYTES / 2)) + iWindowX];

            uiRGBColor = GetRGBColor(s16BPPSrc);

            bR += SGPGetRValue(uiRGBColor);
            bG += SGPGetGValue(uiRGBColor);
            bB += SGPGetBValue(uiRGBColor);

            // Average!
            iCount++;
          }
        }
      }

      if (iCount > 0) {
        bAvR = bR / (UINT8)iCount;
        bAvG = bG / (UINT8)iCount;
        bAvB = bB / (UINT8)iCount;

        sDest16BPPColor = Get16BPPColor(FROMRGB(bAvR, bAvG, bAvB));
      }

      // Write into dest!
      pDestBuf[(iY * (uiDestPitchBYTES / 2)) + iX] = sDest16BPPColor;

      p24BitValues[(iY * (uiDestPitchBYTES / 2)) + iX].r = (UINT8)bAvR;
      p24BitValues[(iY * (uiDestPitchBYTES / 2)) + iX].g = (UINT8)bAvG;
      p24BitValues[(iY * (uiDestPitchBYTES / 2)) + iX].b = (UINT8)bAvB;

      // Increment
      dY += gdYStep;
    }

    // Increment
    dX += gdXStep;
  }

  UnLockVideoSurface(giMiniMap);
  UnLockVideoSurface(FRAME_BUFFER);

  // RENDER!
  BltVideoSurface(FRAME_BUFFER, giMiniMap, 0, 20, 360, VS_BLT_FAST | VS_BLT_USECOLORKEY, NULL);

  // QUantize!
  pDataPtr = (UINT8 *)LockVideoSurface(gi8BitMiniMap, &uiSrcPitchBYTES);
  pDestBuf = (UINT16 *)LockVideoSurface(FRAME_BUFFER, &uiDestPitchBYTES);
  QuantizeImage(pDataPtr, p24BitDest, MINIMAP_X_SIZE, MINIMAP_Y_SIZE, pPalette);
  SetVideoSurfacePalette(ghvSurface, pPalette);
  // Blit!
  Blt8BPPDataTo16BPPBuffer(pDestBuf, uiDestPitchBYTES, ghvSurface, pDataPtr, 300, 360);

  // Write palette!
  {
    INT32 cnt;
    INT32 sX = 0, sY = 420;
    UINT16 usLineColor;

    SetClippingRegionAndImageWidth(uiDestPitchBYTES, 0, 0, 640, 480);

    for (cnt = 0; cnt < 256; cnt++) {
      usLineColor =
          Get16BPPColor(FROMRGB(pPalette[cnt].peRed, pPalette[cnt].peGreen, pPalette[cnt].peBlue));
      RectangleDraw(TRUE, sX, sY, sX, (INT16)(sY + 10), usLineColor, (UINT8 *)pDestBuf);
      sX++;
      RectangleDraw(TRUE, sX, sY, sX, (INT16)(sY + 10), usLineColor, (UINT8 *)pDestBuf);
      sX++;
    }
  }

  UnLockVideoSurface(FRAME_BUFFER);

  // Remove extension
  for (cnt = strlen(zFilename) - 1; cnt >= 0; cnt--) {
    if (zFilename[cnt] == '.') {
      zFilename[cnt] = '\0';
    }
  }

  sprintf(zFilename2, "RADARMAPS\\%s.STI", zFilename);
  WriteSTIFile(pDataPtr, pPalette, MINIMAP_X_SIZE, MINIMAP_Y_SIZE, zFilename2,
               CONVERT_ETRLE_COMPRESS, 0);

  UnLockVideoSurface(gi8BitMiniMap);

  SetFont(TINYFONT1);
  SetFontBackground(FONT_MCOLOR_BLACK);
  SetFontForeground(FONT_MCOLOR_DKGRAY);
  mprintf(10, 340, L"Writing radar image %S", zFilename2);

  mprintf(10, 350, L"Using tileset %s", gTilesets[giCurrentTilesetID].zName);

  InvalidateScreen();

  while (DequeueEvent(&InputEvent) == TRUE) {
    if ((InputEvent.usEvent == KEY_DOWN) && (InputEvent.usParam == ESC)) {  // Exit the program
      gfProgramIsRunning = FALSE;
    }
  }

  // Set next
  FListNode = FListNode->pNext;
  sCurFile++;

  return (MAPUTILITY_SCREEN);
}

UINT32 MapUtilScreenShutdown() { return (TRUE); }

#else  // non-editor version

#include "SGP/Types.h"
#include "ScreenIDs.h"

UINT32 MapUtilScreenInit() { return (TRUE); }

UINT32 MapUtilScreenHandle() {
  // If this screen ever gets set, then this is a bad thing -- endless loop
  return (ERROR_SCREEN);
}

UINT32 MapUtilScreenShutdown() { return (TRUE); }

#endif
