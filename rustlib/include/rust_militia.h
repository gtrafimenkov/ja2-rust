#pragma once

/* Warning, this file is autogenerated by cbindgen. Don't modify this manually. */

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#define MAX_MILITIA_LEVELS 3

enum MilitiaRank {
  GREEN_MILITIA = 0,
  REGULAR_MILITIA,
  ELITE_MILITIA,
};

/**
 * Militia force in a sector.
 */
struct MilitiaCount {
  uint8_t green;
  uint8_t regular;
  uint8_t elite;
};

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * Return militia force in a sector.
 */
struct MilitiaCount GetMilitiaInSector(uint8_t x, uint8_t y);

/**
 * Set militia force in a sector.
 */
void SetMilitiaInSector(uint8_t x, uint8_t y, struct MilitiaCount value);

uint8_t GetMilitiaOfRankInSector(uint8_t x, uint8_t y, enum MilitiaRank rank);

uint8_t CountMilitiaInSector(uint8_t x, uint8_t y);

uint8_t CountMilitiaInSector3D(uint8_t x, uint8_t y, int8_t z);

void SetMilitiaOfRankInSector(uint8_t x, uint8_t y, enum MilitiaRank rank, uint8_t value);

void IncMilitiaOfRankInSector(uint8_t x, uint8_t y, enum MilitiaRank rank, uint8_t increase);

bool IsMilitiaTrainingPayedForSector(uint8_t x, uint8_t y);

void SetMilitiaTrainingPayedForSector(uint8_t x, uint8_t y, bool value);

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus
