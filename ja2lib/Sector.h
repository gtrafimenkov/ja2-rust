#ifndef __SECTOR_H
#define __SECTOR_H

#include "SGP/Types.h"
#include "rust_sector.h"

#define GetSectorID8_STATIC(x, y) ((y - 1) * 16 + x - 1)

struct SectorInfo;

struct SectorInfo* GetSectorInfoByID8(SectorID8 sectorIndex);
struct SectorInfo* GetSectorInfoByXY(uint8_t x, uint8_t y);

// Counts enemies and crepitus, but not bloodcats.
uint8_t NumHostilesInSector(uint8_t sSectorX, uint8_t sSectorY, int8_t sSectorZ);

// Returns TRUE if sector is under player control, has no enemies in it, and isn't currently in
// combat mode
BOOLEAN SectorOursAndPeaceful(uint8_t sMapX, uint8_t sMapY, int8_t bMapZ);

// This will get an ID string like A9- OMERTA...
void GetSectorIDString(uint8_t sSectorX, uint8_t sSectorY, int8_t bSectorZ, wchar_t* zString,
                       size_t bufSize, BOOLEAN fDetailed);

uint8_t GetLoadedSectorX();
uint8_t GetLoadedSectorY();

#endif
