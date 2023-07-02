#include "TileEngine/Lighting.h"

#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>

#include "Editor/EditSys.h"
#include "JAScreens.h"
#include "SGP/Debug.h"
#include "SGP/Input.h"
#include "SGP/Line.h"
#include "SGP/VObject.h"
#include "SGP/VObjectBlitters.h"
#include "SGP/VObjectInternal.h"
#include "SGP/VSurface.h"
#include "SGP/Video.h"
#include "SGP/WCheck.h"
#include "SysGlobals.h"
#include "Tactical/AnimationControl.h"
#include "Tactical/AnimationData.h"
#include "Tactical/PathAI.h"
#include "Tactical/RottingCorpses.h"
#include "Tactical/StructureWrap.h"
#include "TileEngine/Environment.h"
#include "TileEngine/IsometricUtils.h"
#include "TileEngine/RadarScreen.h"
#include "TileEngine/RenderWorld.h"
#include "TileEngine/ShadeTableUtil.h"
#include "TileEngine/Structure.h"
#include "TileEngine/StructureInternals.h"
#include "TileEngine/TileDef.h"
#include "TileEngine/WorldMan.h"
#include "Utils/FontControl.h"
#include "Utils/TimerControl.h"
#include "platform_strings.h"
#include "rust_fileman.h"
#include "rust_images.h"

#define LVL1_L1_PER (50)
#define LVL1_L2_PER (50)

#define LVL2_L1_PER (30)
#define LVL2_L2_PER (70)

#define LIGHT_TREE_REVEAL 5  // width of rect

// Local-use only prototypes

BOOLEAN LightTileBlocked(int16_t iSrcX, int16_t iSrcY, int16_t iX, int16_t iY);
BOOLEAN LightDraw(uint32_t uiLightType, int32_t iLight, int16_t iX, int16_t iY, uint32_t uiSprite);
BOOLEAN LightDelete(int32_t iLight);
int32_t LightGetFree(void);
int32_t LinearDistance(int16_t iX1, int16_t iY1, int16_t iX2, int16_t iY2);
double LinearDistanceDouble(int16_t iX1, int16_t iY1, int16_t iX2, int16_t iY2);
BOOLEAN LightAddTile(uint32_t uiLightType, int16_t iSrcX, int16_t iSrcY, int16_t iX, int16_t iY,
                     uint8_t ubShade, uint32_t uiFlags, BOOLEAN fOnlyWalls);
BOOLEAN LightSubtractTile(uint32_t uiLightType, int16_t iSrcX, int16_t iSrcY, int16_t iX,
                          int16_t iY, uint8_t ubShade, uint32_t uiFlags, BOOLEAN fOnlyWalls);
BOOLEAN LightResetTile(int16_t iX, int16_t iY);
BOOLEAN LightSetTile(int16_t iX, int16_t iY, uint8_t ubShade, uint32_t uiLightType);
BOOLEAN LightSetNaturalTile(int16_t iX, int16_t iY, uint8_t ubShade);
uint16_t LightGetLastNode(int32_t iLight);
BOOLEAN LightAddNode(int32_t iLight, int16_t iHotSpotX, int16_t iHotSpotY, int16_t iX, int16_t iY,
                     uint8_t ubIntensity, uint16_t uiFlags);
BOOLEAN LightInsertNode(int32_t iLight, uint16_t usLightIns, int16_t iHotSpotX, int16_t iHotSpotY,
                        int16_t iX, int16_t iY, uint8_t ubIntensity, uint16_t uiFlags);
uint16_t LightFindNextRay(int32_t iLight, uint16_t usIndex);
BOOLEAN LightCastRay(int32_t iLight, int16_t iStartX, int16_t iStartY, int16_t iEndPointX,
                     int16_t iEndPointY, uint8_t ubStartIntens, uint8_t ubEndIntens);
BOOLEAN LightGenerateElliptical(int32_t iLight, uint8_t iIntensity, int16_t iA, int16_t iB);
BOOLEAN LightGenerateBeam(int32_t iLight, uint8_t iIntensity, int16_t iLength, int16_t iRadius,
                          int16_t iDirection);
BOOLEAN LightCalcRect(int32_t iLight);
BOOLEAN LightIlluminateWall(int16_t iSourceX, int16_t iSourceY, int16_t iTileX, int16_t iTileY,
                            struct LEVELNODE *pStruct);
BOOLEAN LightTileHasWall(int16_t iSrcX, int16_t iSrcY, int16_t iX, int16_t iY);

int32_t LightSpriteGetFree(void);
BOOLEAN LightSpriteDirty(int32_t iLight);

// Top node of linked lists, NULL = FREE
LIGHT_NODE *pLightList[MAX_LIGHT_TEMPLATES];
uint16_t usTemplateSize[MAX_LIGHT_TEMPLATES];
uint16_t *pLightRayList[MAX_LIGHT_TEMPLATES];
uint16_t usRaySize[MAX_LIGHT_TEMPLATES];
int16_t LightHeight[MAX_LIGHT_TEMPLATES];
int16_t LightWidth[MAX_LIGHT_TEMPLATES];
int16_t LightXOffset[MAX_LIGHT_TEMPLATES];
int16_t LightYOffset[MAX_LIGHT_TEMPLATES];
int16_t LightMapLeft[MAX_LIGHT_TEMPLATES];
int16_t LightMapTop[MAX_LIGHT_TEMPLATES];
int16_t LightMapRight[MAX_LIGHT_TEMPLATES];
int16_t LightMapBottom[MAX_LIGHT_TEMPLATES];
char *pLightNames[MAX_LIGHT_TEMPLATES];

// Sprite data
LIGHT_SPRITE LightSprites[MAX_LIGHT_SPRITES];

// Lighting system general data
uint8_t ubAmbientLightLevel = DEFAULT_SHADE_LEVEL;
uint8_t gubNumLightColors = 1;

// Externed in Rotting Corpses.c
struct SGPPaletteEntry gpLightColors[3] = {{0, 0, 0, 0}, {0, 0, 255, 0}, {0, 0, 0, 0}};

struct SGPPaletteEntry gpOrigLights[2] = {{0, 0, 0, 0}, {0, 0, 255, 0}};

/*
uint16_t gusShadeLevels[16][3]={{500, 500, 500},				// green table
                                                                                                                        {450, 450, 450},		// bright
                                                                                                                        {350, 350, 350},
                                                                                                                        {300, 300, 300},
                                                                                                                        {255, 255, 255},		// normal
                                                                                                                        {227, 227, 227},
                                                                                                                        {198, 198, 198},
                                                                                                                        {171, 171, 171},
                                                                                                                        {143, 143, 143},
                                                                                                                        {115, 115, 160},				// darkening
                                                                                                                        {87, 87, 176},
                                                                                                                        {60, 60, 160},
                                                                                                                        {48, 48, 192},
                                                                                                                        {36, 36, 208},
                                                                                                                        {18, 18, 224},
                                                                                                                        {48, 222, 48}};
*/
// Externed in Rotting Corpses.c
// Kris' attempt at blue night lights
/*
uint16_t gusShadeLevels[16][3]={{500, 500, 500},				// green table
                                                                                                                        {450, 450, 450},		// bright
                                                                                                                        {350, 350, 350},
                                                                                                                        {300, 300, 300},
                                                                                                                        {255, 255, 255},		// normal
                                                                                                                        {215, 215, 227},
                                                                                                                        {179, 179, 179},
                                                                                                                        {149, 149, 149},
                                                                                                                        {125, 125, 128},
                                                                                                                        {104, 104, 128},				// darkening
                                                                                                                        {86, 86, 128},
                                                                                                                        {72, 72, 128},
                                                                                                                        {60, 60, 128},
                                                                                                                        {36, 36, 208},
                                                                                                                        {18, 18, 224},
                                                                                                                        {48, 222, 48}};
*/

/*
//Linda's final version

uint16_t gusShadeLevels[16][3] =
{
        500, 500, 500,
        450, 450, 450,	//bright
        350, 350, 350,
        300, 300, 300,
        255, 255, 255,	//normal
        222, 200, 200,
        174, 167, 167,
        150, 137, 137,
        122, 116, 116,	//darkening
        96, 96, 96,
        77, 77, 84,
        58, 58, 69,
        44, 44, 66,			//night
        36, 36, 244,
        18, 18, 224,
        48, 222, 48,
};
*/

// JA2 Gold:
uint16_t gusShadeLevels[16][3] = {
    {500, 500, 500}, {450, 450, 450},                                    // bright
    {350, 350, 350}, {300, 300, 300}, {255, 255, 255},                   // normal
    {231, 199, 199}, {209, 185, 185}, {187, 171, 171}, {165, 157, 157},  // darkening
    {143, 143, 143}, {121, 121, 129}, {99, 99, 115},   {77, 77, 101},    // night
    {36, 36, 244},   {18, 18, 224},   {48, 222, 48},
};

// Set this true if you want the shadetables to be loaded from the text file.
BOOLEAN gfLoadShadeTablesFromTextFile = FALSE;

void LoadShadeTablesFromTextFile() {
  FILE *fp;
  int32_t i, j;
  int32_t num;
  char str[10];

  if (gfLoadShadeTablesFromTextFile) {
    fp = fopen("ShadeTables.txt", "r");
    Assert(fp);
    if (fp) {
      for (i = 0; i < 16; i++) {
        for (j = 0; j < 3; j++) {
          fscanf(fp, "%s", str);
          sscanf(str, "%d", &num);
          gusShadeLevels[i][j] = (uint16_t)num;
        }
      }
      fclose(fp);
    }
  }
}

// Debug variable
uint32_t gNodesAdded = 0;

/****************************************************************************************
 InitLightingSystem

        Initializes the lighting system.

***************************************************************************************/
BOOLEAN InitLightingSystem(void) {
  uint32_t uiCount;

  LoadShadeTablesFromTextFile();

  // init all light lists
  for (uiCount = 0; uiCount < MAX_LIGHT_TEMPLATES; uiCount++) {
    pLightList[uiCount] = NULL;
    pLightNames[uiCount] = NULL;
    pLightRayList[uiCount] = NULL;
    usTemplateSize[uiCount] = 0;
    usRaySize[uiCount] = 0;
  }

  // init all light sprites
  for (uiCount = 0; uiCount < MAX_LIGHT_SPRITES; uiCount++)
    memset(&LightSprites[uiCount], 0, sizeof(LIGHT_SPRITE));

  if (LightLoad("TRANSLUC.LHT") != 0) {
    DebugMsg(TOPIC_GAME, DBG_ERROR, String("Failed to load translucency template"));
    return (FALSE);
  }

  return (TRUE);
}

// THIS MUST BE CALLED ONCE ALL SURFACE VIDEO OBJECTS HAVE BEEN LOADED!
BOOLEAN SetDefaultWorldLightingColors(void) {
  struct SGPPaletteEntry pPal[2];

  pPal[0].peRed = 0;
  pPal[0].peGreen = 0;
  pPal[0].peBlue = 0;
  pPal[1].peRed = 0;
  pPal[1].peGreen = 0;
  pPal[1].peBlue = 128;

  LightSetColors(&pPal[0], 1);

  return (TRUE);
}

/****************************************************************************************
 ShutdownLightingSystem

        Closes down the lighting system. Any lights that were created are destroyed, and the
        memory attached to them freed up.

***************************************************************************************/
BOOLEAN ShutdownLightingSystem(void) {
  uint32_t uiCount;

  // free up all allocated light nodes
  for (uiCount = 0; uiCount < MAX_LIGHT_TEMPLATES; uiCount++)
    if (pLightList[uiCount] != NULL) LightDelete(uiCount);

  return (TRUE);
}

/****************************************************************************************
 LightReset

        Removes all currently active lights, without undrawing them.

***************************************************************************************/
BOOLEAN LightReset(void) {
  uint32_t uiCount;

  // reset all light lists
  for (uiCount = 0; uiCount < MAX_LIGHT_TEMPLATES; uiCount++)
    if (pLightList[uiCount] != NULL) LightDelete(uiCount);

  // init all light sprites
  for (uiCount = 0; uiCount < MAX_LIGHT_SPRITES; uiCount++)
    memset(&LightSprites[uiCount], 0, sizeof(LIGHT_SPRITE));

  if (LightLoad("TRANSLUC.LHT") != 0) {
    DebugMsg(TOPIC_GAME, DBG_ERROR, String("Failed to load translucency template"));
    return (FALSE);
  }

  // Loop through mercs and reset light value
  for (uiCount = 0; uiCount < MAX_NUM_SOLDIERS; uiCount++) {
    MercPtrs[uiCount]->iLight = -1;
  }

  return (TRUE);
}

/****************************************************************************************
 LightCreateTemplateNode

        Creates a new node, and appends it to the template list. The index into the list is
        returned.

***************************************************************************************/
uint16_t LightCreateTemplateNode(int32_t iLight, int16_t iX, int16_t iY, uint8_t ubLight) {
  uint16_t usNumNodes;

  // create a new list
  if (pLightList[iLight] == NULL) {
    if ((pLightList[iLight] = (LIGHT_NODE *)MemAlloc(sizeof(LIGHT_NODE))) == NULL) return (65535);

    pLightList[iLight]->iDX = iX;
    pLightList[iLight]->iDY = iY;
    pLightList[iLight]->ubLight = ubLight;
    pLightList[iLight]->uiFlags = 0;

    usTemplateSize[iLight] = 1;
    return (0);
  } else {
    usNumNodes = usTemplateSize[iLight];
    pLightList[iLight] =
        (LIGHT_NODE *)MemRealloc(pLightList[iLight], (usNumNodes + 1) * sizeof(LIGHT_NODE));
    (pLightList[iLight] + usNumNodes)->iDX = iX;
    (pLightList[iLight] + usNumNodes)->iDY = iY;
    (pLightList[iLight] + usNumNodes)->ubLight = ubLight;
    (pLightList[iLight] + usNumNodes)->uiFlags = 0;
    usTemplateSize[iLight] = usNumNodes + 1;
    return (usNumNodes);
  }
}

/****************************************************************************************
 LightAddTemplateNode

        Adds a node to the template list. If the node does not exist, it creates a new one.
        Returns the index into the list.

***************************************************************************************/
uint16_t LightAddTemplateNode(int32_t iLight, int16_t iX, int16_t iY, uint8_t ubLight) {
  uint16_t usCount;

  for (usCount = 0; usCount < usTemplateSize[iLight]; usCount++) {
    if (((pLightList[iLight] + usCount)->iDX == iX) &&
        ((pLightList[iLight] + usCount)->iDY == iY)) {
      return (usCount);
    }
  }

  return (LightCreateTemplateNode(iLight, iX, iY, ubLight));
}

/****************************************************************************************
 LightAddRayNode

        Adds a node to the ray casting list.

***************************************************************************************/
uint16_t LightAddRayNode(int32_t iLight, int16_t iX, int16_t iY, uint8_t ubLight,
                         uint16_t usFlags) {
  uint16_t usNumNodes;

  // create a new list
  if (pLightRayList[iLight] == NULL) {
    if ((pLightRayList[iLight] = (uint16_t *)MemAlloc(sizeof(uint16_t))) == NULL) return (65535);

    *pLightRayList[iLight] = (LightAddTemplateNode(iLight, iX, iY, ubLight) | usFlags);

    usRaySize[iLight] = 1;
    return (0);
  } else {
    usNumNodes = usRaySize[iLight];
    pLightRayList[iLight] =
        (uint16_t *)MemRealloc(pLightRayList[iLight], (usNumNodes + 1) * sizeof(uint16_t));
    *(pLightRayList[iLight] + usNumNodes) =
        (LightAddTemplateNode(iLight, iX, iY, ubLight) | usFlags);
    usRaySize[iLight] = usNumNodes + 1;
    return (usNumNodes);
  }
}

