#pragma once

/* Warning, this file is autogenerated by cbindgen. Don't modify this manually. */

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

/**
 * Min condition for sam site to be functional
 */
#define MIN_CONDITION_FOR_SAM_SITE_TO_WORK 80

enum SamSite {
  SamSiteChitzena = 0,
  SamSiteDrassen = 1,
  SamSiteCambria = 2,
  SamSiteMeduna = 3,
};

enum OptionalSamSite_Tag {
  Some,
  None,
};

struct OptionalSamSite {
  enum OptionalSamSite_Tag tag;
  union {
    struct {
      enum SamSite some;
    };
  };
};

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * Return total number of SAM sites
 */
uint8_t GetSamSiteCount(void);

/**
 * Return X location of the SAM site
 */
uint8_t GetSamSiteX(enum SamSite site);

/**
 * Return Y location of the SAM site
 */
uint8_t GetSamSiteY(enum SamSite site);

/**
 * Check if the SAM site was found.
 */
bool IsSamSiteFound(enum SamSite site);

/**
 * Set if the SAM site was found.
 */
void SetSamSiteFound(enum SamSite site, bool value);

uint8_t GetSamGraphicsID(enum SamSite site);

int16_t GetSamGridNoA(enum SamSite site);

int16_t GetSamGridNoB(enum SamSite site);

bool DoesSAMExistHere(uint8_t sector_x, uint8_t sector_y, int8_t sector_z, int16_t grid_no);

uint8_t GetSamCondition(enum SamSite site);

void SetSamCondition(enum SamSite site, uint8_t value);

/**
 * Return SAM if it is in the given sector.
 */
struct OptionalSamSite GetSamAtSector(uint8_t x, uint8_t y, int8_t z);

/**
 * Returns SAM controlling the given sector.
 */
struct OptionalSamSite GetSamControllingSector(uint8_t x, uint8_t y);

bool IsThereAFunctionalSamInSector(uint8_t x, uint8_t y, int8_t z);

uint8_t GetNumberOfSAMSitesUnderPlayerControl(void);

bool IsSamUnderPlayerControl(enum SamSite site);

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus
