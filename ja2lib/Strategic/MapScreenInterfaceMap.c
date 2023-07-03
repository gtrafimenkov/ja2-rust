#include "Strategic/MapScreenInterfaceMap.h"

#include "Laptop/Finances.h"
#include "Point.h"
#include "SGP/ButtonSystem.h"
#include "SGP/Debug.h"
#include "SGP/English.h"
#include "SGP/Font.h"
#include "SGP/Line.h"
#include "SGP/VObject.h"
#include "SGP/VObjectBlitters.h"
#include "SGP/VObjectInternal.h"
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
#include "Town.h"
#include "Utils/FontControl.h"
#include "Utils/Message.h"
#include "Utils/Text.h"
#include "Utils/Utilities.h"
#include "rust_images.h"
#include "rust_sam_sites.h"
#include "rust_towns.h"

// zoom x and y coords for map scrolling
INT32 iZoomX = 0;
INT32 iZoomY = 0;

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
UINT32 guiBIGMAP;

// orta .sti icon
UINT32 guiORTAICON;
UINT32 guiTIXAICON;

// boxes for characters on the map
UINT32 guiCHARICONS;

// the merc arrival sector landing zone icon
UINT32 guiBULLSEYE;

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
INT32 giMapMilitiaButtonImage[5];
INT32 giMapMilitiaButton[5] = {-1, -1, -1, -1, -1};

INT16 gsMilitiaSectorButtonColors[] = {
    FONT_LTGREEN,
    FONT_LTBLUE,
    16,
};

// track number of townspeople picked up
INT16 sGreensOnCursor = 0;
INT16 sRegularsOnCursor = 0;
INT16 sElitesOnCursor = 0;

// the current militia town id
INT16 sSelectedMilitiaTown = 0;

extern MINE_LOCATION_TYPE gMineLocation[];
extern MINE_STATUS_TYPE gMineStatus[];

// sublevel graphics
UINT32 guiSubLevel1, guiSubLevel2, guiSubLevel3;

// the between sector icons
UINT32 guiCHARBETWEENSECTORICONS;
UINT32 guiCHARBETWEENSECTORICONSCLOSE;

// tixa found
BOOLEAN fFoundTixa = FALSE;

// selected sector
u8 sSelMapX = 9;
u8 sSelMapY = 1;

// highlighted sector
u8 gsHighlightSectorX = 0xff;
u8 gsHighlightSectorY = 0xff;

// the current sector Z value of the map being displayed
INT32 iCurrentMapSectorZ = 0;

// the palettes
UINT16 *pMapLTRedPalette;
UINT16 *pMapDKRedPalette;
UINT16 *pMapLTGreenPalette;
UINT16 *pMapDKGreenPalette;

// the map border eta pop up
UINT32 guiMapBorderEtaPopUp;

// heli pop up
UINT32 guiMapBorderHeliSectors;

// list of map sectors that player isn't allowed to even highlight
BOOLEAN sBadSectorsList[MAP_WORLD_Y][MAP_WORLD_X];

