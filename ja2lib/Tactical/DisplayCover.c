#include "Tactical/DisplayCover.h"

#include "GameSettings.h"
#include "SGP/Debug.h"
#include "Soldier.h"
#include "Strategic/GameClock.h"
#include "Strategic/StrategicMap.h"
#include "Tactical/AnimationControl.h"
#include "Tactical/Interface.h"
#include "Tactical/LOS.h"
#include "Tactical/Menptr.h"
#include "Tactical/OppList.h"
#include "Tactical/PathAI.h"
#include "Tactical/SoldierControl.h"
#include "Tactical/Weapons.h"
#include "TileEngine/IsometricUtils.h"
#include "TileEngine/RenderFun.h"
#include "TileEngine/RenderWorld.h"
#include "TileEngine/TileDef.h"
#include "TileEngine/WorldMan.h"
#include "Utils/FontControl.h"
#include "Utils/JA25EnglishText.h"
#include "Utils/Message.h"
#include "Utils/Text.h"

//*******  Local Defines **************************************************

#define DC_MAX_COVER_RANGE 31

#define DC__SOLDIER_VISIBLE_RANGE 31

#define DC__MIN_SIZE 4
#define DC__MAX_SIZE 11

typedef struct {
  int16_t sGridNo;
  int8_t bCover;  //% chance that the gridno is fully covered.  ie 100 if safe, 0  is has no cover
  //	BOOLEAN fRoof;
} BEST_COVER_STRUCT;

typedef struct {
  int16_t sGridNo;
  int8_t bVisibleToSoldier;
  BOOLEAN fRoof;
} VISIBLE_TO_SOLDIER_STRUCT;

/*
#define	DC__PRONE				(int8_t)( 0x01 )
#define DC__CROUCH			(int8_t)( 0x02 )
#define DC__STAND				(int8_t)( 0x04 )
*/
enum {
  DC__SEE_NO_STANCES,
  DC__SEE_1_STANCE,
  DC__SEE_2_STANCE,
  DC__SEE_3_STANCE,
};

//******  Global Variables  *****************************************

BEST_COVER_STRUCT gCoverRadius[DC_MAX_COVER_RANGE][DC_MAX_COVER_RANGE];
int16_t gsLastCoverGridNo = NOWHERE;
int16_t gsLastSoldierGridNo = NOWHERE;
int8_t gbLastStance = -1;

VISIBLE_TO_SOLDIER_STRUCT gVisibleToSoldierStruct[DC__SOLDIER_VISIBLE_RANGE]
                                                 [DC__SOLDIER_VISIBLE_RANGE];
int16_t gsLastVisibleToSoldierGridNo = NOWHERE;

//*******  Function Prototypes ***************************************

int8_t CalcCoverForGridNoBasedOnTeamKnownEnemies(struct SOLDIERTYPE *pSoldier,
                                                 int16_t sTargetGridno, int8_t bStance);
void CalculateCoverInRadiusAroundGridno(int16_t sTargetGridNo, int8_t bSearchRange);
void AddCoverTileToEachGridNo();
void AddCoverObjectToWorld(int16_t sGridNo, uint16_t usGraphic, BOOLEAN fRoof);
void RemoveCoverObjectFromWorld(int16_t sGridNo, uint16_t usGraphic, BOOLEAN fRoof);
int8_t GetCurrentMercForDisplayCoverStance();
struct SOLDIERTYPE *GetCurrentMercForDisplayCover();

void CalculateVisibleToSoldierAroundGridno(int16_t sGridNo, int8_t bSearchRange);
void AddVisibleToSoldierToEachGridNo();
int8_t CalcIfSoldierCanSeeGridNo(struct SOLDIERTYPE *pSoldier, int16_t sTargetGridNo,
                                 BOOLEAN fRoof);
BOOLEAN IsTheRoofVisible(int16_t sGridNo);

// ppp

//*******  Functions **************************************************

