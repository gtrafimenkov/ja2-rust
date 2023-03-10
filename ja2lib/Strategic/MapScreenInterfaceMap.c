#include "Strategic/MapScreenInterfaceMap.h"

#include "Laptop/Finances.h"
#include "SGP/ButtonSystem.h"
#include "SGP/Debug.h"
#include "SGP/English.h"
#include "SGP/Font.h"
#include "SGP/Line.h"
#include "SGP/VObject.h"
#include "SGP/VObjectBlitters.h"
#include "SGP/VSurface.h"
#include "SGP/Video.h"
#include "SGP/WCheck.h"
#include "Soldier.h"
#include "Strategic/Assignments.h"
#include "Strategic/CampaignTypes.h"
#include "Strategic/GameClock.h"
#include "Strategic/MapScreen.h"
#include "Strategic/MapScreenHelicopter.h"
#include "Strategic/MapScreenInterface.h"
#include "Strategic/MapScreenInterfaceBorder.h"
#include "Strategic/PlayerCommand.h"
#include "Strategic/PreBattleInterface.h"
#include "Strategic/QueenCommand.h"
#include "Strategic/Strategic.h"
#include "Strategic/StrategicMap.h"
#include "Strategic/StrategicMines.h"
#include "Strategic/StrategicMovement.h"
#include "Strategic/StrategicPathing.h"
#include "Strategic/StrategicTownLoyalty.h"
#include "Strategic/TownMilitia.h"
#include "Tactical.h"
#include "Tactical/AirRaid.h"
#include "Tactical/MapInformation.h"
#include "Tactical/Overhead.h"
#include "Tactical/SoldierProfile.h"
#include "Tactical/Squads.h"
#include "Tactical/TacticalSave.h"
#include "Tactical/Vehicles.h"
#include "TileEngine/RenderDirty.h"
#include "TileEngine/RenderWorld.h"
#include "TileEngine/SysUtil.h"
#include "Town.h"
#include "Utils/FontControl.h"
#include "Utils/Message.h"
#include "Utils/Text.h"
#include "Utils/Utilities.h"
#include "rust_sam_sites.h"

// zoom x and y coords for map scrolling
int32_t iZoomX = 0;
int32_t iZoomY = 0;

// Scroll region width
#define SCROLL_REGION 4

// The Map/Mouse Scroll defines
#define EAST_DIR 0
#define WEST_DIR 1
#define NORTH_DIR 2
#define SOUTH_DIR 3
#define TOP_NORTH 2
#define TOP_SOUTH 13
#define RIGHT_WEST 250
#define RIGHT_EAST 260
#define LEFT_EAST 640
#define LEFT_WEST 630
#define BOTTOM_NORTH 320
#define BOTTOM_SOUTH 330

// Map Scroll Defines
#define SCROLL_EAST 0
#define SCROLL_WEST 1
#define SCROLL_NORTH 2
#define SCROLL_SOUTH 3
#define SCROLL_DELAY 50
#define HORT_SCROLL 14
#define VERT_SCROLL 10

// the pop up for helicopter stuff
#define MAP_HELICOPTER_ETA_POPUP_X 400
#define MAP_HELICOPTER_ETA_POPUP_Y 250
#define MAP_HELICOPTER_UPPER_ETA_POPUP_Y 50
#define MAP_HELICOPTER_ETA_POPUP_WIDTH 120
#define MAP_HELICOPTER_ETA_POPUP_HEIGHT 68

#define MAP_LEVEL_STRING_X 432
#define MAP_LEVEL_STRING_Y 305

// font
#define MAP_FONT BLOCKFONT2

// index color
#define MAP_INDEX_COLOR 32 * 4 - 9

// max number of sectors viewable
#define MAX_VIEW_SECTORS 16

// Map Location index regions

// x start of hort index
#define MAP_HORT_INDEX_X 292

// y position of hort index
#define MAP_HORT_INDEX_Y 10

// height of hort index
#define MAP_HORT_HEIGHT GetFontHeight(MAP_FONT)

// vert index start x
#define MAP_VERT_INDEX_X 273

// vert index start y
#define MAP_VERT_INDEX_Y 31

// vert width
#define MAP_VERT_WIDTH GetFontHeight(MAP_FONT)

// "Boxes" Icons
#define SMALL_YELLOW_BOX 0
#define BIG_YELLOW_BOX 1
#define SMALL_DULL_YELLOW_BOX 2
#define BIG_DULL_YELLOW_BOX 3
#define SMALL_WHITE_BOX 4
#define BIG_WHITE_BOX 5
#define SMALL_RED_BOX 6
#define BIG_RED_BOX 7
#define SMALL_QUESTION_MARK 8
#define BIG_QUESTION_MARK 9

#define MERC_ICONS_PER_LINE 6
#define ROWS_PER_SECTOR 5

#define MAP_X_ICON_OFFSET 2
#define MAP_Y_ICON_OFFSET 1

// Arrow Offsets
#define UP_X 13
#define UP_Y 7
#define DOWN_X 0
#define DOWN_Y -2
#define RIGHT_X -2
#define RIGHT_Y 11
#define LEFT_X 2
#define LEFT_Y 5

// The Path Lines
#define NORTH_LINE 1
#define SOUTH_LINE 0
#define WEST_LINE 3
#define EAST_LINE 2
#define N_TO_E_LINE 4
#define E_TO_S_LINE 5
#define W_TO_N_LINE 6
#define S_TO_W_LINE 7
#define W_TO_S_LINE 8
#define N_TO_W_LINE 9
#define S_TO_E_LINE 10
#define E_TO_N_LINE 11
#define W_TO_E_LINE 12
#define N_TO_S_LINE 13
#define E_TO_W_LINE 14
#define S_TO_N_LINE 15
#define W_TO_E_PART1_LINE 16
#define W_TO_E_PART2_LINE 17
#define E_TO_W_PART1_LINE 18
#define E_TO_W_PART2_LINE 19
#define N_TO_S_PART1_LINE 20
#define N_TO_S_PART2_LINE 21
#define S_TO_N_PART1_LINE 22
#define S_TO_N_PART2_LINE 23
#define GREEN_X_WEST 36
#define GREEN_X_EAST 37
#define GREEN_X_NORTH 38
#define GREEN_X_SOUTH 39
#define RED_X_WEST 40
#define RED_X_EAST 41
#define RED_X_NORTH 42
#define RED_X_SOUTH 43

// The arrows
#define Y_NORTH_ARROW 24
#define Y_SOUTH_ARROW 25
#define Y_EAST_ARROW 26
#define Y_WEST_ARROW 27
#define W_NORTH_ARROW 28
#define W_SOUTH_ARROW 29
#define W_EAST_ARROW 30
#define W_WEST_ARROW 31
#define NORTH_ARROW 32
#define SOUTH_ARROW 33
#define EAST_ARROW 34
#define WEST_ARROW 35

#define ZOOM_Y_NORTH_ARROW 68
#define ZOOM_Y_SOUTH_ARROW 69
#define ZOOM_Y_EAST_ARROW 70
#define ZOOM_Y_WEST_ARROW 71
#define ZOOM_W_NORTH_ARROW 72
#define ZOOM_W_SOUTH_ARROW 73
#define ZOOM_W_EAST_ARROW 74
#define ZOOM_W_WEST_ARROW 75
#define ZOOM_NORTH_ARROW 76
#define ZOOM_SOUTH_ARROW 77
#define ZOOM_EAST_ARROW 78
#define ZOOM_WEST_ARROW 79
#define ARROW_DELAY 20
#define PAUSE_DELAY 1000

// The zoomed in path lines
#define SOUTH_ZOOM_LINE 44
#define NORTH_ZOOM_LINE 45
#define EAST_ZOOM_LINE 46
#define WEST_ZOOM_LINE 47
#define N_TO_E_ZOOM_LINE 48
#define E_TO_S_ZOOM_LINE 49
#define W_TO_N_ZOOM_LINE 50
#define S_TO_W_ZOOM_LINE 51
#define W_TO_S_ZOOM_LINE 52
#define N_TO_W_ZOOM_LINE 53
#define S_TO_E_ZOOM_LINE 54
#define E_TO_N_ZOOM_LINE 55
#define W_TO_E_ZOOM_LINE 56
#define N_TO_S_ZOOM_LINE 57
#define E_TO_W_ZOOM_LINE 58
#define S_TO_N_ZOOM_LINE 59
#define ZOOM_GREEN_X_WEST 80
#define ZOOM_GREEN_X_EAST 81
#define ZOOM_GREEN_X_NORTH 82
#define ZOOM_GREEN_X_SOUTH 83
#define ZOOM_RED_X_WEST 84
#define ZOOM_RED_X_EAST 85
#define ZOOM_RED_X_NORTH 86
#define ZOOM_RED_X_SOUTH 87

#define CHAR_FONT_COLOR 32 * 4 - 9

// Arrow Offsets
#define EAST_OFFSET_X 11
#define EAST_OFFSET_Y 0
#define NORTH_OFFSET_X 9
#define NORTH_OFFSET_Y -9
#define SOUTH_OFFSET_X -9
#define SOUTH_OFFSET_Y 9
#define WEST_OFFSET_X -11
#define WEST_OFFSET_Y 0
#define WEST_TO_SOUTH_OFFSET_Y 0
#define EAST_TO_NORTH_OFFSET_Y 0
#define RED_WEST_OFF_X -MAP_GRID_X
#define RED_EAST_OFF_X MAP_GRID_X
#define RED_NORTH_OFF_Y -21
#define RED_SOUTH_OFF_Y 21

// the font use on the mvt icons for mapscreen
#define MAP_MVT_ICON_FONT SMALLCOMPFONT

// map shading colors

enum {
  MAP_SHADE_BLACK = 0,
  MAP_SHADE_LT_GREEN,
  MAP_SHADE_DK_GREEN,
  MAP_SHADE_LT_RED,
  MAP_SHADE_DK_RED,
};
// the big map .pcx
uint32_t guiBIGMAP;

// orta .sti icon
uint32_t guiORTAICON;
uint32_t guiTIXAICON;

// boxes for characters on the map
uint32_t guiCHARICONS;

// the merc arrival sector landing zone icon
uint32_t guiBULLSEYE;

// the max allowable towns militia in a sector
#define MAP_MILITIA_MAP_X 4
#define MAP_MILITIA_MAP_Y 20
#define MAP_MILITIA_LOWER_ROW_Y 142
#define NUMBER_OF_MILITIA_ICONS_PER_LOWER_ROW 25
#define MILITIA_BOX_ROWS 3
#define MILITIA_BOX_BOX_HEIGHT 36
#define MILITIA_BOX_BOX_WIDTH 42
#define MAP_MILITIA_BOX_POS_X 400
#define MAP_MILITIA_BOX_POS_Y 125

#define POPUP_MILITIA_ICONS_PER_ROW 5  // max 6 rows gives the limit of 30 militia
#define MEDIUM_MILITIA_ICON_SPACING 5
#define LARGE_MILITIA_ICON_SPACING 6

#define MILITIA_BTN_OFFSET_X 26
#define MILITIA_BTN_HEIGHT 11
#define MILITIA_BOX_WIDTH 133
#define MILITIA_BOX_TEXT_OFFSET_Y 4
#define MILITIA_BOX_UNASSIGNED_TEXT_OFFSET_Y 132
#define MILITIA_BOX_TEXT_TITLE_HEIGHT 13

#define MAP_MILITIA_BOX_AUTO_BOX_X 4
#define MAP_MILITIA_BOX_AUTO_BOX_Y 167
#define MAP_MILITIA_BOX_DONE_BOX_X 67

#define HELI_ICON 0
#define HELI_SHADOW_ICON 1

#define HELI_ICON_WIDTH 20
#define HELI_ICON_HEIGHT 10
#define HELI_SHADOW_ICON_WIDTH 19
#define HELI_SHADOW_ICON_HEIGHT 11

// the militia box buttons and images
int32_t giMapMilitiaButtonImage[5];
int32_t giMapMilitiaButton[5] = {-1, -1, -1, -1, -1};

int16_t gsMilitiaSectorButtonColors[] = {
    FONT_LTGREEN,
    FONT_LTBLUE,
    16,
};

// track number of townspeople picked up
int16_t sGreensOnCursor = 0;
int16_t sRegularsOnCursor = 0;
int16_t sElitesOnCursor = 0;

// the current militia town id
int16_t sSelectedMilitiaTown = 0;

extern MINE_LOCATION_TYPE gMineLocation[];
extern MINE_STATUS_TYPE gMineStatus[];

// sublevel graphics
uint32_t guiSubLevel1, guiSubLevel2, guiSubLevel3;

// the between sector icons
uint32_t guiCHARBETWEENSECTORICONS;
uint32_t guiCHARBETWEENSECTORICONSCLOSE;

extern BOOLEAN fMapScreenBottomDirty;

// tixa found
BOOLEAN fFoundTixa = FALSE;

// selected sector
uint8_t sSelMapX = 9;
uint8_t sSelMapY = 1;

// highlighted sector
uint8_t gsHighlightSectorX = 0xff;
uint8_t gsHighlightSectorY = 0xff;

// the current sector Z value of the map being displayed
int32_t iCurrentMapSectorZ = 0;

// the palettes
uint16_t *pMapLTRedPalette;
uint16_t *pMapDKRedPalette;
uint16_t *pMapLTGreenPalette;
uint16_t *pMapDKGreenPalette;

// the map border eta pop up
uint32_t guiMapBorderEtaPopUp;

// heli pop up
uint32_t guiMapBorderHeliSectors;

// list of map sectors that player isn't allowed to even highlight
BOOLEAN sBadSectorsList[MAP_WORLD_Y][MAP_WORLD_X];

uint8_t sBaseSectorList[] = {
    // NOTE: These co-ordinates must match the top left corner of the 3x3 town tiles cutouts in
    // Interface/MilitiaMaps.sti!
    GetSectorID8_STATIC(9, 1),    // Omerta
    GetSectorID8_STATIC(13, 2),   // Drassen
    GetSectorID8_STATIC(13, 8),   // Alma
    GetSectorID8_STATIC(1, 7),    // Grumm
    GetSectorID8_STATIC(8, 9),    // Tixa
    GetSectorID8_STATIC(8, 6),    // Cambria
    GetSectorID8_STATIC(4, 2),    // San Mona
    GetSectorID8_STATIC(5, 8),    // Estoni
    GetSectorID8_STATIC(3, 10),   // Orta
    GetSectorID8_STATIC(11, 11),  // Balime
    GetSectorID8_STATIC(3, 14),   // Meduna
    GetSectorID8_STATIC(2, 1),    // Chitzena
};

// position of town names on the map
// these are no longer PIXELS, but 10 * the X,Y position in SECTORS (fractions possible) to the
// X-CENTER of the town
struct Point pTownPoints[] = {
    {0, 0},     {90, 10},  // Omerta
    {125, 40},             // Drassen
    {130, 90},             // Alma
    {15, 80},              // Grumm
    {85, 100},             // Tixa
    {95, 70},              // Cambria
    {45, 40},              // San Mona
    {55, 90},              // Estoni
    {35, 110},             // Orta
    {110, 120},            // Balime
    {45, 150},             // Meduna
    {15, 20},              // Chitzena
};

// map region
SGPRect MapScreenRect = {(MAP_VIEW_START_X + MAP_GRID_X - 2), (MAP_VIEW_START_Y + MAP_GRID_Y - 1),
                         MAP_VIEW_START_X + MAP_VIEW_WIDTH - 1 + MAP_GRID_X,
                         MAP_VIEW_START_Y + MAP_VIEW_HEIGHT - 10 + MAP_GRID_Y};

// screen region
SGPRect FullScreenRect = {0, 0, 640, 480};

// temp helicopter path
struct path *pTempHelicopterPath = NULL;

// character temp path
struct path *pTempCharacterPath = NULL;

// draw temp path?
BOOLEAN fDrawTempHeliPath = FALSE;

// the map arrows graphics
uint32_t guiMAPCURSORS;

// destination plotting character
int8_t bSelectedDestChar = -1;

// assignment selection character
int8_t bSelectedAssignChar = -1;

// current contract char
int8_t bSelectedContractChar = -1;

// has the temp path for character or helicopter been already drawn?
BOOLEAN fTempPathAlreadyDrawn = FALSE;

// the regions for the mapscreen militia box
struct MOUSE_REGION gMapScreenMilitiaBoxRegions[9];
struct MOUSE_REGION gMapScreenMilitiaRegion;

// the mine icon
uint32_t guiMINEICON;

// militia graphics
uint32_t guiMilitia;
uint32_t guiMilitiaMaps;
uint32_t guiMilitiaSectorHighLight;
uint32_t guiMilitiaSectorOutline;

// the sector that is highlighted on the militia map
int16_t sSectorMilitiaMapSector = -1;
BOOLEAN fMilitiaMapButtonsCreated = FALSE;
int16_t sSectorMilitiaMapSectorOutline = -1;

// have any nodes in the current path list been deleted?
BOOLEAN fDeletedNode = FALSE;

uint16_t gusUndergroundNearBlack;

BOOLEAN gfMilitiaPopupCreated = FALSE;

int32_t giAnimateRouteBaseTime = 0;
int32_t giPotHeliPathBaseTime = 0;
int32_t giClickHeliIconBaseTime = 0;

// display the level string on the strategic map
void DisplayLevelString(void);

void ShowTownText(void);
void DrawTownLabels(wchar_t *pString, wchar_t *pStringA, uint16_t usFirstX, uint16_t usFirstY);
void ShowTeamAndVehicles(int32_t fShowFlags);
BOOLEAN ShadeMapElem(uint8_t sMapX, uint8_t sMapY, int32_t iColor);
BOOLEAN ShadeMapElemZoomIn(uint8_t sMapX, uint8_t sMapY, int32_t iColor);
void AdjustXForLeftMapEdge(wchar_t *wString, int16_t *psX);
static void BlitTownGridMarkers(void);
void BlitMineGridMarkers(void);
void BlitSAMGridMarkers(void);
void BlitMineIcon(uint8_t sMapX, uint8_t sMapY);
void BlitMineText(uint8_t sMapX, uint8_t sMapY);
uint8_t GetBaseSectorForCurrentTown(void);
void RenderIconsPerSectorForSelectedTown(void);
void MilitiaRegionClickCallback(struct MOUSE_REGION *pRegion, int32_t iReason);
void MilitiaRegionMoveCallback(struct MOUSE_REGION *pRegion, int32_t iReason);
void CreateDestroyMilitiaSectorButtons(void);
void ShowHighLightedSectorOnMilitiaMap(void);
void SetMilitiaMapButtonsText(void);
void MilitiaButtonCallback(GUI_BUTTON *btn, int32_t reason);
void DisplayUnallocatedMilitia(void);
BOOLEAN IsThisMilitiaTownSectorAllowable(int16_t sSectorIndexValue);
void DrawTownMilitiaName(void);
void HandleShutDownOfMilitiaPanelIfPeopleOnTheCursor(int16_t sTownValue);
void HandleEveningOutOfTroopsAmongstSectors(void);
void CreateMilitiaPanelBottomButton(void);
void DeleteMilitiaPanelBottomButton(void);
void MilitiaDoneButtonCallback(GUI_BUTTON *btn, int32_t reason);
void MilitiaAutoButtonCallback(GUI_BUTTON *btn, int32_t reason);
void RenderShadingForUnControlledSectors(void);
void DrawTownMilitiaForcesOnMap(void);
void CheckAndUpdateStatesOfSelectedMilitiaSectorButtons(void);
void HandleLowerLevelMapBlit(void);
void ShowSAMSitesOnStrategicMap(void);

// uint8_t NumActiveCharactersInSector( uint8_t sSectorX, uint8_t sSectorY, int16_t bSectorZ );
// uint8_t NumFriendlyInSector( int16_t sX, int16_t sY, int8_t bZ );

// callbacks
void MilitiaBoxMaskBtnCallback(struct MOUSE_REGION *pRegion, int32_t iReason);

// display potential path, yes or no?
void DisplayThePotentialPathForHelicopter(uint8_t sMapX, uint8_t sMapY);
void ShowEnemiesInSector(uint8_t sSectorX, uint8_t sSectorY, int16_t sNumberOfEnemies,
                         uint8_t ubIconPosition);
void ShowUncertainNumberEnemiesInSector(uint8_t sSectorX, uint8_t sSectorY);
void HandleShowingOfEnemyForcesInSector(uint8_t sSectorX, uint8_t sSectorY, int8_t bSectorZ,
                                        uint8_t ubIconPosition);

BOOLEAN CanMilitiaAutoDistribute(void);

void ShowItemsOnMap(void);
void DrawMapBoxIcon(struct VObject *hIconHandle, uint16_t usVOIndex, uint8_t sMapX, uint8_t sMapY,
                    uint8_t ubIconPosition);
void DisplayDestinationOfHelicopter(void);
void DrawOrta();
void DrawTixa();
void DrawBullseye();
void HideExistenceOfUndergroundMapSector(uint8_t ubSectorX, uint8_t ubSectorY);

BOOLEAN CanMercsScoutThisSector(uint8_t sSectorX, uint8_t sSectorY, int8_t bSectorZ);

BOOLEAN TraceCharAnimatedRoute(struct path *pPath, BOOLEAN fCheckFlag, BOOLEAN fForceUpDate);
void AnimateRoute(struct path *pPath);

extern void EndConfirmMapMoveMode(void);
extern BOOLEAN CanDrawSectorCursor(void);

void DrawMapIndexBigMap(BOOLEAN fSelectedCursorIsYellow) {
  // this procedure will draw the coord indexes on the zoomed out map
  int16_t usX, usY;
  int32_t iCount = 0;
  BOOLEAN fDrawCursors;

  SetFontDestBuffer(FRAME_BUFFER, 0, 0, 640, 480, FALSE);
  // SetFontColors(FONT_FCOLOR_GREEN)
  SetFont(MAP_FONT);
  SetFontForeground(MAP_INDEX_COLOR);
  // Dk Red is 163
  SetFontBackground(FONT_MCOLOR_BLACK);

  fDrawCursors = CanDrawSectorCursor();

  for (iCount = 1; iCount <= MAX_VIEW_SECTORS; iCount++) {
    if (fDrawCursors && (iCount == sSelMapX) && (bSelectedDestChar == -1) &&
        (fPlotForHelicopter == FALSE))
      SetFontForeground((uint8_t)(fSelectedCursorIsYellow ? FONT_YELLOW : FONT_WHITE));
    else if (fDrawCursors && (iCount == gsHighlightSectorX))
      SetFontForeground(FONT_WHITE);
    else
      SetFontForeground(MAP_INDEX_COLOR);

    FindFontCenterCoordinates(((int16_t)(MAP_HORT_INDEX_X + (iCount - 1) * MAP_GRID_X)),
                              MAP_HORT_INDEX_Y, MAP_GRID_X, MAP_HORT_HEIGHT, pMapHortIndex[iCount],
                              MAP_FONT, &usX, &usY);
    mprintf(usX, usY, pMapHortIndex[iCount]);

    if (fDrawCursors && (iCount == sSelMapY) && (bSelectedDestChar == -1) &&
        (fPlotForHelicopter == FALSE))
      SetFontForeground((uint8_t)(fSelectedCursorIsYellow ? FONT_YELLOW : FONT_WHITE));
    else if (fDrawCursors && (iCount == gsHighlightSectorY))
      SetFontForeground(FONT_WHITE);
    else
      SetFontForeground(MAP_INDEX_COLOR);

    FindFontCenterCoordinates(
        MAP_VERT_INDEX_X, ((int16_t)(MAP_VERT_INDEX_Y + (iCount - 1) * MAP_GRID_Y)),
        MAP_HORT_HEIGHT, MAP_GRID_Y, pMapVertIndex[iCount], MAP_FONT, &usX, &usY);
    mprintf(usX, usY, pMapVertIndex[iCount]);
  }

  InvalidateRegion(MAP_VERT_INDEX_X, MAP_VERT_INDEX_Y, MAP_VERT_INDEX_X + MAP_HORT_HEIGHT,
                   MAP_VERT_INDEX_Y + (iCount - 1) * MAP_GRID_Y);
  InvalidateRegion(MAP_HORT_INDEX_X, MAP_HORT_INDEX_Y, MAP_HORT_INDEX_X + (iCount - 1) * MAP_GRID_X,
                   MAP_HORT_INDEX_Y + MAP_HORT_HEIGHT);

  SetFontDestBuffer(FRAME_BUFFER, 0, 0, 640, 480, FALSE);
}

void HandleShowingOfEnemiesWithMilitiaOn(void) {
  uint8_t sX = 0, sY = 0;

  // if show militia flag is false, leave
  if (!fShowMilitia) {
    return;
  }

  for (sX = 1; sX < (MAP_WORLD_X - 1); sX++) {
    for (sY = 1; sY < (MAP_WORLD_Y - 1); sY++) {
      HandleShowingOfEnemyForcesInSector(sX, sY, (int8_t)iCurrentMapSectorZ,
                                         CountAllMilitiaInSector(sX, sY));
    }
  }

  return;
}

uint32_t DrawMap(void) {
  struct VSurface *hSrcVSurface;
  uint32_t uiDestPitchBYTES;
  uint32_t uiSrcPitchBYTES;
  uint16_t *pDestBuf;
  uint8_t *pSrcBuf;
  SGPRect clip;
  uint8_t cnt, cnt2;
  int32_t iCounter = 0;

  if (!iCurrentMapSectorZ) {
    pDestBuf = (uint16_t *)LockVideoSurface(guiSAVEBUFFER, &uiDestPitchBYTES);

    CHECKF(GetVideoSurface(&hSrcVSurface, guiBIGMAP));
    pSrcBuf = LockVideoSurface(guiBIGMAP, &uiSrcPitchBYTES);

    // clip blits to mapscreen region
    // ClipBlitsToMapViewRegion( );

    if (fZoomFlag) {
      // set up bounds
      if (iZoomX < WEST_ZOOM_BOUND) iZoomX = WEST_ZOOM_BOUND;
      if (iZoomX > EAST_ZOOM_BOUND) iZoomX = EAST_ZOOM_BOUND;
      if (iZoomY < NORTH_ZOOM_BOUND + 1) iZoomY = NORTH_ZOOM_BOUND;
      if (iZoomY > SOUTH_ZOOM_BOUND) iZoomY = SOUTH_ZOOM_BOUND;

      clip.iLeft = iZoomX - 2;
      clip.iRight = clip.iLeft + MAP_VIEW_WIDTH + 2;
      clip.iTop = iZoomY - 3;
      clip.iBottom = clip.iTop + MAP_VIEW_HEIGHT - 1;

      if (clip.iBottom > hSrcVSurface->usHeight) {
        clip.iBottom = hSrcVSurface->usHeight;
      }

      if (clip.iRight > hSrcVSurface->usWidth) {
        clip.iRight = hSrcVSurface->usWidth;
      }

      Blt8BPPDataSubTo16BPPBuffer(pDestBuf, uiDestPitchBYTES, hSrcVSurface, pSrcBuf,
                                  uiSrcPitchBYTES, MAP_VIEW_START_X + MAP_GRID_X,
                                  MAP_VIEW_START_Y + MAP_GRID_Y - 2, &clip);
    } else {
      Blt8BPPDataTo16BPPBufferHalf(pDestBuf, uiDestPitchBYTES, hSrcVSurface, pSrcBuf,
                                   uiSrcPitchBYTES, MAP_VIEW_START_X + 1, MAP_VIEW_START_Y);
    }

    UnLockVideoSurface(guiBIGMAP);
    UnLockVideoSurface(guiSAVEBUFFER);

    // shade map sectors (must be done after Tixa/Orta/Mine icons have been blitted, but before
    // icons!)
    for (cnt = 1; cnt < MAP_WORLD_X - 1; cnt++) {
      for (cnt2 = 1; cnt2 < MAP_WORLD_Y - 1; cnt2++) {
        // LATE DESIGN CHANGE: darken sectors not yet visited, instead of those under known enemy
        // control
        if (GetSectorFlagStatus(cnt, cnt2, (uint8_t)iCurrentMapSectorZ, SF_ALREADY_VISITED) ==
            FALSE) {
          if (fShowAircraftFlag && !iCurrentMapSectorZ) {
            if (!IsSectorEnemyAirControlled(cnt, cnt2)) {
              // sector not visited, not air controlled
              ShadeMapElem(cnt, cnt2, MAP_SHADE_DK_GREEN);
            } else {
              // sector not visited, controlled and air not
              ShadeMapElem(cnt, cnt2, MAP_SHADE_DK_RED);
            }
          } else {
            // not visited
            ShadeMapElem(cnt, cnt2, MAP_SHADE_BLACK);
          }
        } else {
          if (fShowAircraftFlag && !iCurrentMapSectorZ) {
            if (!IsSectorEnemyAirControlled(cnt, cnt2)) {
              // sector visited and air controlled
              ShadeMapElem(cnt, cnt2, MAP_SHADE_LT_GREEN);
            } else {
              // sector visited but not air controlled
              ShadeMapElem(cnt, cnt2, MAP_SHADE_LT_RED);
            }
          }
        }
      }
    }

    // UNFORTUNATELY, WE CAN'T SHADE THESE ICONS AS PART OF SHADING THE MAP, BECAUSE FOR AIRSPACE,
    // THE SHADE FUNCTION DOESN'T MERELY SHADE THE EXISTING MAP SURFACE, BUT INSTEAD GRABS THE
    // ORIGINAL GRAPHICS FROM BIGMAP, AND CHANGES THEIR PALETTE.  BLITTING ICONS PRIOR TO SHADING
    // WOULD MEAN THEY DON'T SHOW UP IN AIRSPACE VIEW AT ALL.

    // if Orta found
    if (fFoundOrta) {
      DrawOrta();
    }

    // if Tixa found
    if (fFoundTixa) {
      DrawTixa();
    }

    // draw SAM sites
    ShowSAMSitesOnStrategicMap();

    // draw mine icons
    for (iCounter = 0; iCounter < MAX_NUMBER_OF_MINES; iCounter++) {
      BlitMineIcon(gMineLocation[iCounter].sSectorX, gMineLocation[iCounter].sSectorY);
    }

    // if mine details filter is set
    if (fShowMineFlag) {
      // show mine name/production text
      for (iCounter = 0; iCounter < MAX_NUMBER_OF_MINES; iCounter++) {
        BlitMineText(gMineLocation[iCounter].sSectorX, gMineLocation[iCounter].sSectorY);
      }
    }

    // draw towns names & loyalty ratings, and grey town limit borders
    if (fShowTownFlag) {
      BlitTownGridMarkers();
      ShowTownText();
    }

    // draw militia icons
    if (fShowMilitia) {
      DrawTownMilitiaForcesOnMap();
    }

    if (fShowAircraftFlag && !gfInChangeArrivalSectorMode) {
      DrawBullseye();
    }
  } else {
    HandleLowerLevelMapBlit();
  }

  // show mine outlines even when viewing underground sublevels - they indicate where the mine
  // entrances are
  if (fShowMineFlag)
    // draw grey mine sector borders
    BlitMineGridMarkers();

  // do not show mercs/vehicles when airspace is ON
  // commented out on a trial basis!
  //	if( !fShowAircraftFlag )
  {
    if (fShowTeamFlag)
      ShowTeamAndVehicles(SHOW_TEAMMATES | SHOW_VEHICLES);
    else
      HandleShowingOfEnemiesWithMilitiaOn();
  }

  if (fShowItemsFlag) {
    ShowItemsOnMap();
  }

  DisplayLevelString();

  return (TRUE);
}

void GetScreenXYFromMapXY(uint8_t sMapX, uint8_t sMapY, int16_t *psX, int16_t *psY) {
  int16_t sXTempOff = 1;
  int16_t sYTempOff = 1;
  if (fZoomFlag) {
    *psX = ((sMapX / 2 + sXTempOff) * MAP_GRID_ZOOM_X) + MAP_VIEW_START_X;
    *psY = ((sMapY / 2 + sYTempOff) * MAP_GRID_ZOOM_Y) + MAP_VIEW_START_Y;
  } else {
    *psX = (sMapX * MAP_GRID_X) + MAP_VIEW_START_X;
    *psY = (sMapY * MAP_GRID_Y) + MAP_VIEW_START_Y;
  }
}

void GetScreenXYFromMapXYStationary(uint8_t sMapX, uint8_t sMapY, int16_t *psX, int16_t *psY) {
  int16_t sXTempOff = 1;
  int16_t sYTempOff = 1;
  //(MAP_VIEW_START_X+((iCount+1)*MAP_GRID_X)*2-iZoomX));
  *psX = ((sMapX + sXTempOff) * MAP_GRID_X) * 2 - ((int16_t)iZoomX) + MAP_VIEW_START_X;
  *psY = ((sMapY + sYTempOff) * MAP_GRID_Y) * 2 - ((int16_t)iZoomY) + MAP_VIEW_START_Y;
}

void ShowTownText(void) {
  wchar_t sString[32];
  wchar_t sStringA[32];
  int8_t bTown = 0;
  uint16_t usX, usY;
  BOOLEAN fLoyaltyTooLowToTrainMilitia;

  // this procedure will display the town names on the screen

  SetFont(MAP_FONT);
  SetFontBackground(FONT_MCOLOR_BLACK);

  for (bTown = FIRST_TOWN; bTown < NUM_TOWNS; bTown++) {
    // skip Orta/Tixa until found
    if (((fFoundOrta != FALSE) || (bTown != ORTA)) && ((bTown != TIXA) || (fFoundTixa != FALSE))) {
      swprintf(sString, ARR_SIZE(sString), L"%s", pTownNames[bTown]);

      fLoyaltyTooLowToTrainMilitia = FALSE;

      // don't show loyalty string until loyalty tracking for that town has been started
      if (gTownLoyalty[bTown].fStarted && gfTownUsesLoyalty[bTown]) {
        swprintf(sStringA, ARR_SIZE(sStringA), L"%d%%%% %s", gTownLoyalty[bTown].ubRating,
                 gsLoyalString[0]);

        // if loyalty is too low to train militia, and militia training is allowed here
        if ((gTownLoyalty[bTown].ubRating < MIN_RATING_TO_TRAIN_TOWN) &&
            MilitiaTrainingAllowedInTown(bTown)) {
          fLoyaltyTooLowToTrainMilitia = TRUE;
        }
      } else {
        wcscpy(sStringA, L"");
      }

      if (!fZoomFlag) {
        usX = (uint16_t)(MAP_VIEW_START_X + MAP_GRID_X + (pTownPoints[bTown].x * MAP_GRID_X) / 10);
        usY = (uint16_t)(MAP_VIEW_START_Y + MAP_GRID_Y +
                         ((pTownPoints[bTown].y * MAP_GRID_Y) / 10) + 1);
      } else {
        usX = (uint16_t)(MAP_VIEW_START_X + MAP_GRID_X + MAP_GRID_ZOOM_X - iZoomX +
                         (pTownPoints[bTown].x * MAP_GRID_ZOOM_X) / 10);
        usY = (uint16_t)(MAP_VIEW_START_Y + MAP_GRID_Y + MAP_GRID_ZOOM_Y - iZoomY +
                         ((pTownPoints[bTown].y * MAP_GRID_ZOOM_Y) / 10) + 1);
        //			usX = 2 * pTownPoints[ bTown  ].x - iZoomX - MAP_VIEW_START_X +
        // MAP_GRID_X; 			usY = 2 * pTownPoints[ bTown  ].y - iZoomY -
        // MAP_VIEW_START_Y
        // + MAP_GRID_Y;
      }

      // red for low loyalty, green otherwise
      SetFontForeground(
          (uint8_t)(fLoyaltyTooLowToTrainMilitia ? FONT_MCOLOR_RED : FONT_MCOLOR_LTGREEN));

      DrawTownLabels(sString, sStringA, usX, usY);
    }
  }
}

void DrawTownLabels(wchar_t *pString, wchar_t *pStringA, uint16_t usFirstX, uint16_t usFirstY) {
  // this procedure will draw the given strings on the screen centered around the given x and at the
  // given y

  int16_t sSecondX, sSecondY;
  int16_t sPastEdge;

  // if within view region...render, else don't
  if ((usFirstX > MAP_VIEW_START_X + MAP_VIEW_WIDTH) || (usFirstX < MAP_VIEW_START_X) ||
      (usFirstY < MAP_VIEW_START_Y) || (usFirstY > MAP_VIEW_START_Y + MAP_VIEW_HEIGHT)) {
    return;
  }

  SetFontDestBuffer(guiSAVEBUFFER, MapScreenRect.iLeft + 2, MapScreenRect.iTop,
                    MapScreenRect.iRight, MapScreenRect.iBottom, FALSE);

  // clip blits to mapscreen region
  ClipBlitsToMapViewRegion();

  // we're CENTERING the first string AROUND usFirstX, so calculate the starting X
  usFirstX -= StringPixLength(pString, MAP_FONT) / 2;

  // print first string
  gprintfdirty(usFirstX, usFirstY, pString);
  mprintf(usFirstX, usFirstY, pString);

  // calculate starting coordinates for the second string
  VarFindFontCenterCoordinates((int16_t)(usFirstX), (int16_t)usFirstY,
                               StringPixLength(pString, MAP_FONT), 0, MAP_FONT, &sSecondX,
                               &sSecondY, pStringA);

  // make sure we don't go past left edge (Grumm)
  if (!fZoomFlag) {
    sPastEdge = (MAP_VIEW_START_X + 23) - sSecondX;

    if (sPastEdge > 0) sSecondX += sPastEdge;
  }

  // print second string beneath first
  sSecondY = (int16_t)(usFirstY + GetFontHeight(MAP_FONT));
  gprintfdirty(sSecondX, sSecondY, pStringA);
  mprintf(sSecondX, sSecondY, pStringA);

  // restore clip blits
  RestoreClipRegionToFullScreen();
}

// "on duty" includes mercs inside vehicles
int32_t ShowOnDutyTeam(uint8_t sMapX, uint8_t sMapY) {
  uint8_t ubCounter = 0, ubIconPosition = 0;
  struct VObject *hIconHandle;
  struct SOLDIERTYPE *pSoldier = NULL;

  GetVideoObject(&hIconHandle, guiCHARICONS);

  // run through list
  while (gCharactersList[ubCounter].fValid) {
    pSoldier = MercPtrs[gCharactersList[ubCounter].usSolID];

    if (!(pSoldier->uiStatusFlags & SOLDIER_VEHICLE) && (GetSolSectorX(pSoldier) == sMapX) &&
        (GetSolSectorY(pSoldier) == sMapY) && (GetSolSectorZ(pSoldier) == iCurrentMapSectorZ) &&
        ((pSoldier->bAssignment < ON_DUTY) || ((GetSolAssignment(pSoldier) == VEHICLE) &&
                                               (pSoldier->iVehicleId != iHelicopterVehicleId))) &&
        IsSolAlive(pSoldier) && (!PlayerIDGroupInMotion(pSoldier->ubGroupID))) {
      DrawMapBoxIcon(hIconHandle, SMALL_YELLOW_BOX, sMapX, sMapY, ubIconPosition);
      ubIconPosition++;
    }

    ubCounter++;
  }
  return ubIconPosition;
}

int32_t ShowAssignedTeam(uint8_t sMapX, uint8_t sMapY, int32_t iCount) {
  uint8_t ubCounter, ubIconPosition;
  struct VObject *hIconHandle;
  struct SOLDIERTYPE *pSoldier = NULL;

  GetVideoObject(&hIconHandle, guiCHARICONS);
  ubCounter = 0;

  // run through list
  ubIconPosition = (uint8_t)iCount;

  while (gCharactersList[ubCounter].fValid) {
    pSoldier = MercPtrs[gCharactersList[ubCounter].usSolID];

    // given number of on duty members, find number of assigned chars
    // start at beginning of list, look for people who are in sector and assigned
    if (!(pSoldier->uiStatusFlags & SOLDIER_VEHICLE) && (GetSolSectorX(pSoldier) == sMapX) &&
        (GetSolSectorY(pSoldier) == sMapY) && (GetSolSectorZ(pSoldier) == iCurrentMapSectorZ) &&
        (pSoldier->bAssignment >= ON_DUTY) && (pSoldier->bAssignment != VEHICLE) &&
        (pSoldier->bAssignment != IN_TRANSIT) && (pSoldier->bAssignment != ASSIGNMENT_POW) &&
        IsSolAlive(pSoldier) && (!PlayerIDGroupInMotion(pSoldier->ubGroupID))) {
      // skip mercs inside the helicopter if we're showing airspace level - they show up inside
      // chopper icon instead
      if (!fShowAircraftFlag || (pSoldier->bAssignment != VEHICLE) ||
          (pSoldier->iVehicleId != iHelicopterVehicleId)) {
        DrawMapBoxIcon(hIconHandle, SMALL_DULL_YELLOW_BOX, sMapX, sMapY, ubIconPosition);
        ubIconPosition++;
      }
    }

    ubCounter++;
  }
  return ubIconPosition;
}

int32_t ShowVehicles(uint8_t sMapX, uint8_t sMapY, int32_t iCount) {
  uint8_t ubCounter, ubIconPosition;
  struct VObject *hIconHandle;
  struct SOLDIERTYPE *pVehicleSoldier;

  GetVideoObject(&hIconHandle, guiCHARICONS);
  ubCounter = 0;

  ubIconPosition = (uint8_t)iCount;

  // run through list of vehicles
  while (ubCounter < ubNumberOfVehicles) {
    // skip the chopper, it has its own icon and displays in airspace mode
    if (ubCounter != iHelicopterVehicleId) {
      if ((pVehicleList[ubCounter].sSectorX == sMapX) &&
          (pVehicleList[ubCounter].sSectorY == sMapY)) {
        // don't show vehicles between sectors (in motion - they're counted as "people in motion"
        if ((pVehicleList[ubCounter].sSectorZ == iCurrentMapSectorZ) &&
            !PlayerIDGroupInMotion(pVehicleList[ubCounter].ubMovementGroup)) {
          // ATE: Check if this vehicle has a soldier and it's on our team.....
          pVehicleSoldier = GetSoldierStructureForVehicle(ubCounter);

          // this skips the chopper, which has no soldier
          if (pVehicleSoldier) {
            if (pVehicleSoldier->bTeam == gbPlayerNum) {
              DrawMapBoxIcon(hIconHandle, SMALL_WHITE_BOX, sMapX, sMapY, ubIconPosition);
              ubIconPosition++;
            }
          }
        }
      }
    }

    ubCounter++;
  }

  return ubIconPosition;
}

void ShowEnemiesInSector(uint8_t sSectorX, uint8_t sSectorY, int16_t sNumberOfEnemies,
                         uint8_t ubIconPosition) {
  struct VObject *hIconHandle;
  uint8_t ubEnemy = 0;

  // get the video object
  GetVideoObject(&hIconHandle, guiCHARICONS);

  for (ubEnemy = 0; ubEnemy < sNumberOfEnemies; ubEnemy++) {
    DrawMapBoxIcon(hIconHandle, SMALL_RED_BOX, sSectorX, sSectorY, ubIconPosition);
    ubIconPosition++;
  }
}

void ShowUncertainNumberEnemiesInSector(uint8_t sSectorX, uint8_t sSectorY) {
  int16_t sXPosition = 0, sYPosition = 0;
  struct VObject *hIconHandle;

  // grab the x and y postions
  sXPosition = sSectorX;
  sYPosition = sSectorY;

  // get the video object
  GetVideoObject(&hIconHandle, guiCHARICONS);

  // check if we are zoomed in...need to offset in case for scrolling purposes
  if (!fZoomFlag) {
    sXPosition =
        (int16_t)(MAP_X_ICON_OFFSET + (MAP_VIEW_START_X + (sSectorX * MAP_GRID_X + 1)) - 1);
    sYPosition = (int16_t)(((MAP_VIEW_START_Y + (sSectorY * MAP_GRID_Y) + 1)));
    sYPosition -= 2;

    // small question mark
    BltVideoObject(guiSAVEBUFFER, hIconHandle, SMALL_QUESTION_MARK, sXPosition, sYPosition,
                   VO_BLT_SRCTRANSPARENCY, NULL);
    InvalidateRegion(sXPosition, sYPosition, sXPosition + DMAP_GRID_X, sYPosition + DMAP_GRID_Y);
  }
  /*
          else
          {
                  int16_t sX = 0, sY = 0;

                  GetScreenXYFromMapXYStationary( sSectorX, sSectorY, &sX, &sY );
                  sYPosition = sY-MAP_GRID_Y;
                  sXPosition = sX-MAP_GRID_X;

                  // get the x and y position
                  sXPosition = MAP_X_ICON_OFFSET + sXPosition ;
                  sYPosition = sYPosition - 1;

                  // clip blits to mapscreen region
                  ClipBlitsToMapViewRegion( );

                  // large question mark
                  BltVideoObject(guiSAVEBUFFER, hIconHandle, BIG_QUESTION_MARK, sXPosition,
     sYPosition, VO_BLT_SRCTRANSPARENCY, NULL );

                  // restore clip blits
                  RestoreClipRegionToFullScreen( );

                  InvalidateRegion( sXPosition, sYPosition, sXPosition + DMAP_GRID_ZOOM_X,
     sYPosition + DMAP_GRID_ZOOM_Y );
          }
  */
}

void ShowTeamAndVehicles(int32_t fShowFlags) {
  // go through each sector, display the on duty, assigned, and vehicles
  uint8_t sMapX = 0;
  uint8_t sMapY = 0;
  int32_t iIconOffset = 0;
  BOOLEAN fContemplatingRetreating = FALSE;

  if (gfDisplayPotentialRetreatPaths && gpBattleGroup) {
    fContemplatingRetreating = TRUE;
  }

  for (sMapX = 1; sMapX < MAP_WORLD_X - 1; sMapX++) {
    for (sMapY = 1; sMapY < MAP_WORLD_Y - 1; sMapY++) {
      // don't show mercs/vehicles currently in this sector if player is contemplating retreating
      // from THIS sector
      if (!fContemplatingRetreating || (sMapX != gpBattleGroup->ubSectorX) ||
          (sMapY != gpBattleGroup->ubSectorY)) {
        if (fShowFlags & SHOW_TEAMMATES) {
          iIconOffset = ShowOnDutyTeam(sMapX, sMapY);
          iIconOffset = ShowAssignedTeam(sMapX, sMapY, iIconOffset);
        }

        if (fShowFlags & SHOW_VEHICLES) iIconOffset = ShowVehicles(sMapX, sMapY, iIconOffset);
      }

      if (fShowFlags & SHOW_TEAMMATES) {
        HandleShowingOfEnemyForcesInSector(sMapX, sMapY, (int8_t)iCurrentMapSectorZ,
                                           (uint8_t)iIconOffset);
        ShowPeopleInMotion(sMapX, sMapY);
      }
    }
  }
}

BOOLEAN ShadeMapElem(uint8_t sMapX, uint8_t sMapY, int32_t iColor) {
  int16_t sScreenX, sScreenY;
  struct VSurface *hSrcVSurface;
  // struct VSurface* hSAMSurface;
  // struct VSurface* hMineSurface;
  uint32_t uiDestPitchBYTES;
  uint32_t uiSrcPitchBYTES;
  uint16_t *pDestBuf;
  uint8_t *pSrcBuf;
  SGPRect clip;
  uint16_t *pOriginalPallette;

  // get original video surface palette
  CHECKF(GetVideoSurface(&hSrcVSurface, guiBIGMAP));
  // get original video surface palette
  // CHECKF( GetVideoSurface( &hSAMSurface, guiSAMICON ) );
  // get original video surface palette
  // CHECKF( GetVideoSurface( &hMineSurface, guiMINEICON ) );
  // get original video surface palette

  pOriginalPallette = hSrcVSurface->p16BPPPalette;

  if (fZoomFlag)
    ShadeMapElemZoomIn(sMapX, sMapY, iColor);
  else {
    GetScreenXYFromMapXY(sMapX, sMapY, &sScreenX, &sScreenY);

    // compensate for original BIG_MAP blit being done at MAP_VIEW_START_X + 1
    sScreenX += 1;

    // compensate for original BIG_MAP blit being done at MAP_VIEW_START_X + 1
    clip.iLeft = 2 * (sScreenX - (MAP_VIEW_START_X + 1));
    clip.iTop = 2 * (sScreenY - MAP_VIEW_START_Y);
    clip.iRight = clip.iLeft + (2 * MAP_GRID_X);
    clip.iBottom = clip.iTop + (2 * MAP_GRID_Y);

    if (iColor != MAP_SHADE_BLACK) {
      // airspace
      /*
                              if( sMapX == 1 )
                              {
                                      clip.iLeft -= 4;
                                      clip.iRight += 4;
                                      sScreenX -= 2;
                              }
                              else
                              {
                                      sScreenX += 1;
                              }
      */
    } else {
      // non-airspace
      sScreenY -= 1;
    }

    switch (iColor) {
      case (MAP_SHADE_BLACK):
        // simply shade darker
        ShadowVideoSurfaceRect(guiSAVEBUFFER, sScreenX, sScreenY, sScreenX + MAP_GRID_X - 1,
                               sScreenY + MAP_GRID_Y - 1);
        break;

      case (MAP_SHADE_LT_GREEN):
        // grab video surface and set palette
        CHECKF(GetVideoSurface(&hSrcVSurface, guiBIGMAP));

        hSrcVSurface->p16BPPPalette = pMapLTGreenPalette;
        // hMineSurface->p16BPPPalette = pMapLTGreenPalette;
        // hSAMSurface->p16BPPPalette = pMapLTGreenPalette;

        // lock source and dest buffers
        pDestBuf = (uint16_t *)LockVideoSurface(guiSAVEBUFFER, &uiDestPitchBYTES);
        CHECKF(GetVideoSurface(&hSrcVSurface, guiBIGMAP));
        pSrcBuf = LockVideoSurface(guiBIGMAP, &uiSrcPitchBYTES);

        Blt8BPPDataTo16BPPBufferHalfRect(pDestBuf, uiDestPitchBYTES, hSrcVSurface, pSrcBuf,
                                         uiSrcPitchBYTES, sScreenX, sScreenY, &clip);

        // now blit
        // Blt8BPPDataSubTo16BPPBuffer( pDestBuf, uiDestPitchBYTES, hSrcVSurface,
        // pSrcBuf,uiSrcPitchBYTES, sScreenX, sScreenY, &clip);

        // unlock source and dest buffers
        UnLockVideoSurface(guiBIGMAP);
        UnLockVideoSurface(guiSAVEBUFFER);
        break;

      case (MAP_SHADE_DK_GREEN):
        // grab video surface and set palette
        CHECKF(GetVideoSurface(&hSrcVSurface, guiBIGMAP));
        hSrcVSurface->p16BPPPalette = pMapDKGreenPalette;
        // hMineSurface->p16BPPPalette = pMapDKGreenPalette;
        // hSAMSurface->p16BPPPalette = pMapDKGreenPalette;

        /// lock source and dest buffers
        pDestBuf = (uint16_t *)LockVideoSurface(guiSAVEBUFFER, &uiDestPitchBYTES);
        CHECKF(GetVideoSurface(&hSrcVSurface, guiBIGMAP));
        pSrcBuf = LockVideoSurface(guiBIGMAP, &uiSrcPitchBYTES);

        Blt8BPPDataTo16BPPBufferHalfRect(pDestBuf, uiDestPitchBYTES, hSrcVSurface, pSrcBuf,
                                         uiSrcPitchBYTES, sScreenX, sScreenY, &clip);

        // now blit
        // Blt8BPPDataSubTo16BPPBuffer( pDestBuf, uiDestPitchBYTES, hSrcVSurface,
        // pSrcBuf,uiSrcPitchBYTES, sScreenX , sScreenY , &clip);

        // unlock source and dest buffers
        UnLockVideoSurface(guiBIGMAP);
        UnLockVideoSurface(guiSAVEBUFFER);
        break;

      case (MAP_SHADE_LT_RED):
        // grab video surface and set palette
        CHECKF(GetVideoSurface(&hSrcVSurface, guiBIGMAP));
        hSrcVSurface->p16BPPPalette = pMapLTRedPalette;
        // hMineSurface->p16BPPPalette = pMapLTRedPalette;
        // hSAMSurface->p16BPPPalette = pMapLTRedPalette;

        // lock source and dest buffers
        pDestBuf = (uint16_t *)LockVideoSurface(guiSAVEBUFFER, &uiDestPitchBYTES);
        CHECKF(GetVideoSurface(&hSrcVSurface, guiBIGMAP));
        pSrcBuf = LockVideoSurface(guiBIGMAP, &uiSrcPitchBYTES);

        Blt8BPPDataTo16BPPBufferHalfRect(pDestBuf, uiDestPitchBYTES, hSrcVSurface, pSrcBuf,
                                         uiSrcPitchBYTES, sScreenX, sScreenY, &clip);

        // now blit
        // Blt8BPPDataSubTo16BPPBuffer( pDestBuf, uiDestPitchBYTES, hSrcVSurface,
        // pSrcBuf,uiSrcPitchBYTES, sScreenX , sScreenY , &clip);

        // unlock source and dest buffers
        UnLockVideoSurface(guiBIGMAP);
        UnLockVideoSurface(guiSAVEBUFFER);
        break;

      case (MAP_SHADE_DK_RED):
        // grab video surface and set palette
        CHECKF(GetVideoSurface(&hSrcVSurface, guiBIGMAP));
        hSrcVSurface->p16BPPPalette = pMapDKRedPalette;
        // hMineSurface->p16BPPPalette = pMapDKRedPalette;
        // hSAMSurface->p16BPPPalette = pMapDKRedPalette;

        // lock source and dest buffers
        pDestBuf = (uint16_t *)LockVideoSurface(guiSAVEBUFFER, &uiDestPitchBYTES);
        CHECKF(GetVideoSurface(&hSrcVSurface, guiBIGMAP));
        pSrcBuf = LockVideoSurface(guiBIGMAP, &uiSrcPitchBYTES);

        Blt8BPPDataTo16BPPBufferHalfRect(pDestBuf, uiDestPitchBYTES, hSrcVSurface, pSrcBuf,
                                         uiSrcPitchBYTES, sScreenX, sScreenY, &clip);

        // now blit
        // Blt8BPPDataSubTo16BPPBuffer( pDestBuf, uiDestPitchBYTES, hSrcVSurface,
        // pSrcBuf,uiSrcPitchBYTES, sScreenX , sScreenY , &clip);

        // unlock source and dest buffers
        UnLockVideoSurface(guiBIGMAP);
        UnLockVideoSurface(guiSAVEBUFFER);
        break;
    }

    // restore original palette
    CHECKF(GetVideoSurface(&hSrcVSurface, guiBIGMAP));
    hSrcVSurface->p16BPPPalette = pOriginalPallette;
    // hMineSurface->p16BPPPalette = pOriginalPallette;
    // hSAMSurface->p16BPPPalette = pOriginalPallette;
  }

  return (TRUE);
}

BOOLEAN ShadeMapElemZoomIn(uint8_t sMapX, uint8_t sMapY, int32_t iColor) {
  int16_t sScreenX, sScreenY;
  int32_t iX, iY;
  struct VSurface *hSrcVSurface;
  uint32_t uiDestPitchBYTES;
  uint32_t uiSrcPitchBYTES;
  uint16_t *pDestBuf;
  // uint8_t *pDestBuf2;
  uint8_t *pSrcBuf;
  SGPRect clip;
  uint16_t *pOriginalPallette;

  // trabslate to screen co-ords for zoomed
  GetScreenXYFromMapXYStationary(sMapX, sMapY, &sScreenX, &sScreenY);

  // shift left by one sector
  iY = (int32_t)sScreenY - MAP_GRID_Y;
  iX = (int32_t)sScreenX - MAP_GRID_X;

  // get original video surface palette
  CHECKF(GetVideoSurface(&hSrcVSurface, guiBIGMAP));
  pOriginalPallette = hSrcVSurface->p16BPPPalette;

  if ((iX > MapScreenRect.iLeft - MAP_GRID_X * 2) && (iX < MapScreenRect.iRight) &&
      (iY > MapScreenRect.iTop - MAP_GRID_Y * 2) && (iY < MapScreenRect.iBottom)) {
    sScreenX = (int16_t)iX;
    sScreenY = (int16_t)iY;

    if (iColor == MAP_SHADE_BLACK) {
      clip.iLeft = sScreenX + 1;
      clip.iRight = sScreenX + MAP_GRID_X * 2 - 1;
      clip.iTop = sScreenY;
      clip.iBottom = sScreenY + MAP_GRID_Y * 2 - 1;
    } else {
      clip.iLeft = iZoomX + sScreenX - MAP_VIEW_START_X - MAP_GRID_X;
      clip.iRight = clip.iLeft + MAP_GRID_X * 2;
      clip.iTop = iZoomY + sScreenY - MAP_VIEW_START_Y - MAP_GRID_Y;
      clip.iBottom = clip.iTop + MAP_GRID_Y * 2;

      if (sScreenY <= MapScreenRect.iTop + 10) {
        clip.iTop -= 5;
        sScreenY -= 5;
      }

      if (sMapX == 1) {
        clip.iLeft -= 5;
        sScreenX -= 4;
      } else {
        sScreenX += 1;
      }
    }

    if (sScreenX >= MapScreenRect.iRight - 2 * MAP_GRID_X) {
      clip.iRight++;
    }

    if (sScreenY >= MapScreenRect.iBottom - 2 * MAP_GRID_X) {
      clip.iBottom++;
    }

    sScreenX += 1;
    sScreenY += 1;

    if ((sScreenX > MapScreenRect.iRight) || (sScreenY > MapScreenRect.iBottom)) {
      return (FALSE);
    }

    switch (iColor) {
      case (MAP_SHADE_BLACK):
        // simply shade darker
        if (iCurrentMapSectorZ > 0) {
          ShadowVideoSurfaceRect(guiSAVEBUFFER, clip.iLeft, clip.iTop, clip.iRight, clip.iBottom);
        }
        ShadowVideoSurfaceRect(guiSAVEBUFFER, clip.iLeft, clip.iTop, clip.iRight, clip.iBottom);
        break;

      case (MAP_SHADE_LT_GREEN):
        // grab video surface and set palette
        CHECKF(GetVideoSurface(&hSrcVSurface, guiBIGMAP));
        hSrcVSurface->p16BPPPalette = pMapLTGreenPalette;

        // lock source and dest buffers
        pDestBuf = (uint16_t *)LockVideoSurface(guiSAVEBUFFER, &uiDestPitchBYTES);
        CHECKF(GetVideoSurface(&hSrcVSurface, guiBIGMAP));
        pSrcBuf = LockVideoSurface(guiBIGMAP, &uiSrcPitchBYTES);

        // now blit
        Blt8BPPDataSubTo16BPPBuffer(pDestBuf, uiDestPitchBYTES, hSrcVSurface, pSrcBuf,
                                    uiSrcPitchBYTES, sScreenX, sScreenY, &clip);

        // unlock source and dest buffers
        UnLockVideoSurface(guiBIGMAP);
        UnLockVideoSurface(guiSAVEBUFFER);

        break;

      case (MAP_SHADE_DK_GREEN):
        // grab video surface and set palette
        CHECKF(GetVideoSurface(&hSrcVSurface, guiBIGMAP));
        hSrcVSurface->p16BPPPalette = pMapDKGreenPalette;

        /// lock source and dest buffers
        pDestBuf = (uint16_t *)LockVideoSurface(guiSAVEBUFFER, &uiDestPitchBYTES);
        CHECKF(GetVideoSurface(&hSrcVSurface, guiBIGMAP));
        pSrcBuf = LockVideoSurface(guiBIGMAP, &uiSrcPitchBYTES);

        // now blit
        Blt8BPPDataSubTo16BPPBuffer(pDestBuf, uiDestPitchBYTES, hSrcVSurface, pSrcBuf,
                                    uiSrcPitchBYTES, sScreenX, sScreenY, &clip);

        // unlock source and dest buffers
        UnLockVideoSurface(guiBIGMAP);
        UnLockVideoSurface(guiSAVEBUFFER);

        break;

      case (MAP_SHADE_LT_RED):
        // grab video surface and set palette
        CHECKF(GetVideoSurface(&hSrcVSurface, guiBIGMAP));
        hSrcVSurface->p16BPPPalette = pMapLTRedPalette;

        // lock source and dest buffers
        pDestBuf = (uint16_t *)LockVideoSurface(guiSAVEBUFFER, &uiDestPitchBYTES);
        CHECKF(GetVideoSurface(&hSrcVSurface, guiBIGMAP));
        pSrcBuf = LockVideoSurface(guiBIGMAP, &uiSrcPitchBYTES);

        // now blit
        Blt8BPPDataSubTo16BPPBuffer(pDestBuf, uiDestPitchBYTES, hSrcVSurface, pSrcBuf,
                                    uiSrcPitchBYTES, sScreenX, sScreenY, &clip);

        // unlock source and dest buffers
        UnLockVideoSurface(guiBIGMAP);
        UnLockVideoSurface(guiSAVEBUFFER);

        break;

      case (MAP_SHADE_DK_RED):
        // grab video surface and set palette
        CHECKF(GetVideoSurface(&hSrcVSurface, guiBIGMAP));
        hSrcVSurface->p16BPPPalette = pMapDKRedPalette;

        // lock source and dest buffers
        pDestBuf = (uint16_t *)LockVideoSurface(guiSAVEBUFFER, &uiDestPitchBYTES);
        CHECKF(GetVideoSurface(&hSrcVSurface, guiBIGMAP));
        pSrcBuf = LockVideoSurface(guiBIGMAP, &uiSrcPitchBYTES);

        // now blit
        Blt8BPPDataSubTo16BPPBuffer(pDestBuf, uiDestPitchBYTES, hSrcVSurface, pSrcBuf,
                                    uiSrcPitchBYTES, sScreenX, sScreenY, &clip);

        // unlock source and dest buffers
        UnLockVideoSurface(guiBIGMAP);
        UnLockVideoSurface(guiSAVEBUFFER);

        break;
    }
  }

  // restore original palette
  CHECKF(GetVideoSurface(&hSrcVSurface, guiBIGMAP));
  hSrcVSurface->p16BPPPalette = pOriginalPallette;

  return (TRUE);
}

BOOLEAN InitializePalettesForMap(void) {
  // init palettes
  struct VSurface *hSrcVSurface;
  struct SGPPaletteEntry pPalette[256];
  VSURFACE_DESC vs_desc;
  uint32_t uiTempMap;

  // load image
  vs_desc.fCreateFlags = VSURFACE_CREATE_FROMFILE | VSURFACE_SYSTEM_MEM_USAGE;
  strcpy(vs_desc.ImageFile, "INTERFACE\\b_map.pcx");
  CHECKF(AddVideoSurface(&vs_desc, &uiTempMap));

  // get video surface
  CHECKF(GetVideoSurface(&hSrcVSurface, uiTempMap));
  GetVSurfacePaletteEntries(hSrcVSurface, pPalette);

  // set up various palettes
  pMapLTRedPalette = Create16BPPPaletteShaded(pPalette, 400, 0, 0, TRUE);
  pMapDKRedPalette = Create16BPPPaletteShaded(pPalette, 200, 0, 0, TRUE);
  pMapLTGreenPalette = Create16BPPPaletteShaded(pPalette, 0, 400, 0, TRUE);
  pMapDKGreenPalette = Create16BPPPaletteShaded(pPalette, 0, 200, 0, TRUE);

  // delete image
  DeleteVideoSurfaceFromIndex(uiTempMap);

  return (TRUE);
}

void ShutDownPalettesForMap(void) {
  MemFree(pMapLTRedPalette);
  MemFree(pMapDKRedPalette);
  MemFree(pMapLTGreenPalette);
  MemFree(pMapDKGreenPalette);

  pMapLTRedPalette = NULL;
  pMapDKRedPalette = NULL;
  pMapLTGreenPalette = NULL;
  pMapDKGreenPalette = NULL;

  return;
}