u8 sBaseSectorList[] = {
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
UINT32 guiMAPCURSORS;

// destination plotting character
INT8 bSelectedDestChar = -1;

// assignment selection character
INT8 bSelectedAssignChar = -1;

// current contract char
INT8 bSelectedContractChar = -1;

// has the temp path for character or helicopter been already drawn?
BOOLEAN fTempPathAlreadyDrawn = FALSE;

// the regions for the mapscreen militia box
struct MOUSE_REGION gMapScreenMilitiaBoxRegions[9];
struct MOUSE_REGION gMapScreenMilitiaRegion;

// the mine icon
UINT32 guiMINEICON;

// militia graphics
UINT32 guiMilitia;
UINT32 guiMilitiaMaps;
UINT32 guiMilitiaSectorHighLight;
UINT32 guiMilitiaSectorOutline;

// the sector that is highlighted on the militia map
INT16 sSectorMilitiaMapSector = -1;
BOOLEAN fMilitiaMapButtonsCreated = FALSE;
INT16 sSectorMilitiaMapSectorOutline = -1;

// have any nodes in the current path list been deleted?
BOOLEAN fDeletedNode = FALSE;

UINT16 gusUndergroundNearBlack;

BOOLEAN gfMilitiaPopupCreated = FALSE;

INT32 giAnimateRouteBaseTime = 0;
INT32 giPotHeliPathBaseTime = 0;
INT32 giClickHeliIconBaseTime = 0;

// display the level string on the strategic map
void DisplayLevelString(void);

void ShowTownText(void);
void DrawTownLabels(STR16 pString, STR16 pStringA, UINT16 usFirstX, UINT16 usFirstY);
void ShowTeamAndVehicles(INT32 fShowFlags);
BOOLEAN ShadeMapElem(u8 sMapX, u8 sMapY, INT32 iColor);
BOOLEAN ShadeMapElemZoomIn(u8 sMapX, u8 sMapY, INT32 iColor);
void AdjustXForLeftMapEdge(STR16 wString, INT16 *psX);
static void BlitTownGridMarkers(void);
void BlitMineGridMarkers(void);
void BlitSAMGridMarkers(void);
void BlitMineIcon(u8 sMapX, u8 sMapY);
void BlitMineText(u8 sMapX, u8 sMapY);
u8 GetBaseSectorForCurrentTown(void);
void RenderIconsPerSectorForSelectedTown(void);
void MilitiaRegionClickCallback(struct MOUSE_REGION *pRegion, INT32 iReason);
void MilitiaRegionMoveCallback(struct MOUSE_REGION *pRegion, INT32 iReason);
void CreateDestroyMilitiaSectorButtons(void);
void ShowHighLightedSectorOnMilitiaMap(void);
void SetMilitiaMapButtonsText(void);
void MilitiaButtonCallback(GUI_BUTTON *btn, INT32 reason);
void DisplayUnallocatedMilitia(void);
BOOLEAN IsThisMilitiaTownSectorAllowable(INT16 sSectorIndexValue);
void DrawTownMilitiaName(void);
void HandleShutDownOfMilitiaPanelIfPeopleOnTheCursor(INT16 sTownValue);
void HandleEveningOutOfTroopsAmongstSectors(void);
void CreateMilitiaPanelBottomButton(void);
void DeleteMilitiaPanelBottomButton(void);
void MilitiaDoneButtonCallback(GUI_BUTTON *btn, INT32 reason);
void MilitiaAutoButtonCallback(GUI_BUTTON *btn, INT32 reason);
void RenderShadingForUnControlledSectors(void);
void DrawTownMilitiaForcesOnMap(void);
void CheckAndUpdateStatesOfSelectedMilitiaSectorButtons(void);
void HandleLowerLevelMapBlit(void);
void ShowSAMSitesOnStrategicMap(void);

// UINT8 NumActiveCharactersInSector( u8 sSectorX, u8 sSectorY, INT16 bSectorZ );
// UINT8 NumFriendlyInSector( INT16 sX, INT16 sY, INT8 bZ );

// callbacks
void MilitiaBoxMaskBtnCallback(struct MOUSE_REGION *pRegion, INT32 iReason);

// display potential path, yes or no?
void DisplayThePotentialPathForHelicopter(u8 sMapX, u8 sMapY);
void ShowEnemiesInSector(u8 sSectorX, u8 sSectorY, INT16 sNumberOfEnemies, UINT8 ubIconPosition);
void ShowUncertainNumberEnemiesInSector(u8 sSectorX, u8 sSectorY);
void HandleShowingOfEnemyForcesInSector(u8 sSectorX, u8 sSectorY, INT8 bSectorZ,
                                        UINT8 ubIconPosition);

BOOLEAN CanMilitiaAutoDistribute(void);

void ShowItemsOnMap(void);
void DrawMapBoxIcon(struct VObject *hIconHandle, UINT16 usVOIndex, u8 sMapX, u8 sMapY,
                    UINT8 ubIconPosition);
void DisplayDestinationOfHelicopter(void);
void DrawOrta();
void DrawTixa();
void DrawBullseye();
void HideExistenceOfUndergroundMapSector(UINT8 ubSectorX, UINT8 ubSectorY);

BOOLEAN CanMercsScoutThisSector(u8 sSectorX, u8 sSectorY, INT8 bSectorZ);

BOOLEAN TraceCharAnimatedRoute(struct path *pPath, BOOLEAN fCheckFlag, BOOLEAN fForceUpDate);
void AnimateRoute(struct path *pPath);

extern void EndConfirmMapMoveMode(void);
extern BOOLEAN CanDrawSectorCursor(void);

void DrawMapIndexBigMap(BOOLEAN fSelectedCursorIsYellow) {
  // this procedure will draw the coord indexes on the zoomed out map
  INT16 usX, usY;
  INT32 iCount = 0;
  BOOLEAN fDrawCursors;

  SetFontDest(vsFB, 0, 0, 640, 480, FALSE);
  // SetFontColors(FONT_FCOLOR_GREEN)
  SetFont(MAP_FONT);
  SetFontForeground(MAP_INDEX_COLOR);
  // Dk Red is 163
  SetFontBackground(FONT_MCOLOR_BLACK);

  fDrawCursors = CanDrawSectorCursor();

  for (iCount = 1; iCount <= MAX_VIEW_SECTORS; iCount++) {
    if (fDrawCursors && (iCount == sSelMapX) && (bSelectedDestChar == -1) &&
        (fPlotForHelicopter == FALSE))
      SetFontForeground((UINT8)(fSelectedCursorIsYellow ? FONT_YELLOW : FONT_WHITE));
    else if (fDrawCursors && (iCount == gsHighlightSectorX))
      SetFontForeground(FONT_WHITE);
    else
      SetFontForeground(MAP_INDEX_COLOR);

    FindFontCenterCoordinates(((INT16)(MAP_HORT_INDEX_X + (iCount - 1) * MAP_GRID_X)),
                              MAP_HORT_INDEX_Y, MAP_GRID_X, MAP_HORT_HEIGHT, pMapHortIndex[iCount],
                              MAP_FONT, &usX, &usY);
    mprintf(usX, usY, pMapHortIndex[iCount]);

    if (fDrawCursors && (iCount == sSelMapY) && (bSelectedDestChar == -1) &&
        (fPlotForHelicopter == FALSE))
      SetFontForeground((UINT8)(fSelectedCursorIsYellow ? FONT_YELLOW : FONT_WHITE));
    else if (fDrawCursors && (iCount == gsHighlightSectorY))
      SetFontForeground(FONT_WHITE);
    else
      SetFontForeground(MAP_INDEX_COLOR);

    FindFontCenterCoordinates(
        MAP_VERT_INDEX_X, ((INT16)(MAP_VERT_INDEX_Y + (iCount - 1) * MAP_GRID_Y)), MAP_HORT_HEIGHT,
        MAP_GRID_Y, pMapVertIndex[iCount], MAP_FONT, &usX, &usY);
    mprintf(usX, usY, pMapVertIndex[iCount]);
  }

  InvalidateRegion(MAP_VERT_INDEX_X, MAP_VERT_INDEX_Y, MAP_VERT_INDEX_X + MAP_HORT_HEIGHT,
                   MAP_VERT_INDEX_Y + (iCount - 1) * MAP_GRID_Y);
  InvalidateRegion(MAP_HORT_INDEX_X, MAP_HORT_INDEX_Y, MAP_HORT_INDEX_X + (iCount - 1) * MAP_GRID_X,
                   MAP_HORT_INDEX_Y + MAP_HORT_HEIGHT);

  SetFontDest(vsFB, 0, 0, 640, 480, FALSE);
}

void HandleShowingOfEnemiesWithMilitiaOn(void) {
  u8 sX = 0, sY = 0;

  // if show militia flag is false, leave
  if (!fShowMilitia) {
    return;
  }

  for (sX = 1; sX < (MAP_WORLD_X - 1); sX++) {
    for (sY = 1; sY < (MAP_WORLD_Y - 1); sY++) {
      HandleShowingOfEnemyForcesInSector(sX, sY, (INT8)iCurrentMapSectorZ,
                                         CountMilitiaInSector(sX, sY));
    }
  }

  return;
}

UINT32 DrawMap(void) {
  struct VSurface *hSrcVSurface;
  UINT32 uiDestPitchBYTES;
  UINT32 uiSrcPitchBYTES;
  UINT16 *pDestBuf;
  UINT8 *pSrcBuf;
  SGPRect clip;
  u8 cnt, cnt2;
  INT32 iCounter = 0;

  if (!iCurrentMapSectorZ) {
    // clip blits to mapscreen region
    // ClipBlitsToMapViewRegion( );

    if (fZoomFlag) {
      pDestBuf = (UINT16 *)VSurfaceLockOld(vsSB, &uiDestPitchBYTES);

      if (!(GetVideoSurface(&hSrcVSurface, guiBIGMAP))) {
        return FALSE;
      }
      pSrcBuf = VSurfaceLockOld(GetVSByID(guiBIGMAP), &uiSrcPitchBYTES);

      // set up bounds
      if (iZoomX < WEST_ZOOM_BOUND) iZoomX = WEST_ZOOM_BOUND;
      if (iZoomX > EAST_ZOOM_BOUND) iZoomX = EAST_ZOOM_BOUND;
      if (iZoomY < NORTH_ZOOM_BOUND + 1) iZoomY = NORTH_ZOOM_BOUND;
      if (iZoomY > SOUTH_ZOOM_BOUND) iZoomY = SOUTH_ZOOM_BOUND;

      clip.iLeft = iZoomX - 2;
      clip.iRight = clip.iLeft + MAP_VIEW_WIDTH + 2;
      clip.iTop = iZoomY - 3;
      clip.iBottom = clip.iTop + MAP_VIEW_HEIGHT - 1;

      if (clip.iBottom > GetVSurfaceHeight(hSrcVSurface)) {
        clip.iBottom = GetVSurfaceHeight(hSrcVSurface);
      }

      if (clip.iRight > GetVSurfaceWidth(hSrcVSurface)) {
        clip.iRight = GetVSurfaceWidth(hSrcVSurface);
      }

      Blt8BPPDataSubTo16BPPBuffer(pDestBuf, uiDestPitchBYTES, hSrcVSurface, pSrcBuf,
                                  uiSrcPitchBYTES, MAP_VIEW_START_X + MAP_GRID_X,
                                  MAP_VIEW_START_Y + MAP_GRID_Y - 2, &clip);
      VSurfaceUnlock(GetVSByID(guiBIGMAP));
      VSurfaceUnlock(vsSB);
    } else {
      BlitSurfaceToSurfaceScaleDown2x(GetVSByID(guiBIGMAP), vsSB, MAP_VIEW_START_X + 1,
                                      MAP_VIEW_START_Y);
    }

    // shade map sectors (must be done after Tixa/Orta/Mine icons have been blitted, but before
    // icons!)
    for (cnt = 1; cnt < MAP_WORLD_X - 1; cnt++) {
      for (cnt2 = 1; cnt2 < MAP_WORLD_Y - 1; cnt2++) {
        // LATE DESIGN CHANGE: darken sectors not yet visited, instead of those under known enemy
        // control
        if (GetSectorFlagStatus(cnt, cnt2, (UINT8)iCurrentMapSectorZ, SF_ALREADY_VISITED) ==
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

void GetScreenXYFromMapXY(u8 sMapX, u8 sMapY, INT16 *psX, INT16 *psY) {
  INT16 sXTempOff = 1;
  INT16 sYTempOff = 1;
  if (fZoomFlag) {
    *psX = ((sMapX / 2 + sXTempOff) * MAP_GRID_ZOOM_X) + MAP_VIEW_START_X;
    *psY = ((sMapY / 2 + sYTempOff) * MAP_GRID_ZOOM_Y) + MAP_VIEW_START_Y;
  } else {
    *psX = (sMapX * MAP_GRID_X) + MAP_VIEW_START_X;
    *psY = (sMapY * MAP_GRID_Y) + MAP_VIEW_START_Y;
  }
}

void GetScreenXYFromMapXYStationary(u8 sMapX, u8 sMapY, INT16 *psX, INT16 *psY) {
  INT16 sXTempOff = 1;
  INT16 sYTempOff = 1;
  //(MAP_VIEW_START_X+((iCount+1)*MAP_GRID_X)*2-iZoomX));
  *psX = ((sMapX + sXTempOff) * MAP_GRID_X) * 2 - ((INT16)iZoomX) + MAP_VIEW_START_X;
  *psY = ((sMapY + sYTempOff) * MAP_GRID_Y) * 2 - ((INT16)iZoomY) + MAP_VIEW_START_Y;
}

void ShowTownText(void) {
  CHAR16 sString[32];
  CHAR16 sStringA[32];
  INT8 bTown = 0;
  UINT16 usX, usY;
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
      if (IsTownLoyaltyStarted(bTown) && DoesTownUseLoyalty(bTown)) {
        swprintf(sStringA, ARR_SIZE(sStringA), L"%d%%%% %s", GetTownLoyaltyRating(bTown),
                 gsLoyalString[0]);

        // if loyalty is too low to train militia, and militia training is allowed here
        if ((GetTownLoyaltyRating(bTown) < MIN_RATING_TO_TRAIN_TOWN) &&
            MilitiaTrainingAllowedInTown(bTown)) {
          fLoyaltyTooLowToTrainMilitia = TRUE;
        }
      } else {
        wcscpy(sStringA, L"");
      }

      if (!fZoomFlag) {
        usX = (UINT16)(MAP_VIEW_START_X + MAP_GRID_X + (pTownPoints[bTown].x * MAP_GRID_X) / 10);
        usY = (UINT16)(MAP_VIEW_START_Y + MAP_GRID_Y + ((pTownPoints[bTown].y * MAP_GRID_Y) / 10) +
                       1);
      } else {
        usX = (UINT16)(MAP_VIEW_START_X + MAP_GRID_X + MAP_GRID_ZOOM_X - iZoomX +
                       (pTownPoints[bTown].x * MAP_GRID_ZOOM_X) / 10);
        usY = (UINT16)(MAP_VIEW_START_Y + MAP_GRID_Y + MAP_GRID_ZOOM_Y - iZoomY +
                       ((pTownPoints[bTown].y * MAP_GRID_ZOOM_Y) / 10) + 1);
        //			usX = 2 * pTownPoints[ bTown  ].x - iZoomX - MAP_VIEW_START_X +
        // MAP_GRID_X; 			usY = 2 * pTownPoints[ bTown  ].y - iZoomY -
        // MAP_VIEW_START_Y
        // + MAP_GRID_Y;
      }

      // red for low loyalty, green otherwise
      SetFontForeground(
          (UINT8)(fLoyaltyTooLowToTrainMilitia ? FONT_MCOLOR_RED : FONT_MCOLOR_LTGREEN));

      DrawTownLabels(sString, sStringA, usX, usY);
    }
  }
}

void DrawTownLabels(STR16 pString, STR16 pStringA, UINT16 usFirstX, UINT16 usFirstY) {
  // this procedure will draw the given strings on the screen centered around the given x and at the
  // given y

  INT16 sSecondX, sSecondY;
  INT16 sPastEdge;

  // if within view region...render, else don't
  if ((usFirstX > MAP_VIEW_START_X + MAP_VIEW_WIDTH) || (usFirstX < MAP_VIEW_START_X) ||
      (usFirstY < MAP_VIEW_START_Y) || (usFirstY > MAP_VIEW_START_Y + MAP_VIEW_HEIGHT)) {
    return;
  }

  SetFontDest(vsSB, MapScreenRect.iLeft + 2, MapScreenRect.iTop, MapScreenRect.iRight,
              MapScreenRect.iBottom, FALSE);

  // clip blits to mapscreen region
  ClipBlitsToMapViewRegion();

  // we're CENTERING the first string AROUND usFirstX, so calculate the starting X
  usFirstX -= StringPixLength(pString, MAP_FONT) / 2;

  // print first string
  gprintfdirty(usFirstX, usFirstY, pString);
  mprintf(usFirstX, usFirstY, pString);

  // calculate starting coordinates for the second string
  VarFindFontCenterCoordinates((INT16)(usFirstX), (INT16)usFirstY,
                               StringPixLength(pString, MAP_FONT), 0, MAP_FONT, &sSecondX,
                               &sSecondY, pStringA);

  // make sure we don't go past left edge (Grumm)
  if (!fZoomFlag) {
    sPastEdge = (MAP_VIEW_START_X + 23) - sSecondX;

    if (sPastEdge > 0) sSecondX += sPastEdge;
  }

  // print second string beneath first
  sSecondY = (INT16)(usFirstY + GetFontHeight(MAP_FONT));
  gprintfdirty(sSecondX, sSecondY, pStringA);
  mprintf(sSecondX, sSecondY, pStringA);

  // restore clip blits
  RestoreClipRegionToFullScreen();
}

// "on duty" includes mercs inside vehicles
INT32 ShowOnDutyTeam(u8 sMapX, u8 sMapY) {
  UINT8 ubCounter = 0, ubIconPosition = 0;
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

INT32 ShowAssignedTeam(u8 sMapX, u8 sMapY, INT32 iCount) {
  UINT8 ubCounter, ubIconPosition;
  struct VObject *hIconHandle;
  struct SOLDIERTYPE *pSoldier = NULL;

  GetVideoObject(&hIconHandle, guiCHARICONS);
  ubCounter = 0;

  // run through list
  ubIconPosition = (UINT8)iCount;

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

INT32 ShowVehicles(u8 sMapX, u8 sMapY, INT32 iCount) {
  UINT8 ubCounter, ubIconPosition;
  struct VObject *hIconHandle;
  struct SOLDIERTYPE *pVehicleSoldier;

  GetVideoObject(&hIconHandle, guiCHARICONS);
  ubCounter = 0;

  ubIconPosition = (UINT8)iCount;

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

void ShowEnemiesInSector(u8 sSectorX, u8 sSectorY, INT16 sNumberOfEnemies, UINT8 ubIconPosition) {
  struct VObject *hIconHandle;
  UINT8 ubEnemy = 0;

  // get the video object
  GetVideoObject(&hIconHandle, guiCHARICONS);

  for (ubEnemy = 0; ubEnemy < sNumberOfEnemies; ubEnemy++) {
    DrawMapBoxIcon(hIconHandle, SMALL_RED_BOX, sSectorX, sSectorY, ubIconPosition);
    ubIconPosition++;
  }
}

void ShowUncertainNumberEnemiesInSector(u8 sSectorX, u8 sSectorY) {
  INT16 sXPosition = 0, sYPosition = 0;
  struct VObject *hIconHandle;

  // grab the x and y postions
  sXPosition = sSectorX;
  sYPosition = sSectorY;

  // get the video object
  GetVideoObject(&hIconHandle, guiCHARICONS);

  // check if we are zoomed in...need to offset in case for scrolling purposes
  if (!fZoomFlag) {
    sXPosition = (INT16)(MAP_X_ICON_OFFSET + (MAP_VIEW_START_X + (sSectorX * MAP_GRID_X + 1)) - 1);
    sYPosition = (INT16)(((MAP_VIEW_START_Y + (sSectorY * MAP_GRID_Y) + 1)));
    sYPosition -= 2;

    // small question mark
    BltVObject(vsSB, hIconHandle, SMALL_QUESTION_MARK, sXPosition, sYPosition);
    InvalidateRegion(sXPosition, sYPosition, sXPosition + DMAP_GRID_X, sYPosition + DMAP_GRID_Y);
  }
}

void ShowTeamAndVehicles(INT32 fShowFlags) {
  // go through each sector, display the on duty, assigned, and vehicles
  u8 sMapX = 0;
  u8 sMapY = 0;
  INT32 iIconOffset = 0;
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
        HandleShowingOfEnemyForcesInSector(sMapX, sMapY, (INT8)iCurrentMapSectorZ,
                                           (UINT8)iIconOffset);
        ShowPeopleInMotion(sMapX, sMapY);
      }
    }
  }
}

BOOLEAN ShadeMapElem(u8 sMapX, u8 sMapY, INT32 iColor) {
  INT16 sScreenX, sScreenY;
  struct VSurface *hSrcVSurface;
  // struct VSurface* hSAMSurface;
  // struct VSurface* hMineSurface;
  UINT32 uiDestPitchBYTES;
  UINT32 uiSrcPitchBYTES;
  UINT16 *pDestBuf;
  UINT8 *pSrcBuf;
  SGPRect clip;
  UINT16 *pOriginalPallette;

  // get original video surface palette
  if (!(GetVideoSurface(&hSrcVSurface, guiBIGMAP))) {
    return FALSE;
  }
  // get original video surface palette
  // if (!( GetVideoSurface( &hSAMSurface, guiSAMICON ) )) { return FALSE; }
  // get original video surface palette
  // if (!( GetVideoSurface( &hMineSurface, guiMINEICON ) )) { return FALSE; }
  // get original video surface palette

  pOriginalPallette = GetVSurface16BPPPalette(hSrcVSurface);

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
        ShadowVideoSurfaceRect(vsSB, sScreenX, sScreenY, sScreenX + MAP_GRID_X - 1,
                               sScreenY + MAP_GRID_Y - 1);
        break;

      case (MAP_SHADE_LT_GREEN):
        // grab video surface and set palette
        if (!(GetVideoSurface(&hSrcVSurface, guiBIGMAP))) {
          return FALSE;
        }

        SetVSurface16BPPPalette(hSrcVSurface, pMapLTGreenPalette);
        // hMineSurface->p16BPPPalette = pMapLTGreenPalette;
        // hSAMSurface->p16BPPPalette = pMapLTGreenPalette;

        // lock source and dest buffers
        pDestBuf = (UINT16 *)VSurfaceLockOld(vsSB, &uiDestPitchBYTES);
        if (!(GetVideoSurface(&hSrcVSurface, guiBIGMAP))) {
          return FALSE;
        }
        pSrcBuf = VSurfaceLockOld(GetVSByID(guiBIGMAP), &uiSrcPitchBYTES);

        Blt8BPPDataTo16BPPBufferHalfRect(pDestBuf, uiDestPitchBYTES, hSrcVSurface, pSrcBuf,
                                         uiSrcPitchBYTES, sScreenX, sScreenY, &clip);

        // now blit
        // Blt8BPPDataSubTo16BPPBuffer( pDestBuf, uiDestPitchBYTES, hSrcVSurface,
        // pSrcBuf,uiSrcPitchBYTES, sScreenX, sScreenY, &clip);

        // unlock source and dest buffers
        VSurfaceUnlock(GetVSByID(guiBIGMAP));
        VSurfaceUnlock(vsSB);
        break;

      case (MAP_SHADE_DK_GREEN):
        // grab video surface and set palette
        if (!(GetVideoSurface(&hSrcVSurface, guiBIGMAP))) {
          return FALSE;
        }
        SetVSurface16BPPPalette(hSrcVSurface, pMapDKGreenPalette);
        // hMineSurface->p16BPPPalette = pMapDKGreenPalette;
        // hSAMSurface->p16BPPPalette = pMapDKGreenPalette;

        /// lock source and dest buffers
        pDestBuf = (UINT16 *)VSurfaceLockOld(vsSB, &uiDestPitchBYTES);
        if (!(GetVideoSurface(&hSrcVSurface, guiBIGMAP))) {
          return FALSE;
        }
        pSrcBuf = VSurfaceLockOld(GetVSByID(guiBIGMAP), &uiSrcPitchBYTES);

        Blt8BPPDataTo16BPPBufferHalfRect(pDestBuf, uiDestPitchBYTES, hSrcVSurface, pSrcBuf,
                                         uiSrcPitchBYTES, sScreenX, sScreenY, &clip);

        // now blit
        // Blt8BPPDataSubTo16BPPBuffer( pDestBuf, uiDestPitchBYTES, hSrcVSurface,
        // pSrcBuf,uiSrcPitchBYTES, sScreenX , sScreenY , &clip);

        // unlock source and dest buffers
        VSurfaceUnlock(GetVSByID(guiBIGMAP));
        VSurfaceUnlock(vsSB);
        break;

      case (MAP_SHADE_LT_RED):
        // grab video surface and set palette
        if (!(GetVideoSurface(&hSrcVSurface, guiBIGMAP))) {
          return FALSE;
        }
        SetVSurface16BPPPalette(hSrcVSurface, pMapLTRedPalette);
        // hMineSurface->p16BPPPalette = pMapLTRedPalette;
        // hSAMSurface->p16BPPPalette = pMapLTRedPalette;

        // lock source and dest buffers
        pDestBuf = (UINT16 *)VSurfaceLockOld(vsSB, &uiDestPitchBYTES);
        if (!(GetVideoSurface(&hSrcVSurface, guiBIGMAP))) {
          return FALSE;
        }
        pSrcBuf = VSurfaceLockOld(GetVSByID(guiBIGMAP), &uiSrcPitchBYTES);

        Blt8BPPDataTo16BPPBufferHalfRect(pDestBuf, uiDestPitchBYTES, hSrcVSurface, pSrcBuf,
                                         uiSrcPitchBYTES, sScreenX, sScreenY, &clip);

        // now blit
        // Blt8BPPDataSubTo16BPPBuffer( pDestBuf, uiDestPitchBYTES, hSrcVSurface,
        // pSrcBuf,uiSrcPitchBYTES, sScreenX , sScreenY , &clip);

        // unlock source and dest buffers
        VSurfaceUnlock(GetVSByID(guiBIGMAP));
        VSurfaceUnlock(vsSB);
        break;

      case (MAP_SHADE_DK_RED):
        // grab video surface and set palette
        if (!(GetVideoSurface(&hSrcVSurface, guiBIGMAP))) {
          return FALSE;
        }
        SetVSurface16BPPPalette(hSrcVSurface, pMapDKRedPalette);
        // hMineSurface->p16BPPPalette = pMapDKRedPalette;
        // hSAMSurface->p16BPPPalette = pMapDKRedPalette;

        // lock source and dest buffers
        pDestBuf = (UINT16 *)VSurfaceLockOld(vsSB, &uiDestPitchBYTES);
        if (!(GetVideoSurface(&hSrcVSurface, guiBIGMAP))) {
          return FALSE;
        }
        pSrcBuf = VSurfaceLockOld(GetVSByID(guiBIGMAP), &uiSrcPitchBYTES);

        Blt8BPPDataTo16BPPBufferHalfRect(pDestBuf, uiDestPitchBYTES, hSrcVSurface, pSrcBuf,
                                         uiSrcPitchBYTES, sScreenX, sScreenY, &clip);

        // now blit
        // Blt8BPPDataSubTo16BPPBuffer( pDestBuf, uiDestPitchBYTES, hSrcVSurface,
        // pSrcBuf,uiSrcPitchBYTES, sScreenX , sScreenY , &clip);

        // unlock source and dest buffers
        VSurfaceUnlock(GetVSByID(guiBIGMAP));
        VSurfaceUnlock(vsSB);
        break;
    }

    // restore original palette
    if (!(GetVideoSurface(&hSrcVSurface, guiBIGMAP))) {
      return FALSE;
    }
    SetVSurface16BPPPalette(hSrcVSurface, pOriginalPallette);
    // hMineSurface->p16BPPPalette = pOriginalPallette;
    // hSAMSurface->p16BPPPalette = pOriginalPallette;
  }

  return (TRUE);
}

BOOLEAN ShadeMapElemZoomIn(u8 sMapX, u8 sMapY, INT32 iColor) {
  INT16 sScreenX, sScreenY;
  INT32 iX, iY;
  struct VSurface *hSrcVSurface;
  UINT32 uiDestPitchBYTES;
  UINT32 uiSrcPitchBYTES;
  UINT16 *pDestBuf;
  // UINT8 *pDestBuf2;
  UINT8 *pSrcBuf;
  SGPRect clip;
  UINT16 *pOriginalPallette;

  // trabslate to screen co-ords for zoomed
  GetScreenXYFromMapXYStationary(sMapX, sMapY, &sScreenX, &sScreenY);

  // shift left by one sector
  iY = (INT32)sScreenY - MAP_GRID_Y;
  iX = (INT32)sScreenX - MAP_GRID_X;

  // get original video surface palette
  if (!(GetVideoSurface(&hSrcVSurface, guiBIGMAP))) {
    return FALSE;
  }
  pOriginalPallette = GetVSurface16BPPPalette(hSrcVSurface);

  if ((iX > MapScreenRect.iLeft - MAP_GRID_X * 2) && (iX < MapScreenRect.iRight) &&
      (iY > MapScreenRect.iTop - MAP_GRID_Y * 2) && (iY < MapScreenRect.iBottom)) {
    sScreenX = (INT16)iX;
    sScreenY = (INT16)iY;

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
          ShadowVideoSurfaceRect(vsSB, clip.iLeft, clip.iTop, clip.iRight, clip.iBottom);
        }
        ShadowVideoSurfaceRect(vsSB, clip.iLeft, clip.iTop, clip.iRight, clip.iBottom);
        break;

      case (MAP_SHADE_LT_GREEN):
        // grab video surface and set palette
        if (!(GetVideoSurface(&hSrcVSurface, guiBIGMAP))) {
          return FALSE;
        }
        SetVSurface16BPPPalette(hSrcVSurface, pMapLTGreenPalette);

        // lock source and dest buffers
        pDestBuf = (UINT16 *)VSurfaceLockOld(vsSB, &uiDestPitchBYTES);
        if (!(GetVideoSurface(&hSrcVSurface, guiBIGMAP))) {
          return FALSE;
        }
        pSrcBuf = VSurfaceLockOld(GetVSByID(guiBIGMAP), &uiSrcPitchBYTES);

        // now blit
        Blt8BPPDataSubTo16BPPBuffer(pDestBuf, uiDestPitchBYTES, hSrcVSurface, pSrcBuf,
                                    uiSrcPitchBYTES, sScreenX, sScreenY, &clip);

        // unlock source and dest buffers
        VSurfaceUnlock(GetVSByID(guiBIGMAP));
        VSurfaceUnlock(vsSB);

        break;

      case (MAP_SHADE_DK_GREEN):
        // grab video surface and set palette
        if (!(GetVideoSurface(&hSrcVSurface, guiBIGMAP))) {
          return FALSE;
        }
        SetVSurface16BPPPalette(hSrcVSurface, pMapDKGreenPalette);

        /// lock source and dest buffers
        pDestBuf = (UINT16 *)VSurfaceLockOld(vsSB, &uiDestPitchBYTES);
        if (!(GetVideoSurface(&hSrcVSurface, guiBIGMAP))) {
          return FALSE;
        }
        pSrcBuf = VSurfaceLockOld(GetVSByID(guiBIGMAP), &uiSrcPitchBYTES);

        // now blit
        Blt8BPPDataSubTo16BPPBuffer(pDestBuf, uiDestPitchBYTES, hSrcVSurface, pSrcBuf,
                                    uiSrcPitchBYTES, sScreenX, sScreenY, &clip);

        // unlock source and dest buffers
        VSurfaceUnlock(GetVSByID(guiBIGMAP));
        VSurfaceUnlock(vsSB);

        break;

      case (MAP_SHADE_LT_RED):
        // grab video surface and set palette
        if (!(GetVideoSurface(&hSrcVSurface, guiBIGMAP))) {
          return FALSE;
        }
        SetVSurface16BPPPalette(hSrcVSurface, pMapLTRedPalette);

        // lock source and dest buffers
        pDestBuf = (UINT16 *)VSurfaceLockOld(vsSB, &uiDestPitchBYTES);
        if (!(GetVideoSurface(&hSrcVSurface, guiBIGMAP))) {
          return FALSE;
        }
        pSrcBuf = VSurfaceLockOld(GetVSByID(guiBIGMAP), &uiSrcPitchBYTES);

        // now blit
        Blt8BPPDataSubTo16BPPBuffer(pDestBuf, uiDestPitchBYTES, hSrcVSurface, pSrcBuf,
                                    uiSrcPitchBYTES, sScreenX, sScreenY, &clip);

        // unlock source and dest buffers
        VSurfaceUnlock(GetVSByID(guiBIGMAP));
        VSurfaceUnlock(vsSB);

        break;

      case (MAP_SHADE_DK_RED):
        // grab video surface and set palette
        if (!(GetVideoSurface(&hSrcVSurface, guiBIGMAP))) {
          return FALSE;
        }
        SetVSurface16BPPPalette(hSrcVSurface, pMapDKRedPalette);

        // lock source and dest buffers
        pDestBuf = (UINT16 *)VSurfaceLockOld(vsSB, &uiDestPitchBYTES);
        if (!(GetVideoSurface(&hSrcVSurface, guiBIGMAP))) {
          return FALSE;
        }
        pSrcBuf = VSurfaceLockOld(GetVSByID(guiBIGMAP), &uiSrcPitchBYTES);

        // now blit
        Blt8BPPDataSubTo16BPPBuffer(pDestBuf, uiDestPitchBYTES, hSrcVSurface, pSrcBuf,
                                    uiSrcPitchBYTES, sScreenX, sScreenY, &clip);

        // unlock source and dest buffers
        VSurfaceUnlock(GetVSByID(guiBIGMAP));
        VSurfaceUnlock(vsSB);

        break;
    }
  }

  // restore original palette
  if (!(GetVideoSurface(&hSrcVSurface, guiBIGMAP))) {
    return FALSE;
  }
  SetVSurface16BPPPalette(hSrcVSurface, pOriginalPallette);

  return (TRUE);
}

