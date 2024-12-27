#ifndef __STRUCTURE_INTERNAL_H
#define __STRUCTURE_INTERNAL_H

//
// If you wish to use the structure database functions, include
// structure_extern.h, not structure.h!
//

#include "SGP/HImage.h"
#include "SGP/Types.h"

// A few words about the overall structure scheme:
//
// Large structures are split into multiple sections,
// one for each tile.
//
// Each section is treated as a separate object,
// except that it does NOT record information about
// hit points, but instead stores a pointer to the
// base object (section).
//
// Each section has a line of sight profile.  These
// profiles are split into 5 in each horizontal direction
// and 4 vertically, forming 100 "cubes".  In real
// world terms, each section represents a volume
// with a height of 8 feet (and width and length
// of what?)
//
// It is important to note that the vertical
// position of each section is measured in individual
// cubes (rather than, as it were, groups of 4 vertical
// cubes)

#define PROFILE_X_SIZE 5
#define PROFILE_Y_SIZE 5
#define PROFILE_Z_SIZE 4

// these values should be compared for less than rather than less
// than or equal to
#define STRUCTURE_ON_GROUND 0
#define STRUCTURE_ON_ROOF PROFILE_Z_SIZE
#define STRUCTURE_ON_GROUND_MAX PROFILE_Z_SIZE
#define STRUCTURE_ON_ROOF_MAX PROFILE_Z_SIZE * 2

typedef uint8_t PROFILE[PROFILE_X_SIZE][PROFILE_Y_SIZE];

extern uint8_t AtHeight[PROFILE_Z_SIZE];

// MAP_ELEMENT may get later:
// PROFILE *		CombinedLOSProfile;
// PROFILE *		CombinedProtectionProfile;
//
// struct LEVELNODE gets a pointer to a struct STRUCTURE or
// a union between its soldier pointer and a
// struct STRUCTURE pointer

// if (fFlags & STRUCTURE_BASE_TILE)
// then the structure is the "base" of the object
// and its hitpoint value is the one for the object
//
//															        vv
// generic flags for all structures
// vvv      type flags
//

// how to handle explodable structures

// NOT used in DB structures!
#define STRUCTURE_BASE_TILE 0x00000001
#define STRUCTURE_OPEN 0x00000002
#define STRUCTURE_OPENABLE 0x00000004
// synonyms for STRUCTURE_OPENABLE
#define STRUCTURE_CLOSEABLE 0x00000004
#define STRUCTURE_SEARCHABLE 0x00000004
#define STRUCTURE_HIDDEN 0x00000008

#define STRUCTURE_MOBILE 0x00000010
// STRUCTURE_PASSABLE is set for each structure instance where
// the tile flag TILE_PASSABLE is set
#define STRUCTURE_PASSABLE 0x00000020
#define STRUCTURE_EXPLOSIVE 0x00000040
#define STRUCTURE_TRANSPARENT 0x00000080

#define STRUCTURE_GENERIC 0x00000100
#define STRUCTURE_TREE 0x00000200
#define STRUCTURE_FENCE 0x00000400
#define STRUCTURE_WIREFENCE 0x00000800

#define STRUCTURE_HASITEMONTOP 0x00001000  // ATE: HASITEM: struct has item on top of it
#define STRUCTURE_SPECIAL 0x00002000
#define STRUCTURE_LIGHTSOURCE 0x00004000
#define STRUCTURE_VEHICLE 0x00008000

#define STRUCTURE_WALL 0x00010000
#define STRUCTURE_WALLNWINDOW 0x00020000
#define STRUCTURE_SLIDINGDOOR 0x00040000
#define STRUCTURE_DOOR 0x00080000

// a "multi" structure (as opposed to multitiled) is composed of multiple graphics & structures
#define STRUCTURE_MULTI 0x00100000
#define STRUCTURE_CAVEWALL 0x00200000
#define STRUCTURE_DDOOR_LEFT 0x00400000
#define STRUCTURE_DDOOR_RIGHT 0x00800000

#define STRUCTURE_NORMAL_ROOF 0x01000000
#define STRUCTURE_SLANTED_ROOF 0x02000000
#define STRUCTURE_TALL_ROOF 0x04000000
#define STRUCTURE_SWITCH 0x08000000

#define STRUCTURE_ON_LEFT_WALL 0x10000000
#define STRUCTURE_ON_RIGHT_WALL 0x20000000
#define STRUCTURE_CORPSE 0x40000000
#define STRUCTURE_PERSON 0x80000000

