#include "Town.h"

#include "Strategic/Strategic.h"

static TownSectors allTownSectors;

const TownSectors* GetAllTownSectors() { return &allTownSectors; }

void BuildListOfTownSectors(void) {
  for (int i = 0; i < ARR_SIZE(allTownSectors); i++) {
    allTownSectors[i].townID = BLANK_SECTOR;
  }

  INT32 iCounter = 0;
  for (u8 x = 0; x < MAP_WORLD_X; x++) {
    for (u8 y = 0; y < MAP_WORLD_Y; y++) {
      TownID town = GetTownIdForSector(x, y);
      if (town != BLANK_SECTOR) {
        allTownSectors[iCounter].townID = town;
        allTownSectors[iCounter].sectorID = GetSectorID16(x, y);
        iCounter++;
      }
    }
  }
}