void DisplayCoverOfSelectedGridNo() {
  int16_t sGridNo;
  int8_t bStance;

  GetMouseMapPos(&sGridNo);

  // Only allowed in if there is someone selected
  if (gusSelectedSoldier == NOBODY) {
    return;
  }

  // if the cursor is in a the tactical map
  if (sGridNo != NOWHERE && sGridNo != 0) {
    bStance = GetCurrentMercForDisplayCoverStance();

    // if the gridno is different then the last one that was displayed
    if (sGridNo != gsLastCoverGridNo || gbLastStance != bStance ||
        MercPtrs[gusSelectedSoldier]->sGridNo != gsLastSoldierGridNo) {
      // if the cover is currently being displayed
      if (gsLastCoverGridNo != NOWHERE || gbLastStance != -1 || gsLastSoldierGridNo != NOWHERE) {
        // remove the gridnos
        RemoveCoverOfSelectedGridNo();
      } else {
        // if it is the first time in here

        // pop up a message to say we are in the display cover routine
#ifdef JA2TESTVERSION
        {
          wchar_t zString[512];
          swprintf(zString, ARR_SIZE(zString), L"%s, (%d)",
                   zNewTacticalMessages[TCTL_MSG__DISPLAY_COVER],
                   gGameSettings.ubSizeOfDisplayCover);
          ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, zString);
        }
#else
        ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE,
                  zNewTacticalMessages[TCTL_MSG__DISPLAY_COVER]);
#endif

        // increment the display cover counter ( just seeing how many times people use it )
        // gJa25SaveStruct.uiDisplayCoverCounter++;
      }

      gbLastStance = bStance;
      gsLastCoverGridNo = sGridNo;
      gsLastSoldierGridNo = MercPtrs[gusSelectedSoldier]->sGridNo;

      // Fill the array of gridno and cover values
      CalculateCoverInRadiusAroundGridno(sGridNo, gGameSettings.ubSizeOfDisplayCover);

      // Add the graphics to each gridno
      AddCoverTileToEachGridNo();

      // Re-render the scene!
      SetRenderFlags(RENDER_FLAG_FULL);
    }
  }
}

void AddCoverTileToEachGridNo() {
  uint32_t uiCntX, uiCntY;
  BOOLEAN fRoof = (gsInterfaceLevel != I_GROUND_LEVEL);

  // loop through all the gridnos
  for (uiCntY = 0; uiCntY < DC_MAX_COVER_RANGE; uiCntY++) {
    for (uiCntX = 0; uiCntX < DC_MAX_COVER_RANGE; uiCntX++) {
      // if there is a valid cover at this gridno
      if (gCoverRadius[uiCntX][uiCntY].bCover != -1) {
        // if the tile provides 80-100% cover
        if (gCoverRadius[uiCntX][uiCntY].bCover <= 100 &&
            gCoverRadius[uiCntX][uiCntY].bCover > 80) {
          AddCoverObjectToWorld(gCoverRadius[uiCntX][uiCntY].sGridNo, SPECIALTILE_COVER_5, fRoof);
        }

        // else if the tile provides 60-80% cover
        else if (gCoverRadius[uiCntX][uiCntY].bCover <= 80 &&
                 gCoverRadius[uiCntX][uiCntY].bCover > 60) {
          AddCoverObjectToWorld(gCoverRadius[uiCntX][uiCntY].sGridNo, SPECIALTILE_COVER_4, fRoof);
        }

        // else if the tile provides 40-60% cover
        else if (gCoverRadius[uiCntX][uiCntY].bCover <= 60 &&
                 gCoverRadius[uiCntX][uiCntY].bCover > 40) {
          AddCoverObjectToWorld(gCoverRadius[uiCntX][uiCntY].sGridNo, SPECIALTILE_COVER_3, fRoof);
        }

        // else if the tile provides 20-40% cover
        else if (gCoverRadius[uiCntX][uiCntY].bCover <= 40 &&
                 gCoverRadius[uiCntX][uiCntY].bCover > 20) {
          AddCoverObjectToWorld(gCoverRadius[uiCntX][uiCntY].sGridNo, SPECIALTILE_COVER_2, fRoof);
        }

        // else if the tile provides 0-20% cover
        else if (gCoverRadius[uiCntX][uiCntY].bCover <= 20 &&
                 gCoverRadius[uiCntX][uiCntY].bCover >= 0) {
          AddCoverObjectToWorld(gCoverRadius[uiCntX][uiCntY].sGridNo, SPECIALTILE_COVER_1, fRoof);
        }

        // should never get in here
        else {
          Assert(0);
        }
      }
    }
  }
}

