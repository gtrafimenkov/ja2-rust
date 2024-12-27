#ifndef __LIGHT_EFFECTS
#define __LIGHT_EFFECTS

#include "SGP/Types.h"

// Light effect types
enum {
  NO_LIGHT_EFFECT,
  LIGHT_FLARE_MARK_1,
};

typedef struct {
  INT16 sGridNo;  // gridno at which the tear gas cloud is centered

  uint8_t ubDuration;  // the number of turns will remain effective
  uint8_t bRadius;     // the current radius
  INT8 bAge;           // the number of turns light has been around
  BOOLEAN fAllocated;
  INT8 bType;
  INT32 iLight;
  uint32_t uiTimeOfLastUpdate;

} LIGHTEFFECT;

// Decays all light effects...
void DecayLightEffects(uint32_t uiTime);

// Add light to gridno
// ( Replacement algorithm uses distance away )
void AddLightEffectToTile(INT8 bType, INT16 sGridNo);

void RemoveLightEffectFromTile(INT16 sGridNo);

INT32 NewLightEffect(INT16 sGridNo, INT8 bType);

BOOLEAN SaveLightEffectsToSaveGameFile(HWFILE hFile);
BOOLEAN LoadLightEffectsFromLoadGameFile(HWFILE hFile);

BOOLEAN SaveLightEffectsToMapTempFile(INT16 sMapX, INT16 sMapY, INT8 bMapZ);
BOOLEAN LoadLightEffectsFromMapTempFile(INT16 sMapX, INT16 sMapY, INT8 bMapZ);
void ResetLightEffects();

#endif
