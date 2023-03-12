#pragma once

/* Warning, this file is autogenerated by cbindgen. Don't modify this manually. */

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#define FIRST_CIV_GROUP 1

#define NUM_CIV_GROUPS 20

enum CivGroup {
  NON_CIV_GROUP = 0,
  REBEL_CIV_GROUP,
  KINGPIN_CIV_GROUP,
  SANMONA_ARMS_GROUP,
  ANGELS_GROUP,
  BEGGARS_CIV_GROUP,
  TOURISTS_CIV_GROUP,
  ALMA_MILITARY_CIV_GROUP,
  DOCTORS_CIV_GROUP,
  COUPLE1_CIV_GROUP,
  HICKS_CIV_GROUP,
  WARDEN_CIV_GROUP,
  JUNKYARD_CIV_GROUP,
  FACTORY_KIDS_GROUP,
  QUEENS_CIV_GROUP,
  UNNAMED_CIV_GROUP_15,
  UNNAMED_CIV_GROUP_16,
  UNNAMED_CIV_GROUP_17,
  UNNAMED_CIV_GROUP_18,
  UNNAMED_CIV_GROUP_19,
};

enum CivGroupHostility {
  CIV_GROUP_NEUTRAL = 0,
  CIV_GROUP_WILL_EVENTUALLY_BECOME_HOSTILE = 1,
  CIV_GROUP_WILL_BECOME_HOSTILE = 2,
  CIV_GROUP_HOSTILE = 3,
};

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

enum CivGroupHostility GetCivGroupHostility(enum CivGroup group);

void SetCivGroupHostility(enum CivGroup group, enum CivGroupHostility hostility);

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus
