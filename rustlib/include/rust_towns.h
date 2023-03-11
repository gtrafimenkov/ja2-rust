#pragma once

/* Warning, this file is autogenerated by cbindgen. Don't modify this manually. */

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

enum TownID {
  BLANK_SECTOR = 0,
  OMERTA = 1,
  DRASSEN = 2,
  ALMA = 3,
  GRUMM = 4,
  TIXA = 5,
  CAMBRIA = 6,
  SAN_MONA = 7,
  ESTONI = 8,
  ORTA = 9,
  BALIME = 10,
  MEDUNA = 11,
  CHITZENA = 12,
};

struct TownSector {
  enum TownID townID;
  uint8_t x;
  uint8_t y;
};

struct TownSectors {
  uint8_t count;
  struct TownSector sectors[40];
};

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * Returs the list of all sectors belonging to towns
 */
void GetAllTownSectors(struct TownSectors *data);

/**
 * Is militia training allowed in this town?
 */
bool MilitiaTrainingAllowedInTown(enum TownID town);

/**
 * Return TownID the sector belongs to.
 */
enum TownID GetTownIdForSectorID16(SectorID16 sector_id);

/**
 * Return TownID the sector belongs to.
 */
enum TownID GetTownIdForSector(uint8_t x, uint8_t y);

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus
