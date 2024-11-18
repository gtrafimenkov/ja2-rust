#include "Tactical/StructureWrap.h"

#include <stdio.h>

#include "SGP/WCheck.h"
#include "Strategic/StrategicMap.h"
#include "Tactical/Overhead.h"
#include "Tactical/RottingCorpses.h"
#include "TileEngine/IsometricUtils.h"
#include "TileEngine/RenderWorld.h"
#include "TileEngine/Structure.h"
#include "TileEngine/StructureInternals.h"
#include "TileEngine/TileDef.h"
#include "TileEngine/WorldMan.h"
#include "rust_sam_sites.h"

BOOLEAN IsFencePresentAtGridno(int16_t sGridNo) {
  if (FindStructure(sGridNo, STRUCTURE_ANYFENCE) != NULL) {
    return (TRUE);
  }

  return (FALSE);
}

BOOLEAN IsRoofPresentAtGridno(int16_t sGridNo) {
  if (FindStructure(sGridNo, STRUCTURE_ROOF) != NULL) {
    return (TRUE);
  }

  return (FALSE);
}

BOOLEAN IsJumpableFencePresentAtGridno(int16_t sGridNo) {
  struct STRUCTURE *pStructure;

  pStructure = FindStructure(sGridNo, STRUCTURE_OBSTACLE);

  if (pStructure) {
    if (pStructure->fFlags & STRUCTURE_FENCE && !(pStructure->fFlags & STRUCTURE_SPECIAL)) {
      return (TRUE);
    }
    if (pStructure->pDBStructureRef->pDBStructure->ubArmour == MATERIAL_SANDBAG &&
        StructureHeight(pStructure) < 2) {
      return (TRUE);
    }
  }

  return (FALSE);
}

BOOLEAN IsDoorPresentAtGridno(int16_t sGridNo) {
  if (FindStructure(sGridNo, STRUCTURE_ANYDOOR) != NULL) {
    return (TRUE);
  }

  return (FALSE);
}

BOOLEAN IsTreePresentAtGridno(int16_t sGridNo) {
  if (FindStructure(sGridNo, STRUCTURE_TREE) != NULL) {
    return (TRUE);
  }

  return (FALSE);
}

struct LEVELNODE *IsWallPresentAtGridno(int16_t sGridNo) {
  struct LEVELNODE *pNode = NULL;
  struct STRUCTURE *pStructure;

  pStructure = FindStructure(sGridNo, STRUCTURE_WALLSTUFF);

  if (pStructure != NULL) {
    pNode = FindLevelNodeBasedOnStructure(sGridNo, pStructure);
  }

  return (pNode);
}

struct LEVELNODE *GetWallLevelNodeOfSameOrientationAtGridno(int16_t sGridNo, int8_t ubOrientation) {
  struct LEVELNODE *pNode = NULL;
  struct STRUCTURE *pStructure;

  pStructure = FindStructure(sGridNo, STRUCTURE_WALLSTUFF);

  while (pStructure != NULL) {
    // Check orientation
    if (pStructure->ubWallOrientation == ubOrientation) {
      pNode = FindLevelNodeBasedOnStructure(sGridNo, pStructure);
      return (pNode);
    }
    pStructure = FindNextStructure(pStructure, STRUCTURE_WALLSTUFF);
  }

  return (NULL);
}

struct LEVELNODE *GetWallLevelNodeAndStructOfSameOrientationAtGridno(
    int16_t sGridNo, int8_t ubOrientation, struct STRUCTURE **ppStructure) {
  struct LEVELNODE *pNode = NULL;
  struct STRUCTURE *pStructure, *pBaseStructure;

  (*ppStructure) = NULL;

  pStructure = FindStructure(sGridNo, STRUCTURE_WALLSTUFF);

  while (pStructure != NULL) {
    // Check orientation
    if (pStructure->ubWallOrientation == ubOrientation) {
      pBaseStructure = FindBaseStructure(pStructure);
      if (pBaseStructure) {
        pNode = FindLevelNodeBasedOnStructure(pBaseStructure->sGridNo, pBaseStructure);
        (*ppStructure) = pBaseStructure;
        return (pNode);
      }
    }
    pStructure = FindNextStructure(pStructure, STRUCTURE_WALLSTUFF);
  }

  return (NULL);
}

BOOLEAN IsDoorVisibleAtGridNo(int16_t sGridNo) {
  struct STRUCTURE *pStructure;
  int16_t sNewGridNo;

  pStructure = FindStructure(sGridNo, STRUCTURE_ANYDOOR);

  if (pStructure != NULL) {
    // Check around based on orientation
    switch (pStructure->ubWallOrientation) {
      case INSIDE_TOP_LEFT:
      case OUTSIDE_TOP_LEFT:

        // Here, check north direction
        sNewGridNo = NewGridNo(sGridNo, DirectionInc(NORTH));

        if (IsRoofVisible2(sNewGridNo)) {
          // OK, now check south, if true, she's not visible
          sNewGridNo = NewGridNo(sGridNo, DirectionInc(SOUTH));

          if (IsRoofVisible2(sNewGridNo)) {
            return (FALSE);
          }
        }
        break;

      case INSIDE_TOP_RIGHT:
      case OUTSIDE_TOP_RIGHT:

        // Here, check west direction
        sNewGridNo = NewGridNo(sGridNo, DirectionInc(WEST));

        if (IsRoofVisible2(sNewGridNo)) {
          // OK, now check south, if true, she's not visible
          sNewGridNo = NewGridNo(sGridNo, DirectionInc(EAST));

          if (IsRoofVisible2(sNewGridNo)) {
            return (FALSE);
          }
        }
        break;
    }
  }

  // Return true here, even if she does not exist
  return (TRUE);
}

BOOLEAN DoesGridnoContainHiddenStruct(int16_t sGridNo, BOOLEAN *pfVisible) {
  // ATE: These are ignored now - always return false

  // struct STRUCTURE *pStructure;

  // pStructure = FindStructure( sGridNo, STRUCTURE_HIDDEN );

  // if ( pStructure != NULL )
  //{
  //	if ( !(gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REVEALED ) &&
  //!(gTacticalStatus.uiFlags&SHOW_ALL_MERCS)  )
  //	{
  //		*pfVisible = FALSE;
  //	}
  //	else
  //	{
  //		*pfVisible = TRUE;
  //	}//
  //
  //	return( TRUE );
  //}

  return (FALSE);
}

BOOLEAN IsHiddenStructureVisible(int16_t sGridNo, uint16_t usIndex) {
  // Check if it's a hidden struct and we have not revealed anything!
  if (gTileDatabase[usIndex].uiFlags & HIDDEN_TILE) {
    if (!(gpWorldLevelData[sGridNo].uiFlags & MAPELEMENT_REVEALED) &&
        !(gTacticalStatus.uiFlags & SHOW_ALL_MERCS)) {
      // Return false
      return (FALSE);
    }
  }

  return (TRUE);
}

BOOLEAN WallExistsOfTopLeftOrientation(int16_t sGridNo) {
  // CJC: changing to search only for normal walls, July 16, 1998
  struct STRUCTURE *pStructure;

  pStructure = FindStructure(sGridNo, STRUCTURE_WALL);

  while (pStructure != NULL) {
    // Check orientation
    if (pStructure->ubWallOrientation == INSIDE_TOP_LEFT ||
        pStructure->ubWallOrientation == OUTSIDE_TOP_LEFT) {
      return (TRUE);
    }

    pStructure = FindNextStructure(pStructure, STRUCTURE_WALL);
  }

  return (FALSE);
}

BOOLEAN WallExistsOfTopRightOrientation(int16_t sGridNo) {
  // CJC: changing to search only for normal walls, July 16, 1998
  struct STRUCTURE *pStructure;

  pStructure = FindStructure(sGridNo, STRUCTURE_WALL);

  while (pStructure != NULL) {
    // Check orientation
    if (pStructure->ubWallOrientation == INSIDE_TOP_RIGHT ||
        pStructure->ubWallOrientation == OUTSIDE_TOP_RIGHT) {
      return (TRUE);
    }

    pStructure = FindNextStructure(pStructure, STRUCTURE_WALL);
  }

  return (FALSE);
}

