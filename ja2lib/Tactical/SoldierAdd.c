#include "Tactical/SoldierAdd.h"

#include "SGP/Debug.h"
#include "SGP/Random.h"
#include "SGP/Types.h"
#include "Soldier.h"
#include "Strategic/Meanwhile.h"
#include "Strategic/Strategic.h"
#include "Strategic/StrategicMap.h"
#include "Tactical/AnimationControl.h"
#include "Tactical/FOV.h"
#include "Tactical/InterfacePanels.h"
#include "Tactical/MapInformation.h"
#include "Tactical/Overhead.h"
#include "Tactical/OverheadTypes.h"
#include "Tactical/PathAI.h"
#include "Tactical/SoldierControl.h"
#include "Tactical/SoldierInitList.h"
#include "Tactical/SoldierMacros.h"
#include "TileEngine/ExitGrids.h"
#include "TileEngine/IsometricUtils.h"
#include "TileEngine/RenderFun.h"
#include "TileEngine/Structure.h"
#include "TileEngine/StructureInternals.h"

// Adds a soldier to a world gridno and set's direction
void AddSoldierToSectorGridNo(struct SOLDIERTYPE *pSoldier, int16_t sGridNo, uint8_t ubDirection,
                              BOOLEAN fUseAnimation, uint16_t usAnimState, uint16_t usAnimCode);

uint16_t FindGridNoFromSweetSpotWithStructData(struct SOLDIERTYPE *pSoldier, uint16_t usAnimState,
                                             int16_t sSweetGridNo, int8_t ubRadius, uint8_t *pubDirection,
                                             BOOLEAN fClosestToMerc);

// SO, STEPS IN CREATING A MERC!

// 1 ) Setup the SOLDIERCREATE_STRUCT
//			Among other things, this struct needs a sSectorX, sSectorY, and a valid
// InsertionDirection 			and InsertionGridNo. 			This GridNo will be
// determined by a prevouis function that will examine the sector 			Infomration
// regarding placement positions and pick one
// 2 ) Call TacticalCreateSoldier() which will create our soldier
//			What it does is:	Creates a soldier in the MercPtrs[] array.
//												Allocates
// the Animation
// cache for this merc
// Loads up the intial aniamtion file Creates initial palettes, etc And other cool things. Now we
// have an allocated soldier, we just need to set him in the world! 3) When we want them in the
// world, call AddSoldierToSector().
//			This function sets the graphic in the world, lighting effects, etc
//			It also formally adds it to the TacticalSoldier slot and interface panel
// slot.

// Kris:  modified to actually path from sweetspot to gridno.  Previously, it only checked if the
// destination was sittable (though it was possible that that location would be trapped.
uint16_t FindGridNoFromSweetSpot(struct SOLDIERTYPE *pSoldier, int16_t sSweetGridNo, int8_t ubRadius,
                               uint8_t *pubDirection) {
  int16_t sTop, sBottom;
  int16_t sLeft, sRight;
  int16_t cnt1, cnt2;
  int16_t sGridNo;
  int32_t uiRange, uiLowestRange = 999999;
  int16_t sLowestGridNo = -1;
  int32_t leftmost;
  BOOLEAN fFound = FALSE;
  struct SOLDIERTYPE soldier;
  uint8_t ubSaveNPCAPBudget;
  uint8_t ubSaveNPCDistLimit;

  // Save AI pathing vars.  changing the distlimit restricts how
  // far away the pathing will consider.
  ubSaveNPCAPBudget = gubNPCAPBudget;
  ubSaveNPCDistLimit = gubNPCDistLimit;
  gubNPCAPBudget = 0;
  gubNPCDistLimit = ubRadius;

  // create dummy soldier, and use the pathing to determine which nearby slots are
  // reachable.
  memset(&soldier, 0, sizeof(struct SOLDIERTYPE));
  soldier.bLevel = 0;
  soldier.bTeam = 1;
  soldier.sGridNo = sSweetGridNo;

  sTop = ubRadius;
  sBottom = -ubRadius;
  sLeft = -ubRadius;
  sRight = ubRadius;

  // clear the mapelements of potential residue MAPELEMENT_REACHABLE flags
  // in the square region.
  // ATE: CHECK FOR BOUNDARIES!!!!!!
  for (cnt1 = sBottom; cnt1 <= sTop; cnt1++) {
    leftmost = ((sSweetGridNo + (WORLD_COLS * cnt1)) / WORLD_COLS) * WORLD_COLS;

    for (cnt2 = sLeft; cnt2 <= sRight; cnt2++) {
      sGridNo = sSweetGridNo + (WORLD_COLS * cnt1) + cnt2;
      if (sGridNo >= 0 && sGridNo < WORLD_MAX && sGridNo >= leftmost &&
          sGridNo < (leftmost + WORLD_COLS)) {
        gpWorldLevelData[sGridNo].uiFlags &= (~MAPELEMENT_REACHABLE);
      }
    }
  }

  // Now, find out which of these gridnos are reachable
  //(use the fake soldier and the pathing settings)
  FindBestPath(&soldier, NOWHERE, 0, WALKING, COPYREACHABLE,
               (PATH_IGNORE_PERSON_AT_DEST | PATH_THROUGH_PEOPLE));

  uiLowestRange = 999999;

  for (cnt1 = sBottom; cnt1 <= sTop; cnt1++) {
    leftmost = ((sSweetGridNo + (WORLD_COLS * cnt1)) / WORLD_COLS) * WORLD_COLS;

    for (cnt2 = sLeft; cnt2 <= sRight; cnt2++) {
      sGridNo = sSweetGridNo + (WORLD_COLS * cnt1) + cnt2;
      if (sGridNo >= 0 && sGridNo < WORLD_MAX && sGridNo >= leftmost &&
          sGridNo < (leftmost + WORLD_COLS) &&
          gpWorldLevelData[sGridNo].uiFlags & MAPELEMENT_REACHABLE) {
        // Go on sweet stop
        if (NewOKDestination(pSoldier, sGridNo, TRUE, pSoldier->bLevel)) {
          // ATE: INstead of using absolute range, use the path cost!
          // uiRange = PlotPath( &soldier, sGridNo, NO_COPYROUTE, NO_PLOT, TEMPORARY, WALKING,
          // NOT_STEALTH, FORWARD, 50 );
          uiRange = CardinalSpacesAway(sSweetGridNo, sGridNo);

          //	if ( uiRange == 0 )
          //	{
          //		uiRange = 999999;
          //	}

          if (uiRange < uiLowestRange) {
            sLowestGridNo = sGridNo;
            uiLowestRange = uiRange;
            fFound = TRUE;
          }
        }
      }
    }
  }
  gubNPCAPBudget = ubSaveNPCAPBudget;
  gubNPCDistLimit = ubSaveNPCDistLimit;
  if (fFound) {
    // Set direction to center of map!
    *pubDirection = (uint8_t)GetDirectionToGridNoFromGridNo(
        sLowestGridNo, (((WORLD_ROWS / 2) * WORLD_COLS) + (WORLD_COLS / 2)));
    return (sLowestGridNo);
  } else {
    return (NOWHERE);
  }
}

