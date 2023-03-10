#ifndef __LIGHT_EFFECTS
#define __LIGHT_EFFECTS

#include "SGP/Types.h"

// Light effect types
enum {
  NO_LIGHT_EFFECT,
  LIGHT_FLARE_MARK_1,
};

typedef struct {
  int16_t sGridNo;  // gridno at which the tear gas cloud is centered

  uint8_t ubDuration;  // the number of turns will remain effective
  uint8_t bRadius;     // the current radius
  int8_t bAge;         // the number of turns light has been around
  BOOLEAN fAllocated;
  int8_t bType;
  int32_t iLight;
  uint32_t uiTimeOfLastUpdate;

} LIGHTEFFECT;

// Decays all light effects...
void DecayLightEffects(uint32_t uiTime);

// Add light to gridno
// ( Replacement algorithm uses distance away )
void AddLightEffectToTile(int8_t bType, int16_t sGridNo);

void RemoveLightEffectFromTile(int16_t sGridNo);

int32_t NewLightEffect(int16_t sGridNo, int8_t bType);

BOOLEAN SaveLightEffectsToSaveGameFile(HWFILE hFile);
BOOLEAN LoadLightEffectsFromLoadGameFile(HWFILE hFile);

BOOLEAN SaveLightEffectsToMapTempFile(uint8_t sMapX, uint8_t sMapY, int8_t bMapZ);
BOOLEAN LoadLightEffectsFromMapTempFile(uint8_t sMapX, uint8_t sMapY, int8_t bMapZ);
void ResetLightEffects();

#endif