/****************************************************************************************
 LightAddRayNode

        Adds a node to the ray casting list.

***************************************************************************************/
uint16_t LightInsertRayNode(int32_t iLight, uint16_t usIndex, int16_t iX, int16_t iY,
                            uint8_t ubLight, uint16_t usFlags) {
  uint16_t usNumNodes;

  // create a new list
  if (pLightRayList[iLight] == NULL) {
    if ((pLightRayList[iLight] = (uint16_t *)MemAlloc(sizeof(uint16_t))) == NULL) return (65535);

    *pLightRayList[iLight] = (LightAddTemplateNode(iLight, iX, iY, ubLight) | usFlags);

    usRaySize[iLight] = 1;
    return (0);
  } else {
    usNumNodes = usRaySize[iLight];
    pLightRayList[iLight] =
        (uint16_t *)MemRealloc(pLightRayList[iLight], (usNumNodes + 1) * sizeof(uint16_t));

    if (usIndex < usRaySize[iLight]) {
      memmove(pLightRayList[iLight] + usIndex + 1, pLightRayList[iLight] + usIndex,
              (usRaySize[iLight] - usIndex) * sizeof(uint16_t));
    }

    *(pLightRayList[iLight] + usIndex) = (LightAddTemplateNode(iLight, iX, iY, ubLight) | usFlags);
    usRaySize[iLight] = usNumNodes + 1;
    return (usNumNodes);
  }
}

/****************************************************************************************
 LightTileBlocked

        Returns TRUE/FALSE if the tile at the specified tile number can block light.

***************************************************************************************/
BOOLEAN LightTileBlocked(int16_t iSrcX, int16_t iSrcY, int16_t iX, int16_t iY) {
  uint16_t usTileNo, usSrcTileNo;

  Assert(gpWorldLevelData != NULL);

  usTileNo = MAPROWCOLTOPOS(iY, iX);
  usSrcTileNo = MAPROWCOLTOPOS(iSrcY, iSrcX);

  if (usTileNo >= NOWHERE) {
    return (FALSE);
  }

  if (usSrcTileNo >= NOWHERE) {
    return (FALSE);
  }

  if (gpWorldLevelData[usTileNo].sHeight > gpWorldLevelData[usSrcTileNo].sHeight) return (TRUE);
  {
    uint16_t usTileNo;
    struct LEVELNODE *pStruct;

    usTileNo = MAPROWCOLTOPOS(iY, iX);

    pStruct = gpWorldLevelData[usTileNo].pStructHead;
    if (pStruct != NULL) {
      // IF WE ARE A WINDOW, DO NOT BLOCK!
      if (FindStructure(usTileNo, STRUCTURE_WALLNWINDOW) != NULL) {
        return (FALSE);
      }
    }
  }

  return (LightTileHasWall(iSrcX, iSrcY, iX, iY));
}

/****************************************************************************************
 LightTileHasWall

        Returns TRUE/FALSE if the tile at the specified coordinates contains a wall.

***************************************************************************************/
BOOLEAN LightTileHasWall(int16_t iSrcX, int16_t iSrcY, int16_t iX, int16_t iY) {
  // struct LEVELNODE *pStruct;
  // uint32_t uiType;
  uint16_t usTileNo;
  uint16_t usSrcTileNo;
  int8_t bDirection;
  uint8_t ubTravelCost;
  // int8_t		bWallCount = 0;
  // uint16_t	usWallOrientation;

  Assert(gpWorldLevelData != NULL);

  usTileNo = MAPROWCOLTOPOS(iY, iX);
  usSrcTileNo = MAPROWCOLTOPOS(iSrcY, iSrcX);

  if (usTileNo == usSrcTileNo) {
    return (FALSE);
  }

  // if ( usTileNo == 10125 || usTileNo == 10126 )
  //{
  //	int i = 0;
  //}

  if (usTileNo >= NOWHERE) {
    return (FALSE);
  }

  if (usSrcTileNo >= NOWHERE) {
    return (FALSE);
  }

  // Get direction
  // bDirection = atan8( iX, iY, iSrcX, iSrcY );
  bDirection = atan8(iSrcX, iSrcY, iX, iY);

  ubTravelCost = gubWorldMovementCosts[usTileNo][bDirection][0];

  if (ubTravelCost == TRAVELCOST_WALL) {
    return (TRUE);
  }

  if (IS_TRAVELCOST_DOOR(ubTravelCost)) {
    ubTravelCost = DoorTravelCost(NULL, usTileNo, ubTravelCost, TRUE, NULL);

    if (ubTravelCost == TRAVELCOST_OBSTACLE || ubTravelCost == TRAVELCOST_DOOR) {
      return (TRUE);
    }
  }

  return (FALSE);
}

/****************************************************************************************
 LightDelete

        Removes a light template from the list, and frees up the associated node memory.

***************************************************************************************/
BOOLEAN LightDelete(int32_t iLight) {
  if (pLightList[iLight] != NULL) {
    if (pLightList[iLight] != NULL) {
      MemFree(pLightList[iLight]);
      pLightList[iLight] = NULL;
    }

    if (pLightRayList[iLight] != NULL) {
      MemFree(pLightRayList[iLight]);
      pLightRayList[iLight] = NULL;
    }

    if (pLightNames[iLight] != NULL) {
      MemFree(pLightNames[iLight]);
      pLightNames[iLight] = NULL;
    }

    usTemplateSize[iLight] = 0;
    usRaySize[iLight] = 0;

    return (TRUE);
  } else
    return (FALSE);
}

/****************************************************************************************
 LightGetFree

        Returns an available slot for a new light template. If none are available, (-1) is
        returned.

***************************************************************************************/
int32_t LightGetFree(void) {
  uint32_t uiCount;

  for (uiCount = 0; uiCount < MAX_LIGHT_TEMPLATES; uiCount++)
    if (pLightList[uiCount] == NULL) return (uiCount);

  return (-1);
}

/****************************************************************************************
 LinearDistance

        Calculates the 2D linear distance between two points.

***************************************************************************************/
int32_t LinearDistance(int16_t iX1, int16_t iY1, int16_t iX2, int16_t iY2) {
  int32_t iDx, iDy;

  iDx = abs(iX1 - iX2);
  iDx *= iDx;
  iDy = abs(iY1 - iY2);
  iDy *= iDy;

  return ((int32_t)sqrt((double)(iDx + iDy)));
}

/****************************************************************************************
        LinearDistanceDouble

                Calculates the 2D linear distance between two points. Returns the result in a double
        for greater accuracy.

***************************************************************************************/
double LinearDistanceDouble(int16_t iX1, int16_t iY1, int16_t iX2, int16_t iY2) {
  int32_t iDx, iDy;

  iDx = abs(iX1 - iX2);
  iDx *= iDx;
  iDy = abs(iY1 - iY2);
  iDy *= iDy;

  return (sqrt((double)(iDx + iDy)));
}

/****************************************************************************************
        LightTrueLevel

                Returns the light level at a particular level without fake lights

***************************************************************************************/
uint8_t LightTrueLevel(int16_t sGridNo, int8_t bLevel) {
  struct LEVELNODE *pNode;
  int32_t iSum;

  if (bLevel == 0) {
    pNode = gpWorldLevelData[sGridNo].pLandHead;
  } else {
    pNode = gpWorldLevelData[sGridNo].pRoofHead;
  }

  if (pNode == NULL) {
    return (ubAmbientLightLevel);
  } else {
    iSum = pNode->ubNaturalShadeLevel - (pNode->ubSumLights - pNode->ubFakeShadeLevel);

    iSum = min(SHADE_MIN, iSum);
    iSum = max(SHADE_MAX, iSum);
    return ((uint8_t)iSum);
  }
}

/****************************************************************************************
        LightAddNodeTile

                Does the addition of light values to individual LEVELNODEs in the world tile list.

***************************************************************************************/
void LightAddTileNode(struct LEVELNODE *pNode, uint32_t uiLightType, uint8_t ubShadeAdd,
                      BOOLEAN fFake) {
  int16_t sSum;

  pNode->ubSumLights += ubShadeAdd;
  if (fFake) {
    pNode->ubFakeShadeLevel += ubShadeAdd;
  }

  // Now set max
  pNode->ubMaxLights = max(pNode->ubMaxLights, ubShadeAdd);

  sSum = pNode->ubNaturalShadeLevel - pNode->ubMaxLights;

  sSum = min(SHADE_MIN, sSum);
  sSum = max(SHADE_MAX, sSum);

  pNode->ubShadeLevel = (uint8_t)sSum;
}

/****************************************************************************************
        LightAddNodeTile

                Does the subtraction of light values to individual LEVELNODEs in the world tile
list.

***************************************************************************************/
void LightSubtractTileNode(struct LEVELNODE *pNode, uint32_t uiLightType, uint8_t ubShadeSubtract,
                           BOOLEAN fFake) {
  int16_t sSum;

  if (ubShadeSubtract > pNode->ubSumLights) {
    pNode->ubSumLights = 0;
  } else {
    pNode->ubSumLights -= ubShadeSubtract;
  }
  if (fFake) {
    if (ubShadeSubtract > pNode->ubFakeShadeLevel) {
      pNode->ubFakeShadeLevel = 0;
    } else {
      pNode->ubFakeShadeLevel -= ubShadeSubtract;
    }
  }

  // Now set max
  pNode->ubMaxLights = min(pNode->ubMaxLights, pNode->ubSumLights);

  sSum = pNode->ubNaturalShadeLevel - pNode->ubMaxLights;

  sSum = min(SHADE_MIN, sSum);
  sSum = max(SHADE_MAX, sSum);

  pNode->ubShadeLevel = (uint8_t)sSum;
}

/****************************************************************************************
        LightAddTile

                Adds a specified amount of light to all objects on a given tile.

***************************************************************************************/
BOOLEAN LightAddTile(uint32_t uiLightType, int16_t iSrcX, int16_t iSrcY, int16_t iX, int16_t iY,
                     uint8_t ubShade, uint32_t uiFlags, BOOLEAN fOnlyWalls) {
  struct LEVELNODE *pLand, *pStruct, *pObject, *pMerc, *pRoof, *pOnRoof;
  uint8_t ubShadeAdd;
  uint32_t uiTile;
  BOOLEAN fLitWall = FALSE;
  BOOLEAN fFake;

  Assert(gpWorldLevelData != NULL);

  uiTile = MAPROWCOLTOPOS(iY, iX);

  if (uiTile >= NOWHERE) {
    return (FALSE);
  }

  gpWorldLevelData[uiTile].uiFlags |= MAPELEMENT_REDRAW;

  // if((uiFlags&LIGHT_BACKLIGHT) && !(uiFlags&LIGHT_ROOF_ONLY))
  //	ubShadeAdd=ubShade*7/10;
  // else
  ubShadeAdd = ubShade;

  if (uiFlags & LIGHT_FAKE) {
    fFake = TRUE;
  } else {
    fFake = FALSE;
  }

  if (!(uiFlags & LIGHT_ROOF_ONLY) || (uiFlags & LIGHT_EVERYTHING)) {
    pStruct = gpWorldLevelData[uiTile].pStructHead;
    while (pStruct != NULL) {
      if (pStruct->usIndex < NUMBEROFTILES) {
        if ((gTileDatabase[pStruct->usIndex].fType != FIRSTCLIFFHANG) ||
            (uiFlags & LIGHT_EVERYTHING)) {
          if ((uiFlags & LIGHT_IGNORE_WALLS) || gfCaves)
            LightAddTileNode(pStruct, uiLightType, ubShadeAdd, FALSE);
          else if (LightIlluminateWall(iSrcX, iSrcY, iX, iY, pStruct)) {
            if (LightTileHasWall(iSrcX, iSrcY, iX, iY)) fLitWall = TRUE;

            // ATE: Limit shade for walls if in caves
            if (fLitWall && gfCaves) {
              LightAddTileNode(pStruct, uiLightType, (uint8_t)min(ubShadeAdd, (SHADE_MAX + 5)),
                               FALSE);
            } else if (fLitWall) {
              LightAddTileNode(pStruct, uiLightType, ubShadeAdd, FALSE);
            } else if (!fOnlyWalls) {
              LightAddTileNode(pStruct, uiLightType, ubShadeAdd, FALSE);
            }
          }
        }
      } else {
        LightAddTileNode(pStruct, uiLightType, ubShadeAdd, FALSE);
      }
      pStruct = pStruct->pNext;
    }

    ubShadeAdd = ubShade;

    if (!fOnlyWalls) {
      pLand = gpWorldLevelData[uiTile].pLandHead;

      while (pLand) {
        if (gfCaves || !fLitWall) {
          LightAddTileNode(pLand, uiLightType, ubShadeAdd, fFake);
        }
        pLand = pLand->pNext;
      }

      pObject = gpWorldLevelData[uiTile].pObjectHead;
      while (pObject != NULL) {
        if (pObject->usIndex < NUMBEROFTILES) {
          LightAddTileNode(pObject, uiLightType, ubShadeAdd, FALSE);
        }
        pObject = pObject->pNext;
      }

      if (uiFlags & LIGHT_BACKLIGHT) ubShadeAdd = (int16_t)ubShade * 7 / 10;

      pMerc = gpWorldLevelData[uiTile].pMercHead;
      while (pMerc != NULL) {
        LightAddTileNode(pMerc, uiLightType, ubShadeAdd, FALSE);
        pMerc = pMerc->pNext;
      }
    }
  }

  if ((uiFlags & LIGHT_ROOF_ONLY) || (uiFlags & LIGHT_EVERYTHING)) {
    pRoof = gpWorldLevelData[uiTile].pRoofHead;
    while (pRoof != NULL) {
      if (pRoof->usIndex < NUMBEROFTILES) {
        LightAddTileNode(pRoof, uiLightType, ubShadeAdd, fFake);
      }
      pRoof = pRoof->pNext;
    }

    pOnRoof = gpWorldLevelData[uiTile].pOnRoofHead;
    while (pOnRoof != NULL) {
      LightAddTileNode(pOnRoof, uiLightType, ubShadeAdd, FALSE);

      pOnRoof = pOnRoof->pNext;
    }
  }
  return (TRUE);
}

/****************************************************************************************
        LightSubtractTile

                Subtracts a specified amount of light to a given tile.

***************************************************************************************/
BOOLEAN LightSubtractTile(uint32_t uiLightType, int16_t iSrcX, int16_t iSrcY, int16_t iX,
                          int16_t iY, uint8_t ubShade, uint32_t uiFlags, BOOLEAN fOnlyWalls) {
  struct LEVELNODE *pLand, *pStruct, *pObject, *pMerc, *pRoof, *pOnRoof;
  uint8_t ubShadeSubtract;
  uint32_t uiTile;
  BOOLEAN fLitWall = FALSE;
  BOOLEAN fFake;  // only passed in to land and roof layers; others get fed FALSE

  Assert(gpWorldLevelData != NULL);

  uiTile = MAPROWCOLTOPOS(iY, iX);

  if (uiTile >= NOWHERE) {
    return (FALSE);
  }

  gpWorldLevelData[uiTile].uiFlags |= MAPELEMENT_REDRAW;

  //	if((uiFlags&LIGHT_BACKLIGHT) && !(uiFlags&LIGHT_ROOF_ONLY))
  //		ubShadeSubtract=ubShade*7/10;
  //	else
  ubShadeSubtract = ubShade;

  if (uiFlags & LIGHT_FAKE) {
    fFake = TRUE;
  } else {
    fFake = FALSE;
  }

  if (!(uiFlags & LIGHT_ROOF_ONLY) || (uiFlags & LIGHT_EVERYTHING)) {
    pStruct = gpWorldLevelData[uiTile].pStructHead;
    while (pStruct != NULL) {
      if (pStruct->usIndex < NUMBEROFTILES) {
        if ((gTileDatabase[pStruct->usIndex].fType != FIRSTCLIFFHANG) ||
            (uiFlags & LIGHT_EVERYTHING)) {
          if ((uiFlags & LIGHT_IGNORE_WALLS) || gfCaves)
            LightSubtractTileNode(pStruct, uiLightType, ubShadeSubtract, FALSE);
          else if (LightIlluminateWall(iSrcX, iSrcY, iX, iY, pStruct)) {
            if (LightTileHasWall(iSrcX, iSrcY, iX, iY)) fLitWall = TRUE;

            // ATE: Limit shade for walls if in caves
            if (fLitWall && gfCaves) {
              LightSubtractTileNode(pStruct, uiLightType, (uint8_t)max((ubShadeSubtract - 5), 0),
                                    FALSE);
            } else if (fLitWall) {
              LightSubtractTileNode(pStruct, uiLightType, ubShadeSubtract, FALSE);
            } else if (!fOnlyWalls) {
              LightSubtractTileNode(pStruct, uiLightType, ubShadeSubtract, FALSE);
            }
          }
        }
      } else {
        LightSubtractTileNode(pStruct, uiLightType, ubShadeSubtract, FALSE);
      }
      pStruct = pStruct->pNext;
    }

    ubShadeSubtract = ubShade;

    if (!fOnlyWalls) {
      pLand = gpWorldLevelData[uiTile].pLandHead;

      while (pLand) {
        if (gfCaves || !fLitWall) {
          LightSubtractTileNode(pLand, uiLightType, ubShadeSubtract, fFake);
        }
        pLand = pLand->pNext;
      }

      pObject = gpWorldLevelData[uiTile].pObjectHead;
      while (pObject != NULL) {
        if (pObject->usIndex < NUMBEROFTILES) {
          LightSubtractTileNode(pObject, uiLightType, ubShadeSubtract, FALSE);
        }
        pObject = pObject->pNext;
      }

      if (uiFlags & LIGHT_BACKLIGHT) ubShadeSubtract = (int16_t)ubShade * 7 / 10;

      pMerc = gpWorldLevelData[uiTile].pMercHead;
      while (pMerc != NULL) {
        LightSubtractTileNode(pMerc, uiLightType, ubShadeSubtract, FALSE);
        pMerc = pMerc->pNext;
      }
    }
  }

  if ((uiFlags & LIGHT_ROOF_ONLY) || (uiFlags & LIGHT_EVERYTHING)) {
    pRoof = gpWorldLevelData[uiTile].pRoofHead;
    while (pRoof != NULL) {
      if (pRoof->usIndex < NUMBEROFTILES) {
        LightSubtractTileNode(pRoof, uiLightType, ubShadeSubtract, fFake);
      }
      pRoof = pRoof->pNext;
    }

    pOnRoof = gpWorldLevelData[uiTile].pOnRoofHead;
    while (pOnRoof != NULL) {
      if (pOnRoof->usIndex < NUMBEROFTILES) {
        LightSubtractTileNode(pOnRoof, uiLightType, ubShadeSubtract, FALSE);
      }
      pOnRoof = pOnRoof->pNext;
    }
  }

  return (TRUE);
}

/****************************************************************************************
        LightSetNaturalTileNode

                Sets the natural light level (as well as the current) on individual LEVELNODEs.

***************************************************************************************/
void LightSetNaturalTileNode(struct LEVELNODE *pNode, uint8_t ubShade) {
  Assert(pNode != NULL);

  pNode->ubSumLights = 0;
  pNode->ubMaxLights = 0;
  pNode->ubNaturalShadeLevel = ubShade;
  pNode->ubShadeLevel = ubShade;
  // LightAddTileNode(pNode, 0, (int16_t)(SHADE_MIN-ubShade));
}

/****************************************************************************************
        LightSetNaturalTile

                Sets the natural light value of all objects on a given tile to the specified value.
        This is the light value a tile has with no artificial lighting affecting it.

***************************************************************************************/
BOOLEAN LightSetNaturalTile(int16_t iX, int16_t iY, uint8_t ubShade) {
  struct LEVELNODE *pLand, *pStruct, *pObject, *pRoof, *pOnRoof, *pTopmost, *pMerc;
  uint32_t uiIndex;

  if (!(gpWorldLevelData != NULL)) {
    return FALSE;
  }

  uiIndex = MAPROWCOLTOPOS(iY, iX);

  Assert(uiIndex != 0xffff);

  ubShade = max(SHADE_MAX, ubShade);
  ubShade = min(SHADE_MIN, ubShade);

  pLand = gpWorldLevelData[uiIndex].pLandHead;

  while (pLand != NULL) {
    LightSetNaturalTileNode(pLand, ubShade);
    pLand = pLand->pNext;
  }

  pStruct = gpWorldLevelData[uiIndex].pStructHead;

  while (pStruct != NULL) {
    LightSetNaturalTileNode(pStruct, ubShade);
    pStruct = pStruct->pNext;
  }

  pObject = gpWorldLevelData[uiIndex].pObjectHead;
  while (pObject != NULL) {
    LightSetNaturalTileNode(pObject, ubShade);
    pObject = pObject->pNext;
  }

  pRoof = gpWorldLevelData[uiIndex].pRoofHead;
  while (pRoof != NULL) {
    LightSetNaturalTileNode(pRoof, ubShade);
    pRoof = pRoof->pNext;
  }

  pOnRoof = gpWorldLevelData[uiIndex].pOnRoofHead;
  while (pOnRoof != NULL) {
    LightSetNaturalTileNode(pOnRoof, ubShade);
    pOnRoof = pOnRoof->pNext;
  }

  pTopmost = gpWorldLevelData[uiIndex].pTopmostHead;
  while (pTopmost != NULL) {
    LightSetNaturalTileNode(pTopmost, ubShade);
    pTopmost = pTopmost->pNext;
  }

  pMerc = gpWorldLevelData[uiIndex].pMercHead;
  while (pMerc != NULL) {
    LightSetNaturalTileNode(pMerc, ubShade);
    pMerc = pMerc->pNext;
  }
  return (TRUE);
}

/****************************************************************************************
        LightResetTileNode

        Resets the light level of individual LEVELNODEs to the value contained in the
        natural light level.

***************************************************************************************/
void LightResetTileNode(struct LEVELNODE *pNode) {
  pNode->ubSumLights = 0;
  pNode->ubMaxLights = 0;
  pNode->ubShadeLevel = pNode->ubNaturalShadeLevel;
  pNode->ubFakeShadeLevel = 0;
}

/****************************************************************************************
        LightResetTile

                Resets the light values of all objects on a given tile to the "natural" light level
        for that tile.

***************************************************************************************/
BOOLEAN LightResetTile(int16_t iX, int16_t iY) {
  struct LEVELNODE *pLand, *pStruct, *pObject, *pRoof, *pOnRoof, *pTopmost, *pMerc;
  uint32_t uiTile;

  if (!(gpWorldLevelData != NULL)) {
    return FALSE;
  }

  uiTile = MAPROWCOLTOPOS(iY, iX);

  if (!(uiTile != 0xffff)) {
    return FALSE;
  }

  pLand = gpWorldLevelData[uiTile].pLandHead;

  while (pLand != NULL) {
    LightResetTileNode(pLand);
    pLand = pLand->pNext;
  }

  pStruct = gpWorldLevelData[uiTile].pStructHead;

  while (pStruct != NULL) {
    LightResetTileNode(pStruct);
    pStruct = pStruct->pNext;
  }

  pObject = gpWorldLevelData[uiTile].pObjectHead;
  while (pObject != NULL) {
    LightResetTileNode(pObject);
    pObject = pObject->pNext;
  }

  pRoof = gpWorldLevelData[uiTile].pRoofHead;
  while (pRoof != NULL) {
    LightResetTileNode(pRoof);
    pRoof = pRoof->pNext;
  }

  pOnRoof = gpWorldLevelData[uiTile].pOnRoofHead;
  while (pOnRoof != NULL) {
    LightResetTileNode(pOnRoof);
    pOnRoof = pOnRoof->pNext;
  }

  pTopmost = gpWorldLevelData[uiTile].pTopmostHead;
  while (pTopmost != NULL) {
    LightResetTileNode(pTopmost);
    pTopmost = pTopmost->pNext;
  }

  pMerc = gpWorldLevelData[uiTile].pMercHead;
  while (pMerc != NULL) {
    LightResetTileNode(pMerc);
    pMerc = pMerc->pNext;
  }

  return (TRUE);
}

/****************************************************************************************
        LightResetAllTiles

                Resets all tiles on the map to their baseline values.

***************************************************************************************/
BOOLEAN LightResetAllTiles(void) {
  int16_t iCountY, iCountX;

  for (iCountY = 0; iCountY < WORLD_ROWS; iCountY++)
    for (iCountX = 0; iCountX < WORLD_COLS; iCountX++) LightResetTile(iCountX, iCountY);

  return (TRUE);
}

/****************************************************************************************
        LightSetTile

                Sets the current light value of all objects on a given tile to the specified value.

***************************************************************************************/
BOOLEAN LightSetTile(int16_t iX, int16_t iY, uint8_t ubShade, uint32_t uiLightType) {
  /*struct LEVELNODE *pLand, *pStruct, *pObject, *pRoof, *pOnRoof, *pTopmost, *pFog;
  uint32_t uiIndex;

          ubShade=max(SHADE_MAX, ubShade);
          ubShade=min(SHADE_MIN, ubShade);

          uiIndex = MAPROWCOLTOPOS( iY, iX );
          pLand = gpWorldLevelData[ uiIndex ].pLandHead;

          while(pLand!=NULL)
          {
                  pLand->sSumLights[uiLightType]= ubShade-SHADE_MAX;
                  pLand->ubShadeLevel = ubShade;
                  pLand=pLand->pNext;
          }

          pStruct = gpWorldLevelData[ uiIndex ].pStructHead;

          while(pStruct!=NULL)
          {
                  pStruct->sSumLights[uiLightType]= ubShade-SHADE_MAX;
                  pStruct->ubShadeLevel = ubShade;
                  pStruct=pStruct->pNext;
          }

          pObject = gpWorldLevelData[ uiIndex ].pObjectHead;
          while(pObject!=NULL)
          {
                  pObject->sSumLights[uiLightType]= ubShade-SHADE_MAX;
                  pObject->ubShadeLevel = ubShade;
                  pObject=pObject->pNext;
          }

          pRoof = gpWorldLevelData[ uiIndex ].pRoofHead;
          while(pRoof!=NULL)
          {
                  pRoof->sSumLights[uiLightType]= ubShade-SHADE_MAX;
                  pRoof->ubShadeLevel = ubShade;
                  pRoof=pRoof->pNext;
          }

          pOnRoof = gpWorldLevelData[ uiIndex ].pOnRoofHead;
          while(pOnRoof!=NULL)
          {
                  pOnRoof->sSumLights[uiLightType]= ubShade-SHADE_MAX;
                  pOnRoof->ubShadeLevel = ubShade;
                  pOnRoof=pOnRoof->pNext;
          }

          pFog = gpWorldLevelData[ uiIndex ].pFogHead;
          while(pFog!=NULL)
          {
                  pFog->sSumLights[uiLightType]= ubShade-SHADE_MAX;
                  pFog->ubShadeLevel = ubShade;
                  pFog=pFog->pNext;
          }

          pTopmost = gpWorldLevelData[ uiIndex ].pTopmostHead;
          while(pTopmost!=NULL)
          {
                  pTopmost->sSumLights[uiLightType]= ubShade-SHADE_MAX;
                  pTopmost->ubShadeLevel = ubShade;
                  pTopmost=pTopmost->pNext;
          } */
  return (TRUE);
}

/****************************************************************************************
        LightGetLastNode

                Returns a pointer to the last node in a light list. If the list is empty, NULL is
        returned.

***************************************************************************************/
uint16_t LightGetLastNode(int32_t iLight) { return (usRaySize[iLight]); }

/****************************************************************************************
        LightAddNode

                Creates a new node, and adds it to the end of a light list.

***************************************************************************************/
BOOLEAN LightAddNode(int32_t iLight, int16_t iHotSpotX, int16_t iHotSpotY, int16_t iX, int16_t iY,
                     uint8_t ubIntensity, uint16_t uiFlags) {
  double dDistance;
  uint8_t ubShade;
  int32_t iLightDecay;

  dDistance = LinearDistanceDouble(iX, iY, iHotSpotX, iHotSpotY);
  dDistance /= DISTANCE_SCALE;

  iLightDecay = (int32_t)(dDistance * LIGHT_DECAY);

  if ((iLightDecay >= (int32_t)ubIntensity))
    ubShade = 0;
  else
    ubShade = ubIntensity - (uint8_t)iLightDecay;

  iX /= DISTANCE_SCALE;
  iY /= DISTANCE_SCALE;

  LightAddRayNode(iLight, iX, iY, ubShade, uiFlags);
  return (TRUE);
}

/****************************************************************************************
        LightInsertNode

                Creates a new node, and inserts it after the specified node.

***************************************************************************************/
BOOLEAN LightInsertNode(int32_t iLight, uint16_t usLightIns, int16_t iHotSpotX, int16_t iHotSpotY,
                        int16_t iX, int16_t iY, uint8_t ubIntensity, uint16_t uiFlags) {
  double dDistance;
  uint8_t ubShade;
  int32_t iLightDecay;

  dDistance = LinearDistanceDouble(iX, iY, iHotSpotX, iHotSpotY);
  dDistance /= DISTANCE_SCALE;

  iLightDecay = (int32_t)(dDistance * LIGHT_DECAY);

  if ((iLightDecay >= (int32_t)ubIntensity))
    ubShade = 0;
  else
    ubShade = ubIntensity - (uint8_t)iLightDecay;

  iX /= DISTANCE_SCALE;
  iY /= DISTANCE_SCALE;

  LightInsertRayNode(iLight, usLightIns, iX, iY, ubShade, uiFlags);

  return (TRUE);
}

/****************************************************************************************
        LightFindNextRay

                Traverses the linked list until a node with the LIGHT_NEW_RAY marker, and returns
        the pointer. If the end of list is reached, NULL is returned.

***************************************************************************************/
uint16_t LightFindNextRay(int32_t iLight, uint16_t usIndex) {
  uint16_t usNodeIndex;

  usNodeIndex = usIndex;
  while ((usNodeIndex < usRaySize[iLight]) &&
         !(*(pLightRayList[iLight] + usNodeIndex) & LIGHT_NEW_RAY))
    usNodeIndex++;

  return (usNodeIndex);
}

/****************************************************************************************
        LightCastRay

                Casts a ray from an origin to an end point, creating nodes and adding them to the
        light list.

***************************************************************************************/
BOOLEAN LightCastRay(int32_t iLight, int16_t iStartX, int16_t iStartY, int16_t iEndPointX,
                     int16_t iEndPointY, uint8_t ubStartIntens, uint8_t ubEndIntens) {
  int16_t AdjUp, AdjDown, ErrorTerm, XAdvance, XDelta, YDelta;
  int32_t WholeStep, InitialPixelCount, FinalPixelCount, i, j, RunLength;
  int16_t iXPos, iYPos, iEndY, iEndX;
  uint16_t usCurNode = 0, usFlags = 0;
  BOOLEAN fInsertNodes = FALSE;

  if ((iEndPointX > 0) && (iEndPointY > 0)) usFlags = LIGHT_BACKLIGHT;

  /* We'll always draw top to bottom, to reduce the number of cases we have to
  handle, and to make lines between the same endpoints draw the same pixels */
  if (iStartY > iEndPointY) {
    iXPos = iEndPointX;
    iEndX = iStartX;
    iYPos = iEndPointY;
    iEndY = iStartY;
    fInsertNodes = TRUE;
  } else {
    iXPos = iStartX;
    iEndX = iEndPointX;
    iYPos = iStartY;
    iEndY = iEndPointY;
  }

  /* Figure out whether we're going left or right, and how far we're
going horizontally */
  if ((XDelta = (iEndX - iXPos)) < 0) {
    XAdvance = -1;
    XDelta = -XDelta;
  } else {
    XAdvance = 1;
  }
  /* Figure out how far we're going vertically */
  YDelta = iEndY - iYPos;

  // Check for 0 length ray
  if ((XDelta == 0) && (YDelta == 0)) return (FALSE);

  // DebugMsg(TOPIC_GAME, DBG_ERROR, String("Drawing (%d,%d) to (%d,%d)", iXPos, iYPos, iEndX,
  // iEndY));
  LightAddNode(iLight, 32767, 32767, 32767, 32767, 0, LIGHT_NEW_RAY);
  if (fInsertNodes) usCurNode = LightGetLastNode(iLight);

  /* Special-case horizontal, vertical, and diagonal lines, for speed
and to avoid nasty boundary conditions and division by 0 */
  if (XDelta == 0) {
    /* Vertical line */
    if (fInsertNodes) {
      for (i = 0; i <= YDelta; i++) {
        LightInsertNode(iLight, usCurNode, iStartX, iStartY, iXPos, iYPos, ubStartIntens, usFlags);
        iYPos++;
      }
    } else {
      for (i = 0; i <= YDelta; i++) {
        LightAddNode(iLight, iStartX, iStartY, iXPos, iYPos, ubStartIntens, usFlags);
        iYPos++;
      }
    }
    return (TRUE);
  }
  if (YDelta == 0) {
    /* Horizontal line */
    if (fInsertNodes) {
      for (i = 0; i <= XDelta; i++) {
        LightInsertNode(iLight, usCurNode, iStartX, iStartY, iXPos, iYPos, ubStartIntens, usFlags);
        iXPos += XAdvance;
      }
    } else {
      for (i = 0; i <= XDelta; i++) {
        LightAddNode(iLight, iStartX, iStartY, iXPos, iYPos, ubStartIntens, usFlags);
        iXPos += XAdvance;
      }
    }
    return (TRUE);
  }
  if (XDelta == YDelta) {
    /* Diagonal line */
    if (fInsertNodes) {
      for (i = 0; i <= XDelta; i++) {
        LightInsertNode(iLight, usCurNode, iStartX, iStartY, iXPos, iYPos, ubStartIntens, usFlags);
        iXPos += XAdvance;
        iYPos++;
      }
    } else {
      for (i = 0; i <= XDelta; i++) {
        LightAddNode(iLight, iStartX, iStartY, iXPos, iYPos, ubStartIntens, usFlags);
        iXPos += XAdvance;
        iYPos++;
      }
    }
    return (TRUE);
  }

  /* Determine whether the line is X or Y major, and handle accordingly */
  if (XDelta >= YDelta) {
    /* X major line */
    /* Minimum # of pixels in a run in this line */
    WholeStep = XDelta / YDelta;

    /* Error term adjust each time Y steps by 1; used to tell when one
       extra pixel should be drawn as part of a run, to account for
       fractional steps along the X axis per 1-pixel steps along Y */
    AdjUp = (XDelta % YDelta) * 2;

    /* Error term adjust when the error term turns over, used to factor
       out the X step made at that time */
    AdjDown = YDelta * 2;

    /* Initial error term; reflects an initial step of 0.5 along the Y
       axis */
    ErrorTerm = (XDelta % YDelta) - (YDelta * 2);

    /* The initial and last runs are partial, because Y advances only 0.5
       for these runs, rather than 1. Divide one full run, plus the
       initial pixel, between the initial and last runs */
    InitialPixelCount = (WholeStep / 2) + 1;
    FinalPixelCount = InitialPixelCount;

    /* If the basic run length is even and there's no fractional
       advance, we have one pixel that could go to either the initial
       or last partial run, which we'll arbitrarily allocate to the
       last run */
    if ((AdjUp == 0) && ((WholeStep & 0x01) == 0)) {
      InitialPixelCount--;
    }
    /* If there're an odd number of pixels per run, we have 1 pixel that can't
       be allocated to either the initial or last partial run, so we'll add 0.5
       to error term so this pixel will be handled by the normal full-run loop */
    if ((WholeStep & 0x01) != 0) {
      ErrorTerm += YDelta;
    }
    /* Draw the first, partial run of pixels */
    // DrawHorizontalRun(&ScreenPtr, XAdvance, InitialPixelCount, Color);
    if (fInsertNodes) {
      for (i = 0; i < InitialPixelCount; i++) {
        LightInsertNode(iLight, usCurNode, iStartX, iStartY, iXPos, iYPos, ubStartIntens, usFlags);
        iXPos += XAdvance;
      }
    } else {
      for (i = 0; i < InitialPixelCount; i++) {
        LightAddNode(iLight, iStartX, iStartY, iXPos, iYPos, ubStartIntens, usFlags);
        iXPos += XAdvance;
      }
    }
    iYPos++;

    /* Draw all full runs */
    for (j = 0; j < (YDelta - 1); j++) {
      RunLength = WholeStep; /* run is at least this long */
      /* Advance the error term and add an extra pixel if the error
         term so indicates */
      if ((ErrorTerm += AdjUp) > 0) {
        RunLength++;
        ErrorTerm -= AdjDown; /* reset the error term */
      }
      /* Draw this scan line's run */
      // DrawHorizontalRun(&ScreenPtr, XAdvance, RunLength, Color);
      if (fInsertNodes) {
        for (i = 0; i < RunLength; i++) {
          LightInsertNode(iLight, usCurNode, iStartX, iStartY, iXPos, iYPos, ubStartIntens,
                          usFlags);
          iXPos += XAdvance;
        }
      } else {
        for (i = 0; i < RunLength; i++) {
          LightAddNode(iLight, iStartX, iStartY, iXPos, iYPos, ubStartIntens, usFlags);
          iXPos += XAdvance;
        }
      }
      iYPos++;
    }
    /* Draw the final run of pixels */
    // DrawHorizontalRun(&ScreenPtr, XAdvance, FinalPixelCount, Color);
    if (fInsertNodes) {
      for (i = 0; i < FinalPixelCount; i++) {
        LightInsertNode(iLight, usCurNode, iStartX, iStartY, iXPos, iYPos, ubStartIntens, usFlags);
        iXPos += XAdvance;
      }
    } else {
      for (i = 0; i < FinalPixelCount; i++) {
        LightAddNode(iLight, iStartX, iStartY, iXPos, iYPos, ubStartIntens, usFlags);
        iXPos += XAdvance;
      }
    }
    iYPos++;
  } else {
    /* Y major line */

    /* Minimum # of pixels in a run in this line */
    WholeStep = YDelta / XDelta;

    /* Error term adjust each time X steps by 1; used to tell when 1 extra
       pixel should be drawn as part of a run, to account for
       fractional steps along the Y axis per 1-pixel steps along X */
    AdjUp = (YDelta % XDelta) * 2;

    /* Error term adjust when the error term turns over, used to factor
       out the Y step made at that time */
    AdjDown = XDelta * 2;

    /* Initial error term; reflects initial step of 0.5 along the X axis */
    ErrorTerm = (YDelta % XDelta) - (XDelta * 2);

    /* The initial and last runs are partial, because X advances only 0.5
       for these runs, rather than 1. Divide one full run, plus the
       initial pixel, between the initial and last runs */
    InitialPixelCount = (WholeStep / 2) + 1;
    FinalPixelCount = InitialPixelCount;

    /* If the basic run length is even and there's no fractional advance, we
       have 1 pixel that could go to either the initial or last partial run,
       which we'll arbitrarily allocate to the last run */
    if ((AdjUp == 0) && ((WholeStep & 0x01) == 0)) {
      InitialPixelCount--;
    }
    /* If there are an odd number of pixels per run, we have one pixel
       that can't be allocated to either the initial or last partial
       run, so we'll add 0.5 to the error term so this pixel will be
       handled by the normal full-run loop */
    if ((WholeStep & 0x01) != 0) {
      ErrorTerm += XDelta;
    }
    /* Draw the first, partial run of pixels */
    if (fInsertNodes) {
      for (i = 0; i < InitialPixelCount; i++) {
        LightInsertNode(iLight, usCurNode, iStartX, iStartY, iXPos, iYPos, ubStartIntens, usFlags);
        iYPos++;
      }
    } else {
      for (i = 0; i < InitialPixelCount; i++) {
        LightAddNode(iLight, iStartX, iStartY, iXPos, iYPos, ubStartIntens, usFlags);
        iYPos++;
      }
    }
    iXPos += XAdvance;
    // DrawVerticalRun(&ScreenPtr, XAdvance, InitialPixelCount, Color);

    /* Draw all full runs */
    for (j = 0; j < (XDelta - 1); j++) {
      RunLength = WholeStep; /* run is at least this long */
      /* Advance the error term and add an extra pixel if the error
         term so indicates */
      if ((ErrorTerm += AdjUp) > 0) {
        RunLength++;
        ErrorTerm -= AdjDown; /* reset the error term */
      }
      /* Draw this scan line's run */
      // DrawVerticalRun(&ScreenPtr, XAdvance, RunLength, Color);
      if (fInsertNodes) {
        for (i = 0; i < RunLength; i++) {
          LightInsertNode(iLight, usCurNode, iStartX, iStartY, iXPos, iYPos, ubStartIntens,
                          usFlags);
          iYPos++;
        }
      } else {
        for (i = 0; i < RunLength; i++) {
          LightAddNode(iLight, iStartX, iStartY, iXPos, iYPos, ubStartIntens, usFlags);
          iYPos++;
        }
      }
      iXPos += XAdvance;
    }
    /* Draw the final run of pixels */
    // DrawVerticalRun(&ScreenPtr, XAdvance, FinalPixelCount, Color);
    if (fInsertNodes) {
      for (i = 0; i < FinalPixelCount; i++) {
        LightInsertNode(iLight, usCurNode, iStartX, iStartY, iXPos, iYPos, ubStartIntens, usFlags);
        iYPos++;
      }
    } else {
      for (i = 0; i < FinalPixelCount; i++) {
        LightAddNode(iLight, iStartX, iStartY, iXPos, iYPos, ubStartIntens, usFlags);
        iYPos++;
      }
    }
    iXPos += XAdvance;
  }
  return (TRUE);
}

/****************************************************************************************
        LightGenerateElliptical

                Creates an elliptical light, taking two radii.

***************************************************************************************/
BOOLEAN LightGenerateElliptical(int32_t iLight, uint8_t iIntensity, int16_t iA, int16_t iB) {
  int16_t iX, iY;
  int32_t WorkingX, WorkingY;
  double ASquared;
  double BSquared;
  double Temp;

  iX = 0;
  iY = 0;
  ASquared = (double)iA * iA;
  BSquared = (double)iB * iB;

  /* Draw the four symmetric arcs for which X advances faster (that is,
     for which X is the major axis) */
  /* Draw the initial top & bottom points */
  LightCastRay(iLight, iX, iY, (int16_t)iX, (int16_t)(iY + iB), iIntensity, 1);
  LightCastRay(iLight, iX, iY, (int16_t)iX, (int16_t)(iY - iB), iIntensity, 1);

  /* Draw the four arcs */
  for (WorkingX = 0;;) {
    /* Advance one pixel along the X axis */
    WorkingX++;

    /* Calculate the corresponding point along the Y axis. Guard
       against floating-point roundoff making the intermediate term
       less than 0 */
    Temp = BSquared - (BSquared * WorkingX * WorkingX / ASquared);

    if (Temp >= 0)
      WorkingY = (int32_t)(sqrt(Temp) + 0.5);
    else
      WorkingY = 0;

    /* Stop if X is no longer the major axis (the arc has passed the
       45-degree point) */
    if (((double)WorkingY / BSquared) <= ((double)WorkingX / ASquared)) break;

    /* Draw the 4 symmetries of the current point */
    LightCastRay(iLight, iX, iY, (int16_t)(iX + WorkingX), (int16_t)(iY - WorkingY), iIntensity, 1);
    LightCastRay(iLight, iX, iY, (int16_t)(iX - WorkingX), (int16_t)(iY - WorkingY), iIntensity, 1);
    LightCastRay(iLight, iX, iY, (int16_t)(iX + WorkingX), (int16_t)(iY + WorkingY), iIntensity, 1);
    LightCastRay(iLight, iX, iY, (int16_t)(iX - WorkingX), (int16_t)(iY + WorkingY), iIntensity, 1);
  }

  /* Draw the four symmetric arcs for which Y advances faster (that is,
     for which Y is the major axis) */
  /* Draw the initial left & right points */
  LightCastRay(iLight, iX, iY, (int16_t)(iX + iA), iY, iIntensity, 1);
  LightCastRay(iLight, iX, iY, (int16_t)(iX - iA), iY, iIntensity, 1);

  /* Draw the four arcs */
  for (WorkingY = 0;;) {
    /* Advance one pixel along the Y axis */
    WorkingY++;

    /* Calculate the corresponding point along the X axis. Guard
       against floating-point roundoff making the intermediate term
       less than 0 */
    Temp = ASquared - (ASquared * WorkingY * WorkingY / BSquared);

    if (Temp >= 0)
      WorkingX = (int32_t)(sqrt(Temp) + 0.5);
    else
      WorkingX = 0;

    /* Stop if Y is no longer the major axis (the arc has passed the
       45-degree point) */
    if (((double)WorkingX / ASquared) < ((double)WorkingY / BSquared)) break;

    /* Draw the 4 symmetries of the current point */
    LightCastRay(iLight, iX, iY, (int16_t)(iX + WorkingX), (int16_t)(iY - WorkingY), iIntensity, 1);
    LightCastRay(iLight, iX, iY, (int16_t)(iX - WorkingX), (int16_t)(iY - WorkingY), iIntensity, 1);
    LightCastRay(iLight, iX, iY, (int16_t)(iX + WorkingX), (int16_t)(iY + WorkingY), iIntensity, 1);
    LightCastRay(iLight, iX, iY, (int16_t)(iX - WorkingX), (int16_t)(iY + WorkingY), iIntensity, 1);
  }

  return (TRUE);
}

/****************************************************************************************
        LightGenerateSquare

                Creates an square light, taking two radii.

***************************************************************************************/
BOOLEAN LightGenerateSquare(int32_t iLight, uint8_t iIntensity, int16_t iA, int16_t iB) {
  int16_t iX, iY;

  for (iX = 0 - iA; iX <= 0 + iA; iX++)
    LightCastRay(iLight, 0, 0, iX, (int16_t)(0 - iB), iIntensity, 1);

  for (iX = 0 - iA; iX <= 0 + iA; iX++)
    LightCastRay(iLight, 0, 0, iX, (int16_t)(0 + iB), iIntensity, 1);

  for (iY = 0 - iB; iY <= 0 + iB; iY++)
    LightCastRay(iLight, 0, 0, (int16_t)(0 - iA), iY, iIntensity, 1);

  for (iY = 0 - iB; iY <= 0 + iB; iY++)
    LightCastRay(iLight, 0, 0, (int16_t)(0 + iA), iY, iIntensity, 1);

  /*for(iY=0-iB; iY <= 0+iB; iY++)
          LightCastRay(iLight, 0, iY, (int16_t)(0+iA), iY, iIntensity, 1);

  for(iY=0+iB; iY >= 0-iB; iY--)
          LightCastRay(iLight, 0, iY, (int16_t)(0-iA), iY, iIntensity, 1);

  for(iX=0-iA; iX <= 0+iA; iX++)
          LightCastRay(iLight, iX, 0, iX, (int16_t)(0+iB), iIntensity, 1);

  for(iX=0+iA; iX >= 0-iA; iX--)
          LightCastRay(iLight, iX, 0, iX, (int16_t)(0-iB), iIntensity, 1); */

  return (TRUE);
}

/****************************************************************************************
        LightGenerateBeam

                Creates a directional light.

***************************************************************************************/
BOOLEAN LightGenerateBeam(int32_t iLight, uint8_t iIntensity, int16_t iLength, int16_t iRadius,
                          int16_t iDirection) {
  return (FALSE);
}

/****************************************************************************************
        LightSetBaseLevel

                Sets the current and natural light settings for all tiles in the world.

***************************************************************************************/
BOOLEAN LightSetBaseLevel(uint8_t iIntensity) {
  int16_t iCountY, iCountX;
  uint32_t cnt;
  struct SOLDIERTYPE *pSoldier;

  ubAmbientLightLevel = iIntensity;

  if (!gfEditMode) {
    // Loop for all good guys in tactical map and add a light if required
    for (cnt = 0; cnt < guiNumMercSlots; cnt++) {
      pSoldier = MercSlots[cnt];

      if (pSoldier != NULL) {
        if (pSoldier->bTeam == gbPlayerNum) {
          // Re-create soldier lights
          ReCreateSoldierLight(pSoldier);
        }
      }
    }
  }

  for (iCountY = 0; iCountY < WORLD_ROWS; iCountY++)
    for (iCountX = 0; iCountX < WORLD_COLS; iCountX++)
      LightSetNaturalTile(iCountX, iCountY, iIntensity);

  LightSpriteRenderAll();

  if (iIntensity >= LIGHT_DUSK_CUTOFF)
    RenderSetShadows(FALSE);
  else
    RenderSetShadows(TRUE);

  return (TRUE);
}

/****************************************************************************************
        LightAddBaseLevel

                Adds a light value to all the tiles in the world

***************************************************************************************/
BOOLEAN LightAddBaseLevel(uint32_t uiLightType, uint8_t iIntensity) {
  int16_t iCountY, iCountX;

  ubAmbientLightLevel = max(SHADE_MAX, ubAmbientLightLevel - iIntensity);

  for (iCountY = 0; iCountY < WORLD_ROWS; iCountY++)
    for (iCountX = 0; iCountX < WORLD_COLS; iCountX++)
      LightAddTile(uiLightType, iCountX, iCountY, iCountX, iCountY, iIntensity,
                   LIGHT_IGNORE_WALLS | LIGHT_EVERYTHING, FALSE);

  if (ubAmbientLightLevel >= LIGHT_DUSK_CUTOFF)
    RenderSetShadows(FALSE);
  else
    RenderSetShadows(TRUE);

  return (TRUE);
}

/****************************************************************************************
        LightSubtractBaseLevel

                Sets the current and natural light settings for all tiles in the world.

***************************************************************************************/
BOOLEAN LightSubtractBaseLevel(uint32_t uiLightType, uint8_t iIntensity) {
  int16_t iCountY, iCountX;

  ubAmbientLightLevel = min(SHADE_MIN, ubAmbientLightLevel + iIntensity);

  for (iCountY = 0; iCountY < WORLD_ROWS; iCountY++)
    for (iCountX = 0; iCountX < WORLD_COLS; iCountX++)
      LightSubtractTile(uiLightType, iCountX, iCountY, iCountX, iCountY, iIntensity,
                        LIGHT_IGNORE_WALLS | LIGHT_EVERYTHING, FALSE);

  if (ubAmbientLightLevel >= LIGHT_DUSK_CUTOFF)
    RenderSetShadows(FALSE);
  else
    RenderSetShadows(TRUE);

  return (TRUE);
}

/****************************************************************************************
        LightCreateOmni

                Creates a circular light.

***************************************************************************************/
int32_t LightCreateOmni(uint8_t ubIntensity, int16_t iRadius) {
  int32_t iLight;
  char usName[14];

  iLight = LightGetFree();
  if (iLight != (-1)) {
    LightGenerateElliptical(iLight, ubIntensity, (int16_t)(iRadius * DISTANCE_SCALE),
                            (int16_t)(iRadius * DISTANCE_SCALE));
  }

  sprintf(usName, "LTO%d.LHT", iRadius);
  pLightNames[iLight] = (char *)MemAlloc(strlen(usName) + 1);
  strcpy(pLightNames[iLight], usName);

  return (iLight);
}

/****************************************************************************************
        LightCreateSquare

                Creates a square light.

***************************************************************************************/
int32_t LightCreateSquare(uint8_t ubIntensity, int16_t iRadius1, int16_t iRadius2) {
  int32_t iLight;
  char usName[14];

  iLight = LightGetFree();
  if (iLight != (-1)) {
    LightGenerateSquare(iLight, ubIntensity, (int16_t)(iRadius1 * DISTANCE_SCALE),
                        (int16_t)(iRadius2 * DISTANCE_SCALE));
  }

  sprintf(usName, "LTS%d-%d.LHT", iRadius1, iRadius2);
  pLightNames[iLight] = (char *)MemAlloc(strlen(usName) + 1);
  strcpy(pLightNames[iLight], usName);

  return (iLight);
}

/****************************************************************************************
        LightCreateOval

                Creates an elliptical light.

***************************************************************************************/
int32_t LightCreateElliptical(uint8_t ubIntensity, int16_t iRadius1, int16_t iRadius2) {
  int32_t iLight;
  char usName[14];

  iLight = LightGetFree();
  if (iLight != (-1))
    LightGenerateElliptical(iLight, ubIntensity, (int16_t)(iRadius1 * DISTANCE_SCALE),
                            (int16_t)(iRadius2 * DISTANCE_SCALE));

  sprintf(usName, "LTE%d-%d.LHT", iRadius1, iRadius2);
  pLightNames[iLight] = (char *)MemAlloc(strlen(usName) + 1);
  strcpy(pLightNames[iLight], usName);

  return (iLight);
}

/****************************************************************************************
        LightIlluminateWall

                Renders a light template at the specified X,Y coordinates.

***************************************************************************************/
BOOLEAN LightIlluminateWall(int16_t iSourceX, int16_t iSourceY, int16_t iTileX, int16_t iTileY,
                            struct LEVELNODE *pStruct) {
  //	return( LightTileHasWall( iSourceX, iSourceY, iTileX, iTileY ) );

#if 0
uint16_t usWallOrientation;

	GetWallOrientation(pStruct->usIndex, &usWallOrientation);

	switch(usWallOrientation)
	{
		case NO_ORIENTATION:
			return(TRUE);

		case INSIDE_TOP_RIGHT:
		case OUTSIDE_TOP_RIGHT:
			return(iSourceX >= iTileX);

		case INSIDE_TOP_LEFT:
		case OUTSIDE_TOP_LEFT:
			return(iSourceY >= iTileY);

	}
	return(FALSE);

#endif

  return (TRUE);
}

/****************************************************************************************
        LightDraw

                Renders a light template at the specified X,Y coordinates.

***************************************************************************************/
BOOLEAN LightDraw(uint32_t uiLightType, int32_t iLight, int16_t iX, int16_t iY, uint32_t uiSprite) {
  LIGHT_NODE *pLight;
  uint16_t uiCount;
  uint16_t usNodeIndex;
  uint32_t uiFlags;
  int32_t iOldX, iOldY;
  BOOLEAN fBlocked = FALSE;
  BOOLEAN fOnlyWalls;

  // MAP_ELEMENT * pMapElement;

  if (pLightList[iLight] == NULL) return (FALSE);

  // clear out all the flags
  for (uiCount = 0; uiCount < usTemplateSize[iLight]; uiCount++) {
    pLight = pLightList[iLight] + uiCount;
    pLight->uiFlags &= (~LIGHT_NODE_DRAWN);
  }

  /*
          if (!(LightSprites[uiSprite].uiFlags & MERC_LIGHT))
          {
                  uiFlags |= LIGHT_FAKE

                  pMapElement = &(gpWorldLevelData[]);
                  if (pMapElement->pLand != NULL)
                  {
                          // only do this for visible locations
                          // adjust tile's recorded light level
                          pMapElement->sSumRealLight1+=sShadeAdd;

                          sSum=pMapElement->pLand->ubNaturalShadeLevel - max(
     pMapElement->sSumRealLights[0], pMapElement->pLand->sSumLights[1]);

                          sSum=min(SHADE_MIN, sSum);
                          sSum=max(SHADE_MAX, sSum);

                          gpWorldLevelData[ ].ubRealShadeLevel = (uint8_t) sSum;

                  }

          }
  */

  iOldX = iX;
  iOldY = iY;

  for (uiCount = 0; uiCount < usRaySize[iLight]; uiCount++) {
    usNodeIndex = *(pLightRayList[iLight] + uiCount);

    if (!(usNodeIndex & LIGHT_NEW_RAY)) {
      fBlocked = FALSE;
      fOnlyWalls = FALSE;

      pLight = pLightList[iLight] + (usNodeIndex & (~LIGHT_BACKLIGHT));

      if (!(LightSprites[uiSprite].uiFlags & LIGHT_SPR_ONROOF)) {
        if (LightTileBlocked((int16_t)iOldX, (int16_t)iOldY, (int16_t)(iX + pLight->iDX),
                             (int16_t)(iY + pLight->iDY))) {
          uiCount = LightFindNextRay(iLight, uiCount);

          fOnlyWalls = TRUE;
          fBlocked = TRUE;
        }
      }

      if (!(pLight->uiFlags & LIGHT_NODE_DRAWN) && (pLight->ubLight)) {
        uiFlags = (uint32_t)(usNodeIndex & LIGHT_BACKLIGHT);
        if (LightSprites[uiSprite].uiFlags & MERC_LIGHT) uiFlags |= LIGHT_FAKE;
        if (LightSprites[uiSprite].uiFlags & LIGHT_SPR_ONROOF) uiFlags |= LIGHT_ROOF_ONLY;

        LightAddTile(uiLightType, (int16_t)iOldX, (int16_t)iOldY, (int16_t)(iX + pLight->iDX),
                     (int16_t)(iY + pLight->iDY), pLight->ubLight, uiFlags, fOnlyWalls);

        pLight->uiFlags |= LIGHT_NODE_DRAWN;
      }

      if (fBlocked) {
        iOldX = iX;
        iOldY = iY;
      } else {
        iOldX = iX + pLight->iDX;
        iOldY = iY + pLight->iDY;
      }

    } else {
      iOldX = iX;
      iOldY = iY;
    }
  }

  return (TRUE);
}

BOOLEAN LightRevealWall(int16_t sX, int16_t sY, int16_t sSrcX, int16_t sSrcY) {
  struct LEVELNODE *pStruct;
  uint32_t uiTile;
  BOOLEAN fRerender = FALSE, fHitWall = FALSE, fDoRightWalls = TRUE, fDoLeftWalls = TRUE;
  TILE_ELEMENT *TileElem;

  Assert(gpWorldLevelData != NULL);

  uiTile = MAPROWCOLTOPOS(sY, sX);

  if (sX < sSrcX) fDoRightWalls = FALSE;

  if (sY < sSrcY) fDoLeftWalls = FALSE;

  // IF A FENCE, RETURN FALSE
  if (IsFencePresentAtGridno((int16_t)uiTile)) {
    return (FALSE);
  }

  pStruct = gpWorldLevelData[uiTile].pStructHead;
  while (pStruct != NULL) {
    TileElem = &(gTileDatabase[pStruct->usIndex]);
    switch (TileElem->usWallOrientation) {
      case INSIDE_TOP_RIGHT:
      case OUTSIDE_TOP_RIGHT:
        if (!fDoRightWalls) fDoLeftWalls = FALSE;
        break;

      case INSIDE_TOP_LEFT:
      case OUTSIDE_TOP_LEFT:
        if (!fDoLeftWalls) fDoRightWalls = FALSE;
        break;
    }
    pStruct = pStruct->pNext;
  }

  pStruct = gpWorldLevelData[uiTile].pStructHead;
  while (pStruct != NULL) {
    TileElem = &(gTileDatabase[pStruct->usIndex]);
    switch (TileElem->usWallOrientation) {
      case NO_ORIENTATION:
        break;

      case INSIDE_TOP_RIGHT:
      case OUTSIDE_TOP_RIGHT:
        fHitWall = TRUE;
        if ((fDoRightWalls) && (sX >= sSrcX)) {
          pStruct->uiFlags |= LEVELNODE_REVEAL;
          fRerender = TRUE;
        }
        break;

      case INSIDE_TOP_LEFT:
      case OUTSIDE_TOP_LEFT:
        fHitWall = TRUE;
        if ((fDoLeftWalls) && (sY >= sSrcY)) {
          pStruct->uiFlags |= LEVELNODE_REVEAL;
          fRerender = TRUE;
        }
        break;
    }
    pStruct = pStruct->pNext;
  }

  if (fRerender) SetRenderFlags(RENDER_FLAG_FULL);

  return (fHitWall);
}

BOOLEAN LightHideWall(int16_t sX, int16_t sY, int16_t sSrcX, int16_t sSrcY) {
  struct LEVELNODE *pStruct;
  uint32_t uiTile;
  BOOLEAN fRerender = FALSE, fHitWall = FALSE, fDoRightWalls = TRUE, fDoLeftWalls = TRUE;
  TILE_ELEMENT *TileElem;

  Assert(gpWorldLevelData != NULL);

  uiTile = MAPROWCOLTOPOS(sY, sX);

  if (sX < sSrcX) fDoRightWalls = FALSE;

  if (sY < sSrcY) fDoLeftWalls = FALSE;

  pStruct = gpWorldLevelData[uiTile].pStructHead;
  while (pStruct != NULL) {
    TileElem = &(gTileDatabase[pStruct->usIndex]);
    switch (TileElem->usWallOrientation) {
      case INSIDE_TOP_RIGHT:
      case OUTSIDE_TOP_RIGHT:
        if (!fDoRightWalls) fDoLeftWalls = FALSE;
        break;

      case INSIDE_TOP_LEFT:
      case OUTSIDE_TOP_LEFT:
        if (!fDoLeftWalls) fDoRightWalls = FALSE;
        break;
    }
    pStruct = pStruct->pNext;
  }

  pStruct = gpWorldLevelData[uiTile].pStructHead;
  while (pStruct != NULL) {
    TileElem = &(gTileDatabase[pStruct->usIndex]);
    switch (TileElem->usWallOrientation) {
      case NO_ORIENTATION:
        break;

      case INSIDE_TOP_RIGHT:
      case OUTSIDE_TOP_RIGHT:
        fHitWall = TRUE;
        if ((fDoRightWalls) && (sX >= sSrcX)) {
          pStruct->uiFlags &= (~LEVELNODE_REVEAL);
          fRerender = TRUE;
        }
        break;

      case INSIDE_TOP_LEFT:
      case OUTSIDE_TOP_LEFT:
        fHitWall = TRUE;
        if ((fDoLeftWalls) && (sY >= sSrcY)) {
          pStruct->uiFlags &= (~LEVELNODE_REVEAL);
          fRerender = TRUE;
        }
        break;
    }
    pStruct = pStruct->pNext;
  }

  if (fRerender) SetRenderFlags(RENDER_FLAG_FULL);

  return (fHitWall);
}

