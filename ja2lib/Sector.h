#ifndef __SECTOR_H
#define __SECTOR_H

#include "Militia.h"
#include "SGP/Types.h"
#include "rustlib.h"

// Sector ID 0-324 (18x18)
typedef i16 SectorID16;

#define GetSectorID8_STATIC(x, y) ((y - 1) * 16 + x - 1)

#define MAP_WORLD_X 18
#define MAP_WORLD_Y 18

// Convert coordinates (1-16, 1-16) to 0-324 index.
SectorID16 GetSectorID16(u8 x, u8 y);
// Get X [1-16] from SectorID16
u8 SectorID16_X(SectorID16 sectorID);
// Get Y [1-16] from SectorID16
u8 SectorID16_Y(SectorID16 sectorID);

SectorID16 SectorID8To16(SectorID8 sectorID);
SectorID8 SectorID16To8(SectorID16 sectorID);

struct SectorInfo;

struct SectorInfo* GetSectorInfoByID8(SectorID8 sectorIndex);
struct SectorInfo* GetSectorInfoByXY(u8 x, u8 y);

// Counts enemies and crepitus, but not bloodcats.
UINT8 NumHostilesInSector(u8 sSectorX, u8 sSectorY, INT16 sSectorZ);

// Returns TRUE if sector is under player control, has no enemies in it, and isn't currently in
// combat mode
BOOLEAN SectorOursAndPeaceful(INT16 sMapX, INT16 sMapY, INT8 bMapZ);

BOOLEAN IsThisSectorASAMSector(u8 sSectorX, u8 sSectorY, INT8 bSectorZ);

// This will get an ID string like A9- OMERTA...
void GetSectorIDString(u8 sSectorX, u8 sSectorY, INT8 bSectorZ, CHAR16* zString, size_t bufSize,
                       BOOLEAN fDetailed);

i16 GetLoadedSectorX();
i16 GetLoadedSectorY();

bool IsSectorEnemyControlled(i8 sMapX, i8 sMapY);

#endif
