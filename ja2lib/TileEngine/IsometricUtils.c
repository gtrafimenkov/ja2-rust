#include "TileEngine/IsometricUtils.h"

#include <math.h>
#include <stdio.h>

#include "JAScreens.h"
#include "SGP/MouseSystem.h"
#include "SGP/Random.h"
#include "SGP/WCheck.h"
#include "SysGlobals.h"
#include "Tactical/Interface.h"
#include "Tactical/Overhead.h"
#include "Tactical/PathAI.h"
#include "Tactical/StructureWrap.h"
#include "TileEngine/RenderWorld.h"
#include "TileEngine/Structure.h"
#include "TileEngine/StructureInternals.h"
#include "TileEngine/WorldMan.h"
#include "rust_geometry.h"

UINT32 guiForceRefreshMousePositionCalculation = 0;

// GLOBALS
INT16 DirIncrementer[8] = {
    -MAPWIDTH,     // N
    1 - MAPWIDTH,  // NE
    1,             // E
    1 + MAPWIDTH,  // SE
    MAPWIDTH,      // S
    MAPWIDTH - 1,  // SW
    -1,            // W
    -MAPWIDTH - 1  // NW

};

// Opposite directions
UINT8 gOppositeDirection[NUM_WORLD_DIRECTIONS] = {SOUTH,     SOUTHWEST, WEST,     NORTHWEST, NORTH,
                                                  NORTHEAST, EAST,      SOUTHEAST

};

UINT8 gTwoCCDirection[NUM_WORLD_DIRECTIONS] = {WEST,      NORTHWEST, NORTH,    NORTHEAST, EAST,
                                               SOUTHEAST, SOUTH,     SOUTHWEST

};

UINT8 gTwoCDirection[NUM_WORLD_DIRECTIONS] = {EAST,      SOUTHEAST, SOUTH,    SOUTHWEST, WEST,
                                              NORTHWEST, NORTH,     NORTHEAST

};

UINT8 gOneCDirection[NUM_WORLD_DIRECTIONS] = {NORTHEAST, EAST, SOUTHEAST, SOUTH,
                                              SOUTHWEST, WEST, NORTHWEST, NORTH};

UINT8 gOneCCDirection[NUM_WORLD_DIRECTIONS] = {NORTHWEST, NORTH, NORTHEAST, EAST,
                                               SOUTHEAST, SOUTH, SOUTHWEST, WEST};

//														DIRECTION
// FACING			 DIRECTION WE WANT TO GOTO
UINT8 gPurpendicularDirection[NUM_WORLD_DIRECTIONS][NUM_WORLD_DIRECTIONS] = {
    {
        // NORTH
        WEST,  // EITHER
        NORTHWEST,
        NORTH,
        NORTHEAST,
        EAST,  // EITHER
        NORTHWEST,
        NORTH,
        NORTHEAST,
    },
    {
        // NORTH EAST
        NORTHWEST,
        NORTHWEST,  // EITHER
        SOUTH,
        NORTHEAST,
        EAST,
        SOUTHEAST,  // EITHER
        NORTH,
        NORTHEAST,
    },
    {
        // EAST
        EAST,
        SOUTHEAST,
        NORTH,  // EITHER
        NORTHEAST,
        EAST,
        SOUTHEAST,
        NORTH,  // EITHER
        NORTHEAST,
    },
    {
        // SOUTHEAST
        EAST, SOUTHEAST, SOUTH,
        SOUTHWEST,  // EITHER
        SOUTHWEST, SOUTHEAST, SOUTH,
        SOUTHWEST,  // EITHER
    },
    {
        // SOUTH
        WEST,  // EITHER
        SOUTHEAST,
        SOUTH,
        SOUTHWEST,
        EAST,  // EITHER
        SOUTHEAST,
        SOUTH,
        SOUTHWEST,
    },
    {
        // SOUTHWEST
        WEST,
        NORTHWEST,  // EITHER
        SOUTH,
        SOUTHWEST,
        WEST,
        SOUTHEAST,  // EITHER
        SOUTH,
        SOUTHWEST,
    },
    {
        // WEST
        WEST,
        NORTHWEST,
        NORTH,  // EITHER
        SOUTHWEST,
        WEST,
        NORTHWEST,
        SOUTH,  // EITHER
        SOUTHWEST,
    },
    {
        // NORTHWEST
        WEST, NORTHWEST, NORTH,
        SOUTHWEST,  // EITHER
        SOUTHWEST, NORTHWEST, NORTH,
        NORTHEAST,  // EITHER
    },
};

