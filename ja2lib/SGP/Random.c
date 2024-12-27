// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include "SGP/Random.h"

#ifdef PRERANDOM_GENERATOR

uint32_t guiPreRandomIndex = 0;
uint32_t guiPreRandomNums[MAX_PREGENERATED_NUMS];

#ifdef JA2BETAVERSION
uint32_t guiRandoms = 0;
uint32_t guiPreRandoms = 0;
BOOLEAN gfCountRandoms = FALSE;
#endif

#endif

void InitializeRandom() {
  // Seed the random-number generator with current time so that
  // the numbers will be different every time we run.
  srand((unsigned)time(NULL));
#ifdef PRERANDOM_GENERATOR
  // Pregenerate all of the random numbers.
  for (guiPreRandomIndex = 0; guiPreRandomIndex < MAX_PREGENERATED_NUMS; guiPreRandomIndex++) {
    guiPreRandomNums[guiPreRandomIndex] = rand();
  }
  guiPreRandomIndex = 0;
#endif
}

// Returns a pseudo-random integer between 0 and uiRange
uint32_t Random(uint32_t uiRange) {
// Always return 0, if no range given (it's not an error)
#ifdef JA2BETAVERSION
  if (gfCountRandoms) {
    guiRandoms++;
  }
#endif

  if (uiRange == 0) return (0);
  return rand() * uiRange / RAND_MAX % uiRange;
}

BOOLEAN Chance(uint32_t uiChance) { return (BOOLEAN)(Random(100) < uiChance); }

#ifdef PRERANDOM_GENERATOR

uint32_t PreRandom(uint32_t uiRange) {
  uint32_t uiNum;
#ifdef JA2BETAVERSION
  if (gfCountRandoms) {
    guiPreRandoms++;
  }
#endif
  if (!uiRange) return 0;
  // Extract the current pregenerated number
  uiNum = guiPreRandomNums[guiPreRandomIndex] * uiRange / RAND_MAX % uiRange;
  // Replace the current pregenerated number with a new one.

  // This was removed in the name of optimization.  Uncomment if you hate recycling.
  // guiPreRandomNums[ guiPreRandomIndex ] = rand();

  // Go to the next index.
  guiPreRandomIndex++;
  if (guiPreRandomIndex >= (uint32_t)MAX_PREGENERATED_NUMS) guiPreRandomIndex = 0;
  return uiNum;
}

BOOLEAN PreChance(uint32_t uiChance) { return (BOOLEAN)(PreRandom(100) < uiChance); }

#ifdef JA2BETAVERSION
void CountRandomCalls(BOOLEAN fStart) {
  gfCountRandoms = fStart;
  if (fStart) {
    guiRandoms = 0;
    guiPreRandoms = 0;
  }
}

void GetRandomCalls(uint32_t *puiRandoms, uint32_t *puiPreRandoms) {
  *puiRandoms = guiRandoms;
  *puiPreRandoms = guiPreRandoms;
}
#endif

#endif