BOOLEAN InitializePalettesForMap(void) {
  // init palettes
  struct VSurface *hSrcVSurface;
  struct SGPPaletteEntry pPalette[256];
  UINT32 uiTempMap;

  // load image
  if (!(AddVideoSurfaceFromFile("INTERFACE\\b_map.pcx", &uiTempMap))) {
    return FALSE;
  }

  // get video surface
  if (!(GetVideoSurface(&hSrcVSurface, uiTempMap))) {
    return FALSE;
  }
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

void PlotPathForCharacter(struct SOLDIERTYPE *pCharacter, u8 sX, u8 sY,
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
          NULL, GetLastSectorIdInCharactersPath(pCharacter), (INT16)(sX + sY * (MAP_WORLD_X)),
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

void PlotATemporaryPathForCharacter(struct SOLDIERTYPE *pCharacter, u8 sX, u8 sY) {
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
      (INT16)(sX + sY * (MAP_WORLD_X)), GetSoldierGroupId(pCharacter), FALSE /*, TRUE */);

  return;
}

// clear out character path list, after and including this sector
UINT32 ClearPathAfterThisSectorForCharacter(struct SOLDIERTYPE *pCharacter, u8 sX, u8 sY) {
  INT32 iOrigLength = 0;
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
  //	GroupReversingDirectionsBetweenSectors( GetGroup( pCharacter->ubGroupID ), ( UINT8 )(
  // GetSolSectorX(pCharacter) ), ( UINT8 )( GetSolSectorY(pCharacter) ), FALSE );

  // if he's in a vehicle, clear out the vehicle, too
  if (pCharacter->bAssignment == VEHICLE) {
    CancelPathForVehicle(&(pVehicleList[pCharacter->iVehicleId]), TRUE);
  } else {
    // display "travel route canceled" message
    MapScreenMessage(FONT_MCOLOR_LTYELLOW, MSG_MAP_UI_POSITION_MIDDLE, pMapPlotStrings[3]);
  }

  CopyPathToCharactersSquadIfInOne(pCharacter);

  SetMapPanelDirty(true);
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
    // UINT8 ) ( pVehicle->sSectorX ), ( UINT8 ) ( pVehicle->sSectorY ), FALSE );
  }

  // display "travel route canceled" message
  MapScreenMessage(FONT_MCOLOR_LTYELLOW, MSG_MAP_UI_POSITION_MIDDLE, pMapPlotStrings[3]);

  // turn the helicopter flag off here, this prevents the "route aborted" msg from coming up
  fPlotForHelicopter = FALSE;

  fTeamPanelDirty = TRUE;
  SetMapPanelDirty(true);
  fCharacterInfoPanelDirty = TRUE;  // to update ETA
}

