#ifndef __ISOMETRIC_UTILSH
#define __ISOMETRIC_UTILSH

#include "SGP/Types.h"
#include "Tactical/OverheadTypes.h"
#include "TileEngine/WorldDef.h"

struct SOLDIERTYPE;

// DEFINES
#define MAXCOL WORLD_COLS
#define MAXROW WORLD_ROWS
#define GRIDSIZE (MAXCOL * MAXROW)
#define RIGHTMOSTGRID (MAXCOL - 1)
#define LASTROWSTART (GRIDSIZE - MAXCOL)
#define NOWHERE (GRIDSIZE + 1)
#define NO_MAP_POS NOWHERE
#define MAPWIDTH (WORLD_COLS)
#define MAPHEIGHT (WORLD_ROWS)
#define MAPLENGTH (MAPHEIGHT * MAPWIDTH)

#define ADJUST_Y_FOR_HEIGHT(pos, y) (y -= gpWorldLevelData[pos].sHeight)

extern uint8_t gOppositeDirection[NUM_WORLD_DIRECTIONS];
extern uint8_t gTwoCCDirection[NUM_WORLD_DIRECTIONS];
extern uint8_t gTwoCDirection[NUM_WORLD_DIRECTIONS];
extern uint8_t gOneCDirection[NUM_WORLD_DIRECTIONS];
extern uint8_t gOneCCDirection[NUM_WORLD_DIRECTIONS];

extern uint8_t gPurpendicularDirection[NUM_WORLD_DIRECTIONS][NUM_WORLD_DIRECTIONS];

// Macros

//                                                |Check for map
//                                                bounds------------------------------------------|
//                                                |Invalid-|   |Valid-------------------|
#define MAPROWCOLTOPOS(r, c)                                                 \
  (((r < 0) || (r >= WORLD_ROWS) || (c < 0) || (c >= WORLD_COLS)) ? (0xffff) \
                                                                  : ((r) * WORLD_COLS + (c)))

#define GETWORLDINDEXFROMWORLDCOORDS(r, c) \
  ((int16_t)(r / CELL_X_SIZE)) * WORLD_COLS + ((int16_t)(c / CELL_Y_SIZE))

void ConvertGridNoToXY(int16_t sGridNo, int16_t *sXPos, int16_t *sYPos);
void ConvertGridNoToCellXY(int16_t sGridNo, int16_t *sXPos, int16_t *sYPos);
void ConvertGridNoToCenterCellXY(int16_t sGridNo, int16_t *sXPos, int16_t *sYPos);

// GRID NO MANIPULATION FUNCTIONS
int16_t NewGridNo(int16_t sGridno, int16_t sDirInc);
int16_t DirectionInc(int16_t sDirection);
int32_t OutOfBounds(int16_t sGridno, int16_t sProposedGridno);

// Functions
BOOLEAN GetMouseCell(int32_t *piMouseMapPos);
BOOLEAN GetMouseXY(int16_t *psMouseX, int16_t *psMouseY);
BOOLEAN GetMouseWorldCoords(int16_t *psMouseX, int16_t *psMouseY);
BOOLEAN GetMouseMapPos(int16_t *psMapPos);
BOOLEAN GetMouseWorldCoordsInCenter(int16_t *psMouseX, int16_t *psMouseY);
BOOLEAN GetMouseXYWithRemainder(int16_t *psMouseX, int16_t *psMouseY, int16_t *psCellX,
                                int16_t *psCellY);

void GetScreenXYWorldCoords(int16_t sScreenX, int16_t sScreenY, int16_t *pWorldX,
                            int16_t *psWorldY);
void GetScreenXYWorldCell(int16_t sScreenX, int16_t sScreenY, int16_t *psWorldCellX,
                          int16_t *psWorldCellY);
void GetScreenXYGridNo(int16_t sScreenX, int16_t sScreenY, int16_t *psMapPos);
void GetWorldXYAbsoluteScreenXY(int32_t sWorldCellX, int32_t sWorldCellY, int16_t *psWorldScreenX,
                                int16_t *psWorldScreenY);
void GetFromAbsoluteScreenXYWorldXY(int32_t *psWorldCellX, int32_t *psWorldCellY,
                                    int16_t sWorldScreenX, int16_t sWorldScreenY);

void FromCellToScreenCoordinates(int16_t sCellX, int16_t sCellY, int16_t *psScreenX,
                                 int16_t *psScreenY);
void FromScreenToCellCoordinates(int16_t sScreenX, int16_t sScreenY, int16_t *psCellX,
                                 int16_t *psCellY);

// Higher resolution convertion functions
void FloatFromCellToScreenCoordinates(float dCellX, float dCellY, float *pdScreenX,
                                      float *pdScreenY);
void FloatFromScreenToCellCoordinates(float dScreenX, float dScreenY, float *pdCellX,
                                      float *pdCellY);

BOOLEAN GridNoOnVisibleWorldTile(int16_t sGridNo);
BOOLEAN GridNoOnVisibleWorldTileGivenYLimits(int16_t sGridNo);
BOOLEAN GridNoOnEdgeOfMap(int16_t sGridNo, int8_t *pbDirection);

BOOLEAN ConvertMapPosToWorldTileCenter(uint16_t usMapPos, int16_t *psXPos, int16_t *psYPos);

BOOLEAN CellXYToScreenXY(int16_t sCellX, int16_t sCellY, int16_t *sScreenX, int16_t *sScreenY);

int32_t GetRangeFromGridNoDiff(int16_t sGridNo1, int16_t sGridNo2);
int32_t GetRangeInCellCoordsFromGridNoDiff(int16_t sGridNo1, int16_t sGridNo2);

BOOLEAN IsPointInScreenRect(int16_t sXPos, int16_t sYPos, SGPRect *pRect);
BOOLEAN IsPointInScreenRectWithRelative(int16_t sXPos, int16_t sYPos, SGPRect *pRect,
                                        int16_t *sXRel, int16_t *sRelY);

int16_t PythSpacesAway(int16_t sOrigin, int16_t sDest);
int16_t SpacesAway(int16_t sOrigin, int16_t sDest);
int16_t CardinalSpacesAway(int16_t sOrigin, int16_t sDest);
int8_t FindNumTurnsBetweenDirs(int8_t sDir1, int8_t sDir2);
BOOLEAN FindHeigherLevel(struct SOLDIERTYPE *pSoldier, int16_t sGridNo, int8_t bStartingDir,
                         int8_t *pbDirection);
BOOLEAN FindLowerLevel(struct SOLDIERTYPE *pSoldier, int16_t sGridNo, int8_t bStartingDir,
                       int8_t *pbDirection);

int16_t QuickestDirection(int16_t origin, int16_t dest);
int16_t ExtQuickestDirection(int16_t origin, int16_t dest);

// Returns the (center ) cell coordinates in X
int16_t CenterX(int16_t sGridno);

// Returns the (center ) cell coordinates in Y
int16_t CenterY(int16_t sGridno);

int16_t MapX(int16_t sGridNo);
int16_t MapY(int16_t sGridNo);
BOOLEAN FindFenceJumpDirection(struct SOLDIERTYPE *pSoldier, int16_t sGridNo, int8_t bStartingDir,
                               int8_t *pbDirection);

// Simply chooses a random gridno within valid boundaries (for dropping things in unloaded sectors)
int16_t RandomGridNo();

extern uint32_t guiForceRefreshMousePositionCalculation;

#endif
