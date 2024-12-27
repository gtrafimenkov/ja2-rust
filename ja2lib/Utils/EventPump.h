#ifndef EVENT_PROCESSOR_H
#define EVENT_PROCESSOR_H

#include "SGP/Types.h"
#include "Utils/EventManager.h"

#define NETWORK_PATH_DATA_SIZE 6
// Enumerate all events for JA2
typedef enum {

  E_PLAYSOUND,
  S_CHANGEDEST,
  //	S_GETNEWPATH,
  S_BEGINTURN,
  S_CHANGESTANCE,
  S_SETDESIREDDIRECTION,
  S_BEGINFIREWEAPON,
  S_FIREWEAPON,
  S_WEAPONHIT,
  S_STRUCTUREHIT,
  S_WINDOWHIT,
  S_MISS,
  S_NOISE,
  S_STOP_MERC,

  EVENTS_LOCAL_AND_NETWORK,  // Events above here are sent locally and over network

  S_GETNEWPATH,
  S_SETPOSITION,
  S_CHANGESTATE,
  S_SETDIRECTION,
  EVENTS_ONLY_USED_LOCALLY,  // Events above are only used locally

  S_SENDPATHTONETWORK,
  S_UPDATENETWORKSOLDIER,
  EVENTS_ONLY_SENT_OVER_NETWORK,  // Events above are only sent to the network

  NUM_EVENTS

} eJA2Events;

// This definition is used to denote events with a special delay value;
// it indicates that these events will not be processed until specifically
// called for in a special loop.
#define DEMAND_EVENT_DELAY 0xFFFF

// Enumerate all structures for events
typedef struct {
  uint16_t usIndex;
  uint16_t usRate;
  uint8_t ubVolume;
  uint8_t ubLoops;
  uint32_t uiPan;

} EV_E_PLAYSOUND;

typedef struct {
  uint16_t usSoldierID;
  uint32_t uiUniqueId;
  uint16_t usNewState;
  INT16 sXPos;
  INT16 sYPos;
  uint16_t usStartingAniCode;
  BOOLEAN fForce;

} EV_S_CHANGESTATE;

typedef struct {
  uint16_t usSoldierID;
  uint32_t uiUniqueId;
  uint16_t usNewDestination;

} EV_S_CHANGEDEST;

typedef struct {
  uint16_t usSoldierID;
  uint32_t uiUniqueId;
  FLOAT dNewXPos;
  FLOAT dNewYPos;

} EV_S_SETPOSITION;

typedef struct {
  uint16_t usSoldierID;
  uint32_t uiUniqueId;
  INT16 sDestGridNo;
  uint16_t usMovementAnim;

} EV_S_GETNEWPATH;

typedef struct {
  uint16_t usSoldierID;
  uint32_t uiUniqueId;
} EV_S_BEGINTURN;

typedef struct {
  uint16_t usSoldierID;
  uint32_t uiUniqueId;
  uint8_t ubNewStance;
  INT16 sXPos;
  INT16 sYPos;

} EV_S_CHANGESTANCE;

typedef struct {
  uint16_t usSoldierID;
  uint32_t uiUniqueId;
  uint16_t usNewDirection;

} EV_S_SETDIRECTION;

typedef struct {
  uint16_t usSoldierID;
  uint32_t uiUniqueId;
  uint16_t usDesiredDirection;

} EV_S_SETDESIREDDIRECTION;

typedef struct {
  uint16_t usSoldierID;
  uint32_t uiUniqueId;
  INT16 sTargetGridNo;
  INT8 bTargetLevel;
  INT8 bTargetCubeLevel;
} EV_S_BEGINFIREWEAPON;

typedef struct {
  uint16_t usSoldierID;
  uint32_t uiUniqueId;
  INT16 sTargetGridNo;
  INT8 bTargetLevel;
  INT8 bTargetCubeLevel;
} EV_S_FIREWEAPON;

typedef struct {
  uint16_t usSoldierID;
  uint32_t uiUniqueId;
  uint16_t usWeaponIndex;
  INT16 sDamage;
  INT16 sBreathLoss;
  uint16_t usDirection;
  INT16 sXPos;
  INT16 sYPos;
  INT16 sZPos;
  INT16 sRange;
  uint8_t ubAttackerID;
  BOOLEAN fHit;
  uint8_t ubSpecial;
  uint8_t ubLocation;

} EV_S_WEAPONHIT;

typedef struct {
  INT16 sXPos;
  INT16 sYPos;
  INT16 sZPos;
  uint16_t usWeaponIndex;
  INT8 bWeaponStatus;
  uint8_t ubAttackerID;
  uint16_t usStructureID;
  INT32 iImpact;
  INT32 iBullet;

} EV_S_STRUCTUREHIT;

typedef struct {
  INT16 sGridNo;
  uint16_t usStructureID;
  BOOLEAN fBlowWindowSouth;
  BOOLEAN fLargeForce;
} EV_S_WINDOWHIT;

typedef struct {
  uint8_t ubAttackerID;
} EV_S_MISS;

typedef struct {
  uint8_t ubNoiseMaker;
  INT16 sGridNo;
  uint8_t bLevel;
  uint8_t ubTerrType;
  uint8_t ubVolume;
  uint8_t ubNoiseType;
} EV_S_NOISE;

typedef struct {
  uint16_t usSoldierID;
  uint32_t uiUniqueId;
  INT8 bDirection;
  INT16 sGridNo;
  INT16 sXPos;
  INT16 sYPos;

} EV_S_STOP_MERC;

typedef struct {
  uint8_t usSoldierID;
  uint32_t uiUniqueId;
  uint8_t usPathDataSize;      // Size of Path
  INT16 sAtGridNo;             // Owner merc is at this tile when sending packet
  uint8_t usCurrentPathIndex;  // Index the owner of the merc is at when sending packet
  uint8_t usPathData[NETWORK_PATH_DATA_SIZE];  // make define  // Next X tile to go to
  uint8_t ubNewState;                          // new movment Anim
  //	INT8		bActionPoints;
  //	INT8		bBreath;			// current breath value
  //	INT8		bDesiredDirection;

  // maybe send current action & breath points
} EV_S_SENDPATHTONETWORK;

typedef struct {
  uint8_t usSoldierID;
  uint32_t uiUniqueId;
  INT16 sAtGridNo;     // Owner merc is at this tile when sending packet
  INT8 bActionPoints;  // current A.P. value
  INT8 bBreath;        // current breath value
} EV_S_UPDATENETWORKSOLDIER;

// FUNCTIONS
BOOLEAN AddGameEvent(uint32_t uiEvent, uint16_t usDelay, PTR pEventData);
BOOLEAN AddGameEventFromNetwork(uint32_t uiEvent, uint16_t usDelay, PTR pEventData);
BOOLEAN DequeAllGameEvents(BOOLEAN fExecute);
BOOLEAN DequeueAllDemandGameEvents(BOOLEAN fExecute);

// clean out the evetn queue
BOOLEAN ClearEventQueue(void);

#endif
