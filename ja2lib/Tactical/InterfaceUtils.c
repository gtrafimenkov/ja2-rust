#include "Tactical/InterfaceUtils.h"

#include <stdarg.h>
#include <stdio.h>

#include "JAScreens.h"
#include "SGP/HImage.h"
#include "SGP/Line.h"
#include "SGP/Types.h"
#include "SGP/VObject.h"
#include "SGP/VSurface.h"
#include "SGP/Video.h"
#include "SGP/WCheck.h"
#include "ScreenIDs.h"
#include "Soldier.h"
#include "StrUtils.h"
#include "Tactical/Faces.h"
#include "Tactical/Interface.h"
#include "Tactical/Overhead.h"
#include "Tactical/SoldierControl.h"
#include "Tactical/SoldierMacros.h"
#include "Tactical/Vehicles.h"
#include "Tactical/Weapons.h"
#include "TileEngine/RenderDirty.h"
#include "TileEngine/SysUtil.h"
#include "UI.h"

#define LIFE_BAR_SHADOW FROMRGB(108, 12, 12)
#define LIFE_BAR FROMRGB(200, 0, 0)
#define BANDAGE_BAR_SHADOW FROMRGB(156, 60, 60)
#define BANDAGE_BAR FROMRGB(222, 132, 132)
#define BLEEDING_BAR_SHADOW FROMRGB(128, 128, 60)
#define BLEEDING_BAR FROMRGB(240, 240, 20)
#define CURR_BREATH_BAR_SHADOW FROMRGB(8, 12, 118)  // the MAX max breatth, always at 100%
#define CURR_BREATH_BAR FROMRGB(8, 12, 160)
#define CURR_MAX_BREATH FROMRGB(0, 0, 0)  // the current max breath, black
#define CURR_MAX_BREATH_SHADOW FROMRGB(0, 0, 0)
#define MORALE_BAR_SHADOW FROMRGB(8, 112, 12)
#define MORALE_BAR FROMRGB(8, 180, 12)
#define BREATH_BAR_SHADOW FROMRGB(60, 108, 108)  // the lt blue current breath
#define BREATH_BAR FROMRGB(113, 178, 218)
#define BREATH_BAR_SHAD_BACK FROMRGB(1, 1, 1)
#define FACE_WIDTH 48
#define FACE_HEIGHT 43

// backgrounds for breath max background
extern uint32_t guiBrownBackgroundForTeamPanel;
extern uint32_t guiGoldBackgroundForTeamPanel;

// selected grunt
extern uint16_t gusSelectedSoldier;

// car portraits
enum {
  ELDORADO_PORTRAIT = 0,
  HUMMER_PORTRAIT,
  ICE_CREAM_TRUCT_PORTRAIT,
  JEEP_PORTRAIT,
  NUMBER_CAR_PORTRAITS,
};

// the ids for the car portraits
int32_t giCarPortraits[4] = {-1, -1, -1, -1};

// the car portrait file names
char *pbCarPortraitFileNames[] = {
    "INTERFACE\\eldorado.sti",
    "INTERFACE\\Hummer.sti",
    "INTERFACE\\ice Cream Truck.sti",
    "INTERFACE\\Jeep.sti",
};

// load int he portraits for the car faces that will be use in mapscreen
BOOLEAN LoadCarPortraitValues(void) {
  int32_t iCounter = 0;
  VOBJECT_DESC VObjectDesc;

  if (giCarPortraits[0] != -1) {
    return FALSE;
  }
  for (iCounter = 0; iCounter < NUMBER_CAR_PORTRAITS; iCounter++) {
    VObjectDesc.fCreateFlags = VOBJECT_CREATE_FROMFILE;
    strcopy(VObjectDesc.ImageFile, sizeof(VObjectDesc.ImageFile), pbCarPortraitFileNames[iCounter]);
    CHECKF(AddVideoObject(&VObjectDesc, &giCarPortraits[iCounter]));
  }
  return (TRUE);
}