void PlotPathForCharacter(struct SOLDIERTYPE *pCharacter, uint8_t sX, uint8_t sY,
                          BOOLEAN fTacticalTraversal) {
  // will plot a path for this character

  // is cursor allowed here?..if not..don't build path
  if (!IsTheCursorAllowedToHighLightThisSector(sX, sY)) {
    return;
  }

  // is the character in transit?..then leave
  if (pCharacter->bAssignment == IN_TRANSIT) {
    // leave
    return;
  }

  if (pCharacter->bSectorZ != 0) {
    if (pCharacter->bAssignment >= ON_DUTY) {
      // not on the surface, character won't move until they reach surface..info player of this fact
      MapScreenMessage(FONT_MCOLOR_DKRED, MSG_INTERFACE, L"%s %s", pCharacter->name,
                       gsUndergroundString[0]);
    } else  // squad
    {
      MapScreenMessage(FONT_MCOLOR_DKRED, MSG_INTERFACE, L"%s %s",
                       pLongAssignmentStrings[pCharacter->bAssignment], gsUndergroundString[0]);
    }
    return;
  }

  if ((pCharacter->bAssignment == VEHICLE) || (pCharacter->uiStatusFlags & SOLDIER_VEHICLE)) {
    SetUpMvtGroupForVehicle(pCharacter);
  }

  // make sure we are at the beginning
  pCharacter->pMercPath = MoveToBeginningOfPathList(pCharacter->pMercPath);

  // will plot a path from current position to sX, sY
  // get last sector in characters list, build new path, remove tail section, move to beginning of
  // list, and append onto old list
  pCharacter->pMercPath = AppendStrategicPath(
      MoveToBeginningOfPathList(BuildAStrategicPath(
          NULL, GetLastSectorIdInCharactersPath(pCharacter), (int16_t)(sX + sY * (MAP_WORLD_X)),
          GetSoldierGroupId(pCharacter), fTacticalTraversal /*, FALSE */)),
      pCharacter->pMercPath);

  // move to beginning of list
  pCharacter->pMercPath = MoveToBeginningOfPathList(pCharacter->pMercPath);

  // check if in vehicle, if so, copy path to vehicle
  if ((pCharacter->bAssignment == VEHICLE) || (pCharacter->uiStatusFlags & SOLDIER_VEHICLE)) {
    MoveCharactersPathToVehicle(pCharacter);
  } else {
    CopyPathToCharactersSquadIfInOne(pCharacter);
  }
}

void PlotATemporaryPathForCharacter(struct SOLDIERTYPE *pCharacter, uint8_t sX, uint8_t sY) {
  // make sure we're at the beginning
  pTempCharacterPath = MoveToBeginningOfPathList(pTempCharacterPath);

  // clear old temp path
  pTempCharacterPath = ClearStrategicPathList(pTempCharacterPath, -1);

  // is cursor allowed here?..if not..don't build temp path
  if (!IsTheCursorAllowedToHighLightThisSector(sX, sY)) {
    return;
  }

  // build path
  pTempCharacterPath = BuildAStrategicPath(
      pTempCharacterPath, GetLastSectorIdInCharactersPath(pCharacter),
      (int16_t)(sX + sY * (MAP_WORLD_X)), GetSoldierGroupId(pCharacter), FALSE /*, TRUE */);

  return;
}

// clear out character path list, after and including this sector
uint32_t ClearPathAfterThisSectorForCharacter(struct SOLDIERTYPE *pCharacter, uint8_t sX,
                                              uint8_t sY) {
  int32_t iOrigLength = 0;
  VEHICLETYPE *pVehicle = NULL;

  iOrigLength = GetLengthOfMercPath(pCharacter);

  if (!iOrigLength) {
    // no previous path, nothing to do
    return (ABORT_PLOTTING);
  }

  // if we're clearing everything beyond the current sector, that's quite different.  Since we're
  // basically cancelling his movement completely, we must also make sure his next X,Y are changed
  // and he officially "returns" to his sector
  if ((sX == GetSolSectorX(pCharacter)) && (sY == GetSolSectorY(pCharacter))) {
    // if we're in confirm map move mode, cancel that (before new UI messages are issued)
    EndConfirmMapMoveMode();

    CancelPathsOfAllSelectedCharacters();
    return (PATH_CLEARED);
  } else  // click not in the current sector
  {
    // if the clicked sector is along current route, this will repath only as far as it.  If not,
    // the entire path will be canceled.

    // if a vehicle
    if (pCharacter->uiStatusFlags & SOLDIER_VEHICLE) {
      pVehicle = &(pVehicleList[pCharacter->bVehicleID]);
    }
    // or in a vehicle
    else if (pCharacter->bAssignment == VEHICLE) {
      pVehicle = &(pVehicleList[pCharacter->iVehicleId]);
    } else {
      // foot soldier
      pCharacter->pMercPath = ClearStrategicPathListAfterThisSector(pCharacter->pMercPath, sX, sY,
                                                                    pCharacter->ubGroupID);
    }

    // if there's an associated vehicle structure
    if (pVehicle != NULL) {
      // do it for the vehicle, too
      pVehicle->pMercPath = ClearStrategicPathListAfterThisSector(pVehicle->pMercPath, sX, sY,
                                                                  pVehicle->ubMovementGroup);
    }

    if (GetLengthOfMercPath(pCharacter) < iOrigLength) {
      CopyPathToAllSelectedCharacters(pCharacter->pMercPath);
      // path WAS actually shortened
      return (PATH_SHORTENED);
    } else {
      // same path as before - it's not any shorter
      return (ABORT_PLOTTING);
    }
  }
}

void CancelPathForCharacter(struct SOLDIERTYPE *pCharacter) {
  // clear out character's entire path list, he and his squad will stay/return to his current
  // sector.
  pCharacter->pMercPath = ClearStrategicPathList(pCharacter->pMercPath, pCharacter->ubGroupID);
  // NOTE: This automatically calls RemoveGroupWaypoints() internally for valid movement groups

  // This causes the group to effectively reverse directions (even if they've never actually left),
  // so handle that. They are going to return to their current X,Y sector.
  RebuildWayPointsForGroupPath(pCharacter->pMercPath, pCharacter->ubGroupID);
  //	GroupReversingDirectionsBetweenSectors( GetGroup( pCharacter->ubGroupID ), ( uint8_t )(
  // GetSolSectorX(pCharacter) ), ( uint8_t )( GetSolSectorY(pCharacter) ), FALSE );

  // if he's in a vehicle, clear out the vehicle, too
  if (pCharacter->bAssignment == VEHICLE) {
    CancelPathForVehicle(&(pVehicleList[pCharacter->iVehicleId]), TRUE);
  } else {
    // display "travel route canceled" message
    MapScreenMessage(FONT_MCOLOR_LTYELLOW, MSG_MAP_UI_POSITION_MIDDLE, pMapPlotStrings[3]);
  }

  CopyPathToCharactersSquadIfInOne(pCharacter);

  MarkForRedrawalStrategicMap();
  fTeamPanelDirty = TRUE;
  fCharacterInfoPanelDirty = TRUE;  // to update ETA
}

void CancelPathForVehicle(VEHICLETYPE *pVehicle, BOOLEAN fAlreadyReversed) {
  // we're clearing everything beyond the *current* sector, that's quite different.  Since we're
  // basically cancelling his movement completely, we must also make sure his next X,Y are changed
  // and he officially "returns" to his sector
  pVehicle->pMercPath = ClearStrategicPathList(pVehicle->pMercPath, pVehicle->ubMovementGroup);
  // NOTE: This automatically calls RemoveGroupWaypoints() internally for valid movement groups

  // if we already reversed one of the passengers, flag will be TRUE,
  // don't do it again or we're headed back where we came from!
  if (!fAlreadyReversed) {
    // This causes the group to effectively reverse directions (even if they've never actually
    // left), so handle that. They are going to return to their current X,Y sector.
    RebuildWayPointsForGroupPath(pVehicle->pMercPath, pVehicle->ubMovementGroup);
    //		GroupReversingDirectionsBetweenSectors( GetGroup( pVehicle->ubMovementGroup ), (
    // uint8_t ) ( pVehicle->sSectorX ), ( uint8_t ) ( pVehicle->sSectorY ), FALSE );
  }

  // display "travel route canceled" message
  MapScreenMessage(FONT_MCOLOR_LTYELLOW, MSG_MAP_UI_POSITION_MIDDLE, pMapPlotStrings[3]);

  // turn the helicopter flag off here, this prevents the "route aborted" msg from coming up
  fPlotForHelicopter = FALSE;

  fTeamPanelDirty = TRUE;
  MarkForRedrawalStrategicMap();
  fCharacterInfoPanelDirty = TRUE;  // to update ETA
}

void CancelPathForGroup(struct GROUP *pGroup) {
  int32_t iVehicleId;

  // if it's the chopper, but player can't redirect it
  if (pGroup->fPlayer && IsGroupTheHelicopterGroup(pGroup) && (CanHelicopterFly() == FALSE)) {
    // explain & ignore
    ExplainWhySkyriderCantFly();
    return;
  }

  // is it a non-vehicle group?
  if ((pGroup->fPlayer) && (pGroup->fVehicle == FALSE)) {
    if (pGroup->pPlayerList) {
      if (pGroup->pPlayerList->pSoldier) {
        // clearing one merc should be enough, it copies changes to his squad on its own
        CancelPathForCharacter(pGroup->pPlayerList->pSoldier);
      }
    }
  }
  // is it a vehicle group?
  else if (pGroup->fVehicle) {
    iVehicleId = GivenMvtGroupIdFindVehicleId(pGroup->ubGroupID);

    // must be valid!
    Assert(iVehicleId != -1);
    if (iVehicleId == -1) return;

    CancelPathForVehicle(&(pVehicleList[iVehicleId]), FALSE);
  }
}

void CopyPathToCharactersSquadIfInOne(struct SOLDIERTYPE *pCharacter) {
  int8_t bSquad = 0;

  // check if on a squad, if so, do same thing for all characters

  // what squad is character in?
  bSquad = SquadCharacterIsIn(pCharacter);

  // check to see if character is on a squad, if so, copy path to squad
  if (bSquad != -1) {
    // copy path over
    CopyPathOfCharacterToSquad(pCharacter, bSquad);
  }
}

void DisplaySoldierPath(struct SOLDIERTYPE *pCharacter) {
  struct path *pPath = NULL;

  /* ARM: Hopefully no longer required once using GetSoldierMercPathPtr() ???
          // check if in vehicle, if so, copy path to vehicle
          if( ( pCharacter->bAssignment == VEHICLE )||( pCharacter->uiStatusFlags & SOLDIER_VEHICLE
     ) )
          {
                  // get the real path from vehicle's structure and copy it into this soldier's
                  CopyVehiclePathToSoldier( pCharacter );
          }
  */

  pPath = GetSoldierMercPathPtr(pCharacter);

  // trace real route
  TracePathRoute(FALSE, TRUE, pPath);
  AnimateRoute(pPath);

  return;
}

void DisplaySoldierTempPath(struct SOLDIERTYPE *pCharacter) {
  // now render temp route
  TracePathRoute(FALSE, TRUE, pTempCharacterPath);

  return;
}

void DisplayHelicopterPath(void) {
  // move to beginning of path list
  pVehicleList[iHelicopterVehicleId].pMercPath =
      MoveToBeginningOfPathList(pVehicleList[iHelicopterVehicleId].pMercPath);

  // clip to map
  ClipBlitsToMapViewRegion();

  // trace both lists..temp is conditional if cursor has sat in same sector grid long enough
  TracePathRoute(TRUE, TRUE, pVehicleList[iHelicopterVehicleId].pMercPath);
  AnimateRoute(pVehicleList[iHelicopterVehicleId].pMercPath);

  // restore
  RestoreClipRegionToFullScreen();

  return;
}

void DisplayHelicopterTempPath(void) {
  // should we draw temp path?
  if (fDrawTempHeliPath) {
    TracePathRoute(TRUE, TRUE, pTempHelicopterPath);
  }

  return;
}

void PlotPathForHelicopter(uint8_t sX, uint8_t sY) {
  // will plot the path for the helicopter

  // no heli...go back
  if (!fShowAircraftFlag || !fHelicopterAvailable) {
    return;
  }

  // is cursor allowed here?..if not..don't build path
  if (!IsTheCursorAllowedToHighLightThisSector(sX, sY)) {
    return;
  }

  // set up mvt group for helicopter
  SetUpHelicopterForMovement();

  // move to beginning of list
  // pHelicopterPath = MoveToBeginningOfPathList( pVehicleList[ iHelicopterVehicleId ].pMercPath );
  MoveToBeginningOfPathList(pVehicleList[iHelicopterVehicleId].pMercPath);

  // will plot a path from current position to sX, sY
  // get last sector in helicopters list, build new path, remove tail section, move to beginning of
  // list, and append onto old list
  pVehicleList[iHelicopterVehicleId].pMercPath = AppendStrategicPath(
      MoveToBeginningOfPathList(BuildAStrategicPath(
          NULL, GetLastSectorOfHelicoptersPath(), (int16_t)(sX + sY * (MAP_WORLD_X)),
          pVehicleList[iHelicopterVehicleId].ubMovementGroup, FALSE /*, FALSE */)),
      pVehicleList[iHelicopterVehicleId].pMercPath);

  // move to beginning of list
  pVehicleList[iHelicopterVehicleId].pMercPath =
      MoveToBeginningOfPathList(pVehicleList[iHelicopterVehicleId].pMercPath);

  MarkForRedrawalStrategicMap();

  return;
}

void PlotATemporaryPathForHelicopter(uint8_t sX, uint8_t sY) {
  // clear old temp path
  pTempHelicopterPath = ClearStrategicPathList(pTempHelicopterPath, 0);

  // is cursor allowed here?..if not..don't build temp path
  if (!IsTheCursorAllowedToHighLightThisSector(sX, sY)) {
    return;
  }

  // build path
  pTempHelicopterPath = BuildAStrategicPath(
      NULL, GetLastSectorOfHelicoptersPath(), (int16_t)(sX + sY * (MAP_WORLD_X)),
      pVehicleList[iHelicopterVehicleId].ubMovementGroup, FALSE /*, TRUE */);

  return;
}

// clear out helicopter path list, after and including this sector
uint32_t ClearPathAfterThisSectorForHelicopter(uint8_t sX, uint8_t sY) {
  VEHICLETYPE *pVehicle = NULL;
  int32_t iOrigLength = 0;

  // clear out helicopter path list, after and including this sector
  if (iHelicopterVehicleId == -1 || !CanHelicopterFly()) {
    // abort plotting, shouldn't even be here
    return (ABORT_PLOTTING);
  }

  pVehicle = &(pVehicleList[iHelicopterVehicleId]);

  iOrigLength = GetLengthOfPath(pVehicle->pMercPath);
  if (!iOrigLength) {
    // no previous path, nothing to do, and we didn't shorten it
    return (ABORT_PLOTTING);
  }

  // are we clearing everything beyond the helicopter's CURRENT sector?
  if ((sX == pVehicle->sSectorX) && (sY == pVehicle->sSectorY)) {
    // if we're in confirm map move mode, cancel that (before new UI messages are issued)
    EndConfirmMapMoveMode();

    CancelPathForVehicle(pVehicle, FALSE);
    return (PATH_CLEARED);
  } else  // click not in the current sector
  {
    // if the clicked sector is along current route, this will repath only as far as it.  If not,
    // the entire path will be canceled.
    pVehicle->pMercPath = ClearStrategicPathListAfterThisSector(pVehicle->pMercPath, sX, sY,
                                                                pVehicle->ubMovementGroup);

    if (GetLengthOfPath(pVehicle->pMercPath) < iOrigLength) {
      // really shortened!
      return (PATH_SHORTENED);
    } else {
      // same path as before - it's not any shorter
      return (ABORT_PLOTTING);
    }
  }
}

int16_t GetLastSectorOfHelicoptersPath(void) {
  // will return the last sector of the helicopter's current path
  int16_t sLastSector = pVehicleList[iHelicopterVehicleId].sSectorX +
                        pVehicleList[iHelicopterVehicleId].sSectorY * MAP_WORLD_X;
  struct path *pNode = NULL;

  pNode = pVehicleList[iHelicopterVehicleId].pMercPath;

  while (pNode) {
    sLastSector = (int16_t)(pNode->uiSectorId);
    pNode = pNode->pNext;
  }

  return sLastSector;
}

BOOLEAN TracePathRoute(BOOLEAN fCheckFlag, BOOLEAN fForceUpDate, struct path *pPath) {
  BOOLEAN fSpeedFlag = FALSE;
  int32_t iArrow = -1;
  int32_t iX, iY;
  int16_t sX, sY;
  int32_t iArrowX, iArrowY;
  int32_t iDeltaA, iDeltaB, iDeltaB1;
  int32_t iDirection = 0;
  BOOLEAN fUTurnFlag = FALSE;
  struct path *pNode = NULL;
  struct path *pPastNode = NULL;
  struct path *pNextNode = NULL;
  struct VObject *hMapHandle;

  if (pPath == NULL) {
    return FALSE;
  }

  while (pPath->pPrev) {
    pPath = pPath->pPrev;
  }

  pNode = pPath;

  iDirection = -1;
  if (pNode->pNext)
    pNextNode = pNode->pNext;
  else
    pNextNode = NULL;
  if (pNode->pPrev)
    pPastNode = pNode->pPrev;
  else
    pPastNode = NULL;

  GetVideoObject(&hMapHandle, guiMAPCURSORS);
  // go through characters list and display arrows for path
  while (pNode) {
    fUTurnFlag = FALSE;
    if ((pPastNode) && (pNextNode)) {
      iDeltaA = (int16_t)pNode->uiSectorId - (int16_t)pPastNode->uiSectorId;
      iDeltaB = (int16_t)pNode->uiSectorId - (int16_t)pNextNode->uiSectorId;
      if (iDeltaA == 0) return (FALSE);
      if (pNode->fSpeed)
        fSpeedFlag = FALSE;
      else
        fSpeedFlag = TRUE;
      if (!fZoomFlag) {
        iX = (SectorID16_X(pNode->uiSectorId));
        iY = (SectorID16_Y(pNode->uiSectorId));
        iX = (iX * MAP_GRID_X) + MAP_VIEW_START_X;
        iY = (iY * MAP_GRID_Y) + MAP_VIEW_START_Y;

      } else {
        GetScreenXYFromMapXYStationary(((uint16_t)(SectorID16_X(pNode->uiSectorId))),
                                       ((uint16_t)(SectorID16_Y(pNode->uiSectorId))), &sX, &sY);
        iY = sY - MAP_GRID_Y;
        iX = sX - MAP_GRID_X;
      }
      iArrowX = iX;
      iArrowY = iY;
      if ((pPastNode->pPrev) && (pNextNode->pNext)) {
        fUTurnFlag = FALSE;
        // check to see if out-of sector U-turn
        // for placement of arrows
        iDeltaB1 = pNextNode->uiSectorId - pNextNode->pNext->uiSectorId;
        if ((iDeltaB1 == -MAP_WORLD_X) && (iDeltaA == -MAP_WORLD_X) && (iDeltaB == -1)) {
          fUTurnFlag = TRUE;
        } else if ((iDeltaB1 == -MAP_WORLD_X) && (iDeltaA == -MAP_WORLD_X) && (iDeltaB == 1)) {
          fUTurnFlag = TRUE;
        } else if ((iDeltaB1 == MAP_WORLD_X) && (iDeltaA == MAP_WORLD_X) && (iDeltaB == 1)) {
          fUTurnFlag = TRUE;
        } else if ((iDeltaB1 == -MAP_WORLD_X) && (iDeltaA == -MAP_WORLD_X) && (iDeltaB == 1)) {
          fUTurnFlag = TRUE;
        } else if ((iDeltaB1 == -1) && (iDeltaA == -1) && (iDeltaB == -MAP_WORLD_X)) {
          fUTurnFlag = TRUE;
        } else if ((iDeltaB1 == -1) && (iDeltaA == -1) && (iDeltaB == MAP_WORLD_X)) {
          fUTurnFlag = TRUE;
        } else if ((iDeltaB1 == 1) && (iDeltaA == 1) && (iDeltaB == -MAP_WORLD_X)) {
          fUTurnFlag = TRUE;
        } else if ((iDeltaB1 == 1) && (iDeltaA == 1) && (iDeltaB == MAP_WORLD_X)) {
          fUTurnFlag = TRUE;
        } else
          fUTurnFlag = FALSE;
      }

      if ((pPastNode->uiSectorId == pNextNode->uiSectorId)) {
        if (pPastNode->uiSectorId + MAP_WORLD_X == pNode->uiSectorId) {
          if (!(pNode->fSpeed))
            fSpeedFlag = TRUE;
          else
            fSpeedFlag = FALSE;

          if (fZoomFlag) {
            iDirection = S_TO_N_ZOOM_LINE;
            if (fSpeedFlag)
              iArrow = ZOOM_Y_NORTH_ARROW;
            else
              iArrow = ZOOM_NORTH_ARROW;
            iArrowX += NORTH_OFFSET_X * 2;
            iArrowY += NORTH_OFFSET_Y * 2;
          } else {
            iDirection = S_TO_N_LINE;
            if (fSpeedFlag)
              iArrow = Y_NORTH_ARROW;
            else
              iArrow = NORTH_ARROW;
            iArrowX += NORTH_OFFSET_X;
            iArrowY += NORTH_OFFSET_Y;
          }
        } else if (pPastNode->uiSectorId - MAP_WORLD_X == pNode->uiSectorId) {
          if (fZoomFlag) {
            iDirection = N_TO_S_ZOOM_LINE;
            if (fSpeedFlag)
              iArrow = ZOOM_Y_SOUTH_ARROW;
            else
              iArrow = ZOOM_SOUTH_ARROW;
            iArrowX += SOUTH_OFFSET_X * 2;
            iArrowY += SOUTH_OFFSET_Y * 2;
          } else {
            iDirection = N_TO_S_LINE;
            if (fSpeedFlag)
              iArrow = Y_SOUTH_ARROW;
            else
              iArrow = SOUTH_ARROW;
            iArrowX += SOUTH_OFFSET_X;
            iArrowY += SOUTH_OFFSET_Y;
          }
        } else if (pPastNode->uiSectorId + 1 == pNode->uiSectorId) {
          if (fZoomFlag) {
            iDirection = E_TO_W_ZOOM_LINE;
            if (fSpeedFlag)
              iArrow = ZOOM_Y_WEST_ARROW;
            else
              iArrow = ZOOM_WEST_ARROW;
            iArrowX += WEST_OFFSET_X * 2;
            iArrowY += WEST_OFFSET_Y * 2;
          } else {
            iDirection = E_TO_W_LINE;
            if (fSpeedFlag)
              iArrow = Y_WEST_ARROW;
            else
              iArrow = WEST_ARROW;
            iArrowX += WEST_OFFSET_X;
            iArrowY += WEST_OFFSET_Y;
          }
        } else {
          if (fZoomFlag) {
            iDirection = W_TO_E_ZOOM_LINE;
            if (fSpeedFlag)
              iArrow = ZOOM_Y_EAST_ARROW;
            else
              iArrow = ZOOM_EAST_ARROW;
            iArrowX += EAST_OFFSET_X * 2;
            iArrowY += EAST_OFFSET_Y * 2;
          } else {
            iDirection = W_TO_E_LINE;
            if (fSpeedFlag)
              iArrow = Y_EAST_ARROW;
            else
              iArrow = EAST_ARROW;
            iArrowX += EAST_OFFSET_X;
            iArrowY += EAST_OFFSET_Y;
          }
        }
      } else {
        if ((iDeltaA == -1) && (iDeltaB == 1)) {
          /*
                                                  if( pPastNode == NULL )
                                                  {
                                                          fSpeedFlag = !fSpeedFlag;
                                                  }

          */
          if (fZoomFlag) {
            iDirection = WEST_ZOOM_LINE;
            if (fSpeedFlag)
              iArrow = ZOOM_Y_WEST_ARROW;
            else
              iArrow = ZOOM_WEST_ARROW;
            iArrowX += WEST_OFFSET_X * 2;
            iArrowY += WEST_OFFSET_Y * 2;
          } else {
            iDirection = WEST_LINE;
            if (fSpeedFlag)
              iArrow = Y_WEST_ARROW;
            else
              iArrow = WEST_ARROW;
            iArrowX += WEST_OFFSET_X;
            iArrowY += WEST_OFFSET_Y;
          }
        } else if ((iDeltaA == 1) && (iDeltaB == -1)) {
          if (fZoomFlag) {
            iDirection = EAST_ZOOM_LINE;
            if (fSpeedFlag)
              iArrow = ZOOM_Y_EAST_ARROW;
            else
              iArrow = ZOOM_EAST_ARROW;
            iArrowX += EAST_OFFSET_X * 2;
            iArrowY += EAST_OFFSET_Y * 2;
          } else {
            iDirection = EAST_LINE;
            if (fSpeedFlag)
              iArrow = Y_EAST_ARROW;
            else
              iArrow = EAST_ARROW;
            iArrowX += EAST_OFFSET_X;
            iArrowY += EAST_OFFSET_Y;
          }
        } else if ((iDeltaA == -MAP_WORLD_X) && (iDeltaB == MAP_WORLD_X)) {
          if (fZoomFlag) {
            iDirection = NORTH_ZOOM_LINE;
            if (fSpeedFlag)
              iArrow = ZOOM_Y_NORTH_ARROW;
            else
              iArrow = ZOOM_NORTH_ARROW;
            iArrowX += NORTH_OFFSET_X * 2;
            iArrowY += NORTH_OFFSET_Y * 2;
          } else {
            iDirection = NORTH_LINE;
            if (fSpeedFlag)
              iArrow = Y_NORTH_ARROW;
            else
              iArrow = NORTH_ARROW;
            iArrowX += NORTH_OFFSET_X;
            iArrowY += NORTH_OFFSET_Y;
          }
        } else if ((iDeltaA == MAP_WORLD_X) && (iDeltaB == -MAP_WORLD_X)) {
          if (fZoomFlag) {
            iDirection = SOUTH_ZOOM_LINE;
            if (fSpeedFlag)
              iArrow = ZOOM_Y_SOUTH_ARROW;
            else
              iArrow = ZOOM_SOUTH_ARROW;
            iArrowX += SOUTH_OFFSET_X * 2;
            iArrowY += SOUTH_OFFSET_Y * 2;
          } else {
            iDirection = SOUTH_LINE;
            if (fSpeedFlag)
              iArrow = Y_SOUTH_ARROW;
            else
              iArrow = SOUTH_ARROW;
            iArrowX += SOUTH_OFFSET_X;
            iArrowY += SOUTH_OFFSET_Y;
          }
        } else if ((iDeltaA == -MAP_WORLD_X) && (iDeltaB == -1)) {
          if (fZoomFlag) {
            iDirection = N_TO_E_ZOOM_LINE;
            if (fSpeedFlag)
              iArrow = ZOOM_Y_EAST_ARROW;
            else
              iArrow = ZOOM_EAST_ARROW;
            iArrowX += EAST_OFFSET_X * 2;
            iArrowY += EAST_OFFSET_Y * 2;
          } else {
            iDirection = N_TO_E_LINE;
            if (fSpeedFlag)
              iArrow = Y_EAST_ARROW;
            else
              iArrow = EAST_ARROW;
            iArrowX += EAST_OFFSET_X;
            iArrowY += EAST_OFFSET_Y;
          }
        } else if ((iDeltaA == MAP_WORLD_X) && (iDeltaB == 1)) {
          if (fZoomFlag) {
            iDirection = S_TO_W_ZOOM_LINE;
            if (fSpeedFlag)
              iArrow = ZOOM_Y_WEST_ARROW;
            else
              iArrow = ZOOM_WEST_ARROW;
            iArrowX += WEST_OFFSET_X * 2;
            iArrowY += WEST_OFFSET_Y * 2;
          } else {
            iDirection = S_TO_W_LINE;
            if (fSpeedFlag)
              iArrow = Y_WEST_ARROW;
            else
              iArrow = WEST_ARROW;
            iArrowX += WEST_OFFSET_X;
            iArrowY += WEST_OFFSET_Y;
          }
        } else if ((iDeltaA == 1) && (iDeltaB == -MAP_WORLD_X)) {
          if (fZoomFlag) {
            iDirection = E_TO_S_ZOOM_LINE;
            if (fSpeedFlag)
              iArrow = ZOOM_Y_SOUTH_ARROW;
            else
              iArrow = ZOOM_SOUTH_ARROW;
            iArrowX += SOUTH_OFFSET_X * 2;
            iArrowY += SOUTH_OFFSET_Y * 2;
          } else {
            iDirection = E_TO_S_LINE;
            if (fSpeedFlag)
              iArrow = Y_SOUTH_ARROW;
            else
              iArrow = SOUTH_ARROW;
            iArrowX += SOUTH_OFFSET_X;
            iArrowY += SOUTH_OFFSET_Y;
          }
        } else if ((iDeltaA == -1) && (iDeltaB == MAP_WORLD_X)) {
          if (fZoomFlag) {
            iDirection = W_TO_N_ZOOM_LINE;
            if (fSpeedFlag)
              iArrow = ZOOM_Y_NORTH_ARROW;
            else
              iArrow = ZOOM_NORTH_ARROW;
            iArrowX += NORTH_OFFSET_X * 2;
            iArrowY += NORTH_OFFSET_Y * 2;
          } else {
            iDirection = W_TO_N_LINE;
            if (fSpeedFlag)
              iArrow = Y_NORTH_ARROW;
            else
              iArrow = NORTH_ARROW;
            iArrowX += NORTH_OFFSET_X;
            iArrowY += NORTH_OFFSET_Y;
          }
        } else if ((iDeltaA == -1) && (iDeltaB == -MAP_WORLD_X)) {
          if (fZoomFlag) {
            iDirection = W_TO_S_ZOOM_LINE;
            if (fSpeedFlag)
              iArrow = ZOOM_Y_SOUTH_ARROW;
            else
              iArrow = ZOOM_SOUTH_ARROW;
            iArrowX += SOUTH_OFFSET_X * 2;
            iArrowY += (SOUTH_OFFSET_Y + WEST_TO_SOUTH_OFFSET_Y) * 2;
          } else {
            iDirection = W_TO_S_LINE;
            if (fSpeedFlag)
              iArrow = Y_SOUTH_ARROW;
            else
              iArrow = SOUTH_ARROW;
            iArrowX += SOUTH_OFFSET_X;
            iArrowY += (SOUTH_OFFSET_Y + WEST_TO_SOUTH_OFFSET_Y);
          }
        } else if ((iDeltaA == -MAP_WORLD_X) && (iDeltaB == 1)) {
          if (fZoomFlag) {
            iDirection = N_TO_W_ZOOM_LINE;
            if (fSpeedFlag)
              iArrow = ZOOM_Y_WEST_ARROW;
            else
              iArrow = ZOOM_WEST_ARROW;
            iArrowX += WEST_OFFSET_X * 2;
            iArrowY += WEST_OFFSET_Y * 2;
          } else {
            iDirection = N_TO_W_LINE;
            if (fSpeedFlag)
              iArrow = Y_WEST_ARROW;
            else
              iArrow = WEST_ARROW;
            iArrowX += WEST_OFFSET_X;
            iArrowY += WEST_OFFSET_Y;
          }
        } else if ((iDeltaA == MAP_WORLD_X) && (iDeltaB == -1)) {
          if (fZoomFlag) {
            iDirection = S_TO_E_ZOOM_LINE;
            if (fSpeedFlag)
              iArrow = ZOOM_Y_EAST_ARROW;
            else
              iArrow = ZOOM_EAST_ARROW;
            iArrowX += EAST_OFFSET_X * 2;
            iArrowY += EAST_OFFSET_Y * 2;
          } else {
            iDirection = S_TO_E_LINE;
            if (fSpeedFlag)
              iArrow = Y_EAST_ARROW;
            else
              iArrow = EAST_ARROW;
            iArrowX += EAST_OFFSET_X;
            iArrowY += EAST_OFFSET_Y;
          }
        } else if ((iDeltaA == 1) && (iDeltaB == MAP_WORLD_X)) {
          if (fZoomFlag) {
            iDirection = E_TO_N_ZOOM_LINE;
            if (fSpeedFlag)
              iArrow = ZOOM_Y_NORTH_ARROW;
            else
              iArrow = ZOOM_NORTH_ARROW;
            iArrowX += (NORTH_OFFSET_X * 2);
            iArrowY += (NORTH_OFFSET_Y + EAST_TO_NORTH_OFFSET_Y) * 2;
          } else {
            iDirection = E_TO_N_LINE;
            if (fSpeedFlag)
              iArrow = Y_NORTH_ARROW;
            else
              iArrow = NORTH_ARROW;
            iArrowX += NORTH_OFFSET_X;
            iArrowY += NORTH_OFFSET_Y + EAST_TO_NORTH_OFFSET_Y;
          }
        }
      }
    } else {
      if (!fZoomFlag) {
        iX = (SectorID16_X(pNode->uiSectorId));
        iY = (SectorID16_Y(pNode->uiSectorId));
        iX = (iX * MAP_GRID_X) + MAP_VIEW_START_X;
        iY = (iY * MAP_GRID_Y) + MAP_VIEW_START_Y;

      } else {
        GetScreenXYFromMapXYStationary(((uint16_t)(SectorID16_X(pNode->uiSectorId))),
                                       ((uint16_t)(SectorID16_Y(pNode->uiSectorId))), &sX, &sY);
        iY = sY - MAP_GRID_Y;
        iX = sX - MAP_GRID_X;
      }
      iArrowX = iX;
      iArrowY = iY;
      if ((pNode->fSpeed))
        fSpeedFlag = FALSE;
      else
        fSpeedFlag = TRUE;
      // display enter and exit 'X's
      if (pPastNode) {
        fUTurnFlag = TRUE;
        iDeltaA = (int16_t)pNode->uiSectorId - (int16_t)pPastNode->uiSectorId;
        if (iDeltaA == -1) {
          if (fZoomFlag) {
            iDirection = ZOOM_RED_X_WEST;
            // iX-=MAP_GRID_X;
            // iY-=MAP_GRID_Y;
          } else
            iDirection = RED_X_WEST;
          // iX+=RED_WEST_OFF_X;
        } else if (iDeltaA == 1) {
          if (fZoomFlag) {
            iDirection = ZOOM_RED_X_EAST;
          } else
            iDirection = RED_X_EAST;
          // iX+=RED_EAST_OFF_X;
        } else if (iDeltaA == -MAP_WORLD_X) {
          if (fZoomFlag) {
            iDirection = ZOOM_RED_X_NORTH;
          } else
            iDirection = RED_X_NORTH;
          // iY+=RED_NORTH_OFF_Y;
        } else {
          if (fZoomFlag) {
            iDirection = ZOOM_RED_X_SOUTH;
          } else
            iDirection = RED_X_SOUTH;
          //	iY+=RED_SOUTH_OFF_Y;
        }
      }
      if (pNextNode) {
        fUTurnFlag = FALSE;
        iDeltaB = (int16_t)pNode->uiSectorId - (int16_t)pNextNode->uiSectorId;
        if ((pNode->fSpeed))
          fSpeedFlag = FALSE;
        else
          fSpeedFlag = TRUE;

        if (iDeltaB == -1) {
          if (fZoomFlag) {
            iDirection = ZOOM_GREEN_X_EAST;
            if (fSpeedFlag)
              iArrow = ZOOM_Y_EAST_ARROW;
            else
              iArrow = ZOOM_EAST_ARROW;
            iX -= 0;
            iY -= 0;
            iArrowX += EAST_OFFSET_X * 2;
            iArrowY += EAST_OFFSET_Y * 2;
          } else {
            iDirection = GREEN_X_EAST;
            if (fSpeedFlag)
              iArrow = Y_EAST_ARROW;
            else
              iArrow = EAST_ARROW;
            iArrowX += EAST_OFFSET_X;
            iArrowY += EAST_OFFSET_Y;
          }
          // iX+=RED_EAST_OFF_X;
        } else if (iDeltaB == 1) {
          if (fZoomFlag) {
            iDirection = ZOOM_GREEN_X_WEST;
            if (fSpeedFlag)
              iArrow = ZOOM_Y_WEST_ARROW;
            else
              iArrow = ZOOM_WEST_ARROW;
            iArrowX += WEST_OFFSET_X * 2;
            iArrowY += WEST_OFFSET_Y * 2;
          } else {
            iDirection = GREEN_X_WEST;
            if (fSpeedFlag)
              iArrow = Y_WEST_ARROW;
            else
              iArrow = WEST_ARROW;
            iArrowX += WEST_OFFSET_X;
            iArrowY += WEST_OFFSET_Y;
          }
          // iX+=RED_WEST_OFF_X;
        } else if (iDeltaB == MAP_WORLD_X) {
          if (fZoomFlag) {
            iDirection = ZOOM_GREEN_X_NORTH;
            if (fSpeedFlag)
              iArrow = ZOOM_Y_NORTH_ARROW;
            else
              iArrow = ZOOM_NORTH_ARROW;
            iArrowX += NORTH_OFFSET_X * 2;
            iArrowY += NORTH_OFFSET_Y * 2;
          } else {
            iDirection = GREEN_X_NORTH;
            if (fSpeedFlag)
              iArrow = Y_NORTH_ARROW;
            else
              iArrow = NORTH_ARROW;
            iArrowX += NORTH_OFFSET_X;
            iArrowY += NORTH_OFFSET_Y;
            // iY+=RED_NORTH_OFF_Y;
          }
        } else {
          if (fZoomFlag) {
            iDirection = ZOOM_GREEN_X_SOUTH;
            if (fSpeedFlag)
              iArrow = ZOOM_Y_SOUTH_ARROW;
            else
              iArrow = ZOOM_SOUTH_ARROW;
            iArrowX += SOUTH_OFFSET_X * 2;
            iArrowY += SOUTH_OFFSET_Y * 2;
          } else {
            iDirection = GREEN_X_SOUTH;
            if (fSpeedFlag)
              iArrow = Y_SOUTH_ARROW;
            else
              iArrow = SOUTH_ARROW;
            iArrowX += SOUTH_OFFSET_X;
            iArrowY += SOUTH_OFFSET_Y;
            // iY+=RED_SOUTH_OFF_Y;
          }
        }
      }
    }
    if ((iDirection != -1)) {
      if ((!fZoomFlag) ||
          ((fZoomFlag) && (iX > MAP_VIEW_START_X) && (iY > MAP_VIEW_START_Y) &&
           (iX < 640 - MAP_GRID_X * 2) && (iY < MAP_VIEW_START_Y + MAP_VIEW_HEIGHT))) {
        BltVideoObject(FRAME_BUFFER, hMapHandle, (uint16_t)iDirection, iX, iY,
                       VO_BLT_SRCTRANSPARENCY, NULL);

        if (!fUTurnFlag) {
          BltVideoObject(FRAME_BUFFER, hMapHandle, (uint16_t)iArrow, iArrowX, iArrowY,
                         VO_BLT_SRCTRANSPARENCY, NULL);
          InvalidateRegion(iArrowX, iArrowY, iArrowX + 2 * MAP_GRID_X, iArrowY + 2 * MAP_GRID_Y);
        }

        InvalidateRegion(iX, iY, iX + 2 * MAP_GRID_X, iY + 2 * MAP_GRID_Y);

        fUTurnFlag = FALSE;
      }
    }
    // check to see if there is a turn

    pPastNode = pNode;
    pNode = pNode->pNext;
    if (!pNode) return (FALSE);
    if (pNode->pNext)
      pNextNode = pNode->pNext;
    else
      pNextNode = NULL;
  }

  return (TRUE);
}

