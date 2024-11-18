#ifndef __SMOKE_EFFECTS
#define __SMOKE_EFFECTS

#include "SGP/Types.h"

// Smoke effect types
enum {
  NO_SMOKE_EFFECT,
  NORMAL_SMOKE_EFFECT,
  TEARGAS_SMOKE_EFFECT,
  MUSTARDGAS_SMOKE_EFFECT,
  CREATURE_SMOKE_EFFECT,
};

#define SMOKE_EFFECT_INDOORS 0x01
#define SMOKE_EFFECT_ON_ROOF 0x02
#define SMOKE_EFFECT_MARK_FOR_UPDATE 0x04

typedef struct TAG_SMOKE_EFFECT {
  int16_t sGridNo;  // gridno at which the tear gas cloud is centered

  uint8_t ubDuration;  // the number of turns gas will remain effective
  uint8_t ubRadius;    // the current radius of the cloud in map tiles
  uint8_t bFlags;      // 0 - outdoors (fast spread), 1 - indoors (slow)
  int8_t bAge;         // the number of turns gas has been around
  BOOLEAN fAllocated;
  int8_t bType;
  uint16_t usItem;
  uint8_t ubOwner;
  uint8_t ubPadding;
  uint32_t uiTimeOfLastUpdate;

} SMOKEEFFECT;

// Returns NO_SMOKE_EFFECT if none there...
int8_t GetSmokeEffectOnTile(int16_t sGridNo, int8_t bLevel);

// Decays all smoke effects...
void DecaySmokeEffects(uint32_t uiTime);

// Add smoke to gridno
// ( Replacement algorithm uses distance away )
void AddSmokeEffectToTile(int32_t iSmokeEffectID, int8_t bType, int16_t sGridNo, int8_t bLevel);

void RemoveSmokeEffectFromTile(int16_t sGridNo, int8_t bLevel);

int32_t NewSmokeEffect(int16_t sGridNo, uint16_t usItem, int8_t bLevel, uint8_t ubOwner);

BOOLEAN SaveSmokeEffectsToSaveGameFile(FileID hFile);
BOOLEAN LoadSmokeEffectsFromLoadGameFile(FileID hFile);

BOOLEAN SaveSmokeEffectsToMapTempFile(u8 sMapX, u8 sMapY, i8 bMapZ);
BOOLEAN LoadSmokeEffectsFromMapTempFile(u8 sMapX, u8 sMapY, i8 bMapZ);

void ResetSmokeEffects();

void UpdateSmokeEffectGraphics();

#endif
