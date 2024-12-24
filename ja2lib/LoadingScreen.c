#include "LoadingScreen.h"

#include <string.h>

#include "SGP/Debug.h"
#include "SGP/Font.h"
#include "SGP/Random.h"
#include "SGP/VSurface.h"
#include "SGP/Video.h"
#include "Strategic/CampaignTypes.h"
#include "Strategic/GameClock.h"
#include "TileEngine/RenderDirty.h"
#include "Utils/FontControl.h"

uint8_t gubLastLoadingScreenID = LOADINGSCREEN_NOTHING;

// returns the uint8_t ID for the specified sector.
uint8_t GetLoadScreenID(u8 sSectorX, u8 sSectorY, int8_t bSectorZ) {
  SECTORINFO* pSector;
  uint8_t ubSectorID;
  BOOLEAN fNight = FALSE;

  ubSectorID = GetSectorID8(sSectorX, sSectorY);
  if (IsNightTime())  // before 5AM or after 9PM
  {
    fNight = TRUE;
  }
  switch (bSectorZ) {
    case 0:
      switch (ubSectorID) {
        case SEC_A2:
        case SEC_B2:
          if (fNight) return LOADINGSCREEN_NIGHTCHITZENA;
          return LOADINGSCREEN_DAYCHITZENA;
        case SEC_A9:
          if (!DidGameJustStart()) {
            if (fNight) return LOADINGSCREEN_NIGHTOMERTA;
            return LOADINGSCREEN_DAYOMERTA;
          }
          return LOADINGSCREEN_HELI;
        case SEC_A10:
          if (fNight) return LOADINGSCREEN_NIGHTOMERTA;
          return LOADINGSCREEN_DAYOMERTA;
        case SEC_P3:
          if (fNight) return LOADINGSCREEN_NIGHTPALACE;
          return LOADINGSCREEN_DAYPALACE;
        case SEC_H13:
        case SEC_H14:  // military installations
        case SEC_I13:
        case SEC_N7:
          if (fNight) return LOADINGSCREEN_NIGHTMILITARY;
          return LOADINGSCREEN_DAYMILITARY;
        case SEC_K4:
          if (fNight) return LOADINGSCREEN_NIGHTLAB;
          return LOADINGSCREEN_DAYLAB;
        case SEC_J9:
          if (fNight) return LOADINGSCREEN_NIGHTPRISON;
          return LOADINGSCREEN_DAYPRISON;
        case SEC_D2:
        case SEC_D15:
        case SEC_I8:
        case SEC_N4:
          if (fNight) return LOADINGSCREEN_NIGHTSAM;
          return LOADINGSCREEN_DAYSAM;
        case SEC_F8:
          if (fNight) return LOADINGSCREEN_NIGHTHOSPITAL;
          return LOADINGSCREEN_DAYHOSPITAL;
        case SEC_B13:
        case SEC_N3:
          if (fNight) return LOADINGSCREEN_NIGHTAIRPORT;
          return LOADINGSCREEN_DAYAIRPORT;
        case SEC_L11:
        case SEC_L12:
          if (fNight) return LOADINGSCREEN_NIGHTBALIME;
          return LOADINGSCREEN_DAYBALIME;
        case SEC_H3:
        case SEC_H8:
        case SEC_D4:
          if (fNight) return LOADINGSCREEN_NIGHTMINE;
          return LOADINGSCREEN_DAYMINE;
      }
      pSector = &SectorInfo[ubSectorID];
      switch (pSector->ubTraversability[4]) {
        case TOWN:
          if (fNight) {
            if (Random(2)) {
              return LOADINGSCREEN_NIGHTTOWN2;
            }
            return LOADINGSCREEN_NIGHTTOWN1;
          }
          if (Random(2)) {
            return LOADINGSCREEN_DAYTOWN2;
          }
          return LOADINGSCREEN_DAYTOWN1;
        case SAND:
        case SAND_ROAD:
          if (fNight) {
            return LOADINGSCREEN_NIGHTDESERT;
          }
          return LOADINGSCREEN_DAYDESERT;
        case FARMLAND:
        case FARMLAND_ROAD:
        case ROAD:
          if (fNight) {
            return LOADINGSCREEN_NIGHTGENERIC;
          }
          return LOADINGSCREEN_DAYGENERIC;
        case PLAINS:
        case SPARSE:
        case HILLS:
        case PLAINS_ROAD:
        case SPARSE_ROAD:
        case HILLS_ROAD:
          if (fNight) {
            return LOADINGSCREEN_NIGHTWILD;
          }
          return LOADINGSCREEN_DAYWILD;
        case DENSE:
        case SWAMP:
        case SWAMP_ROAD:
        case DENSE_ROAD:
          if (fNight) {
            return LOADINGSCREEN_NIGHTFOREST;
          }
          return LOADINGSCREEN_DAYFOREST;
        case TROPICS:
        case TROPICS_ROAD:
        case WATER:
        case NS_RIVER:
        case EW_RIVER:
        case COASTAL:
        case COASTAL_ROAD:
          if (fNight) {
            return LOADINGSCREEN_NIGHTTROPICAL;
          }
          return LOADINGSCREEN_DAYTROPICAL;
        default:
          Assert(0);
          if (fNight) {
            return LOADINGSCREEN_NIGHTGENERIC;
          }
          return LOADINGSCREEN_DAYGENERIC;
      }
      break;
    case 1:
      switch (ubSectorID) {
        case SEC_A10:  // Miguel's basement
        case SEC_I13:  // Alma prison dungeon
        case SEC_J9:   // Tixa prison dungeon
        case SEC_K4:   // Orta weapons plant
        case SEC_O3:   // Meduna
        case SEC_P3:   // Meduna
          return LOADINGSCREEN_BASEMENT;
        default:  // rest are mines
          return LOADINGSCREEN_MINE;
      }
      break;
    case 2:
    case 3:
      // all level 2 and 3 maps are caves!
      return LOADINGSCREEN_CAVE;
    default:
      // shouldn't ever happen
      Assert(FALSE);

      if (fNight) {
        return LOADINGSCREEN_NIGHTGENERIC;
      }
      return LOADINGSCREEN_DAYGENERIC;
  }
}