void AnimateRoute(struct path *pPath) {
  // set buffer
  SetFontDestBuffer(FRAME_BUFFER, 0, 0, 640, 480, FALSE);

  // the animated path
  if (TraceCharAnimatedRoute(pPath, FALSE, FALSE)) {
    // ARM? Huh?  Why the same thing twice more?
    TraceCharAnimatedRoute(pPath, FALSE, TRUE);
    TraceCharAnimatedRoute(pPath, FALSE, TRUE);
  }
}

void RestoreArrowBackgroundsForTrace(int32_t iArrow, int32_t iArrowX, int32_t iArrowY,
                                     BOOLEAN fZoom) {
  int16_t sArrow = 0;
  int32_t iX = -1, iY = -1;
  // find location of arrow and restore appropriate background

  if ((iArrow == SOUTH_ARROW) || (iArrow == W_SOUTH_ARROW) || (iArrow == ZOOM_W_SOUTH_ARROW) ||
      (ZOOM_SOUTH_ARROW == iArrow)) {
    sArrow = SOUTH_ARROW;
  } else if ((iArrow == NORTH_ARROW) || (iArrow == W_NORTH_ARROW) ||
             (iArrow == ZOOM_W_NORTH_ARROW) || (ZOOM_NORTH_ARROW == iArrow)) {
    sArrow = NORTH_ARROW;
  } else if ((iArrow == WEST_ARROW) || (iArrow == W_WEST_ARROW) || (iArrow == ZOOM_W_WEST_ARROW) ||
             (ZOOM_WEST_ARROW == iArrow)) {
    sArrow = WEST_ARROW;
  } else if ((iArrow == EAST_ARROW) || (iArrow == W_EAST_ARROW) || (iArrow == ZOOM_W_EAST_ARROW) ||
             (ZOOM_EAST_ARROW == iArrow)) {
    sArrow = EAST_ARROW;
  }

  switch (sArrow) {
    case (SOUTH_ARROW):
      iX = iArrowX;
      iY = iArrowY;
      break;
    case (NORTH_ARROW):
      iX = iArrowX;
      iY = iArrowY;
      break;
    case (WEST_ARROW):
      iX = iArrowX;
      iY = iArrowY;
      break;
    case (EAST_ARROW):
      iX = iArrowX;
      iY = iArrowY;
      break;
  }

  // error check
  if (iX == -1) {
    return;
  }

  if (!fZoom)
    RestoreExternBackgroundRect(((int16_t)iX), ((int16_t)iY), DMAP_GRID_X / 2, DMAP_GRID_Y / 2);
  else
    RestoreExternBackgroundRect(((int16_t)iX), ((int16_t)iY), DMAP_GRID_ZOOM_X, DMAP_GRID_ZOOM_Y);

  return;
}

BOOLEAN TraceCharAnimatedRoute(struct path *pPath, BOOLEAN fCheckFlag, BOOLEAN fForceUpDate) {
  static struct path *pCurrentNode = NULL;
  static BOOLEAN fUpDateFlag = FALSE;
  static BOOLEAN fPauseFlag = TRUE;
  static uint8_t ubCounter = 1;

  struct VObject *hMapHandle;
  BOOLEAN fSpeedFlag = FALSE;
  int32_t iDifference = 0;
  int32_t iArrow = -1;
  int32_t iX = 0, iY = 0;
  int32_t iPastX, iPastY;
  int16_t sX = 0, sY = 0;
  int32_t iArrowX, iArrowY;
  int32_t iDeltaA, iDeltaB, iDeltaB1;
  int32_t iDirection = -1;
  BOOLEAN fUTurnFlag = FALSE;
  BOOLEAN fNextNode = FALSE;
  struct path *pTempNode = NULL;
  struct path *pNode = NULL;
  struct path *pPastNode = NULL;
  struct path *pNextNode = NULL;

  // must be plotting movement
  if ((bSelectedDestChar == -1) && (fPlotForHelicopter == FALSE)) {
    return FALSE;
  }

  // if any nodes have been deleted, reset current node to beginning of the list
  if (fDeletedNode) {
    fDeletedNode = FALSE;
    pCurrentNode = NULL;
  }

  // Valid path?
  if (pPath == NULL) {
    return FALSE;
  } else {
    if (pCurrentNode == NULL) {
      pCurrentNode = pPath;
    }
  }

  // Check Timer
  if (giAnimateRouteBaseTime == 0) {
    giAnimateRouteBaseTime = GetJA2Clock();
    return FALSE;
  }

  // check difference in time
  iDifference = GetJA2Clock() - giAnimateRouteBaseTime;

  // if pause flag, check time, if time passed, reset, continue on, else return
  if (fPauseFlag) {
    if (iDifference < PAUSE_DELAY) {
      return FALSE;
    } else {
      fPauseFlag = FALSE;
      giAnimateRouteBaseTime = GetJA2Clock();
    }
  }

  // if is checkflag and change in status, return TRUE;
  if (!fForceUpDate) {
    if (iDifference < ARROW_DELAY) {
      if (!fUpDateFlag) return FALSE;
    } else {
      // sufficient time, update base time
      giAnimateRouteBaseTime = GetJA2Clock();
      fUpDateFlag = !fUpDateFlag;

      if (fCheckFlag) return TRUE;

      fNextNode = TRUE;
    }
  }

  // check to see if Current node has not been deleted
  pTempNode = pPath;

  while (pTempNode) {
    if (pTempNode == pCurrentNode) {
      // not deleted
      // reset pause flag
      break;
    } else
      pTempNode = pTempNode->pNext;
  }

  // if deleted, restart at beginnning
  if (pTempNode == NULL) {
    pCurrentNode = pPath;

    // set pause flag
    if (!pCurrentNode) return FALSE;
  }

  // Grab Video Objects
  GetVideoObject(&hMapHandle, guiMAPCURSORS);

  // Handle drawing of arrow
  pNode = pCurrentNode;
  if ((!pNode->pPrev) && (ubCounter == 1) && (fForceUpDate)) {
    ubCounter = 0;
    return FALSE;
  } else if ((ubCounter == 1) && (fForceUpDate)) {
    pNode = pCurrentNode->pPrev;
  }
  if (pNode->pNext)
    pNextNode = pNode->pNext;
  else
    pNextNode = NULL;

  if (pNode->pPrev)
    pPastNode = pNode->pPrev;
  else
    pPastNode = NULL;

  // go through characters list and display arrows for path
  fUTurnFlag = FALSE;
  if ((pPastNode) && (pNextNode)) {
    iDeltaA = (int16_t)pNode->uiSectorId - (int16_t)pPastNode->uiSectorId;
    iDeltaB = (int16_t)pNode->uiSectorId - (int16_t)pNextNode->uiSectorId;
    if (!pNode->fSpeed)
      fSpeedFlag = TRUE;
    else
      fSpeedFlag = FALSE;
    if (iDeltaA == 0) return FALSE;
    if (!fZoomFlag) {
      iX = (SectorID16_X(pNode->uiSectorId));
      iY = (SectorID16_Y(pNode->uiSectorId));
      iX = (iX * MAP_GRID_X) + MAP_VIEW_START_X;
      iY = (iY * MAP_GRID_Y) + MAP_VIEW_START_Y;
    } else {
      GetScreenXYFromMapXYStationary(((uint16_t)(SectorID16_X(pNode->uiSectorId))),
                                     ((uint16_t)(SectorID16_Y(pNode->uiSectorId))), &sX, &sY);
      iY = sY - MAP_GRID_Y;
      iX = sX - MAP_GRID_X;
    }
    iArrowX = iX;
    iArrowY = iY;
    if ((pPastNode->pPrev) && (pNextNode->pNext)) {
      fUTurnFlag = FALSE;
      // check to see if out-of sector U-turn
      // for placement of arrows
      iDeltaB1 = pNextNode->uiSectorId - pNextNode->pNext->uiSectorId;
      if ((iDeltaB1 == -MAP_WORLD_X) && (iDeltaA == -MAP_WORLD_X) && (iDeltaB == -1)) {
        fUTurnFlag = TRUE;
      } else if ((iDeltaB1 == -MAP_WORLD_X) && (iDeltaA == -MAP_WORLD_X) && (iDeltaB == 1)) {
        fUTurnFlag = TRUE;
      } else if ((iDeltaB1 == MAP_WORLD_X) && (iDeltaA == MAP_WORLD_X) && (iDeltaB == 1)) {
        fUTurnFlag = TRUE;
      } else if ((iDeltaB1 == -MAP_WORLD_X) && (iDeltaA == -MAP_WORLD_X) && (iDeltaB == 1)) {
        fUTurnFlag = TRUE;
      } else if ((iDeltaB1 == -1) && (iDeltaA == -1) && (iDeltaB == -MAP_WORLD_X)) {
        fUTurnFlag = TRUE;
      } else if ((iDeltaB1 == -1) && (iDeltaA == -1) && (iDeltaB == MAP_WORLD_X)) {
        fUTurnFlag = TRUE;
      } else if ((iDeltaB1 == 1) && (iDeltaA == 1) && (iDeltaB == -MAP_WORLD_X)) {
        fUTurnFlag = TRUE;
      } else if ((iDeltaB1 == 1) && (iDeltaA == 1) && (iDeltaB == MAP_WORLD_X)) {
        fUTurnFlag = TRUE;
      } else
        fUTurnFlag = FALSE;
    }

    if ((pPastNode->uiSectorId == pNextNode->uiSectorId)) {
      if (pPastNode->uiSectorId + MAP_WORLD_X == pNode->uiSectorId) {
        if (fZoomFlag) {
          iDirection = S_TO_N_ZOOM_LINE;
          if (!ubCounter)
            iArrow = ZOOM_W_NORTH_ARROW;
          else if (fSpeedFlag)
            iArrow = ZOOM_Y_NORTH_ARROW;
          else
            iArrow = ZOOM_NORTH_ARROW;
          iArrowX += NORTH_OFFSET_X * 2;
          iArrowY += NORTH_OFFSET_Y * 2;
        } else {
          iDirection = S_TO_N_LINE;
          if (!ubCounter)
            iArrow = W_NORTH_ARROW;
          else if (fSpeedFlag)
            iArrow = Y_NORTH_ARROW;
          else
            iArrow = NORTH_ARROW;

          iArrowX += NORTH_OFFSET_X;
          iArrowY += NORTH_OFFSET_Y;
        }
      } else if (pPastNode->uiSectorId - MAP_WORLD_X == pNode->uiSectorId) {
        if (fZoomFlag) {
          iDirection = N_TO_S_ZOOM_LINE;
          if (!ubCounter)
            iArrow = ZOOM_W_SOUTH_ARROW;
          else if (fSpeedFlag)
            iArrow = ZOOM_Y_SOUTH_ARROW;
          else
            iArrow = ZOOM_SOUTH_ARROW;
          iArrowX += SOUTH_OFFSET_X * 2;
          iArrowY += SOUTH_OFFSET_Y * 2;
        } else {
          iDirection = N_TO_S_LINE;
          if (!ubCounter)
            iArrow = W_SOUTH_ARROW;
          else if (fSpeedFlag)
            iArrow = Y_SOUTH_ARROW;
          else
            iArrow = SOUTH_ARROW;
          iArrowX += SOUTH_OFFSET_X;
          iArrowY += SOUTH_OFFSET_Y;
        }
      } else if (pPastNode->uiSectorId + 1 == pNode->uiSectorId) {
        if (fZoomFlag) {
          iDirection = E_TO_W_ZOOM_LINE;
          if (!ubCounter)
            iArrow = ZOOM_W_WEST_ARROW;
          else if (fSpeedFlag)
            iArrow = ZOOM_Y_WEST_ARROW;
          else
            iArrow = ZOOM_WEST_ARROW;
          iArrowX += WEST_OFFSET_X * 2;
          iArrowY += WEST_OFFSET_Y * 2;
        } else {
          iDirection = E_TO_W_LINE;
          if (!ubCounter)
            iArrow = W_WEST_ARROW;
          else if (fSpeedFlag)
            iArrow = Y_WEST_ARROW;
          else
            iArrow = WEST_ARROW;
          iArrowX += WEST_OFFSET_X;
          iArrowY += WEST_OFFSET_Y;
        }
      } else {
        if (fZoomFlag) {
          iDirection = W_TO_E_ZOOM_LINE;
          if (!ubCounter)
            iArrow = ZOOM_W_EAST_ARROW;
          else if (fSpeedFlag)
            iArrow = ZOOM_Y_EAST_ARROW;
          else
            iArrow = ZOOM_EAST_ARROW;
          iArrowX += EAST_OFFSET_X * 2;
          iArrowY += EAST_OFFSET_Y * 2;
        } else {
          iDirection = W_TO_E_LINE;
          if (!ubCounter)
            iArrow = W_EAST_ARROW;
          else if (fSpeedFlag)
            iArrow = Y_EAST_ARROW;
          else
            iArrow = EAST_ARROW;
          iArrowX += EAST_OFFSET_X;
          iArrowY += EAST_OFFSET_Y;
        }
      }
    } else {
      if ((iDeltaA == -1) && (iDeltaB == 1)) {
        if (fZoomFlag) {
          iDirection = WEST_ZOOM_LINE;
          if (!ubCounter)
            iArrow = ZOOM_W_WEST_ARROW;
          else if (fSpeedFlag)
            iArrow = ZOOM_Y_WEST_ARROW;
          else
            iArrow = ZOOM_WEST_ARROW;

          iArrowX += WEST_OFFSET_X * 2;
          iArrowY += WEST_OFFSET_Y * 2;
        } else {
          iDirection = WEST_LINE;
          if (!ubCounter)
            iArrow = W_WEST_ARROW;
          else if (fSpeedFlag)
            iArrow = Y_WEST_ARROW;
          else
            iArrow = WEST_ARROW;

          iArrowX += WEST_OFFSET_X;
          iArrowY += WEST_OFFSET_Y;
        }
      } else if ((iDeltaA == 1) && (iDeltaB == -1)) {
        if (fZoomFlag) {
          iDirection = EAST_ZOOM_LINE;
          if (!ubCounter)
            iArrow = ZOOM_W_EAST_ARROW;
          else
            iArrow = ZOOM_EAST_ARROW;

          iArrowX += EAST_OFFSET_X * 2;
          iArrowY += EAST_OFFSET_Y * 2;
        } else {
          iDirection = EAST_LINE;
          if (!ubCounter)
            iArrow = W_EAST_ARROW;
          else if (fSpeedFlag)
            iArrow = Y_EAST_ARROW;
          else
            iArrow = EAST_ARROW;

          iArrowX += EAST_OFFSET_X;
          iArrowY += EAST_OFFSET_Y;
        }
      } else if ((iDeltaA == -MAP_WORLD_X) && (iDeltaB == MAP_WORLD_X)) {
        if (fZoomFlag) {
          iDirection = NORTH_ZOOM_LINE;
          if (!ubCounter)
            iArrow = ZOOM_W_NORTH_ARROW;
          else if (fSpeedFlag)
            iArrow = ZOOM_Y_NORTH_ARROW;
          else
            iArrow = ZOOM_NORTH_ARROW;

          iArrowX += NORTH_OFFSET_X * 2;
          iArrowY += NORTH_OFFSET_Y * 2;
        } else {
          iDirection = NORTH_LINE;
          if (!ubCounter)
            iArrow = W_NORTH_ARROW;
          else if (fSpeedFlag)
            iArrow = Y_NORTH_ARROW;
          else
            iArrow = NORTH_ARROW;

          iArrowX += NORTH_OFFSET_X;
          iArrowY += NORTH_OFFSET_Y;
        }
      } else if ((iDeltaA == MAP_WORLD_X) && (iDeltaB == -MAP_WORLD_X)) {
        if (fZoomFlag) {
          iDirection = SOUTH_ZOOM_LINE;
          if (!ubCounter)
            iArrow = ZOOM_W_SOUTH_ARROW;
          else if (fSpeedFlag)
            iArrow = ZOOM_Y_SOUTH_ARROW;
          else
            iArrow = ZOOM_SOUTH_ARROW;

          iArrowX += SOUTH_OFFSET_X * 2;
          iArrowY += SOUTH_OFFSET_Y * 2;
        } else {
          iDirection = SOUTH_LINE;
          if (!ubCounter)
            iArrow = W_SOUTH_ARROW;
          else if (fSpeedFlag)
            iArrow = Y_SOUTH_ARROW;
          else
            iArrow = SOUTH_ARROW;

          iArrowX += SOUTH_OFFSET_X;
          iArrowY += SOUTH_OFFSET_Y;
        }
      } else if ((iDeltaA == -MAP_WORLD_X) && (iDeltaB == -1)) {
        if (fZoomFlag) {
          iDirection = N_TO_E_ZOOM_LINE;
          if (!ubCounter)
            iArrow = ZOOM_W_EAST_ARROW;
          else if (fSpeedFlag)
            iArrow = ZOOM_Y_EAST_ARROW;
          else
            iArrow = ZOOM_EAST_ARROW;

          iArrowX += EAST_OFFSET_X * 2;
          iArrowY += EAST_OFFSET_Y * 2;
        } else {
          iDirection = N_TO_E_LINE;
          if (!ubCounter)
            iArrow = W_EAST_ARROW;
          else if (fSpeedFlag)
            iArrow = Y_EAST_ARROW;
          else
            iArrow = EAST_ARROW;

          iArrowX += EAST_OFFSET_X;
          iArrowY += EAST_OFFSET_Y;
        }
      } else if ((iDeltaA == MAP_WORLD_X) && (iDeltaB == 1)) {
        if (fZoomFlag) {
          iDirection = S_TO_W_ZOOM_LINE;
          if (!ubCounter)
            iArrow = ZOOM_W_WEST_ARROW;
          else if (fSpeedFlag)
            iArrow = ZOOM_Y_WEST_ARROW;
          else
            iArrow = ZOOM_WEST_ARROW;

          iArrowX += WEST_OFFSET_X * 2;
          iArrowY += WEST_OFFSET_Y * 2;
        } else {
          iDirection = S_TO_W_LINE;
          if (!ubCounter)
            iArrow = W_WEST_ARROW;
          else if (fSpeedFlag)
            iArrow = Y_WEST_ARROW;
          else
            iArrow = WEST_ARROW;

          iArrowX += WEST_OFFSET_X;
          iArrowY += WEST_OFFSET_Y;
        }
      } else if ((iDeltaA == 1) && (iDeltaB == -MAP_WORLD_X)) {
        if (fZoomFlag) {
          iDirection = E_TO_S_ZOOM_LINE;
          if (!ubCounter)
            iArrow = ZOOM_W_SOUTH_ARROW;
          else if (fSpeedFlag)
            iArrow = ZOOM_Y_SOUTH_ARROW;
          else
            iArrow = ZOOM_SOUTH_ARROW;

          iArrowX += SOUTH_OFFSET_X * 2;
          iArrowY += SOUTH_OFFSET_Y * 2;
        } else {
          iDirection = E_TO_S_LINE;
          if (!ubCounter)
            iArrow = W_SOUTH_ARROW;
          else if (fSpeedFlag)
            iArrow = Y_SOUTH_ARROW;
          else
            iArrow = SOUTH_ARROW;

          iArrowX += SOUTH_OFFSET_X;
          iArrowY += SOUTH_OFFSET_Y;
        }
      } else if ((iDeltaA == -1) && (iDeltaB == MAP_WORLD_X)) {
        if (fZoomFlag) {
          iDirection = W_TO_N_ZOOM_LINE;
          if (!ubCounter)
            iArrow = ZOOM_W_NORTH_ARROW;
          else if (fSpeedFlag)
            iArrow = ZOOM_Y_NORTH_ARROW;
          else
            iArrow = ZOOM_NORTH_ARROW;

          iArrowX += NORTH_OFFSET_X * 2;
          iArrowY += NORTH_OFFSET_Y * 2;
        } else {
          iDirection = W_TO_N_LINE;
          if (!ubCounter)
            iArrow = W_NORTH_ARROW;
          else if (fSpeedFlag)
            iArrow = Y_NORTH_ARROW;
          else
            iArrow = NORTH_ARROW;

          iArrowX += NORTH_OFFSET_X;
          iArrowY += NORTH_OFFSET_Y;
        }
      } else if ((iDeltaA == -1) && (iDeltaB == -MAP_WORLD_X)) {
        if (fZoomFlag) {
          iDirection = W_TO_S_ZOOM_LINE;
          if (!ubCounter)
            iArrow = ZOOM_W_SOUTH_ARROW;
          else if (fSpeedFlag)
            iArrow = ZOOM_Y_SOUTH_ARROW;
          else
            iArrow = ZOOM_SOUTH_ARROW;

          iArrowX += SOUTH_OFFSET_X * 2;
          iArrowY += (SOUTH_OFFSET_Y + WEST_TO_SOUTH_OFFSET_Y) * 2;
        } else {
          iDirection = W_TO_S_LINE;
          if (!ubCounter)
            iArrow = W_SOUTH_ARROW;
          else if (fSpeedFlag)
            iArrow = Y_SOUTH_ARROW;
          else
            iArrow = SOUTH_ARROW;
          iArrowX += SOUTH_OFFSET_X;
          iArrowY += (SOUTH_OFFSET_Y + WEST_TO_SOUTH_OFFSET_Y);
        }
      } else if ((iDeltaA == -MAP_WORLD_X) && (iDeltaB == 1)) {
        if (fZoomFlag) {
          iDirection = N_TO_W_ZOOM_LINE;
          if (!ubCounter)
            iArrow = ZOOM_W_WEST_ARROW;
          else if (fSpeedFlag)
            iArrow = ZOOM_Y_WEST_ARROW;
          else
            iArrow = ZOOM_WEST_ARROW;

          iArrowX += WEST_OFFSET_X * 2;
          iArrowY += WEST_OFFSET_Y * 2;
        } else {
          iDirection = N_TO_W_LINE;
          if (!ubCounter)
            iArrow = W_WEST_ARROW;
          else if (fSpeedFlag)
            iArrow = Y_WEST_ARROW;
          else
            iArrow = WEST_ARROW;

          iArrowX += WEST_OFFSET_X;
          iArrowY += WEST_OFFSET_Y;
        }
      } else if ((iDeltaA == MAP_WORLD_X) && (iDeltaB == -1)) {
        if (fZoomFlag) {
          iDirection = S_TO_E_ZOOM_LINE;
          if (!ubCounter)
            iArrow = ZOOM_W_EAST_ARROW;
          else if (fSpeedFlag)
            iArrow = ZOOM_Y_EAST_ARROW;
          else
            iArrow = ZOOM_EAST_ARROW;
          iArrowX += EAST_OFFSET_X * 2;
          iArrowY += EAST_OFFSET_Y * 2;
        } else {
          iDirection = S_TO_E_LINE;
          if (!ubCounter)
            iArrow = W_EAST_ARROW;
          else if (fSpeedFlag)
            iArrow = Y_EAST_ARROW;
          else
            iArrow = EAST_ARROW;
          iArrowX += EAST_OFFSET_X;
          iArrowY += EAST_OFFSET_Y;
        }
      } else if ((iDeltaA == 1) && (iDeltaB == MAP_WORLD_X)) {
        if (fZoomFlag) {
          iDirection = E_TO_N_ZOOM_LINE;
          if (!ubCounter)
            iArrow = ZOOM_W_NORTH_ARROW;
          else if (fSpeedFlag)
            iArrow = ZOOM_Y_NORTH_ARROW;
          else
            iArrow = ZOOM_NORTH_ARROW;
          iArrowX += (NORTH_OFFSET_X * 2);
          iArrowY += (NORTH_OFFSET_Y + EAST_TO_NORTH_OFFSET_Y) * 2;
        } else {
          iDirection = E_TO_N_LINE;
          if (!ubCounter)
            iArrow = W_NORTH_ARROW;
          else if (fSpeedFlag)
            iArrow = Y_NORTH_ARROW;
          else
            iArrow = NORTH_ARROW;

          iArrowX += NORTH_OFFSET_X;
          iArrowY += NORTH_OFFSET_Y + EAST_TO_NORTH_OFFSET_Y;
        }
      }
    }

  }

  else {
    iX = (SectorID16_X(pNode->uiSectorId));
    iY = (SectorID16_Y(pNode->uiSectorId));
    iX = (iX * MAP_GRID_X) + MAP_VIEW_START_X;
    iY = (iY * MAP_GRID_Y) + MAP_VIEW_START_Y;
    if (pPastNode) {
      iPastX = (SectorID16_X(pPastNode->uiSectorId));
      iPastY = (SectorID16_Y(pPastNode->uiSectorId));
      iPastX = (iPastX * MAP_GRID_X) + MAP_VIEW_START_X;
      iPastY = (iPastY * MAP_GRID_Y) + MAP_VIEW_START_Y;
    }
    if (pNode->fSpeed)
      fSpeedFlag = TRUE;
    else
      fSpeedFlag = FALSE;
    iArrowX = iX;
    iArrowY = iY;
    // display enter and exit 'X's
    if (pPastNode) {
      // red 'X'
      fUTurnFlag = TRUE;
      iDeltaA = (int16_t)pNode->uiSectorId - (int16_t)pPastNode->uiSectorId;
      if (iDeltaA == -1) {
        iDirection = RED_X_WEST;
        // iX+=RED_WEST_OFF_X;
      } else if (iDeltaA == 1) {
        iDirection = RED_X_EAST;
        // iX+=RED_EAST_OFF_X;
      } else if (iDeltaA == -MAP_WORLD_X) {
        iDirection = RED_X_NORTH;
        // iY+=RED_NORTH_OFF_Y;
      } else {
        iDirection = RED_X_SOUTH;
        //	iY+=RED_SOUTH_OFF_Y;
      }
    }
    if (pNextNode) {
      fUTurnFlag = FALSE;
      iDeltaB = (int16_t)pNode->uiSectorId - (int16_t)pNextNode->uiSectorId;
      if (iDeltaB == -1) {
        iDirection = GREEN_X_EAST;
        if (!ubCounter)
          iArrow = W_EAST_ARROW;
        else if (fSpeedFlag)
          iArrow = Y_EAST_ARROW;
        else
          iArrow = EAST_ARROW;

        iArrowX += EAST_OFFSET_X;
        iArrowY += EAST_OFFSET_Y;
        // iX+=RED_EAST_OFF_X;
      } else if (iDeltaB == 1) {
        iDirection = GREEN_X_WEST;
        if (!ubCounter)
          iArrow = W_WEST_ARROW;
        else if (fSpeedFlag)
          iArrow = Y_WEST_ARROW;
        else
          iArrow = WEST_ARROW;

        iArrowX += WEST_OFFSET_X;
        iArrowY += WEST_OFFSET_Y;
        // iX+=RED_WEST_OFF_X;
      } else if (iDeltaB == MAP_WORLD_X) {
        iDirection = GREEN_X_NORTH;
        if (!ubCounter)
          iArrow = W_NORTH_ARROW;
        else if (fSpeedFlag)
          iArrow = Y_NORTH_ARROW;
        else
          iArrow = NORTH_ARROW;

        iArrowX += NORTH_OFFSET_X;
        iArrowY += NORTH_OFFSET_Y;
        // iY+=RED_NORTH_OFF_Y;
      } else {
        iDirection = GREEN_X_SOUTH;
        if (!ubCounter)
          iArrow = W_SOUTH_ARROW;
        else if (fSpeedFlag)
          iArrow = Y_SOUTH_ARROW;
        else
          iArrow = SOUTH_ARROW;
        iArrowX += SOUTH_OFFSET_X;
        iArrowY += SOUTH_OFFSET_Y;
        // iY+=RED_SOUTH_OFF_Y;
      }
    }
  }
  if (fNextNode) {
    if (!ubCounter) {
      pCurrentNode = pCurrentNode->pNext;
      if (!pCurrentNode) fPauseFlag = TRUE;
    }
  }
  if ((iDirection != -1) && (iArrow != -1)) {
    if (!fUTurnFlag) {
      if ((!fZoomFlag) ||
          ((fZoomFlag) && (iX > MAP_VIEW_START_X) && (iY > MAP_VIEW_START_Y) &&
           (iX < 640 - MAP_GRID_X * 2) && (iY < MAP_VIEW_START_Y + MAP_VIEW_HEIGHT))) {
        // if(!fZoomFlag)
        // RestoreExternBackgroundRect(((int16_t)iArrowX),((int16_t)iArrowY),DMAP_GRID_X,
        // DMAP_GRID_Y); else RestoreExternBackgroundRect(((int16_t)iArrowX),
        // ((int16_t)iArrowY),DMAP_GRID_ZOOM_X, DMAP_GRID_ZOOM_Y);
        if (pNode != pPath) {
          BltVideoObject(FRAME_BUFFER, hMapHandle, (uint16_t)iArrow, iArrowX, iArrowY,
                         VO_BLT_SRCTRANSPARENCY, NULL);
          InvalidateRegion(iArrowX, iArrowY, iArrowX + 2 * MAP_GRID_X, iArrowY + 2 * MAP_GRID_Y);
        }
      }
      if (ubCounter == 1)
        ubCounter = 0;
      else
        ubCounter = 1;
      return TRUE;
    }
    if (ubCounter == 1)
      ubCounter = 0;
    else
      ubCounter = 1;
  }
  // move to next arrow

  // ARM who knows what it should return here?
  return FALSE;
}