// get rid of the images we loaded for the mapscreen car portraits
void UnLoadCarPortraits(void) {
  int32_t iCounter = 0;

  // car protraits loaded?
  if (giCarPortraits[0] == -1) {
    return;
  }
  for (iCounter = 0; iCounter < NUMBER_CAR_PORTRAITS; iCounter++) {
    DeleteVideoObjectFromIndex(giCarPortraits[iCounter]);
    giCarPortraits[iCounter] = -1;
  }
  return;
}

void DrawLifeUIBarEx(struct SOLDIERTYPE *pSoldier, int16_t sXPos, int16_t sYPos, int16_t sWidth,
                     int16_t sHeight, BOOLEAN fErase, uint32_t uiBuffer) {
  float dStart, dEnd, dPercentage;
  // uint16_t usLineColor;

  uint32_t uiDestPitchBYTES;
  uint8_t *pDestBuf;
  uint16_t usLineColor;
  int8_t bBandage;

  // Erase what was there
  if (fErase) {
    RestoreExternBackgroundRect(sXPos, (int16_t)(sYPos - sHeight), sWidth, (int16_t)(sHeight + 1));
  }

  if (pSoldier->bLife == 0) {
    // are they dead?
    return;
  }

  pDestBuf = LockVideoSurface(uiBuffer, &uiDestPitchBYTES);
  SetClippingRegionAndImageWidth(uiDestPitchBYTES, 0, 0, 640, 480);

  // FIRST DO MAX LIFE
  dPercentage = (float)pSoldier->bLife / (float)100;
  dEnd = dPercentage * sHeight;
  dStart = sYPos;

  usLineColor = Get16BPPColor(LIFE_BAR_SHADOW);
  RectangleDraw(TRUE, sXPos, (int32_t)dStart, sXPos, (int32_t)(dStart - dEnd), usLineColor,
                pDestBuf);

  usLineColor = Get16BPPColor(LIFE_BAR);
  RectangleDraw(TRUE, sXPos + 1, (int32_t)dStart, sXPos + 1, (int32_t)(dStart - dEnd), usLineColor,
                pDestBuf);

  usLineColor = Get16BPPColor(LIFE_BAR_SHADOW);
  RectangleDraw(TRUE, sXPos + 2, (int32_t)dStart, sXPos + 2, (int32_t)(dStart - dEnd), usLineColor,
                pDestBuf);

  // NOW DO BANDAGE

  // Calculate bandage
  bBandage = pSoldier->bLifeMax - pSoldier->bLife - pSoldier->bBleeding;

  if (bBandage) {
    dPercentage = (float)bBandage / (float)100;
    dStart = (float)(sYPos - dEnd);
    dEnd = (dPercentage * sHeight);

    usLineColor = Get16BPPColor(BANDAGE_BAR_SHADOW);
    RectangleDraw(TRUE, sXPos, (int32_t)dStart, sXPos, (int32_t)(dStart - dEnd), usLineColor,
                  pDestBuf);

    usLineColor = Get16BPPColor(BANDAGE_BAR);
    RectangleDraw(TRUE, sXPos + 1, (int32_t)dStart, sXPos + 1, (int32_t)(dStart - dEnd),
                  usLineColor, pDestBuf);

    usLineColor = Get16BPPColor(BANDAGE_BAR_SHADOW);
    RectangleDraw(TRUE, sXPos + 2, (int32_t)dStart, sXPos + 2, (int32_t)(dStart - dEnd),
                  usLineColor, pDestBuf);
  }

  // NOW DO BLEEDING
  if (pSoldier->bBleeding) {
    dPercentage = (float)pSoldier->bBleeding / (float)100;
    dStart = (float)(dStart - dEnd);
    dEnd = (dPercentage * sHeight);

    usLineColor = Get16BPPColor(BLEEDING_BAR_SHADOW);
    RectangleDraw(TRUE, sXPos, (int32_t)dStart, sXPos, (int32_t)(dStart - dEnd), usLineColor,
                  pDestBuf);

    usLineColor = Get16BPPColor(BLEEDING_BAR);
    RectangleDraw(TRUE, sXPos + 1, (int32_t)dStart, sXPos + 1, (int32_t)(dStart - dEnd),
                  usLineColor, pDestBuf);

    usLineColor = Get16BPPColor(BLEEDING_BAR_SHADOW);
    RectangleDraw(TRUE, sXPos + 2, (int32_t)dStart, sXPos + 2, (int32_t)(dStart - dEnd),
                  usLineColor, pDestBuf);
  }

  UnLockVideoSurface(uiBuffer);
}