BOOLEAN WallOrClosedDoorExistsOfTopLeftOrientation(int16_t sGridNo) {
  struct STRUCTURE *pStructure;

  pStructure = FindStructure(sGridNo, STRUCTURE_WALLSTUFF);

  while (pStructure != NULL) {
    // skip it if it's an open door
    if (!((pStructure->fFlags & STRUCTURE_ANYDOOR) && (pStructure->fFlags & STRUCTURE_OPEN))) {
      // Check orientation
      if (pStructure->ubWallOrientation == INSIDE_TOP_LEFT ||
          pStructure->ubWallOrientation == OUTSIDE_TOP_LEFT) {
        return (TRUE);
      }
    }

    pStructure = FindNextStructure(pStructure, STRUCTURE_WALLSTUFF);
  }

  return (FALSE);
}

BOOLEAN WallOrClosedDoorExistsOfTopRightOrientation(int16_t sGridNo) {
  struct STRUCTURE *pStructure;

  pStructure = FindStructure(sGridNo, STRUCTURE_WALLSTUFF);

  while (pStructure != NULL) {
    // skip it if it's an open door
    if (!((pStructure->fFlags & STRUCTURE_ANYDOOR) && (pStructure->fFlags & STRUCTURE_OPEN))) {
      // Check orientation
      if (pStructure->ubWallOrientation == INSIDE_TOP_RIGHT ||
          pStructure->ubWallOrientation == OUTSIDE_TOP_RIGHT) {
        return (TRUE);
      }
    }

    pStructure = FindNextStructure(pStructure, STRUCTURE_WALLSTUFF);
  }

  return (FALSE);
}

BOOLEAN OpenRightOrientedDoorWithDoorOnRightOfEdgeExists(int16_t sGridNo) {
  struct STRUCTURE *pStructure;

  pStructure = FindStructure(sGridNo, STRUCTURE_ANYDOOR);

  while (pStructure != NULL && (pStructure->fFlags & STRUCTURE_OPEN)) {
    // Check orientation
    if (pStructure->ubWallOrientation == INSIDE_TOP_RIGHT ||
        pStructure->ubWallOrientation == OUTSIDE_TOP_RIGHT) {
      if ((pStructure->fFlags & STRUCTURE_DOOR) || (pStructure->fFlags & STRUCTURE_DDOOR_RIGHT)) {
        return (TRUE);
      }
    }

    pStructure = FindNextStructure(pStructure, STRUCTURE_ANYDOOR);
  }

  return (FALSE);
}

BOOLEAN OpenLeftOrientedDoorWithDoorOnLeftOfEdgeExists(int16_t sGridNo) {
  struct STRUCTURE *pStructure;

  pStructure = FindStructure(sGridNo, STRUCTURE_ANYDOOR);

  while (pStructure != NULL && (pStructure->fFlags & STRUCTURE_OPEN)) {
    // Check orientation
    if (pStructure->ubWallOrientation == INSIDE_TOP_LEFT ||
        pStructure->ubWallOrientation == OUTSIDE_TOP_LEFT) {
      if ((pStructure->fFlags & STRUCTURE_DOOR) || (pStructure->fFlags & STRUCTURE_DDOOR_LEFT)) {
        return (TRUE);
      }
    }

    pStructure = FindNextStructure(pStructure, STRUCTURE_ANYDOOR);
  }

  return (FALSE);
}

struct STRUCTURE *FindCuttableWireFenceAtGridNo(int16_t sGridNo) {
  struct STRUCTURE *pStructure;

  pStructure = FindStructure(sGridNo, STRUCTURE_WIREFENCE);
  if (pStructure != NULL && pStructure->ubWallOrientation != NO_ORIENTATION &&
      !(pStructure->fFlags & STRUCTURE_OPEN)) {
    return (pStructure);
  }
  return (NULL);
}

BOOLEAN CutWireFence(int16_t sGridNo) {
  struct STRUCTURE *pStructure;

  pStructure = FindCuttableWireFenceAtGridNo(sGridNo);
  if (pStructure) {
    pStructure = SwapStructureForPartnerAndStoreChangeInMap(sGridNo, pStructure);
    if (pStructure) {
      RecompileLocalMovementCosts(sGridNo);
      SetRenderFlags(RENDER_FLAG_FULL);
      return (TRUE);
    }
  }
  return (FALSE);
}