void DisplayThePotentialPathForHelicopter(uint8_t sMapX, uint8_t sMapY) {
  // simply check if we want to refresh the screen to display path
  static BOOLEAN fOldShowAirCraft = FALSE;
  static int16_t sOldMapX, sOldMapY;
  int32_t iDifference = 0;

  if (fOldShowAirCraft != fShowAircraftFlag) {
    fOldShowAirCraft = fShowAircraftFlag;
    giPotHeliPathBaseTime = GetJA2Clock();

    sOldMapX = sMapX;
    sOldMapY = sMapY;
    fTempPathAlreadyDrawn = FALSE;
    fDrawTempHeliPath = FALSE;
  }

  if ((sMapX != sOldMapX) || (sMapY != sOldMapY)) {
    giPotHeliPathBaseTime = GetJA2Clock();

    sOldMapX = sMapX;
    sOldMapY = sMapY;

    // path was plotted and we moved, re draw map..to clean up mess
    if (fTempPathAlreadyDrawn) {
      MarkForRedrawalStrategicMap();
    }

    fTempPathAlreadyDrawn = FALSE;
    fDrawTempHeliPath = FALSE;
  }

  iDifference = GetJA2Clock() - giPotHeliPathBaseTime;

  if (fTempPathAlreadyDrawn) {
    return;
  }

  if (iDifference > MIN_WAIT_TIME_FOR_TEMP_PATH) {
    fDrawTempHeliPath = TRUE;
    giPotHeliPathBaseTime = GetJA2Clock();
    fTempPathAlreadyDrawn = TRUE;
  }

  return;
}

BOOLEAN IsTheCursorAllowedToHighLightThisSector(uint8_t sSectorX, uint8_t sSectorY) {
  // check to see if this sector is a blocked out sector?

  if (sBadSectorsList[sSectorX][sSectorY]) {
    return (FALSE);
  } else {
    // return cursor is allowed to highlight this sector
    return (TRUE);
  }
}

void SetUpBadSectorsList(void) {
  // initalizes all sectors to highlighable and then the ones non highlightable are marked as such
  int8_t bY;

  memset(&sBadSectorsList, 0, sizeof(sBadSectorsList));

  // the border regions
  for (bY = 0; bY < MAP_WORLD_X; bY++) {
    sBadSectorsList[0][bY] = sBadSectorsList[MAP_WORLD_X - 1][bY] = sBadSectorsList[bY][0] =
        sBadSectorsList[bY][MAP_WORLD_X - 1] = TRUE;
  }

  sBadSectorsList[4][1] = TRUE;
  sBadSectorsList[5][1] = TRUE;
  sBadSectorsList[16][1] = TRUE;
  sBadSectorsList[16][5] = TRUE;
  sBadSectorsList[16][6] = TRUE;

  sBadSectorsList[16][10] = TRUE;
  sBadSectorsList[16][11] = TRUE;
  sBadSectorsList[16][12] = TRUE;
  sBadSectorsList[16][13] = TRUE;
  sBadSectorsList[16][14] = TRUE;
  sBadSectorsList[16][15] = TRUE;
  sBadSectorsList[16][16] = TRUE;

  sBadSectorsList[15][13] = TRUE;
  sBadSectorsList[15][14] = TRUE;
  sBadSectorsList[15][15] = TRUE;
  sBadSectorsList[15][16] = TRUE;

  sBadSectorsList[14][14] = TRUE;
  sBadSectorsList[14][15] = TRUE;
  sBadSectorsList[14][16] = TRUE;

  sBadSectorsList[13][14] = TRUE;
  return;
}

void RestoreBackgroundForMapGrid(uint8_t sMapX, uint8_t sMapY) {
  int16_t sX, sY;

  if (!fZoomFlag) {
    // screen values
    sX = (sMapX * MAP_GRID_X) + MAP_VIEW_START_X;
    sY = (sMapY * MAP_GRID_Y) + MAP_VIEW_START_Y;

    // restore background
    RestoreExternBackgroundRect(sX, sY, DMAP_GRID_X, DMAP_GRID_Y);
  } else {
    // get screen coords from map values
    GetScreenXYFromMapXYStationary(sMapX, sMapY, &sX, &sY);

    // is this on the screen?
    if ((sX > MapScreenRect.iLeft) && (sX < MapScreenRect.iRight) && (sY > MapScreenRect.iTop) &&
        (sY < MapScreenRect.iBottom)) {
      // offset
      sY = sY - MAP_GRID_Y;
      sX = sX - MAP_GRID_X;

      // restore
      RestoreExternBackgroundRect(sX, sY, DMAP_GRID_ZOOM_X, DMAP_GRID_ZOOM_Y);
    }
  }
}

void ClipBlitsToMapViewRegion(void) {
  // the standard mapscreen rectangle doesn't work for clipping while zoomed...
  SGPRect ZoomedMapScreenClipRect = {MAP_VIEW_START_X + MAP_GRID_X,
                                     MAP_VIEW_START_Y + MAP_GRID_Y - 1,
                                     MAP_VIEW_START_X + MAP_VIEW_WIDTH + MAP_GRID_X,
                                     MAP_VIEW_START_Y + MAP_VIEW_HEIGHT + MAP_GRID_Y - 10};
  SGPRect *pRectToUse;

  if (fZoomFlag)
    pRectToUse = &ZoomedMapScreenClipRect;
  else
    pRectToUse = &MapScreenRect;

  SetClippingRect(pRectToUse);
  memcpy(&gOldClipRect, &gDirtyClipRect, sizeof(gOldClipRect));
  memcpy(&gDirtyClipRect, pRectToUse, sizeof(gDirtyClipRect));
}

void RestoreClipRegionToFullScreen(void) {
  SetClippingRect(&FullScreenRect);
  memcpy(&gDirtyClipRect, &gOldClipRect, sizeof(gDirtyClipRect));
}

void ClipBlitsToMapViewRegionForRectangleAndABit(uint32_t uiDestPitchBYTES) {
  // clip blits to map view region
  // because MC's map coordinates system is so screwy, these had to be hand-tuned to work right...
  // ARM
  if (fZoomFlag)
    SetClippingRegionAndImageWidth(uiDestPitchBYTES, MapScreenRect.iLeft + 2, MapScreenRect.iTop,
                                   MapScreenRect.iRight - MapScreenRect.iLeft,
                                   MapScreenRect.iBottom - MapScreenRect.iTop);
  else
    SetClippingRegionAndImageWidth(uiDestPitchBYTES, MapScreenRect.iLeft - 1,
                                   MapScreenRect.iTop - 1,
                                   MapScreenRect.iRight - MapScreenRect.iLeft + 3,
                                   MapScreenRect.iBottom - MapScreenRect.iTop + 2);

  return;
}

void RestoreClipRegionToFullScreenForRectangle(uint32_t uiDestPitchBYTES) {
  // clip blits to map view region
  SetClippingRegionAndImageWidth(uiDestPitchBYTES, 0, 0, 640, 480);

  return;
}

// mvt icon offset defines
#define SOUTH_Y_MVT_OFFSET +10
#define SOUTH_X_MVT_OFFSET 0
#define NORTH_Y_MVT_OFFSET -10
#define NORTH_X_MVT_OFFSET +10
#define NORTH_SOUTH_CENTER_OFFSET +5

#define SOUTH_Y_MVT_OFFSET_ZOOM +30
#define SOUTH_X_MVT_OFFSET_ZOOM +5
#define NORTH_Y_MVT_OFFSET_ZOOM -8
#define NORTH_X_MVT_OFFSET_ZOOM +25
#define NORTH_SOUTH_CENTER_OFFSET_ZOOM +15

#define EAST_Y_MVT_OFFSET +8
#define EAST_X_MVT_OFFSET 15
#define WEST_Y_MVT_OFFSET -2
#define WEST_X_MVT_OFFSET -8
#define EAST_WEST_CENTER_OFFSET +2

#define EAST_Y_MVT_OFFSET_ZOOM +24
#define EAST_X_MVT_OFFSET_ZOOM +36
#define WEST_Y_MVT_OFFSET_ZOOM +5
#define WEST_X_MVT_OFFSET_ZOOM -10
#define EAST_WEST_CENTER_OFFSET_ZOOM +12

#define NORTH_TEXT_X_OFFSET +1
#define NORTH_TEXT_Y_OFFSET +4
#define SOUTH_TEXT_X_OFFSET +1
#define SOUTH_TEXT_Y_OFFSET +2

#define EAST_TEXT_X_OFFSET +2
#define EAST_TEXT_Y_OFFSET 0
#define WEST_TEXT_X_OFFSET +4
#define WEST_TEXT_Y_OFFSET 0

#define ICON_WIDTH 8

void ShowPeopleInMotion(uint8_t sX, uint8_t sY) {
  int32_t sExiting = 0;
  int32_t sEntering = 0;
  int16_t sDest = 0;
  int16_t sSource = 0;
  int16_t sOffsetX = 0, sOffsetY = 0;
  int16_t iX = sX, iY = sY;
  int16_t sXPosition = 0, sYPosition = 0;
  int32_t iCounter = 0;
  struct VObject *hIconHandle;
  BOOLEAN fAboutToEnter = FALSE;
  wchar_t sString[32];
  int16_t sTextXOffset = 0;
  int16_t sTextYOffset = 0;
  int16_t usX, usY;
  int32_t iWidth = 0, iHeight = 0;
  int32_t iDeltaXForError = 0, iDeltaYForError = 0;

  if (iCurrentMapSectorZ != 0) {
    return;
  }

  // show the icons for people in motion from this sector to the next guy over
  for (iCounter = 0; iCounter < 4; iCounter++) {
    // find how many people are coming and going in this sector
    sExiting = 0;
    sEntering = 0;
    sSource = GetSectorID16(sX, sY);
    sOffsetX = 0;
    sOffsetY = 0;
    iX = sX;
    iY = sY;

    // reset fact about to enter
    fAboutToEnter = FALSE;

    sDest = sSource;

    if ((iCounter == 0) && sY > 1) {
      sDest += NORTH_MOVE;
    } else if ((iCounter == 1) && (sX < MAP_WORLD_X - 1)) {
      sDest += EAST_MOVE;
    } else if ((iCounter == 2) && (sY < MAP_WORLD_Y - 1)) {
      sDest += SOUTH_MOVE;
    } else if ((iCounter == 3) && (sX > 1)) {
      sDest += WEST_MOVE;
    }

    // if not at edge of map
    if (sDest != sSource) {
      if (PlayersBetweenTheseSectors(
              (int16_t)GetSectorID8(SectorID16_X(sSource), SectorID16_Y(sSource)),
              (int16_t)GetSectorID8(SectorID16_X(sDest), SectorID16_Y(sDest)), &sExiting,
              &sEntering, &fAboutToEnter)) {
        // someone is leaving

        // now find position
        if (!(iCounter % 2)) {
          // guys going north or south
          if (sEntering > 0) {
            // more than one coming in, offset from middle
            sOffsetX = (!iCounter ? (!fZoomFlag ? NORTH_X_MVT_OFFSET : NORTH_X_MVT_OFFSET_ZOOM)
                                  : (!fZoomFlag ? SOUTH_X_MVT_OFFSET : SOUTH_X_MVT_OFFSET_ZOOM));
          } else {
            sOffsetX = (!fZoomFlag ? NORTH_SOUTH_CENTER_OFFSET : NORTH_SOUTH_CENTER_OFFSET_ZOOM);
          }

          if (!iCounter) {
            // going north
            sOffsetY = (!fZoomFlag ? NORTH_Y_MVT_OFFSET : NORTH_Y_MVT_OFFSET_ZOOM);
          } else {
            // going south
            sOffsetY = (!fZoomFlag ? SOUTH_Y_MVT_OFFSET : SOUTH_Y_MVT_OFFSET_ZOOM);
          }
        } else {
          // going east/west

          if (sEntering > 0) {
            // people also entering, offset from middle
            sOffsetY = (iCounter == 1 ? (!fZoomFlag ? EAST_Y_MVT_OFFSET : EAST_Y_MVT_OFFSET_ZOOM)
                                      : (!fZoomFlag ? WEST_Y_MVT_OFFSET : WEST_Y_MVT_OFFSET_ZOOM));
          } else {
            sOffsetY = (!fZoomFlag ? EAST_WEST_CENTER_OFFSET : EAST_WEST_CENTER_OFFSET_ZOOM);
          }

          if (iCounter == 1) {
            // going east
            sOffsetX = (!fZoomFlag ? EAST_X_MVT_OFFSET : EAST_X_MVT_OFFSET_ZOOM);
          } else {
            // going west
            sOffsetX = (!fZoomFlag ? WEST_X_MVT_OFFSET : WEST_X_MVT_OFFSET_ZOOM);
          }
        }

        switch (iCounter) {
          case 0:
            sTextXOffset = NORTH_TEXT_X_OFFSET;
            sTextYOffset = NORTH_TEXT_Y_OFFSET;
            break;
          case 1:
            sTextXOffset = EAST_TEXT_X_OFFSET;
            sTextYOffset = EAST_TEXT_Y_OFFSET;
            break;
          case 2:
            sTextXOffset = SOUTH_TEXT_X_OFFSET;
            sTextYOffset = SOUTH_TEXT_Y_OFFSET;
            break;
          case 3:
            sTextXOffset = WEST_TEXT_X_OFFSET;
            sTextYOffset = WEST_TEXT_Y_OFFSET;
            break;
        }

        // blit the text

        SetFont(MAP_MVT_ICON_FONT);

        if (!fAboutToEnter) {
          SetFontForeground(FONT_WHITE);
        } else {
          SetFontForeground(FONT_BLACK);
        }

        SetFontBackground(FONT_BLACK);

        swprintf(sString, ARR_SIZE(sString), L"%d", sExiting);

        // about to enter
        if (!fAboutToEnter) {
          // draw blue arrows
          GetVideoObject(&hIconHandle, guiCHARBETWEENSECTORICONS);
        } else {
          // draw yellow arrows
          GetVideoObject(&hIconHandle, guiCHARBETWEENSECTORICONSCLOSE);
        }

        // zoomed in or not?
        if (!fZoomFlag) {
          iX = MAP_VIEW_START_X + (iX * MAP_GRID_X) + sOffsetX;
          iY = MAP_Y_ICON_OFFSET + MAP_VIEW_START_Y + (iY * MAP_GRID_Y) + sOffsetY;

          BltVideoObject(guiSAVEBUFFER, hIconHandle, (uint16_t)iCounter, (int16_t)iX, (int16_t)iY,
                         VO_BLT_SRCTRANSPARENCY, NULL);
        } else {
          GetScreenXYFromMapXYStationary(((uint8_t)(iX)), ((uint8_t)(iY)), &sXPosition,
                                         &sYPosition);

          iY = sYPosition - MAP_GRID_Y + sOffsetY;
          iX = sXPosition - MAP_GRID_X + sOffsetX;

          // clip blits to mapscreen region
          ClipBlitsToMapViewRegion();

          BltVideoObject(guiSAVEBUFFER, hIconHandle, (uint16_t)iCounter, iX, iY,
                         VO_BLT_SRCTRANSPARENCY, NULL);

          // restore clip blits
          RestoreClipRegionToFullScreen();
        }

        FindFontCenterCoordinates((int16_t)(iX + sTextXOffset), 0, ICON_WIDTH, 0, sString, MAP_FONT,
                                  &usX, &usY);
        SetFontDestBuffer(guiSAVEBUFFER, 0, 0, 640, 480, FALSE);
        mprintf(usX, iY + sTextYOffset, sString);

        switch (iCounter % 2) {
          case 0:
            // north south
            iWidth = 10;
            iHeight = 12;
            break;
          case 1:
            // east west
            iWidth = 12;
            iHeight = 7;
            break;
        }

        // error correction for scrolling with people on the move
        if (iX < 0) {
          iDeltaXForError = 0 - iX;
          iWidth -= iDeltaXForError;
          iX = 0;
        }

        if (iY < 0) {
          iDeltaYForError = 0 - iY;
          iHeight -= iDeltaYForError;
          iY = 0;
        }

        if ((iWidth > 0) && (iHeight > 0)) {
          RestoreExternBackgroundRect(iX, iY, (int16_t)iWidth, (int16_t)iHeight);
        }
      }
    }
  }

  // restore buffer
  SetFontDestBuffer(FRAME_BUFFER, 0, 0, 640, 480, FALSE);
}

void DisplayDistancesForHelicopter(void) {
  // calculate the distance travelled, the proposed distance, and total distance one can go
  // display these on screen
  int16_t sDistanceToGo = 0;  //, sDistanceSoFar = 0, sTotalCanTravel = 0;
  int16_t sX = 0, sY = 0;
  wchar_t sString[32];
  struct VObject *hHandle;
  int16_t sTotalOfTrip = 0;
  int32_t iTime = 0;
  uint8_t sMapX, sMapY;
  int16_t sYPosition = 0;
  static int16_t sOldYPosition = 0;
  int16_t sNumSafeSectors;
  int16_t sNumUnSafeSectors;
  uint32_t uiTripCost;

  if (GetMouseMapXY(&sMapX, &sMapY) && !fZoomFlag && (sMapY >= 13)) {
    sYPosition = MAP_HELICOPTER_UPPER_ETA_POPUP_Y;
  } else {
    sYPosition = MAP_HELICOPTER_ETA_POPUP_Y;
  }

  if ((sOldYPosition != 0) && (sOldYPosition != sYPosition)) {
    RestoreExternBackgroundRect(MAP_HELICOPTER_ETA_POPUP_X, sOldYPosition,
                                MAP_HELICOPTER_ETA_POPUP_WIDTH + 20,
                                MAP_HELICOPTER_ETA_POPUP_HEIGHT);
  }

  sOldYPosition = sYPosition;

  // blit in background
  GetVideoObject(&hHandle, guiMapBorderHeliSectors);
  BltVideoObject(FRAME_BUFFER, hHandle, 0, MAP_HELICOPTER_ETA_POPUP_X, sYPosition,
                 VO_BLT_SRCTRANSPARENCY, NULL);

  //	sTotalCanTravel = ( int16_t )GetTotalDistanceHelicopterCanTravel( );
  sDistanceToGo = (int16_t)DistanceOfIntendedHelicopterPath();
  sTotalOfTrip = sDistanceToGo;  // + ( int16_t ) ( DistanceToNearestRefuelPoint( ( int16_t )(
                                 // LastSectorInHelicoptersPath() % MAP_WORLD_X ), ( int16_t ) (
                                 // LastSectorInHelicoptersPath() / MAP_WORLD_X ) ) );

  sNumSafeSectors = GetNumSafeSectorsInPath();
  sNumUnSafeSectors = GetNumUnSafeSectorsInPath();

  //	sDistanceSoFar = ( int16_t )HowFarHelicopterhasTravelledSinceRefueling( );
  //	 sTotalDistanceOfTrip = ( int16_t )DistanceToNearestRefuelPoint( )

  if (sDistanceToGo == 9999) {
    sDistanceToGo = 0;
  }

  // set the font stuff
  SetFont(MAP_FONT);
  SetFontForeground(FONT_LTGREEN);
  SetFontBackground(FONT_BLACK);

  swprintf(sString, ARR_SIZE(sString), L"%s", pHelicopterEtaStrings[0]);
  mprintf(MAP_HELICOPTER_ETA_POPUP_X + 5, sYPosition + 5, sString);

  /*
    if ( IsSectorOutOfTheWay( sMapX, sMapY ) )
    {
                  SetFontForeground( FONT_RED );
          }
          else
  */
  { SetFontForeground(FONT_LTGREEN); }

  swprintf(sString, ARR_SIZE(sString), L"%d", sTotalOfTrip);
  FindFontRightCoordinates(MAP_HELICOPTER_ETA_POPUP_X + 5, MAP_HELICOPTER_ETA_POPUP_Y + 5,
                           MAP_HELICOPTER_ETA_POPUP_WIDTH, 0, sString, MAP_FONT, &sX, &sY);
  mprintf(sX, sYPosition + 5, sString);

  SetFontForeground(FONT_LTGREEN);

  swprintf(sString, ARR_SIZE(sString), L"%s", pHelicopterEtaStrings[1]);
  mprintf(MAP_HELICOPTER_ETA_POPUP_X + 5, sYPosition + 5 + GetFontHeight(MAP_FONT), sString);

  swprintf(sString, ARR_SIZE(sString), L"%d", sNumSafeSectors);
  FindFontRightCoordinates(MAP_HELICOPTER_ETA_POPUP_X + 5,
                           (int16_t)(MAP_HELICOPTER_ETA_POPUP_Y + 5 + 2 * GetFontHeight(MAP_FONT)),
                           MAP_HELICOPTER_ETA_POPUP_WIDTH, 0, sString, MAP_FONT, &sX, &sY);
  mprintf(sX, (int16_t)(sYPosition + 5 + GetFontHeight(MAP_FONT)), sString);

  swprintf(sString, ARR_SIZE(sString), L"%s", pHelicopterEtaStrings[2]);
  mprintf(MAP_HELICOPTER_ETA_POPUP_X + 5, sYPosition + 5 + 2 * GetFontHeight(MAP_FONT), sString);

  swprintf(sString, ARR_SIZE(sString), L"%d", sNumUnSafeSectors);
  FindFontRightCoordinates(MAP_HELICOPTER_ETA_POPUP_X + 5,
                           (int16_t)(MAP_HELICOPTER_ETA_POPUP_Y + 5 + 2 * GetFontHeight(MAP_FONT)),
                           MAP_HELICOPTER_ETA_POPUP_WIDTH, 0, sString, MAP_FONT, &sX, &sY);
  mprintf(sX, (int16_t)(sYPosition + 5 + 2 * GetFontHeight(MAP_FONT)), sString);

  swprintf(sString, ARR_SIZE(sString), L"%s", pHelicopterEtaStrings[3]);
  mprintf(MAP_HELICOPTER_ETA_POPUP_X + 5, sYPosition + 5 + 3 * GetFontHeight(MAP_FONT), sString);

  // calculate the cost of the trip based on the number of safe and unsafe sectors it will pass
  // through
  uiTripCost = (sNumSafeSectors * COST_AIRSPACE_SAFE) + (sNumUnSafeSectors * COST_AIRSPACE_UNSAFE);

  swprintf(sString, ARR_SIZE(sString), L"%d", uiTripCost);
  InsertCommasForDollarFigure(sString);
  InsertDollarSignInToString(sString);
  FindFontRightCoordinates(MAP_HELICOPTER_ETA_POPUP_X + 5,
                           (int16_t)(MAP_HELICOPTER_ETA_POPUP_Y + 5 + 3 * GetFontHeight(MAP_FONT)),
                           MAP_HELICOPTER_ETA_POPUP_WIDTH, 0, sString, MAP_FONT, &sX, &sY);
  mprintf(sX, (int16_t)(sYPosition + 5 + 3 * GetFontHeight(MAP_FONT)), sString);

  swprintf(sString, ARR_SIZE(sString), L"%s", pHelicopterEtaStrings[4]);
  mprintf(MAP_HELICOPTER_ETA_POPUP_X + 5, sYPosition + 5 + 4 * GetFontHeight(MAP_FONT), sString);

  // get travel time for the last path segment
  iTime = GetPathTravelTimeDuringPlotting(pTempHelicopterPath);

  // add travel time for any prior path segments (stored in the helicopter's mercpath, but
  // waypoints aren't built)
  iTime += GetPathTravelTimeDuringPlotting(pVehicleList[iHelicopterVehicleId].pMercPath);

  swprintf(sString, ARR_SIZE(sString), L"%d%s %d%s", iTime / 60, gsTimeStrings[0], iTime % 60,
           gsTimeStrings[1]);
  FindFontRightCoordinates(MAP_HELICOPTER_ETA_POPUP_X + 5,
                           (int16_t)(sYPosition + 5 + 4 * GetFontHeight(MAP_FONT)),
                           MAP_HELICOPTER_ETA_POPUP_WIDTH, 0, sString, MAP_FONT, &sX, &sY);
  mprintf(sX, (int16_t)(sYPosition + 5 + 4 * GetFontHeight(MAP_FONT)), sString);

  // show # of passengers aboard the chopper
  mprintf(MAP_HELICOPTER_ETA_POPUP_X + 5, sYPosition + 5 + 5 * GetFontHeight(MAP_FONT),
          pHelicopterEtaStrings[6]);
  swprintf(sString, ARR_SIZE(sString), L"%d", GetNumberOfPassengersInHelicopter());
  FindFontRightCoordinates(MAP_HELICOPTER_ETA_POPUP_X + 5,
                           (int16_t)(MAP_HELICOPTER_ETA_POPUP_Y + 5 + 5 * GetFontHeight(MAP_FONT)),
                           MAP_HELICOPTER_ETA_POPUP_WIDTH, 0, sString, MAP_FONT, &sX, &sY);
  mprintf(sX, (int16_t)(sYPosition + 5 + 5 * GetFontHeight(MAP_FONT)), sString);

  InvalidateRegion(MAP_HELICOPTER_ETA_POPUP_X, sOldYPosition,
                   MAP_HELICOPTER_ETA_POPUP_X + MAP_HELICOPTER_ETA_POPUP_WIDTH + 20,
                   sOldYPosition + MAP_HELICOPTER_ETA_POPUP_HEIGHT);
  return;
}