uint16_t FindGridNoFromSweetSpotThroughPeople(struct SOLDIERTYPE *pSoldier, int16_t sSweetGridNo,
                                            int8_t ubRadius, uint8_t *pubDirection) {
  int16_t sTop, sBottom;
  int16_t sLeft, sRight;
  int16_t cnt1, cnt2;
  int16_t sGridNo;
  int32_t uiRange, uiLowestRange = 999999;
  int16_t sLowestGridNo = -1;
  int32_t leftmost;
  BOOLEAN fFound = FALSE;
  struct SOLDIERTYPE soldier;
  uint8_t ubSaveNPCAPBudget;
  uint8_t ubSaveNPCDistLimit;

  // Save AI pathing vars.  changing the distlimit restricts how
  // far away the pathing will consider.
  ubSaveNPCAPBudget = gubNPCAPBudget;
  ubSaveNPCDistLimit = gubNPCDistLimit;
  gubNPCAPBudget = 0;
  gubNPCDistLimit = ubRadius;

  // create dummy soldier, and use the pathing to determine which nearby slots are
  // reachable.
  memset(&soldier, 0, sizeof(struct SOLDIERTYPE));
  soldier.bLevel = 0;
  soldier.bTeam = pSoldier->bTeam;
  soldier.sGridNo = sSweetGridNo;

  sTop = ubRadius;
  sBottom = -ubRadius;
  sLeft = -ubRadius;
  sRight = ubRadius;

  // clear the mapelements of potential residue MAPELEMENT_REACHABLE flags
  // in the square region.
  // ATE: CHECK FOR BOUNDARIES!!!!!!
  for (cnt1 = sBottom; cnt1 <= sTop; cnt1++) {
    leftmost = ((sSweetGridNo + (WORLD_COLS * cnt1)) / WORLD_COLS) * WORLD_COLS;

    for (cnt2 = sLeft; cnt2 <= sRight; cnt2++) {
      sGridNo = sSweetGridNo + (WORLD_COLS * cnt1) + cnt2;
      if (sGridNo >= 0 && sGridNo < WORLD_MAX && sGridNo >= leftmost &&
          sGridNo < (leftmost + WORLD_COLS)) {
        gpWorldLevelData[sGridNo].uiFlags &= (~MAPELEMENT_REACHABLE);
      }
    }
  }

  // Now, find out which of these gridnos are reachable
  //(use the fake soldier and the pathing settings)
  FindBestPath(&soldier, NOWHERE, 0, WALKING, COPYREACHABLE,
               (PATH_IGNORE_PERSON_AT_DEST | PATH_THROUGH_PEOPLE));

  uiLowestRange = 999999;

  for (cnt1 = sBottom; cnt1 <= sTop; cnt1++) {
    leftmost = ((sSweetGridNo + (WORLD_COLS * cnt1)) / WORLD_COLS) * WORLD_COLS;

    for (cnt2 = sLeft; cnt2 <= sRight; cnt2++) {
      sGridNo = sSweetGridNo + (WORLD_COLS * cnt1) + cnt2;
      if (sGridNo >= 0 && sGridNo < WORLD_MAX && sGridNo >= leftmost &&
          sGridNo < (leftmost + WORLD_COLS) &&
          gpWorldLevelData[sGridNo].uiFlags & MAPELEMENT_REACHABLE) {
        // Go on sweet stop
        if (NewOKDestination(pSoldier, sGridNo, TRUE, pSoldier->bLevel)) {
          uiRange = GetRangeInCellCoordsFromGridNoDiff(sSweetGridNo, sGridNo);

          {
            if (uiRange < uiLowestRange) {
              sLowestGridNo = sGridNo;
              uiLowestRange = uiRange;
              fFound = TRUE;
            }
          }
        }
      }
    }
  }
  gubNPCAPBudget = ubSaveNPCAPBudget;
  gubNPCDistLimit = ubSaveNPCDistLimit;
  if (fFound) {
    // Set direction to center of map!
    *pubDirection = (uint8_t)GetDirectionToGridNoFromGridNo(
        sLowestGridNo, (((WORLD_ROWS / 2) * WORLD_COLS) + (WORLD_COLS / 2)));
    return (sLowestGridNo);
  } else {
    return (NOWHERE);
  }
}

// Kris:  modified to actually path from sweetspot to gridno.  Previously, it only checked if the
// destination was sittable (though it was possible that that location would be trapped.
uint16_t FindGridNoFromSweetSpotWithStructData(struct SOLDIERTYPE *pSoldier, uint16_t usAnimState,
                                             int16_t sSweetGridNo, int8_t ubRadius, uint8_t *pubDirection,
                                             BOOLEAN fClosestToMerc) {
  int16_t sTop, sBottom;
  int16_t sLeft, sRight;
  int16_t cnt1, cnt2, cnt3;
  int16_t sGridNo;
  int32_t uiRange, uiLowestRange = 999999;
  int16_t sLowestGridNo = -1;
  int32_t leftmost;
  BOOLEAN fFound = FALSE;
  struct SOLDIERTYPE soldier;
  uint8_t ubSaveNPCAPBudget;
  uint8_t ubSaveNPCDistLimit;
  uint8_t ubBestDirection = 0;

  // Save AI pathing vars.  changing the distlimit restricts how
  // far away the pathing will consider.
  ubSaveNPCAPBudget = gubNPCAPBudget;
  ubSaveNPCDistLimit = gubNPCDistLimit;
  gubNPCAPBudget = 0;
  gubNPCDistLimit = ubRadius;

  // create dummy soldier, and use the pathing to determine which nearby slots are
  // reachable.
  memset(&soldier, 0, sizeof(struct SOLDIERTYPE));
  soldier.bLevel = 0;
  soldier.bTeam = 1;
  soldier.sGridNo = sSweetGridNo;

  sTop = ubRadius;
  sBottom = -ubRadius;
  sLeft = -ubRadius;
  sRight = ubRadius;

  // If we are already at this gridno....
  if (pSoldier->sGridNo == sSweetGridNo && !(pSoldier->uiStatusFlags & SOLDIER_VEHICLE)) {
    *pubDirection = pSoldier->bDirection;
    return (sSweetGridNo);
  }

  // clear the mapelements of potential residue MAPELEMENT_REACHABLE flags
  // in the square region.
  // ATE: CHECK FOR BOUNDARIES!!!!!!
  for (cnt1 = sBottom; cnt1 <= sTop; cnt1++) {
    leftmost = ((sSweetGridNo + (WORLD_COLS * cnt1)) / WORLD_COLS) * WORLD_COLS;

    for (cnt2 = sLeft; cnt2 <= sRight; cnt2++) {
      sGridNo = sSweetGridNo + (WORLD_COLS * cnt1) + cnt2;
      if (sGridNo >= 0 && sGridNo < WORLD_MAX && sGridNo >= leftmost &&
          sGridNo < (leftmost + WORLD_COLS)) {
        gpWorldLevelData[sGridNo].uiFlags &= (~MAPELEMENT_REACHABLE);
      }
    }
  }

  // Now, find out which of these gridnos are reachable
  //(use the fake soldier and the pathing settings)
  FindBestPath(&soldier, NOWHERE, 0, WALKING, COPYREACHABLE,
               (PATH_IGNORE_PERSON_AT_DEST | PATH_THROUGH_PEOPLE));

  uiLowestRange = 999999;

  for (cnt1 = sBottom; cnt1 <= sTop; cnt1++) {
    leftmost = ((sSweetGridNo + (WORLD_COLS * cnt1)) / WORLD_COLS) * WORLD_COLS;

    for (cnt2 = sLeft; cnt2 <= sRight; cnt2++) {
      sGridNo = sSweetGridNo + (WORLD_COLS * cnt1) + cnt2;
      if (sGridNo >= 0 && sGridNo < WORLD_MAX && sGridNo >= leftmost &&
          sGridNo < (leftmost + WORLD_COLS) &&
          gpWorldLevelData[sGridNo].uiFlags & MAPELEMENT_REACHABLE) {
        // Go on sweet stop
        if (NewOKDestination(pSoldier, sGridNo, TRUE, pSoldier->bLevel)) {
          BOOLEAN fDirectionFound = FALSE;
          uint16_t usOKToAddStructID;
          struct STRUCTURE_FILE_REF *pStructureFileRef;
          uint16_t usAnimSurface;

          if (pSoldier->pLevelNode != NULL) {
            if (pSoldier->pLevelNode->pStructureData != NULL) {
              usOKToAddStructID = pSoldier->pLevelNode->pStructureData->usStructureID;
            } else {
              usOKToAddStructID = INVALID_STRUCTURE_ID;
            }
          } else {
            usOKToAddStructID = INVALID_STRUCTURE_ID;
          }

          // Get animation surface...
          usAnimSurface = DetermineSoldierAnimationSurface(pSoldier, usAnimState);
          // Get structure ref...
          pStructureFileRef = GetAnimationStructureRef(pSoldier->ubID, usAnimSurface, usAnimState);

          if (!pStructureFileRef) {
            Assert(0);
          }

          // Check each struct in each direction
          for (cnt3 = 0; cnt3 < 8; cnt3++) {
            if (OkayToAddStructureToWorld(
                    (int16_t)sGridNo, pSoldier->bLevel,
                    &(pStructureFileRef->pDBStructureRef[gOneCDirection[cnt3]]),
                    usOKToAddStructID)) {
              fDirectionFound = TRUE;
              break;
            }
          }

          if (fDirectionFound) {
            if (fClosestToMerc) {
              uiRange = FindBestPath(pSoldier, sGridNo, pSoldier->bLevel,
                                     pSoldier->usUIMovementMode, NO_COPYROUTE, 0);

              if (uiRange == 0) {
                uiRange = 999;
              }
            } else {
              uiRange = GetRangeInCellCoordsFromGridNoDiff(sSweetGridNo, sGridNo);
            }

            if (uiRange < uiLowestRange) {
              ubBestDirection = (uint8_t)cnt3;
              sLowestGridNo = sGridNo;
              uiLowestRange = uiRange;
              fFound = TRUE;
            }
          }
        }
      }
    }
  }
  gubNPCAPBudget = ubSaveNPCAPBudget;
  gubNPCDistLimit = ubSaveNPCDistLimit;
  if (fFound) {
    // Set direction we chose...
    *pubDirection = ubBestDirection;

    return (sLowestGridNo);
  } else {
    return (NOWHERE);
  }
}

