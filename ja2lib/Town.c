#include "Town.h"

#include "Strategic/Strategic.h"

static TownSectors allTownSectors;

const TownSectors* GetAllTownSectors() { return &allTownSectors; }

void BuildListOfTownSectors(void) {
  for (int i = 0; i < ARR_SIZE(allTownSectors); i++) {
    allTownSectors[i].townID = BLANK_SECTOR;
  }

  int32_t iCounter = 0;
  for (int32_t iCounterX = 0; iCounterX < MAP_WORLD_X; iCounterX++) {
    for (int32_t iCounterY = 0; iCounterY < MAP_WORLD_Y; iCounterY++) {
      uint16_t usSector = GetSectorID16(iCounterX, iCounterY);

      if ((StrategicMap[usSector].townID >= FIRST_TOWN) &&
          (StrategicMap[usSector].townID < NUM_TOWNS)) {
        allTownSectors[iCounter].townID = StrategicMap[usSector].townID;
        allTownSectors[iCounter].sectorID = usSector;
        iCounter++;
      }
    }
  }
}
