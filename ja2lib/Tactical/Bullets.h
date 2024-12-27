// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#ifndef __BULLETS_H
#define __BULLETS_H

#include "SGP/Types.h"
#include "Tactical/LOS.h"
#include "TileEngine/TileAnimation.h"
#include "rust_fileman.h"

struct SOLDIERTYPE;

#define BULLET_FLAG_CREATURE_SPIT 0x0001
#define BULLET_FLAG_KNIFE 0x0002
#define BULLET_FLAG_MISSILE 0x0004
#define BULLET_FLAG_SMALL_MISSILE 0x0008
#define BULLET_STOPPED 0x0010
#define BULLET_FLAG_TANK_CANNON 0x0020
#define BULLET_FLAG_BUCKSHOT 0x0040
#define BULLET_FLAG_FLAME 0x0080

typedef struct {
  int32_t iBullet;
  uint8_t ubFirerID;
  uint8_t ubTargetID;
  int8_t bStartCubesAboveLevelZ;
  int8_t bEndCubesAboveLevelZ;
  uint32_t sGridNo;
  int16_t sUnused;
  uint16_t usLastStructureHit;
  FIXEDPT qCurrX;
  FIXEDPT qCurrY;
  FIXEDPT qCurrZ;
  FIXEDPT qIncrX;
  FIXEDPT qIncrY;
  FIXEDPT qIncrZ;
  double ddHorizAngle;
  int32_t iCurrTileX;
  int32_t iCurrTileY;
  int8_t bLOSIndexX;
  int8_t bLOSIndexY;
  BOOLEAN fCheckForRoof;
  int32_t iCurrCubesZ;
  int32_t iLoop;
  BOOLEAN fAllocated;
  BOOLEAN fToDelete;
  BOOLEAN fLocated;
  BOOLEAN fReal;
  BOOLEAN fAimed;
  uint32_t uiLastUpdate;
  uint8_t ubTilesPerUpdate;
  uint16_t usClockTicksPerUpdate;
  struct SOLDIERTYPE *pFirer;
  uint32_t sTargetGridNo;
  int16_t sHitBy;
  int32_t iImpact;
  int32_t iImpactReduction;
  int32_t iRange;
  int32_t iDistanceLimit;
  uint16_t usFlags;
  ANITILE *pAniTile;
  ANITILE *pShadowAniTile;
  uint8_t ubItemStatus;

} BULLET;

extern uint32_t guiNumBullets;

int32_t CreateBullet(uint8_t ubFirer, BOOLEAN fFake, uint16_t usFlags);
void RemoveBullet(int32_t iBullet);
void StopBullet(int32_t iBullet);
void UpdateBullets();
BULLET *GetBulletPtr(int32_t iBullet);

void DeleteAllBullets();

void LocateBullet(int32_t iBulletIndex);

void HandleBulletSpecialFlags(int32_t iBulletIndex);

void AddMissileTrail(BULLET *pBullet, FIXEDPT qCurrX, FIXEDPT qCurrY, FIXEDPT qCurrZ);

// Save the bullet table to the saved game file
BOOLEAN SaveBulletStructureToSaveGameFile(FileID hFile);

// Load the bullet table from the saved game file
BOOLEAN LoadBulletStructureFromSavedGameFile(FileID hFile);

#endif