void RemoveCoverOfSelectedGridNo() {
  uint32_t uiCntX, uiCntY;
  BOOLEAN fRoof = (gsInterfaceLevel != I_GROUND_LEVEL);

  if (gsLastCoverGridNo == NOWHERE) {
    return;
  }

  // loop through all the gridnos
  for (uiCntY = 0; uiCntY < DC_MAX_COVER_RANGE; uiCntY++) {
    for (uiCntX = 0; uiCntX < DC_MAX_COVER_RANGE; uiCntX++) {
      // if there is a valid cover at this gridno
      if (gCoverRadius[uiCntX][uiCntY].bCover != -1) {
        //				fRoof = gCoverRadius[ uiCntX ][ uiCntY ].fRoof;

        // if the tile provides 80-100% cover
        if (gCoverRadius[uiCntX][uiCntY].bCover <= 100 &&
            gCoverRadius[uiCntX][uiCntY].bCover > 80) {
          RemoveCoverObjectFromWorld(gCoverRadius[uiCntX][uiCntY].sGridNo, SPECIALTILE_COVER_5,
                                     fRoof);
        }

        // else if the tile provides 60-80% cover
        else if (gCoverRadius[uiCntX][uiCntY].bCover <= 80 &&
                 gCoverRadius[uiCntX][uiCntY].bCover > 60) {
          RemoveCoverObjectFromWorld(gCoverRadius[uiCntX][uiCntY].sGridNo, SPECIALTILE_COVER_4,
                                     fRoof);
        }

        // else if the tile provides 40-60% cover
        else if (gCoverRadius[uiCntX][uiCntY].bCover <= 60 &&
                 gCoverRadius[uiCntX][uiCntY].bCover > 40) {
          RemoveCoverObjectFromWorld(gCoverRadius[uiCntX][uiCntY].sGridNo, SPECIALTILE_COVER_3,
                                     fRoof);
        }

        // else if the tile provides 20-40% cover
        else if (gCoverRadius[uiCntX][uiCntY].bCover <= 40 &&
                 gCoverRadius[uiCntX][uiCntY].bCover > 20) {
          RemoveCoverObjectFromWorld(gCoverRadius[uiCntX][uiCntY].sGridNo, SPECIALTILE_COVER_2,
                                     fRoof);
        }

        // else if the tile provides 0-20% cover
        else if (gCoverRadius[uiCntX][uiCntY].bCover <= 20 &&
                 gCoverRadius[uiCntX][uiCntY].bCover >= 0) {
          RemoveCoverObjectFromWorld(gCoverRadius[uiCntX][uiCntY].sGridNo, SPECIALTILE_COVER_1,
                                     fRoof);
        }

        // should never get in here
        else {
          Assert(0);
        }
      }
    }
  }

  // Re-render the scene!
  SetRenderFlags(RENDER_FLAG_FULL);

  gsLastCoverGridNo = NOWHERE;
  gbLastStance = -1;
  gsLastSoldierGridNo = NOWHERE;
}

void CalculateCoverInRadiusAroundGridno(int16_t sTargetGridNo, int8_t bSearchRange) {
  int16_t sMaxLeft, sMaxRight, sMaxUp, sMaxDown, sXOffset, sYOffset;
  struct SOLDIERTYPE *pSoldier = NULL;
  int16_t sGridNo;
  int16_t sCounterX, sCounterY;
  uint8_t ubID;
  int8_t bStance;
  //	BOOLEAN fRoof;

  // clear out the array first
  //	memset( gCoverRadius, -1, DC_MAX_COVER_RANGE * DC_MAX_COVER_RANGE );
  // loop through all the gridnos that we are interested in
  for (sCounterY = 0; sCounterY < DC_MAX_COVER_RANGE; sCounterY++) {
    for (sCounterX = 0; sCounterX < DC_MAX_COVER_RANGE; sCounterX++) {
      gCoverRadius[sCounterX][sCounterY].sGridNo = -1;
      gCoverRadius[sCounterX][sCounterY].bCover = -1;
    }
  }

  if (bSearchRange > (DC_MAX_COVER_RANGE / 2)) bSearchRange = (DC_MAX_COVER_RANGE / 2);

  // determine maximum horizontal limits
  sMaxLeft = min(bSearchRange, (sTargetGridNo % MAXCOL));
  sMaxRight = min(bSearchRange, MAXCOL - ((sTargetGridNo % MAXCOL) + 1));

  // determine maximum vertical limits
  sMaxUp = min(bSearchRange, (sTargetGridNo / MAXROW));
  sMaxDown = min(bSearchRange, MAXROW - ((sTargetGridNo / MAXROW) + 1));

  // Find out which tiles around the location are reachable
  LocalReachableTest(sTargetGridNo, bSearchRange);

  pSoldier = GetCurrentMercForDisplayCover();

  sCounterX = sCounterY = 0;

  // Determine the stance to use
  bStance = GetCurrentMercForDisplayCoverStance();

  // loop through all the gridnos that we are interested in
  for (sYOffset = -sMaxUp; sYOffset <= sMaxDown; sYOffset++) {
    for (sXOffset = -sMaxLeft; sXOffset <= sMaxRight; sXOffset++) {
      sGridNo = sTargetGridNo + sXOffset + (MAXCOL * sYOffset);

      // record the gridno
      gCoverRadius[sCounterX][sCounterY].sGridNo = sGridNo;

      /*
                              fRoof = FALSE;

                              //is there a roof above this gridno
                              if( FlatRoofAboveGridNo( sGridNo ) )
                              {
                                      if( IsTheRoofVisible( sGridNo ) )
                                      {
                                              fRoof = TRUE;
                                      }
                              }
      */
      // if the gridno is NOT on screen
      if (!GridNoOnScreen(sGridNo)) {
        continue;
      }

      // if we are to display cover for the roofs, and there is a roof above us
      if (gsInterfaceLevel == I_ROOF_LEVEL && !FlatRoofAboveGridNo(sGridNo)) {
        continue;
      }

      // if the gridno cant be reached
      if (!(gpWorldLevelData[sGridNo].uiFlags & MAPELEMENT_REACHABLE)) {
        // skip to the next gridno
        sCounterX++;
        continue;
      }

      // if someone (visible) is there, skip
      // Check both bottom level, and top level
      ubID = WhoIsThere2(sGridNo, 0);
      if (ubID == NOBODY) {
        ubID = WhoIsThere2(sGridNo, 1);
      }
      // if someone is here, and they are an enemy, skip over them
      if (ubID != NOBODY && Menptr[ubID].bVisible == TRUE &&
          Menptr[ubID].bTeam != pSoldier->bTeam) {
        continue;
      }

      // Calculate the cover for this gridno
      gCoverRadius[sCounterX][sCounterY].bCover =
          CalcCoverForGridNoBasedOnTeamKnownEnemies(pSoldier, sGridNo, bStance);
      //			gCoverRadius[ sCounterX ][ sCounterY ].fRoof = fRoof;

      sCounterX++;
    }
    sCounterY++;
    sCounterX = 0;
  }
}

