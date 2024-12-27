#include "TileEngine/RenderFun.h"

#include <errno.h>
#include <math.h>
#include <stdio.h>

#include "JAScreens.h"
#include "SGP/Debug.h"
#include "SGP/Random.h"
#include "SGP/WCheck.h"
#include "Tactical/DialogueControl.h"
#include "Tactical/FOV.h"
#include "Tactical/HandleItems.h"
#include "TileEngine/Environment.h"
#include "TileEngine/FogOfWar.h"
#include "TileEngine/IsometricUtils.h"
#include "TileEngine/RenderWorld.h"
#include "TileEngine/Structure.h"
#include "TileEngine/StructureInternals.h"
#include "TileEngine/SysUtil.h"
#include "TileEngine/TileDef.h"
#include "TileEngine/WorldMan.h"
#include "Utils/TimerControl.h"

// Room Information
uint8_t gubWorldRoomInfo[WORLD_MAX];
uint8_t gubWorldRoomHidden[MAX_ROOMS];

BOOLEAN InitRoomDatabase() {
  memset(gubWorldRoomInfo, NO_ROOM, sizeof(gubWorldRoomInfo));
  memset(gubWorldRoomHidden, TRUE, sizeof(gubWorldRoomHidden));
  return (TRUE);
}

void ShutdownRoomDatabase() {}

void SetTileRoomNum(int16_t sGridNo, uint8_t ubRoomNum) {
  // Add to global room list
  gubWorldRoomInfo[sGridNo] = ubRoomNum;
}

void SetTileRangeRoomNum(SGPRect *pSelectRegion, uint8_t ubRoomNum) {
  int32_t cnt1, cnt2;

  for (cnt1 = pSelectRegion->iTop; cnt1 <= pSelectRegion->iBottom; cnt1++) {
    for (cnt2 = pSelectRegion->iLeft; cnt2 <= pSelectRegion->iRight; cnt2++) {
      gubWorldRoomInfo[(int16_t)MAPROWCOLTOPOS(cnt1, cnt2)] = ubRoomNum;
    }
  }
}

BOOLEAN InARoom(uint16_t sGridNo, uint8_t *pubRoomNo) {
  if (gubWorldRoomInfo[sGridNo] != NO_ROOM) {
    if (pubRoomNo) {
      *pubRoomNo = gubWorldRoomInfo[sGridNo];
    }
    return (TRUE);
  }

  return (FALSE);
}

BOOLEAN InAHiddenRoom(uint16_t sGridNo, uint8_t *pubRoomNo) {
  if (gubWorldRoomInfo[sGridNo] != NO_ROOM) {
    if ((gubWorldRoomHidden[gubWorldRoomInfo[sGridNo]])) {
      *pubRoomNo = gubWorldRoomInfo[sGridNo];
      return (TRUE);
    }
  }

  return (FALSE);
}

// @@ATECLIP TO WORLD!
void SetRecalculateWireFrameFlagRadius(int16_t sX, int16_t sY, int16_t sRadius) {
  int16_t sCountX, sCountY;
  uint32_t uiTile;

  for (sCountY = sY - sRadius; sCountY < (sY + sRadius + 2); sCountY++) {
    for (sCountX = sX - sRadius; sCountX < (sX + sRadius + 2); sCountX++) {
      uiTile = MAPROWCOLTOPOS(sCountY, sCountX);

      gpWorldLevelData[uiTile].uiFlags |= MAPELEMENT_RECALCULATE_WIREFRAMES;
    }
  }
}