void FromCellToScreenCoordinates(INT16 sCellX, INT16 sCellY, INT16 *psScreenX, INT16 *psScreenY) {
  *psScreenX = (2 * sCellX) - (2 * sCellY);
  *psScreenY = sCellX + sCellY;
}

void FromScreenToCellCoordinates(INT16 sScreenX, INT16 sScreenY, INT16 *psCellX, INT16 *psCellY) {
  *psCellX = ((sScreenX + (2 * sScreenY)) / 4);
  *psCellY = ((2 * sScreenY) - sScreenX) / 4;
}

// These two functions take into account that our world is projected and attached
// to the screen (0,0) in a specific way, and we MUSt take that into account then
// determining screen coords

void FloatFromCellToScreenCoordinates(FLOAT dCellX, FLOAT dCellY, FLOAT *pdScreenX,
                                      FLOAT *pdScreenY) {
  FLOAT dScreenX, dScreenY;

  dScreenX = (2 * dCellX) - (2 * dCellY);
  dScreenY = dCellX + dCellY;

  *pdScreenX = dScreenX;
  *pdScreenY = dScreenY;
}

void FloatFromScreenToCellCoordinates(FLOAT dScreenX, FLOAT dScreenY, FLOAT *pdCellX,
                                      FLOAT *pdCellY) {
  FLOAT dCellX, dCellY;

  dCellX = ((dScreenX + (2 * dScreenY)) / 4);
  dCellY = ((2 * dScreenY) - dScreenX) / 4;

  *pdCellX = dCellX;
  *pdCellY = dCellY;
}

BOOLEAN GetMouseXY(INT16 *psMouseX, INT16 *psMouseY) {
  INT16 sWorldX, sWorldY;

  if (!GetMouseWorldCoords(&sWorldX, &sWorldY)) {
    (*psMouseX) = 0;
    (*psMouseY) = 0;
    return (FALSE);
  }

  // Find start block
  (*psMouseX) = (sWorldX / CELL_X_SIZE);
  (*psMouseY) = (sWorldY / CELL_Y_SIZE);

  return (TRUE);
}

BOOLEAN GetMouseXYWithRemainder(INT16 *psMouseX, INT16 *psMouseY, INT16 *psCellX, INT16 *psCellY) {
  INT16 sWorldX, sWorldY;

  if (!GetMouseWorldCoords(&sWorldX, &sWorldY)) {
    return (FALSE);
  }

  // Find start block
  (*psMouseX) = (sWorldX / CELL_X_SIZE);
  (*psMouseY) = (sWorldY / CELL_Y_SIZE);

  (*psCellX) = sWorldX - ((*psMouseX) * CELL_X_SIZE);
  (*psCellY) = sWorldY - ((*psMouseY) * CELL_Y_SIZE);

  return (TRUE);
}

BOOLEAN GetMouseWorldCoords(INT16 *psMouseX, INT16 *psMouseY) {
  INT16 sOffsetX, sOffsetY;
  INT16 sTempPosX_W, sTempPosY_W;
  INT16 sStartPointX_W, sStartPointY_W;

  // Convert mouse screen coords into offset from center
  if (!(gViewportRegion.uiFlags & MSYS_MOUSE_IN_AREA)) {
    *psMouseX = 0;
    *psMouseY = 0;
    return (FALSE);
  }

  sOffsetX = gViewportRegion.MouseXPos -
             ((gsVIEWPORT_END_X - gsVIEWPORT_START_X) / 2);  // + gsRenderWorldOffsetX;
  sOffsetY = gViewportRegion.MouseYPos - ((gsVIEWPORT_END_Y - gsVIEWPORT_START_Y) / 2) +
             10;  // + gsRenderWorldOffsetY;

  // OK, Let's offset by a value if our interfac level is changed!
  if (gsInterfaceLevel != 0) {
    // sOffsetY -= 50;
  }

  FromScreenToCellCoordinates(sOffsetX, sOffsetY, &sTempPosX_W, &sTempPosY_W);

  // World start point is Render center plus this distance
  sStartPointX_W = gsRenderCenterX + sTempPosX_W;
  sStartPointY_W = gsRenderCenterY + sTempPosY_W;

  // check if we are out of bounds..
  if (sStartPointX_W < 0 || sStartPointX_W >= WORLD_COORD_ROWS || sStartPointY_W < 0 ||
      sStartPointY_W >= WORLD_COORD_COLS) {
    *psMouseX = 0;
    *psMouseY = 0;
    return (FALSE);
  }

  // Determine Start block and render offsets
  // Find start block
  // Add adjustment for render origin as well
  (*psMouseX) = sStartPointX_W;
  (*psMouseY) = sStartPointY_W;

  return (TRUE);
}