void CancelPathForGroup(struct GROUP *pGroup) {
  INT32 iVehicleId;

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
  INT8 bSquad = 0;

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

void PlotPathForHelicopter(u8 sX, u8 sY) {
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
          NULL, GetLastSectorOfHelicoptersPath(), (INT16)(sX + sY * (MAP_WORLD_X)),
          pVehicleList[iHelicopterVehicleId].ubMovementGroup, FALSE /*, FALSE */)),
      pVehicleList[iHelicopterVehicleId].pMercPath);

  // move to beginning of list
  pVehicleList[iHelicopterVehicleId].pMercPath =
      MoveToBeginningOfPathList(pVehicleList[iHelicopterVehicleId].pMercPath);

  SetMapPanelDirty(true);

  return;
}

void PlotATemporaryPathForHelicopter(u8 sX, u8 sY) {
  // clear old temp path
  pTempHelicopterPath = ClearStrategicPathList(pTempHelicopterPath, 0);

  // is cursor allowed here?..if not..don't build temp path
  if (!IsTheCursorAllowedToHighLightThisSector(sX, sY)) {
    return;
  }

  // build path
  pTempHelicopterPath =
      BuildAStrategicPath(NULL, GetLastSectorOfHelicoptersPath(), (INT16)(sX + sY * (MAP_WORLD_X)),
                          pVehicleList[iHelicopterVehicleId].ubMovementGroup, FALSE /*, TRUE */);

  return;
}