void DrawBreathUIBarEx(struct SOLDIERTYPE *pSoldier, int16_t sXPos, int16_t sYPos, int16_t sWidth,
                       int16_t sHeight, BOOLEAN fErase, uint32_t uiBuffer) {
  float dStart, dEnd, dPercentage;
  // uint16_t usLineColor;

  uint32_t uiDestPitchBYTES;
  uint8_t *pDestBuf;
  uint16_t usLineColor;
  struct VObject *hHandle;

  // Erase what was there
  if (fErase) {
    RestoreExternBackgroundRect(sXPos, (int16_t)(sYPos - sHeight), sWidth, (int16_t)(sHeight + 1));
  }

  if (pSoldier->bLife == 0) {
    // are they dead?
    return;
  }

  //  DO MAX MAX BREATH
  dPercentage = 1.;
  dEnd = dPercentage * sHeight;
  dStart = sYPos;

  // brown guy
  GetVideoObject(&hHandle, guiBrownBackgroundForTeamPanel);

  // DO MAX BREATH
  if (!IsMapScreen_2()) {
    if (gusSelectedSoldier == GetSolID(pSoldier) && gTacticalStatus.ubCurrentTeam == OUR_TEAM &&
        OK_INTERRUPT_MERC(pSoldier)) {
      // gold, the second entry in the .sti
      BltVideoObject(uiBuffer, hHandle, 1, sXPos, (int16_t)(sYPos - sHeight),
                     VO_BLT_SRCTRANSPARENCY, NULL);

    } else {
      // brown, first entry
      BltVideoObject(uiBuffer, hHandle, 0, sXPos, (int16_t)(sYPos - sHeight),
                     VO_BLT_SRCTRANSPARENCY, NULL);
    }
  } else {
    // brown, first entry
    BltVideoObject(uiBuffer, hHandle, 0, sXPos, (int16_t)(sYPos - sHeight), VO_BLT_SRCTRANSPARENCY,
                   NULL);
  }

  pDestBuf = LockVideoSurface(uiBuffer, &uiDestPitchBYTES);
  SetClippingRegionAndImageWidth(uiDestPitchBYTES, 0, 0, 640, 480);

  if (pSoldier->bBreathMax <= 97) {
    dPercentage = (float)((pSoldier->bBreathMax + 3)) / (float)100;
    dEnd = dPercentage * sHeight;
    dStart = sYPos;

    // the old background colors for breath max diff
    usLineColor = Get16BPPColor(BREATH_BAR_SHAD_BACK);
    RectangleDraw(TRUE, sXPos, (int32_t)dStart, sXPos, (int32_t)(dStart - dEnd), usLineColor,
                  pDestBuf);

    usLineColor = Get16BPPColor(BREATH_BAR_SHAD_BACK);
    RectangleDraw(TRUE, sXPos + 1, (int32_t)dStart, sXPos + 1, (int32_t)(dStart - dEnd),
                  usLineColor, pDestBuf);

    usLineColor = Get16BPPColor(BREATH_BAR_SHAD_BACK);
    RectangleDraw(TRUE, sXPos + 2, (int32_t)dStart, sXPos + 2, (int32_t)(dStart - dEnd),
                  usLineColor, pDestBuf);
  }

  dPercentage = (float)pSoldier->bBreathMax / (float)100;
  dEnd = dPercentage * sHeight;
  dStart = sYPos;

  usLineColor = Get16BPPColor(CURR_MAX_BREATH_SHADOW);
  RectangleDraw(TRUE, sXPos, (int32_t)dStart, sXPos, (int32_t)(dStart - dEnd), usLineColor,
                pDestBuf);

  usLineColor = Get16BPPColor(CURR_MAX_BREATH);
  RectangleDraw(TRUE, sXPos + 1, (int32_t)dStart, sXPos + 1, (int32_t)(dStart - dEnd), usLineColor,
                pDestBuf);

  usLineColor = Get16BPPColor(CURR_MAX_BREATH_SHADOW);
  RectangleDraw(TRUE, sXPos + 2, (int32_t)dStart, sXPos + 2, (int32_t)(dStart - dEnd), usLineColor,
                pDestBuf);

  // NOW DO BREATH
  dPercentage = (float)pSoldier->bBreath / (float)100;
  dEnd = dPercentage * sHeight;
  dStart = sYPos;

  usLineColor = Get16BPPColor(CURR_BREATH_BAR_SHADOW);
  RectangleDraw(TRUE, sXPos, (int32_t)dStart, sXPos, (int32_t)(dStart - dEnd), usLineColor,
                pDestBuf);

  usLineColor = Get16BPPColor(CURR_BREATH_BAR);
  RectangleDraw(TRUE, sXPos + 1, (int32_t)dStart, sXPos + 1, (int32_t)(dStart - dEnd), usLineColor,
                pDestBuf);

  usLineColor = Get16BPPColor(CURR_BREATH_BAR_SHADOW);
  RectangleDraw(TRUE, sXPos + 2, (int32_t)dStart, sXPos + 2, (int32_t)(dStart - dEnd), usLineColor,
                pDestBuf);

  UnLockVideoSurface(uiBuffer);
}

