#ifndef STRUCTURE_H
#define STRUCTURE_H

#include "Tactical/OverheadTypes.h"

struct DB_STRUCTURE_REF;
struct LEVELNODE;
struct STRUCTURE;
struct VObject;

#define NOTHING_BLOCKING 0
#define BLOCKING_REDUCE_RANGE 1
#define BLOCKING_NEXT_TILE 10
#define BLOCKING_TOPLEFT_WINDOW 30
#define BLOCKING_TOPRIGHT_WINDOW 40
#define BLOCKING_TOPLEFT_DOOR 50
#define BLOCKING_TOPRIGHT_DOOR 60
#define FULL_BLOCKING 70
#define BLOCKING_TOPLEFT_OPEN_WINDOW 90
#define BLOCKING_TOPRIGHT_OPEN_WINDOW 100

// ATE: Increased to allow corpses to not collide with soldiers
// 100 == MAX_CORPSES
#define INVALID_STRUCTURE_ID (TOTAL_SOLDIERS + 100)
#define IGNORE_PEOPLE_STRUCTURE_ID (TOTAL_SOLDIERS + 101)

#define STRUCTURE_DAMAGE_EXPLOSION 1
#define STRUCTURE_DAMAGE_GUNFIRE 2

// functions at the structure database level
//
struct STRUCTURE_FILE_REF *LoadStructureFile(STR szFileName);
void FreeAllStructureFiles(void);
BOOLEAN FreeStructureFile(struct STRUCTURE_FILE_REF *pStructureFile);

//
// functions at the structure instance level
//
BOOLEAN OkayToAddStructureToWorld(INT16 sBaseGridNo, int8_t bLevel,
                                  struct DB_STRUCTURE_REF *pDBStructureRef, INT16 sExclusionID);

// for the PTR argument of AddStructureToWorld, pass in a struct LEVELNODE * please!
BOOLEAN AddStructureToWorld(INT16 sBaseGridNo, int8_t bLevel,
                            struct DB_STRUCTURE_REF *pDBStructureRef, PTR pLevelN);
BOOLEAN DeleteStructureFromWorld(struct STRUCTURE *pStructure);

//
// functions to find a structure in a location
//
struct STRUCTURE *FindStructure(INT16 sGridNo, uint32_t fFlags);
struct STRUCTURE *FindNextStructure(struct STRUCTURE *pStructure, uint32_t fFlags);
struct STRUCTURE *FindStructureByID(INT16 sGridNo, uint16_t usStructureID);
struct STRUCTURE *FindBaseStructure(struct STRUCTURE *pStructure);
struct STRUCTURE *FindNonBaseStructure(INT16 sGridNo, struct STRUCTURE *pStructure);

//
// functions related to interactive tiles
//
struct STRUCTURE *SwapStructureForPartner(INT16 sGridNo, struct STRUCTURE *pStructure);
struct STRUCTURE *SwapStructureForPartnerWithoutTriggeringSwitches(INT16 sGridNo,
                                                                   struct STRUCTURE *pStructure);
struct STRUCTURE *SwapStructureForPartnerAndStoreChangeInMap(INT16 sGridNo,
                                                             struct STRUCTURE *pStructure);
//
// functions useful for AI that return info about heights
//
int8_t StructureHeight(struct STRUCTURE *pStructure);
int8_t StructureBottomLevel(struct STRUCTURE *pStructure);
int8_t GetTallestStructureHeight(INT16 sGridNo, BOOLEAN fOnRoof);
int8_t GetStructureTargetHeight(INT16 sGridNo, BOOLEAN fOnRoof);

BOOLEAN StructureDensity(struct STRUCTURE *pStructure, uint8_t *pubLevel0, uint8_t *pubLevel1,
                         uint8_t *pubLevel2, uint8_t *pubLevel3);

BOOLEAN FindAndSwapStructure(INT16 sGridNo);
INT16 GetBaseTile(struct STRUCTURE *pStructure);

void DebugStructurePage1(void);

BOOLEAN AddZStripInfoToVObject(struct VObject *hVObject,
                               struct STRUCTURE_FILE_REF *pStructureFileRef, BOOLEAN fFromAnimation,
                               INT16 sSTIStartIndex);

// FUNCTIONS FOR DETERMINING STUFF THAT BLOCKS VIEW FOR TILE_bASED LOS
int8_t GetBlockingStructureInfo(INT16 sGridNo, int8_t bDir, int8_t bNextDir, int8_t bLevel,
                                int8_t *pStructHeight, struct STRUCTURE **ppTallestStructure,
                                BOOLEAN fWallsBlock);

BOOLEAN DamageStructure(struct STRUCTURE *pStructure, uint8_t ubDamage, uint8_t ubReason,
                        INT16 sGridNo, INT16 sX, INT16 sY, uint8_t ubOwner);

// Material armour type enumeration
enum {
  MATERIAL_NOTHING,
  MATERIAL_WOOD_WALL,
  MATERIAL_PLYWOOD_WALL,
  MATERIAL_LIVE_WOOD,
  MATERIAL_LIGHT_VEGETATION,
  MATERIAL_FURNITURE,
  MATERIAL_PORCELAIN,
  MATERIAL_CACTUS,
  MATERIAL_NOTUSED1,
  MATERIAL_NOTUSED2,

  MATERIAL_NOTUSED3,
  MATERIAL_STONE,
  MATERIAL_CONCRETE1,
  MATERIAL_CONCRETE2,
  MATERIAL_ROCK,
  MATERIAL_RUBBER,
  MATERIAL_SAND,
  MATERIAL_CLOTH,
  MATERIAL_SANDBAG,
  MATERIAL_NOTUSED5,

  MATERIAL_NOTUSED6,
  MATERIAL_LIGHT_METAL,
  MATERIAL_THICKER_METAL,
  MATERIAL_HEAVY_METAL,
  MATERIAL_INDESTRUCTABLE_STONE,
  MATERIAL_INDESTRUCTABLE_METAL,
  MATERIAL_THICKER_METAL_WITH_SCREEN_WINDOWS,
  NUM_MATERIAL_TYPES
};

extern INT32 guiMaterialHitSound[NUM_MATERIAL_TYPES];

struct STRUCTURE *FindStructureBySavedInfo(INT16 sGridNo, uint8_t ubType, uint8_t ubWallOrientation,
                                           int8_t bLevel);
uint32_t StructureTypeToFlag(uint8_t ubType);
uint8_t StructureFlagToType(uint32_t uiFlag);

uint32_t GetStructureOpenSound(struct STRUCTURE *pStructure, BOOLEAN fClose);

#endif