int8_t CalcCoverForGridNoBasedOnTeamKnownEnemies(struct SOLDIERTYPE *pSoldier,
                                                 int16_t sTargetGridNo, int8_t bStance) {
  int32_t iTotalCoverPoints = 0;
  int8_t bNumEnemies = 0;
  int8_t bPercentCoverForGridno = 0;
  uint32_t uiLoop;
  struct SOLDIERTYPE *pOpponent;
  int8_t *pbPersOL;
  int8_t *pbPublOL;
  int32_t iGetThrough = 0;
  int32_t iBulletGetThrough = 0;
  int32_t iHighestValue = 0;
  int32_t iCover = 0;
  uint16_t usMaxRange;
  uint16_t usRange;
  uint16_t usSightLimit;

  // loop through all the enemies and determine the cover
  for (uiLoop = 0; uiLoop < guiNumMercSlots; uiLoop++) {
    pOpponent = MercSlots[uiLoop];

    // if this merc is inactive, at base, on assignment, dead, unconscious
    if (!pOpponent || pOpponent->bLife < OKLIFE) {
      continue;  // next merc
    }

    // if this man is neutral / on the same side, he's not an opponent
    if (CONSIDERED_NEUTRAL(pSoldier, pOpponent) || (pSoldier->bSide == pOpponent->bSide)) {
      continue;  // next merc
    }

    pbPersOL = pSoldier->bOppList + pOpponent->ubID;
    pbPublOL = gbPublicOpplist[OUR_TEAM] + pOpponent->ubID;

    // if this opponent is unknown personally and publicly
    if (*pbPersOL != SEEN_CURRENTLY && *pbPersOL != SEEN_THIS_TURN && *pbPublOL != SEEN_CURRENTLY &&
        *pbPublOL != SEEN_THIS_TURN) {
      continue;  // next merc
    }

    usRange = (uint16_t)GetRangeInCellCoordsFromGridNoDiff(pOpponent->sGridNo, sTargetGridNo);
    usSightLimit = DistanceVisible(pOpponent, DIRECTION_IRRELEVANT, DIRECTION_IRRELEVANT,
                                   sTargetGridNo, pSoldier->bLevel);

    if (usRange > (usSightLimit * CELL_X_SIZE)) {
      continue;
    }

    // if actual LOS check fails, then chance to hit is 0, ignore this guy
    if (SoldierToVirtualSoldierLineOfSightTest(pOpponent, sTargetGridNo, pSoldier->bLevel, bStance,
                                               (uint8_t)usSightLimit, TRUE) == 0) {
      continue;
    }

    iGetThrough = SoldierToLocationChanceToGetThrough(pOpponent, sTargetGridNo, pSoldier->bLevel,
                                                      bStance, NOBODY);
    //	iBulletGetThrough = CalcChanceToHitGun( pOpponent, sTargetGridNo, AP_MAX_AIM_ATTACK,
    // AIM_SHOT_TORSO );

    if (WeaponInHand(pOpponent)) {
      usMaxRange = GunRange(&pOpponent->inv[HANDPOS]);
    } else {
      usMaxRange = Weapon[GLOCK_18].usRange;
    }

    iBulletGetThrough =
        min(max((int32_t)(((((usMaxRange - usRange) / (FLOAT)(usMaxRange)) + .3) * 100)), 0), 100);

    if (iBulletGetThrough > 5 && iGetThrough > 0) {
      iCover = (iGetThrough * iBulletGetThrough / 100);

      if (iCover > iHighestValue) iHighestValue = iCover;

      iTotalCoverPoints += iCover;
      bNumEnemies++;
    }
  }

  if (bNumEnemies == 0) {
    bPercentCoverForGridno = 100;
  } else {
    int32_t iTemp;

    bPercentCoverForGridno = (iTotalCoverPoints / bNumEnemies);

    iTemp = bPercentCoverForGridno - (iHighestValue / bNumEnemies);

    iTemp = iTemp + iHighestValue;

    bPercentCoverForGridno = 100 - (min(iTemp, 100));
  }

  return (bPercentCoverForGridno);
}