BOOLEAN IsCuttableWireFenceAtGridNo(int16_t sGridNo) {
  return (FindCuttableWireFenceAtGridNo(sGridNo) != NULL);
}

BOOLEAN IsRepairableStructAtGridNo(int16_t sGridNo, uint8_t *pubID) {
  uint8_t ubMerc;

  // OK, first look for a vehicle....
  ubMerc = WhoIsThere2(sGridNo, 0);

  if (pubID != NULL) {
    (*pubID) = ubMerc;
  }

  if (ubMerc != NOBODY) {
    if (MercPtrs[ubMerc]->uiStatusFlags & SOLDIER_VEHICLE) {
      return (2);
    }
  }
  // Then for over a robot....

  // then for SAM site....
  if (DoesSAMExistHere((u8)gWorldSectorX, (u8)gWorldSectorY, gbWorldSectorZ, sGridNo)) {
    return (3);
  }

  return (FALSE);
}

BOOLEAN IsRefuelableStructAtGridNo(int16_t sGridNo, uint8_t *pubID) {
  uint8_t ubMerc;

  // OK, first look for a vehicle....
  ubMerc = WhoIsThere2(sGridNo, 0);

  if (pubID != NULL) {
    (*pubID) = ubMerc;
  }

  if (ubMerc != NOBODY) {
    if (MercPtrs[ubMerc]->uiStatusFlags & SOLDIER_VEHICLE) {
      return (TRUE);
    }
  }
  return (FALSE);
}

BOOLEAN IsCutWireFenceAtGridNo(int16_t sGridNo) {
  struct STRUCTURE *pStructure;

  pStructure = FindStructure(sGridNo, STRUCTURE_WIREFENCE);
  if (pStructure != NULL && (pStructure->ubWallOrientation != NO_ORIENTATION) &&
      (pStructure->fFlags & STRUCTURE_OPEN)) {
    return (TRUE);
  }
  return (FALSE);
}

int16_t FindDoorAtGridNoOrAdjacent(int16_t sGridNo) {
  struct STRUCTURE *pStructure;
  struct STRUCTURE *pBaseStructure;
  int16_t sTestGridNo;

  sTestGridNo = sGridNo;
  pStructure = FindStructure(sTestGridNo, STRUCTURE_ANYDOOR);
  if (pStructure) {
    pBaseStructure = FindBaseStructure(pStructure);
    return (pBaseStructure->sGridNo);
  }

  sTestGridNo = sGridNo + DirectionInc(NORTH);
  pStructure = FindStructure(sTestGridNo, STRUCTURE_ANYDOOR);
  if (pStructure) {
    pBaseStructure = FindBaseStructure(pStructure);
    return (pBaseStructure->sGridNo);
  }

  sTestGridNo = sGridNo + DirectionInc(WEST);
  pStructure = FindStructure(sTestGridNo, STRUCTURE_ANYDOOR);
  if (pStructure) {
    pBaseStructure = FindBaseStructure(pStructure);
    return (pBaseStructure->sGridNo);
  }

  return (NOWHERE);
}

BOOLEAN IsCorpseAtGridNo(int16_t sGridNo, uint8_t ubLevel) {
  if (GetCorpseAtGridNo(sGridNo, ubLevel) != NULL) {
    return (TRUE);
  } else {
    return (FALSE);
  }
}

BOOLEAN SetOpenableStructureToClosed(int16_t sGridNo, uint8_t ubLevel) {
  struct STRUCTURE *pStructure;
  struct STRUCTURE *pNewStructure;

  pStructure = FindStructure(sGridNo, STRUCTURE_OPENABLE);
  if (!pStructure) {
    return (FALSE);
  }

  if (pStructure->fFlags & STRUCTURE_OPEN) {
    pNewStructure = SwapStructureForPartner(sGridNo, pStructure);
    if (pNewStructure != NULL) {
      RecompileLocalMovementCosts(sGridNo);
      SetRenderFlags(RENDER_FLAG_FULL);
    }
  }
  // else leave it as is!
  return (TRUE);
}