uint16_t FindGridNoFromSweetSpotWithStructDataUsingGivenDirectionFirst(
    struct SOLDIERTYPE *pSoldier, uint16_t usAnimState, int16_t sSweetGridNo, int8_t ubRadius,
    uint8_t *pubDirection, BOOLEAN fClosestToMerc, int8_t bGivenDirection) {
  int16_t sTop, sBottom;
  int16_t sLeft, sRight;
  int16_t cnt1, cnt2, cnt3;
  int16_t sGridNo;
  int32_t uiRange, uiLowestRange = 999999;
  int16_t sLowestGridNo = -1;
  int32_t leftmost;
  BOOLEAN fFound = FALSE;
  struct SOLDIERTYPE soldier;
  uint8_t ubSaveNPCAPBudget;
  uint8_t ubSaveNPCDistLimit;
  uint8_t ubBestDirection = 0;

  // Save AI pathing vars.  changing the distlimit restricts how
  // far away the pathing will consider.
  ubSaveNPCAPBudget = gubNPCAPBudget;
  ubSaveNPCDistLimit = gubNPCDistLimit;
  gubNPCAPBudget = 0;
  gubNPCDistLimit = ubRadius;

  // create dummy soldier, and use the pathing to determine which nearby slots are
  // reachable.
  memset(&soldier, 0, sizeof(struct SOLDIERTYPE));
  soldier.bLevel = 0;
  soldier.bTeam = 1;
  soldier.sGridNo = sSweetGridNo;

  sTop = ubRadius;
  sBottom = -ubRadius;
  sLeft = -ubRadius;
  sRight = ubRadius;

  // If we are already at this gridno....
  if (pSoldier->sGridNo == sSweetGridNo && !(pSoldier->uiStatusFlags & SOLDIER_VEHICLE)) {
    *pubDirection = pSoldier->bDirection;
    return (sSweetGridNo);
  }

  // clear the mapelements of potential residue MAPELEMENT_REACHABLE flags
  // in the square region.
  // ATE: CHECK FOR BOUNDARIES!!!!!!
  for (cnt1 = sBottom; cnt1 <= sTop; cnt1++) {
    leftmost = ((sSweetGridNo + (WORLD_COLS * cnt1)) / WORLD_COLS) * WORLD_COLS;

    for (cnt2 = sLeft; cnt2 <= sRight; cnt2++) {
      sGridNo = sSweetGridNo + (WORLD_COLS * cnt1) + cnt2;
      if (sGridNo >= 0 && sGridNo < WORLD_MAX && sGridNo >= leftmost &&
          sGridNo < (leftmost + WORLD_COLS)) {
        gpWorldLevelData[sGridNo].uiFlags &= (~MAPELEMENT_REACHABLE);
      }
    }
  }

  // Now, find out which of these gridnos are reachable
  //(use the fake soldier and the pathing settings)
  FindBestPath(&soldier, NOWHERE, 0, WALKING, COPYREACHABLE,
               (PATH_IGNORE_PERSON_AT_DEST | PATH_THROUGH_PEOPLE));

  uiLowestRange = 999999;

  for (cnt1 = sBottom; cnt1 <= sTop; cnt1++) {
    leftmost = ((sSweetGridNo + (WORLD_COLS * cnt1)) / WORLD_COLS) * WORLD_COLS;

    for (cnt2 = sLeft; cnt2 <= sRight; cnt2++) {
      sGridNo = sSweetGridNo + (WORLD_COLS * cnt1) + cnt2;
      if (sGridNo >= 0 && sGridNo < WORLD_MAX && sGridNo >= leftmost &&
          sGridNo < (leftmost + WORLD_COLS) &&
          gpWorldLevelData[sGridNo].uiFlags & MAPELEMENT_REACHABLE) {
        // Go on sweet stop
        if (NewOKDestination(pSoldier, sGridNo, TRUE, pSoldier->bLevel)) {
          BOOLEAN fDirectionFound = FALSE;
          uint16_t usOKToAddStructID;
          struct STRUCTURE_FILE_REF *pStructureFileRef;
          uint16_t usAnimSurface;

          if (pSoldier->pLevelNode != NULL) {
            if (pSoldier->pLevelNode->pStructureData != NULL) {
              usOKToAddStructID = pSoldier->pLevelNode->pStructureData->usStructureID;
            } else {
              usOKToAddStructID = INVALID_STRUCTURE_ID;
            }
          } else {
            usOKToAddStructID = INVALID_STRUCTURE_ID;
          }

          // Get animation surface...
          usAnimSurface = DetermineSoldierAnimationSurface(pSoldier, usAnimState);
          // Get structure ref...
          pStructureFileRef = GetAnimationStructureRef(pSoldier->ubID, usAnimSurface, usAnimState);

          if (!pStructureFileRef) {
            Assert(0);
          }

          // OK, check the perfered given direction first
          if (OkayToAddStructureToWorld(
                  (int16_t)sGridNo, pSoldier->bLevel,
                  &(pStructureFileRef->pDBStructureRef[gOneCDirection[bGivenDirection]]),
                  usOKToAddStructID)) {
            fDirectionFound = TRUE;
            cnt3 = bGivenDirection;
          } else {
            // Check each struct in each direction
            for (cnt3 = 0; cnt3 < 8; cnt3++) {
              if (cnt3 != bGivenDirection) {
                if (OkayToAddStructureToWorld(
                        (int16_t)sGridNo, pSoldier->bLevel,
                        &(pStructureFileRef->pDBStructureRef[gOneCDirection[cnt3]]),
                        usOKToAddStructID)) {
                  fDirectionFound = TRUE;
                  break;
                }
              }
            }
          }

          if (fDirectionFound) {
            if (fClosestToMerc) {
              uiRange = FindBestPath(pSoldier, sGridNo, pSoldier->bLevel,
                                     pSoldier->usUIMovementMode, NO_COPYROUTE, 0);

              if (uiRange == 0) {
                uiRange = 999;
              }
            } else {
              uiRange = GetRangeInCellCoordsFromGridNoDiff(sSweetGridNo, sGridNo);
            }

            if (uiRange < uiLowestRange) {
              ubBestDirection = (uint8_t)cnt3;
              sLowestGridNo = sGridNo;
              uiLowestRange = uiRange;
              fFound = TRUE;
            }
          }
        }
      }
    }
  }
  gubNPCAPBudget = ubSaveNPCAPBudget;
  gubNPCDistLimit = ubSaveNPCDistLimit;
  if (fFound) {
    // Set direction we chose...
    *pubDirection = ubBestDirection;

    return (sLowestGridNo);
  } else {
    return (NOWHERE);
  }
}

