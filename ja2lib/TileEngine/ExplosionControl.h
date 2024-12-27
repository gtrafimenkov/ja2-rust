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

  INT16 sX;       // World X ( optional )
  INT16 sY;       // World Y ( optional )
  INT16 sZ;       // World Z ( optional )
  INT16 sGridNo;  // World GridNo
  BOOLEAN fLocate;
  int8_t bLevel;  // World level
  uint8_t ubUnsed[1];

} EXPLOSION_PARAMS;

typedef struct {
  EXPLOSION_PARAMS Params;
  BOOLEAN fAllocated;
  INT16 sCurrentFrame;
  INT32 iID;
  INT32 iLightID;
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

void IgniteExplosion(uint8_t ubOwner, INT16 sX, INT16 sY, INT16 sZ, INT16 sGridNo, uint16_t usItem,
                     int8_t bLevel);
void InternalIgniteExplosion(uint8_t ubOwner, INT16 sX, INT16 sY, INT16 sZ, INT16 sGridNo,
                             uint16_t usItem, BOOLEAN fLocate, int8_t bLevel);

void GenerateExplosion(EXPLOSION_PARAMS *pExpParams);

void SpreadEffect(INT16 sGridNo, uint8_t ubRadius, uint16_t usItem, uint8_t ubOwner,
                  BOOLEAN fSubsequent, int8_t bLevel, INT32 iSmokeEffectNum);

void AddBombToQueue(uint32_t uiWorldBombIndex, uint32_t uiTimeStamp);

void DecayBombTimers(void);
void SetOffBombsByFrequency(uint8_t ubID, int8_t bFrequency);
BOOLEAN SetOffBombsInGridNo(uint8_t ubID, INT16 sGridNo, BOOLEAN fAllBombs, int8_t bLevel);
void ActivateSwitchInGridNo(uint8_t ubID, INT16 sGridNo);
void SetOffPanicBombs(uint8_t ubID, int8_t bPanicTrigger);

void UpdateExplosionFrame(INT32 iIndex, INT16 sCurrentFrame);
void RemoveExplosionData(INT32 iIndex);

void UpdateAndDamageSAMIfFound(u8 sSectorX, u8 sSectorY, INT16 sSectorZ, INT16 sGridNo,
                               uint8_t ubDamage);
void UpdateSAMDoneRepair(u8 sSectorX, u8 sSectorY, INT16 sSectorZ);

BOOLEAN SaveExplosionTableToSaveGameFile(HWFILE hFile);

BOOLEAN LoadExplosionTableFromSavedGameFile(HWFILE hFile);

INT32 FindActiveTimedBomb(void);
BOOLEAN ActiveTimedBombExists(void);
void RemoveAllActiveTimedBombs(void);

#define GASMASK_MIN_STATUS 70

BOOLEAN DishOutGasDamage(struct SOLDIERTYPE *pSoldier, EXPLOSIVETYPE *pExplosive, INT16 sSubsequent,
                         BOOLEAN fRecompileMovementCosts, INT16 sWoundAmt, INT16 sBreathAmt,
                         uint8_t ubOwner);

#endif