void DrawMoraleUIBarEx(struct SOLDIERTYPE *pSoldier, int16_t sXPos, int16_t sYPos, int16_t sWidth,
                       int16_t sHeight, BOOLEAN fErase, uint32_t uiBuffer) {
  float dStart, dEnd, dPercentage;
  // uint16_t usLineColor;

  uint32_t uiDestPitchBYTES;
  uint8_t *pDestBuf;
  uint16_t usLineColor;

  // Erase what was there
  if (fErase) {
    RestoreExternBackgroundRect(sXPos, (int16_t)(sYPos - sHeight), sWidth, (int16_t)(sHeight + 1));
  }

  if (pSoldier->bLife == 0) {
    // are they dead?
    return;
  }

  pDestBuf = LockVideoSurface(uiBuffer, &uiDestPitchBYTES);
  SetClippingRegionAndImageWidth(uiDestPitchBYTES, 0, 0, 640, 480);

  // FIRST DO BREATH
  dPercentage = (float)pSoldier->bMorale / (float)100;
  dEnd = dPercentage * sHeight;
  dStart = sYPos;

  usLineColor = Get16BPPColor(MORALE_BAR_SHADOW);
  RectangleDraw(TRUE, sXPos, (int32_t)dStart, sXPos, (int32_t)(dStart - dEnd), usLineColor,
                pDestBuf);

  usLineColor = Get16BPPColor(MORALE_BAR);
  RectangleDraw(TRUE, sXPos + 1, (int32_t)dStart, sXPos + 1, (int32_t)(dStart - dEnd), usLineColor,
                pDestBuf);

  usLineColor = Get16BPPColor(MORALE_BAR_SHADOW);
  RectangleDraw(TRUE, sXPos + 2, (int32_t)dStart, sXPos + 2, (int32_t)(dStart - dEnd), usLineColor,
                pDestBuf);

  UnLockVideoSurface(uiBuffer);
}

