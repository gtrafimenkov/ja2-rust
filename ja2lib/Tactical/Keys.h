#ifndef _KEYS_H_
#define _KEYS_H_

#include "SGP/Types.h"

struct OBJECTTYPE;
struct SOLDIERTYPE;

typedef struct {
  uint16_t usItem;         // index in item table for key
  uint8_t fFlags;          // flags...
  uint16_t usSectorFound;  // where and
  uint16_t usDateFound;    // when the key was found
} KEY;

#define KEY_USED 0x01

#define LOCK_UNOPENABLE 255
#define NO_KEY 255

#define MAX_KEYS_PER_LOCK 4

#define LOCK_REGULAR 1
#define LOCK_PADLOCK 2
#define LOCK_CARD 3
#define LOCK_ELECTRONIC 4
#define LOCK_SPECIAL 5

/*
typedef struct
{
        uint8_t		ubKeyID[MAX_KEYS_PER_LOCK];
        uint8_t		ubLockType;							// numeric
lock type value... easier to use than flags!
        int8_t		bPickSkillAdjustment;		// difficulty to pick a lock which takes
this key int8_t		bSmashSkillAdjustment;	// the strength of the lock (resistance to smashing)
} LOCK;
*/

#define MAXLOCKDESCLENGTH 40
typedef struct {
  uint8_t ubEditorName[MAXLOCKDESCLENGTH];  // name to display in editor
  uint16_t usKeyItem;                       // key for this door uses which graphic (item #)?
  uint8_t ubLockType;                       // regular, padlock, electronic, etc
  uint8_t ubPickDifficulty;                 // difficulty to pick such a lock
  uint8_t ubSmashDifficulty;                // difficulty to smash such a lock
  uint8_t ubFiller;
} LOCK;

// Defines below for the perceived value of the door
#define DOOR_PERCEIVED_UNKNOWN 0
#define DOOR_PERCEIVED_LOCKED 1
#define DOOR_PERCEIVED_UNLOCKED 2
#define DOOR_PERCEIVED_BROKEN 3

#define DOOR_PERCEIVED_TRAPPED 1
#define DOOR_PERCEIVED_UNTRAPPED 2

typedef struct {
  int16_t sGridNo;
  BOOLEAN fLocked;           // is the door locked
  uint8_t ubTrapLevel;       // difficulty of finding the trap, 0-10
  uint8_t ubTrapID;          // the trap type (0 is no trap)
  uint8_t ubLockID;          // the lock (0 is no lock)
  int8_t bPerceivedLocked;   // The perceived lock value can be different than the fLocked.
                             // Values for this include the fact that we don't know the status of
                             // the door, etc
  int8_t bPerceivedTrapped;  // See above, but with respect to traps rather than locked status
  int8_t bLockDamage;        // Damage to the lock
  int8_t bPadding[4];        // extra bytes
} DOOR;

typedef enum {
  NO_TRAP = 0,
  EXPLOSION,
  ELECTRIC,
  SIREN,
  SILENT_ALARM,
  BROTHEL_SIREN,
  SUPER_ELECTRIC,
  NUM_DOOR_TRAPS
} DoorTrapTypes;

#define DOOR_TRAP_STOPS_ACTION 0x01
#define DOOR_TRAP_RECURRING 0x02
#define DOOR_TRAP_SILENT 0x04

typedef struct {
  uint8_t fFlags;  // stops action?  recurring trap?
} DOORTRAP;

// The status of the door, either open or closed
#define DOOR_OPEN 0x01
#define DOOR_PERCEIVED_OPEN 0x02
#define DOOR_PERCEIVED_NOTSET 0x04
#define DOOR_BUSY 0x08
#define DOOR_HAS_TIN_CAN 0x10

#define DONTSETDOORSTATUS 2

typedef struct {
  int16_t sGridNo;
  uint8_t ubFlags;

} DOOR_STATUS;

// This is the number of different types of doors we can have
// in one map at a time...

#define NUM_KEYS 64
#define NUM_LOCKS 64
#define INVALID_KEY_NUMBER 255

#define ANYKEY 252
#define AUTOUNLOCK 253
#define OPENING_NOT_POSSIBLE 254

extern KEY KeyTable[NUM_KEYS];
extern LOCK LockTable[NUM_LOCKS];
extern DOORTRAP DoorTrapTable[NUM_DOOR_TRAPS];

extern BOOLEAN AddKeysToKeyRing(struct SOLDIERTYPE *pSoldier, uint8_t ubKeyID, uint8_t ubNumber);
extern BOOLEAN RemoveKeyFromKeyRing(struct SOLDIERTYPE *pSoldier, uint8_t ubPos,
                                    struct OBJECTTYPE *pObj);
extern BOOLEAN RemoveAllOfKeyFromKeyRing(struct SOLDIERTYPE *pSoldier, uint8_t ubPos,
                                         struct OBJECTTYPE *pObj);
extern BOOLEAN KeyExistsInInventory(struct SOLDIERTYPE *pSoldier, uint8_t ubKeyID);
extern BOOLEAN KeyExistsInKeyRing(struct SOLDIERTYPE *pSoldier, uint8_t ubKeyID, uint8_t *pubPos);
extern BOOLEAN SoldierHasKey(struct SOLDIERTYPE *pSoldier, uint8_t ubKeyID);

extern STR16 sKeyDescriptionStrings[];
/**********************************
 * Door utils add by Kris Morness *
 **********************************/

// Dynamic array of Doors.  For general game purposes, the doors that are locked and/or trapped
// are permanently saved within the map, and are loaded and allocated when the map is loaded.
// Because the editor allows more doors to be added, or removed, the actual size of the DoorTable
// may change.
extern DOOR *DoorTable;

