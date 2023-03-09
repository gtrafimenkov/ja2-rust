#ifndef __SECTOR_H
#define __SECTOR_H

#include "Militia.h"
#include "SGP/Types.h"
#include "rust_sector.h"

#define GetSectorID8_STATIC(x, y) ((y - 1) * 16 + x - 1)

SectorID16 SectorID8To16(SectorID8 sectorID);
SectorID8 SectorID16To8(SectorID16 sectorID);

struct SectorInfo;

struct SectorInfo* GetSectorInfoByID8(SectorID8 sectorIndex);
struct SectorInfo* GetSectorInfoByXY(uint8_t x, uint8_t y);

// Counts enemies and crepitus, but not bloodcats.
uint8_t NumHostilesInSector(uint8_t sSectorX, uint8_t sSectorY, int16_t sSectorZ);

// Returns TRUE if sector is under player control, has no enemies in it, and isn't currently in
// combat mode
BOOLEAN SectorOursAndPeaceful(int16_t sMapX, int16_t sMapY, int8_t bMapZ);

// This will get an ID string like A9- OMERTA...
void GetSectorIDString(uint8_t sSectorX, uint8_t sSectorY, int8_t bSectorZ, wchar_t* zString,
                       size_t bufSize, BOOLEAN fDetailed);

int16_t GetLoadedSectorX();
int16_t GetLoadedSectorY();

#endif