uint16_t FindGridNoFromSweetSpotWithStructDataFromSoldier(struct SOLDIERTYPE *pSoldier,
                                                        uint16_t usAnimState, int8_t ubRadius,
                                                        uint8_t *pubDirection, BOOLEAN fClosestToMerc,
                                                        struct SOLDIERTYPE *pSrcSoldier) {
  int16_t sTop, sBottom;
  int16_t sLeft, sRight;
  int16_t cnt1, cnt2, cnt3;
  int16_t sGridNo;
  int32_t uiRange, uiLowestRange = 999999;
  int16_t sLowestGridNo = -1;
  int32_t leftmost;
  BOOLEAN fFound = FALSE;
  uint8_t ubSaveNPCAPBudget;
  uint8_t ubSaveNPCDistLimit;
  uint8_t ubBestDirection = 0;
  int16_t sSweetGridNo;
  struct SOLDIERTYPE soldier;

  sSweetGridNo = pSrcSoldier->sGridNo;

  // Save AI pathing vars.  changing the distlimit restricts how
  // far away the pathing will consider.
  ubSaveNPCAPBudget = gubNPCAPBudget;
  ubSaveNPCDistLimit = gubNPCDistLimit;
  gubNPCAPBudget = 0;
  gubNPCDistLimit = ubRadius;

  // create dummy soldier, and use the pathing to determine which nearby slots are
  // reachable.
  memset(&soldier, 0, sizeof(struct SOLDIERTYPE));
  soldier.bLevel = 0;
  soldier.bTeam = 1;
  soldier.sGridNo = sSweetGridNo;

  sTop = ubRadius;
  sBottom = -ubRadius;
  sLeft = -ubRadius;
  sRight = ubRadius;

  // clear the mapelements of potential residue MAPELEMENT_REACHABLE flags
  // in the square region.
  // ATE: CHECK FOR BOUNDARIES!!!!!!
  for (cnt1 = sBottom; cnt1 <= sTop; cnt1++) {
    leftmost = ((sSweetGridNo + (WORLD_COLS * cnt1)) / WORLD_COLS) * WORLD_COLS;

    for (cnt2 = sLeft; cnt2 <= sRight; cnt2++) {
      sGridNo = sSweetGridNo + (WORLD_COLS * cnt1) + cnt2;
      if (sGridNo >= 0 && sGridNo < WORLD_MAX && sGridNo >= leftmost &&
          sGridNo < (leftmost + WORLD_COLS)) {
        gpWorldLevelData[sGridNo].uiFlags &= (~MAPELEMENT_REACHABLE);
      }
    }
  }

  // Now, find out which of these gridnos are reachable
  FindBestPath(&soldier, NOWHERE, 0, WALKING, COPYREACHABLE,
               (PATH_IGNORE_PERSON_AT_DEST | PATH_THROUGH_PEOPLE));

  uiLowestRange = 999999;

  for (cnt1 = sBottom; cnt1 <= sTop; cnt1++) {
    leftmost = ((sSweetGridNo + (WORLD_COLS * cnt1)) / WORLD_COLS) * WORLD_COLS;

    for (cnt2 = sLeft; cnt2 <= sRight; cnt2++) {
      sGridNo = sSweetGridNo + (WORLD_COLS * cnt1) + cnt2;
      if (sGridNo >= 0 && sGridNo < WORLD_MAX && sGridNo >= leftmost &&
          sGridNo < (leftmost + WORLD_COLS) &&
          gpWorldLevelData[sGridNo].uiFlags & MAPELEMENT_REACHABLE) {
        // Go on sweet stop
        if (NewOKDestination(pSoldier, sGridNo, TRUE, pSoldier->bLevel)) {
          BOOLEAN fDirectionFound = FALSE;
          uint16_t usOKToAddStructID;
          struct STRUCTURE_FILE_REF *pStructureFileRef;
          uint16_t usAnimSurface;

          if (fClosestToMerc != 3) {
            if (pSoldier->pLevelNode != NULL && pSoldier->pLevelNode->pStructureData != NULL) {
              usOKToAddStructID = pSoldier->pLevelNode->pStructureData->usStructureID;
            } else {
              usOKToAddStructID = INVALID_STRUCTURE_ID;
            }

            // Get animation surface...
            usAnimSurface = DetermineSoldierAnimationSurface(pSoldier, usAnimState);
            // Get structure ref...
            pStructureFileRef =
                GetAnimationStructureRef(pSoldier->ubID, usAnimSurface, usAnimState);

            // Check each struct in each direction
            for (cnt3 = 0; cnt3 < 8; cnt3++) {
              if (OkayToAddStructureToWorld(
                      (int16_t)sGridNo, pSoldier->bLevel,
                      &(pStructureFileRef->pDBStructureRef[gOneCDirection[cnt3]]),
                      usOKToAddStructID)) {
                fDirectionFound = TRUE;
                break;
              }
            }
          } else {
            fDirectionFound = TRUE;
            cnt3 = (uint8_t)Random(8);
          }

          if (fDirectionFound) {
            if (fClosestToMerc == 1) {
              uiRange = GetRangeInCellCoordsFromGridNoDiff(pSoldier->sGridNo, sGridNo);
            } else if (fClosestToMerc == 2) {
              uiRange = GetRangeInCellCoordsFromGridNoDiff(pSoldier->sGridNo, sGridNo) +
                        GetRangeInCellCoordsFromGridNoDiff(sSweetGridNo, sGridNo);
            } else {
              // uiRange = GetRangeInCellCoordsFromGridNoDiff( sSweetGridNo, sGridNo );
              uiRange = abs((sSweetGridNo / MAXCOL) - (sGridNo / MAXCOL)) +
                        abs((sSweetGridNo % MAXROW) - (sGridNo % MAXROW));
            }

            if (uiRange < uiLowestRange || (uiRange == uiLowestRange &&
                                            PythSpacesAway(pSoldier->sGridNo, sGridNo) <
                                                PythSpacesAway(pSoldier->sGridNo, sLowestGridNo))) {
              ubBestDirection = (uint8_t)cnt3;
              sLowestGridNo = sGridNo;
              uiLowestRange = uiRange;
              fFound = TRUE;
            }
          }
        }
      }
    }
  }
  gubNPCAPBudget = ubSaveNPCAPBudget;
  gubNPCDistLimit = ubSaveNPCDistLimit;
  if (fFound) {
    // Set direction we chose...
    *pubDirection = ubBestDirection;

    return (sLowestGridNo);
  } else {
    return (NOWHERE);
  }
}

uint16_t FindGridNoFromSweetSpotExcludingSweetSpot(struct SOLDIERTYPE *pSoldier, int16_t sSweetGridNo,
                                                 int8_t ubRadius, uint8_t *pubDirection) {
  int16_t sTop, sBottom;
  int16_t sLeft, sRight;
  int16_t cnt1, cnt2;
  int16_t sGridNo;
  int32_t uiRange, uiLowestRange = 999999;
  int16_t sLowestGridNo = -1;
  int32_t leftmost;
  BOOLEAN fFound = FALSE;

  sTop = ubRadius;
  sBottom = -ubRadius;
  sLeft = -ubRadius;
  sRight = ubRadius;

  uiLowestRange = 999999;

  for (cnt1 = sBottom; cnt1 <= sTop; cnt1++) {
    leftmost = ((sSweetGridNo + (WORLD_COLS * cnt1)) / WORLD_COLS) * WORLD_COLS;

    for (cnt2 = sLeft; cnt2 <= sRight; cnt2++) {
      sGridNo = sSweetGridNo + (WORLD_COLS * cnt1) + cnt2;

      if (sSweetGridNo == sGridNo) {
        continue;
      }

      if (sGridNo >= 0 && sGridNo < WORLD_MAX && sGridNo >= leftmost &&
          sGridNo < (leftmost + WORLD_COLS)) {
        // Go on sweet stop
        if (NewOKDestination(pSoldier, sGridNo, TRUE, pSoldier->bLevel)) {
          uiRange = GetRangeInCellCoordsFromGridNoDiff(sSweetGridNo, sGridNo);

          if (uiRange < uiLowestRange) {
            sLowestGridNo = sGridNo;
            uiLowestRange = uiRange;

            fFound = TRUE;
          }
        }
      }
    }
  }

  if (fFound) {
    // Set direction to center of map!
    *pubDirection = (uint8_t)GetDirectionToGridNoFromGridNo(
        sLowestGridNo, (((WORLD_ROWS / 2) * WORLD_COLS) + (WORLD_COLS / 2)));

    return (sLowestGridNo);
  } else {
    return (NOWHERE);
  }
}