void SetGridNoRevealedFlag(uint16_t sGridNo) {
  //	uint32_t cnt;
  //  struct ITEM_POOL					*pItemPool;
  //	int16_t							sX, sY;
  struct LEVELNODE *pNode = NULL;
  struct STRUCTURE *pStructure, *pBase;

  // Set hidden flag, for any roofs
  SetRoofIndexFlagsFromTypeRange(sGridNo, FIRSTROOF, FOURTHROOF, LEVELNODE_HIDDEN);

  // ATE: Do this only if we are in a room...
  if (gubWorldRoomInfo[sGridNo] != NO_ROOM) {
    SetStructAframeFlags(sGridNo, LEVELNODE_HIDDEN);
    // Find gridno one east as well...

    if ((sGridNo + WORLD_COLS) < NOWHERE) {
      SetStructAframeFlags(sGridNo + WORLD_COLS, LEVELNODE_HIDDEN);
    }

    if ((sGridNo + 1) < NOWHERE) {
      SetStructAframeFlags(sGridNo + 1, LEVELNODE_HIDDEN);
    }
  }

  // Set gridno as revealed
  gpWorldLevelData[sGridNo].uiFlags |= MAPELEMENT_REVEALED;
  if (gfCaves) {
    RemoveFogFromGridNo(sGridNo);
  }

  // ATE: If there are any structs here, we can render them with the obscured flag!
  // Look for anything but walls pn this gridno!
  pStructure = gpWorldLevelData[(int16_t)sGridNo].pStructureHead;

  while (pStructure != NULL) {
    if (pStructure->sCubeOffset == STRUCTURE_ON_GROUND ||
        (pStructure->fFlags & STRUCTURE_SLANTED_ROOF)) {
      if (((pStructure->fFlags & STRUCTURE_OBSTACLE) &&
           !(pStructure->fFlags & (STRUCTURE_PERSON | STRUCTURE_CORPSE))) ||
          (pStructure->fFlags & STRUCTURE_SLANTED_ROOF)) {
        pBase = FindBaseStructure(pStructure);

        // Get struct LEVELNODE for struct and remove!
        pNode = FindLevelNodeBasedOnStructure(pBase->sGridNo, pBase);

        if (pNode) pNode->uiFlags |= LEVELNODE_SHOW_THROUGH;

        if (pStructure->fFlags & STRUCTURE_SLANTED_ROOF) {
          AddSlantRoofFOVSlot(pBase->sGridNo);

          // Set hidden...
          pNode->uiFlags |= LEVELNODE_HIDDEN;
        }
      }
    }

    pStructure = pStructure->pNext;
  }

  gubWorldRoomHidden[gubWorldRoomInfo[sGridNo]] = FALSE;
}

void ExamineGridNoForSlantRoofExtraGraphic(uint16_t sCheckGridNo) {
  struct LEVELNODE *pNode = NULL;
  struct STRUCTURE *pStructure, *pBase;
  uint8_t ubLoop;
  DB_STRUCTURE_TILE **ppTile;
  int16_t sGridNo;
  uint16_t usIndex;
  BOOLEAN fChanged = FALSE;

  // CHECK FOR A SLANTED ROOF HERE....
  pStructure = FindStructure(sCheckGridNo, STRUCTURE_SLANTED_ROOF);

  if (pStructure != NULL) {
    // We have a slanted roof here ... find base and remove...
    pBase = FindBaseStructure(pStructure);

    // Get struct LEVELNODE for struct and remove!
    pNode = FindLevelNodeBasedOnStructure(pBase->sGridNo, pBase);

    // Loop through each gridno and see if revealed....
    for (ubLoop = 0; ubLoop < pBase->pDBStructureRef->pDBStructure->ubNumberOfTiles; ubLoop++) {
      ppTile = pBase->pDBStructureRef->ppTile;
      sGridNo = pBase->sGridNo + ppTile[ubLoop]->sPosRelToBase;

      if (sGridNo < 0 || sGridNo > WORLD_MAX) {
        continue;
      }

      // Given gridno,
      // IF NOT REVEALED AND HIDDEN....
      if (!(gpWorldLevelData[sGridNo].uiFlags & MAPELEMENT_REVEALED) &&
          pNode->uiFlags & LEVELNODE_HIDDEN) {
        // Add graphic if one does not already exist....
        if (!TypeExistsInRoofLayer(sGridNo, SLANTROOFCEILING, &usIndex)) {
          // Add
          AddRoofToHead(sGridNo, SLANTROOFCEILING1);
          fChanged = TRUE;
        }
      }

      // Revealed?
      if (gpWorldLevelData[sGridNo].uiFlags & MAPELEMENT_REVEALED) {
        /// Remove any slant roof items if they exist
        if (TypeExistsInRoofLayer(sGridNo, SLANTROOFCEILING, &usIndex)) {
          RemoveRoof(sGridNo, usIndex);
          fChanged = TRUE;
        }
      }
    }
  }

  if (fChanged) {
    // DIRTY THE WORLD!
    InvalidateWorldRedundency();
    SetRenderFlags(RENDER_FLAG_FULL);
  }
}