/****************************************************************************************
        CalcTranslucentWalls

                Tags walls as being translucent using a light template.

***************************************************************************************/
BOOLEAN CalcTranslucentWalls(int16_t iX, int16_t iY) {
  LIGHT_NODE *pLight;
  uint16_t uiCount;
  uint16_t usNodeIndex;
  if (pLightList[0] == NULL) return (FALSE);
  for (uiCount = 0; uiCount < usRaySize[0]; uiCount++) {
    usNodeIndex = *(pLightRayList[0] + uiCount);

    if (!(usNodeIndex & LIGHT_NEW_RAY)) {
      pLight = pLightList[0] + (usNodeIndex & (~LIGHT_BACKLIGHT));

      // Kris:  added map boundary checking!!!
      if (LightRevealWall((int16_t)min(max((iX + pLight->iDX), 0), WORLD_COLS - 1),
                          (int16_t)min(max((iY + pLight->iDY), 0), WORLD_ROWS - 1),
                          (int16_t)min(max(iX, 0), WORLD_COLS - 1),
                          (int16_t)min(max(iY, 0), WORLD_ROWS - 1))) {
        uiCount = LightFindNextRay(0, uiCount);
        SetRenderFlags(RENDER_FLAG_FULL);
      }
    }
  }

  return (TRUE);
}

BOOLEAN LightGreenTile(int16_t sX, int16_t sY, int16_t sSrcX, int16_t sSrcY) {
  struct LEVELNODE *pStruct, *pLand;
  uint32_t uiTile;
  BOOLEAN fHitWall = FALSE;
  TILE_ELEMENT *TileElem;

  Assert(gpWorldLevelData != NULL);

  uiTile = MAPROWCOLTOPOS(sY, sX);
  pStruct = gpWorldLevelData[uiTile].pStructHead;

  while (pStruct != NULL) {
    TileElem = &(gTileDatabase[pStruct->usIndex]);
    switch (TileElem->usWallOrientation) {
      case NO_ORIENTATION:
        break;

      case INSIDE_TOP_RIGHT:
      case OUTSIDE_TOP_RIGHT:
        fHitWall = TRUE;
        if (sX >= sSrcX) {
          pStruct->uiFlags |= LEVELNODE_REVEAL;
        }
        break;

      case INSIDE_TOP_LEFT:
      case OUTSIDE_TOP_LEFT:
        fHitWall = TRUE;
        if (sY >= sSrcY) {
          pStruct->uiFlags |= LEVELNODE_REVEAL;
        }
        break;
    }
    pStruct = pStruct->pNext;
  }

  pLand = gpWorldLevelData[uiTile].pLandHead;
  while (pLand != NULL) {
    pLand->ubShadeLevel = 0;
    pLand = pLand->pNext;
  }

  gpWorldLevelData[uiTile].uiFlags |= MAPELEMENT_REDRAW;
  SetRenderFlags(RENDER_FLAG_MARKED);

  return (fHitWall);
}

/****************************************************************************************
        LightShowRays

                Draws a template by making the ground tiles green. Must be polled once for
each tile drawn to facilitate animating the drawing process for debugging.

***************************************************************************************/
BOOLEAN LightShowRays(int16_t iX, int16_t iY, BOOLEAN fReset) {
  LIGHT_NODE *pLight;
  static uint16_t uiCount = 0;
  uint16_t usNodeIndex;

  if (fReset) uiCount = 0;

  if (pLightList[0] == NULL) return (FALSE);

  if (uiCount < usRaySize[0]) {
    usNodeIndex = *(pLightRayList[0] + uiCount);

    if (!(usNodeIndex & LIGHT_NEW_RAY)) {
      pLight = pLightList[0] + (usNodeIndex & (~LIGHT_BACKLIGHT));

      if (LightGreenTile((int16_t)(iX + pLight->iDX), (int16_t)(iY + pLight->iDY), iX, iY)) {
        uiCount = LightFindNextRay(0, uiCount);
        SetRenderFlags(RENDER_FLAG_MARKED);
      }
    }

    uiCount++;
    return (TRUE);
  } else
    return (FALSE);
}

/****************************************************************************************
        LightHideGreen

        Removes the green from the tiles that was drawn to show the path of the rays.

***************************************************************************************/
BOOLEAN LightHideGreen(int16_t sX, int16_t sY, int16_t sSrcX, int16_t sSrcY) {
  struct LEVELNODE *pStruct, *pLand;
  uint32_t uiTile;
  BOOLEAN fHitWall = FALSE;
  TILE_ELEMENT *TileElem;

  Assert(gpWorldLevelData != NULL);

  uiTile = MAPROWCOLTOPOS(sY, sX);
  pStruct = gpWorldLevelData[uiTile].pStructHead;

  while (pStruct != NULL) {
    TileElem = &(gTileDatabase[pStruct->usIndex]);
    switch (TileElem->usWallOrientation) {
      case NO_ORIENTATION:
        break;

      case INSIDE_TOP_RIGHT:
      case OUTSIDE_TOP_RIGHT:
        fHitWall = TRUE;
        if (sX >= sSrcX) {
          pStruct->uiFlags &= (~LEVELNODE_REVEAL);
        }
        break;

      case INSIDE_TOP_LEFT:
      case OUTSIDE_TOP_LEFT:
        fHitWall = TRUE;
        if (sY >= sSrcY) {
          pStruct->uiFlags &= (~LEVELNODE_REVEAL);
        }
        break;
    }
    pStruct = pStruct->pNext;
  }

  pLand = gpWorldLevelData[uiTile].pLandHead;
  while (pLand != NULL) {
    pLand->ubShadeLevel = pLand->ubNaturalShadeLevel;
    pLand = pLand->pNext;
  }

  gpWorldLevelData[uiTile].uiFlags |= MAPELEMENT_REDRAW;
  SetRenderFlags(RENDER_FLAG_MARKED);

  return (fHitWall);
}

/****************************************************************************************
        LightHideRays

                Hides walls that were revealed by CalcTranslucentWalls.

***************************************************************************************/
BOOLEAN LightHideRays(int16_t iX, int16_t iY) {
  LIGHT_NODE *pLight;
  uint16_t uiCount;
  uint16_t usNodeIndex;

  if (pLightList[0] == NULL) return (FALSE);

  for (uiCount = 0; uiCount < usRaySize[0]; uiCount++) {
    usNodeIndex = *(pLightRayList[0] + uiCount);

    if (!(usNodeIndex & LIGHT_NEW_RAY)) {
      pLight = pLightList[0] + (usNodeIndex & (~LIGHT_BACKLIGHT));

      if (LightHideWall((int16_t)(iX + pLight->iDX), (int16_t)(iY + pLight->iDY), iX, iY)) {
        uiCount = LightFindNextRay(0, uiCount);
        SetRenderFlags(RENDER_FLAG_MARKED);
      }
    }
  }

  return (TRUE);
}

/****************************************************************************************
        ApplyTranslucencyToWalls

                Hides walls that were revealed by CalcTranslucentWalls.

***************************************************************************************/
BOOLEAN ApplyTranslucencyToWalls(int16_t iX, int16_t iY) {
  LIGHT_NODE *pLight;
  uint16_t uiCount;
  uint16_t usNodeIndex;

  if (pLightList[0] == NULL) return (FALSE);

  for (uiCount = 0; uiCount < usRaySize[0]; uiCount++) {
    usNodeIndex = *(pLightRayList[0] + uiCount);

    if (!(usNodeIndex & LIGHT_NEW_RAY)) {
      pLight = pLightList[0] + (usNodeIndex & (~LIGHT_BACKLIGHT));
      // Kris:  added map boundary checking!!!
      if (LightHideWall((int16_t)min(max((iX + pLight->iDX), 0), WORLD_COLS - 1),
                        (int16_t)min(max((iY + pLight->iDY), 0), WORLD_ROWS - 1),
                        (int16_t)min(max(iX, 0), WORLD_COLS - 1),
                        (int16_t)min(max(iY, 0), WORLD_ROWS - 1))) {
        uiCount = LightFindNextRay(0, uiCount);
        SetRenderFlags(RENDER_FLAG_FULL);
      }
    }
  }

  return (TRUE);
}

/****************************************************************************************
        LightTranslucentTrees

                Makes all the near-side trees around a given coordinate translucent.
***************************************************************************************/
BOOLEAN LightTranslucentTrees(int16_t iX, int16_t iY) {
  int32_t iCountX, iCountY;
  uint32_t uiTile;
  struct LEVELNODE *pNode;
  BOOLEAN fRerender = FALSE;
  uint32_t fTileFlags;

  for (iCountY = iY; iCountY < (int16_t)(iY + LIGHT_TREE_REVEAL); iCountY++)
    for (iCountX = iX; iCountX < (int16_t)(iX + LIGHT_TREE_REVEAL); iCountX++) {
      uiTile = MAPROWCOLTOPOS(iCountY, iCountX);
      pNode = gpWorldLevelData[uiTile].pStructHead;
      while (pNode != NULL) {
        GetTileFlags(pNode->usIndex, &fTileFlags);

        if (fTileFlags & FULL3D_TILE) {
          if (!(pNode->uiFlags & LEVELNODE_REVEALTREES)) {
            // pNode->uiFlags |= ( LEVELNODE_REVEALTREES | LEVELNODE_ERASEZ );
            pNode->uiFlags |= (LEVELNODE_REVEALTREES);
            gpWorldLevelData[uiTile].uiFlags |= MAPELEMENT_REDRAW;
          }

          fRerender = TRUE;
        }
        pNode = pNode->pNext;
      }
    }

  if (fRerender) {
    // SetRenderFlags(RENDER_FLAG_MARKED);
    SetRenderFlags(RENDER_FLAG_FULL);
    return (TRUE);
  } else
    return (FALSE);
}

/****************************************************************************************
        LightHideTrees

                Removes the translucency from any trees in the area.
***************************************************************************************/
BOOLEAN LightHideTrees(int16_t iX, int16_t iY) {
  int32_t iCountX, iCountY;
  uint32_t uiTile;
  struct LEVELNODE *pNode;
  BOOLEAN fRerender = FALSE;
  uint32_t fTileFlags;

  // Kris:  added map boundary checking!!!
  for (iCountY = (int16_t)max(iY - LIGHT_TREE_REVEAL, 0);
       iCountY < (int16_t)min(iY + LIGHT_TREE_REVEAL, WORLD_ROWS - 1); iCountY++)
    for (iCountX = (int16_t)max(iX - LIGHT_TREE_REVEAL, 0);
         iCountX < (int16_t)min(iX + LIGHT_TREE_REVEAL, WORLD_COLS - 1); iCountX++) {
      uiTile = MAPROWCOLTOPOS(iCountY, iCountX);
      pNode = gpWorldLevelData[uiTile].pStructHead;
      while (pNode != NULL) {
        GetTileFlags(pNode->usIndex, &fTileFlags);

        if (fTileFlags & FULL3D_TILE) {
          if ((pNode->uiFlags & LEVELNODE_REVEALTREES)) {
            // pNode->uiFlags  &=(~( LEVELNODE_REVEALTREES | LEVELNODE_ERASEZ ) );
            pNode->uiFlags &= (~(LEVELNODE_REVEALTREES));
            gpWorldLevelData[uiTile].uiFlags |= MAPELEMENT_REDRAW;
          }

          fRerender = TRUE;
        }
        pNode = pNode->pNext;
      }
    }

  if (fRerender) {
    // SetRenderFlags(RENDER_FLAG_MARKED);
    SetRenderFlags(RENDER_FLAG_FULL);
    return (TRUE);
  } else
    return (FALSE);
}

/****************************************************************************************
        LightErase

                Reverts all tiles a given light affects to their natural light levels.

***************************************************************************************/
BOOLEAN LightErase(uint32_t uiLightType, int32_t iLight, int16_t iX, int16_t iY,
                   uint32_t uiSprite) {
  LIGHT_NODE *pLight;
  uint16_t uiCount;
  uint16_t usNodeIndex;
  uint32_t uiFlags;
  int32_t iOldX, iOldY;
  BOOLEAN fBlocked = FALSE;
  BOOLEAN fOnlyWalls;

  if (pLightList[iLight] == NULL) return (FALSE);

  // clear out all the flags
  for (uiCount = 0; uiCount < usTemplateSize[iLight]; uiCount++) {
    pLight = pLightList[iLight] + uiCount;
    pLight->uiFlags &= (~LIGHT_NODE_DRAWN);
  }

  iOldX = iX;
  iOldY = iY;

  for (uiCount = 0; uiCount < usRaySize[iLight]; uiCount++) {
    usNodeIndex = *(pLightRayList[iLight] + uiCount);
    if (!(usNodeIndex & LIGHT_NEW_RAY)) {
      fBlocked = FALSE;
      fOnlyWalls = FALSE;

      pLight = pLightList[iLight] + (usNodeIndex & (~LIGHT_BACKLIGHT));

      if (!(LightSprites[uiSprite].uiFlags & LIGHT_SPR_ONROOF)) {
        if (LightTileBlocked((int16_t)iOldX, (int16_t)iOldY, (int16_t)(iX + pLight->iDX),
                             (int16_t)(iY + pLight->iDY))) {
          uiCount = LightFindNextRay(iLight, uiCount);

          fOnlyWalls = TRUE;
          fBlocked = TRUE;
        }
      }

      if (!(pLight->uiFlags & LIGHT_NODE_DRAWN) && (pLight->ubLight)) {
        uiFlags = (uint32_t)(usNodeIndex & LIGHT_BACKLIGHT);
        if (LightSprites[uiSprite].uiFlags & MERC_LIGHT) uiFlags |= LIGHT_FAKE;
        if (LightSprites[uiSprite].uiFlags & LIGHT_SPR_ONROOF) uiFlags |= LIGHT_ROOF_ONLY;

        LightSubtractTile(uiLightType, (int16_t)iOldX, (int16_t)iOldY, (int16_t)(iX + pLight->iDX),
                          (int16_t)(iY + pLight->iDY), pLight->ubLight, uiFlags, fOnlyWalls);
        pLight->uiFlags |= LIGHT_NODE_DRAWN;
      }

      if (fBlocked) {
        iOldX = iX;
        iOldY = iY;
      } else {
        iOldX = iX + pLight->iDX;
        iOldY = iY + pLight->iDY;
      }
    } else {
      iOldX = iX;
      iOldY = iY;
    }
  }

  return (TRUE);
}