uint16_t FindGridNoFromSweetSpotExcludingSweetSpotInQuardent(struct SOLDIERTYPE *pSoldier,
                                                           int16_t sSweetGridNo, int8_t ubRadius,
                                                           uint8_t *pubDirection,
                                                           int8_t ubQuardentDir) {
  int16_t sTop, sBottom;
  int16_t sLeft, sRight;
  int16_t cnt1, cnt2;
  int16_t sGridNo;
  int32_t uiRange, uiLowestRange = 999999;
  int16_t sLowestGridNo = -1;
  int32_t leftmost;
  BOOLEAN fFound = FALSE;

  sTop = ubRadius;
  sBottom = -ubRadius;
  sLeft = -ubRadius;
  sRight = ubRadius;

  // Switch on quadrent
  if (ubQuardentDir == SOUTHEAST) {
    sBottom = 0;
    sLeft = 0;
  }

  uiLowestRange = 999999;

  for (cnt1 = sBottom; cnt1 <= sTop; cnt1++) {
    leftmost = ((sSweetGridNo + (WORLD_COLS * cnt1)) / WORLD_COLS) * WORLD_COLS;

    for (cnt2 = sLeft; cnt2 <= sRight; cnt2++) {
      sGridNo = sSweetGridNo + (WORLD_COLS * cnt1) + cnt2;

      if (sSweetGridNo == sGridNo) {
        continue;
      }

      if (sGridNo >= 0 && sGridNo < WORLD_MAX && sGridNo >= leftmost &&
          sGridNo < (leftmost + WORLD_COLS)) {
        // Go on sweet stop
        if (NewOKDestination(pSoldier, sGridNo, TRUE, pSoldier->bLevel)) {
          uiRange = GetRangeInCellCoordsFromGridNoDiff(sSweetGridNo, sGridNo);

          if (uiRange < uiLowestRange) {
            sLowestGridNo = sGridNo;
            uiLowestRange = uiRange;
            fFound = TRUE;
          }
        }
      }
    }
  }

  if (fFound) {
    // Set direction to center of map!
    *pubDirection = (uint8_t)GetDirectionToGridNoFromGridNo(
        sLowestGridNo, (((WORLD_ROWS / 2) * WORLD_COLS) + (WORLD_COLS / 2)));

    return (sLowestGridNo);
  } else {
    return (NOWHERE);
  }
}

BOOLEAN CanSoldierReachGridNoInGivenTileLimit(struct SOLDIERTYPE *pSoldier, int16_t sGridNo,
                                              int16_t sMaxTiles, int8_t bLevel) {
  int32_t iNumTiles;
  int16_t sActionGridNo;
  uint8_t ubDirection;

  if (pSoldier->bLevel != bLevel) {
    return (FALSE);
  }

  sActionGridNo = FindAdjacentGridEx(pSoldier, sGridNo, &ubDirection, NULL, FALSE, FALSE);

  if (sActionGridNo == -1) {
    sActionGridNo = sGridNo;
  }

  if (sActionGridNo == pSoldier->sGridNo) {
    return (TRUE);
  }

  iNumTiles = FindBestPath(pSoldier, sActionGridNo, pSoldier->bLevel, WALKING, NO_COPYROUTE,
                           PATH_IGNORE_PERSON_AT_DEST);

  if (iNumTiles <= sMaxTiles && iNumTiles != 0) {
    return (TRUE);
  } else {
    return (FALSE);
  }
}

uint16_t FindRandomGridNoFromSweetSpot(struct SOLDIERTYPE *pSoldier, int16_t sSweetGridNo,
                                     int8_t ubRadius, uint8_t *pubDirection) {
  int16_t sX, sY;
  int16_t sGridNo;
  int32_t leftmost;
  BOOLEAN fFound = FALSE;
  uint32_t cnt = 0;
  struct SOLDIERTYPE soldier;
  uint8_t ubSaveNPCAPBudget;
  uint8_t ubSaveNPCDistLimit;
  int16_t sTop, sBottom;
  int16_t sLeft, sRight;
  int16_t cnt1, cnt2;
  uint8_t ubRoomNum;

  // Save AI pathing vars.  changing the distlimit restricts how
  // far away the pathing will consider.
  ubSaveNPCAPBudget = gubNPCAPBudget;
  ubSaveNPCDistLimit = gubNPCDistLimit;
  gubNPCAPBudget = 0;
  gubNPCDistLimit = ubRadius;

  // create dummy soldier, and use the pathing to determine which nearby slots are
  // reachable.
  memset(&soldier, 0, sizeof(struct SOLDIERTYPE));
  soldier.bLevel = 0;
  soldier.bTeam = 1;
  soldier.sGridNo = sSweetGridNo;

  sTop = ubRadius;
  sBottom = -ubRadius;
  sLeft = -ubRadius;
  sRight = ubRadius;

  // ATE: CHECK FOR BOUNDARIES!!!!!!
  for (cnt1 = sBottom; cnt1 <= sTop; cnt1++) {
    leftmost = ((sSweetGridNo + (WORLD_COLS * cnt1)) / WORLD_COLS) * WORLD_COLS;

    for (cnt2 = sLeft; cnt2 <= sRight; cnt2++) {
      sGridNo = sSweetGridNo + (WORLD_COLS * cnt1) + cnt2;
      if (sGridNo >= 0 && sGridNo < WORLD_MAX && sGridNo >= leftmost &&
          sGridNo < (leftmost + WORLD_COLS)) {
        gpWorldLevelData[sGridNo].uiFlags &= (~MAPELEMENT_REACHABLE);
      }
    }
  }

  // Now, find out which of these gridnos are reachable
  //(use the fake soldier and the pathing settings)
  FindBestPath(&soldier, NOWHERE, 0, WALKING, COPYREACHABLE,
               (PATH_IGNORE_PERSON_AT_DEST | PATH_THROUGH_PEOPLE));

  do {
    sX = (uint16_t)Random(ubRadius);
    sY = (uint16_t)Random(ubRadius);

    leftmost = ((sSweetGridNo + (WORLD_COLS * sY)) / WORLD_COLS) * WORLD_COLS;

    sGridNo = sSweetGridNo + (WORLD_COLS * sY) + sX;

    if (sGridNo >= 0 && sGridNo < WORLD_MAX && sGridNo >= leftmost &&
        sGridNo < (leftmost + WORLD_COLS) &&
        gpWorldLevelData[sGridNo].uiFlags & MAPELEMENT_REACHABLE) {
      // Go on sweet stop
      if (NewOKDestination(pSoldier, sGridNo, TRUE, pSoldier->bLevel)) {
        // If we are a crow, we need this additional check
        if (pSoldier->ubBodyType == CROW) {
          if (!InARoom(sGridNo, &ubRoomNum)) {
            fFound = TRUE;
          }
        } else {
          fFound = TRUE;
        }
      }
    }

    cnt++;

    if (cnt > 2000) {
      return (NOWHERE);
    }

  } while (!fFound);

  // Set direction to center of map!
  *pubDirection = (uint8_t)GetDirectionToGridNoFromGridNo(
      sGridNo, (((WORLD_ROWS / 2) * WORLD_COLS) + (WORLD_COLS / 2)));

  gubNPCAPBudget = ubSaveNPCAPBudget;
  gubNPCDistLimit = ubSaveNPCDistLimit;

  return (sGridNo);
}

