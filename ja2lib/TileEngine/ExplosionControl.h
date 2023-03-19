#ifndef _EXPLOSION_CONTROL_H
#define _EXPLOSION_CONTROL_H

#include "Tactical/Weapons.h"

#define MAX_DISTANCE_EXPLOSIVE_CAN_DESTROY_STRUCTURES 2

#define EXPLOSION_FLAG_USEABSPOS 0x00000001
#define EXPLOSION_FLAG_DISPLAYONLY 0x00000002

// Explosion Data
typedef struct {
  uint32_t uiFlags;

  uint8_t ubOwner;
  uint8_t ubTypeID;

  uint16_t usItem;

  int16_t sX;       // World X ( optional )
  int16_t sY;       // World Y ( optional )
  int16_t sZ;       // World Z ( optional )
  int16_t sGridNo;  // World GridNo
  BOOLEAN fLocate;
  int8_t bLevel;  // World level
  uint8_t ubUnsed[1];

} EXPLOSION_PARAMS;

typedef struct {
  EXPLOSION_PARAMS Params;
  BOOLEAN fAllocated;
  int16_t sCurrentFrame;
  int32_t iID;
  int32_t iLightID;
  uint8_t ubUnsed[2];

} EXPLOSIONTYPE;

typedef enum {
  NO_BLAST,
  BLAST_1,
  BLAST_2,
  BLAST_3,
  STUN_BLAST,
  WATER_BLAST,
  TARGAS_EXP,
  SMOKE_EXP,
  MUSTARD_EXP,

  NUM_EXP_TYPES

} EXPLOSION_TYPES;

typedef struct {
  uint32_t uiWorldBombIndex;
  uint32_t uiTimeStamp;
  uint8_t fExists;
} ExplosionQueueElement;

#define ERASE_SPREAD_EFFECT 2
#define BLOOD_SPREAD_EFFECT 3
#define REDO_SPREAD_EFFECT 4

#define NUM_EXPLOSION_SLOTS 100
extern EXPLOSIONTYPE gExplosionData[NUM_EXPLOSION_SLOTS];

extern uint8_t gubElementsOnExplosionQueue;
extern BOOLEAN gfExplosionQueueActive;

void IgniteExplosion(uint8_t ubOwner, int16_t sX, int16_t sY, int16_t sZ, int16_t sGridNo,
                     uint16_t usItem, int8_t bLevel);
void InternalIgniteExplosion(uint8_t ubOwner, int16_t sX, int16_t sY, int16_t sZ, int16_t sGridNo,
                             uint16_t usItem, BOOLEAN fLocate, int8_t bLevel);

void GenerateExplosion(EXPLOSION_PARAMS *pExpParams);

void SpreadEffect(int16_t sGridNo, uint8_t ubRadius, uint16_t usItem, uint8_t ubOwner,
                  BOOLEAN fSubsequent, int8_t bLevel, int32_t iSmokeEffectNum);

void AddBombToQueue(uint32_t uiWorldBombIndex, uint32_t uiTimeStamp);

void DecayBombTimers(void);
void SetOffBombsByFrequency(uint8_t ubID, int8_t bFrequency);
BOOLEAN SetOffBombsInGridNo(uint8_t ubID, int16_t sGridNo, BOOLEAN fAllBombs, int8_t bLevel);
void ActivateSwitchInGridNo(uint8_t ubID, int16_t sGridNo);
void SetOffPanicBombs(uint8_t ubID, int8_t bPanicTrigger);

void UpdateExplosionFrame(int32_t iIndex, int16_t sCurrentFrame);
void RemoveExplosionData(int32_t iIndex);

void UpdateAndDamageSAMIfFound(uint8_t sSectorX, uint8_t sSectorY, int8_t sSectorZ, int16_t sGridNo,
                               uint8_t ubDamage);
void UpdateSAMDoneRepair(uint8_t sSectorX, uint8_t sSectorY, int8_t sSectorZ);

BOOLEAN SaveExplosionTableToSaveGameFile(FileID hFile);

BOOLEAN LoadExplosionTableFromSavedGameFile(FileID hFile);

int32_t FindActiveTimedBomb(void);
BOOLEAN ActiveTimedBombExists(void);
void RemoveAllActiveTimedBombs(void);

#define GASMASK_MIN_STATUS 70

BOOLEAN DishOutGasDamage(struct SOLDIERTYPE *pSoldier, EXPLOSIVETYPE *pExplosive,
                         int16_t sSubsequent, BOOLEAN fRecompileMovementCosts, int16_t sWoundAmt,
                         int16_t sBreathAmt, uint8_t ubOwner);

#endif