// Current number of doors in world.
extern uint8_t gubNumDoors;
// Current max number of doors.  This is only used by the editor.  When adding doors to the
// world, we may run out of space in the DoorTable, so we will allocate a new array with extra
// slots, then copy everything over again.  gubMaxDoors holds the arrays actual number of slots,
// even though the current number (gubNumDoors) will be <= to it.
extern uint8_t gubMaxDoors;
// File I/O for loading the door information from the map.  This automatically allocates
// the exact number of slots when loading.

extern void LoadDoorTableFromMap(int8_t **hBuffer);
// Saves the existing door information to the map.  Before it actually saves, it'll verify that the
// door still exists.  Otherwise, it'll ignore it.  It is possible in the editor to delete doors in
// many different ways, so I opted to put it in the saving routine.
extern void SaveDoorTableToMap(HWFILE fp);
// The editor adds locks to the world.  If the gridno already exists, then the currently existing
// door information is overwritten.
extern void AddDoorInfoToTable(DOOR *pDoor);
// When the editor removes a door from the world, this function looks for and removes accompanying
// door information.  If the entry is not the last entry, the last entry is move to it's current
// slot, to keep everything contiguous.
extern void RemoveDoorInfoFromTable(INT32 iMapIndex);
// This is the link to see if a door exists at a gridno.
DOOR *FindDoorInfoAtGridNo(INT32 iMapIndex);
// Upon world deallocation, the door table needs to be deallocated.
extern void TrashDoorTable();

BOOLEAN AttemptToUnlockDoor(struct SOLDIERTYPE *pSoldier, DOOR *pDoor);
BOOLEAN AttemptToLockDoor(struct SOLDIERTYPE *pSoldier, DOOR *pDoor);
BOOLEAN AttemptToSmashDoor(struct SOLDIERTYPE *pSoldier, DOOR *pDoor);
BOOLEAN AttemptToPickLock(struct SOLDIERTYPE *pSoldier, DOOR *pDoor);
BOOLEAN AttemptToBlowUpLock(struct SOLDIERTYPE *pSoldier, DOOR *pDoor);
BOOLEAN AttemptToUntrapDoor(struct SOLDIERTYPE *pSoldier, DOOR *pDoor);
BOOLEAN ExamineDoorForTraps(struct SOLDIERTYPE *pSoldier, DOOR *pDoor);
BOOLEAN HasDoorTrapGoneOff(struct SOLDIERTYPE *pSoldier, DOOR *pDoor);
void HandleDoorTrap(struct SOLDIERTYPE *pSoldier, DOOR *pDoor);

// Updates the perceived value to the user of the state of the door
void UpdateDoorPerceivedValue(DOOR *pDoor);

// Saves the Door Table array to the temp file
BOOLEAN SaveDoorTableToDoorTableTempFile(u8 sSectorX, u8 sSectorY, int8_t bSectorZ);

// Load the door table from the temp file
BOOLEAN LoadDoorTableFromDoorTableTempFile();

//	Adds a door to the Door status array.  As the user comes across the door, they are added.
//  if the door already exists, nothing happens
// fOpen is True if the door is to be initially open, false if it is closed
// fInitiallyPercieveOpen is true if the door is to be initially open, else false
BOOLEAN ModifyDoorStatus(int16_t sGridNo, BOOLEAN fOpen, BOOLEAN fInitiallyPercieveOpen);

// Deletes the door status array
void TrashDoorStatusArray();

// Returns true if the door is open, otherwise false
BOOLEAN IsDoorOpen(int16_t sGridNo);

// Returns true if the door is perceioved as open
BOOLEAN IsDoorPerceivedOpen(int16_t sGridNo);

// Saves the Door Status array to the MapTempfile
BOOLEAN SaveDoorStatusArrayToDoorStatusTempFile(u8 sSectorX, u8 sSectorY, int8_t bSectorZ);

// Load the door status from the door status temp file
BOOLEAN LoadDoorStatusArrayFromDoorStatusTempFile();

// Modify the Doors open status
BOOLEAN SetDoorOpenStatus(int16_t sGridNo, BOOLEAN fOpen);

// Modify the doors perceived open status
BOOLEAN SetDoorPerceivedOpenStatus(int16_t sGridNo, BOOLEAN fPerceivedOpen);

// Save the key table to the saved game file
BOOLEAN SaveKeyTableToSaveGameFile(HWFILE hFile);

// Load the key table from the saved game file
BOOLEAN LoadKeyTableFromSaveedGameFile(HWFILE hFile);

// Returns a doors status value, NULL if not found
DOOR_STATUS *GetDoorStatus(int16_t sGridNo);

BOOLEAN UpdateDoorStatusPerceivedValue(int16_t sGridNo);

BOOLEAN AllMercsLookForDoor(int16_t sGridNo, BOOLEAN fUpdateValue);

BOOLEAN MercLooksForDoors(struct SOLDIERTYPE *pSoldier, BOOLEAN fUpdateValue);

void UpdateDoorGraphicsFromStatus(BOOLEAN fUsePerceivedStatus, BOOLEAN fDirty);

BOOLEAN AttemptToCrowbarLock(struct SOLDIERTYPE *pSoldier, DOOR *pDoor);

BOOLEAN LoadLockTable(void);

void ExamineDoorsOnEnteringSector();

void HandleDoorsChangeWhenEnteringSectorCurrentlyLoaded();

void AttachStringToDoor(int16_t sGridNo);

void DropKeysInKeyRing(struct SOLDIERTYPE *pSoldier, int16_t sGridNo, int8_t bLevel,
                       int8_t bVisible, BOOLEAN fAddToDropList, INT32 iDropListSlot,
                       BOOLEAN fUseUnLoaded);

#endif