void RemoveRoomRoof(uint16_t sGridNo, uint8_t bRoomNum, struct SOLDIERTYPE *pSoldier) {
  uint32_t cnt;
  struct ITEM_POOL *pItemPool;
  int16_t sX, sY;
  BOOLEAN fSaidItemSeenQuote = FALSE;

  //	struct STRUCTURE					*pStructure;//, *pBase;

  // LOOP THORUGH WORLD AND CHECK ROOM INFO
  for (cnt = 0; cnt < WORLD_MAX; cnt++) {
    if (gubWorldRoomInfo[cnt] == bRoomNum) {
      SetGridNoRevealedFlag((uint16_t)cnt);

      RemoveRoofIndexFlagsFromTypeRange(cnt, FIRSTROOF, SECONDSLANTROOF, LEVELNODE_REVEAL);

      // Reveal any items if here!
      if (GetItemPool((int16_t)cnt, &pItemPool, 0)) {
        // Set visible! ( only if invisible... )
        if (SetItemPoolVisibilityOn(pItemPool, INVISIBLE, TRUE)) {
          if (!fSaidItemSeenQuote) {
            fSaidItemSeenQuote = TRUE;

            if (pSoldier != NULL) {
              TacticalCharacterDialogue(pSoldier,
                                        (uint16_t)(QUOTE_SPOTTED_SOMETHING_ONE + Random(2)));
            }
          }
        }
      }

      // OK, re-set writeframes ( in a radius )
      // Get XY
      ConvertGridNoToXY((int16_t)cnt, &sX, &sY);
      SetRecalculateWireFrameFlagRadius(sX, sY, 2);
    }
  }

  // for ( cnt = 0; cnt < WORLD_MAX; cnt++ )
  //{
  //	if ( gubWorldRoomInfo[ cnt ] == bRoomNum )
  //	{
  //		ExamineGridNoForSlantRoofExtraGraphic( (uint16_t)cnt );
  //	}
  //}

  // DIRTY THE WORLD!
  InvalidateWorldRedundency();
  SetRenderFlags(RENDER_FLAG_FULL);

  CalculateWorldWireFrameTiles(FALSE);
}

BOOLEAN AddSpecialTileRange(SGPRect *pSelectRegion) {
  int32_t cnt1, cnt2;

  for (cnt1 = pSelectRegion->iTop; cnt1 <= pSelectRegion->iBottom; cnt1++) {
    for (cnt2 = pSelectRegion->iLeft; cnt2 <= pSelectRegion->iRight; cnt2++) {
      AddObjectToHead((int16_t)MAPROWCOLTOPOS(cnt1, cnt2), SPECIALTILE_MAPEXIT);
    }
  }

  return (TRUE);
}

BOOLEAN RemoveSpecialTileRange(SGPRect *pSelectRegion) {
  int32_t cnt1, cnt2;

  for (cnt1 = pSelectRegion->iTop; cnt1 <= pSelectRegion->iBottom; cnt1++) {
    for (cnt2 = pSelectRegion->iLeft; cnt2 <= pSelectRegion->iRight; cnt2++) {
      RemoveObject((int16_t)MAPROWCOLTOPOS(cnt1, cnt2), SPECIALTILE_MAPEXIT);
    }
  }

  return (TRUE);
}
