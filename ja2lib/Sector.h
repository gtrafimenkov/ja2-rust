#ifndef __SECTOR_H
#define __SECTOR_H

#include "Militia.h"
#include "SGP/Types.h"

// Sector ID 0-255 (16x16)
typedef uint8_t SectorID8;

// Sector ID 0-324 (18x18)
typedef int16_t SectorID16;

// Convert coordinates ([1-16], [1-16]) to 0-255 index.
// This function should be prefered over GetSectorID8_STATIC macro.
SectorID8 GetSectorID8(uint8_t x, uint8_t y);
#define GetSectorID8_STATIC(x, y) ((y - 1) * 16 + x - 1)
// Get X [1-16] from SectorID8
uint8_t SectorID8_X(SectorID8 sectorID);
// Get Y [1-16] from SectorID8
uint8_t SectorID8_Y(SectorID8 sectorID);

#define MAP_WORLD_X 18
#define MAP_WORLD_Y 18

// Convert coordinates (1-16, 1-16) to 0-324 index.
SectorID16 GetSectorID16(uint8_t x, uint8_t y);
// Get X [1-16] from SectorID16
uint8_t SectorID16_X(SectorID16 sectorID);
// Get Y [1-16] from SectorID16
uint8_t SectorID16_Y(SectorID16 sectorID);

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

BOOLEAN IsThisSectorASAMSector(uint8_t sSectorX, uint8_t sSectorY, int8_t bSectorZ);

// This will get an ID string like A9- OMERTA...
void GetSectorIDString(uint8_t sSectorX, uint8_t sSectorY, int8_t bSectorZ, wchar_t* zString,
                       size_t bufSize, BOOLEAN fDetailed);

int16_t GetLoadedSectorX();
int16_t GetLoadedSectorY();

bool IsSectorEnemyControlled(int8_t sMapX, int8_t sMapY);

#endif
