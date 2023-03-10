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
struct SectorInfo* GetSectorInfoByXY(u8 x, u8 y);

// Counts enemies and crepitus, but not bloodcats.
UINT8 NumHostilesInSector(u8 sSectorX, u8 sSectorY, i8 sSectorZ);

// Returns TRUE if sector is under player control, has no enemies in it, and isn't currently in
// combat mode
BOOLEAN SectorOursAndPeaceful(u8 sMapX, u8 sMapY, INT8 bMapZ);

// This will get an ID string like A9- OMERTA...
void GetSectorIDString(u8 sSectorX, u8 sSectorY, INT8 bSectorZ, CHAR16* zString, size_t bufSize,
                       BOOLEAN fDetailed);

u8 GetLoadedSectorX();
u8 GetLoadedSectorY();

#endif