BOOLEAN GetMouseWorldCoordsInCenter(INT16 *psMouseX, INT16 *psMouseY) {
  INT16 sMouseX, sMouseY;

  // Get grid position
  if (!GetMouseXY(&sMouseX, &sMouseY)) {
    return (FALSE);
  }

  // Now adjust these cell coords into world coords
  *psMouseX = ((sMouseX)*CELL_X_SIZE) + (CELL_X_SIZE / 2);
  *psMouseY = ((sMouseY)*CELL_Y_SIZE) + (CELL_Y_SIZE / 2);

  return (TRUE);
}

BOOLEAN GetMouseMapPos(INT16 *psMapPos) {
  INT16 sWorldX, sWorldY;
  static INT16 sSameCursorPos;
  static UINT32 uiOldFrameNumber = 99999;

  // Check if this is the same frame as before, return already calculated value if so!
  if (uiOldFrameNumber == guiGameCycleCounter && !guiForceRefreshMousePositionCalculation) {
    (*psMapPos) = sSameCursorPos;

    if (sSameCursorPos == 0) {
      return (FALSE);
    }
    return (TRUE);
  }

  uiOldFrameNumber = guiGameCycleCounter;
  guiForceRefreshMousePositionCalculation = FALSE;

  if (GetMouseXY(&sWorldX, &sWorldY)) {
    *psMapPos = MAPROWCOLTOPOS(sWorldY, sWorldX);
    sSameCursorPos = (*psMapPos);
    return (TRUE);
  } else {
    *psMapPos = 0;
    sSameCursorPos = (*psMapPos);
    return (FALSE);
  }
}

BOOLEAN ConvertMapPosToWorldTileCenter(UINT16 usMapPos, INT16 *psXPos, INT16 *psYPos) {
  INT16 sWorldX, sWorldY;
  INT16 sCellX, sCellY;

  // Get X, Y world GRID Coordinates
  sWorldY = (usMapPos / WORLD_COLS);
  sWorldX = usMapPos - (sWorldY * WORLD_COLS);

  // Convert into cell coords
  sCellY = sWorldY * CELL_Y_SIZE;
  sCellX = sWorldX * CELL_X_SIZE;

  // Add center tile positions
  *psXPos = sCellX + (CELL_X_SIZE / 2);
  *psYPos = sCellY + (CELL_Y_SIZE / 2);

  return (TRUE);
}

void GetScreenXYWorldCoords(INT16 sScreenX, INT16 sScreenY, INT16 *psWorldX, INT16 *psWorldY) {
  INT16 sOffsetX, sOffsetY;
  INT16 sTempPosX_W, sTempPosY_W;
  INT16 sStartPointX_W, sStartPointY_W;

  // Convert mouse screen coords into offset from center
  sOffsetX = sScreenX - (gsVIEWPORT_END_X - gsVIEWPORT_START_X) / 2;
  sOffsetY = sScreenY - (gsVIEWPORT_END_Y - gsVIEWPORT_START_Y) / 2;

  FromScreenToCellCoordinates(sOffsetX, sOffsetY, &sTempPosX_W, &sTempPosY_W);

  // World start point is Render center plus this distance
  sStartPointX_W = gsRenderCenterX + sTempPosX_W;
  sStartPointY_W = gsRenderCenterY + sTempPosY_W;

  // Determine Start block and render offsets
  // Find start block
  // Add adjustment for render origin as well
  (*psWorldX) = sStartPointX_W;
  (*psWorldY) = sStartPointY_W;
}