uint16_t FindRandomGridNoFromSweetSpotExcludingSweetSpot(struct SOLDIERTYPE *pSoldier,
                                                       int16_t sSweetGridNo, int8_t ubRadius,
                                                       uint8_t *pubDirection) {
  int16_t sX, sY;
  int16_t sGridNo;
  int32_t leftmost;
  BOOLEAN fFound = FALSE;
  uint32_t cnt = 0;

  do {
    sX = (uint16_t)Random(ubRadius);
    sY = (uint16_t)Random(ubRadius);

    leftmost = ((sSweetGridNo + (WORLD_COLS * sY)) / WORLD_COLS) * WORLD_COLS;

    sGridNo = sSweetGridNo + (WORLD_COLS * sY) + sX;

    if (sGridNo == sSweetGridNo) {
      continue;
    }

    if (sGridNo >= 0 && sGridNo < WORLD_MAX && sGridNo >= leftmost &&
        sGridNo < (leftmost + WORLD_COLS)) {
      // Go on sweet stop
      if (NewOKDestination(pSoldier, sGridNo, TRUE, pSoldier->bLevel)) {
        fFound = TRUE;
      }
    }

    cnt++;

    if (cnt > 2000) {
      return (NOWHERE);
    }

  } while (!fFound);

  // Set direction to center of map!
  *pubDirection = (uint8_t)GetDirectionToGridNoFromGridNo(
      sGridNo, (((WORLD_ROWS / 2) * WORLD_COLS) + (WORLD_COLS / 2)));

  return (sGridNo);
}

BOOLEAN InternalAddSoldierToSector(uint8_t ubID, BOOLEAN fCalculateDirection, BOOLEAN fUseAnimation,
                                   uint16_t usAnimState, uint16_t usAnimCode) {
  uint8_t ubDirection, ubCalculatedDirection;
  struct SOLDIERTYPE *pSoldier;
  int16_t sGridNo;
  int16_t sExitGridNo;

  pSoldier = MercPtrs[ubID];

  if (IsSolActive(pSoldier)) {
    // ATE: Make sure life of elliot is OK if from a meanwhile
    if (AreInMeanwhile() && GetSolProfile(pSoldier) == ELLIOT) {
      if (pSoldier->bLife < OKLIFE) {
        pSoldier->bLife = 25;
      }
    }

    // ADD SOLDIER TO SLOT!
    if (pSoldier->uiStatusFlags & SOLDIER_OFF_MAP) {
      AddAwaySlot(pSoldier);

      // Guy is NOT "in sector"
      pSoldier->bInSector = FALSE;

    } else {
      AddMercSlot(pSoldier);

      // Add guy to sector flag
      pSoldier->bInSector = TRUE;
    }

    // If a driver or passenger - stop here!
    if (pSoldier->uiStatusFlags & SOLDIER_DRIVER || pSoldier->uiStatusFlags & SOLDIER_PASSENGER) {
      return (FALSE);
    }

    // Add to panel
    CheckForAndAddMercToTeamPanel(pSoldier);

    pSoldier->usQuoteSaidFlags &= (~SOLDIER_QUOTE_SAID_SPOTTING_CREATURE_ATTACK);
    pSoldier->usQuoteSaidFlags &= (~SOLDIER_QUOTE_SAID_SMELLED_CREATURE);
    pSoldier->usQuoteSaidFlags &= (~SOLDIER_QUOTE_SAID_WORRIED_ABOUT_CREATURES);

    // Add to interface if the are ours
    if (pSoldier->bTeam == CREATURE_TEAM) {
      sGridNo = FindGridNoFromSweetSpotWithStructData(
          pSoldier, STANDING, pSoldier->sInsertionGridNo, 7, &ubCalculatedDirection, FALSE);
      if (fCalculateDirection)
        ubDirection = ubCalculatedDirection;
      else
        ubDirection = pSoldier->ubInsertionDirection;
    } else {
      if (pSoldier->sInsertionGridNo ==
          NOWHERE) {  // Add the soldier to the respective entrypoint.  This is an error condition.
      }
      if (pSoldier->uiStatusFlags & SOLDIER_VEHICLE) {
        sGridNo = FindGridNoFromSweetSpotWithStructDataUsingGivenDirectionFirst(
            pSoldier, STANDING, pSoldier->sInsertionGridNo, 12, &ubCalculatedDirection, FALSE,
            pSoldier->ubInsertionDirection);
        // ATE: Override insertion direction
        pSoldier->ubInsertionDirection = ubCalculatedDirection;
      } else {
        sGridNo = FindGridNoFromSweetSpot(pSoldier, pSoldier->sInsertionGridNo, 7,
                                          &ubCalculatedDirection);

        // ATE: Error condition - if nowhere use insertion gridno!
        if (sGridNo == NOWHERE) {
          sGridNo = pSoldier->sInsertionGridNo;
        }
      }

      // Override calculated direction if we were told to....
      if (pSoldier->ubInsertionDirection > 100) {
        pSoldier->ubInsertionDirection = pSoldier->ubInsertionDirection - 100;
        fCalculateDirection = FALSE;
      }

      if (fCalculateDirection) {
        ubDirection = ubCalculatedDirection;

        // Check if we need to get direction from exit grid...
        if (pSoldier->bUseExitGridForReentryDirection) {
          pSoldier->bUseExitGridForReentryDirection = FALSE;

          // OK, we know there must be an exit gridno SOMEWHERE close...
          sExitGridNo = FindClosestExitGrid(pSoldier, sGridNo, 10);

          if (sExitGridNo != NOWHERE) {
            // We found one
            // Calculate direction...
            ubDirection = (uint8_t)GetDirectionToGridNoFromGridNo(sExitGridNo, sGridNo);
          }
        }
      } else {
        ubDirection = pSoldier->ubInsertionDirection;
      }
    }

    // Add
    if (gTacticalStatus.uiFlags & LOADING_SAVED_GAME)
      AddSoldierToSectorGridNo(pSoldier, sGridNo, pSoldier->bDirection, fUseAnimation, usAnimState,
                               usAnimCode);
    else
      AddSoldierToSectorGridNo(pSoldier, sGridNo, ubDirection, fUseAnimation, usAnimState,
                               usAnimCode);

    CheckForPotentialAddToBattleIncrement(pSoldier);

    return (TRUE);
  }

  return (FALSE);
}

BOOLEAN AddSoldierToSector(uint8_t ubID) {
  return (InternalAddSoldierToSector(ubID, TRUE, FALSE, 0, 0));
}

BOOLEAN AddSoldierToSectorNoCalculateDirection(uint8_t ubID) {
  return (InternalAddSoldierToSector(ubID, FALSE, FALSE, 0, 0));
}

BOOLEAN AddSoldierToSectorNoCalculateDirectionUseAnimation(uint8_t ubID, uint16_t usAnimState,
                                                           uint16_t usAnimCode) {
  return (InternalAddSoldierToSector(ubID, FALSE, TRUE, usAnimState, usAnimCode));
}

void InternalSoldierInSectorSleep(struct SOLDIERTYPE *pSoldier, int16_t sGridNo,
                                  BOOLEAN fDoTransition) {
  int16_t sWorldX, sWorldY;
  uint8_t ubNewDirection;
  int16_t sGoodGridNo;
  uint16_t usAnim = SLEEPING;

  if (!pSoldier->bInSector) {
    return;
  }

  if (AM_AN_EPC(pSoldier)) {
    usAnim = STANDING;
  }

  // OK, look for sutable placement....
  sGoodGridNo =
      FindGridNoFromSweetSpotWithStructData(pSoldier, usAnim, sGridNo, 5, &ubNewDirection, FALSE);

  sWorldX = CenterX(sGoodGridNo);
  sWorldY = CenterY(sGoodGridNo);

  EVENT_SetSoldierPosition(pSoldier, sWorldX, sWorldY);

  EVENT_SetSoldierDirection(pSoldier, ubNewDirection);
  EVENT_SetSoldierDesiredDirection(pSoldier, ubNewDirection);

  // pSoldier->bDesiredDirection = pSoldier->bDirection;

  if (AM_AN_EPC(pSoldier)) {
    EVENT_InitNewSoldierAnim(pSoldier, STANDING, 1, TRUE);
  } else {
    if (fDoTransition) {
      EVENT_InitNewSoldierAnim(pSoldier, GOTO_SLEEP, 1, TRUE);
    } else {
      EVENT_InitNewSoldierAnim(pSoldier, SLEEPING, 1, TRUE);
    }
  }
}