void AddCoverObjectToWorld(int16_t sGridNo, uint16_t usGraphic, BOOLEAN fRoof) {
  struct LEVELNODE *pNode;

  if (fRoof) {
    AddOnRoofToHead(sGridNo, usGraphic);
    pNode = gpWorldLevelData[sGridNo].pOnRoofHead;
  } else {
    AddObjectToHead(sGridNo, usGraphic);
    pNode = gpWorldLevelData[sGridNo].pObjectHead;
  }

  pNode->uiFlags |= LEVELNODE_REVEAL;

  if (NightTime()) {
    pNode->ubShadeLevel = DEFAULT_SHADE_LEVEL;
    pNode->ubNaturalShadeLevel = DEFAULT_SHADE_LEVEL;
  }
}

void RemoveCoverObjectFromWorld(int16_t sGridNo, uint16_t usGraphic, BOOLEAN fRoof) {
  if (fRoof) {
    RemoveOnRoof(sGridNo, usGraphic);
  } else {
    RemoveObject(sGridNo, usGraphic);
  }
}

struct SOLDIERTYPE *GetCurrentMercForDisplayCover() {
  struct SOLDIERTYPE *pSoldier = NULL;
  // Get a soldier that is on the player team
  if (gusSelectedSoldier != NOBODY) {
    GetSoldier(&pSoldier, gusSelectedSoldier);
  } else {
    Assert(0);
  }
  return (pSoldier);
}

int8_t GetCurrentMercForDisplayCoverStance() {
  int8_t bStance;
  struct SOLDIERTYPE *pSoldier = NULL;

  pSoldier = GetCurrentMercForDisplayCover();

  switch (pSoldier->usUIMovementMode) {
    case PRONE:
    case CRAWLING:
      bStance = ANIM_PRONE;
      break;

    case KNEEL_DOWN:
    case SWATTING:
    case CROUCHING:
      bStance = ANIM_CROUCH;
      break;

    case WALKING:
    case RUNNING:
    case STANDING:
      bStance = ANIM_STAND;
      break;

    default:
      bStance = ANIM_CROUCH;
      break;
  }

  return (bStance);
}

void DisplayRangeToTarget(struct SOLDIERTYPE *pSoldier, int16_t sTargetGridNo) {
  uint16_t usRange = 0;
  wchar_t zOutputString[512];

  if (sTargetGridNo == NOWHERE || sTargetGridNo == 0) {
    return;
  }

  // Get the range to the target location
  usRange = GetRangeInCellCoordsFromGridNoDiff(pSoldier->sGridNo, sTargetGridNo);

  usRange = usRange / 10;

  // if the soldier has a weapon in hand,
  if (WeaponInHand(pSoldier)) {
    // display a string with the weapons range, then range to target
    swprintf(zOutputString, ARR_SIZE(zOutputString),
             zNewTacticalMessages[TCTL_MSG__RANGE_TO_TARGET_AND_GUN_RANGE],
             Weapon[pSoldier->inv[HANDPOS].usItem].usRange / 10, usRange);
  } else {
    // display a string with the range to target
    swprintf(zOutputString, ARR_SIZE(zOutputString),
             zNewTacticalMessages[TCTL_MSG__RANGE_TO_TARGET], usRange);
  }

  // Display the msg
  ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, zOutputString);

  // if the target is out of the mercs gun range or knife
  if (!InRange(pSoldier, sTargetGridNo) &&
      (Item[pSoldier->inv[HANDPOS].usItem].usItemClass == IC_GUN ||
       Item[pSoldier->inv[HANDPOS].usItem].usItemClass == IC_THROWING_KNIFE)) {
    // Display a warning saying so
    ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, TacticalStr[OUT_OF_RANGE_STRING]);
  }

  // increment the display gun range counter ( just seeing how many times people use it )
  // gJa25SaveStruct.uiDisplayGunRangeCounter++;
}