// COMBINATION FLAGS
#define STRUCTURE_ANYFENCE 0x00000C00
#define STRUCTURE_ANYDOOR 0x00CC0000
#define STRUCTURE_OBSTACLE 0x00008F00
#define STRUCTURE_WALLSTUFF 0x00CF0000
#define STRUCTURE_BLOCKSMOVES 0x00208F00
#define STRUCTURE_TYPE_DEFINED 0x8FEF8F00
#define STRUCTURE_ROOF 0x07000000

#define TILE_ON_ROOF 0x01
#define TILE_PASSABLE 0x02

typedef struct TAG_STRUCTURE_TILE {
  int16_t sPosRelToBase;  // "single-axis"
  int8_t bXPosRelToBase;
  int8_t bYPosRelToBase;
  PROFILE Shape;  // 25 bytes
  uint8_t fFlags;
  uint8_t ubVehicleHitLocation;
  BYTE bUnused[1];
} DB_STRUCTURE_TILE;  // 32 bytes

#define BASE_TILE 0

#define NO_PARTNER_STRUCTURE 0

struct DB_STRUCTURE {
  uint8_t ubArmour;
  uint8_t ubHitPoints;
  uint8_t ubDensity;
  uint8_t ubNumberOfTiles;
  uint32_t fFlags;
  uint16_t usStructureNumber;
  uint8_t ubWallOrientation;
  int8_t bDestructionPartner;  // >0 = debris number (bDP - 1), <0 = partner graphic
  int8_t bPartnerDelta;        // opened/closed version, etc... 0 for unused
  int8_t bZTileOffsetX;
  int8_t bZTileOffsetY;
  BYTE bUnused[1];
};  // 16 bytes

struct DB_STRUCTURE_REF {
  struct DB_STRUCTURE* pDBStructure;
  DB_STRUCTURE_TILE** ppTile;  // dynamic array
};  // 8 bytes

struct STRUCTURE {
  struct STRUCTURE* pPrev;
  struct STRUCTURE* pNext;
  int16_t sGridNo;
  uint16_t usStructureID;
  struct DB_STRUCTURE_REF* pDBStructureRef;
  union {
    struct {
      uint8_t ubHitPoints;
      uint8_t ubLockStrength;
    };
    struct {
      int16_t sBaseGridNo;
    };
  };  // 2 bytes
  int16_t sCubeOffset;  // height of bottom of object in profile "cubes"
  uint32_t fFlags;      // need to have something to indicate base tile/not
  PROFILE* pShape;
  uint8_t ubWallOrientation;
  uint8_t ubVehicleHitLocation;
  uint8_t ubStructureHeight;  // if 0, then unset; otherwise stores height of structure when last
                              // calculated
  uint8_t ubUnused[1];
};  // 32 bytes

struct STRUCTURE_FILE_REF {
  struct STRUCTURE_FILE_REF* pPrev;
  struct STRUCTURE_FILE_REF* pNext;
  struct AuxObjectData* pAuxData;
  struct RelTileLoc* pTileLocData;
  uint8_t* pubStructureData;
  struct DB_STRUCTURE_REF* pDBStructureRef;  // dynamic array
  uint16_t usNumberOfStructures;
  uint16_t usNumberOfStructuresStored;
};  // 24 bytes

// IMPORTANT THING TO REMEMBER
//
// Although the number of structures and images about which information
// may be stored in a file, the two are stored very differently.
//
// The structure data stored amounts to a sparse array, with no data
// saved for any structures that are not defined.
//
// For image information, however, an array is stored with every entry
// filled regardless of whether there is non-zero data defined for
// that graphic!
typedef struct TAG_STRUCTURE_FILE_HEADER {
  CHAR8 szId[4];
  union {
    struct {
      uint16_t usNumberOfStructures;
    };
    struct {
      uint16_t usNumberOfImages;
    };
  };
  uint16_t usNumberOfStructuresStored;
  uint16_t usStructureDataSize;
  uint8_t fFlags;
  uint8_t bUnused[3];
  uint16_t usNumberOfImageTileLocsStored;
} STRUCTURE_FILE_HEADER;  // 16 bytes

// "J2SD" = Jagged 2 Structure Data
#define STRUCTURE_FILE_ID "J2SD"
#define STRUCTURE_FILE_ID_LEN 4

#define STRUCTURE_SCRIPT_FILE_EXTENSION "JSS"
#define STRUCTURE_FILE_EXTENSION "JSD"

#define STRUCTURE_FILE_CONTAINS_AUXIMAGEDATA 0x01
#define STRUCTURE_FILE_CONTAINS_STRUCTUREDATA 0x02

#endif
