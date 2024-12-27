#ifndef _ROTTING_CORPSES_H
#define _ROTTING_CORPSES_H

#include "SGP/Types.h"
#include "Tactical/SoldierControl.h"
#include "TileEngine/TileAnimation.h"

#define NUM_CORPSE_SHADES 17

typedef enum {
  NO_CORPSE,
  SMERC_JFK,
  SMERC_BCK,
  SMERC_FWD,
  SMERC_DHD,
  SMERC_PRN,
  SMERC_WTR,
  SMERC_FALL,
  SMERC_FALLF,

  MMERC_JFK,
  MMERC_BCK,
  MMERC_FWD,
  MMERC_DHD,
  MMERC_PRN,
  MMERC_WTR,
  MMERC_FALL,
  MMERC_FALLF,

  FMERC_JFK,
  FMERC_BCK,
  FMERC_FWD,
  FMERC_DHD,
  FMERC_PRN,
  FMERC_WTR,
  FMERC_FALL,
  FMERC_FALLF,

  // CIVS
  M_DEAD1,
  K_DEAD1,
  H_DEAD1,
  FT_DEAD1,
  S_DEAD1,
  W_DEAD1,
  C_DEAD1,
  M_DEAD2,
  K_DEAD2,
  H_DEAD2,

  FT_DEAD2,
  S_DEAD2,
  W_DEAD2,
  C_DEAD2,
  BLOODCAT_DEAD,
  COW_DEAD,
  ADULTMONSTER_DEAD,
  INFANTMONSTER_DEAD,
  LARVAEMONSTER_DEAD,
  ROTTING_STAGE2,

  TANK1_DEAD,
  TANK2_DEAD,
  HUMMER_DEAD,
  ICECREAM_DEAD,
  QUEEN_MONSTER_DEAD,
  ROBOT_DEAD,
  BURNT_DEAD,
  EXPLODE_DEAD,

  NUM_CORPSES,

} RottingCorpseDefines;

#define ROTTING_CORPSE_FIND_SWEETSPOT_FROM_GRIDNO 0x01  // Find the closest spot to the given gridno
#define ROTTING_CORPSE_USE_NORTH_ENTRY_POINT 0x02  // Find the spot closest to the north entry grid
#define ROTTING_CORPSE_USE_SOUTH_ENTRY_POINT 0x04  // Find the spot closest to the south entry grid
#define ROTTING_CORPSE_USE_EAST_ENTRY_POINT 0x08   // Find the spot closest to the east entry grid
#define ROTTING_CORPSE_USE_WEST_ENTRY_POINT 0x10   // Find the spot closest to the west entry grid
#define ROTTING_CORPSE_USE_CAMMO_PALETTE 0x20      // We use cammo palette here....
#define ROTTING_CORPSE_VEHICLE 0x40                // Vehicle Corpse

typedef struct {
  uint8_t ubType;
  uint8_t ubBodyType;
  int16_t sGridNo;
  FLOAT dXPos;
  FLOAT dYPos;
  int16_t sHeightAdjustment;

  PaletteRepID HeadPal;  // Palette reps
  PaletteRepID PantsPal;
  PaletteRepID VestPal;
  PaletteRepID SkinPal;

  int8_t bDirection;
  uint32_t uiTimeOfDeath;

  uint16_t usFlags;

  int8_t bLevel;

  int8_t bVisible;
  int8_t bNumServicingCrows;
  uint8_t ubProfile;
  BOOLEAN fHeadTaken;
  uint8_t ubAIWarningValue;

  uint8_t ubFiller[12];

} ROTTING_CORPSE_DEFINITION;

typedef struct {
  ROTTING_CORPSE_DEFINITION def;
  BOOLEAN fActivated;

  ANITILE *pAniTile;

  struct SGPPaletteEntry *p8BPPPalette;
  uint16_t *p16BPPPalette;
  uint16_t *pShades[NUM_CORPSE_SHADES];
  int16_t sGraphicNum;
  INT32 iCachedTileID;
  FLOAT dXPos;
  FLOAT dYPos;

  BOOLEAN fAttractCrowsOnlyWhenOnScreen;
  INT32 iID;

} ROTTING_CORPSE;

INT32 AddRottingCorpse(ROTTING_CORPSE_DEFINITION *pCorpseDef);

void RemoveCorpse(INT32 iCorpseID);
void RemoveCorpses();

BOOLEAN TurnSoldierIntoCorpse(struct SOLDIERTYPE *pSoldier, BOOLEAN fRemoveMerc,
                              BOOLEAN fCheckForLOS);

int16_t FindNearestRottingCorpse(struct SOLDIERTYPE *pSoldier);

void AllMercsOnTeamLookForCorpse(ROTTING_CORPSE *pCorpse, int8_t bTeam);
void MercLooksForCorpses(struct SOLDIERTYPE *pSoldier);
void RebuildAllCorpseShadeTables();

uint16_t CreateCorpsePaletteTables(ROTTING_CORPSE *pCorpse);

int16_t FindNearestAvailableGridNoForCorpse(ROTTING_CORPSE_DEFINITION *pCorpseDef, int8_t ubRadius);

void HandleRottingCorpses();
void AddCrowToCorpse(ROTTING_CORPSE *pCorpse);

void VaporizeCorpse(int16_t sGridNo, uint16_t usStructureID);
void CorpseHit(int16_t sGridNo, uint16_t usStructureID);

void HandleCrowLeave(struct SOLDIERTYPE *pSoldier);

void HandleCrowFlyAway(struct SOLDIERTYPE *pSoldier);

#define MAX_ROTTING_CORPSES 100

extern ROTTING_CORPSE gRottingCorpse[MAX_ROTTING_CORPSES];
extern INT32 giNumRottingCorpse;
extern uint8_t gb4DirectionsFrom8[8];

ROTTING_CORPSE *GetCorpseAtGridNo(int16_t sGridNo, int8_t bLevel);
BOOLEAN IsValidDecapitationCorpse(ROTTING_CORPSE *pCorpse);
void DecapitateCorpse(struct SOLDIERTYPE *pSoldier, int16_t sGridNo, int8_t bLevel);

void GetBloodFromCorpse(struct SOLDIERTYPE *pSoldier);

uint16_t GetCorpseStructIndex(ROTTING_CORPSE_DEFINITION *pCorpseDef, BOOLEAN fForImage);

void LookForAndMayCommentOnSeeingCorpse(struct SOLDIERTYPE *pSoldier, int16_t sGridNo,
                                        uint8_t ubLevel);

int16_t GetGridNoOfCorpseGivenProfileID(uint8_t ubProfileID);

void DecayRottingCorpseAIWarnings(void);
uint8_t GetNearestRottingCorpseAIWarning(int16_t sGridNo);

#endif