void DisplayGridNoVisibleToSoldierGrid() {
  int16_t sGridNo;
  //	int8_t	bStance;

  GetMouseMapPos(&sGridNo);

  // Only allowed in if there is someone selected
  if (gusSelectedSoldier == NOBODY) {
    return;
  }

  // if the cursor is in a the tactical map
  if (sGridNo != NOWHERE && sGridNo != 0) {
    // if the gridno is different then the last one that was displayed
    if (sGridNo != gsLastVisibleToSoldierGridNo ||
        MercPtrs[gusSelectedSoldier]->sGridNo != gsLastSoldierGridNo) {
      // if the cover is currently being displayed
      if (gsLastVisibleToSoldierGridNo != NOWHERE || gsLastSoldierGridNo != NOWHERE) {
        // remove the gridnos
        RemoveVisibleGridNoAtSelectedGridNo();
      } else {
#ifdef JA2TESTVERSION
        {
          wchar_t zString[512];
          swprintf(zString, ARR_SIZE(zString), L"%s, (%d)", zNewTacticalMessages[TCTL_MSG__LOS],
                   gGameSettings.ubSizeOfLOS);
          ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, zString);
        }
#else
        // pop up a message to say we are in the display cover routine
        ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, zNewTacticalMessages[TCTL_MSG__LOS]);
#endif
        // increment the display LOS counter ( just seeing how many times people use it )
        // gJa25SaveStruct.uiDisplayLosCounter++;
      }

      gsLastVisibleToSoldierGridNo = sGridNo;
      gsLastSoldierGridNo = MercPtrs[gusSelectedSoldier]->sGridNo;

      // Fill the array of gridno and cover values
      CalculateVisibleToSoldierAroundGridno(sGridNo, gGameSettings.ubSizeOfLOS);

      // Add the graphics to each gridno
      AddVisibleToSoldierToEachGridNo();

      // Re-render the scene!
      SetRenderFlags(RENDER_FLAG_FULL);
    }
  }
}

void CalculateVisibleToSoldierAroundGridno(int16_t sTargetGridNo, int8_t bSearchRange) {
  int16_t sMaxLeft, sMaxRight, sMaxUp, sMaxDown, sXOffset, sYOffset;
  struct SOLDIERTYPE *pSoldier = NULL;
  int16_t sGridNo;
  int16_t sCounterX, sCounterY;
  BOOLEAN fRoof = FALSE;

  // clear out the struct
  memset(gVisibleToSoldierStruct, 0,
         sizeof(VISIBLE_TO_SOLDIER_STRUCT) * DC__SOLDIER_VISIBLE_RANGE * DC__SOLDIER_VISIBLE_RANGE);

  if (bSearchRange > (DC_MAX_COVER_RANGE / 2)) bSearchRange = (DC_MAX_COVER_RANGE / 2);

  // determine maximum horizontal limits
  sMaxLeft = min(bSearchRange, (sTargetGridNo % MAXCOL));
  sMaxRight = min(bSearchRange, MAXCOL - ((sTargetGridNo % MAXCOL) + 1));

  // determine maximum vertical limits
  sMaxUp = min(bSearchRange, (sTargetGridNo / MAXROW));
  sMaxDown = min(bSearchRange, MAXROW - ((sTargetGridNo / MAXROW) + 1));

  pSoldier = GetCurrentMercForDisplayCover();

  sCounterX = 0;
  sCounterY = 0;

  // loop through all the gridnos that we are interested in
  for (sYOffset = -sMaxUp; sYOffset <= sMaxDown; sYOffset++) {
    sCounterX = 0;
    for (sXOffset = -sMaxLeft; sXOffset <= sMaxRight; sXOffset++) {
      sGridNo = sTargetGridNo + sXOffset + (MAXCOL * sYOffset);
      fRoof = FALSE;

      // record the gridno
      gVisibleToSoldierStruct[sCounterX][sCounterY].sGridNo = sGridNo;

      // if the gridno is NOT on screen
      if (!GridNoOnScreen(sGridNo)) {
        continue;
      }

      // is there a roof above this gridno
      if (FlatRoofAboveGridNo(sGridNo)) {
        if (IsTheRoofVisible(sGridNo) && gbWorldSectorZ == 0) {
          fRoof = TRUE;
        }

        // if wer havent explored the area yet and we are underground, dont show cover
        else if (!(gpWorldLevelData[sGridNo].uiFlags & MAPELEMENT_REVEALED) &&
                 gbWorldSectorZ != 0) {
          continue;
        }
      }

      /*
                              //if we are to display cover for the roofs, and there is a roof above
         us if( gsInterfaceLevel == I_ROOF_LEVEL && !FlatRoofAboveGridNo( sGridNo ) )
                              {
                                      continue;
                              }
      */
      /*
                              // if someone (visible) is there, skip
                              //Check both bottom level, and top level
                              ubID = WhoIsThere2( sGridNo, 0 );
                              if( ubID == NOBODY )
                              {
                                      ubID = WhoIsThere2( sGridNo, 1 );
                              }
                              //if someone is here, and they are an enemy, skip over them
                              if ( ubID != NOBODY && Menptr[ ubID ].bVisible == TRUE && Menptr[ ubID
         ].bTeam != pSoldier->bTeam )
                              {
                                      continue;
                              }

                              //Calculate the cover for this gridno
                              gCoverRadius[ sCounterX ][ sCounterY ].bCover =
         CalcCoverForGridNoBasedOnTeamKnownEnemies( pSoldier, sGridNo, bStance );
      */

      gVisibleToSoldierStruct[sCounterX][sCounterY].bVisibleToSoldier =
          CalcIfSoldierCanSeeGridNo(pSoldier, sGridNo, fRoof);
      gVisibleToSoldierStruct[sCounterX][sCounterY].fRoof = fRoof;
      sCounterX++;
    }

    sCounterY++;
  }
}