void DrawItemUIBarEx(struct OBJECTTYPE *pObject, uint8_t ubStatus, int16_t sXPos, int16_t sYPos,
                     int16_t sWidth, int16_t sHeight, int16_t sColor1, int16_t sColor2,
                     BOOLEAN fErase, uint32_t uiBuffer) {
  float dStart, dEnd, dPercentage;
  // uint16_t usLineColor;

  uint32_t uiDestPitchBYTES;
  uint8_t *pDestBuf;
  uint16_t usLineColor;
  int16_t sValue;

  if (ubStatus >= DRAW_ITEM_STATUS_ATTACHMENT1) {
    sValue = pObject->bAttachStatus[ubStatus - DRAW_ITEM_STATUS_ATTACHMENT1];
  } else {
    sValue = pObject->bStatus[ubStatus];
  }

  // Adjust for ammo, other thingys..
  if (Item[pObject->usItem].usItemClass & IC_AMMO) {
    sValue = sValue * 100 / Magazine[Item[pObject->usItem].ubClassIndex].ubMagSize;

    if (sValue > 100) {
      sValue = 100;
    }
  }

  if (Item[pObject->usItem].usItemClass & IC_KEY) {
    sValue = 100;
  }

  // ATE: Subtract 1 to exagerate bad status
  if (sValue < 100 && sValue > 1) {
    sValue--;
  }

  // Erase what was there
  if (fErase) {
    // RestoreExternBackgroundRect( sXPos, (int16_t)(sYPos - sHeight), sWidth, (int16_t)(sHeight + 1
    // )
    // );
  }

  pDestBuf = LockVideoSurface(uiBuffer, &uiDestPitchBYTES);
  SetClippingRegionAndImageWidth(uiDestPitchBYTES, 0, 0, 640, 480);

  // FIRST DO BREATH
  dPercentage = (float)sValue / (float)100;
  dEnd = dPercentage * sHeight;
  dStart = sYPos;

  // usLineColor = Get16BPPColor( STATUS_BAR );
  usLineColor = sColor1;
  RectangleDraw(TRUE, sXPos, (int32_t)dStart, sXPos, (int32_t)(dStart - dEnd), usLineColor,
                pDestBuf);

  usLineColor = sColor2;
  RectangleDraw(TRUE, sXPos + 1, (int32_t)dStart, sXPos + 1, (int32_t)(dStart - dEnd), usLineColor,
                pDestBuf);

  UnLockVideoSurface(uiBuffer);

  if (uiBuffer == guiSAVEBUFFER) {
    RestoreExternBackgroundRect(sXPos, (int16_t)(sYPos - sHeight), sWidth, (int16_t)(sHeight + 1));
  } else {
    InvalidateRegion(sXPos, (int16_t)(sYPos - sHeight), sXPos + sWidth, (int16_t)(sYPos + 1));
  }
}

void RenderSoldierFace(struct SOLDIERTYPE *pSoldier, int16_t sFaceX, int16_t sFaceY,
                       BOOLEAN fAutoFace) {
  BOOLEAN fDoFace = FALSE;
  uint8_t ubVehicleType = 0;

  if (IsSolActive(pSoldier)) {
    if (pSoldier->uiStatusFlags & SOLDIER_VEHICLE) {
      // get the type of vehicle
      ubVehicleType = pVehicleList[pSoldier->bVehicleID].ubVehicleType;

      // just draw the vehicle
      BltVideoObjectFromIndex(guiSAVEBUFFER, giCarPortraits[ubVehicleType], 0, sFaceX, sFaceY,
                              VO_BLT_SRCTRANSPARENCY, NULL);
      RestoreExternBackgroundRect(sFaceX, sFaceY, FACE_WIDTH, FACE_HEIGHT);

      return;
    }

    if (fAutoFace) {
      // OK, check if this face actually went active...
      if (gFacesData[pSoldier->iFaceIndex].uiFlags & FACE_INACTIVE_HANDLED_ELSEWHERE) {
        // Render as an extern face...
        fAutoFace = FALSE;
      } else {
        SetAutoFaceActiveFromSoldier(FRAME_BUFFER, guiSAVEBUFFER, GetSolID(pSoldier), sFaceX,
                                     sFaceY);
        //	SetAutoFaceActiveFromSoldier( FRAME_BUFFER, FACE_AUTO_RESTORE_BUFFER,
        // GetSolID(pSoldier) , sFaceX, sFaceY );
      }
    }

    fDoFace = TRUE;

    if (fDoFace) {
      if (fAutoFace) {
        RenderAutoFaceFromSoldier(pSoldier->ubID);
      } else {
        ExternRenderFaceFromSoldier(guiSAVEBUFFER, GetSolID(pSoldier), sFaceX, sFaceY);
      }
    }
  } else {
    BltVideoObjectFromIndex(guiSAVEBUFFER, guiCLOSE, 5, sFaceX, sFaceY, VO_BLT_SRCTRANSPARENCY,
                            NULL);
    RestoreExternBackgroundRect(sFaceX, sFaceY, FACE_WIDTH, FACE_HEIGHT);
  }
}