// grab position of helicopter and blt to screen
void DisplayPositionOfHelicopter(void) {
  static int16_t sOldMapX = 0, sOldMapY = 0;
  //	int16_t sX =0, sY = 0;
  float flRatio = 0.0;
  uint32_t x, y;
  uint16_t minX, minY, maxX, maxY;
  struct GROUP *pGroup;
  struct VObject *hHandle;
  int32_t iNumberOfPeopleInHelicopter = 0;
  wchar_t sString[4];

  AssertMsg((sOldMapX >= 0) && (sOldMapX < 640),
            String("DisplayPositionOfHelicopter: Invalid sOldMapX = %d", sOldMapX));
  AssertMsg((sOldMapY >= 0) && (sOldMapY < 480),
            String("DisplayPositionOfHelicopter: Invalid sOldMapY = %d", sOldMapY));

  // restore background on map where it is
  if (sOldMapX != 0) {
    RestoreExternBackgroundRect(sOldMapX, sOldMapY, HELI_ICON_WIDTH, HELI_ICON_HEIGHT);
    sOldMapX = 0;
  }

  if (iHelicopterVehicleId != -1) {
    // draw the destination icon first, so when they overlap, the real one is on top!
    DisplayDestinationOfHelicopter();

    // check if mvt group
    if (pVehicleList[iHelicopterVehicleId].ubMovementGroup != 0) {
      pGroup = GetGroup(pVehicleList[iHelicopterVehicleId].ubMovementGroup);

      // this came up in one bug report!
      Assert(pGroup->uiTraverseTime != -1);

      if ((pGroup->uiTraverseTime > 0) && (pGroup->uiTraverseTime != 0xffffffff)) {
        flRatio = ((pGroup->uiTraverseTime + GetWorldTotalMin()) - pGroup->uiArrivalTime) /
                  (float)pGroup->uiTraverseTime;
      }

      /*
                              AssertMsg( ( flRatio >= 0 ) && ( flRatio <= 100 ), String(
         "DisplayPositionOfHelicopter: Invalid flRatio = %6.2f, trav %d, arr %d, time %d",
         flRatio, pGroup->uiTraverseTime, pGroup->uiArrivalTime, GetWorldTotalMin() ) );
      */

      if (flRatio < 0) {
        flRatio = 0;
      } else if (flRatio > 100) {
        flRatio = 100;
      }

      //			if( !fZoomFlag )
      {
        // grab min and max locations to interpolate sub sector position
        minX = MAP_VIEW_START_X + MAP_GRID_X * (pGroup->ubSectorX);
        maxX = MAP_VIEW_START_X + MAP_GRID_X * (pGroup->ubNextX);
        minY = MAP_VIEW_START_Y + MAP_GRID_Y * (pGroup->ubSectorY);
        maxY = MAP_VIEW_START_Y + MAP_GRID_Y * (pGroup->ubNextY);
      }
      /*
                              else
                              {

                                      // grab coords for nextx,y and current x,y

                                      // zoomed in, takes a little more work
                                      GetScreenXYFromMapXYStationary(
         ((uint16_t)(pGroup->ubSectorX)),((uint16_t)(pGroup->ubSectorY)) , &sX, &sY );
         sY=sY-MAP_GRID_Y; sX=sX-MAP_GRID_X;

                                      minX = ( sX );
                                      minY = ( sY );

                                      GetScreenXYFromMapXYStationary(
         ((uint16_t)(pGroup->ubNextX)),((uint16_t)(pGroup->ubNextY)) , &sX, &sY );
         sY=sY-MAP_GRID_Y; sX=sX-MAP_GRID_X;

                                      maxX = ( sX );
                                      maxY = ( sY );
                              }
      */

      AssertMsg((minX >= 0) && (minX < 640),
                String("DisplayPositionOfHelicopter: Invalid minX = %d", minX));
      AssertMsg((maxX >= 0) && (maxX < 640),
                String("DisplayPositionOfHelicopter: Invalid maxX = %d", maxX));
      AssertMsg((minY >= 0) && (minY < 640),
                String("DisplayPositionOfHelicopter: Invalid minY = %d", minY));
      AssertMsg((maxY >= 0) && (maxY < 640),
                String("DisplayPositionOfHelicopter: Invalid maxY = %d", maxY));

      // IMPORTANT: Since min can easily be larger than max, we gotta cast to as signed value
      x = (uint32_t)(minX + flRatio * ((int16_t)maxX - (int16_t)minX));
      y = (uint32_t)(minY + flRatio * ((int16_t)maxY - (int16_t)minY));

      /*
                              if( fZoomFlag )
                              {
                                      x += 13;
                                      y += 8;
                              }
                              else
      */
      {
        x += 1;
        y += 3;
      }

      AssertMsg((x >= 0) && (x < 640), String("DisplayPositionOfHelicopter: Invalid x = %d.  At "
                                              "%d,%d.  Next %d,%d.  Min/Max X = %d/%d",
                                              x, pGroup->ubSectorX, pGroup->ubSectorY,
                                              pGroup->ubNextX, pGroup->ubNextY, minX, maxX));

      AssertMsg((y >= 0) && (y < 480), String("DisplayPositionOfHelicopter: Invalid y = %d.  At "
                                              "%d,%d.  Next %d,%d.  Min/Max Y = %d/%d",
                                              y, pGroup->ubSectorX, pGroup->ubSectorY,
                                              pGroup->ubNextX, pGroup->ubNextY, minY, maxY));

      // clip blits to mapscreen region
      ClipBlitsToMapViewRegion();

      GetVideoObject(&hHandle, guiHelicopterIcon);
      BltVideoObject(FRAME_BUFFER, hHandle, HELI_ICON, x, y, VO_BLT_SRCTRANSPARENCY, NULL);

      // now get number of people and blit that too
      iNumberOfPeopleInHelicopter = GetNumberOfPassengersInHelicopter();
      swprintf(sString, ARR_SIZE(sString), L"%d", iNumberOfPeopleInHelicopter);

      SetFont(MAP_MVT_ICON_FONT);
      SetFontForeground(FONT_WHITE);
      SetFontBackground(FONT_BLACK);

      mprintf(x + 5, y + 1, sString);

      InvalidateRegion(x, y, x + HELI_ICON_WIDTH, y + HELI_ICON_HEIGHT);

      RestoreClipRegionToFullScreen();

      // now store the old stuff
      sOldMapX = (int16_t)x;
      sOldMapY = (int16_t)y;
    }
  }

  return;
}

void DisplayDestinationOfHelicopter(void) {
  static int16_t sOldMapX = 0, sOldMapY = 0;
  int16_t sSector;
  uint8_t sMapX, sMapY;
  uint32_t x, y;
  struct VObject *hHandle;

  AssertMsg((sOldMapX >= 0) && (sOldMapX < 640),
            String("DisplayDestinationOfHelicopter: Invalid sOldMapX = %d", sOldMapX));
  AssertMsg((sOldMapY >= 0) && (sOldMapY < 480),
            String("DisplayDestinationOfHelicopter: Invalid sOldMapY = %d", sOldMapY));

  // restore background on map where it is
  if (sOldMapX != 0) {
    RestoreExternBackgroundRect(sOldMapX, sOldMapY, HELI_SHADOW_ICON_WIDTH,
                                HELI_SHADOW_ICON_HEIGHT);
    sOldMapX = 0;
  }

  // if helicopter is going somewhere
  if (GetLengthOfPath(pVehicleList[iHelicopterVehicleId].pMercPath) > 1) {
    // get destination
    sSector = GetLastSectorIdInVehiclePath(iHelicopterVehicleId);
    sMapX = SectorID16_X(sSector);
    sMapY = SectorID16_Y(sSector);

    x = MAP_VIEW_START_X + (MAP_GRID_X * sMapX) + 1;
    y = MAP_VIEW_START_Y + (MAP_GRID_Y * sMapY) + 3;

    AssertMsg(
        (x >= 0) && (x < 640),
        String("DisplayDestinationOfHelicopter: Invalid x = %d.  Dest %d,%d", x, sMapX, sMapY));
    AssertMsg(
        (y >= 0) && (y < 480),
        String("DisplayDestinationOfHelicopter: Invalid y = %d.  Dest %d,%d", y, sMapX, sMapY));

    // clip blits to mapscreen region
    ClipBlitsToMapViewRegion();

    GetVideoObject(&hHandle, guiHelicopterIcon);
    BltVideoObject(FRAME_BUFFER, hHandle, HELI_SHADOW_ICON, x, y, VO_BLT_SRCTRANSPARENCY, NULL);
    InvalidateRegion(x, y, x + HELI_SHADOW_ICON_WIDTH, y + HELI_SHADOW_ICON_HEIGHT);

    RestoreClipRegionToFullScreen();

    // now store the old stuff
    sOldMapX = (int16_t)x;
    sOldMapY = (int16_t)y;
  }
}

BOOLEAN CheckForClickOverHelicopterIcon(uint8_t sClickedSectorX, uint8_t sClickedSectorY) {
  struct GROUP *pGroup = NULL;
  BOOLEAN fHelicopterOverNextSector = FALSE;
  float flRatio = 0.0;
  int16_t sSectorX;
  int16_t sSectorY;

  giClickHeliIconBaseTime = GetJA2Clock();

  if (!fHelicopterAvailable || !fShowAircraftFlag) {
    return (FALSE);
  }

  if (iHelicopterVehicleId == -1) {
    return (FALSE);
  }

  // figure out over which sector the helicopter APPEARS to be to the player (because we slide it
  // smoothly across the map, unlike groups travelling on the ground, it can appear over its next
  // sector while it's not there yet.

  pGroup = GetGroup(pVehicleList[iHelicopterVehicleId].ubMovementGroup);
  Assert(pGroup);

  if (pGroup->fBetweenSectors) {
    // this came up in one bug report!
    Assert(pGroup->uiTraverseTime != -1);

    if ((pGroup->uiTraverseTime > 0) && (pGroup->uiTraverseTime != 0xffffffff)) {
      flRatio = (pGroup->uiTraverseTime - pGroup->uiArrivalTime + GetWorldTotalMin()) /
                (float)pGroup->uiTraverseTime;
    }

    // if more than halfway there, the chopper appears more over the next sector, not over its
    // current one(!)
    if (flRatio > 0.5) {
      fHelicopterOverNextSector = TRUE;
    }
  }

  if (fHelicopterOverNextSector) {
    // use the next sector's coordinates
    sSectorX = pGroup->ubNextX;
    sSectorY = pGroup->ubNextY;
  } else {
    // use current sector's coordinates
    sSectorX = pVehicleList[iHelicopterVehicleId].sSectorX;
    sSectorY = pVehicleList[iHelicopterVehicleId].sSectorY;
  }

  // check if helicopter appears where he clicked
  if ((sSectorX != sClickedSectorX) || (sSectorY != sClickedSectorY)) {
    return (FALSE);
  }

  return (TRUE);
}

void BlitMineIcon(uint8_t sMapX, uint8_t sMapY) {
  struct VObject *hHandle;
  uint32_t uiDestPitchBYTES;
  int16_t sScreenX, sScreenY;

  GetVideoObject(&hHandle, guiMINEICON);

  LockVideoSurface(guiSAVEBUFFER, &uiDestPitchBYTES);
  SetClippingRegionAndImageWidth(uiDestPitchBYTES, MAP_VIEW_START_X + MAP_GRID_X - 1,
                                 MAP_VIEW_START_Y + MAP_GRID_Y - 1, MAP_VIEW_WIDTH + 1,
                                 MAP_VIEW_HEIGHT - 9);
  UnLockVideoSurface(guiSAVEBUFFER);

  if (fZoomFlag) {
    GetScreenXYFromMapXYStationary((int16_t)(sMapX), (int16_t)(sMapY), &sScreenX, &sScreenY);
    // when zoomed, the x,y returned is the CENTER of the map square in question
    BltVideoObject(guiSAVEBUFFER, hHandle, 0, sScreenX - MAP_GRID_ZOOM_X / 4,
                   sScreenY - MAP_GRID_ZOOM_Y / 4, VO_BLT_SRCTRANSPARENCY, NULL);
  } else {
    GetScreenXYFromMapXY((int16_t)(sMapX), (int16_t)(sMapY), &sScreenX, &sScreenY);
    // when not zoomed, the x,y returned is the top left CORNER of the map square in question
    BltVideoObject(guiSAVEBUFFER, hHandle, 1, sScreenX + MAP_GRID_X / 4, sScreenY + MAP_GRID_Y / 4,
                   VO_BLT_SRCTRANSPARENCY, NULL);
  }
}

void BlitMineText(uint8_t sMapX, uint8_t sMapY) {
  int16_t sScreenX, sScreenY;
  wchar_t wString[32], wSubString[32];
  uint8_t ubMineIndex;
  uint8_t ubLineCnt = 0;

  if (fZoomFlag) {
    GetScreenXYFromMapXYStationary((int16_t)(sMapX), (int16_t)(sMapY), &sScreenX, &sScreenY);

    // set coordinates for start of mine text
    sScreenY += MAP_GRID_ZOOM_Y / 2 + 1;  // slightly below
  } else {
    GetScreenXYFromMapXY((int16_t)(sMapX), (int16_t)(sMapY), &sScreenX, &sScreenY);

    // set coordinates for start of mine text
    sScreenX += MAP_GRID_X / 2;  // centered around middle of mine square
    sScreenY += MAP_GRID_Y + 1;  // slightly below
  }

  // show detailed mine info (name, production rate, daily production)

  SetFontDestBuffer(guiSAVEBUFFER, MAP_VIEW_START_X, MAP_VIEW_START_Y,
                    MAP_VIEW_START_X + MAP_VIEW_WIDTH + MAP_GRID_X,
                    MAP_VIEW_START_Y + MAP_VIEW_HEIGHT + 7, FALSE);

  SetFont(MAP_FONT);
  SetFontForeground(FONT_LTGREEN);
  SetFontBackground(FONT_BLACK);

  ubMineIndex = GetMineIndexForSector(sMapX, sMapY);

  // display associated town name, followed by "mine"
  swprintf(wString, ARR_SIZE(wString), L"%s %s",
           pTownNames[GetTownAssociatedWithMine(GetMineIndexForSector(sMapX, sMapY))],
           pwMineStrings[0]);
  AdjustXForLeftMapEdge(wString, &sScreenX);
  mprintf((sScreenX - StringPixLength(wString, MAP_FONT) / 2),
          sScreenY + ubLineCnt * GetFontHeight(MAP_FONT), wString);
  ubLineCnt++;

  // check if mine is empty (abandoned) or running out
  if (gMineStatus[ubMineIndex].fEmpty) {
    swprintf(wString, ARR_SIZE(wString), L"%s", pwMineStrings[5]);
    AdjustXForLeftMapEdge(wString, &sScreenX);
    mprintf((sScreenX - StringPixLength(wString, MAP_FONT) / 2),
            sScreenY + ubLineCnt * GetFontHeight(MAP_FONT), wString);
    ubLineCnt++;
  } else if (gMineStatus[ubMineIndex].fShutDown) {
    swprintf(wString, ARR_SIZE(wString), L"%s", pwMineStrings[6]);
    AdjustXForLeftMapEdge(wString, &sScreenX);
    mprintf((sScreenX - StringPixLength(wString, MAP_FONT) / 2),
            sScreenY + ubLineCnt * GetFontHeight(MAP_FONT), wString);
    ubLineCnt++;
  } else if (gMineStatus[ubMineIndex].fRunningOut) {
    swprintf(wString, ARR_SIZE(wString), L"%s", pwMineStrings[7]);
    AdjustXForLeftMapEdge(wString, &sScreenX);
    mprintf((sScreenX - StringPixLength(wString, MAP_FONT) / 2),
            sScreenY + ubLineCnt * GetFontHeight(MAP_FONT), wString);
    ubLineCnt++;
  }

  // only show production if player controls it and it's actually producing
  if (PlayerControlsMine(ubMineIndex) && !gMineStatus[ubMineIndex].fEmpty) {
    // show current production
    swprintf(wSubString, ARR_SIZE(wSubString), L"%d", PredictDailyIncomeFromAMine(ubMineIndex));
    InsertCommasForDollarFigure(wSubString);
    InsertDollarSignInToString(wSubString);
    wcscpy(wString, wSubString);

    /*
                    // show maximum potential production
                    swprintf( wSubString, L"%d", GetMaxDailyRemovalFromMine(ubMineIndex) );
                    InsertCommasForDollarFigure( wSubString );
                    InsertDollarSignInToString( wSubString );
                    wcscat( wString, L" / ");
                    wcscat( wString, wSubString );
    */

    // if potential is not nil, show percentage of the two
    if (GetMaxPeriodicRemovalFromMine(ubMineIndex) > 0) {
      swprintf(wSubString, ARR_SIZE(wSubString), L" (%d%%%%)",
               (PredictDailyIncomeFromAMine(ubMineIndex) * 100) /
                   GetMaxDailyRemovalFromMine(ubMineIndex));
      wcscat(wString, wSubString);
    }

    AdjustXForLeftMapEdge(wString, &sScreenX);
    mprintf((sScreenX - StringPixLengthArg(MAP_FONT, wcslen(wString), wString) / 2),
            sScreenY + ubLineCnt * GetFontHeight(MAP_FONT), wString);
    ubLineCnt++;
  }

  SetFontDestBuffer(FRAME_BUFFER, MAP_VIEW_START_X, MAP_VIEW_START_Y,
                    MAP_VIEW_START_X + MAP_VIEW_WIDTH + MAP_GRID_X,
                    MAP_VIEW_START_Y + MAP_VIEW_HEIGHT + 7, FALSE);
}

void AdjustXForLeftMapEdge(wchar_t *wString, int16_t *psX) {
  int16_t sStartingX, sPastEdge;

  if (fZoomFlag)
    // it's ok to cut strings off in zoomed mode
    return;

  sStartingX = *psX - (StringPixLengthArg(MAP_FONT, wcslen(wString), wString) / 2);
  sPastEdge = (MAP_VIEW_START_X + 23) - sStartingX;

  if (sPastEdge > 0) *psX += sPastEdge;
}

void BlitTownGridMarkers(void) {
  int16_t sScreenX = 0, sScreenY = 0;
  uint32_t uiDestPitchBYTES;
  uint8_t *pDestBuf;
  uint16_t usColor = 0;
  int32_t iCounter = 0;
  int16_t sWidth = 0, sHeight = 0;

  // get 16 bpp color
  usColor = Get16BPPColor(FROMRGB(100, 100, 100));

  // blit in the highlighted sector
  pDestBuf = LockVideoSurface(guiSAVEBUFFER, &uiDestPitchBYTES);

  // clip to view region
  ClipBlitsToMapViewRegionForRectangleAndABit(uiDestPitchBYTES);

  const TownSectors *townSectors = GetAllTownSectors();

  // go through list of towns and place on screen
  while ((*townSectors)[iCounter].townID != 0) {
    TownID townID = (*townSectors)[iCounter].townID;
    SectorID16 sectorID = (*townSectors)[iCounter].sectorID;
    // skip Orta/Tixa until found
    if (((fFoundOrta != FALSE) || (townID != ORTA)) &&
        ((townID != TIXA) || (fFoundTixa != FALSE))) {
      if (fZoomFlag) {
        GetScreenXYFromMapXYStationary(SectorID16_X(sectorID), SectorID16_Y(sectorID), &sScreenX,
                                       &sScreenY);
        sScreenX -= MAP_GRID_X - 1;
        sScreenY -= MAP_GRID_Y;

        sWidth = 2 * MAP_GRID_X;
        sHeight = 2 * MAP_GRID_Y;
      } else {
        // get location on screen
        GetScreenXYFromMapXY(SectorID16_X(sectorID), SectorID16_Y(sectorID), &sScreenX, &sScreenY);
        sWidth = MAP_GRID_X - 1;
        sHeight = MAP_GRID_Y;

        sScreenX += 2;
      }

      if (StrategicMap[sectorID - MAP_WORLD_X].townID == BLANK_SECTOR) {
        LineDraw(TRUE, sScreenX - 1, sScreenY - 1, sScreenX + sWidth - 1, sScreenY - 1, usColor,
                 pDestBuf);
      }

      if ((StrategicMap[sectorID + MAP_WORLD_X].townID == BLANK_SECTOR)) {
        LineDraw(TRUE, sScreenX - 1, sScreenY + sHeight - 1, sScreenX + sWidth - 1,
                 sScreenY + sHeight - 1, usColor, pDestBuf);
      }

      if (StrategicMap[sectorID - 1].townID == BLANK_SECTOR) {
        LineDraw(TRUE, sScreenX - 2, sScreenY - 1, sScreenX - 2, sScreenY + sHeight - 1, usColor,
                 pDestBuf);
      }

      if (StrategicMap[sectorID + 1].townID == BLANK_SECTOR) {
        LineDraw(TRUE, sScreenX + sWidth - 1, sScreenY - 1, sScreenX + sWidth - 1,
                 sScreenY + sHeight - 1, usColor, pDestBuf);
      }
    }

    iCounter++;
  }

  // restore clips
  RestoreClipRegionToFullScreenForRectangle(uiDestPitchBYTES);

  // unlock surface
  UnLockVideoSurface(guiSAVEBUFFER);

  return;
}

void BlitMineGridMarkers(void) {
  int16_t sScreenX = 0, sScreenY = 0;
  uint32_t uiDestPitchBYTES;
  uint8_t *pDestBuf;
  uint16_t usColor = 0;
  int32_t iCounter = 0;
  int16_t sWidth = 0, sHeight = 0;

  // get 16 bpp color
  usColor = Get16BPPColor(FROMRGB(100, 100, 100));

  // blit in the highlighted sector
  pDestBuf = LockVideoSurface(guiSAVEBUFFER, &uiDestPitchBYTES);

  // clip to view region
  ClipBlitsToMapViewRegionForRectangleAndABit(uiDestPitchBYTES);

  for (iCounter = 0; iCounter < MAX_NUMBER_OF_MINES; iCounter++) {
    if (fZoomFlag) {
      GetScreenXYFromMapXYStationary((int16_t)(gMineLocation[iCounter].sSectorX),
                                     (int16_t)(gMineLocation[iCounter].sSectorY), &sScreenX,
                                     &sScreenY);
      sScreenX -= MAP_GRID_X;
      sScreenY -= MAP_GRID_Y;

      sWidth = 2 * MAP_GRID_X;
      sHeight = 2 * MAP_GRID_Y;
    } else {
      // get location on screen
      GetScreenXYFromMapXY((int16_t)(gMineLocation[iCounter].sSectorX),
                           (int16_t)(gMineLocation[iCounter].sSectorY), &sScreenX, &sScreenY);
      sWidth = MAP_GRID_X;
      sHeight = MAP_GRID_Y;
    }

    // draw rectangle
    RectangleDraw(TRUE, sScreenX, sScreenY - 1, sScreenX + sWidth, sScreenY + sHeight - 1, usColor,
                  pDestBuf);
  }

  // restore clips
  RestoreClipRegionToFullScreenForRectangle(uiDestPitchBYTES);

  // unlock surface
  UnLockVideoSurface(guiSAVEBUFFER);

  return;
}

void DisplayLevelString(void) {
  wchar_t sString[32];

  // given the current level being displayed on the map, show a sub level message

  // at the surface
  if (!iCurrentMapSectorZ) {
    return;
  }

  // otherwise we will have to display the string with the level number

  SetFontDestBuffer(guiSAVEBUFFER, MAP_VIEW_START_X, MAP_VIEW_START_Y,
                    MAP_VIEW_START_X + MAP_VIEW_WIDTH + MAP_GRID_X,
                    MAP_VIEW_START_Y + MAP_VIEW_HEIGHT + 7, FALSE);

  SetFont(MAP_FONT);
  SetFontForeground(MAP_INDEX_COLOR);
  SetFontBackground(FONT_BLACK);
  swprintf(sString, ARR_SIZE(sString), L"%s %d", sMapLevelString[0], iCurrentMapSectorZ);

  mprintf(MAP_LEVEL_STRING_X, MAP_LEVEL_STRING_Y, sString);

  SetFontDestBuffer(FRAME_BUFFER, 0, 0, 640, 480, FALSE);

  return;
}

// function to manipulate the number of towns people on the cursor
static BOOLEAN PickUpATownPersonFromSector(uint8_t ubType, uint8_t sX, uint8_t sY) {
  // see if there are any militia of this type in this sector
  if (GetMilitiaOfRankInSector(sX, sY, ubType) == 0) {
    // failed, no one here
    return (FALSE);
  }

  // are they in the same town as they were pickedup from
  if (GetTownIdForSector(sX, sY) != sSelectedMilitiaTown) {
    return (FALSE);
  }

  if (!SectorOursAndPeaceful(sX, sY, 0)) {
    return (FALSE);
  }

  if (GetSectorID8(sX, sY) == GetSectorID8((uint8_t)gWorldSectorX, (uint8_t)gWorldSectorY)) {
    TacticalMilitiaRefreshRequired();
  }

  // otherwise pick this guy up
  switch (ubType) {
    case (GREEN_MILITIA):
      sGreensOnCursor++;
      break;
    case (REGULAR_MILITIA):
      sRegularsOnCursor++;
      break;
    case (ELITE_MILITIA):
      sElitesOnCursor++;
      break;
  }

  // reduce number in this sector
  SetMilitiaOfRankInSector(sX, sY, ubType, GetMilitiaOfRankInSector(sX, sY, ubType) - 1);

  MarkForRedrawalStrategicMap();

  return (TRUE);
}

BOOLEAN DropAPersonInASector(uint8_t ubType, uint8_t sX, uint8_t sY) {
  // are they in the same town as they were pickedup from
  if (GetTownIdForSector(sX, sY) != sSelectedMilitiaTown) {
    return (FALSE);
  }

  if (CountAllMilitiaInSector(sX, sY) >= MAX_ALLOWABLE_MILITIA_PER_SECTOR) {
    return (FALSE);
  }

  if (!SectorOursAndPeaceful(sX, sY, 0)) {
    return (FALSE);
  }

  if (GetSectorID8(sX, sY) == GetSectorID8((uint8_t)gWorldSectorX, (uint8_t)gWorldSectorY)) {
    TacticalMilitiaRefreshRequired();
  }

  // drop the guy into this sector
  switch (ubType) {
    case (GREEN_MILITIA):
      if (!sGreensOnCursor) {
        return (FALSE);
      }
      sGreensOnCursor--;
      break;
    case (REGULAR_MILITIA):
      if (!sRegularsOnCursor) {
        return (FALSE);
      }
      sRegularsOnCursor--;
      break;
    case (ELITE_MILITIA):
      if (!sElitesOnCursor) {
        return (FALSE);
      }
      sElitesOnCursor--;
      break;
  }

  // up the number in this sector of this type of militia
  IncMilitiaOfRankInSector(sX, sY, ubType, 1);

  MarkForRedrawalStrategicMap();

  return (TRUE);
}

BOOLEAN LoadMilitiaPopUpBox(void) {
  VOBJECT_DESC VObjectDesc;

  // load the militia pop up box
  VObjectDesc.fCreateFlags = VOBJECT_CREATE_FROMFILE;
  FilenameForBPP("INTERFACE\\Militia.sti", VObjectDesc.ImageFile);
  CHECKF(AddVideoObject(&VObjectDesc, &guiMilitia));

  VObjectDesc.fCreateFlags = VOBJECT_CREATE_FROMFILE;
  FilenameForBPP("INTERFACE\\Militiamaps.sti", VObjectDesc.ImageFile);
  CHECKF(AddVideoObject(&VObjectDesc, &guiMilitiaMaps));

  VObjectDesc.fCreateFlags = VOBJECT_CREATE_FROMFILE;
  FilenameForBPP("INTERFACE\\MilitiamapsectorOutline2.sti", VObjectDesc.ImageFile);
  CHECKF(AddVideoObject(&VObjectDesc, &guiMilitiaSectorHighLight));

  VObjectDesc.fCreateFlags = VOBJECT_CREATE_FROMFILE;
  FilenameForBPP("INTERFACE\\MilitiamapsectorOutline.sti", VObjectDesc.ImageFile);
  CHECKF(AddVideoObject(&VObjectDesc, &guiMilitiaSectorOutline));

  return (TRUE);
}

void RemoveMilitiaPopUpBox(void) {
  // delete the militia pop up box graphic
  DeleteVideoObjectFromIndex(guiMilitia);
  DeleteVideoObjectFromIndex(guiMilitiaMaps);
  DeleteVideoObjectFromIndex(guiMilitiaSectorHighLight);
  DeleteVideoObjectFromIndex(guiMilitiaSectorOutline);

  return;
}

BOOLEAN DrawMilitiaPopUpBox(void) {
  struct VObject *hVObject;
  ETRLEObject *pTrav;

  if (!fShowMilitia) {
    sSelectedMilitiaTown = 0;
  }

  // create buttons
  CreateDestroyMilitiaSectorButtons();

  // create mouse regions if we need to
  CreateDestroyMilitiaPopUPRegions();

  if (!sSelectedMilitiaTown) {
    return (FALSE);
  }

  // update states of militia selected sector buttons
  CheckAndUpdateStatesOfSelectedMilitiaSectorButtons();

  // get the properties of the militia object
  GetVideoObject(&hVObject, guiMilitia);

  BltVideoObject(FRAME_BUFFER, hVObject, 0, MAP_MILITIA_BOX_POS_X, MAP_MILITIA_BOX_POS_Y,
                 VO_BLT_SRCTRANSPARENCY, NULL);

  GetVideoObject(&hVObject, guiMilitiaMaps);
  BltVideoObject(FRAME_BUFFER, hVObject, (uint16_t)(sSelectedMilitiaTown - 1),
                 MAP_MILITIA_BOX_POS_X + MAP_MILITIA_MAP_X,
                 MAP_MILITIA_BOX_POS_Y + MAP_MILITIA_MAP_Y, VO_BLT_SRCTRANSPARENCY, NULL);

  // set font color for labels and "total militia" counts
  SetFontForeground(FONT_WHITE);

  // draw name of town, and the "unassigned" label
  DrawTownMilitiaName();

  // render the icons for each sector in the town
  RenderIconsPerSectorForSelectedTown();

  // shade any sectors not under our control
  RenderShadingForUnControlledSectors();

  // display anyone picked up
  DisplayUnallocatedMilitia();

  // draw the highlight last
  ShowHighLightedSectorOnMilitiaMap();

  GetVideoObject(&hVObject, guiMilitia);
  pTrav = &(hVObject->pETRLEObject[0]);

  InvalidateRegion(MAP_MILITIA_BOX_POS_X, MAP_MILITIA_BOX_POS_Y,
                   MAP_MILITIA_BOX_POS_X + pTrav->usWidth, MAP_MILITIA_BOX_POS_Y + pTrav->usHeight);

  // set the text for the militia map sector info buttons
  SetMilitiaMapButtonsText();

  // render buttons
  MarkButtonsDirty();

  return (TRUE);
}

void CreateDestroyMilitiaPopUPRegions(void) {
  static int16_t sOldTown = 0;
  int32_t iCounter = 0;

  // create destroy militia pop up regions for mapscreen militia pop up box
  if (sSelectedMilitiaTown != 0) {
    sOldTown = sSelectedMilitiaTown;
  }

  if (fShowMilitia && sSelectedMilitiaTown && !gfMilitiaPopupCreated) {
    for (iCounter = 0; iCounter < 9; iCounter++) {
      MSYS_DefineRegion(&gMapScreenMilitiaBoxRegions[iCounter],
                        (int16_t)(MAP_MILITIA_BOX_POS_X + MAP_MILITIA_MAP_X +
                                  (iCounter % MILITIA_BOX_ROWS) * MILITIA_BOX_BOX_WIDTH),
                        (int16_t)(MAP_MILITIA_BOX_POS_Y + MAP_MILITIA_MAP_Y +
                                  (iCounter / MILITIA_BOX_ROWS) * MILITIA_BOX_BOX_HEIGHT),
                        (int16_t)(MAP_MILITIA_BOX_POS_X + MAP_MILITIA_MAP_X +
                                  (((iCounter) % MILITIA_BOX_ROWS) + 1) * MILITIA_BOX_BOX_WIDTH),
                        (int16_t)(MAP_MILITIA_BOX_POS_Y + MAP_MILITIA_MAP_Y +
                                  (((iCounter) / MILITIA_BOX_ROWS) + 1) * MILITIA_BOX_BOX_HEIGHT),
                        MSYS_PRIORITY_HIGHEST - 3, MSYS_NO_CURSOR, MilitiaRegionMoveCallback,
                        MilitiaRegionClickCallback);

      MSYS_SetRegionUserData(&gMapScreenMilitiaBoxRegions[iCounter], 0, iCounter);
    }

    // create militia panel buttons
    CreateMilitiaPanelBottomButton();

    gfMilitiaPopupCreated = TRUE;
  } else if (gfMilitiaPopupCreated && (!fShowMilitia || !sSelectedMilitiaTown)) {
    for (iCounter = 0; iCounter < 9; iCounter++) {
      // remove region
      MSYS_RemoveRegion(&gMapScreenMilitiaBoxRegions[iCounter]);
    }

    // handle the shutdown of the panel...there maybe people on the cursor, distribute them evenly
    // over all the sectors
    HandleShutDownOfMilitiaPanelIfPeopleOnTheCursor(sOldTown);

    DeleteMilitiaPanelBottomButton();

    gfMilitiaPopupCreated = FALSE;
  }

  return;
}