void SoldierInSectorIncompaciated(struct SOLDIERTYPE *pSoldier, int16_t sGridNo) {
  int16_t sWorldX, sWorldY;
  uint8_t ubNewDirection;
  int16_t sGoodGridNo;

  if (!pSoldier->bInSector) {
    return;
  }

  // OK, look for sutable placement....
  sGoodGridNo = FindGridNoFromSweetSpotWithStructData(pSoldier, STAND_FALLFORWARD_STOP, sGridNo, 5,
                                                      &ubNewDirection, FALSE);

  sWorldX = CenterX(sGoodGridNo);
  sWorldY = CenterY(sGoodGridNo);

  EVENT_SetSoldierPosition(pSoldier, sWorldX, sWorldY);

  EVENT_SetSoldierDirection(pSoldier, ubNewDirection);
  EVENT_SetSoldierDesiredDirection(pSoldier, ubNewDirection);

  // pSoldier->bDesiredDirection = pSoldier->bDirection;

  EVENT_InitNewSoldierAnim(pSoldier, STAND_FALLFORWARD_STOP, 1, TRUE);
}

/*
void SoldierInSectorSleep( struct SOLDIERTYPE *pSoldier, int16_t sGridNo )
{
        InternalSoldierInSectorSleep( pSoldier, sGridNo, TRUE );
}
*/

void SoldierInSectorPatient(struct SOLDIERTYPE *pSoldier, int16_t sGridNo) {
  int16_t sWorldX, sWorldY;
  uint8_t ubNewDirection;
  int16_t sGoodGridNo;

  if (!pSoldier->bInSector) {
    return;
  }

  // OK, look for sutable placement....
  sGoodGridNo = FindGridNoFromSweetSpotWithStructData(pSoldier, BEING_PATIENT, sGridNo, 5,
                                                      &ubNewDirection, FALSE);

  sWorldX = CenterX(sGoodGridNo);
  sWorldY = CenterY(sGoodGridNo);

  EVENT_SetSoldierPosition(pSoldier, sWorldX, sWorldY);

  EVENT_SetSoldierDirection(pSoldier, ubNewDirection);
  EVENT_SetSoldierDesiredDirection(pSoldier, ubNewDirection);

  // pSoldier->bDesiredDirection = pSoldier->bDirection;

  if (!IS_MERC_BODY_TYPE(pSoldier)) {
    EVENT_InitNewSoldierAnim(pSoldier, STANDING, 1, TRUE);
  } else {
    EVENT_InitNewSoldierAnim(pSoldier, BEING_PATIENT, 1, TRUE);
  }
}

void SoldierInSectorDoctor(struct SOLDIERTYPE *pSoldier, int16_t sGridNo) {
  int16_t sWorldX, sWorldY;
  uint8_t ubNewDirection;
  int16_t sGoodGridNo;

  if (!pSoldier->bInSector) {
    return;
  }

  // OK, look for sutable placement....
  sGoodGridNo = FindGridNoFromSweetSpotWithStructData(pSoldier, BEING_DOCTOR, sGridNo, 5,
                                                      &ubNewDirection, FALSE);

  sWorldX = CenterX(sGoodGridNo);
  sWorldY = CenterY(sGoodGridNo);

  EVENT_SetSoldierPosition(pSoldier, sWorldX, sWorldY);

  EVENT_SetSoldierDirection(pSoldier, ubNewDirection);
  EVENT_SetSoldierDesiredDirection(pSoldier, ubNewDirection);

  // pSoldier->bDesiredDirection = pSoldier->bDirection;

  if (!IS_MERC_BODY_TYPE(pSoldier)) {
    EVENT_InitNewSoldierAnim(pSoldier, STANDING, 1, TRUE);
  } else {
    EVENT_InitNewSoldierAnim(pSoldier, BEING_DOCTOR, 1, TRUE);
  }
}

void SoldierInSectorRepair(struct SOLDIERTYPE *pSoldier, int16_t sGridNo) {
  int16_t sWorldX, sWorldY;
  uint8_t ubNewDirection;
  int16_t sGoodGridNo;

  if (!pSoldier->bInSector) {
    return;
  }

  // OK, look for sutable placement....
  sGoodGridNo = FindGridNoFromSweetSpotWithStructData(pSoldier, BEING_REPAIRMAN, sGridNo, 5,
                                                      &ubNewDirection, FALSE);

  sWorldX = CenterX(sGoodGridNo);
  sWorldY = CenterY(sGoodGridNo);

  EVENT_SetSoldierPosition(pSoldier, sWorldX, sWorldY);

  EVENT_SetSoldierDirection(pSoldier, ubNewDirection);
  EVENT_SetSoldierDesiredDirection(pSoldier, ubNewDirection);

  // pSoldier->bDesiredDirection = pSoldier->bDirection;

  if (!IS_MERC_BODY_TYPE(pSoldier)) {
    EVENT_InitNewSoldierAnim(pSoldier, STANDING, 1, TRUE);
  } else {
    EVENT_InitNewSoldierAnim(pSoldier, BEING_REPAIRMAN, 1, TRUE);
  }
}

extern void EVENT_SetSoldierPositionAndMaybeFinalDestAndMaybeNotDestination(
    struct SOLDIERTYPE *pSoldier, float dNewXPos, float dNewYPos, BOOLEAN fUpdateDest,
    BOOLEAN fUpdateFinalDest);