// clear out helicopter path list, after and including this sector
UINT32 ClearPathAfterThisSectorForHelicopter(u8 sX, u8 sY) {
  VEHICLETYPE *pVehicle = NULL;
  INT32 iOrigLength = 0;

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

INT16 GetLastSectorOfHelicoptersPath(void) {
  // will return the last sector of the helicopter's current path
  INT16 sLastSector = pVehicleList[iHelicopterVehicleId].sSectorX +
                      pVehicleList[iHelicopterVehicleId].sSectorY * MAP_WORLD_X;
  struct path *pNode = NULL;

  pNode = pVehicleList[iHelicopterVehicleId].pMercPath;

  while (pNode) {
    sLastSector = (INT16)(pNode->uiSectorId);
    pNode = pNode->pNext;
  }

  return sLastSector;
}

BOOLEAN TracePathRoute(BOOLEAN fCheckFlag, BOOLEAN fForceUpDate, struct path *pPath) {
  BOOLEAN fSpeedFlag = FALSE;
  INT32 iArrow = -1;
  INT32 iX, iY;
  INT16 sX, sY;
  INT32 iArrowX, iArrowY;
  INT32 iDeltaA, iDeltaB, iDeltaB1;
  INT32 iDirection = 0;
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
      iDeltaA = (INT16)pNode->uiSectorId - (INT16)pPastNode->uiSectorId;
      iDeltaB = (INT16)pNode->uiSectorId - (INT16)pNextNode->uiSectorId;
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
        GetScreenXYFromMapXYStationary(((UINT16)(SectorID16_X(pNode->uiSectorId))),
                                       ((UINT16)(SectorID16_Y(pNode->uiSectorId))), &sX, &sY);
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
        GetScreenXYFromMapXYStationary(((UINT16)(SectorID16_X(pNode->uiSectorId))),
                                       ((UINT16)(SectorID16_Y(pNode->uiSectorId))), &sX, &sY);
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
        iDeltaA = (INT16)pNode->uiSectorId - (INT16)pPastNode->uiSectorId;
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
        iDeltaB = (INT16)pNode->uiSectorId - (INT16)pNextNode->uiSectorId;
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
        BltVObject(vsFB, hMapHandle, (UINT16)iDirection, iX, iY);

        if (!fUTurnFlag) {
          BltVObject(vsFB, hMapHandle, (UINT16)iArrow, iArrowX, iArrowY);
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
  SetFontDest(vsFB, 0, 0, 640, 480, FALSE);

  // the animated path
  if (TraceCharAnimatedRoute(pPath, FALSE, FALSE)) {
    // ARM? Huh?  Why the same thing twice more?
    TraceCharAnimatedRoute(pPath, FALSE, TRUE);
    TraceCharAnimatedRoute(pPath, FALSE, TRUE);
  }
}

void RestoreArrowBackgroundsForTrace(INT32 iArrow, INT32 iArrowX, INT32 iArrowY, BOOLEAN fZoom) {
  INT16 sArrow = 0;
  INT32 iX = -1, iY = -1;
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
    RestoreExternBackgroundRect(((INT16)iX), ((INT16)iY), DMAP_GRID_X / 2, DMAP_GRID_Y / 2);
  else
    RestoreExternBackgroundRect(((INT16)iX), ((INT16)iY), DMAP_GRID_ZOOM_X, DMAP_GRID_ZOOM_Y);

  return;
}

BOOLEAN TraceCharAnimatedRoute(struct path *pPath, BOOLEAN fCheckFlag, BOOLEAN fForceUpDate) {
  static struct path *pCurrentNode = NULL;
  static BOOLEAN fUpDateFlag = FALSE;
  static BOOLEAN fPauseFlag = TRUE;
  static UINT8 ubCounter = 1;

  struct VObject *hMapHandle;
  BOOLEAN fSpeedFlag = FALSE;
  INT32 iDifference = 0;
  INT32 iArrow = -1;
  INT32 iX = 0, iY = 0;
  INT32 iPastX, iPastY;
  INT16 sX = 0, sY = 0;
  INT32 iArrowX, iArrowY;
  INT32 iDeltaA, iDeltaB, iDeltaB1;
  INT32 iDirection = -1;
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
    iDeltaA = (INT16)pNode->uiSectorId - (INT16)pPastNode->uiSectorId;
    iDeltaB = (INT16)pNode->uiSectorId - (INT16)pNextNode->uiSectorId;
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
      GetScreenXYFromMapXYStationary(((UINT16)(SectorID16_X(pNode->uiSectorId))),
                                     ((UINT16)(SectorID16_Y(pNode->uiSectorId))), &sX, &sY);
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
      iDeltaA = (INT16)pNode->uiSectorId - (INT16)pPastNode->uiSectorId;
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
      iDeltaB = (INT16)pNode->uiSectorId - (INT16)pNextNode->uiSectorId;
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
        // RestoreExternBackgroundRect(((INT16)iArrowX),((INT16)iArrowY),DMAP_GRID_X, DMAP_GRID_Y);
        // else
        // RestoreExternBackgroundRect(((INT16)iArrowX), ((INT16)iArrowY),DMAP_GRID_ZOOM_X,
        // DMAP_GRID_ZOOM_Y);
        if (pNode != pPath) {
          BltVObject(vsFB, hMapHandle, (UINT16)iArrow, iArrowX, iArrowY);
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

void DisplayThePotentialPathForHelicopter(u8 sMapX, u8 sMapY) {
  // simply check if we want to refresh the screen to display path
  static BOOLEAN fOldShowAirCraft = FALSE;
  static INT16 sOldMapX, sOldMapY;
  INT32 iDifference = 0;

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
      SetMapPanelDirty(true);
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

BOOLEAN IsTheCursorAllowedToHighLightThisSector(u8 sSectorX, u8 sSectorY) {
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
  INT8 bY;

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

void RestoreBackgroundForMapGrid(u8 sMapX, u8 sMapY) {
  INT16 sX, sY;

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

void ClipBlitsToMapViewRegionForRectangleAndABit(UINT32 uiDestPitchBYTES) {
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

void RestoreClipRegionToFullScreenForRectangle(UINT32 uiDestPitchBYTES) {
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

void ShowPeopleInMotion(u8 sX, u8 sY) {
  INT32 sExiting = 0;
  INT32 sEntering = 0;
  INT16 sDest = 0;
  INT16 sSource = 0;
  INT16 sOffsetX = 0, sOffsetY = 0;
  INT16 iX = sX, iY = sY;
  INT16 sXPosition = 0, sYPosition = 0;
  INT32 iCounter = 0;
  struct VObject *hIconHandle;
  BOOLEAN fAboutToEnter = FALSE;
  CHAR16 sString[32];
  INT16 sTextXOffset = 0;
  INT16 sTextYOffset = 0;
  INT16 usX, usY;
  INT32 iWidth = 0, iHeight = 0;
  INT32 iDeltaXForError = 0, iDeltaYForError = 0;

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
              (INT16)GetSectorID8(SectorID16_X(sSource), SectorID16_Y(sSource)),
              (INT16)GetSectorID8(SectorID16_X(sDest), SectorID16_Y(sDest)), &sExiting, &sEntering,
              &fAboutToEnter)) {
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

          BltVObject(vsSB, hIconHandle, (UINT16)iCounter, (INT16)iX, (INT16)iY);
        } else {
          GetScreenXYFromMapXYStationary(((u8)(iX)), ((u8)(iY)), &sXPosition, &sYPosition);

          iY = sYPosition - MAP_GRID_Y + sOffsetY;
          iX = sXPosition - MAP_GRID_X + sOffsetX;

          // clip blits to mapscreen region
          ClipBlitsToMapViewRegion();

          BltVObject(vsSB, hIconHandle, (UINT16)iCounter, iX, iY);

          // restore clip blits
          RestoreClipRegionToFullScreen();
        }

        FindFontCenterCoordinates((INT16)(iX + sTextXOffset), 0, ICON_WIDTH, 0, sString, MAP_FONT,
                                  &usX, &usY);
        SetFontDest(vsSB, 0, 0, 640, 480, FALSE);
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
          RestoreExternBackgroundRect(iX, iY, (INT16)iWidth, (INT16)iHeight);
        }
      }
    }
  }

  // restore buffer
  SetFontDest(vsFB, 0, 0, 640, 480, FALSE);
}

void DisplayDistancesForHelicopter(void) {
  // calculate the distance travelled, the proposed distance, and total distance one can go
  // display these on screen
  INT16 sDistanceToGo = 0;  //, sDistanceSoFar = 0, sTotalCanTravel = 0;
  INT16 sX = 0, sY = 0;
  CHAR16 sString[32];
  struct VObject *hHandle;
  INT16 sTotalOfTrip = 0;
  INT32 iTime = 0;
  u8 sMapX, sMapY;
  INT16 sYPosition = 0;
  static INT16 sOldYPosition = 0;
  INT16 sNumSafeSectors;
  INT16 sNumUnSafeSectors;
  UINT32 uiTripCost;

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
  BltVObject(vsFB, hHandle, 0, MAP_HELICOPTER_ETA_POPUP_X, sYPosition);

  //	sTotalCanTravel = ( INT16 )GetTotalDistanceHelicopterCanTravel( );
  sDistanceToGo = (INT16)DistanceOfIntendedHelicopterPath();
  sTotalOfTrip = sDistanceToGo;  // + ( INT16 ) ( DistanceToNearestRefuelPoint( ( INT16 )(
                                 // LastSectorInHelicoptersPath() % MAP_WORLD_X ), ( INT16 ) (
                                 // LastSectorInHelicoptersPath() / MAP_WORLD_X ) ) );

  sNumSafeSectors = GetNumSafeSectorsInPath();
  sNumUnSafeSectors = GetNumUnSafeSectorsInPath();

  //	sDistanceSoFar = ( INT16 )HowFarHelicopterhasTravelledSinceRefueling( );
  //	 sTotalDistanceOfTrip = ( INT16 )DistanceToNearestRefuelPoint( )

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
                           (INT16)(MAP_HELICOPTER_ETA_POPUP_Y + 5 + 2 * GetFontHeight(MAP_FONT)),
                           MAP_HELICOPTER_ETA_POPUP_WIDTH, 0, sString, MAP_FONT, &sX, &sY);
  mprintf(sX, (INT16)(sYPosition + 5 + GetFontHeight(MAP_FONT)), sString);

  swprintf(sString, ARR_SIZE(sString), L"%s", pHelicopterEtaStrings[2]);
  mprintf(MAP_HELICOPTER_ETA_POPUP_X + 5, sYPosition + 5 + 2 * GetFontHeight(MAP_FONT), sString);

  swprintf(sString, ARR_SIZE(sString), L"%d", sNumUnSafeSectors);
  FindFontRightCoordinates(MAP_HELICOPTER_ETA_POPUP_X + 5,
                           (INT16)(MAP_HELICOPTER_ETA_POPUP_Y + 5 + 2 * GetFontHeight(MAP_FONT)),
                           MAP_HELICOPTER_ETA_POPUP_WIDTH, 0, sString, MAP_FONT, &sX, &sY);
  mprintf(sX, (INT16)(sYPosition + 5 + 2 * GetFontHeight(MAP_FONT)), sString);

  swprintf(sString, ARR_SIZE(sString), L"%s", pHelicopterEtaStrings[3]);
  mprintf(MAP_HELICOPTER_ETA_POPUP_X + 5, sYPosition + 5 + 3 * GetFontHeight(MAP_FONT), sString);

  // calculate the cost of the trip based on the number of safe and unsafe sectors it will pass
  // through
  uiTripCost = (sNumSafeSectors * COST_AIRSPACE_SAFE) + (sNumUnSafeSectors * COST_AIRSPACE_UNSAFE);

  swprintf(sString, ARR_SIZE(sString), L"%d", uiTripCost);
  InsertCommasForDollarFigure(sString);
  InsertDollarSignInToString(sString);
  FindFontRightCoordinates(MAP_HELICOPTER_ETA_POPUP_X + 5,
                           (INT16)(MAP_HELICOPTER_ETA_POPUP_Y + 5 + 3 * GetFontHeight(MAP_FONT)),
                           MAP_HELICOPTER_ETA_POPUP_WIDTH, 0, sString, MAP_FONT, &sX, &sY);
  mprintf(sX, (INT16)(sYPosition + 5 + 3 * GetFontHeight(MAP_FONT)), sString);

  swprintf(sString, ARR_SIZE(sString), L"%s", pHelicopterEtaStrings[4]);
  mprintf(MAP_HELICOPTER_ETA_POPUP_X + 5, sYPosition + 5 + 4 * GetFontHeight(MAP_FONT), sString);

  // get travel time for the last path segment
  iTime = GetPathTravelTimeDuringPlotting(pTempHelicopterPath);

  // add travel time for any prior path segments (stored in the helicopter's mercpath, but waypoints
  // aren't built)
  iTime += GetPathTravelTimeDuringPlotting(pVehicleList[iHelicopterVehicleId].pMercPath);

  swprintf(sString, ARR_SIZE(sString), L"%d%s %d%s", iTime / 60, gsTimeStrings[0], iTime % 60,
           gsTimeStrings[1]);
  FindFontRightCoordinates(MAP_HELICOPTER_ETA_POPUP_X + 5,
                           (INT16)(sYPosition + 5 + 4 * GetFontHeight(MAP_FONT)),
                           MAP_HELICOPTER_ETA_POPUP_WIDTH, 0, sString, MAP_FONT, &sX, &sY);
  mprintf(sX, (INT16)(sYPosition + 5 + 4 * GetFontHeight(MAP_FONT)), sString);

  // show # of passengers aboard the chopper
  mprintf(MAP_HELICOPTER_ETA_POPUP_X + 5, sYPosition + 5 + 5 * GetFontHeight(MAP_FONT),
          pHelicopterEtaStrings[6]);
  swprintf(sString, ARR_SIZE(sString), L"%d", GetNumberOfPassengersInHelicopter());
  FindFontRightCoordinates(MAP_HELICOPTER_ETA_POPUP_X + 5,
                           (INT16)(MAP_HELICOPTER_ETA_POPUP_Y + 5 + 5 * GetFontHeight(MAP_FONT)),
                           MAP_HELICOPTER_ETA_POPUP_WIDTH, 0, sString, MAP_FONT, &sX, &sY);
  mprintf(sX, (INT16)(sYPosition + 5 + 5 * GetFontHeight(MAP_FONT)), sString);

  InvalidateRegion(MAP_HELICOPTER_ETA_POPUP_X, sOldYPosition,
                   MAP_HELICOPTER_ETA_POPUP_X + MAP_HELICOPTER_ETA_POPUP_WIDTH + 20,
                   sOldYPosition + MAP_HELICOPTER_ETA_POPUP_HEIGHT);
  return;
}

// grab position of helicopter and blt to screen
void DisplayPositionOfHelicopter(void) {
  static INT16 sOldMapX = 0, sOldMapY = 0;
  //	INT16 sX =0, sY = 0;
  FLOAT flRatio = 0.0;
  UINT32 x, y;
  UINT16 minX, minY, maxX, maxY;
  struct GROUP *pGroup;
  struct VObject *hHandle;
  INT32 iNumberOfPeopleInHelicopter = 0;
  CHAR16 sString[4];

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
        flRatio = ((pGroup->uiTraverseTime + GetGameTimeInMin()) - pGroup->uiArrivalTime) /
                  (float)pGroup->uiTraverseTime;
      }

      /*
                              AssertMsg( ( flRatio >= 0 ) && ( flRatio <= 100 ), String(
         "DisplayPositionOfHelicopter: Invalid flRatio = %6.2f, trav %d, arr %d, time %d", flRatio,
         pGroup->uiTraverseTime, pGroup->uiArrivalTime, GetGameTimeInMin() ) );
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
         ((UINT16)(pGroup->ubSectorX)),((UINT16)(pGroup->ubSectorY)) , &sX, &sY ); sY=sY-MAP_GRID_Y;
                                      sX=sX-MAP_GRID_X;

                                      minX = ( sX );
                                      minY = ( sY );

                                      GetScreenXYFromMapXYStationary(
         ((UINT16)(pGroup->ubNextX)),((UINT16)(pGroup->ubNextY)) , &sX, &sY ); sY=sY-MAP_GRID_Y;
                                      sX=sX-MAP_GRID_X;

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
      x = (UINT32)(minX + flRatio * ((INT16)maxX - (INT16)minX));
      y = (UINT32)(minY + flRatio * ((INT16)maxY - (INT16)minY));

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
      BltVObject(vsFB, hHandle, HELI_ICON, x, y);

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
      sOldMapX = (INT16)x;
      sOldMapY = (INT16)y;
    }
  }

  return;
}

void DisplayDestinationOfHelicopter(void) {
  static INT16 sOldMapX = 0, sOldMapY = 0;
  INT16 sSector;
  u8 sMapX, sMapY;
  UINT32 x, y;
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
    BltVObject(vsFB, hHandle, HELI_SHADOW_ICON, x, y);
    InvalidateRegion(x, y, x + HELI_SHADOW_ICON_WIDTH, y + HELI_SHADOW_ICON_HEIGHT);

    RestoreClipRegionToFullScreen();

    // now store the old stuff
    sOldMapX = (INT16)x;
    sOldMapY = (INT16)y;
  }
}

BOOLEAN CheckForClickOverHelicopterIcon(u8 sClickedSectorX, u8 sClickedSectorY) {
  struct GROUP *pGroup = NULL;
  BOOLEAN fHelicopterOverNextSector = FALSE;
  FLOAT flRatio = 0.0;
  INT16 sSectorX;
  INT16 sSectorY;

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
      flRatio = (pGroup->uiTraverseTime - pGroup->uiArrivalTime + GetGameTimeInMin()) /
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

void BlitMineIcon(u8 sMapX, u8 sMapY) {
  struct VObject *hHandle;
  UINT32 uiDestPitchBYTES;
  INT16 sScreenX, sScreenY;

  GetVideoObject(&hHandle, guiMINEICON);

  VSurfaceLockOld(vsSB, &uiDestPitchBYTES);
  SetClippingRegionAndImageWidth(uiDestPitchBYTES, MAP_VIEW_START_X + MAP_GRID_X - 1,
                                 MAP_VIEW_START_Y + MAP_GRID_Y - 1, MAP_VIEW_WIDTH + 1,
                                 MAP_VIEW_HEIGHT - 9);
  VSurfaceUnlock(vsSB);

  if (fZoomFlag) {
    GetScreenXYFromMapXYStationary((INT16)(sMapX), (INT16)(sMapY), &sScreenX, &sScreenY);
    // when zoomed, the x,y returned is the CENTER of the map square in question
    BltVObject(vsSB, hHandle, 0, sScreenX - MAP_GRID_ZOOM_X / 4, sScreenY - MAP_GRID_ZOOM_Y / 4);
  } else {
    GetScreenXYFromMapXY((INT16)(sMapX), (INT16)(sMapY), &sScreenX, &sScreenY);
    // when not zoomed, the x,y returned is the top left CORNER of the map square in question
    BltVObject(vsSB, hHandle, 1, sScreenX + MAP_GRID_X / 4, sScreenY + MAP_GRID_Y / 4);
  }
}

void BlitMineText(u8 sMapX, u8 sMapY) {
  INT16 sScreenX, sScreenY;
  CHAR16 wString[32], wSubString[32];
  UINT8 ubMineIndex;
  UINT8 ubLineCnt = 0;

  if (fZoomFlag) {
    GetScreenXYFromMapXYStationary((INT16)(sMapX), (INT16)(sMapY), &sScreenX, &sScreenY);

    // set coordinates for start of mine text
    sScreenY += MAP_GRID_ZOOM_Y / 2 + 1;  // slightly below
  } else {
    GetScreenXYFromMapXY((INT16)(sMapX), (INT16)(sMapY), &sScreenX, &sScreenY);

    // set coordinates for start of mine text
    sScreenX += MAP_GRID_X / 2;  // centered around middle of mine square
    sScreenY += MAP_GRID_Y + 1;  // slightly below
  }

  // show detailed mine info (name, production rate, daily production)

  SetFontDest(vsSB, MAP_VIEW_START_X, MAP_VIEW_START_Y,
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

  SetFontDest(vsFB, MAP_VIEW_START_X, MAP_VIEW_START_Y,
              MAP_VIEW_START_X + MAP_VIEW_WIDTH + MAP_GRID_X,
              MAP_VIEW_START_Y + MAP_VIEW_HEIGHT + 7, FALSE);
}

void AdjustXForLeftMapEdge(STR16 wString, INT16 *psX) {
  INT16 sStartingX, sPastEdge;

  if (fZoomFlag)
    // it's ok to cut strings off in zoomed mode
    return;

  sStartingX = *psX - (StringPixLengthArg(MAP_FONT, wcslen(wString), wString) / 2);
  sPastEdge = (MAP_VIEW_START_X + 23) - sStartingX;

  if (sPastEdge > 0) *psX += sPastEdge;
}

void BlitTownGridMarkers(void) {
  INT16 sScreenX = 0, sScreenY = 0;
  UINT32 uiDestPitchBYTES;
  UINT8 *pDestBuf;
  UINT16 usColor = 0;
  INT16 sWidth = 0, sHeight = 0;

  // get 16 bpp color
  usColor = Get16BPPColor(FROMRGB(100, 100, 100));

  // blit in the highlighted sector
  pDestBuf = VSurfaceLockOld(vsSB, &uiDestPitchBYTES);

  // clip to view region
  ClipBlitsToMapViewRegionForRectangleAndABit(uiDestPitchBYTES);

  struct TownSectors townSectors;
  GetAllTownSectors(&townSectors);

  // go through list of towns and place on screen
  for (int iCounter = 0; iCounter < townSectors.count; iCounter++) {
    TownID townID = townSectors.sectors[iCounter].townID;
    u8 sX = townSectors.sectors[iCounter].x;
    u8 sY = townSectors.sectors[iCounter].y;
    // skip Orta/Tixa until found
    if (((fFoundOrta != FALSE) || (townID != ORTA)) &&
        ((townID != TIXA) || (fFoundTixa != FALSE))) {
      if (fZoomFlag) {
        GetScreenXYFromMapXYStationary(sX, sY, &sScreenX, &sScreenY);
        sScreenX -= MAP_GRID_X - 1;
        sScreenY -= MAP_GRID_Y;

        sWidth = 2 * MAP_GRID_X;
        sHeight = 2 * MAP_GRID_Y;
      } else {
        // get location on screen
        GetScreenXYFromMapXY(sX, sY, &sScreenX, &sScreenY);
        sWidth = MAP_GRID_X - 1;
        sHeight = MAP_GRID_Y;

        sScreenX += 2;
      }

      if (GetTownIdForSector(sX, sY - 1) == BLANK_SECTOR) {
        LineDraw(TRUE, sScreenX - 1, sScreenY - 1, sScreenX + sWidth - 1, sScreenY - 1, usColor,
                 pDestBuf);
      }

      if ((GetTownIdForSector(sX, sY + 1) == BLANK_SECTOR)) {
        LineDraw(TRUE, sScreenX - 1, sScreenY + sHeight - 1, sScreenX + sWidth - 1,
                 sScreenY + sHeight - 1, usColor, pDestBuf);
      }

      if (GetTownIdForSector(sX - 1, sY) == BLANK_SECTOR) {
        LineDraw(TRUE, sScreenX - 2, sScreenY - 1, sScreenX - 2, sScreenY + sHeight - 1, usColor,
                 pDestBuf);
      }

      if (GetTownIdForSector(sX + 1, sY) == BLANK_SECTOR) {
        LineDraw(TRUE, sScreenX + sWidth - 1, sScreenY - 1, sScreenX + sWidth - 1,
                 sScreenY + sHeight - 1, usColor, pDestBuf);
      }
    }
  }

  // restore clips
  RestoreClipRegionToFullScreenForRectangle(uiDestPitchBYTES);

  // unlock surface
  VSurfaceUnlock(vsSB);

  return;
}

void BlitMineGridMarkers(void) {
  INT16 sScreenX = 0, sScreenY = 0;
  UINT32 uiDestPitchBYTES;
  UINT8 *pDestBuf;
  UINT16 usColor = 0;
  INT32 iCounter = 0;
  INT16 sWidth = 0, sHeight = 0;

  // get 16 bpp color
  usColor = Get16BPPColor(FROMRGB(100, 100, 100));

  // blit in the highlighted sector
  pDestBuf = VSurfaceLockOld(vsSB, &uiDestPitchBYTES);

  // clip to view region
  ClipBlitsToMapViewRegionForRectangleAndABit(uiDestPitchBYTES);

  for (iCounter = 0; iCounter < MAX_NUMBER_OF_MINES; iCounter++) {
    if (fZoomFlag) {
      GetScreenXYFromMapXYStationary((INT16)(gMineLocation[iCounter].sSectorX),
                                     (INT16)(gMineLocation[iCounter].sSectorY), &sScreenX,
                                     &sScreenY);
      sScreenX -= MAP_GRID_X;
      sScreenY -= MAP_GRID_Y;

      sWidth = 2 * MAP_GRID_X;
      sHeight = 2 * MAP_GRID_Y;
    } else {
      // get location on screen
      GetScreenXYFromMapXY((INT16)(gMineLocation[iCounter].sSectorX),
                           (INT16)(gMineLocation[iCounter].sSectorY), &sScreenX, &sScreenY);
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
  VSurfaceUnlock(vsSB);

  return;
}

void DisplayLevelString(void) {
  CHAR16 sString[32];

  // given the current level being displayed on the map, show a sub level message

  // at the surface
  if (!iCurrentMapSectorZ) {
    return;
  }

  // otherwise we will have to display the string with the level number

  SetFontDest(vsSB, MAP_VIEW_START_X, MAP_VIEW_START_Y,
              MAP_VIEW_START_X + MAP_VIEW_WIDTH + MAP_GRID_X,
              MAP_VIEW_START_Y + MAP_VIEW_HEIGHT + 7, FALSE);

  SetFont(MAP_FONT);
  SetFontForeground(MAP_INDEX_COLOR);
  SetFontBackground(FONT_BLACK);
  swprintf(sString, ARR_SIZE(sString), L"%s %d", sMapLevelString[0], iCurrentMapSectorZ);

  mprintf(MAP_LEVEL_STRING_X, MAP_LEVEL_STRING_Y, sString);

  SetFontDest(vsFB, 0, 0, 640, 480, FALSE);

  return;
}

// function to manipulate the number of towns people on the cursor
static BOOLEAN PickUpATownPersonFromSector(UINT8 ubType, u8 sX, u8 sY) {
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

  if (GetSectorID8(sX, sY) == GetSectorID8((u8)gWorldSectorX, (u8)gWorldSectorY)) {
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

  SetMapPanelDirty(true);

  return (TRUE);
}

BOOLEAN DropAPersonInASector(UINT8 ubType, u8 sX, u8 sY) {
  // are they in the same town as they were pickedup from
  if (GetTownIdForSector(sX, sY) != sSelectedMilitiaTown) {
    return (FALSE);
  }

  if (CountMilitiaInSector(sX, sY) >= MAX_ALLOWABLE_MILITIA_PER_SECTOR) {
    return (FALSE);
  }

  if (!SectorOursAndPeaceful(sX, sY, 0)) {
    return (FALSE);
  }

  if (GetSectorID8(sX, sY) == GetSectorID8((u8)gWorldSectorX, (u8)gWorldSectorY)) {
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

  SetMapPanelDirty(true);

  return (TRUE);
}

BOOLEAN LoadMilitiaPopUpBox(void) {
  // load the militia pop up box
  if (!AddVObjectFromFile("INTERFACE\\Militia.sti", &guiMilitia)) {
    return FALSE;
  }

  if (!AddVObjectFromFile("INTERFACE\\Militiamaps.sti", &guiMilitiaMaps)) {
    return FALSE;
  }

  if (!AddVObjectFromFile("INTERFACE\\MilitiamapsectorOutline2.sti", &guiMilitiaSectorHighLight)) {
    return FALSE;
  }

  if (!AddVObjectFromFile("INTERFACE\\MilitiamapsectorOutline.sti", &guiMilitiaSectorOutline)) {
    return FALSE;
  }

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
  struct Subimage *pTrav;

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

  BltVObject(vsFB, hVObject, 0, MAP_MILITIA_BOX_POS_X, MAP_MILITIA_BOX_POS_Y);

  GetVideoObject(&hVObject, guiMilitiaMaps);
  BltVObject(vsFB, hVObject, (UINT16)(sSelectedMilitiaTown - 1),
             MAP_MILITIA_BOX_POS_X + MAP_MILITIA_MAP_X, MAP_MILITIA_BOX_POS_Y + MAP_MILITIA_MAP_Y);

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
  pTrav = &(hVObject->subimages[0]);

  InvalidateRegion(MAP_MILITIA_BOX_POS_X, MAP_MILITIA_BOX_POS_Y,
                   MAP_MILITIA_BOX_POS_X + pTrav->width, MAP_MILITIA_BOX_POS_Y + pTrav->height);

  // set the text for the militia map sector info buttons
  SetMilitiaMapButtonsText();

  // render buttons
  MarkButtonsDirty();

  return (TRUE);
}

void CreateDestroyMilitiaPopUPRegions(void) {
  static INT16 sOldTown = 0;
  INT32 iCounter = 0;

  // create destroy militia pop up regions for mapscreen militia pop up box
  if (sSelectedMilitiaTown != 0) {
    sOldTown = sSelectedMilitiaTown;
  }

  if (fShowMilitia && sSelectedMilitiaTown && !gfMilitiaPopupCreated) {
    for (iCounter = 0; iCounter < 9; iCounter++) {
      MSYS_DefineRegion(&gMapScreenMilitiaBoxRegions[iCounter],
                        (INT16)(MAP_MILITIA_BOX_POS_X + MAP_MILITIA_MAP_X +
                                (iCounter % MILITIA_BOX_ROWS) * MILITIA_BOX_BOX_WIDTH),
                        (INT16)(MAP_MILITIA_BOX_POS_Y + MAP_MILITIA_MAP_Y +
                                (iCounter / MILITIA_BOX_ROWS) * MILITIA_BOX_BOX_HEIGHT),
                        (INT16)(MAP_MILITIA_BOX_POS_X + MAP_MILITIA_MAP_X +
                                (((iCounter) % MILITIA_BOX_ROWS) + 1) * MILITIA_BOX_BOX_WIDTH),
                        (INT16)(MAP_MILITIA_BOX_POS_Y + MAP_MILITIA_MAP_Y +
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
  u8 sBaseSectorValue = 0;
  u8 sCurrentSectorValue = 0;
  INT32 iCounter = 0;
  INT32 iNumberOfGreens = 0;
  INT32 iNumberOfRegulars = 0;
  INT32 iNumberOfElites = 0;
  INT32 iTotalNumberOfTroops = 0;
  INT32 iCurrentTroopIcon = 0;
  struct VObject *hVObject;
  INT32 iCurrentIcon = 0;
  INT16 sX, sY;
  CHAR16 sString[32];
  u8 sSectorX = 0, sSectorY = 0;

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
    FindFontRightCoordinates((INT16)(MAP_MILITIA_BOX_POS_X + MAP_MILITIA_MAP_X +
                                     ((iCounter % MILITIA_BOX_ROWS) * MILITIA_BOX_BOX_WIDTH)),
                             (INT16)(MAP_MILITIA_BOX_POS_Y + MAP_MILITIA_MAP_Y +
                                     ((iCounter / MILITIA_BOX_ROWS) * MILITIA_BOX_BOX_HEIGHT)),
                             MILITIA_BOX_BOX_WIDTH, 0, sString, FONT10ARIAL, &sX, &sY);

    if (GetTownIdForSector(sSectorX, sSectorY) != BLANK_SECTOR &&
        !IsSectorEnemyControlled(sSectorX, sSectorY)) {
      if (sSectorMilitiaMapSector != iCounter) {
        mprintf(sX, (INT16)(sY + MILITIA_BOX_BOX_HEIGHT - 5), sString);
      } else {
        mprintf(sX - 15, (INT16)(sY + MILITIA_BOX_BOX_HEIGHT - 5), sString);
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

      BltVObject(vsFB, hVObject, (UINT16)(iCurrentIcon), sX, sY);
    }
  }

  return;
}

u8 GetBaseSectorForCurrentTown(void) {
  u8 sBaseSector = 0;

  // is the current town
  if (sSelectedMilitiaTown != 0) {
    sBaseSector = sBaseSectorList[(INT16)(sSelectedMilitiaTown - 1)];
  }

  // return the current sector value
  return (sBaseSector);
}

void ShowHighLightedSectorOnMilitiaMap(void) {
  // show the highlighted sector on the militia map
  struct VObject *hVObject;
  INT16 sX = 0, sY = 0;

  if (sSectorMilitiaMapSector != -1) {
    sX = MAP_MILITIA_BOX_POS_X + MAP_MILITIA_MAP_X +
         ((sSectorMilitiaMapSector % MILITIA_BOX_ROWS) * MILITIA_BOX_BOX_WIDTH);
    sY = MAP_MILITIA_BOX_POS_Y + MAP_MILITIA_MAP_Y +
         ((sSectorMilitiaMapSector / MILITIA_BOX_ROWS) * MILITIA_BOX_BOX_HEIGHT);

    // get the object
    GetVideoObject(&hVObject, guiMilitiaSectorHighLight);

    // blt the object
    BltVObject(vsFB, hVObject, 0, sX, sY);
  }

  if (sSectorMilitiaMapSectorOutline != -1) {
    sX = MAP_MILITIA_BOX_POS_X + MAP_MILITIA_MAP_X +
         ((sSectorMilitiaMapSectorOutline % MILITIA_BOX_ROWS) * MILITIA_BOX_BOX_WIDTH);
    sY = MAP_MILITIA_BOX_POS_Y + MAP_MILITIA_MAP_Y +
         ((sSectorMilitiaMapSectorOutline / MILITIA_BOX_ROWS) * MILITIA_BOX_BOX_HEIGHT);

    // get the object
    GetVideoObject(&hVObject, guiMilitiaSectorOutline);

    // blt the object
    BltVObject(vsFB, hVObject, 0, sX, sY);
  }

  return;
}

void MilitiaRegionClickCallback(struct MOUSE_REGION *pRegion, INT32 iReason) {
  INT32 iValue = 0;

  iValue = MSYS_GetRegionUserData(pRegion, 0);

  if ((iReason & MSYS_CALLBACK_REASON_LBUTTON_UP)) {
    if (IsThisMilitiaTownSectorAllowable((INT16)iValue)) {
      if (sSectorMilitiaMapSector == (INT16)iValue) {
        sSectorMilitiaMapSector = -1;
      } else {
        sSectorMilitiaMapSector = (INT16)iValue;
      }
    } else {
      sSectorMilitiaMapSector = -1;
    }
  }

  if ((iReason & MSYS_CALLBACK_REASON_RBUTTON_UP)) {
    sSectorMilitiaMapSector = -1;
  }
}

void MilitiaRegionMoveCallback(struct MOUSE_REGION *pRegion, INT32 iReason) {
  INT32 iValue = 0;

  iValue = MSYS_GetRegionUserData(pRegion, 0);

  if ((iReason & MSYS_CALLBACK_REASON_GAIN_MOUSE)) {
    if (IsThisMilitiaTownSectorAllowable((INT16)iValue)) {
      sSectorMilitiaMapSectorOutline = (INT16)iValue;
    } else {
      sSectorMilitiaMapSectorOutline = -1;
    }
  } else if (iReason & MSYS_CALLBACK_REASON_LOST_MOUSE) {
    sSectorMilitiaMapSectorOutline = -1;
  }
}

void CreateDestroyMilitiaSectorButtons(void) {
  static BOOLEAN fCreated = FALSE;
  static INT16 sOldSectorValue = -1;
  INT16 sX = 0, sY = 0;
  INT32 iCounter = 0;
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
    // MSYS_DefineRegion( &gMapScreenMilitiaRegion, ( INT16 ) ( MAP_MILITIA_BOX_POS_X ), ( INT16
    // )( MAP_MILITIA_BOX_POS_Y  ), ( INT16 )( MAP_MILITIA_BOX_POS_X + pTrav->width ), ( INT16
    // )( MAP_MILITIA_BOX_POS_Y + pTrav->height ), MSYS_PRIORITY_HIGHEST - 2, MSYS_NO_CURSOR,
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
  CHAR16 sString[64];
  INT32 iNumberOfGreens = 0, iNumberOfRegulars = 0, iNumberOfElites = 0;
  u8 sBaseSectorValue = 0, sGlobalMapSector = 0;

  if (!fMilitiaMapButtonsCreated) {
    return;
  }

  // grab the appropriate global sector value in the world
  sBaseSectorValue = GetBaseSectorForCurrentTown();
  sGlobalMapSector = sBaseSectorValue + ((sSectorMilitiaMapSector % MILITIA_BOX_ROWS) +
                                         (sSectorMilitiaMapSector / MILITIA_BOX_ROWS) * (16));

  struct MilitiaCount milCount =
      GetMilitiaInSector(SectorID8_X(sGlobalMapSector), SectorID8_Y(sGlobalMapSector));
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

void MilitiaButtonCallback(GUI_BUTTON *btn, INT32 reason) {
  u8 sGlobalMapSector = 0;
  u8 sBaseSectorValue = 0;
  INT32 iValue = 0;

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
      DropAPersonInASector((UINT8)(iValue), (INT16)((sGlobalMapSector % 16) + 1),
                           (INT16)((sGlobalMapSector / 16) + 1));
    }
  } else if (reason & MSYS_CALLBACK_REASON_RBUTTON_DWN) {
    btn->uiFlags |= (BUTTON_CLICKED_ON);
  } else if (reason & MSYS_CALLBACK_REASON_RBUTTON_UP) {
    if (btn->uiFlags & BUTTON_CLICKED_ON) {
      btn->uiFlags &= ~(BUTTON_CLICKED_ON);
      PickUpATownPersonFromSector((UINT8)(iValue), (INT16)((sGlobalMapSector % 16) + 1),
                                  (INT16)((sGlobalMapSector / 16) + 1));
    }
  }
}

void DisplayUnallocatedMilitia(void) {
  // show the nunber on the cursor
  INT32 iTotalNumberOfTroops = 0, iNumberOfGreens = 0, iNumberOfRegulars = 0, iNumberOfElites = 0,
        iCurrentTroopIcon = 0;
  INT32 iCurrentIcon = 0;
  INT16 sX = 0, sY = 0;
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

    BltVObject(vsFB, hVObject, (UINT16)(iCurrentIcon), sX, sY);
  }
}

BOOLEAN IsThisMilitiaTownSectorAllowable(INT16 sSectorIndexValue) {
  u8 sBaseSectorValue = 0, sGlobalMapSector = 0;
  u8 sSectorX, sSectorY;

  // is this sector allowed to be clicked on?
  sBaseSectorValue = GetBaseSectorForCurrentTown();
  sGlobalMapSector = sBaseSectorValue + ((sSectorIndexValue % MILITIA_BOX_ROWS) +
                                         (sSectorIndexValue / MILITIA_BOX_ROWS) * 16);

  sSectorX = SectorID8_X(sGlobalMapSector);
  sSectorY = SectorID8_Y(sGlobalMapSector);

  // is this in fact part of a town?
  if (GetTownIdForSector(sSectorX, sSectorY) == BLANK_SECTOR) {
    return (FALSE);
  }

  if (!SectorOursAndPeaceful(sSectorX, sSectorY, 0)) {
    return (FALSE);
  }

  // valid
  return (TRUE);
}

void DrawTownMilitiaName(void) {
  CHAR16 sString[64];
  INT16 sX, sY;

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

void HandleShutDownOfMilitiaPanelIfPeopleOnTheCursor(INT16 sTownValue) {
  INT32 iNumberUnderControl = 0, iNumberThatCanFitInSector = 0, iCount = 0;
  BOOLEAN fLastOne = FALSE;

  // check if anyone still on the cursor
  if (!sGreensOnCursor && !sRegularsOnCursor && !sElitesOnCursor) {
    return;
  }

  // yep someone left
  iNumberUnderControl = GetTownSectorsUnderControl((INT8)sTownValue);

  // find number of sectors under player's control
  struct TownSectors townSectors;
  GetAllTownSectors(&townSectors);
  for (int iCounter = 0; iCounter < townSectors.count; iCounter++) {
    if (townSectors.sectors[iCounter].townID == sTownValue) {
      u8 sX = townSectors.sectors[iCounter].x;
      u8 sY = townSectors.sectors[iCounter].y;
      if (SectorOursAndPeaceful(sX, sY, 0)) {
        iCount = 0;
        iNumberThatCanFitInSector = MAX_ALLOWABLE_MILITIA_PER_SECTOR;
        iNumberThatCanFitInSector -= CountMilitiaInSector(sX, sY);

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

        if (GetSectorID8(sX, sY) == GetSectorID8((u8)gWorldSectorX, (u8)gWorldSectorY)) {
          TacticalMilitiaRefreshRequired();
        }
      }

      fLastOne = TRUE;

      for (int iCounterB = iCounter + 1; iCounterB < townSectors.count; iCounterB++) {
        if (townSectors.sectors[iCounterB].townID == sTownValue) {
          fLastOne = FALSE;
        }
      }

      if (fLastOne) {
        IncMilitiaOfRankInSector(sX, sY, GREEN_MILITIA, sGreensOnCursor % iNumberUnderControl);
        IncMilitiaOfRankInSector(sX, sY, REGULAR_MILITIA, sRegularsOnCursor % iNumberUnderControl);
        IncMilitiaOfRankInSector(sX, sY, ELITE_MILITIA, sElitesOnCursor % iNumberUnderControl);
      }
    }
  }

  // zero out numbers on the cursor
  sGreensOnCursor = 0;
  sRegularsOnCursor = 0;
  sElitesOnCursor = 0;

  return;
}

void HandleEveningOutOfTroopsAmongstSectors(void) {
  // even out troops among the town
  INT32 iCounter = 0, iNumberUnderControl = 0, iNumberOfGreens = 0, iNumberOfRegulars = 0,
        iNumberOfElites = 0, iTotalNumberOfTroops = 0;
  INT32 iNumberLeftOverGreen = 0, iNumberLeftOverRegular = 0, iNumberLeftOverElite = 0;
  u8 sBaseSectorValue = 0, sCurrentSectorValue = 0;
  INT16 sTotalSoFar = 0;

  // how many sectors in the selected town do we control?
  iNumberUnderControl = GetTownSectorsUnderControl((INT8)sSelectedMilitiaTown);

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

    u8 sSectorX = SectorID8_X(sCurrentSectorValue);
    u8 sSectorY = SectorID8_Y(sCurrentSectorValue);

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
  struct TownSectors townSectors;
  GetAllTownSectors(&townSectors);
  for (int iCounter = 0; iCounter < townSectors.count; iCounter++) {
    if (townSectors.sectors[iCounter].townID == sSelectedMilitiaTown) {
      u8 sX = townSectors.sectors[iCounter].x;
      u8 sY = townSectors.sectors[iCounter].y;

      if (!IsSectorEnemyControlled(sX, sY) && !NumHostilesInSector(sX, sY, 0)) {
        // distribute here
        SetMilitiaOfRankInSector(sX, sY, GREEN_MILITIA, iNumberOfGreens / iNumberUnderControl);
        SetMilitiaOfRankInSector(sX, sY, REGULAR_MILITIA, iNumberOfRegulars / iNumberUnderControl);
        SetMilitiaOfRankInSector(sX, sY, ELITE_MILITIA, iNumberOfElites / iNumberUnderControl);
        sTotalSoFar = CountMilitiaInSector(sX, sY);

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
        if (GetSectorID8(sX, sY) == GetSectorID8((u8)gWorldSectorX, (u8)gWorldSectorY) &&
            gWorldSectorY != 0) {
          TacticalMilitiaRefreshRequired();
        }
      }
    }
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
  SetMapPanelDirty(true);
}

void MilitiaAutoButtonCallback(GUI_BUTTON *btn, INT32 reason) {
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    btn->uiFlags |= (BUTTON_CLICKED_ON);
  } else if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    if (btn->uiFlags & BUTTON_CLICKED_ON) {
      btn->uiFlags &= ~(BUTTON_CLICKED_ON);

      // distribute troops over all the sectors under control
      HandleEveningOutOfTroopsAmongstSectors();
      SetMapPanelDirty(true);
    }
  }

  return;
}

void MilitiaDoneButtonCallback(GUI_BUTTON *btn, INT32 reason) {
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    btn->uiFlags |= (BUTTON_CLICKED_ON);
  } else if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    if (btn->uiFlags & BUTTON_CLICKED_ON) {
      btn->uiFlags &= ~(BUTTON_CLICKED_ON);

      // reset fact we are in the box
      sSelectedMilitiaTown = 0;
      SetMapPanelDirty(true);
    }
  }

  return;
}

void RenderShadingForUnControlledSectors(void) {
  // now render shading over any uncontrolled sectors
  // get the sector value for the upper left corner
  u8 sBaseSectorValue = 0, sCurrentSectorValue = 0, sX = 0, sY = 0;
  INT32 iCounter = 0;

  // get the base sector value
  sBaseSectorValue = GetBaseSectorForCurrentTown();

  // render icons for map
  for (iCounter = 0; iCounter < 9; iCounter++) {
    // grab current sector value
    sCurrentSectorValue =
        sBaseSectorValue + ((iCounter % MILITIA_BOX_ROWS) + (iCounter / MILITIA_BOX_ROWS) * (16));

    u8 sSectorX = SectorID8_X(sCurrentSectorValue);
    u8 sSectorY = SectorID8_Y(sCurrentSectorValue);
    if ((GetTownIdForSector(sSectorX, sSectorY) != BLANK_SECTOR) &&
        ((IsSectorEnemyControlled(sSectorX, sSectorY)) ||
         (NumHostilesInSector(sSectorX, sSectorY, 0)))) {
      // shade this sector, not under our control
      sX = MAP_MILITIA_BOX_POS_X + MAP_MILITIA_MAP_X +
           ((iCounter % MILITIA_BOX_ROWS) * MILITIA_BOX_BOX_WIDTH);
      sY = MAP_MILITIA_BOX_POS_Y + MAP_MILITIA_MAP_Y +
           ((iCounter / MILITIA_BOX_ROWS) * MILITIA_BOX_BOX_HEIGHT);

      ShadowVideoSurfaceRect(vsFB, sX, sY, sX + MILITIA_BOX_BOX_WIDTH - 1,
                             sY + MILITIA_BOX_BOX_HEIGHT - 1);
    }
  }

  return;
}

void DrawTownMilitiaForcesOnMap(void) {
  INT32 iCounter = 0, iCounterB = 0, iTotalNumberOfTroops = 0, iIconValue = 0;
  struct VObject *hVObject;

  // get militia video object
  GetVideoObject(&hVObject, guiMilitia);

  // clip blits to mapscreen region
  ClipBlitsToMapViewRegion();

  struct TownSectors townSectors;
  GetAllTownSectors(&townSectors);
  for (int iCounter = 0; iCounter < townSectors.count; iCounter++) {
    // run through each town sector and plot the icons for the militia forces in the town
    u8 sSectorX = townSectors.sectors[iCounter].x;
    u8 sSectorY = townSectors.sectors[iCounter].y;
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

        DrawMapBoxIcon(hVObject, (UINT16)iIconValue, sSectorX, sSectorY, (UINT8)iCounterB);
      }
    }
  }

  // now handle militia for sam sectors
  for (iCounter = 0; iCounter < GetSamSiteCount(); iCounter++) {
    u8 sSectorX = GetSamSiteX(iCounter);
    u8 sSectorY = GetSamSiteY(iCounter);
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

        DrawMapBoxIcon(hVObject, (UINT16)iIconValue, sSectorX, sSectorY, (UINT8)iCounterB);
      }
    }
  }
  // restore clip blits
  RestoreClipRegionToFullScreen();

  return;
}

void CheckAndUpdateStatesOfSelectedMilitiaSectorButtons(void) {
  // now set the militia map button text
  INT32 iNumberOfGreens = 0, iNumberOfRegulars = 0, iNumberOfElites = 0;
  u8 sBaseSectorValue = 0, sGlobalMapSector = 0;

  if (!fMilitiaMapButtonsCreated) {
    EnableButton(giMapMilitiaButton[4]);
    return;
  }

  // grab the appropriate global sector value in the world
  sBaseSectorValue = GetBaseSectorForCurrentTown();
  sGlobalMapSector = sBaseSectorValue + ((sSectorMilitiaMapSector % MILITIA_BOX_ROWS) +
                                         (sSectorMilitiaMapSector / MILITIA_BOX_ROWS) * (16));

  struct MilitiaCount milCount =
      GetMilitiaInSector(SectorID8_X(sGlobalMapSector), SectorID8_Y(sGlobalMapSector));
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

BOOLEAN ShadeUndergroundMapElem(u8 sSectorX, u8 sSectorY) {
  INT16 sScreenX, sScreenY;

  GetScreenXYFromMapXY(sSectorX, sSectorY, &sScreenX, &sScreenY);

  sScreenX += 1;

  ShadowVideoSurfaceRect(vsSB, sScreenX, sScreenY, sScreenX + MAP_GRID_X - 2,
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
    if (pNode->ubSectorZ == (UINT8)iCurrentMapSectorZ && !pNode->fVisited) {
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
  BltVObject(vsSB, hHandle, 0, MAP_VIEW_START_X + 21, MAP_VIEW_START_Y + 17);

  // handle shading of sublevels
  ShadeSubLevelsNotVisited();

  return;
}

void MilitiaBoxMaskBtnCallback(struct MOUSE_REGION *pRegion, INT32 iReason) {
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
  INT32 i;
  for (i = 0; i < 256; i++) {
    SectorInfo[i].uiFlags &= ~SF_PLAYER_KNOWS_ENEMIES_ARE_HERE;
  }

  // redraw map
  SetMapPanelDirty(true);
}

UINT32 WhatPlayerKnowsAboutEnemiesInSector(u8 sSectorX, u8 sSectorY) {
  UINT32 uiSectorFlags = SectorInfo[GetSectorID8(sSectorX, sSectorY)].uiFlags;

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

BOOLEAN CanMercsScoutThisSector(u8 sSectorX, u8 sSectorY, INT8 bSectorZ) {
  INT32 iFirstId = 0, iLastId = 0;
  INT32 iCounter = 0;
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

void HandleShowingOfEnemyForcesInSector(u8 sSectorX, u8 sSectorY, INT8 bSectorZ,
                                        UINT8 ubIconPosition) {
  INT16 sNumberOfEnemies = 0;

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
  INT32 iCounter = 0;
  INT16 sX = 0, sY = 0;
  struct VObject *hHandle;
  INT8 ubVidObjIndex = 0;
  UINT32 uiDestPitchBYTES;
  CHAR16 wString[40];

  if (fShowAircraftFlag) {
    BlitSAMGridMarkers();
  }

  for (iCounter = 0; iCounter < GetSamSiteCount(); iCounter++) {
    // has the sam site here been found?
    if (!IsSamSiteFound(iCounter)) {
      continue;
    }

    // get the sector x and y
    u8 sSectorX = GetSamSiteX(iCounter);
    u8 sSectorY = GetSamSiteY(iCounter);

    if (fZoomFlag) {
      VSurfaceLockOld(vsSB, &uiDestPitchBYTES);
      SetClippingRegionAndImageWidth(uiDestPitchBYTES, MAP_VIEW_START_X + MAP_GRID_X - 1,
                                     MAP_VIEW_START_Y + MAP_GRID_Y - 1, MAP_VIEW_WIDTH + 1,
                                     MAP_VIEW_HEIGHT - 9);
      VSurfaceUnlock(vsSB);

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
    BltVObject(vsSB, hHandle, ubVidObjIndex, sX, sY);

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

      SetFontDest(vsSB, MapScreenRect.iLeft + 2, MapScreenRect.iTop, MapScreenRect.iRight,
                  MapScreenRect.iBottom, FALSE);

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
  INT16 sScreenX = 0, sScreenY = 0;
  UINT32 uiDestPitchBYTES;
  UINT8 *pDestBuf;
  UINT16 usColor = 0;
  INT32 iCounter = 0;
  INT16 sWidth = 0, sHeight = 0;

  // get 16 bpp color
  usColor = Get16BPPColor(FROMRGB(100, 100, 100));

  pDestBuf = VSurfaceLockOld(vsSB, &uiDestPitchBYTES);

  // clip to view region
  ClipBlitsToMapViewRegionForRectangleAndABit(uiDestPitchBYTES);

  for (iCounter = 0; iCounter < GetSamSiteCount(); iCounter++) {
    // has the sam site here been found?
    if (!IsSamSiteFound(iCounter)) {
      continue;
    }

    u8 sX = GetSamSiteX(iCounter);
    u8 sY = GetSamSiteY(iCounter);

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
  VSurfaceUnlock(vsSB);

  return;
}

BOOLEAN CanMilitiaAutoDistribute(void) {
  INT32 iCounter = 0;
  u8 sBaseSectorValue = 0, sCurrentSectorValue = 0;
  u8 sSectorX = 0, sSectorY = 0;
  INT32 iTotalTroopsInTown = 0;

  // can't auto-distribute if we don't have a town selected (this excludes SAM sites)
  if (sSelectedMilitiaTown == BLANK_SECTOR) return (FALSE);

  // can't auto-distribute if we don't control any sectors in the the town
  if (!GetTownSectorsUnderControl((INT8)sSelectedMilitiaTown)) return (FALSE);

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
      iTotalTroopsInTown += CountMilitiaInSector(sSectorX, sSectorY);
    }
  }

  // can't auto-distribute if we don't have any militia in the town
  if (!iTotalTroopsInTown) return (FALSE);

  // can auto-distribute
  return (TRUE);
}

void ShowItemsOnMap(void) {
  u8 sMapX, sMapY;
  INT16 sXCorner, sYCorner;
  INT16 usXPos, usYPos;
  UINT32 uiItemCnt;
  CHAR16 sString[10];

  // clip blits to mapscreen region
  ClipBlitsToMapViewRegion();

  SetFontDest(vsSB, MapScreenRect.iLeft + 2, MapScreenRect.iTop, MapScreenRect.iRight,
              MapScreenRect.iBottom, FALSE);

  SetFont(MAP_FONT);
  SetFontForeground(FONT_MCOLOR_LTGREEN);
  SetFontBackground(FONT_MCOLOR_BLACK);

  // run through sectors
  for (sMapX = 1; sMapX < (MAP_WORLD_X - 1); sMapX++) {
    for (sMapY = 1; sMapY < (MAP_WORLD_Y - 1); sMapY++) {
      // to speed this up, only look at sector that player has visited
      if (GetSectorFlagStatus(sMapX, sMapY, (UINT8)iCurrentMapSectorZ, SF_ALREADY_VISITED)) {
        //				uiItemCnt = GetSizeOfStashInSector( sMapX, sMapY, ( INT16 )
        // iCurrentMapSectorZ, FALSE );
        uiItemCnt = GetNumberOfVisibleWorldItemsFromSectorStructureForSector(
            sMapX, sMapY, (UINT8)iCurrentMapSectorZ);

        if (uiItemCnt > 0) {
          sXCorner = (INT16)(MAP_VIEW_START_X + (sMapX * MAP_GRID_X));
          sYCorner = (INT16)(MAP_VIEW_START_Y + (sMapY * MAP_GRID_Y));

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

void DrawMapBoxIcon(struct VObject *hIconHandle, UINT16 usVOIndex, u8 sMapX, u8 sMapY,
                    UINT8 ubIconPosition) {
  INT32 iRowNumber, iColumnNumber;
  INT32 iX, iY;

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

    BltVObject(vsSB, hIconHandle, usVOIndex, iX, iY);
    InvalidateRegion(iX, iY, iX + DMAP_GRID_X, iY + DMAP_GRID_Y);
  }
}

void DrawOrta() {
  UINT32 uiDestPitchBYTES;
  INT16 sX, sY;
  UINT8 ubVidObjIndex;
  struct VObject *hHandle;

  if (fZoomFlag) {
    VSurfaceLockOld(vsSB, &uiDestPitchBYTES);
    SetClippingRegionAndImageWidth(uiDestPitchBYTES, MAP_VIEW_START_X + MAP_GRID_X - 1,
                                   MAP_VIEW_START_Y + MAP_GRID_Y - 1, MAP_VIEW_WIDTH + 1,
                                   MAP_VIEW_HEIGHT - 9);
    VSurfaceUnlock(vsSB);

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
  BltVObject(vsSB, hHandle, ubVidObjIndex, sX, sY);
}

void DrawTixa() {
  UINT32 uiDestPitchBYTES;
  INT16 sX, sY;
  UINT8 ubVidObjIndex;
  struct VObject *hHandle;

  if (fZoomFlag) {
    VSurfaceLockOld(vsSB, &uiDestPitchBYTES);
    SetClippingRegionAndImageWidth(uiDestPitchBYTES, MAP_VIEW_START_X + MAP_GRID_X - 1,
                                   MAP_VIEW_START_Y + MAP_GRID_Y - 1, MAP_VIEW_WIDTH + 1,
                                   MAP_VIEW_HEIGHT - 9);
    VSurfaceUnlock(vsSB);
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
  BltVObject(vsSB, hHandle, ubVidObjIndex, sX, sY);
}

void DrawBullseye() {
  INT16 sX, sY;
  struct VObject *hHandle;

  GetScreenXYFromMapXY(gsMercArriveSectorX, gsMercArriveSectorY, &sX, &sY);
  sY -= 2;

  // draw the bullseye in that sector
  GetVideoObject(&hHandle, guiBULLSEYE);
  BltVObject(vsSB, hHandle, 0, sX, sY);
}

void HideExistenceOfUndergroundMapSector(UINT8 ubSectorX, UINT8 ubSectorY) {
  INT16 sScreenX;
  INT16 sScreenY;

  GetScreenXYFromMapXY(ubSectorX, ubSectorY, &sScreenX, &sScreenY);

  // fill it with near black
  VSurfaceColorFill(vsSB, sScreenX + 1, sScreenY, sScreenX + MAP_GRID_X, sScreenY + MAP_GRID_Y - 1,
                    gusUndergroundNearBlack);
}

void InitMapSecrets(void) {
  UINT8 ubSamIndex;

  fFoundTixa = FALSE;
  fFoundOrta = FALSE;

  for (ubSamIndex = 0; ubSamIndex < GetSamSiteCount(); ubSamIndex++) {
    SetSamSiteFound(ubSamIndex, false);
  }
}

BOOLEAN CanRedistributeMilitiaInSector(INT16 sClickedSectorX, INT16 sClickedSectorY,
                                       INT8 bClickedTownId) {
  INT32 iCounter = 0;
  u8 sBaseSectorValue = 0, sCurrentSectorValue = 0;
  u8 sSectorX = 0, sSectorY = 0;

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
    if (GetTownIdForSector(sSectorX, sSectorY) != bClickedTownId) {
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
