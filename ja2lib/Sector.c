#include "Sector.h"

#include "Strategic/QueenCommand.h"
#include "Strategic/Strategic.h"
#include "Strategic/StrategicMap.h"
#include "Tactical/Overhead.h"
#include "rust_sam_sites.h"

SectorID16 SectorID8To16(SectorID8 sectorID) {
  return GetSectorID16(SectorID8_X(sectorID), SectorID8_Y(sectorID));
}
SectorID8 SectorID16To8(SectorID16 sectorID) {
  return GetSectorID8(SectorID16_X(sectorID), SectorID16_Y(sectorID));
}

struct SectorInfo* GetSectorInfoByID8(SectorID8 sectorID) { return &SectorInfo[sectorID]; }
struct SectorInfo* GetSectorInfoByXY(u8 x, u8 y) { return &SectorInfo[GetSectorID8(x, y)]; }

BOOLEAN SectorOursAndPeaceful(INT16 sMapX, INT16 sMapY, INT8 bMapZ) {
  // if this sector is currently loaded
  if ((sMapX == gWorldSectorX) && (sMapY == gWorldSectorY) && (bMapZ == gbWorldSectorZ)) {
    // and either there are enemies prowling this sector, or combat is in progress
    if (gTacticalStatus.fEnemyInSector || (gTacticalStatus.uiFlags & INCOMBAT)) {
      return FALSE;
    }
  }

  // if sector is controlled by enemies, it's not ours (duh!)
  if (!bMapZ && IsSectorEnemyControlled(sMapX, sMapY)) {
    return FALSE;
  }

  if (NumHostilesInSector(sMapX, sMapY, bMapZ)) {
    return FALSE;
  }

  // safe & secure, s'far as we can tell
  return (TRUE);
}

i16 GetLoadedSectorX() { return gWorldSectorX; }
i16 GetLoadedSectorY() { return gWorldSectorY; }

bool IsSectorEnemyControlled(i8 sMapX, i8 sMapY) {
  return StrategicMap[GetSectorID16(sMapX, sMapY)].fEnemyControlled;
}

// fEnemyControlled