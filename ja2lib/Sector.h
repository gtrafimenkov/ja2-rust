#ifndef __SECTOR_H
#define __SECTOR_H

#include "SGP/Types.h"
#include "rust_sector.h"

#define GetSectorID8_STATIC(x, y) ((y - 1) * 16 + x - 1)

struct SectorInfo;

struct SectorInfo* GetSectorInfoByID8(SectorID8 sectorIndex);
struct SectorInfo* GetSectorInfoByXY(u8 x, u8 y);

// Counts enemies and crepitus, but not bloodcats.
uint8_t NumHostilesInSector(u8 sSectorX, u8 sSectorY, i8 sSectorZ);

// Returns TRUE if sector is under player control, has no enemies in it, and isn't currently in
// combat mode
BOOLEAN SectorOursAndPeaceful(u8 sMapX, u8 sMapY, int8_t bMapZ);

// This will get an ID string like A9- OMERTA...
void GetSectorIDString(u8 sSectorX, u8 sSectorY, int8_t bSectorZ, wchar_t* zString, size_t bufSize,
                       BOOLEAN fDetailed);

u8 GetLoadedSectorX();
u8 GetLoadedSectorY();

#endif