void GetScreenXYWorldCell(INT16 sScreenX, INT16 sScreenY, INT16 *psWorldCellX,
                          INT16 *psWorldCellY) {
  INT16 sWorldX, sWorldY;

  GetScreenXYWorldCoords(sScreenX, sScreenY, &sWorldX, &sWorldY);

  // Find start block
  (*psWorldCellX) = (sWorldX / CELL_X_SIZE);
  (*psWorldCellY) = (sWorldY / CELL_Y_SIZE);
}

void GetScreenXYGridNo(INT16 sScreenX, INT16 sScreenY, INT16 *psMapPos) {
  INT16 sWorldX, sWorldY;

  GetScreenXYWorldCell(sScreenX, sScreenY, &sWorldX, &sWorldY);

  *psMapPos = MAPROWCOLTOPOS(sWorldY, sWorldX);
}

void GetWorldXYAbsoluteScreenXY(INT32 sWorldCellX, INT32 sWorldCellY, INT16 *psWorldScreenX,
                                INT16 *psWorldScreenY) {
  INT16 sScreenCenterX, sScreenCenterY;
  INT16 sDistToCenterY, sDistToCenterX;

  // Find the diustance from render center to true world center
  sDistToCenterX = (sWorldCellX * CELL_X_SIZE) - gCenterWorldX;
  sDistToCenterY = (sWorldCellY * CELL_Y_SIZE) - gCenterWorldY;

  // From render center in world coords, convert to render center in "screen" coords

  // ATE: We should call the fowllowing function but I'm putting it here verbatim for speed
  // FromCellToScreenCoordinates( sDistToCenterX , sDistToCenterY, &sScreenCenterX, &sScreenCenterY
  // );
  sScreenCenterX = (2 * sDistToCenterX) - (2 * sDistToCenterY);
  sScreenCenterY = sDistToCenterX + sDistToCenterY;

  // Subtract screen center
  *psWorldScreenX = sScreenCenterX + gsCX - gsTLX;
  *psWorldScreenY = sScreenCenterY + gsCY - gsTLY;
}

void GetFromAbsoluteScreenXYWorldXY(INT32 *psWorldCellX, INT32 *psWorldCellY, INT16 sWorldScreenX,
                                    INT16 sWorldScreenY) {
  INT16 sWorldCenterX, sWorldCenterY;
  INT16 sDistToCenterY, sDistToCenterX;

  // Subtract screen center
  sDistToCenterX = sWorldScreenX - gsCX + gsTLX;
  sDistToCenterY = sWorldScreenY - gsCY + gsTLY;

  // From render center in world coords, convert to render center in "screen" coords

  // ATE: We should call the fowllowing function but I'm putting it here verbatim for speed
  // FromCellToScreenCoordinates( sDistToCenterX , sDistToCenterY, &sScreenCenterX, &sScreenCenterY
  // );
  sWorldCenterX = ((sDistToCenterX + (2 * sDistToCenterY)) / 4);
  sWorldCenterY = ((2 * sDistToCenterY) - sDistToCenterX) / 4;

  // Goto center again
  *psWorldCellX = sWorldCenterX + gCenterWorldX;
  *psWorldCellY = sWorldCenterY + gCenterWorldY;
}

// UTILITY FUNTIONS

INT32 OutOfBounds(INT16 sGridno, INT16 sProposedGridno) {
  INT16 sMod, sPropMod;

  // get modulas of our origin
  sMod = sGridno % MAXCOL;

  if (sMod != 0)                   // if we're not on leftmost grid
    if (sMod != RIGHTMOSTGRID)     // if we're not on rightmost grid
      if (sGridno < LASTROWSTART)  // if we're above bottom row
        if (sGridno > MAXCOL)      // if we're below top row
          // Everything's OK - we're not on the edge of the map
          return (FALSE);

  // if we've got this far, there's a potential problem - check it out!

  if (sProposedGridno < 0) return (TRUE);

  sPropMod = sProposedGridno % MAXCOL;

  if (sMod == 0 && sPropMod == RIGHTMOSTGRID)
    return (TRUE);
  else if (sMod == RIGHTMOSTGRID && sPropMod == 0)
    return (TRUE);
  else if (sGridno >= LASTROWSTART && sProposedGridno >= GRIDSIZE)
    return (TRUE);
  else
    return (FALSE);
}

INT16 NewGridNo(INT16 sGridno, INT16 sDirInc) {
  INT16 sProposedGridno = sGridno + sDirInc;

  // now check for out-of-bounds
  if (OutOfBounds(sGridno, sProposedGridno))
    // return ORIGINAL gridno to user
    sProposedGridno = sGridno;

  return (sProposedGridno);
}

INT16 DirectionInc(INT16 sDirection) {
  if ((sDirection < 0) || (sDirection > 7)) {
    // #ifdef BETAVERSION
    //    NumMessage("DirectionInc: Invalid direction received, = ",direction);
    // #endif

    // direction = random(8);	// replace garbage with random direction
    sDirection = 1;
  }

  return (DirIncrementer[sDirection]);
}

BOOLEAN CellXYToScreenXY(INT16 sCellX, INT16 sCellY, INT16 *sScreenX, INT16 *sScreenY) {
  INT16 sDeltaCellX, sDeltaCellY;
  INT16 sDeltaScreenX, sDeltaScreenY;

  sDeltaCellX = sCellX - gsRenderCenterX;
  sDeltaCellY = sCellY - gsRenderCenterY;

  FromCellToScreenCoordinates(sDeltaCellX, sDeltaCellY, &sDeltaScreenX, &sDeltaScreenY);

  *sScreenX = (((gsVIEWPORT_END_X - gsVIEWPORT_START_X) / 2) + sDeltaScreenX);
  *sScreenY = (((gsVIEWPORT_END_Y - gsVIEWPORT_START_Y) / 2) + sDeltaScreenY);

  return (TRUE);
}

void ConvertGridNoToXY(INT16 sGridNo, INT16 *sXPos, INT16 *sYPos) {
  *sYPos = sGridNo / WORLD_COLS;
  *sXPos = (sGridNo - (*sYPos * WORLD_COLS));
}

void ConvertGridNoToCellXY(INT16 sGridNo, INT16 *sXPos, INT16 *sYPos) {
  *sYPos = (sGridNo / WORLD_COLS);
  *sXPos = sGridNo - (*sYPos * WORLD_COLS);

  *sYPos = (*sYPos * CELL_Y_SIZE);
  *sXPos = (*sXPos * CELL_X_SIZE);
}

void ConvertGridNoToCenterCellXY(INT16 sGridNo, INT16 *sXPos, INT16 *sYPos) {
  *sYPos = (sGridNo / WORLD_COLS);
  *sXPos = (sGridNo - (*sYPos * WORLD_COLS));

  *sYPos = (*sYPos * CELL_Y_SIZE) + (CELL_Y_SIZE / 2);
  *sXPos = (*sXPos * CELL_X_SIZE) + (CELL_X_SIZE / 2);
}

INT32 GetRangeFromGridNoDiff(INT16 sGridNo1, INT16 sGridNo2) {
  INT32 uiDist;
  INT16 sXPos, sYPos, sXPos2, sYPos2;

  // Convert our grid-not into an XY
  ConvertGridNoToXY(sGridNo1, &sXPos, &sYPos);

  // Convert our grid-not into an XY
  ConvertGridNoToXY(sGridNo2, &sXPos2, &sYPos2);

  uiDist =
      (INT16)sqrt((float)(sXPos2 - sXPos) * (sXPos2 - sXPos) + (sYPos2 - sYPos) * (sYPos2 - sYPos));

  return (uiDist);
}

INT32 GetRangeInCellCoordsFromGridNoDiff(INT16 sGridNo1, INT16 sGridNo2) {
  INT16 sXPos, sYPos, sXPos2, sYPos2;

  // Convert our grid-not into an XY
  ConvertGridNoToXY(sGridNo1, &sXPos, &sYPos);

  // Convert our grid-not into an XY
  ConvertGridNoToXY(sGridNo2, &sXPos2, &sYPos2);

  return ((INT32)(sqrt((float)(sXPos2 - sXPos) * (sXPos2 - sXPos) +
                       (sYPos2 - sYPos) * (sYPos2 - sYPos)) *
                  CELL_X_SIZE));
}

BOOLEAN IsPointInScreenRect(INT16 sXPos, INT16 sYPos, struct GRect *pRect) {
  if ((sXPos >= pRect->iLeft) && (sXPos <= pRect->iRight) && (sYPos >= pRect->iTop) &&
      (sYPos <= pRect->iBottom)) {
    return (TRUE);
  } else {
    return (FALSE);
  }
}

BOOLEAN IsPointInScreenRectWithRelative(INT16 sXPos, INT16 sYPos, struct GRect *pRect, INT16 *sXRel,
                                        INT16 *sYRel) {
  if ((sXPos >= pRect->iLeft) && (sXPos <= pRect->iRight) && (sYPos >= pRect->iTop) &&
      (sYPos <= pRect->iBottom)) {
    (*sXRel) = pRect->iLeft - sXPos;
    (*sYRel) = sYPos - (INT16)pRect->iTop;

    return (TRUE);
  } else {
    return (FALSE);
  }
}

INT16 PythSpacesAway(INT16 sOrigin, INT16 sDest) {
  INT16 sRows, sCols, sResult;

  sRows = abs((sOrigin / MAXCOL) - (sDest / MAXCOL));
  sCols = abs((sOrigin % MAXROW) - (sDest % MAXROW));

  // apply Pythagoras's theorem for right-handed triangle:
  // dist^2 = rows^2 + cols^2, so use the square root to get the distance
  sResult = (INT16)sqrt((float)(sRows * sRows) + (sCols * sCols));

  return (sResult);
}

INT16 SpacesAway(INT16 sOrigin, INT16 sDest) {
  INT16 sRows, sCols;

  sRows = abs((sOrigin / MAXCOL) - (sDest / MAXCOL));
  sCols = abs((sOrigin % MAXROW) - (sDest % MAXROW));

  return (max(sRows, sCols));
}

INT16 CardinalSpacesAway(INT16 sOrigin, INT16 sDest)
// distance away, ignoring diagonals!
{
  INT16 sRows, sCols;

  sRows = abs((sOrigin / MAXCOL) - (sDest / MAXCOL));
  sCols = abs((sOrigin % MAXROW) - (sDest % MAXROW));

  return ((INT16)(sRows + sCols));
}

INT8 FindNumTurnsBetweenDirs(INT8 sDir1, INT8 sDir2) {
  INT16 sDirection;
  INT16 sNumTurns = 0;

  sDirection = sDir1;

  do {
    sDirection = sDirection + QuickestDirection(sDir1, sDir2);

    if (sDirection > 7) {
      sDirection = 0;
    } else {
      if (sDirection < 0) {
        sDirection = 7;
      }
    }

    if (sDirection == sDir2) {
      break;
    }

    sNumTurns++;

    // SAFEGUARD ! - if we (somehow) do not get to were we want!
    if (sNumTurns > 100) {
      sNumTurns = 0;
      break;
    }
  } while (TRUE);

  return ((INT8)sNumTurns);
}

BOOLEAN FindHeigherLevel(struct SOLDIERTYPE *pSoldier, INT16 sGridNo, INT8 bStartingDir,
                         INT8 *pbDirection) {
  INT32 cnt;
  INT16 sNewGridNo;
  BOOLEAN fFound = FALSE;
  UINT8 bMinNumTurns = 100;
  INT8 bNumTurns;
  INT8 bMinDirection = 0;

  // IF there is a roof over our heads, this is an ivalid....
  // return ( FALSE );l
  if (FindStructure(sGridNo, STRUCTURE_ROOF) != NULL) {
    return (FALSE);
  }

  // LOOP THROUGH ALL 8 DIRECTIONS
  for (cnt = 0; cnt < 8; cnt += 2) {
    sNewGridNo = NewGridNo((UINT16)sGridNo, (UINT16)DirectionInc((UINT8)cnt));

    if (NewOKDestination(pSoldier, sNewGridNo, TRUE, 1)) {
      // Check if this tile has a higher level
      if (IsHeigherLevel(sNewGridNo)) {
        fFound = TRUE;

        // FInd how many turns we should go to get here
        bNumTurns = FindNumTurnsBetweenDirs((INT8)cnt, bStartingDir);

        if (bNumTurns < bMinNumTurns) {
          bMinNumTurns = bNumTurns;
          bMinDirection = (INT8)cnt;
        }
      }
    }
  }

  if (fFound) {
    *pbDirection = bMinDirection;
    return (TRUE);
  }

  return (FALSE);
}