void RenderIconsPerSectorForSelectedTown(void) {
  uint8_t sBaseSectorValue = 0;
  uint8_t sCurrentSectorValue = 0;
  int32_t iCounter = 0;
  int32_t iNumberOfGreens = 0;
  int32_t iNumberOfRegulars = 0;
  int32_t iNumberOfElites = 0;
  int32_t iTotalNumberOfTroops = 0;
  int32_t iCurrentTroopIcon = 0;
  struct VObject *hVObject;
  int32_t iCurrentIcon = 0;
  int16_t sX, sY;
  wchar_t sString[32];
  uint8_t sSectorX = 0, sSectorY = 0;

  // get the sector value for the upper left corner
  sBaseSectorValue = GetBaseSectorForCurrentTown();

  // get militia video object
  GetVideoObject(&hVObject, guiMilitia);

  // render icons for map
  for (iCounter = 0; iCounter < 9; iCounter++) {
    // grab current sector value
    sCurrentSectorValue =
        sBaseSectorValue + ((iCounter % MILITIA_BOX_ROWS) + (iCounter / MILITIA_BOX_ROWS) * (16));

    sSectorX = SectorID8_X(sCurrentSectorValue);
    sSectorY = SectorID8_Y(sCurrentSectorValue);

    // skip sectors not in the selected town (nearby other towns or wilderness SAM Sites)
    if (GetTownIdForSector(sSectorX, sSectorY) != sSelectedMilitiaTown) {
      continue;
    }

    // get number of each
    struct MilitiaCount milCount = GetMilitiaInSector(sSectorX, sSectorY);
    iNumberOfGreens = milCount.green;
    iNumberOfRegulars = milCount.regular;
    iNumberOfElites = milCount.elite;

    // get total
    iTotalNumberOfTroops = iNumberOfGreens + iNumberOfRegulars + iNumberOfElites;

    // printf number of troops
    SetFont(FONT10ARIAL);
    swprintf(sString, ARR_SIZE(sString), L"%d", iTotalNumberOfTroops);
    FindFontRightCoordinates((int16_t)(MAP_MILITIA_BOX_POS_X + MAP_MILITIA_MAP_X +
                                       ((iCounter % MILITIA_BOX_ROWS) * MILITIA_BOX_BOX_WIDTH)),
                             (int16_t)(MAP_MILITIA_BOX_POS_Y + MAP_MILITIA_MAP_Y +
                                       ((iCounter / MILITIA_BOX_ROWS) * MILITIA_BOX_BOX_HEIGHT)),
                             MILITIA_BOX_BOX_WIDTH, 0, sString, FONT10ARIAL, &sX, &sY);

    if (StrategicMap[SectorID8To16(sCurrentSectorValue)].townID != BLANK_SECTOR &&
        !IsSectorEnemyControlled(SectorID8_X(sCurrentSectorValue),
                                 SectorID8_Y(sCurrentSectorValue))) {
      if (sSectorMilitiaMapSector != iCounter) {
        mprintf(sX, (int16_t)(sY + MILITIA_BOX_BOX_HEIGHT - 5), sString);
      } else {
        mprintf(sX - 15, (int16_t)(sY + MILITIA_BOX_BOX_HEIGHT - 5), sString);
      }
    }

    // now display
    for (iCurrentTroopIcon = 0; iCurrentTroopIcon < iTotalNumberOfTroops; iCurrentTroopIcon++) {
      // get screen x and y coords
      if (sSectorMilitiaMapSector == iCounter) {
        sX = (iCurrentTroopIcon % POPUP_MILITIA_ICONS_PER_ROW) * MEDIUM_MILITIA_ICON_SPACING +
             MAP_MILITIA_BOX_POS_X + MAP_MILITIA_MAP_X +
             ((iCounter % MILITIA_BOX_ROWS) * MILITIA_BOX_BOX_WIDTH) + 2;
        sY = (iCurrentTroopIcon / POPUP_MILITIA_ICONS_PER_ROW) * (MEDIUM_MILITIA_ICON_SPACING - 1) +
             MAP_MILITIA_BOX_POS_Y + MAP_MILITIA_MAP_Y +
             ((iCounter / MILITIA_BOX_ROWS) * MILITIA_BOX_BOX_HEIGHT) + 3;

        if (iCurrentTroopIcon < iNumberOfGreens) {
          iCurrentIcon = 5;
        } else if (iCurrentTroopIcon < iNumberOfGreens + iNumberOfRegulars) {
          iCurrentIcon = 6;
        } else {
          iCurrentIcon = 7;
        }
      } else {
        sX = (iCurrentTroopIcon % POPUP_MILITIA_ICONS_PER_ROW) * MEDIUM_MILITIA_ICON_SPACING +
             MAP_MILITIA_BOX_POS_X + MAP_MILITIA_MAP_X +
             ((iCounter % MILITIA_BOX_ROWS) * MILITIA_BOX_BOX_WIDTH) + 3;
        sY = (iCurrentTroopIcon / POPUP_MILITIA_ICONS_PER_ROW) * (MEDIUM_MILITIA_ICON_SPACING) +
             MAP_MILITIA_BOX_POS_Y + MAP_MILITIA_MAP_Y +
             ((iCounter / MILITIA_BOX_ROWS) * MILITIA_BOX_BOX_HEIGHT) + 3;

        if (iCurrentTroopIcon < iNumberOfGreens) {
          iCurrentIcon = 8;
        } else if (iCurrentTroopIcon < iNumberOfGreens + iNumberOfRegulars) {
          iCurrentIcon = 9;
        } else {
          iCurrentIcon = 10;
        }
      }

      BltVideoObject(FRAME_BUFFER, hVObject, (uint16_t)(iCurrentIcon), sX, sY,
                     VO_BLT_SRCTRANSPARENCY, NULL);
    }
  }

  return;
}

uint8_t GetBaseSectorForCurrentTown(void) {
  uint8_t sBaseSector = 0;

  // is the current town
  if (sSelectedMilitiaTown != 0) {
    sBaseSector = sBaseSectorList[(int16_t)(sSelectedMilitiaTown - 1)];
  }

  // return the current sector value
  return (sBaseSector);
}

void ShowHighLightedSectorOnMilitiaMap(void) {
  // show the highlighted sector on the militia map
  struct VObject *hVObject;
  int16_t sX = 0, sY = 0;

  if (sSectorMilitiaMapSector != -1) {
    sX = MAP_MILITIA_BOX_POS_X + MAP_MILITIA_MAP_X +
         ((sSectorMilitiaMapSector % MILITIA_BOX_ROWS) * MILITIA_BOX_BOX_WIDTH);
    sY = MAP_MILITIA_BOX_POS_Y + MAP_MILITIA_MAP_Y +
         ((sSectorMilitiaMapSector / MILITIA_BOX_ROWS) * MILITIA_BOX_BOX_HEIGHT);

    // get the object
    GetVideoObject(&hVObject, guiMilitiaSectorHighLight);

    // blt the object
    BltVideoObject(FRAME_BUFFER, hVObject, 0, sX, sY, VO_BLT_SRCTRANSPARENCY, NULL);
  }

  if (sSectorMilitiaMapSectorOutline != -1) {
    sX = MAP_MILITIA_BOX_POS_X + MAP_MILITIA_MAP_X +
         ((sSectorMilitiaMapSectorOutline % MILITIA_BOX_ROWS) * MILITIA_BOX_BOX_WIDTH);
    sY = MAP_MILITIA_BOX_POS_Y + MAP_MILITIA_MAP_Y +
         ((sSectorMilitiaMapSectorOutline / MILITIA_BOX_ROWS) * MILITIA_BOX_BOX_HEIGHT);

    // get the object
    GetVideoObject(&hVObject, guiMilitiaSectorOutline);

    // blt the object
    BltVideoObject(FRAME_BUFFER, hVObject, 0, sX, sY, VO_BLT_SRCTRANSPARENCY, NULL);
  }

  return;
}

void MilitiaRegionClickCallback(struct MOUSE_REGION *pRegion, int32_t iReason) {
  int32_t iValue = 0;

  iValue = MSYS_GetRegionUserData(pRegion, 0);

  if ((iReason & MSYS_CALLBACK_REASON_LBUTTON_UP)) {
    if (IsThisMilitiaTownSectorAllowable((int16_t)iValue)) {
      if (sSectorMilitiaMapSector == (int16_t)iValue) {
        sSectorMilitiaMapSector = -1;
      } else {
        sSectorMilitiaMapSector = (int16_t)iValue;
      }
    } else {
      sSectorMilitiaMapSector = -1;
    }
  }

  if ((iReason & MSYS_CALLBACK_REASON_RBUTTON_UP)) {
    sSectorMilitiaMapSector = -1;
  }
}

void MilitiaRegionMoveCallback(struct MOUSE_REGION *pRegion, int32_t iReason) {
  int32_t iValue = 0;

  iValue = MSYS_GetRegionUserData(pRegion, 0);

  if ((iReason & MSYS_CALLBACK_REASON_GAIN_MOUSE)) {
    if (IsThisMilitiaTownSectorAllowable((int16_t)iValue)) {
      sSectorMilitiaMapSectorOutline = (int16_t)iValue;
    } else {
      sSectorMilitiaMapSectorOutline = -1;
    }
  } else if (iReason & MSYS_CALLBACK_REASON_LOST_MOUSE) {
    sSectorMilitiaMapSectorOutline = -1;
  }
}

void CreateDestroyMilitiaSectorButtons(void) {
  static BOOLEAN fCreated = FALSE;
  static int16_t sOldSectorValue = -1;
  int16_t sX = 0, sY = 0;
  int32_t iCounter = 0;
  struct VObject *hVObject;

  if (sOldSectorValue == sSectorMilitiaMapSector && fShowMilitia && sSelectedMilitiaTown &&
      !fCreated && sSectorMilitiaMapSector != -1) {
    fCreated = TRUE;

    // given sector..place down the 3 buttons

    for (iCounter = 0; iCounter < 3; iCounter++) {
      // set screen x and y positions
      sX = MAP_MILITIA_BOX_POS_X + MAP_MILITIA_MAP_X +
           ((sSectorMilitiaMapSector % MILITIA_BOX_ROWS) * MILITIA_BOX_BOX_WIDTH);
      sY = MAP_MILITIA_BOX_POS_Y + MAP_MILITIA_MAP_Y +
           ((sSectorMilitiaMapSector / MILITIA_BOX_ROWS) * MILITIA_BOX_BOX_HEIGHT);

      // adjust offsets
      sX += MILITIA_BTN_OFFSET_X;
      sY += (iCounter * (MILITIA_BTN_HEIGHT) + 2);

      // set the button image
      giMapMilitiaButtonImage[iCounter] =
          LoadButtonImage("INTERFACE\\militia.sti", -1, 3, -1, 4, -1);

      // set the button value
      giMapMilitiaButton[iCounter] = QuickCreateButton(
          giMapMilitiaButtonImage[iCounter], sX, sY, BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST - 1,
          BtnGenericMouseMoveButtonCallback, (GUI_CALLBACK)MilitiaButtonCallback);

      // set button user data
      MSYS_SetBtnUserData(giMapMilitiaButton[iCounter], 0, iCounter);
      SpecifyButtonFont(giMapMilitiaButton[iCounter], FONT10ARIAL);
      SpecifyButtonUpTextColors(giMapMilitiaButton[iCounter], gsMilitiaSectorButtonColors[iCounter],
                                FONT_BLACK);
      SpecifyButtonDownTextColors(giMapMilitiaButton[iCounter],
                                  gsMilitiaSectorButtonColors[iCounter], FONT_BLACK);

      GetVideoObject(&hVObject, guiMilitia);

      SetButtonFastHelpText(giMapMilitiaButton[iCounter], pMilitiaButtonsHelpText[iCounter]);
    }

    // mark here the militia box left click region
    // MSYS_DefineRegion( &gMapScreenMilitiaRegion, ( int16_t ) ( MAP_MILITIA_BOX_POS_X ), (
    // int16_t
    // )( MAP_MILITIA_BOX_POS_Y  ), ( int16_t )( MAP_MILITIA_BOX_POS_X + pTrav->usWidth ), (
    // int16_t
    // )( MAP_MILITIA_BOX_POS_Y + pTrav->usHeight ), MSYS_PRIORITY_HIGHEST - 2, MSYS_NO_CURSOR,
    // MilitiaRegionMoveCallback, MilitiaBoxMaskBtnCallback );

    CreateScreenMaskForMoveBox();

    // ste the fact that the buttons were in fact created
    fMilitiaMapButtonsCreated = TRUE;
  } else if (fCreated && (sOldSectorValue != sSectorMilitiaMapSector || !fShowMilitia ||
                          !sSelectedMilitiaTown || sSectorMilitiaMapSector == -1)) {
    sOldSectorValue = sSectorMilitiaMapSector;
    fCreated = FALSE;

    // the militia box left click region
    //	MSYS_RemoveRegion( &gMapScreenMilitiaRegion );

    // get rid of the buttons
    for (iCounter = 0; iCounter < 3; iCounter++) {
      RemoveButton(giMapMilitiaButton[iCounter]);
      UnloadButtonImage(giMapMilitiaButtonImage[iCounter]);
    }

    if (!fShowMilitia || !sSelectedMilitiaTown) {
      sSectorMilitiaMapSector = -1;
      sSelectedMilitiaTown = 0;
    }

    RemoveScreenMaskForMoveBox();

    // set the fact that the buttons were destroyed
    fMilitiaMapButtonsCreated = FALSE;
  }

  sOldSectorValue = sSectorMilitiaMapSector;
}

void SetMilitiaMapButtonsText(void) {
  // now set the militia map button text
  wchar_t sString[64];
  int32_t iNumberOfGreens = 0, iNumberOfRegulars = 0, iNumberOfElites = 0;
  uint8_t sBaseSectorValue = 0, sGlobalMapSector = 0;

  if (!fMilitiaMapButtonsCreated) {
    return;
  }

  // grab the appropriate global sector value in the world
  sBaseSectorValue = GetBaseSectorForCurrentTown();
  sGlobalMapSector = sBaseSectorValue + ((sSectorMilitiaMapSector % MILITIA_BOX_ROWS) +
                                         (sSectorMilitiaMapSector / MILITIA_BOX_ROWS) * (16));

  struct MilitiaCount milCount = GetMilitiaInSectorID8(sGlobalMapSector);
  iNumberOfGreens = milCount.green;
  iNumberOfRegulars = milCount.regular;
  iNumberOfElites = milCount.elite;

  // the greens in this sector
  swprintf(sString, ARR_SIZE(sString), L"%d", iNumberOfGreens);
  SpecifyButtonText(giMapMilitiaButton[0], sString);

  // the regulars in this sector
  swprintf(sString, ARR_SIZE(sString), L"%d", iNumberOfRegulars);
  SpecifyButtonText(giMapMilitiaButton[1], sString);

  // the number of elites in this sector
  swprintf(sString, ARR_SIZE(sString), L"%d", iNumberOfElites);
  SpecifyButtonText(giMapMilitiaButton[2], sString);

  return;
}

void MilitiaButtonCallback(GUI_BUTTON *btn, int32_t reason) {
  uint8_t sGlobalMapSector = 0;
  uint8_t sBaseSectorValue = 0;
  int32_t iValue = 0;

  // is the button enabled
  if (!(btn->uiFlags & BUTTON_ENABLED)) return;

  // get the value
  iValue = MSYS_GetBtnUserData(btn, 0);

  // get the sector value for the upper left corner
  sBaseSectorValue = GetBaseSectorForCurrentTown();
  sGlobalMapSector = sBaseSectorValue + ((sSectorMilitiaMapSector % MILITIA_BOX_ROWS) +
                                         (sSectorMilitiaMapSector / MILITIA_BOX_ROWS) * (16));

  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    btn->uiFlags |= (BUTTON_CLICKED_ON);
  } else if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    if (btn->uiFlags & BUTTON_CLICKED_ON) {
      btn->uiFlags &= ~(BUTTON_CLICKED_ON);
      DropAPersonInASector((uint8_t)(iValue), (int16_t)((sGlobalMapSector % 16) + 1),
                           (int16_t)((sGlobalMapSector / 16) + 1));
    }
  } else if (reason & MSYS_CALLBACK_REASON_RBUTTON_DWN) {
    btn->uiFlags |= (BUTTON_CLICKED_ON);
  } else if (reason & MSYS_CALLBACK_REASON_RBUTTON_UP) {
    if (btn->uiFlags & BUTTON_CLICKED_ON) {
      btn->uiFlags &= ~(BUTTON_CLICKED_ON);
      PickUpATownPersonFromSector((uint8_t)(iValue), (int16_t)((sGlobalMapSector % 16) + 1),
                                  (int16_t)((sGlobalMapSector / 16) + 1));
    }
  }
}

void DisplayUnallocatedMilitia(void) {
  // show the nunber on the cursor
  int32_t iTotalNumberOfTroops = 0, iNumberOfGreens = 0, iNumberOfRegulars = 0, iNumberOfElites = 0,
          iCurrentTroopIcon = 0;
  int32_t iCurrentIcon = 0;
  int16_t sX = 0, sY = 0;
  struct VObject *hVObject;

  // get number of each
  iNumberOfGreens = sGreensOnCursor;
  iNumberOfRegulars = sRegularsOnCursor;
  iNumberOfElites = sElitesOnCursor;

  // get total
  iTotalNumberOfTroops = iNumberOfGreens + iNumberOfRegulars + iNumberOfElites;

  // get militia video object
  GetVideoObject(&hVObject, guiMilitia);

  // now display
  for (iCurrentTroopIcon = 0; iCurrentTroopIcon < iTotalNumberOfTroops; iCurrentTroopIcon++) {
    // get screen x and y coords
    sX = (iCurrentTroopIcon % NUMBER_OF_MILITIA_ICONS_PER_LOWER_ROW) * MEDIUM_MILITIA_ICON_SPACING +
         MAP_MILITIA_BOX_POS_X + MAP_MILITIA_MAP_X + 1;
    sY = (iCurrentTroopIcon / NUMBER_OF_MILITIA_ICONS_PER_LOWER_ROW) * MEDIUM_MILITIA_ICON_SPACING +
         MAP_MILITIA_BOX_POS_Y + MAP_MILITIA_LOWER_ROW_Y;

    if (iCurrentTroopIcon < iNumberOfGreens) {
      iCurrentIcon = 8;
    } else if (iCurrentTroopIcon < iNumberOfGreens + iNumberOfRegulars) {
      iCurrentIcon = 9;
    } else {
      iCurrentIcon = 10;
    }

    BltVideoObject(FRAME_BUFFER, hVObject, (uint16_t)(iCurrentIcon), sX, sY, VO_BLT_SRCTRANSPARENCY,
                   NULL);
  }
}

BOOLEAN IsThisMilitiaTownSectorAllowable(int16_t sSectorIndexValue) {
  uint8_t sBaseSectorValue = 0, sGlobalMapSector = 0;
  uint8_t sSectorX, sSectorY;

  // is this sector allowed to be clicked on?
  sBaseSectorValue = GetBaseSectorForCurrentTown();
  sGlobalMapSector = sBaseSectorValue + ((sSectorIndexValue % MILITIA_BOX_ROWS) +
                                         (sSectorIndexValue / MILITIA_BOX_ROWS) * 16);

  sSectorX = SectorID8_X(sGlobalMapSector);
  sSectorY = SectorID8_Y(sGlobalMapSector);

  // is this in fact part of a town?
  if (StrategicMap[GetSectorID16(sSectorX, sSectorY)].townID == BLANK_SECTOR) {
    return (FALSE);
  }

  if (!SectorOursAndPeaceful(sSectorX, sSectorY, 0)) {
    return (FALSE);
  }

  // valid
  return (TRUE);
}

void DrawTownMilitiaName(void) {
  wchar_t sString[64];
  int16_t sX, sY;

  // get the name for the current militia town
  swprintf(sString, ARR_SIZE(sString), L"%s %s", pTownNames[sSelectedMilitiaTown],
           pMilitiaString[0]);
  FindFontCenterCoordinates(MAP_MILITIA_BOX_POS_X,
                            MAP_MILITIA_BOX_POS_Y + MILITIA_BOX_TEXT_OFFSET_Y, MILITIA_BOX_WIDTH,
                            MILITIA_BOX_TEXT_TITLE_HEIGHT, sString, FONT10ARIAL, &sX, &sY);
  mprintf(sX, sY, sString);

  // might as well show the unassigned string
  swprintf(sString, ARR_SIZE(sString), L"%s %s", pTownNames[sSelectedMilitiaTown],
           pMilitiaString[1]);
  FindFontCenterCoordinates(
      MAP_MILITIA_BOX_POS_X, MAP_MILITIA_BOX_POS_Y + MILITIA_BOX_UNASSIGNED_TEXT_OFFSET_Y,
      MILITIA_BOX_WIDTH, GetFontHeight(FONT10ARIAL), sString, FONT10ARIAL, &sX, &sY);
  mprintf(sX, sY, sString);

  return;
}

void HandleShutDownOfMilitiaPanelIfPeopleOnTheCursor(int16_t sTownValue) {
  int32_t iCounter = 0, iCounterB = 0, iNumberUnderControl = 0, iNumberThatCanFitInSector = 0,
          iCount = 0;
  BOOLEAN fLastOne = FALSE;

  // check if anyone still on the cursor
  if (!sGreensOnCursor && !sRegularsOnCursor && !sElitesOnCursor) {
    return;
  }

  // yep someone left
  iNumberUnderControl = GetTownSectorsUnderControl((int8_t)sTownValue);

  // find number of sectors under player's control
  const TownSectors *townSectors = GetAllTownSectors();
  while ((*townSectors)[iCounter].townID != 0) {
    if ((*townSectors)[iCounter].townID == sTownValue) {
      SectorID16 sectorID = (*townSectors)[iCounter].sectorID;
      if (SectorOursAndPeaceful(SectorID16_X(sectorID), SectorID16_Y(sectorID), 0)) {
        iCount = 0;
        iNumberThatCanFitInSector = MAX_ALLOWABLE_MILITIA_PER_SECTOR;
        iNumberThatCanFitInSector -= CountAllMilitiaInSectorID8(SectorID16To8(sectorID));

        uint8_t sX = SectorID16_X(sectorID);
        uint8_t sY = SectorID16_Y(sectorID);

        while ((iCount < iNumberThatCanFitInSector) &&
               ((sGreensOnCursor) || (sRegularsOnCursor) || (sElitesOnCursor))) {
          // green
          if ((iCount + 1 <= iNumberThatCanFitInSector) && (sGreensOnCursor)) {
            IncMilitiaOfRankInSector(sX, sY, GREEN_MILITIA, 1);
            iCount++;
            sGreensOnCursor--;
          }

          // regular
          if ((iCount + 1 <= iNumberThatCanFitInSector) && (sRegularsOnCursor)) {
            IncMilitiaOfRankInSector(sX, sY, REGULAR_MILITIA, 1);
            iCount++;
            sRegularsOnCursor--;
          }

          // elite
          if ((iCount + 1 <= iNumberThatCanFitInSector) && (sElitesOnCursor)) {
            IncMilitiaOfRankInSector(sX, sY, ELITE_MILITIA, 1);
            iCount++;
            sElitesOnCursor--;
          }
        }

        if (SectorID16To8(sectorID) ==
            GetSectorID8((uint8_t)gWorldSectorX, (uint8_t)gWorldSectorY)) {
          TacticalMilitiaRefreshRequired();
        }
      }

      fLastOne = TRUE;

      iCounterB = iCounter + 1;

      const TownSectors *townSectors = GetAllTownSectors();
      while ((*townSectors)[iCounterB].townID != 0) {
        if ((*townSectors)[iCounterB].townID == sTownValue) {
          fLastOne = FALSE;
        }

        iCounterB++;
      }

      if (fLastOne) {
        uint8_t sX = SectorID16_X(sectorID);
        uint8_t sY = SectorID16_Y(sectorID);
        IncMilitiaOfRankInSector(sX, sY, GREEN_MILITIA, sGreensOnCursor % iNumberUnderControl);
        IncMilitiaOfRankInSector(sX, sY, REGULAR_MILITIA, sRegularsOnCursor % iNumberUnderControl);
        IncMilitiaOfRankInSector(sX, sY, ELITE_MILITIA, sElitesOnCursor % iNumberUnderControl);
      }
    }

    iCounter++;
  }

  // zero out numbers on the cursor
  sGreensOnCursor = 0;
  sRegularsOnCursor = 0;
  sElitesOnCursor = 0;

  return;
}

void HandleEveningOutOfTroopsAmongstSectors(void) {
  // even out troops among the town
  int32_t iCounter = 0, iNumberUnderControl = 0, iNumberOfGreens = 0, iNumberOfRegulars = 0,
          iNumberOfElites = 0, iTotalNumberOfTroops = 0;
  int32_t iNumberLeftOverGreen = 0, iNumberLeftOverRegular = 0, iNumberLeftOverElite = 0;
  uint8_t sBaseSectorValue = 0, sCurrentSectorValue = 0;
  int16_t sTotalSoFar = 0;

  // how many sectors in the selected town do we control?
  iNumberUnderControl = GetTownSectorsUnderControl((int8_t)sSelectedMilitiaTown);

  // if none, there's nothing to be done!
  if (!iNumberUnderControl) {
    return;
  }

  // get the sector value for the upper left corner
  sBaseSectorValue = GetBaseSectorForCurrentTown();

  // render icons for map
  for (iCounter = 0; iCounter < 9; iCounter++) {
    // grab current sector value
    sCurrentSectorValue =
        sBaseSectorValue + ((iCounter % MILITIA_BOX_ROWS) + (iCounter / MILITIA_BOX_ROWS) * (16));

    uint8_t sSectorX = SectorID8_X(sCurrentSectorValue);
    uint8_t sSectorY = SectorID8_Y(sCurrentSectorValue);

    // skip sectors not in the selected town (nearby other towns or wilderness SAM Sites)
    if (GetTownIdForSector(sSectorX, sSectorY) != sSelectedMilitiaTown) {
      continue;
    }

    if (!IsSectorEnemyControlled(sSectorX, sSectorY)) {
      struct MilitiaCount milCount = GetMilitiaInSector(sSectorX, sSectorY);
      iNumberOfGreens += milCount.green;
      iNumberOfRegulars += milCount.regular;
      iNumberOfElites += milCount.elite;
    }
  }

  // now grab those on the cursor
  iNumberOfGreens += sGreensOnCursor;
  iNumberOfRegulars += sRegularsOnCursor;
  iNumberOfElites += sElitesOnCursor;

  // now get the left overs
  iNumberLeftOverGreen = iNumberOfGreens % iNumberUnderControl;
  iNumberLeftOverRegular = iNumberOfRegulars % iNumberUnderControl;
  iNumberLeftOverElite = iNumberOfElites % iNumberUnderControl;

  // get total
  iTotalNumberOfTroops = iNumberOfGreens + iNumberOfRegulars + iNumberOfElites;

  if (!iTotalNumberOfTroops) {
    return;
  }

  iCounter = 0;
  const TownSectors *townSectors = GetAllTownSectors();
  while ((*townSectors)[iCounter].townID != 0) {
    if ((*townSectors)[iCounter].townID == sSelectedMilitiaTown) {
      uint8_t sX = SectorID16_X((*townSectors)[iCounter].sectorID);
      uint8_t sY = SectorID16_Y((*townSectors)[iCounter].sectorID);

      if (!IsSectorEnemyControlled(sX, sY) && !NumHostilesInSector(sX, sY, 0)) {
        // distribute here
        SetMilitiaOfRankInSector(sX, sY, GREEN_MILITIA, iNumberOfGreens / iNumberUnderControl);
        SetMilitiaOfRankInSector(sX, sY, REGULAR_MILITIA, iNumberOfRegulars / iNumberUnderControl);
        SetMilitiaOfRankInSector(sX, sY, ELITE_MILITIA, iNumberOfElites / iNumberUnderControl);
        sTotalSoFar = CountAllMilitiaInSector(sX, sY);

        // add leftovers that weren't included in the div operation
        if ((iNumberLeftOverGreen) && (sTotalSoFar < MAX_ALLOWABLE_MILITIA_PER_SECTOR)) {
          IncMilitiaOfRankInSector(sX, sY, GREEN_MILITIA, 1);
          sTotalSoFar++;
          iNumberLeftOverGreen--;
        }

        if ((iNumberLeftOverRegular) && (sTotalSoFar < MAX_ALLOWABLE_MILITIA_PER_SECTOR)) {
          IncMilitiaOfRankInSector(sX, sY, REGULAR_MILITIA, 1);
          sTotalSoFar++;
          iNumberLeftOverRegular--;
        }

        if ((iNumberLeftOverElite) && (sTotalSoFar < MAX_ALLOWABLE_MILITIA_PER_SECTOR)) {
          IncMilitiaOfRankInSector(sX, sY, ELITE_MILITIA, 1);
          sTotalSoFar++;
          iNumberLeftOverElite--;
        }

        // if this sector is currently loaded
        if (GetSectorID8(sX, sY) == GetSectorID8((uint8_t)gWorldSectorX, (uint8_t)gWorldSectorY) &&
            gWorldSectorY != 0) {
          TacticalMilitiaRefreshRequired();
        }
      }
    }

    iCounter++;
  }

  // zero out numbers on the cursor
  sGreensOnCursor = 0;
  sRegularsOnCursor = 0;
  sElitesOnCursor = 0;

  return;
}

void CreateMilitiaPanelBottomButton(void) {
  // set the button image
  giMapMilitiaButtonImage[3] = LoadButtonImage("INTERFACE\\militia.sti", -1, 1, -1, 2, -1);
  giMapMilitiaButtonImage[4] = LoadButtonImage("INTERFACE\\militia.sti", -1, 1, -1, 2, -1);

  giMapMilitiaButton[3] = QuickCreateButton(
      giMapMilitiaButtonImage[3], MAP_MILITIA_BOX_POS_X + MAP_MILITIA_BOX_AUTO_BOX_X,
      MAP_MILITIA_BOX_POS_Y + MAP_MILITIA_BOX_AUTO_BOX_Y, BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST - 1,
      BtnGenericMouseMoveButtonCallback, (GUI_CALLBACK)MilitiaAutoButtonCallback);

  giMapMilitiaButton[4] = QuickCreateButton(
      giMapMilitiaButtonImage[4], MAP_MILITIA_BOX_POS_X + MAP_MILITIA_BOX_DONE_BOX_X,
      MAP_MILITIA_BOX_POS_Y + MAP_MILITIA_BOX_AUTO_BOX_Y, BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST - 1,
      BtnGenericMouseMoveButtonCallback, (GUI_CALLBACK)MilitiaDoneButtonCallback);

  SpecifyButtonFont(giMapMilitiaButton[3], FONT10ARIAL);
  SpecifyButtonUpTextColors(giMapMilitiaButton[3], FONT_BLACK, FONT_BLACK);
  SpecifyButtonDownTextColors(giMapMilitiaButton[3], FONT_BLACK, FONT_BLACK);

  SpecifyButtonFont(giMapMilitiaButton[4], FONT10ARIAL);
  SpecifyButtonUpTextColors(giMapMilitiaButton[4], FONT_BLACK, FONT_BLACK);
  SpecifyButtonDownTextColors(giMapMilitiaButton[4], FONT_BLACK, FONT_BLACK);

  SpecifyButtonText(giMapMilitiaButton[3], pMilitiaButtonString[0]);
  SpecifyButtonText(giMapMilitiaButton[4], pMilitiaButtonString[1]);

  // AUTO button help
  SetButtonFastHelpText(giMapMilitiaButton[3], pMilitiaButtonsHelpText[3]);

  // if auto-distribution is not possible
  if (!CanMilitiaAutoDistribute()) {
    // disable the AUTO button
    DisableButton(giMapMilitiaButton[3]);
  }

  return;
}

void DeleteMilitiaPanelBottomButton(void) {
  // delete militia panel bottom
  RemoveButton(giMapMilitiaButton[3]);
  RemoveButton(giMapMilitiaButton[4]);

  UnloadButtonImage(giMapMilitiaButtonImage[3]);
  UnloadButtonImage(giMapMilitiaButtonImage[4]);

  if (sSelectedMilitiaTown != 0) {
    HandleShutDownOfMilitiaPanelIfPeopleOnTheCursor(sSelectedMilitiaTown);
  }

  // redraw the map
  MarkForRedrawalStrategicMap();
}

void MilitiaAutoButtonCallback(GUI_BUTTON *btn, int32_t reason) {
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    btn->uiFlags |= (BUTTON_CLICKED_ON);
  } else if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    if (btn->uiFlags & BUTTON_CLICKED_ON) {
      btn->uiFlags &= ~(BUTTON_CLICKED_ON);

      // distribute troops over all the sectors under control
      HandleEveningOutOfTroopsAmongstSectors();
      MarkForRedrawalStrategicMap();
    }
  }

  return;
}

void MilitiaDoneButtonCallback(GUI_BUTTON *btn, int32_t reason) {
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    btn->uiFlags |= (BUTTON_CLICKED_ON);
  } else if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    if (btn->uiFlags & BUTTON_CLICKED_ON) {
      btn->uiFlags &= ~(BUTTON_CLICKED_ON);

      // reset fact we are in the box
      sSelectedMilitiaTown = 0;
      MarkForRedrawalStrategicMap();
    }
  }

  return;
}

