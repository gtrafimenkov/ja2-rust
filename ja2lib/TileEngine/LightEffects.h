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

  UINT8 ubDuration;  // the number of turns will remain effective
  UINT8 bRadius;     // the current radius
  INT8 bAge;         // the number of turns light has been around
  BOOLEAN fAllocated;
  INT8 bType;
  INT32 iLight;
  UINT32 uiTimeOfLastUpdate;

} LIGHTEFFECT;

// Decays all light effects...
void DecayLightEffects(UINT32 uiTime);

// Add light to gridno
// ( Replacement algorithm uses distance away )
void AddLightEffectToTile(INT8 bType, INT16 sGridNo);

void RemoveLightEffectFromTile(INT16 sGridNo);

INT32 NewLightEffect(INT16 sGridNo, INT8 bType);

BOOLEAN SaveLightEffectsToSaveGameFile(FileID hFile);
BOOLEAN LoadLightEffectsFromLoadGameFile(FileID hFile);

BOOLEAN SaveLightEffectsToMapTempFile(u8 sMapX, u8 sMapY, i8 bMapZ);
BOOLEAN LoadLightEffectsFromMapTempFile(u8 sMapX, u8 sMapY, i8 bMapZ);
void ResetLightEffects();

#endif
