#include "TileEngine/IsometricUtils.h"

#include <math.h>
#include <stdio.h>

#include "JAScreens.h"
#include "SGP/Debug.h"
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

uint32_t guiForceRefreshMousePositionCalculation = 0;

// GLOBALS
int16_t DirIncrementer[8] = {
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
uint8_t gOppositeDirection[NUM_WORLD_DIRECTIONS] = {SOUTH,     SOUTHWEST, WEST,
                                                    NORTHWEST, NORTH,     NORTHEAST,
                                                    EAST,      SOUTHEAST

};

uint8_t gTwoCCDirection[NUM_WORLD_DIRECTIONS] = {WEST,      NORTHWEST, NORTH,    NORTHEAST, EAST,
                                                 SOUTHEAST, SOUTH,     SOUTHWEST

};

uint8_t gTwoCDirection[NUM_WORLD_DIRECTIONS] = {EAST,      SOUTHEAST, SOUTH,    SOUTHWEST, WEST,
                                                NORTHWEST, NORTH,     NORTHEAST

};

uint8_t gOneCDirection[NUM_WORLD_DIRECTIONS] = {NORTHEAST, EAST, SOUTHEAST, SOUTH,
                                                SOUTHWEST, WEST, NORTHWEST, NORTH};

uint8_t gOneCCDirection[NUM_WORLD_DIRECTIONS] = {NORTHWEST, NORTH, NORTHEAST, EAST,
                                                 SOUTHEAST, SOUTH, SOUTHWEST, WEST};

//														DIRECTION
// FACING			 DIRECTION WE WANT TO GOTO
uint8_t gPurpendicularDirection[NUM_WORLD_DIRECTIONS][NUM_WORLD_DIRECTIONS] = {
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

void FromCellToScreenCoordinates(int16_t sCellX, int16_t sCellY, int16_t *psScreenX,
                                 int16_t *psScreenY) {
  *psScreenX = (2 * sCellX) - (2 * sCellY);
  *psScreenY = sCellX + sCellY;
}

void FromScreenToCellCoordinates(int16_t sScreenX, int16_t sScreenY, int16_t *psCellX,
                                 int16_t *psCellY) {
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

BOOLEAN GetMouseXY(int16_t *psMouseX, int16_t *psMouseY) {
  int16_t sWorldX, sWorldY;

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

BOOLEAN GetMouseXYWithRemainder(int16_t *psMouseX, int16_t *psMouseY, int16_t *psCellX,
                                int16_t *psCellY) {
  int16_t sWorldX, sWorldY;

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

BOOLEAN GetMouseWorldCoords(int16_t *psMouseX, int16_t *psMouseY) {
  int16_t sOffsetX, sOffsetY;
  int16_t sTempPosX_W, sTempPosY_W;
  int16_t sStartPointX_W, sStartPointY_W;

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

BOOLEAN GetMouseWorldCoordsInCenter(int16_t *psMouseX, int16_t *psMouseY) {
  int16_t sMouseX, sMouseY;

  // Get grid position
  if (!GetMouseXY(&sMouseX, &sMouseY)) {
    return (FALSE);
  }

  // Now adjust these cell coords into world coords
  *psMouseX = ((sMouseX)*CELL_X_SIZE) + (CELL_X_SIZE / 2);
  *psMouseY = ((sMouseY)*CELL_Y_SIZE) + (CELL_Y_SIZE / 2);

  return (TRUE);
}

BOOLEAN GetMouseMapPos(int16_t *psMapPos) {
  int16_t sWorldX, sWorldY;
  static int16_t sSameCursorPos;
  static uint32_t uiOldFrameNumber = 99999;

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

BOOLEAN ConvertMapPosToWorldTileCenter(uint16_t usMapPos, int16_t *psXPos, int16_t *psYPos) {
  int16_t sWorldX, sWorldY;
  int16_t sCellX, sCellY;

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

void GetScreenXYWorldCoords(int16_t sScreenX, int16_t sScreenY, int16_t *psWorldX,
                            int16_t *psWorldY) {
  int16_t sOffsetX, sOffsetY;
  int16_t sTempPosX_W, sTempPosY_W;
  int16_t sStartPointX_W, sStartPointY_W;

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

void GetScreenXYWorldCell(int16_t sScreenX, int16_t sScreenY, int16_t *psWorldCellX,
                          int16_t *psWorldCellY) {
  int16_t sWorldX, sWorldY;

  GetScreenXYWorldCoords(sScreenX, sScreenY, &sWorldX, &sWorldY);

  // Find start block
  (*psWorldCellX) = (sWorldX / CELL_X_SIZE);
  (*psWorldCellY) = (sWorldY / CELL_Y_SIZE);
}

void GetScreenXYGridNo(int16_t sScreenX, int16_t sScreenY, int16_t *psMapPos) {
  int16_t sWorldX, sWorldY;

  GetScreenXYWorldCell(sScreenX, sScreenY, &sWorldX, &sWorldY);

  *psMapPos = MAPROWCOLTOPOS(sWorldY, sWorldX);
}

void GetWorldXYAbsoluteScreenXY(INT32 sWorldCellX, INT32 sWorldCellY, int16_t *psWorldScreenX,
                                int16_t *psWorldScreenY) {
  int16_t sScreenCenterX, sScreenCenterY;
  int16_t sDistToCenterY, sDistToCenterX;

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

void GetFromAbsoluteScreenXYWorldXY(INT32 *psWorldCellX, INT32 *psWorldCellY, int16_t sWorldScreenX,
                                    int16_t sWorldScreenY) {
  int16_t sWorldCenterX, sWorldCenterY;
  int16_t sDistToCenterY, sDistToCenterX;

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

INT32 OutOfBounds(int16_t sGridno, int16_t sProposedGridno) {
  int16_t sMod, sPropMod;

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

int16_t NewGridNo(int16_t sGridno, int16_t sDirInc) {
  int16_t sProposedGridno = sGridno + sDirInc;

  // now check for out-of-bounds
  if (OutOfBounds(sGridno, sProposedGridno))
    // return ORIGINAL gridno to user
    sProposedGridno = sGridno;

  return (sProposedGridno);
}

int16_t DirectionInc(int16_t sDirection) {
  if ((sDirection < 0) || (sDirection > 7)) {
    // #ifdef BETAVERSION
    //    NumMessage("DirectionInc: Invalid direction received, = ",direction);
    // #endif

    // direction = random(8);	// replace garbage with random direction
    sDirection = 1;
  }

  return (DirIncrementer[sDirection]);
}

BOOLEAN CellXYToScreenXY(int16_t sCellX, int16_t sCellY, int16_t *sScreenX, int16_t *sScreenY) {
  int16_t sDeltaCellX, sDeltaCellY;
  int16_t sDeltaScreenX, sDeltaScreenY;

  sDeltaCellX = sCellX - gsRenderCenterX;
  sDeltaCellY = sCellY - gsRenderCenterY;

  FromCellToScreenCoordinates(sDeltaCellX, sDeltaCellY, &sDeltaScreenX, &sDeltaScreenY);

  *sScreenX = (((gsVIEWPORT_END_X - gsVIEWPORT_START_X) / 2) + sDeltaScreenX);
  *sScreenY = (((gsVIEWPORT_END_Y - gsVIEWPORT_START_Y) / 2) + sDeltaScreenY);

  return (TRUE);
}

void ConvertGridNoToXY(int16_t sGridNo, int16_t *sXPos, int16_t *sYPos) {
  *sYPos = sGridNo / WORLD_COLS;
  *sXPos = (sGridNo - (*sYPos * WORLD_COLS));
}

void ConvertGridNoToCellXY(int16_t sGridNo, int16_t *sXPos, int16_t *sYPos) {
  *sYPos = (sGridNo / WORLD_COLS);
  *sXPos = sGridNo - (*sYPos * WORLD_COLS);

  *sYPos = (*sYPos * CELL_Y_SIZE);
  *sXPos = (*sXPos * CELL_X_SIZE);
}

void ConvertGridNoToCenterCellXY(int16_t sGridNo, int16_t *sXPos, int16_t *sYPos) {
  *sYPos = (sGridNo / WORLD_COLS);
  *sXPos = (sGridNo - (*sYPos * WORLD_COLS));

  *sYPos = (*sYPos * CELL_Y_SIZE) + (CELL_Y_SIZE / 2);
  *sXPos = (*sXPos * CELL_X_SIZE) + (CELL_X_SIZE / 2);
}

INT32 GetRangeFromGridNoDiff(int16_t sGridNo1, int16_t sGridNo2) {
  INT32 uiDist;
  int16_t sXPos, sYPos, sXPos2, sYPos2;

  // Convert our grid-not into an XY
  ConvertGridNoToXY(sGridNo1, &sXPos, &sYPos);

  // Convert our grid-not into an XY
  ConvertGridNoToXY(sGridNo2, &sXPos2, &sYPos2);

  uiDist = (int16_t)sqrt((float)(sXPos2 - sXPos) * (sXPos2 - sXPos) +
                         (sYPos2 - sYPos) * (sYPos2 - sYPos));

  return (uiDist);
}

INT32 GetRangeInCellCoordsFromGridNoDiff(int16_t sGridNo1, int16_t sGridNo2) {
  int16_t sXPos, sYPos, sXPos2, sYPos2;

  // Convert our grid-not into an XY
  ConvertGridNoToXY(sGridNo1, &sXPos, &sYPos);

  // Convert our grid-not into an XY
  ConvertGridNoToXY(sGridNo2, &sXPos2, &sYPos2);

  return ((INT32)(sqrt((float)(sXPos2 - sXPos) * (sXPos2 - sXPos) +
                       (sYPos2 - sYPos) * (sYPos2 - sYPos)) *
                  CELL_X_SIZE));
}

BOOLEAN IsPointInScreenRect(int16_t sXPos, int16_t sYPos, SGPRect *pRect) {
  if ((sXPos >= pRect->iLeft) && (sXPos <= pRect->iRight) && (sYPos >= pRect->iTop) &&
      (sYPos <= pRect->iBottom)) {
    return (TRUE);
  } else {
    return (FALSE);
  }
}

BOOLEAN IsPointInScreenRectWithRelative(int16_t sXPos, int16_t sYPos, SGPRect *pRect,
                                        int16_t *sXRel, int16_t *sYRel) {
  if ((sXPos >= pRect->iLeft) && (sXPos <= pRect->iRight) && (sYPos >= pRect->iTop) &&
      (sYPos <= pRect->iBottom)) {
    (*sXRel) = pRect->iLeft - sXPos;
    (*sYRel) = sYPos - (int16_t)pRect->iTop;

    return (TRUE);
  } else {
    return (FALSE);
  }
}

int16_t PythSpacesAway(int16_t sOrigin, int16_t sDest) {
  int16_t sRows, sCols, sResult;

  sRows = abs((sOrigin / MAXCOL) - (sDest / MAXCOL));
  sCols = abs((sOrigin % MAXROW) - (sDest % MAXROW));

  // apply Pythagoras's theorem for right-handed triangle:
  // dist^2 = rows^2 + cols^2, so use the square root to get the distance
  sResult = (int16_t)sqrt((float)(sRows * sRows) + (sCols * sCols));

  return (sResult);
}

int16_t SpacesAway(int16_t sOrigin, int16_t sDest) {
  int16_t sRows, sCols;

  sRows = abs((sOrigin / MAXCOL) - (sDest / MAXCOL));
  sCols = abs((sOrigin % MAXROW) - (sDest % MAXROW));

  return (max(sRows, sCols));
}

int16_t CardinalSpacesAway(int16_t sOrigin, int16_t sDest)
// distance away, ignoring diagonals!
{
  int16_t sRows, sCols;

  sRows = abs((sOrigin / MAXCOL) - (sDest / MAXCOL));
  sCols = abs((sOrigin % MAXROW) - (sDest % MAXROW));

  return ((int16_t)(sRows + sCols));
}

int8_t FindNumTurnsBetweenDirs(int8_t sDir1, int8_t sDir2) {
  int16_t sDirection;
  int16_t sNumTurns = 0;

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

  return ((int8_t)sNumTurns);
}

BOOLEAN FindHeigherLevel(struct SOLDIERTYPE *pSoldier, int16_t sGridNo, int8_t bStartingDir,
                         int8_t *pbDirection) {
  INT32 cnt;
  int16_t sNewGridNo;
  BOOLEAN fFound = FALSE;
  uint8_t bMinNumTurns = 100;
  int8_t bNumTurns;
  int8_t bMinDirection = 0;

  // IF there is a roof over our heads, this is an ivalid....
  // return ( FALSE );l
  if (FindStructure(sGridNo, STRUCTURE_ROOF) != NULL) {
    return (FALSE);
  }

  // LOOP THROUGH ALL 8 DIRECTIONS
  for (cnt = 0; cnt < 8; cnt += 2) {
    sNewGridNo = NewGridNo((uint16_t)sGridNo, (uint16_t)DirectionInc((uint8_t)cnt));

    if (NewOKDestination(pSoldier, sNewGridNo, TRUE, 1)) {
      // Check if this tile has a higher level
      if (IsHeigherLevel(sNewGridNo)) {
        fFound = TRUE;

        // FInd how many turns we should go to get here
        bNumTurns = FindNumTurnsBetweenDirs((int8_t)cnt, bStartingDir);

        if (bNumTurns < bMinNumTurns) {
          bMinNumTurns = bNumTurns;
          bMinDirection = (int8_t)cnt;
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

BOOLEAN FindLowerLevel(struct SOLDIERTYPE *pSoldier, int16_t sGridNo, int8_t bStartingDir,
                       int8_t *pbDirection) {
  INT32 cnt;
  int16_t sNewGridNo;
  BOOLEAN fFound = FALSE;
  uint8_t bMinNumTurns = 100;
  int8_t bNumTurns;
  int8_t bMinDirection = 0;

  // LOOP THROUGH ALL 8 DIRECTIONS
  for (cnt = 0; cnt < 8; cnt += 2) {
    sNewGridNo = NewGridNo((uint16_t)sGridNo, (uint16_t)DirectionInc((uint8_t)cnt));

    // Make sure there is NOT a roof here...
    // Check OK destination
    if (NewOKDestination(pSoldier, sNewGridNo, TRUE, 0)) {
      if (FindStructure(sNewGridNo, STRUCTURE_ROOF) == NULL) {
        {
          fFound = TRUE;

          // FInd how many turns we should go to get here
          bNumTurns = FindNumTurnsBetweenDirs((int8_t)cnt, bStartingDir);

          if (bNumTurns < bMinNumTurns) {
            bMinNumTurns = bNumTurns;
            bMinDirection = (int8_t)cnt;
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

int16_t QuickestDirection(int16_t origin, int16_t dest) {
  int16_t v1, v2;

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

int16_t ExtQuickestDirection(int16_t origin, int16_t dest) {
  int16_t v1, v2;

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
int16_t CenterX(int16_t sGridNo) {
  int16_t sYPos, sXPos;

  sYPos = sGridNo / WORLD_COLS;
  sXPos = (sGridNo - (sYPos * WORLD_COLS));

  return ((sXPos * CELL_X_SIZE) + (CELL_X_SIZE / 2));
}

// Returns the (center ) cell coordinates in Y
int16_t CenterY(int16_t sGridNo) {
  int16_t sYPos;

  sYPos = sGridNo / WORLD_COLS;

  return ((sYPos * CELL_Y_SIZE) + (CELL_Y_SIZE / 2));
}

int16_t MapX(int16_t sGridNo) {
  int16_t sYPos, sXPos;

  sYPos = sGridNo / WORLD_COLS;
  sXPos = (sGridNo - (sYPos * WORLD_COLS));

  return (sXPos);
}

int16_t MapY(int16_t sGridNo) {
  int16_t sYPos;

  sYPos = sGridNo / WORLD_COLS;

  return (sYPos);
}

BOOLEAN GridNoOnVisibleWorldTile(int16_t sGridNo) {
  int16_t sWorldX;
  int16_t sWorldY;
  int16_t sXMapPos, sYMapPos;

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
BOOLEAN GridNoOnVisibleWorldTileGivenYLimits(int16_t sGridNo) {
  int16_t sWorldX;
  int16_t sWorldY;
  int16_t sXMapPos, sYMapPos;

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

BOOLEAN GridNoOnEdgeOfMap(int16_t sGridNo, int8_t *pbDirection) {
  int8_t bDir;

  // check NE, SE, SW, NW because of tilt of isometric display

  for (bDir = NORTHEAST; bDir < NUM_WORLD_DIRECTIONS; bDir += 2) {
    if (gubWorldMovementCosts[(sGridNo + DirectionInc(bDir))][bDir][0] == TRAVELCOST_OFF_MAP)
    // if ( !GridNoOnVisibleWorldTile( (int16_t) (sGridNo + DirectionInc( bDir ) ) ) )
    {
      *pbDirection = bDir;
      return (TRUE);
    }
  }
  return (FALSE);
}

BOOLEAN FindFenceJumpDirection(struct SOLDIERTYPE *pSoldier, int16_t sGridNo, int8_t bStartingDir,
                               int8_t *pbDirection) {
  INT32 cnt;
  int16_t sNewGridNo, sOtherSideOfFence;
  BOOLEAN fFound = FALSE;
  uint8_t bMinNumTurns = 100;
  int8_t bNumTurns;
  int8_t bMinDirection = 0;

  // IF there is a fence in this gridno, return false!
  if (IsJumpableFencePresentAtGridno(sGridNo)) {
    return (FALSE);
  }

  // LOOP THROUGH ALL 8 DIRECTIONS
  for (cnt = 0; cnt < 8; cnt += 2) {
    // go out *2* tiles
    sNewGridNo = NewGridNo((uint16_t)sGridNo, (uint16_t)DirectionInc((uint8_t)cnt));
    sOtherSideOfFence = NewGridNo((uint16_t)sNewGridNo, (uint16_t)DirectionInc((uint8_t)cnt));

    if (NewOKDestination(pSoldier, sOtherSideOfFence, TRUE, 0)) {
      // ATE: Check if there is somebody waiting here.....

      // Check if we have a fence here
      if (IsJumpableFencePresentAtGridno(sNewGridNo)) {
        fFound = TRUE;

        // FInd how many turns we should go to get here
        bNumTurns = FindNumTurnsBetweenDirs((int8_t)cnt, bStartingDir);

        if (bNumTurns < bMinNumTurns) {
          bMinNumTurns = bNumTurns;
          bMinDirection = (int8_t)cnt;
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
int16_t RandomGridNo() {
  INT32 iMapXPos, iMapYPos, iMapIndex;
  do {
    iMapXPos = Random(WORLD_COLS);
    iMapYPos = Random(WORLD_ROWS);
    iMapIndex = iMapYPos * WORLD_COLS + iMapXPos;
  } while (!GridNoOnVisibleWorldTile((int16_t)iMapIndex));
  return (int16_t)iMapIndex;
}