void RenderShadingForUnControlledSectors(void) {
  // now render shading over any uncontrolled sectors
  // get the sector value for the upper left corner
  uint8_t sBaseSectorValue = 0, sCurrentSectorValue = 0, sX = 0, sY = 0;
  int32_t iCounter = 0;

  // get the base sector value
  sBaseSectorValue = GetBaseSectorForCurrentTown();

  // render icons for map
  for (iCounter = 0; iCounter < 9; iCounter++) {
    // grab current sector value
    sCurrentSectorValue =
        sBaseSectorValue + ((iCounter % MILITIA_BOX_ROWS) + (iCounter / MILITIA_BOX_ROWS) * (16));

    if ((StrategicMap[SectorID8To16(sCurrentSectorValue)].townID != BLANK_SECTOR) &&
        ((IsSectorEnemyControlled(SectorID8_X(sCurrentSectorValue),
                                  SectorID8_Y(sCurrentSectorValue))) ||
         (NumHostilesInSector(SectorID8_X(sCurrentSectorValue), SectorID8_Y(sCurrentSectorValue),
                              0)))) {
      // shade this sector, not under our control
      sX = MAP_MILITIA_BOX_POS_X + MAP_MILITIA_MAP_X +
           ((iCounter % MILITIA_BOX_ROWS) * MILITIA_BOX_BOX_WIDTH);
      sY = MAP_MILITIA_BOX_POS_Y + MAP_MILITIA_MAP_Y +
           ((iCounter / MILITIA_BOX_ROWS) * MILITIA_BOX_BOX_HEIGHT);

      ShadowVideoSurfaceRect(FRAME_BUFFER, sX, sY, sX + MILITIA_BOX_BOX_WIDTH - 1,
                             sY + MILITIA_BOX_BOX_HEIGHT - 1);
    }
  }

  return;
}

void DrawTownMilitiaForcesOnMap(void) {
  int32_t iCounter = 0, iCounterB = 0, iTotalNumberOfTroops = 0, iIconValue = 0;
  struct VObject *hVObject;

  // get militia video object
  GetVideoObject(&hVObject, guiMilitia);

  // clip blits to mapscreen region
  ClipBlitsToMapViewRegion();

  const TownSectors *townSectors = GetAllTownSectors();
  while ((*townSectors)[iCounter].townID != 0) {
    // run through each town sector and plot the icons for the militia forces in the town
    uint8_t sSectorX = SectorID16_X((*townSectors)[iCounter].sectorID);
    uint8_t sSectorY = SectorID16_Y((*townSectors)[iCounter].sectorID);
    if (!IsSectorEnemyControlled(sSectorX, sSectorY)) {
      struct MilitiaCount milCount = GetMilitiaInSector(sSectorX, sSectorY);

      // set the total for loop upper bound
      iTotalNumberOfTroops = milCount.green + milCount.regular + milCount.elite;

      for (iCounterB = 0; iCounterB < iTotalNumberOfTroops; iCounterB++) {
        if (fZoomFlag) {
          // LARGE icon offset in the .sti
          iIconValue = 11;
        } else {
          // SMALL icon offset in the .sti
          iIconValue = 5;
        }

        // get the offset further into the .sti
        if (iCounterB < milCount.green) {
          iIconValue += 0;
        } else if (iCounterB < milCount.green + milCount.regular) {
          iIconValue += 1;
        } else {
          iIconValue += 2;
        }

        DrawMapBoxIcon(hVObject, (uint16_t)iIconValue, sSectorX, sSectorY, (uint8_t)iCounterB);
      }
    }

    iCounter++;
  }

  // now handle militia for sam sectors
  for (iCounter = 0; iCounter < GetSamSiteCount(); iCounter++) {
    uint8_t sSectorX = GetSamSiteX(iCounter);
    uint8_t sSectorY = GetSamSiteY(iCounter);
    struct MilitiaCount milCount = GetMilitiaInSector(sSectorX, sSectorY);

    if (!IsSectorEnemyControlled(sSectorX, sSectorY)) {
      // ste the total for loop upper bound
      iTotalNumberOfTroops = milCount.green + milCount.regular + milCount.elite;

      for (iCounterB = 0; iCounterB < iTotalNumberOfTroops; iCounterB++) {
        if (fZoomFlag) {
          // LARGE icon offset in the .sti
          iIconValue = 11;
        } else {
          // SMALL icon offset in the .sti
          iIconValue = 5;
        }

        // get the offset further into the .sti
        if (iCounterB < milCount.green) {
          iIconValue += 0;
        } else if (iCounterB < milCount.green + milCount.regular) {
          iIconValue += 1;
        } else {
          iIconValue += 2;
        }

        DrawMapBoxIcon(hVObject, (uint16_t)iIconValue, sSectorX, sSectorY, (uint8_t)iCounterB);
      }
    }
  }
  // restore clip blits
  RestoreClipRegionToFullScreen();

  return;
}

void CheckAndUpdateStatesOfSelectedMilitiaSectorButtons(void) {
  // now set the militia map button text
  int32_t iNumberOfGreens = 0, iNumberOfRegulars = 0, iNumberOfElites = 0;
  uint8_t sBaseSectorValue = 0, sGlobalMapSector = 0;

  if (!fMilitiaMapButtonsCreated) {
    EnableButton(giMapMilitiaButton[4]);
    return;
  }

  // grab the appropriate global sector value in the world
  sBaseSectorValue = GetBaseSectorForCurrentTown();
  sGlobalMapSector = sBaseSectorValue + ((sSectorMilitiaMapSector % MILITIA_BOX_ROWS) +
                                         (sSectorMilitiaMapSector / MILITIA_BOX_ROWS) * (16));

  struct MilitiaCount milCount = GetMilitiaInSectorID8(sGlobalMapSector);
  iNumberOfGreens = milCount.green + sGreensOnCursor;
  iNumberOfRegulars = milCount.regular + sRegularsOnCursor;
  iNumberOfElites = milCount.elite + sElitesOnCursor;

  if ((sGreensOnCursor > 0) || (sRegularsOnCursor > 0) || (sElitesOnCursor > 0)) {
    DisableButton(giMapMilitiaButton[4]);  // DONE
  } else {
    EnableButton(giMapMilitiaButton[4]);  // DONE
  }

  // greens button
  if (!iNumberOfGreens) {
    DisableButton(giMapMilitiaButton[0]);
  } else {
    EnableButton(giMapMilitiaButton[1]);
  }

  // regulars button
  if (!iNumberOfRegulars) {
    DisableButton(giMapMilitiaButton[1]);
  } else {
    EnableButton(giMapMilitiaButton[1]);
  }

  // elites button
  if (!iNumberOfElites) {
    DisableButton(giMapMilitiaButton[2]);
  } else {
    EnableButton(giMapMilitiaButton[2]);
  }

  return;
}

BOOLEAN ShadeUndergroundMapElem(uint8_t sSectorX, uint8_t sSectorY) {
  int16_t sScreenX, sScreenY;

  GetScreenXYFromMapXY(sSectorX, sSectorY, &sScreenX, &sScreenY);

  sScreenX += 1;

  ShadowVideoSurfaceRect(guiSAVEBUFFER, sScreenX, sScreenY, sScreenX + MAP_GRID_X - 2,
                         sScreenY + MAP_GRID_Y - 2);

  return (TRUE);
}

void ShadeSubLevelsNotVisited(void) {
  UNDERGROUND_SECTORINFO *pNode = gpUndergroundSectorInfoHead;

  // obtain the 16-bit version of the same color used in the mine STIs
  gusUndergroundNearBlack = Get16BPPColor(FROMRGB(2, 2, 0));

  // run through all (real & possible) underground sectors
  while (pNode) {
    // if the sector is on the currently displayed sublevel, and has never been visited
    if (pNode->ubSectorZ == (uint8_t)iCurrentMapSectorZ && !pNode->fVisited) {
      // remove that portion of the "mine" graphics from view
      HideExistenceOfUndergroundMapSector(pNode->ubSectorX, pNode->ubSectorY);
    }

    pNode = pNode->next;
  }
}

void HandleLowerLevelMapBlit(void) {
  struct VObject *hHandle;

  // blits the sub level maps
  switch (iCurrentMapSectorZ) {
    case (1):
      GetVideoObject(&hHandle, guiSubLevel1);
      break;
    case (2):
      GetVideoObject(&hHandle, guiSubLevel2);
      break;
    case (3):
      GetVideoObject(&hHandle, guiSubLevel3);
      break;
  }

  // handle the blt of the sublevel
  BltVideoObject(guiSAVEBUFFER, hHandle, 0, MAP_VIEW_START_X + 21, MAP_VIEW_START_Y + 17,
                 VO_BLT_SRCTRANSPARENCY, NULL);

  // handle shading of sublevels
  ShadeSubLevelsNotVisited();

  return;
}

void MilitiaBoxMaskBtnCallback(struct MOUSE_REGION *pRegion, int32_t iReason) {
  // btn callback handler for assignment screen mask region
  if ((iReason & MSYS_CALLBACK_REASON_LBUTTON_UP)) {
    sSectorMilitiaMapSector = -1;
  } else if (iReason & MSYS_CALLBACK_REASON_RBUTTON_UP) {
    sSectorMilitiaMapSector = -1;
  }

  return;
}

// There is a special case flag used when players encounter enemies in a sector, then retreat. The
// number of enemies will display on mapscreen until time is compressed.  When time is compressed,
// the flag is cleared, and a question mark is displayed to reflect that the player no longer
// knows. This is the function that clears that flag.
void ClearAnySectorsFlashingNumberOfEnemies() {
  int32_t i;
  for (i = 0; i < 256; i++) {
    SectorInfo[i].uiFlags &= ~SF_PLAYER_KNOWS_ENEMIES_ARE_HERE;
  }

  // redraw map
  MarkForRedrawalStrategicMap();
}

uint32_t WhatPlayerKnowsAboutEnemiesInSector(uint8_t sSectorX, uint8_t sSectorY) {
  uint32_t uiSectorFlags = SectorInfo[GetSectorID8(sSectorX, sSectorY)].uiFlags;

  // if player has militia close enough to scout this sector out, if there are mercs who can scout
  // here, OR
  // Special case flag used when players encounter enemies in a sector, then retreat.  The number
  // of enemies will display on mapscreen until time is compressed.  When time is compressed, the
  // flag is cleared, and a question mark is displayed to reflect that the player no longer knows.
  if (CanMercsScoutThisSector(sSectorX, sSectorY, 0) ||
      CanNearbyMilitiaScoutThisSector(sSectorX, sSectorY) ||
      (uiSectorFlags & SF_PLAYER_KNOWS_ENEMIES_ARE_HERE)) {
    // if the enemies are stationary (i.e. mercs attacking a garrison)
    if (NumStationaryEnemiesInSector(sSectorX, sSectorY) > 0) {
      // inside a garrison - hide their # (show question mark) to match what the PBI is showing
      return KNOWS_THEYRE_THERE;
    } else {
      // other situations - show exactly how many there are
      return KNOWS_HOW_MANY;
    }
  }

  // if the player has visited the sector during this game
  if (GetSectorFlagStatus(sSectorX, sSectorY, 0, SF_ALREADY_VISITED) == TRUE) {
    // then he always knows about any enemy presence for the remainder of the game, but not exact
    // numbers
    return KNOWS_THEYRE_THERE;
  }

  // if Skyrider noticed the enemis in the sector recently
  if (uiSectorFlags & SF_SKYRIDER_NOTICED_ENEMIES_HERE) {
    // and Skyrider is still in this sector, flying
    if (IsSkyriderIsFlyingInSector(sSectorX, sSectorY)) {
      // player remains aware of them as long as Skyrider remains in the sector
      return KNOWS_THEYRE_THERE;
    } else {
      // Skyrider is gone, reset the flag that he noticed enemies here
      SectorInfo[GetSectorID8(sSectorX, sSectorY)].uiFlags &= ~SF_SKYRIDER_NOTICED_ENEMIES_HERE;
    }
  }

  // no information available
  return KNOWS_NOTHING;
}

BOOLEAN CanMercsScoutThisSector(uint8_t sSectorX, uint8_t sSectorY, int8_t bSectorZ) {
  int32_t iFirstId = 0, iLastId = 0;
  int32_t iCounter = 0;
  struct SOLDIERTYPE *pSoldier = NULL;

  // to speed it up a little?
  iFirstId = gTacticalStatus.Team[OUR_TEAM].bFirstID;
  iLastId = gTacticalStatus.Team[OUR_TEAM].bLastID;

  for (iCounter = iFirstId; iCounter <= iLastId; iCounter++) {
    // get the soldier
    pSoldier = GetSoldierByID(iCounter);

    // is the soldier active
    if (IsSolActive(pSoldier) == FALSE) {
      continue;
    }

    // vehicles can't scout!
    if (pSoldier->uiStatusFlags & SOLDIER_VEHICLE) {
      continue;
    }

    // POWs, dead guys, guys in transit, sleeping, and really hurt guys can't scout!
    if ((GetSolAssignment(pSoldier) == IN_TRANSIT) ||
        (GetSolAssignment(pSoldier) == ASSIGNMENT_POW) ||
        (GetSolAssignment(pSoldier) == ASSIGNMENT_DEAD) || (pSoldier->fMercAsleep == TRUE) ||
        (pSoldier->bLife < OKLIFE)) {
      continue;
    }

    // don't count mercs aboard Skyrider
    if ((GetSolAssignment(pSoldier) == VEHICLE) && (pSoldier->iVehicleId == iHelicopterVehicleId)) {
      continue;
    }

    // mercs on the move can't scout
    if (pSoldier->fBetweenSectors) {
      continue;
    }

    // is he here?
    if ((GetSolSectorX(pSoldier) == sSectorX) && (GetSolSectorY(pSoldier) == sSectorY) &&
        (GetSolSectorZ(pSoldier) == bSectorZ)) {
      return (TRUE);
    }
  }

  // none here who can scout
  return (FALSE);
}

void HandleShowingOfEnemyForcesInSector(uint8_t sSectorX, uint8_t sSectorY, int8_t bSectorZ,
                                        uint8_t ubIconPosition) {
  int16_t sNumberOfEnemies = 0;

  // ATE: If game has just started, don't do it!
  if (DidGameJustStart()) {
    return;
  }

  // never display enemies underground - sector info doesn't have support for it!
  if (bSectorZ != 0) {
    return;
  }

  // get total number of badguys here
  sNumberOfEnemies = NumEnemiesInSector(sSectorX, sSectorY);

  // anyone here?
  if (!sNumberOfEnemies) {
    // nope - display nothing
    return;
  }

  switch (WhatPlayerKnowsAboutEnemiesInSector(sSectorX, sSectorY)) {
    case KNOWS_NOTHING:
      // display nothing
      break;

    case KNOWS_THEYRE_THERE:
      // display a question mark
      ShowUncertainNumberEnemiesInSector(sSectorX, sSectorY);
      break;

    case KNOWS_HOW_MANY:
      // display individual icons for each enemy, starting at the received icon position index
      ShowEnemiesInSector(sSectorX, sSectorY, sNumberOfEnemies, ubIconPosition);
      break;
  }
}

void ShowSAMSitesOnStrategicMap(void) {
  int32_t iCounter = 0;
  int16_t sX = 0, sY = 0;
  struct VObject *hHandle;
  int8_t ubVidObjIndex = 0;
  uint32_t uiDestPitchBYTES;
  wchar_t wString[40];

  if (fShowAircraftFlag) {
    BlitSAMGridMarkers();
  }

  for (iCounter = 0; iCounter < GetSamSiteCount(); iCounter++) {
    // has the sam site here been found?
    if (!IsSamSiteFound(iCounter)) {
      continue;
    }

    // get the sector x and y
    uint8_t sSectorX = GetSamSiteX(iCounter);
    uint8_t sSectorY = GetSamSiteY(iCounter);

    if (fZoomFlag) {
      LockVideoSurface(guiSAVEBUFFER, &uiDestPitchBYTES);
      SetClippingRegionAndImageWidth(uiDestPitchBYTES, MAP_VIEW_START_X + MAP_GRID_X - 1,
                                     MAP_VIEW_START_Y + MAP_GRID_Y - 1, MAP_VIEW_WIDTH + 1,
                                     MAP_VIEW_HEIGHT - 9);
      UnLockVideoSurface(guiSAVEBUFFER);

      GetScreenXYFromMapXYStationary(sSectorX, sSectorY, &sX, &sY);
      sX -= 8;
      sY -= 10;
      ubVidObjIndex = 0;
    } else {
      GetScreenXYFromMapXY(sSectorX, sSectorY, &sX, &sY);
      sX += 5;
      sY += 3;
      ubVidObjIndex = 1;
    }

    // draw SAM site icon
    GetVideoObject(&hHandle, guiSAMICON);
    BltVideoObject(guiSAVEBUFFER, hHandle, ubVidObjIndex, sX, sY, VO_BLT_SRCTRANSPARENCY, NULL);

    if (fShowAircraftFlag) {
      // write "SAM Site" centered underneath

      if (fZoomFlag) {
        sX += 9;
        sY += 19;
      } else {
        sX += 6;
        sY += 16;
      }

      wcscpy(wString, pLandTypeStrings[SAM_SITE]);

      // we're CENTERING the first string AROUND sX, so calculate the starting X value
      sX -= StringPixLength(wString, MAP_FONT) / 2;

      // if within view region...render, else don't
      if ((sX > MAP_VIEW_START_X + MAP_VIEW_WIDTH) || (sX < MAP_VIEW_START_X) ||
          (sY > MAP_VIEW_START_Y + MAP_VIEW_HEIGHT) || (sY < MAP_VIEW_START_Y)) {
        continue;
      }

      SetFontDestBuffer(guiSAVEBUFFER, MapScreenRect.iLeft + 2, MapScreenRect.iTop,
                        MapScreenRect.iRight, MapScreenRect.iBottom, FALSE);

      // clip blits to mapscreen region
      ClipBlitsToMapViewRegion();

      SetFont(MAP_FONT);
      // Green on green doesn't contrast well, use Yellow
      SetFontForeground(FONT_MCOLOR_LTYELLOW);
      SetFontBackground(FONT_MCOLOR_BLACK);

      // draw the text
      gprintfdirty(sX, sY, wString);
      mprintf(sX, sY, wString);

      // restore clip blits
      RestoreClipRegionToFullScreen();
    }
  }

  return;
}

void BlitSAMGridMarkers(void) {
  int16_t sScreenX = 0, sScreenY = 0;
  uint32_t uiDestPitchBYTES;
  uint8_t *pDestBuf;
  uint16_t usColor = 0;
  int32_t iCounter = 0;
  int16_t sWidth = 0, sHeight = 0;

  // get 16 bpp color
  usColor = Get16BPPColor(FROMRGB(100, 100, 100));

  pDestBuf = LockVideoSurface(guiSAVEBUFFER, &uiDestPitchBYTES);

  // clip to view region
  ClipBlitsToMapViewRegionForRectangleAndABit(uiDestPitchBYTES);

  for (iCounter = 0; iCounter < GetSamSiteCount(); iCounter++) {
    // has the sam site here been found?
    if (!IsSamSiteFound(iCounter)) {
      continue;
    }

    uint8_t sX = GetSamSiteX(iCounter);
    uint8_t sY = GetSamSiteY(iCounter);

    if (fZoomFlag) {
      GetScreenXYFromMapXYStationary(sX, sY, &sScreenX, &sScreenY);
      sScreenX -= MAP_GRID_X;
      sScreenY -= MAP_GRID_Y;

      sWidth = 2 * MAP_GRID_X;
      sHeight = 2 * MAP_GRID_Y;
    } else {
      // get location on screen
      GetScreenXYFromMapXY(sX, sY, &sScreenX, &sScreenY);
      sWidth = MAP_GRID_X;
      sHeight = MAP_GRID_Y;
    }

    // draw rectangle
    RectangleDraw(TRUE, sScreenX, sScreenY - 1, sScreenX + sWidth, sScreenY + sHeight - 1, usColor,
                  pDestBuf);
  }

  // restore clips
  RestoreClipRegionToFullScreenForRectangle(uiDestPitchBYTES);

  // unlock surface
  UnLockVideoSurface(guiSAVEBUFFER);

  return;
}

BOOLEAN CanMilitiaAutoDistribute(void) {
  int32_t iCounter = 0;
  uint8_t sBaseSectorValue = 0, sCurrentSectorValue = 0;
  uint8_t sSectorX = 0, sSectorY = 0;
  int32_t iTotalTroopsInTown = 0;

  // can't auto-distribute if we don't have a town selected (this excludes SAM sites)
  if (sSelectedMilitiaTown == BLANK_SECTOR) return (FALSE);

  // can't auto-distribute if we don't control any sectors in the the town
  if (!GetTownSectorsUnderControl((int8_t)sSelectedMilitiaTown)) return (FALSE);

  // get the sector value for the upper left corner
  sBaseSectorValue = GetBaseSectorForCurrentTown();

  // render icons for map
  for (iCounter = 0; iCounter < 9; iCounter++) {
    // grab current sector value
    sCurrentSectorValue =
        sBaseSectorValue + ((iCounter % MILITIA_BOX_ROWS) + (iCounter / MILITIA_BOX_ROWS) * (16));

    sSectorX = SectorID8_X(sCurrentSectorValue);
    sSectorY = SectorID8_Y(sCurrentSectorValue);

    // skip sectors not in the selected town (nearby other towns or wilderness SAM Sites)
    if (GetTownIdForSector(sSectorX, sSectorY) != sSelectedMilitiaTown) {
      continue;
    }

    if (!IsSectorEnemyControlled(sSectorX, sSectorY)) {
      // get number of each
      iTotalTroopsInTown += CountAllMilitiaInSectorID8(sCurrentSectorValue);
    }
  }

  // can't auto-distribute if we don't have any militia in the town
  if (!iTotalTroopsInTown) return (FALSE);

  // can auto-distribute
  return (TRUE);
}

void ShowItemsOnMap(void) {
  uint8_t sMapX, sMapY;
  int16_t sXCorner, sYCorner;
  int16_t usXPos, usYPos;
  uint32_t uiItemCnt;
  wchar_t sString[10];

  // clip blits to mapscreen region
  ClipBlitsToMapViewRegion();

  SetFontDestBuffer(guiSAVEBUFFER, MapScreenRect.iLeft + 2, MapScreenRect.iTop,
                    MapScreenRect.iRight, MapScreenRect.iBottom, FALSE);

  SetFont(MAP_FONT);
  SetFontForeground(FONT_MCOLOR_LTGREEN);
  SetFontBackground(FONT_MCOLOR_BLACK);

  // run through sectors
  for (sMapX = 1; sMapX < (MAP_WORLD_X - 1); sMapX++) {
    for (sMapY = 1; sMapY < (MAP_WORLD_Y - 1); sMapY++) {
      // to speed this up, only look at sector that player has visited
      if (GetSectorFlagStatus(sMapX, sMapY, (uint8_t)iCurrentMapSectorZ, SF_ALREADY_VISITED)) {
        //				uiItemCnt = GetSizeOfStashInSector( sMapX, sMapY, ( int16_t
        //)
        // iCurrentMapSectorZ, FALSE );
        uiItemCnt = GetNumberOfVisibleWorldItemsFromSectorStructureForSector(
            sMapX, sMapY, (uint8_t)iCurrentMapSectorZ);

        if (uiItemCnt > 0) {
          sXCorner = (int16_t)(MAP_VIEW_START_X + (sMapX * MAP_GRID_X));
          sYCorner = (int16_t)(MAP_VIEW_START_Y + (sMapY * MAP_GRID_Y));

          swprintf(sString, ARR_SIZE(sString), L"%d", uiItemCnt);

          FindFontCenterCoordinates(sXCorner, sYCorner, MAP_GRID_X, MAP_GRID_Y, sString, MAP_FONT,
                                    &usXPos, &usYPos);
          //				sXPos -= StringPixLength( sString, MAP_FONT ) / 2;

          gprintfdirty(usXPos, usYPos, sString);
          mprintf(usXPos, usYPos, sString);
        }
      }
    }
  }

  // restore clip blits
  RestoreClipRegionToFullScreen();
}

void DrawMapBoxIcon(struct VObject *hIconHandle, uint16_t usVOIndex, uint8_t sMapX, uint8_t sMapY,
                    uint8_t ubIconPosition) {
  int32_t iRowNumber, iColumnNumber;
  int32_t iX, iY;

  // don't show any more icons than will fit into one sector, to keep them from spilling into
  // sector(s) beneath
  if (ubIconPosition >= (MERC_ICONS_PER_LINE * ROWS_PER_SECTOR)) {
    return;
  }

  iColumnNumber = ubIconPosition % MERC_ICONS_PER_LINE;
  iRowNumber = ubIconPosition / MERC_ICONS_PER_LINE;

  if (!fZoomFlag) {
    iX = MAP_VIEW_START_X + (sMapX * MAP_GRID_X) + MAP_X_ICON_OFFSET + (3 * iColumnNumber);
    iY = MAP_VIEW_START_Y + (sMapY * MAP_GRID_Y) + MAP_Y_ICON_OFFSET + (3 * iRowNumber);

    BltVideoObject(guiSAVEBUFFER, hIconHandle, usVOIndex, iX, iY, VO_BLT_SRCTRANSPARENCY, NULL);
    InvalidateRegion(iX, iY, iX + DMAP_GRID_X, iY + DMAP_GRID_Y);
  }
  /*
          else
          {
                  INT sX, sY;

                  GetScreenXYFromMapXYStationary( ( uint16_t ) sX,( uint16_t ) sY, &sX, &sY );
                  iY = sY-MAP_GRID_Y;
                  iX = sX-MAP_GRID_X;

                  // clip blits to mapscreen region
                  ClipBlitsToMapViewRegion( );

                  BltVideoObject(guiSAVEBUFFER,
     hIconHandle,BIG_YELLOW_BOX,MAP_X_ICON_OFFSET+iX+6*iColumnNumber+2,MAP_Y_ICON_OFFSET+iY+6*iRowNumber,
     VO_BLT_SRCTRANSPARENCY, NULL );

                  // restore clip blits
                  RestoreClipRegionToFullScreen( );

                  InvalidateRegion(MAP_X_ICON_OFFSET+iX+6*iColumnNumber+2,
     MAP_Y_ICON_OFFSET+iY+6*iRowNumber,MAP_X_ICON_OFFSET+iX+6*iColumnNumber+2+ DMAP_GRID_ZOOM_X,
     MAP_Y_ICON_OFFSET+iY+6*iRowNumber + DMAP_GRID_ZOOM_Y );
          }
  */
}

void DrawOrta() {
  uint32_t uiDestPitchBYTES;
  int16_t sX, sY;
  uint8_t ubVidObjIndex;
  struct VObject *hHandle;

  if (fZoomFlag) {
    LockVideoSurface(guiSAVEBUFFER, &uiDestPitchBYTES);
    SetClippingRegionAndImageWidth(uiDestPitchBYTES, MAP_VIEW_START_X + MAP_GRID_X - 1,
                                   MAP_VIEW_START_Y + MAP_GRID_Y - 1, MAP_VIEW_WIDTH + 1,
                                   MAP_VIEW_HEIGHT - 9);
    UnLockVideoSurface(guiSAVEBUFFER);

    GetScreenXYFromMapXYStationary(ORTA_SECTOR_X, ORTA_SECTOR_Y, &sX, &sY);
    sX += -MAP_GRID_X + 2;
    sY += -MAP_GRID_Y - 6;
    ubVidObjIndex = 0;
  } else {
    GetScreenXYFromMapXY(ORTA_SECTOR_X, ORTA_SECTOR_Y, &sX, &sY);
    sX += +2;
    sY += -3;
    ubVidObjIndex = 1;
  }

  // draw Orta in its sector
  GetVideoObject(&hHandle, guiORTAICON);
  BltVideoObject(guiSAVEBUFFER, hHandle, ubVidObjIndex, sX, sY, VO_BLT_SRCTRANSPARENCY, NULL);
}

void DrawTixa() {
  uint32_t uiDestPitchBYTES;
  int16_t sX, sY;
  uint8_t ubVidObjIndex;
  struct VObject *hHandle;

  if (fZoomFlag) {
    LockVideoSurface(guiSAVEBUFFER, &uiDestPitchBYTES);
    SetClippingRegionAndImageWidth(uiDestPitchBYTES, MAP_VIEW_START_X + MAP_GRID_X - 1,
                                   MAP_VIEW_START_Y + MAP_GRID_Y - 1, MAP_VIEW_WIDTH + 1,
                                   MAP_VIEW_HEIGHT - 9);
    UnLockVideoSurface(guiSAVEBUFFER);

    GetScreenXYFromMapXYStationary(TIXA_SECTOR_X, TIXA_SECTOR_Y, &sX, &sY);
    sX += -MAP_GRID_X + 3;
    sY += -MAP_GRID_Y + 6;
    ubVidObjIndex = 0;
  } else {
    GetScreenXYFromMapXY(TIXA_SECTOR_X, TIXA_SECTOR_Y, &sX, &sY);
    sY += +2;
    ubVidObjIndex = 1;
  }

  // draw Tixa in its sector
  GetVideoObject(&hHandle, guiTIXAICON);
  BltVideoObject(guiSAVEBUFFER, hHandle, ubVidObjIndex, sX, sY, VO_BLT_SRCTRANSPARENCY, NULL);
}

void DrawBullseye() {
  int16_t sX, sY;
  struct VObject *hHandle;

  GetScreenXYFromMapXY(gsMercArriveSectorX, gsMercArriveSectorY, &sX, &sY);
  sY -= 2;

  // draw the bullseye in that sector
  GetVideoObject(&hHandle, guiBULLSEYE);
  BltVideoObject(guiSAVEBUFFER, hHandle, 0, sX, sY, VO_BLT_SRCTRANSPARENCY, NULL);
}

void HideExistenceOfUndergroundMapSector(uint8_t ubSectorX, uint8_t ubSectorY) {
  int16_t sScreenX;
  int16_t sScreenY;

  GetScreenXYFromMapXY(ubSectorX, ubSectorY, &sScreenX, &sScreenY);

  // fill it with near black
  ColorFillVideoSurfaceArea(guiSAVEBUFFER, sScreenX + 1, sScreenY, sScreenX + MAP_GRID_X,
                            sScreenY + MAP_GRID_Y - 1, gusUndergroundNearBlack);
}

void InitMapSecrets(void) {
  uint8_t ubSamIndex;

  fFoundTixa = FALSE;
  fFoundOrta = FALSE;

  for (ubSamIndex = 0; ubSamIndex < GetSamSiteCount(); ubSamIndex++) {
    SetSamSiteFound(ubSamIndex, false);
  }
}

BOOLEAN CanRedistributeMilitiaInSector(int16_t sClickedSectorX, int16_t sClickedSectorY,
                                       int8_t bClickedTownId) {
  int32_t iCounter = 0;
  uint8_t sBaseSectorValue = 0, sCurrentSectorValue = 0;
  uint8_t sSectorX = 0, sSectorY = 0;

  // if no world is loaded, we can't be in combat (PBI/Auto-resolve locks out normal mapscreen
  // interface for this)
  if (!gfWorldLoaded) {
    // ok to redistribute
    return (TRUE);
  }

  // if tactically not in combat, hostile sector, or air-raid
  if (!(gTacticalStatus.uiFlags & INCOMBAT) && !(gTacticalStatus.fEnemyInSector) && !InAirRaid()) {
    // ok to redistribute
    return (TRUE);
  }

  // if the fight is underground
  if (gbWorldSectorZ != 0) {
    // ok to redistribute
    return (TRUE);
  }

  // currently loaded surface sector IS hostile - so we must check if it's also one of the sectors
  // in this "militia map"

  // get the sector value for the upper left corner
  sBaseSectorValue = sBaseSectorList[bClickedTownId - 1];

  // render icons for map
  for (iCounter = 0; iCounter < 9; iCounter++) {
    // grab current sector value
    sCurrentSectorValue =
        sBaseSectorValue + ((iCounter % MILITIA_BOX_ROWS) + (iCounter / MILITIA_BOX_ROWS) * (16));

    sSectorX = SectorID8_X(sCurrentSectorValue);
    sSectorY = SectorID8_Y(sCurrentSectorValue);

    // not in the same town?
    if (StrategicMap[GetSectorID16(sSectorX, sSectorY)].townID != bClickedTownId) {
      continue;
    }

    // if this is the loaded sector that is currently hostile
    if ((sSectorX == gWorldSectorX) && (sSectorY == gWorldSectorY)) {
      // the fight is within this town!  Can't redistribute.
      return (FALSE);
    }
  }

  // the fight is elsewhere - ok to redistribute
  return (TRUE);
}