void AddSoldierToSectorGridNo(struct SOLDIERTYPE *pSoldier, int16_t sGridNo, uint8_t ubDirection,
                              BOOLEAN fUseAnimation, uint16_t usAnimState, uint16_t usAnimCode) {
  int16_t sWorldX, sWorldY;
  int16_t sNewGridNo;
  uint8_t ubNewDirection;
  uint8_t ubInsertionCode;
  BOOLEAN fUpdateFinalPosition = TRUE;

  // Add merc to gridno
  sWorldX = CenterX(sGridNo);
  sWorldY = CenterY(sGridNo);

  // Set reserved location!
  pSoldier->sReservedMovementGridNo = NOWHERE;

  // Save OLD insertion code.. as this can change...
  ubInsertionCode = pSoldier->ubStrategicInsertionCode;

  // Remove any pending animations
  pSoldier->usPendingAnimation = NO_PENDING_ANIMATION;
  pSoldier->usPendingAnimation2 = NO_PENDING_ANIMATION;
  pSoldier->ubPendingDirection = NO_PENDING_DIRECTION;
  pSoldier->ubPendingAction = NO_PENDING_ACTION;

  // If we are not loading a saved game
  if ((gTacticalStatus.uiFlags & LOADING_SAVED_GAME)) {
    // Set final dest to be the same...
    fUpdateFinalPosition = FALSE;
  }

  // If this is a special insertion location, get path!
  if (ubInsertionCode == INSERTION_CODE_ARRIVING_GAME) {
    EVENT_SetSoldierPositionAndMaybeFinalDestAndMaybeNotDestination(
        pSoldier, sWorldX, sWorldY, fUpdateFinalPosition, fUpdateFinalPosition);
    EVENT_SetSoldierDirection(pSoldier, ubDirection);
    EVENT_SetSoldierDesiredDirection(pSoldier, ubDirection);
  } else if (ubInsertionCode == INSERTION_CODE_CHOPPER) {
  } else {
    EVENT_SetSoldierPositionAndMaybeFinalDestAndMaybeNotDestination(
        pSoldier, sWorldX, sWorldY, fUpdateFinalPosition, fUpdateFinalPosition);

    // if we are loading, dont set the direction ( they are already set )
    if (!(gTacticalStatus.uiFlags & LOADING_SAVED_GAME)) {
      EVENT_SetSoldierDirection(pSoldier, ubDirection);

      EVENT_SetSoldierDesiredDirection(pSoldier, ubDirection);
    }
  }

  if (!(gTacticalStatus.uiFlags & LOADING_SAVED_GAME)) {
    if (!(pSoldier->uiStatusFlags & SOLDIER_DEAD)) {
      if (pSoldier->bTeam == gbPlayerNum) {
        RevealRoofsAndItems(pSoldier, TRUE, FALSE, pSoldier->bLevel, TRUE);

        // ATE: Patch fix: If we are in an non-interruptable animation, stop!
        if (pSoldier->usAnimState == HOPFENCE) {
          pSoldier->fInNonintAnim = FALSE;
          SoldierGotoStationaryStance(pSoldier);
        }

        EVENT_StopMerc(pSoldier, sGridNo, ubDirection);
      }
    }

    // If just arriving, set a destination to walk into from!
    if (ubInsertionCode == INSERTION_CODE_ARRIVING_GAME) {
      // Find a sweetspot near...
      sNewGridNo =
          FindGridNoFromSweetSpot(pSoldier, gMapInformation.sNorthGridNo, 4, &ubNewDirection);
      EVENT_GetNewSoldierPath(pSoldier, sNewGridNo, WALKING);
    }

    // If he's an enemy... set presence
    if (!pSoldier->bNeutral && (pSoldier->bSide != gbPlayerNum)) {
      // ATE: Added if not bloodcats
      // only do this once they are seen.....
      if (pSoldier->ubBodyType != BLOODCAT) {
        SetEnemyPresence();
      }
    }
  }

  if (!(pSoldier->uiStatusFlags & SOLDIER_DEAD)) {
    // if we are loading a 'pristine' map ( ie, not loading a saved game )
    if (!(gTacticalStatus.uiFlags & LOADING_SAVED_GAME)) {
      // ATE: Double check if we are on the roof that there is a roof there!
      if (pSoldier->bLevel == 1) {
        if (!FindStructure(pSoldier->sGridNo, STRUCTURE_ROOF)) {
          SetSoldierHeight(pSoldier, (float)(0));
        }
      }

      if (ubInsertionCode != INSERTION_CODE_ARRIVING_GAME) {
        // default to standing on arrival
        if (pSoldier->usAnimState != HELIDROP) {
          if (fUseAnimation) {
            EVENT_InitNewSoldierAnim(pSoldier, usAnimState, usAnimCode, TRUE);
          } else if (pSoldier->ubBodyType != CROW) {
            EVENT_InitNewSoldierAnim(pSoldier, STANDING, 1, TRUE);
          }
        }

        // ATE: if we are below OK life, make them lie down!
        if (pSoldier->bLife < OKLIFE) {
          SoldierInSectorIncompaciated(pSoldier, pSoldier->sInsertionGridNo);
        } else if (pSoldier->fMercAsleep == TRUE) {
          InternalSoldierInSectorSleep(pSoldier, pSoldier->sInsertionGridNo, FALSE);
        } else if (GetSolAssignment(pSoldier) == PATIENT) {
          SoldierInSectorPatient(pSoldier, pSoldier->sInsertionGridNo);
        } else if (GetSolAssignment(pSoldier) == DOCTOR) {
          SoldierInSectorDoctor(pSoldier, pSoldier->sInsertionGridNo);
        } else if (GetSolAssignment(pSoldier) == REPAIR) {
          SoldierInSectorRepair(pSoldier, pSoldier->sInsertionGridNo);
        }

        // ATE: Make sure movement mode is up to date!
        pSoldier->usUIMovementMode =
            GetMoveStateBasedOnStance(pSoldier, gAnimControl[pSoldier->usAnimState].ubEndHeight);
      }
    } else {
      // THIS ALL SHOULD HAVE BEEN HANDLED BY THE FACT THAT A GAME WAS LOADED

      // EVENT_InitNewSoldierAnim( pSoldier, pSoldier->usAnimState, pSoldier->usAniCode, TRUE );

      // if the merc had a final destination, get the merc walking there
      // if( pSoldier->sFinalDestination != pSoldier->sGridNo )
      //{
      //	EVENT_GetNewSoldierPath( pSoldier, pSoldier->sFinalDestination,
      // pSoldier->usUIMovementMode );
      //}
    }
  }
}

// IsMercOnTeam() checks to see if the passed in Merc Profile ID is currently on the player's team
BOOLEAN IsMercOnTeam(uint8_t ubMercID) {
  uint16_t cnt;
  uint8_t ubLastTeamID;
  struct SOLDIERTYPE *pTeamSoldier;

  cnt = gTacticalStatus.Team[OUR_TEAM].bFirstID;

  ubLastTeamID = gTacticalStatus.Team[OUR_TEAM].bLastID;

  // look for all mercs on the same team,
  for (pTeamSoldier = MercPtrs[cnt]; cnt <= ubLastTeamID; cnt++, pTeamSoldier++) {
    if (pTeamSoldier->ubProfile == ubMercID) {
      if (pTeamSoldier->bActive) return (TRUE);
    }
  }

  return (FALSE);
}

// ATE: Added this new function for contract renewals
BOOLEAN IsMercOnTeamAndAlive(uint8_t ubMercID) {
  uint16_t cnt;
  uint8_t ubLastTeamID;
  struct SOLDIERTYPE *pTeamSoldier;

  cnt = gTacticalStatus.Team[OUR_TEAM].bFirstID;

  ubLastTeamID = gTacticalStatus.Team[OUR_TEAM].bLastID;

  // look for all mercs on the same team,
  for (pTeamSoldier = MercPtrs[cnt]; cnt <= ubLastTeamID; cnt++, pTeamSoldier++) {
    if (pTeamSoldier->ubProfile == ubMercID) {
      if (pTeamSoldier->bActive) {
        if (pTeamSoldier->bLife > 0) {
          return (TRUE);
        }
      }
    }
  }

  return (FALSE);
}

BOOLEAN IsMercOnTeamAndInOmertaAlready(uint8_t ubMercID) {
  uint16_t cnt;
  uint8_t ubLastTeamID;
  struct SOLDIERTYPE *pTeamSoldier;

  cnt = gTacticalStatus.Team[OUR_TEAM].bFirstID;

  ubLastTeamID = gTacticalStatus.Team[OUR_TEAM].bLastID;

  // look for all mercs on the same team,
  for (pTeamSoldier = MercPtrs[cnt]; cnt <= ubLastTeamID; cnt++, pTeamSoldier++) {
    if (pTeamSoldier->ubProfile == ubMercID) {
      if (pTeamSoldier->bActive && pTeamSoldier->bAssignment != IN_TRANSIT) return (TRUE);
    }
  }

  return (FALSE);
}

BOOLEAN IsMercOnTeamAndInOmertaAlreadyAndAlive(uint8_t ubMercID) {
  uint16_t cnt;
  uint8_t ubLastTeamID;
  struct SOLDIERTYPE *pTeamSoldier;

  cnt = gTacticalStatus.Team[OUR_TEAM].bFirstID;

  ubLastTeamID = gTacticalStatus.Team[OUR_TEAM].bLastID;

  // look for all mercs on the same team,
  for (pTeamSoldier = MercPtrs[cnt]; cnt <= ubLastTeamID; cnt++, pTeamSoldier++) {
    if (pTeamSoldier->ubProfile == ubMercID) {
      if (pTeamSoldier->bActive && pTeamSoldier->bAssignment != IN_TRANSIT) {
        if (pTeamSoldier->bLife > 0) {
          return (TRUE);
        }
      }
    }
  }

  return (FALSE);
}

// GetSoldierIDFromMercID() Gets the Soldier ID from the Merc Profile ID, else returns -1
int16_t GetSoldierIDFromMercID(uint8_t ubMercID) {
  uint16_t cnt;
  uint8_t ubLastTeamID;
  struct SOLDIERTYPE *pTeamSoldier;

  cnt = gTacticalStatus.Team[OUR_TEAM].bFirstID;

  ubLastTeamID = gTacticalStatus.Team[OUR_TEAM].bLastID;

  // look for all mercs on the same team,
  for (pTeamSoldier = MercPtrs[cnt]; cnt <= ubLastTeamID; cnt++, pTeamSoldier++) {
    if (pTeamSoldier->ubProfile == ubMercID) {
      if (pTeamSoldier->bActive) return (cnt);
    }
  }

  return (-1);
}
