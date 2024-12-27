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
  int16_t sXPos;
  int16_t sYPos;
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
  int16_t sDestGridNo;
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
  int16_t sXPos;
  int16_t sYPos;

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
  int16_t sTargetGridNo;
  int8_t bTargetLevel;
  int8_t bTargetCubeLevel;
} EV_S_BEGINFIREWEAPON;

typedef struct {
  uint16_t usSoldierID;
  uint32_t uiUniqueId;
  int16_t sTargetGridNo;
  int8_t bTargetLevel;
  int8_t bTargetCubeLevel;
} EV_S_FIREWEAPON;

typedef struct {
  uint16_t usSoldierID;
  uint32_t uiUniqueId;
  uint16_t usWeaponIndex;
  int16_t sDamage;
  int16_t sBreathLoss;
  uint16_t usDirection;
  int16_t sXPos;
  int16_t sYPos;
  int16_t sZPos;
  int16_t sRange;
  uint8_t ubAttackerID;
  BOOLEAN fHit;
  uint8_t ubSpecial;
  uint8_t ubLocation;

} EV_S_WEAPONHIT;

typedef struct {
  int16_t sXPos;
  int16_t sYPos;
  int16_t sZPos;
  uint16_t usWeaponIndex;
  int8_t bWeaponStatus;
  uint8_t ubAttackerID;
  uint16_t usStructureID;
  int32_t iImpact;
  int32_t iBullet;

} EV_S_STRUCTUREHIT;

typedef struct {
  int16_t sGridNo;
  uint16_t usStructureID;
  BOOLEAN fBlowWindowSouth;
  BOOLEAN fLargeForce;
} EV_S_WINDOWHIT;

typedef struct {
  uint8_t ubAttackerID;
} EV_S_MISS;

typedef struct {
  uint8_t ubNoiseMaker;
  int16_t sGridNo;
  uint8_t bLevel;
  uint8_t ubTerrType;
  uint8_t ubVolume;
  uint8_t ubNoiseType;
} EV_S_NOISE;

typedef struct {
  uint16_t usSoldierID;
  uint32_t uiUniqueId;
  int8_t bDirection;
  int16_t sGridNo;
  int16_t sXPos;
  int16_t sYPos;

} EV_S_STOP_MERC;

typedef struct {
  uint8_t usSoldierID;
  uint32_t uiUniqueId;
  uint8_t usPathDataSize;      // Size of Path
  int16_t sAtGridNo;           // Owner merc is at this tile when sending packet
  uint8_t usCurrentPathIndex;  // Index the owner of the merc is at when sending packet
  uint8_t usPathData[NETWORK_PATH_DATA_SIZE];  // make define  // Next X tile to go to
  uint8_t ubNewState;                          // new movment Anim
  //	int8_t		bActionPoints;
  //	int8_t		bBreath;			// current breath value
  //	int8_t		bDesiredDirection;

  // maybe send current action & breath points
} EV_S_SENDPATHTONETWORK;

typedef struct {
  uint8_t usSoldierID;
  uint32_t uiUniqueId;
  int16_t sAtGridNo;     // Owner merc is at this tile when sending packet
  int8_t bActionPoints;  // current A.P. value
  int8_t bBreath;        // current breath value
} EV_S_UPDATENETWORKSOLDIER;

// FUNCTIONS
BOOLEAN AddGameEvent(uint32_t uiEvent, uint16_t usDelay, void* pEventData);
BOOLEAN AddGameEventFromNetwork(uint32_t uiEvent, uint16_t usDelay, void* pEventData);
BOOLEAN DequeAllGameEvents(BOOLEAN fExecute);
BOOLEAN DequeueAllDemandGameEvents(BOOLEAN fExecute);

// clean out the evetn queue
BOOLEAN ClearEventQueue(void);

#endif