/****************************************************************************************
        LightCalcRect

                Calculates the rect size of a given light, used in dirtying the screen after
updating a light.

***************************************************************************************/
BOOLEAN LightCalcRect(int32_t iLight) {
  SGPRect MaxRect;
  int16_t sXValue, sYValue, sDummy;
  uint32_t uiCount;
  LIGHT_NODE *pLight;

  if (pLightList[iLight] == NULL) return (FALSE);

  pLight = pLightList[iLight];

  MaxRect.iLeft = 99999;
  MaxRect.iRight = -99999;
  MaxRect.iTop = 99999;
  MaxRect.iBottom = -99999;

  for (uiCount = 0; uiCount < usTemplateSize[iLight]; uiCount++) {
    pLight = pLightList[iLight] + uiCount;
    if (pLight->ubLight) {
      MaxRect.iLeft = min(MaxRect.iLeft, pLight->iDX);
      MaxRect.iRight = max(MaxRect.iRight, pLight->iDX);
      MaxRect.iTop = min(MaxRect.iTop, pLight->iDY);
      MaxRect.iBottom = max(MaxRect.iBottom, pLight->iDY);
    }
  }

  FromCellToScreenCoordinates((int16_t)(MaxRect.iLeft * CELL_X_SIZE),
                              (int16_t)(MaxRect.iTop * CELL_Y_SIZE), &sDummy, &sYValue);

  LightMapLeft[iLight] = (int16_t)MaxRect.iLeft;
  LightMapTop[iLight] = (int16_t)MaxRect.iTop;
  LightMapRight[iLight] = (int16_t)MaxRect.iRight;
  LightMapBottom[iLight] = (int16_t)MaxRect.iBottom;

  LightHeight[iLight] = -sYValue;
  LightYOffset[iLight] = sYValue;

  FromCellToScreenCoordinates((int16_t)(MaxRect.iRight * CELL_X_SIZE),
                              (int16_t)(MaxRect.iBottom * CELL_Y_SIZE), &sDummy, &sYValue);
  LightHeight[iLight] += sYValue;

  FromCellToScreenCoordinates((int16_t)(MaxRect.iLeft * CELL_X_SIZE),
                              (int16_t)(MaxRect.iBottom * CELL_Y_SIZE), &sXValue, &sDummy);
  LightWidth[iLight] = -sXValue;
  LightXOffset[iLight] = sXValue;

  FromCellToScreenCoordinates((int16_t)(MaxRect.iRight * CELL_X_SIZE),
                              (int16_t)(MaxRect.iTop * CELL_Y_SIZE), &sXValue, &sDummy);
  LightWidth[iLight] += sXValue;

  LightHeight[iLight] += WORLD_TILE_X * 2;
  LightWidth[iLight] += WORLD_TILE_Y * 3;
  LightXOffset[iLight] -= WORLD_TILE_X * 2;
  LightYOffset[iLight] -= WORLD_TILE_Y * 2;

  return (TRUE);
}

/****************************************************************************************
        LightSave

                Saves the light list of a given template to a file. Passing in NULL for the
        filename forces the system to save the light with the internal filename (recommended).

***************************************************************************************/
BOOLEAN LightSave(int32_t iLight, char *pFilename) {
  FileID hFile = FILE_ID_ERR;
  char *pName;

  if (pLightList[iLight] == NULL)
    return (FALSE);
  else {
    if (pFilename == NULL)
      pName = pLightNames[iLight];
    else
      pName = pFilename;

    if ((hFile = File_OpenForWriting(pName)) != 0) {
      File_Write(hFile, &usTemplateSize[iLight], sizeof(uint16_t), NULL);
      File_Write(hFile, pLightList[iLight], sizeof(LIGHT_NODE) * usTemplateSize[iLight], NULL);
      File_Write(hFile, &usRaySize[iLight], sizeof(uint16_t), NULL);
      File_Write(hFile, pLightRayList[iLight], sizeof(uint16_t) * usRaySize[iLight], NULL);

      File_Close(hFile);
    } else
      return (FALSE);
  }
  return (TRUE);
}

/****************************************************************************************
        LightLoad

                Loads a light template from disk. The light template number is returned, or (-1)
        if the file wasn't loaded.

***************************************************************************************/
int32_t LightLoad(char *pFilename) {
  FileID hFile = FILE_ID_ERR;
  int32_t iLight;

  if ((iLight = LightGetFree()) == (-1))
    return (-1);
  else {
    if ((hFile = File_OpenForReading(pFilename)) != 0) {
      File_Read(hFile, &usTemplateSize[iLight], sizeof(uint16_t), NULL);
      if ((pLightList[iLight] =
               (LIGHT_NODE *)MemAlloc(usTemplateSize[iLight] * sizeof(LIGHT_NODE))) == NULL) {
        usTemplateSize[iLight] = 0;
        return (-1);
      }
      File_Read(hFile, pLightList[iLight], sizeof(LIGHT_NODE) * usTemplateSize[iLight], NULL);

      File_Read(hFile, &usRaySize[iLight], sizeof(uint16_t), NULL);
      if ((pLightRayList[iLight] = (uint16_t *)MemAlloc(usRaySize[iLight] * sizeof(uint16_t))) ==
          NULL) {
        usTemplateSize[iLight] = 0;
        usRaySize[iLight] = 0;
        MemFree(pLightList[iLight]);
        return (-1);
      }
      File_Read(hFile, pLightRayList[iLight], sizeof(uint16_t) * usRaySize[iLight], NULL);

      File_Close(hFile);

      pLightNames[iLight] = (char *)MemAlloc(strlen(pFilename) + 1);
      strcpy(pLightNames[iLight], pFilename);
    } else
      return (-1);
  }
  LightCalcRect(iLight);
  return (iLight);
}

/****************************************************************************************
        LightLoadCachedTemplate

                Figures out whether a light template is already in memory, or needs to be loaded
from disk. Returns the index of the template, or (-1) if it couldn't be loaded.

***************************************************************************************/
int32_t LightLoadCachedTemplate(char *pFilename) {
  int32_t iCount;

  for (iCount = 0; iCount < MAX_LIGHT_TEMPLATES; iCount++) {
    if ((pLightNames[iCount] != NULL) && !(strcasecmp(pFilename, pLightNames[iCount])))
      return (iCount);
  }

  return (LightLoad(pFilename));
}

uint8_t LightGetColors(struct SGPPaletteEntry *pPal) {
  if (pPal != NULL)
    memcpy(pPal, &gpOrigLights[0], sizeof(struct SGPPaletteEntry) * gubNumLightColors);

  return (gubNumLightColors);
}

/****************************************************************************************
        LightSetColors

        Sets the number of light colors, and the RGB value for each.

***************************************************************************************/
#ifdef JA2EDITOR
BOOLEAN gfEditorForceRebuildAllColors = FALSE;
#endif

extern void SetAllNewTileSurfacesLoaded(BOOLEAN fNew);

BOOLEAN LightSetColors(struct SGPPaletteEntry *pPal, uint8_t ubNumColors) {
  int16_t sRed, sGreen, sBlue;

  Assert(ubNumColors >= 1 && ubNumColors <= 2);
  Assert(pPal);

  if (pPal[0].peRed != gpLightColors[0].peRed || pPal[0].peGreen != gpLightColors[0].peGreen ||
      pPal[0].peBlue != gpLightColors[0].peBlue) {  // Set the entire tileset database so that it
                                                    // reloads everything.  It has to because the
    // colors have changed.
    SetAllNewTileSurfacesLoaded(TRUE);
  }

  // before doing anything, get rid of all the old palettes
  DestroyTileShadeTables();

  // we will have at least one light color
  memcpy(&gpLightColors[0], &pPal[0], sizeof(struct SGPPaletteEntry));
  memcpy(&gpOrigLights[0], &pPal[0], sizeof(struct SGPPaletteEntry) * 2);

  gubNumLightColors = ubNumColors;

  // if there are two colors, calculate a third palette that is a mix of the two
  if (ubNumColors == 2) {
    sRed = min((((int16_t)pPal[0].peRed) * LVL1_L1_PER / 100 +
                ((int16_t)pPal[1].peRed) * LVL1_L2_PER / 100),
               255);
    sGreen = min((((int16_t)pPal[0].peGreen) * LVL1_L1_PER / 100 +
                  ((int16_t)pPal[1].peGreen) * LVL1_L2_PER / 100),
                 255);
    sBlue = min((((int16_t)pPal[0].peBlue) * LVL1_L1_PER / 100 +
                 ((int16_t)pPal[1].peBlue) * LVL1_L2_PER / 100),
                255);

    gpLightColors[1].peRed = (uint8_t)(sRed);
    gpLightColors[1].peGreen = (uint8_t)(sGreen);
    gpLightColors[1].peBlue = (uint8_t)(sBlue);

    sRed = min((((int16_t)pPal[0].peRed) * LVL2_L1_PER / 100 +
                ((int16_t)pPal[1].peRed) * LVL2_L2_PER / 100),
               255);
    sGreen = min((((int16_t)pPal[0].peGreen) * LVL2_L1_PER / 100 +
                  ((int16_t)pPal[1].peGreen) * LVL2_L2_PER / 100),
                 255);
    sBlue = min((((int16_t)pPal[0].peBlue) * LVL2_L1_PER / 100 +
                 ((int16_t)pPal[1].peBlue) * LVL2_L2_PER / 100),
                255);

    gpLightColors[2].peRed = (uint8_t)(sRed);
    gpLightColors[2].peGreen = (uint8_t)(sGreen);
    gpLightColors[2].peBlue = (uint8_t)(sBlue);
  }

  BuildTileShadeTables();

  // Build all palettes for all soldiers in the world
  // ( THIS FUNCTION WILL ERASE THEM IF THEY EXIST )
  RebuildAllSoldierShadeTables();

  RebuildAllCorpseShadeTables();

  SetRenderFlags(RENDER_FLAG_FULL);

  return (TRUE);
}

//---------------------------------------------------------------------------------------
// Light Manipulation Layer
//---------------------------------------------------------------------------------------

/********************************************************************************
 * LightSpriteGetFree
 *
 *		Returns the index of the next available sprite.
 *
 ********************************************************************************/
int32_t LightSpriteGetFree(void) {
  int32_t iCount;

  for (iCount = 0; iCount < MAX_LIGHT_SPRITES; iCount++) {
    if (!(LightSprites[iCount].uiFlags & LIGHT_SPR_ACTIVE)) return (iCount);
  }

  return (-1);
}

/********************************************************************************
 * LightSpriteCreate
 *
 *		Creates an instance of a light. The template is loaded if it isn't already.
 * If this function fails (out of sprites, or bad template name) it returns (-1).
 *
 ********************************************************************************/
int32_t LightSpriteCreate(char *pName, uint32_t uiLightType) {
  int32_t iSprite;

  if ((iSprite = LightSpriteGetFree()) != (-1)) {
    memset(&LightSprites[iSprite], 0, sizeof(LIGHT_SPRITE));
    LightSprites[iSprite].iX = WORLD_COLS + 1;
    LightSprites[iSprite].iY = WORLD_ROWS + 1;
    LightSprites[iSprite].iOldX = WORLD_COLS + 1;
    LightSprites[iSprite].iOldY = WORLD_ROWS + 1;
    LightSprites[iSprite].uiLightType = uiLightType;

    if ((LightSprites[iSprite].iTemplate = LightLoadCachedTemplate(pName)) == (-1)) return (-1);

    LightSprites[iSprite].uiFlags |= LIGHT_SPR_ACTIVE;
  }

  return (iSprite);
}

/********************************************************************************
 * LightSpriteFake
 *
 *		Sets the flag of a light sprite to "fake" (in game for merc navig purposes)
 *
 ********************************************************************************/
BOOLEAN LightSpriteFake(int32_t iSprite) {
  if (LightSprites[iSprite].uiFlags & LIGHT_SPR_ACTIVE) {
    LightSprites[iSprite].uiFlags |= MERC_LIGHT;
    return (TRUE);
  } else {
    return (FALSE);
  }
}

/********************************************************************************
 * LightSpriteDestroy
 *
 *		Removes an instance of a light. If it was on, it is erased from the scene.
 *
 ********************************************************************************/
BOOLEAN LightSpriteDestroy(int32_t iSprite) {
  if (LightSprites[iSprite].uiFlags & LIGHT_SPR_ACTIVE) {
    if (LightSprites[iSprite].uiFlags & LIGHT_SPR_ERASE) {
      if ((LightSprites[iSprite].iX < WORLD_COLS) && (LightSprites[iSprite].iY < WORLD_ROWS)) {
        LightErase(LightSprites[iSprite].uiLightType, LightSprites[iSprite].iTemplate,
                   LightSprites[iSprite].iX, LightSprites[iSprite].iY, iSprite);
        LightSpriteDirty(iSprite);
      }
      LightSprites[iSprite].uiFlags &= (~LIGHT_SPR_ERASE);
    }

    LightSprites[iSprite].uiFlags &= (~LIGHT_SPR_ACTIVE);
    return (TRUE);
  }

  return (FALSE);
}

/********************************************************************************
 * LightSpriteRender
 *
 *		Currently unused.
 *
 ********************************************************************************/
BOOLEAN LightSpriteRender(void) {
  // int32_t iCount;
  // BOOLEAN fRenderLights=FALSE;

  return (FALSE);

  /*	for(iCount=0; iCount < MAX_LIGHT_SPRITES; iCount++)
          {
                  if(LightSprites[iCount].uiFlags&LIGHT_SPR_ACTIVE)
                  {
                          if((LightSprites[iCount].iX!=LightSprites[iCount].iOldX)
                                  || (LightSprites[iCount].iY!=LightSprites[iCount].iOldY)
                                  || (LightSprites[iCount].uiFlags&LIGHT_SPR_REDRAW))
                          {
                                  if(LightSprites[iCount].iOldX < WORLD_COLS)
                                  {
                                          fRenderLights=TRUE;
                                          LightSpriteDirty(iCount);
                                  }

                                  LightSprites[iCount].iOldX=LightSprites[iCount].iX;
                                  LightSprites[iCount].iOldY=LightSprites[iCount].iY;

                                  if(LightSprites[iCount].uiFlags&LIGHT_SPR_ON)
                                  {
                                                  LightSpriteDirty(iCount);
                                                  fRenderLights=TRUE;
                                  }

                                  LightSprites[iCount].uiFlags&=(~LIGHT_SPR_REDRAW);
                          }
                  }
          }

          if(fRenderLights)
          {
                  LightResetAllTiles();
                  for(iCount=0; iCount < MAX_LIGHT_SPRITES; iCount++)
                          if((LightSprites[iCount].uiFlags&LIGHT_SPR_ACTIVE) &&
     (LightSprites[iCount].uiFlags&LIGHT_SPR_ON)) LightDraw(LightSprites[iCount].iTemplate,
     LightSprites[iCount].iX, LightSprites[iCount].iY, iCount); return(TRUE);
          }

          return(FALSE); */
}

/********************************************************************************
 * LightSpriteRenderAll
 *
 *		Resets all tiles in the world to the ambient light level, and redraws all
 * active lights.
 *
 ********************************************************************************/
