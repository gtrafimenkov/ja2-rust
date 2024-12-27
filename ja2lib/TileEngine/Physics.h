#ifndef __PHYSICS_H
#define __PHYSICS_H

#include "SGP/Types.h"
#include "Tactical/Items.h"
#include "TileEngine/PhysMath.h"

struct SOLDIERTYPE;

extern uint32_t guiNumObjectSlots;

typedef struct {
  BOOLEAN fAllocated;
  BOOLEAN fAlive;
  BOOLEAN fApplyFriction;
  BOOLEAN fColliding;
  BOOLEAN fZOnRest;
  BOOLEAN fVisible;
  BOOLEAN fInWater;
  BOOLEAN fTestObject;
  BOOLEAN fTestEndedWithCollision;
  BOOLEAN fTestPositionNotSet;

  real TestZTarget;
  real OneOverMass;
  real AppliedMu;

  vector_3 Position;
  vector_3 TestTargetPosition;
  vector_3 OldPosition;
  vector_3 Velocity;
  vector_3 OldVelocity;
  vector_3 InitialForce;
  vector_3 Force;
  vector_3 CollisionNormal;
  vector_3 CollisionVelocity;
  real CollisionElasticity;

  int16_t sGridNo;
  INT32 iID;
  struct LEVELNODE *pNode;
  struct LEVELNODE *pShadow;

  int16_t sConsecutiveCollisions;
  int16_t sConsecutiveZeroVelocityCollisions;
  INT32 iOldCollisionCode;

  FLOAT dLifeLength;
  FLOAT dLifeSpan;
  struct OBJECTTYPE Obj;
  BOOLEAN fFirstTimeMoved;
  int16_t sFirstGridNo;
  uint8_t ubOwner;
  uint8_t ubActionCode;
  uint32_t uiActionData;
  BOOLEAN fDropItem;
  uint32_t uiNumTilesMoved;
  BOOLEAN fCatchGood;
  BOOLEAN fAttemptedCatch;
  BOOLEAN fCatchAnimOn;
  BOOLEAN fCatchCheckDone;
  BOOLEAN fEndedWithCollisionPositionSet;
  vector_3 EndedWithCollisionPosition;
  BOOLEAN fHaveHitGround;
  BOOLEAN fPotentialForDebug;
  int16_t sLevelNodeGridNo;
  INT32 iSoundID;
  uint8_t ubLastTargetTakenDamage;
  uint8_t ubPadding[1];

} REAL_OBJECT;

#define NUM_OBJECT_SLOTS 50

extern REAL_OBJECT ObjectSlots[NUM_OBJECT_SLOTS];

// OBJECT LIST STUFF
INT32 CreatePhysicalObject(struct OBJECTTYPE *pGameObj, real dLifeLength, real xPos, real yPos,
                           real zPos, real xForce, real yForce, real zForce, uint8_t ubOwner,
                           uint8_t ubActionCode, uint32_t uiActionData);
BOOLEAN RemoveObjectSlot(INT32 iObject);
void RemoveAllPhysicsObjects();

FLOAT CalculateLaunchItemAngle(struct SOLDIERTYPE *pSoldier, int16_t sGridNo, uint8_t ubHeight,
                               real dForce, struct OBJECTTYPE *pItem, int16_t *psGridNo);

BOOLEAN CalculateLaunchItemChanceToGetThrough(struct SOLDIERTYPE *pSoldier,
                                              struct OBJECTTYPE *pItem, int16_t sGridNo,
                                              uint8_t ubLevel, int16_t sEndZ,
                                              int16_t *psFinalGridNo, BOOLEAN fArmed,
                                              int8_t *pbLevel, BOOLEAN fFromUI);

void CalculateLaunchItemParamsForThrow(struct SOLDIERTYPE *pSoldier, int16_t sGridNo,
                                       uint8_t ubLevel, int16_t sZPos, struct OBJECTTYPE *pItem,
                                       int8_t bMissBy, uint8_t ubActionCode, uint32_t uiActionData);

// SIMULATE WORLD
void SimulateWorld();

BOOLEAN SavePhysicsTableToSaveGameFile(HWFILE hFile);

BOOLEAN LoadPhysicsTableFromSavedGameFile(HWFILE hFile);

#endif