// sets up the loadscreen with specified ID, and draws it to the FRAME_BUFFER,
// and refreshing the screen with it.
void DisplayLoadScreenWithID(uint8_t ubLoadScreenID) {
  SGPFILENAME ImageFile;

  switch (ubLoadScreenID) {
    case LOADINGSCREEN_NOTHING:
      strcpy(ImageFile, "LOADSCREENS\\LS_Heli.sti");
      break;
    case LOADINGSCREEN_DAYGENERIC:
      strcpy(ImageFile, "LOADSCREENS\\LS_DayGeneric.sti");
      break;
    case LOADINGSCREEN_DAYTOWN1:
      strcpy(ImageFile, "LOADSCREENS\\LS_DayTown1.sti");
      break;
    case LOADINGSCREEN_DAYTOWN2:
      strcpy(ImageFile, "LOADSCREENS\\LS_DayTown2.sti");
      break;
    case LOADINGSCREEN_DAYWILD:
      strcpy(ImageFile, "LOADSCREENS\\LS_DayWild.sti");
      break;
    case LOADINGSCREEN_DAYTROPICAL:
      strcpy(ImageFile, "LOADSCREENS\\LS_DayTropical.sti");
      break;
    case LOADINGSCREEN_DAYFOREST:
      strcpy(ImageFile, "LOADSCREENS\\LS_DayForest.sti");
      break;
    case LOADINGSCREEN_DAYDESERT:
      strcpy(ImageFile, "LOADSCREENS\\LS_DayDesert.sti");
      break;
    case LOADINGSCREEN_DAYPALACE:
      strcpy(ImageFile, "LOADSCREENS\\LS_DayPalace.sti");
      break;
    case LOADINGSCREEN_NIGHTGENERIC:
      strcpy(ImageFile, "LOADSCREENS\\LS_NightGeneric.sti");
      break;
    case LOADINGSCREEN_NIGHTWILD:
      strcpy(ImageFile, "LOADSCREENS\\LS_NightWild.sti");
      break;
    case LOADINGSCREEN_NIGHTTOWN1:
      strcpy(ImageFile, "LOADSCREENS\\LS_NightTown1.sti");
      break;
    case LOADINGSCREEN_NIGHTTOWN2:
      strcpy(ImageFile, "LOADSCREENS\\LS_NightTown2.sti");
      break;
    case LOADINGSCREEN_NIGHTFOREST:
      strcpy(ImageFile, "LOADSCREENS\\LS_NightForest.sti");
      break;
    case LOADINGSCREEN_NIGHTTROPICAL:
      strcpy(ImageFile, "LOADSCREENS\\LS_NightTropical.sti");
      break;
    case LOADINGSCREEN_NIGHTDESERT:
      strcpy(ImageFile, "LOADSCREENS\\LS_NightDesert.sti");
      break;
    case LOADINGSCREEN_NIGHTPALACE:
      strcpy(ImageFile, "LOADSCREENS\\LS_NightPalace.sti");
      break;
    case LOADINGSCREEN_HELI:
      strcpy(ImageFile, "LOADSCREENS\\LS_Heli.sti");
      break;
    case LOADINGSCREEN_BASEMENT:
      strcpy(ImageFile, "LOADSCREENS\\LS_Basement.sti");
      break;
    case LOADINGSCREEN_MINE:
      strcpy(ImageFile, "LOADSCREENS\\LS_Mine.sti");
      break;
    case LOADINGSCREEN_CAVE:
      strcpy(ImageFile, "LOADSCREENS\\LS_Cave.sti");
      break;
    case LOADINGSCREEN_DAYPINE:
      strcpy(ImageFile, "LOADSCREENS\\LS_DayPine.sti");
      break;
    case LOADINGSCREEN_NIGHTPINE:
      strcpy(ImageFile, "LOADSCREENS\\LS_NightPine.sti");
      break;
    case LOADINGSCREEN_DAYMILITARY:
      strcpy(ImageFile, "LOADSCREENS\\LS_DayMilitary.sti");
      break;
    case LOADINGSCREEN_NIGHTMILITARY:
      strcpy(ImageFile, "LOADSCREENS\\LS_NightMilitary.sti");
      break;
    case LOADINGSCREEN_DAYSAM:
      strcpy(ImageFile, "LOADSCREENS\\LS_DaySAM.sti");
      break;
    case LOADINGSCREEN_NIGHTSAM:
      strcpy(ImageFile, "LOADSCREENS\\LS_NightSAM.sti");
      break;
    case LOADINGSCREEN_DAYPRISON:
      strcpy(ImageFile, "LOADSCREENS\\LS_DayPrison.sti");
      break;
    case LOADINGSCREEN_NIGHTPRISON:
      strcpy(ImageFile, "LOADSCREENS\\LS_NightPrison.sti");
      break;
    case LOADINGSCREEN_DAYHOSPITAL:
      strcpy(ImageFile, "LOADSCREENS\\LS_DayHospital.sti");
      break;
    case LOADINGSCREEN_NIGHTHOSPITAL:
      strcpy(ImageFile, "LOADSCREENS\\LS_NightHospital.sti");
      break;
    case LOADINGSCREEN_DAYAIRPORT:
      strcpy(ImageFile, "LOADSCREENS\\LS_DayAirport.sti");
      break;
    case LOADINGSCREEN_NIGHTAIRPORT:
      strcpy(ImageFile, "LOADSCREENS\\LS_NightAirport.sti");
      break;
    case LOADINGSCREEN_DAYLAB:
      strcpy(ImageFile, "LOADSCREENS\\LS_DayLab.sti");
      break;
    case LOADINGSCREEN_NIGHTLAB:
      strcpy(ImageFile, "LOADSCREENS\\LS_NightLab.sti");
      break;
    case LOADINGSCREEN_DAYOMERTA:
      strcpy(ImageFile, "LOADSCREENS\\LS_DayOmerta.sti");
      break;
    case LOADINGSCREEN_NIGHTOMERTA:
      strcpy(ImageFile, "LOADSCREENS\\LS_NightOmerta.sti");
      break;
    case LOADINGSCREEN_DAYCHITZENA:
      strcpy(ImageFile, "LOADSCREENS\\LS_DayChitzena.sti");
      break;
    case LOADINGSCREEN_NIGHTCHITZENA:
      strcpy(ImageFile, "LOADSCREENS\\LS_NightChitzena.sti");
      break;
    case LOADINGSCREEN_DAYMINE:
      strcpy(ImageFile, "LOADSCREENS\\LS_DayMine.sti");
      break;
    case LOADINGSCREEN_NIGHTMINE:
      strcpy(ImageFile, "LOADSCREENS\\LS_NightMine.sti");
      break;
    case LOADINGSCREEN_DAYBALIME:
      strcpy(ImageFile, "LOADSCREENS\\LS_DayBalime.sti");
      break;
    case LOADINGSCREEN_NIGHTBALIME:
      strcpy(ImageFile, "LOADSCREENS\\LS_NightBalime.sti");
      break;
    default:
      strcpy(ImageFile, "LOADSCREENS\\LS_Heli.sti");
      break;
  }

  struct Image* image = CreateImage(ImageFile, false);
  if (image) {
    BlitImageToSurface(image, vsFB, 0, 0);
    DestroyImage(image);
  } else {
    // Failed to load the file, so use a black screen and print out message.
    SetFont(FONT10ARIAL);
    SetFontForeground(FONT_YELLOW);
    SetFontShadow(FONT_NEARBLACK);
    VSurfaceColorFill(vsFB, 0, 0, 640, 480, 0);
    mprintf(5, 5, L"%S loadscreen data file not found...", ImageFile);
  }

  gubLastLoadingScreenID = ubLoadScreenID;
  InvalidateScreen();
  ExecuteBaseDirtyRectQueue();
  EndFrameBufferRender();
  RefreshScreen();
}