BOOLEAN LightSpriteRenderAll(void) {
  int32_t iCount;

  LightResetAllTiles();
  for (iCount = 0; iCount < MAX_LIGHT_SPRITES; iCount++) {
    LightSprites[iCount].uiFlags &= (~LIGHT_SPR_ERASE);

    if ((LightSprites[iCount].uiFlags & LIGHT_SPR_ACTIVE) &&
        (LightSprites[iCount].uiFlags & LIGHT_SPR_ON)) {
      LightDraw(LightSprites[iCount].uiLightType, LightSprites[iCount].iTemplate,
                LightSprites[iCount].iX, LightSprites[iCount].iY, iCount);
      LightSprites[iCount].uiFlags |= LIGHT_SPR_ERASE;
      LightSpriteDirty(iCount);
    }

    LightSprites[iCount].iOldX = LightSprites[iCount].iX;
    LightSprites[iCount].iOldY = LightSprites[iCount].iY;
  }

  return (TRUE);
}

/********************************************************************************
 * LightSpritePosition
 *
 *		Sets the X,Y position (IN TILES) of a light instance.
 *
 ********************************************************************************/
BOOLEAN LightSpritePosition(int32_t iSprite, int16_t iX, int16_t iY) {
  if (LightSprites[iSprite].uiFlags & LIGHT_SPR_ACTIVE) {
    if ((LightSprites[iSprite].iX == iX) && (LightSprites[iSprite].iY == iY)) return (TRUE);

    if (LightSprites[iSprite].uiFlags & LIGHT_SPR_ERASE) {
      if ((LightSprites[iSprite].iX < WORLD_COLS) && (LightSprites[iSprite].iY < WORLD_ROWS)) {
        LightErase(LightSprites[iSprite].uiLightType, LightSprites[iSprite].iTemplate,
                   LightSprites[iSprite].iX, LightSprites[iSprite].iY, iSprite);
        LightSpriteDirty(iSprite);
      }
    }

    // LightSprites[iSprite].iOldX=LightSprites[iSprite].iX;
    // LightSprites[iSprite].iOldY=LightSprites[iSprite].iY;

    LightSprites[iSprite].iX = iX;
    LightSprites[iSprite].iY = iY;

    if (LightSprites[iSprite].uiFlags & LIGHT_SPR_ON) {
      if ((LightSprites[iSprite].iX < WORLD_COLS) && (LightSprites[iSprite].iY < WORLD_ROWS)) {
        LightDraw(LightSprites[iSprite].uiLightType, LightSprites[iSprite].iTemplate, iX, iY,
                  iSprite);
        LightSprites[iSprite].uiFlags |= LIGHT_SPR_ERASE;
        LightSpriteDirty(iSprite);
      }
    }
  } else
    return (FALSE);

  return (TRUE);
}

/********************************************************************************
 * LightSpriteRoofStatus
 *
 *		Determines whether a light is on a roof or not.
 *
 ********************************************************************************/
BOOLEAN LightSpriteRoofStatus(int32_t iSprite, BOOLEAN fOnRoof) {
  if (fOnRoof && (LightSprites[iSprite].uiFlags & LIGHT_SPR_ONROOF)) return (FALSE);

  if (!fOnRoof && !(LightSprites[iSprite].uiFlags & LIGHT_SPR_ONROOF)) return (FALSE);

  if (LightSprites[iSprite].uiFlags & LIGHT_SPR_ACTIVE) {
    if (LightSprites[iSprite].uiFlags & LIGHT_SPR_ERASE) {
      if ((LightSprites[iSprite].iX < WORLD_COLS) && (LightSprites[iSprite].iY < WORLD_ROWS)) {
        LightErase(LightSprites[iSprite].uiLightType, LightSprites[iSprite].iTemplate,
                   LightSprites[iSprite].iX, LightSprites[iSprite].iY, iSprite);
        LightSpriteDirty(iSprite);
      }
    }

    if (fOnRoof)
      LightSprites[iSprite].uiFlags |= LIGHT_SPR_ONROOF;
    else
      LightSprites[iSprite].uiFlags &= (~LIGHT_SPR_ONROOF);

    if (LightSprites[iSprite].uiFlags & LIGHT_SPR_ON) {
      if ((LightSprites[iSprite].iX < WORLD_COLS) && (LightSprites[iSprite].iY < WORLD_ROWS)) {
        LightDraw(LightSprites[iSprite].uiLightType, LightSprites[iSprite].iTemplate,
                  LightSprites[iSprite].iX, LightSprites[iSprite].iY, iSprite);
        LightSprites[iSprite].uiFlags |= LIGHT_SPR_ERASE;
        LightSpriteDirty(iSprite);
      }
    }
  } else
    return (FALSE);

  return (TRUE);
}

/********************************************************************************
 * LightSpritePower
 *
 *		Turns on or off a light, based on the BOOLEAN.
 *
 ********************************************************************************/
BOOLEAN LightSpritePower(int32_t iSprite, BOOLEAN fOn) {
  if (fOn) {
    LightSprites[iSprite].uiFlags |= (LIGHT_SPR_ON | LIGHT_SPR_REDRAW);
    LightSprites[iSprite].iOldX = WORLD_COLS;
  } else
    LightSprites[iSprite].uiFlags &= (~LIGHT_SPR_ON);

  return (TRUE);
}

/********************************************************************************
 * LightSpriteDirty
 *
 *		Sets the flag for the renderer to draw all marked tiles.
 *
 ********************************************************************************/
BOOLEAN LightSpriteDirty(int32_t iSprite) {
  // int16_t iLeft_s, iTop_s;
  // int16_t iMapLeft, iMapTop, iMapRight, iMapBottom;

  // CellXYToScreenXY((int16_t)(LightSprites[iSprite].iX*CELL_X_SIZE),
  //								(int16_t)(LightSprites[iSprite].iY*CELL_Y_SIZE),
  //&iLeft_s, &iTop_s);

  // iLeft_s+=LightXOffset[LightSprites[iSprite].iTemplate];
  // iTop_s+=LightYOffset[LightSprites[iSprite].iTemplate];

  // iMapLeft=LightSprites[iSprite].iX+LightMapLeft[LightSprites[iSprite].iTemplate];
  // iMapTop=LightSprites[iSprite].iY+LightMapTop[LightSprites[iSprite].iTemplate];
  // iMapRight=LightSprites[iSprite].iX+LightMapRight[LightSprites[iSprite].iTemplate];
  // iMapBottom=LightSprites[iSprite].iY+LightMapBottom[LightSprites[iSprite].iTemplate];

  // ReRenderWorld(iMapLeft, iMapTop, iMapRight, iMapBottom);
  // UpdateSaveBuffer();
  // AddBaseDirtyRect(gsVIEWPORT_START_X, gsVIEWPORT_START_Y, gsVIEWPORT_END_X, gsVIEWPORT_END_Y );
  // AddBaseDirtyRect(iLeft_s, iTop_s,
  //								(int16_t)(iLeft_s+LightWidth[LightSprites[iSprite].iTemplate]),
  //								(int16_t)(iTop_s+LightHeight[LightSprites[iSprite].iTemplate]));

  SetRenderFlags(RENDER_FLAG_MARKED);

  return (TRUE);
}

BOOLEAN CreateObjectPalette(struct VObject *pObj, uint32_t uiBase,
                            struct SGPPaletteEntry *pShadePal) {
  uint32_t uiCount;

  pObj->pShades[uiBase] = Create16BPPPaletteShaded(
      pShadePal, gusShadeLevels[0][0], gusShadeLevels[0][1], gusShadeLevels[0][2], TRUE);

  for (uiCount = 1; uiCount < 16; uiCount++) {
    pObj->pShades[uiBase + uiCount] =
        Create16BPPPaletteShaded(pShadePal, gusShadeLevels[uiCount][0], gusShadeLevels[uiCount][1],
                                 gusShadeLevels[uiCount][2], FALSE);
  }

  return (TRUE);
}

BOOLEAN CreateSoldierShadedPalette(struct SOLDIERTYPE *pSoldier, uint32_t uiBase,
                                   struct SGPPaletteEntry *pShadePal) {
  uint32_t uiCount;

  pSoldier->pShades[uiBase] = Create16BPPPaletteShaded(
      pShadePal, gusShadeLevels[0][0], gusShadeLevels[0][1], gusShadeLevels[0][2], TRUE);

  for (uiCount = 1; uiCount < 16; uiCount++) {
    pSoldier->pShades[uiBase + uiCount] =
        Create16BPPPaletteShaded(pShadePal, gusShadeLevels[uiCount][0], gusShadeLevels[uiCount][1],
                                 gusShadeLevels[uiCount][2], FALSE);
  }

  return (TRUE);
}

/**********************************************************************************************
 CreateObjectPaletteTables

                Creates the shading tables for 8-bit brushes. One highlight table is created, based
on the object-type, 3 brightening tables, 1 normal, and 11 darkening tables. The entries are created
iteratively, rather than in a loop to allow hand-tweaking of the values. If you change the
HVOBJECT_SHADE_TABLES symbol, remember to add/delete entries here, it won't adjust automagically.

**********************************************************************************************/
#ifdef JA2TESTVERSION
extern uint32_t uiNumTablesSaved;
#endif

uint16_t CreateTilePaletteTables(struct VObject *pObj, uint32_t uiTileIndex, BOOLEAN fForce) {
  uint32_t uiCount;
  struct SGPPaletteEntry LightPal[256];
  BOOLEAN fLoaded = FALSE;

  Assert(pObj != NULL);

  // create the basic shade table
  if (!gfForceBuildShadeTables && !fForce) {  // The overwhelming majority of maps use the neutral
                                              // 0,0,0 light for outdoors.  These shadetables
    // are extremely time consuming to generate, so we will attempt to load them.  If we do, then
    // we skip the generation process altogether.
    if (LoadShadeTable(pObj, uiTileIndex)) {
      fLoaded = TRUE;
    }
  }
  if (!fLoaded) {  // This is expensive as hell to call!
    for (uiCount = 0; uiCount < 256; uiCount++) {
      // combine the rgb of the light color with the object's palette
      LightPal[uiCount].peRed = (uint8_t)(min(
          (uint16_t)pObj->pPaletteEntry[uiCount].peRed + (uint16_t)gpLightColors[0].peRed, 255));
      LightPal[uiCount].peGreen = (uint8_t)(min(
          (uint16_t)pObj->pPaletteEntry[uiCount].peGreen + (uint16_t)gpLightColors[0].peGreen,
          255));
      LightPal[uiCount].peBlue = (uint8_t)(min(
          (uint16_t)pObj->pPaletteEntry[uiCount].peBlue + (uint16_t)gpLightColors[0].peBlue, 255));
    }
    // build the shade tables
    CreateObjectPalette(pObj, 0, LightPal);

    // We paid to generate the shade table, so now save it, so we don't have to regenerate it ever
    // again!
    if (!gfForceBuildShadeTables && !gpLightColors[0].peRed && !gpLightColors[0].peGreen &&
        !gpLightColors[0].peBlue) {
      SaveShadeTable(pObj, uiTileIndex);
    }
#ifdef JA2TESTVERSION
    else
      uiNumTablesSaved++;
#endif
  }

  // if two lights are active
  if (gubNumLightColors == 2) {
    // build the second light's palette and table
    for (uiCount = 0; uiCount < 256; uiCount++) {
      LightPal[uiCount].peRed = (uint8_t)(min(
          (uint16_t)pObj->pPaletteEntry[uiCount].peRed + (uint16_t)gpLightColors[1].peRed, 255));
      LightPal[uiCount].peGreen = (uint8_t)(min(
          (uint16_t)pObj->pPaletteEntry[uiCount].peGreen + (uint16_t)gpLightColors[1].peGreen,
          255));
      LightPal[uiCount].peBlue = (uint8_t)(min(
          (uint16_t)pObj->pPaletteEntry[uiCount].peBlue + (uint16_t)gpLightColors[1].peBlue, 255));
    }
    CreateObjectPalette(pObj, 16, LightPal);

    // build a table that is a mix of the first two
    for (uiCount = 0; uiCount < 256; uiCount++) {
      LightPal[uiCount].peRed = (uint8_t)(min(
          (uint16_t)pObj->pPaletteEntry[uiCount].peRed + (uint16_t)gpLightColors[2].peRed, 255));
      LightPal[uiCount].peGreen = (uint8_t)(min(
          (uint16_t)pObj->pPaletteEntry[uiCount].peGreen + (uint16_t)gpLightColors[2].peGreen,
          255));
      LightPal[uiCount].peBlue = (uint8_t)(min(
          (uint16_t)pObj->pPaletteEntry[uiCount].peBlue + (uint16_t)gpLightColors[2].peBlue, 255));
    }
    CreateObjectPalette(pObj, 32, LightPal);
  }

  // build neutral palette as well!
  // Set current shade table to neutral color
  pObj->pShadeCurrent = pObj->pShades[4];
  pObj->pGlow = pObj->pShades[0];

  return (TRUE);
}

uint16_t CreateSoldierPaletteTables(struct SOLDIERTYPE *pSoldier, uint32_t uiType) {
  struct SGPPaletteEntry LightPal[256];
  uint32_t uiCount;

  // create the basic shade table
  for (uiCount = 0; uiCount < 256; uiCount++) {
    // combine the rgb of the light color with the object's palette
    LightPal[uiCount].peRed = (uint8_t)(min(
        (uint16_t)pSoldier->p8BPPPalette[uiCount].peRed + (uint16_t)gpLightColors[0].peRed, 255));
    LightPal[uiCount].peGreen = (uint8_t)(min(
        (uint16_t)pSoldier->p8BPPPalette[uiCount].peGreen + (uint16_t)gpLightColors[0].peGreen,
        255));
    LightPal[uiCount].peBlue = (uint8_t)(min(
        (uint16_t)pSoldier->p8BPPPalette[uiCount].peBlue + (uint16_t)gpLightColors[0].peBlue, 255));
  }
  // build the shade tables
  CreateSoldierShadedPalette(pSoldier, 0, LightPal);

  // if two lights are active
  if (gubNumLightColors == 2) {
    // build the second light's palette and table
    for (uiCount = 0; uiCount < 256; uiCount++) {
      LightPal[uiCount].peRed = (uint8_t)(min(
          (uint16_t)pSoldier->p8BPPPalette[uiCount].peRed + (uint16_t)gpLightColors[1].peRed, 255));
      LightPal[uiCount].peGreen = (uint8_t)(min(
          (uint16_t)pSoldier->p8BPPPalette[uiCount].peGreen + (uint16_t)gpLightColors[1].peGreen,
          255));
      LightPal[uiCount].peBlue = (uint8_t)(min(
          (uint16_t)pSoldier->p8BPPPalette[uiCount].peBlue + (uint16_t)gpLightColors[1].peBlue,
          255));
    }
    CreateSoldierShadedPalette(pSoldier, 16, LightPal);

    // build a table that is a mix of the first two
    for (uiCount = 0; uiCount < 256; uiCount++) {
      LightPal[uiCount].peRed = (uint8_t)(min(
          (uint16_t)pSoldier->p8BPPPalette[uiCount].peRed + (uint16_t)gpLightColors[2].peRed, 255));
      LightPal[uiCount].peGreen = (uint8_t)(min(
          (uint16_t)pSoldier->p8BPPPalette[uiCount].peGreen + (uint16_t)gpLightColors[2].peGreen,
          255));
      LightPal[uiCount].peBlue = (uint8_t)(min(
          (uint16_t)pSoldier->p8BPPPalette[uiCount].peBlue + (uint16_t)gpLightColors[2].peBlue,
          255));
    }
    CreateSoldierShadedPalette(pSoldier, 32, LightPal);
  }

  // build neutral palette as well!
  // Set current shade table to neutral color
  pSoldier->pCurrentShade = pSoldier->pShades[4];
  // pSoldier->pGlow=pSoldier->pShades[0];

  return (TRUE);
}