BOOLEAN FindLowerLevel(struct SOLDIERTYPE *pSoldier, INT16 sGridNo, INT8 bStartingDir,
                       INT8 *pbDirection) {
  INT32 cnt;
  INT16 sNewGridNo;
  BOOLEAN fFound = FALSE;
  UINT8 bMinNumTurns = 100;
  INT8 bNumTurns;
  INT8 bMinDirection = 0;

  // LOOP THROUGH ALL 8 DIRECTIONS
  for (cnt = 0; cnt < 8; cnt += 2) {
    sNewGridNo = NewGridNo((UINT16)sGridNo, (UINT16)DirectionInc((UINT8)cnt));

    // Make sure there is NOT a roof here...
    // Check OK destination
    if (NewOKDestination(pSoldier, sNewGridNo, TRUE, 0)) {
      if (FindStructure(sNewGridNo, STRUCTURE_ROOF) == NULL) {
        {
          fFound = TRUE;

          // FInd how many turns we should go to get here
          bNumTurns = FindNumTurnsBetweenDirs((INT8)cnt, bStartingDir);

          if (bNumTurns < bMinNumTurns) {
            bMinNumTurns = bNumTurns;
            bMinDirection = (INT8)cnt;
          }
        }
      }
    }
  }

  if (fFound) {
    *pbDirection = bMinDirection;
    return (TRUE);
  }

  return (FALSE);
}

INT16 QuickestDirection(INT16 origin, INT16 dest) {
  INT16 v1, v2;

  if (origin == dest) return (0);

  if ((abs(origin - dest)) == 4)
    return (1);  // this could be made random
  else if (origin > dest) {
    v1 = abs(origin - dest);
    v2 = (8 - origin) + dest;
    if (v1 > v2)
      return (1);
    else
      return (-1);
  } else {
    v1 = abs(origin - dest);
    v2 = (8 - dest) + origin;
    if (v1 > v2)
      return (-1);
    else
      return (1);
  }
}

INT16 ExtQuickestDirection(INT16 origin, INT16 dest) {
  INT16 v1, v2;

  if (origin == dest) return (0);

  if ((abs(origin - dest)) == 16)
    return (1);  // this could be made random
  else if (origin > dest) {
    v1 = abs(origin - dest);
    v2 = (32 - origin) + dest;
    if (v1 > v2)
      return (1);
    else
      return (-1);
  } else {
    v1 = abs(origin - dest);
    v2 = (32 - dest) + origin;
    if (v1 > v2)
      return (-1);
    else
      return (1);
  }
}

// Returns the (center ) cell coordinates in X
INT16 CenterX(INT16 sGridNo) {
  INT16 sYPos, sXPos;

  sYPos = sGridNo / WORLD_COLS;
  sXPos = (sGridNo - (sYPos * WORLD_COLS));

  return ((sXPos * CELL_X_SIZE) + (CELL_X_SIZE / 2));
}

// Returns the (center ) cell coordinates in Y
INT16 CenterY(INT16 sGridNo) {
  INT16 sYPos;

  sYPos = sGridNo / WORLD_COLS;

  return ((sYPos * CELL_Y_SIZE) + (CELL_Y_SIZE / 2));
}

INT16 MapX(INT16 sGridNo) {
  INT16 sYPos, sXPos;

  sYPos = sGridNo / WORLD_COLS;
  sXPos = (sGridNo - (sYPos * WORLD_COLS));

  return (sXPos);
}

INT16 MapY(INT16 sGridNo) {
  INT16 sYPos;

  sYPos = sGridNo / WORLD_COLS;

  return (sYPos);
}