void AddVisibleToSoldierToEachGridNo() {
  uint32_t uiCntX, uiCntY;
  int8_t bVisibleToSoldier = 0;
  BOOLEAN fRoof;
  int16_t sGridNo;

  // loop through all the gridnos
  for (uiCntY = 0; uiCntY < DC_MAX_COVER_RANGE; uiCntY++) {
    for (uiCntX = 0; uiCntX < DC_MAX_COVER_RANGE; uiCntX++) {
      bVisibleToSoldier = gVisibleToSoldierStruct[uiCntX][uiCntY].bVisibleToSoldier;
      if (bVisibleToSoldier == -1) {
        continue;
      }

      fRoof = gVisibleToSoldierStruct[uiCntX][uiCntY].fRoof;
      sGridNo = gVisibleToSoldierStruct[uiCntX][uiCntY].sGridNo;

      // if the soldier can easily see this gridno.  Can see all 3 positions
      if (bVisibleToSoldier == DC__SEE_3_STANCE) {
        AddCoverObjectToWorld(sGridNo, SPECIALTILE_COVER_5, fRoof);
      }

      // cant see a thing
      else if (bVisibleToSoldier == DC__SEE_NO_STANCES) {
        AddCoverObjectToWorld(gVisibleToSoldierStruct[uiCntX][uiCntY].sGridNo, SPECIALTILE_COVER_1,
                              fRoof);
      }

      // can only see prone
      else if (bVisibleToSoldier == DC__SEE_1_STANCE) {
        AddCoverObjectToWorld(gVisibleToSoldierStruct[uiCntX][uiCntY].sGridNo, SPECIALTILE_COVER_2,
                              fRoof);
      }

      // can see crouch or prone
      else if (bVisibleToSoldier == DC__SEE_2_STANCE) {
        AddCoverObjectToWorld(gVisibleToSoldierStruct[uiCntX][uiCntY].sGridNo, SPECIALTILE_COVER_3,
                              fRoof);
      }

      else {
        Assert(0);
      }
    }
  }
}

void RemoveVisibleGridNoAtSelectedGridNo() {
  uint32_t uiCntX, uiCntY;
  int8_t bVisibleToSoldier;
  BOOLEAN fRoof;

  // make sure to only remove it when its right
  if (gsLastVisibleToSoldierGridNo == NOWHERE) {
    return;
  }

  // loop through all the gridnos
  for (uiCntY = 0; uiCntY < DC_MAX_COVER_RANGE; uiCntY++) {
    for (uiCntX = 0; uiCntX < DC_MAX_COVER_RANGE; uiCntX++) {
      bVisibleToSoldier = gVisibleToSoldierStruct[uiCntX][uiCntY].bVisibleToSoldier;
      fRoof = gVisibleToSoldierStruct[uiCntX][uiCntY].fRoof;

      // if there is a valid cover at this gridno
      if (bVisibleToSoldier == DC__SEE_3_STANCE) {
        RemoveCoverObjectFromWorld(gVisibleToSoldierStruct[uiCntX][uiCntY].sGridNo,
                                   SPECIALTILE_COVER_5, fRoof);
      }

      // cant see a thing
      else if (bVisibleToSoldier == DC__SEE_NO_STANCES) {
        RemoveCoverObjectFromWorld(gVisibleToSoldierStruct[uiCntX][uiCntY].sGridNo,
                                   SPECIALTILE_COVER_1, fRoof);
      }

      // can only see prone
      else if (bVisibleToSoldier == DC__SEE_1_STANCE) {
        RemoveCoverObjectFromWorld(gVisibleToSoldierStruct[uiCntX][uiCntY].sGridNo,
                                   SPECIALTILE_COVER_2, fRoof);
      }

      // can see crouch or prone
      else if (bVisibleToSoldier == DC__SEE_2_STANCE) {
        RemoveCoverObjectFromWorld(gVisibleToSoldierStruct[uiCntX][uiCntY].sGridNo,
                                   SPECIALTILE_COVER_3, fRoof);
      }

      else {
        Assert(0);
      }
    }
  }

  // Re-render the scene!
  SetRenderFlags(RENDER_FLAG_FULL);

  gsLastVisibleToSoldierGridNo = NOWHERE;
  gsLastSoldierGridNo = NOWHERE;
}

int8_t CalcIfSoldierCanSeeGridNo(struct SOLDIERTYPE *pSoldier, int16_t sTargetGridNo,
                                 BOOLEAN fRoof) {
  int8_t bRetVal = 0;
  int32_t iLosForGridNo = 0;
  uint16_t usSightLimit = 0;
  int8_t *pPersOL, *pbPublOL;
  uint8_t ubID;
  BOOLEAN bAware = FALSE;

  if (fRoof) {
    ubID = WhoIsThere2(sTargetGridNo, 1);
  } else {
    ubID = WhoIsThere2(sTargetGridNo, 0);
  }

  if (ubID != NOBODY) {
    pPersOL = &(pSoldier->bOppList[ubID]);
    pbPublOL = &(gbPublicOpplist[pSoldier->bTeam][ubID]);

    // if soldier is known about (SEEN or HEARD within last few turns)
    if (*pPersOL || *pbPublOL) {
      bAware = TRUE;
    }
  }

  usSightLimit =
      DistanceVisible(pSoldier, DIRECTION_IRRELEVANT, DIRECTION_IRRELEVANT, sTargetGridNo, fRoof);

  //
  // Prone
  //
  iLosForGridNo = SoldierToVirtualSoldierLineOfSightTest(pSoldier, sTargetGridNo, fRoof, ANIM_PRONE,
                                                         (uint8_t)usSightLimit, bAware);
  if (iLosForGridNo != 0) {
    bRetVal++;
  }

  //
  // Crouch
  //
  iLosForGridNo = SoldierToVirtualSoldierLineOfSightTest(
      pSoldier, sTargetGridNo, fRoof, ANIM_CROUCH, (uint8_t)usSightLimit, bAware);
  if (iLosForGridNo != 0) {
    bRetVal++;
  }

  //
  // Standing
  //
  iLosForGridNo = SoldierToVirtualSoldierLineOfSightTest(pSoldier, sTargetGridNo, fRoof, ANIM_STAND,
                                                         (uint8_t)usSightLimit, bAware);
  if (iLosForGridNo != 0) {
    bRetVal++;
  }

  return (bRetVal);
}

BOOLEAN IsTheRoofVisible(int16_t sGridNo) {
  uint8_t ubRoom;

  if (InARoom(sGridNo, &ubRoom)) {
    if (gpWorldLevelData[sGridNo].uiFlags & MAPELEMENT_REVEALED) {
      if (gTacticalStatus.uiFlags & SHOW_ALL_ROOFS)
        return (TRUE);
      else
        return (FALSE);
    } else {
      return (TRUE);
    }
  }

  return (FALSE);
}

#ifdef JA2TESTVERSION
/*
void DisplayLosAndDisplayCoverUsageScreenMsg()
{
        wchar_t	zString[512];

        swprintf( zString, L"Display Cover: %d", gJa25SaveStruct.uiDisplayCoverCounter );
        ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, zString );

        swprintf( zString, L"LOS: %d", gJa25SaveStruct.uiDisplayLosCounter );
        ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, zString );

        swprintf( zString, L"Gun Range: %d", gJa25SaveStruct.uiDisplayGunRangeCounter );
        ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, zString );
}
*/
#endif

void ChangeSizeOfDisplayCover(int32_t iNewSize) {
  // if the new size is smaller or greater, scale it
  if (iNewSize < DC__MIN_SIZE) {
    iNewSize = DC__MIN_SIZE;
  } else if (iNewSize > DC__MAX_SIZE) {
    iNewSize = DC__MAX_SIZE;
  }

  // Set new size
  gGameSettings.ubSizeOfDisplayCover = (uint8_t)iNewSize;

  // redisplay the cover
  RemoveCoverOfSelectedGridNo();
  DisplayCoverOfSelectedGridNo();
}

void ChangeSizeOfLOS(int32_t iNewSize) {
  // if the new size is smaller or greater, scale it
  if (iNewSize < DC__MIN_SIZE) {
    iNewSize = DC__MIN_SIZE;
  } else if (iNewSize > DC__MAX_SIZE) {
    iNewSize = DC__MAX_SIZE;
  }

  // Set new size
  gGameSettings.ubSizeOfLOS = (uint8_t)iNewSize;

  // ReDisplay the los
  RemoveVisibleGridNoAtSelectedGridNo();
  DisplayGridNoVisibleToSoldierGrid();
}