BOOLEAN GridNoOnVisibleWorldTile(INT16 sGridNo) {
  INT16 sWorldX;
  INT16 sWorldY;
  INT16 sXMapPos, sYMapPos;

  // Check for valid gridno...
  ConvertGridNoToXY(sGridNo, &sXMapPos, &sYMapPos);

  // Get screen coordinates for current position of soldier
  GetWorldXYAbsoluteScreenXY(sXMapPos, sYMapPos, &sWorldX, &sWorldY);

  if (sWorldX > 0 && sWorldX < (gsTRX - gsTLX - 20) && sWorldY > 20 &&
      sWorldY < (gsBLY - gsTLY - 20)) {
    return (TRUE);
  }

  return (FALSE);
}

// This function is used when we care about astetics with the top Y portion of the
// gma eplay area
// mostly due to UI bar that comes down....
BOOLEAN GridNoOnVisibleWorldTileGivenYLimits(INT16 sGridNo) {
  INT16 sWorldX;
  INT16 sWorldY;
  INT16 sXMapPos, sYMapPos;

  // Check for valid gridno...
  ConvertGridNoToXY(sGridNo, &sXMapPos, &sYMapPos);

  // Get screen coordinates for current position of soldier
  GetWorldXYAbsoluteScreenXY(sXMapPos, sYMapPos, &sWorldX, &sWorldY);

  if (sWorldX > 0 && sWorldX < (gsTRX - gsTLX - 20) && sWorldY > 40 &&
      sWorldY < (gsBLY - gsTLY - 20)) {
    return (TRUE);
  }

  return (FALSE);
}

BOOLEAN GridNoOnEdgeOfMap(INT16 sGridNo, INT8 *pbDirection) {
  INT8 bDir;

  // check NE, SE, SW, NW because of tilt of isometric display

  for (bDir = NORTHEAST; bDir < NUM_WORLD_DIRECTIONS; bDir += 2) {
    if (gubWorldMovementCosts[(sGridNo + DirectionInc(bDir))][bDir][0] == TRAVELCOST_OFF_MAP)
    // if ( !GridNoOnVisibleWorldTile( (INT16) (sGridNo + DirectionInc( bDir ) ) ) )
    {
      *pbDirection = bDir;
      return (TRUE);
    }
  }
  return (FALSE);
}

BOOLEAN FindFenceJumpDirection(struct SOLDIERTYPE *pSoldier, INT16 sGridNo, INT8 bStartingDir,
                               INT8 *pbDirection) {
  INT32 cnt;
  INT16 sNewGridNo, sOtherSideOfFence;
  BOOLEAN fFound = FALSE;
  UINT8 bMinNumTurns = 100;
  INT8 bNumTurns;
  INT8 bMinDirection = 0;

  // IF there is a fence in this gridno, return false!
  if (IsJumpableFencePresentAtGridno(sGridNo)) {
    return (FALSE);
  }

  // LOOP THROUGH ALL 8 DIRECTIONS
  for (cnt = 0; cnt < 8; cnt += 2) {
    // go out *2* tiles
    sNewGridNo = NewGridNo((UINT16)sGridNo, (UINT16)DirectionInc((UINT8)cnt));
    sOtherSideOfFence = NewGridNo((UINT16)sNewGridNo, (UINT16)DirectionInc((UINT8)cnt));

    if (NewOKDestination(pSoldier, sOtherSideOfFence, TRUE, 0)) {
      // ATE: Check if there is somebody waiting here.....

      // Check if we have a fence here
      if (IsJumpableFencePresentAtGridno(sNewGridNo)) {
        fFound = TRUE;

        // FInd how many turns we should go to get here
        bNumTurns = FindNumTurnsBetweenDirs((INT8)cnt, bStartingDir);

        if (bNumTurns < bMinNumTurns) {
          bMinNumTurns = bNumTurns;
          bMinDirection = (INT8)cnt;
        }
      }
    }
  }

  if (fFound) {
    *pbDirection = bMinDirection;
    return (TRUE);
  }

  return (FALSE);
}

// Simply chooses a random gridno within valid boundaries (for dropping things in unloaded sectors)
INT16 RandomGridNo() {
  INT32 iMapXPos, iMapYPos, iMapIndex;
  do {
    iMapXPos = Random(WORLD_COLS);
    iMapYPos = Random(WORLD_ROWS);
    iMapIndex = iMapYPos * WORLD_COLS + iMapXPos;
  } while (!GridNoOnVisibleWorldTile((INT16)iMapIndex));
  return (INT16)iMapIndex;
}
