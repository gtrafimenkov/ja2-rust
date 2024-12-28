// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#ifndef _TILE_ANIMATION_H
#define _TILE_ANIMATION_H

#include "SGP/Types.h"

#define ANITILE_DOOR 0x00000001
#define ANITILE_BACKWARD 0x00000020
#define ANITILE_FORWARD 0x00000040
#define ANITILE_PAUSED 0x00000200
#define ANITILE_EXISTINGTILE 0x00000400
#define ANITILE_USEABSOLUTEPOS 0x00004000
#define ANITILE_CACHEDTILE 0x00008000
#define ANITILE_LOOPING 0x00020000
#define ANITILE_NOZBLITTER 0x00040000
#define ANITILE_REVERSE_LOOPING 0x00080000
#define ANITILE_ALWAYS_TRANSLUCENT 0x00100000
#define ANITILE_USEBEST_TRANSLUCENT 0x00200000
#define ANITILE_OPTIMIZEFORSLOWMOVING 0x00400000
#define ANITILE_ANIMATE_Z 0x00800000
#define ANITILE_USE_DIRECTION_FOR_START_FRAME 0x01000000
#define ANITILE_PAUSE_AFTER_LOOP 0x02000000
#define ANITILE_ERASEITEMFROMSAVEBUFFFER 0x04000000
#define ANITILE_OPTIMIZEFORSMOKEEFFECT 0x08000000
#define ANITILE_SMOKE_EFFECT 0x10000000
#define ANITILE_EXPLOSION 0x20000000
#define ANITILE_RELEASE_ATTACKER_WHEN_DONE 0x40000000
#define ANITILE_USE_4DIRECTION_FOR_START_FRAME 0x02000000

#define ANI_LAND_LEVEL 1
#define ANI_SHADOW_LEVEL 2
#define ANI_OBJECT_LEVEL 3
#define ANI_STRUCT_LEVEL 4
#define ANI_ROOF_LEVEL 5
#define ANI_ONROOF_LEVEL 6
#define ANI_TOPMOST_LEVEL 7

typedef struct TAG_anitile {
  struct TAG_anitile *pNext;
  uint32_t uiFlags;           // flags struct
  uint32_t uiTimeLastUpdate;  // Stuff for animated tiles

  struct LEVELNODE *pLevelNode;
  uint8_t ubLevelID;
  int16_t sCurrentFrame;
  int16_t sStartFrame;
  int16_t sDelay;
  uint16_t usTileType;
  uint16_t usNumFrames;

  uint16_t usMissAnimationPlayed;
  uint16_t ubAttackerMissed;
  int16_t sRelativeX;
  int16_t sRelativeY;
  int16_t sRelativeZ;
  int16_t sGridNo;
  uint16_t usTileIndex;

  uint16_t usCachedTileSubIndex;  // sub Index
  int16_t sCachedTileID;          // Index into cached tile ID

  uint8_t ubOwner;
  uint8_t ubKeyFrame1;
  uint32_t uiKeyFrame1Code;
  uint8_t ubKeyFrame2;
  uint32_t uiKeyFrame2Code;

  uint32_t uiUserData;
  uint8_t ubUserData2;
  uint32_t uiUserData3;

  int8_t bFrameCountAfterStart;

} ANITILE;

typedef struct TAG_anitile_params {
  uint32_t uiFlags;      // flags struct
  uint8_t ubLevelID;     // Level ID for rendering layer
  int16_t sStartFrame;   // Start frame
  int16_t sDelay;        // Delay time
  uint16_t usTileType;   // Tile databse type ( optional )
  uint16_t usTileIndex;  // Tile database index ( optional )
  int16_t sX;            // World X ( optional )
  int16_t sY;            // World Y ( optional )
  int16_t sZ;            // World Z ( optional )
  int16_t sGridNo;       // World GridNo

  struct LEVELNODE *pGivenLevelNode;  // Levelnode for existing tile ( optional )
  char zCachedFile[100];              // Filename for cached tile name ( optional )

  uint8_t ubOwner;           // UBID for the owner
  uint8_t ubKeyFrame1;       // Key frame 1
  uint32_t uiKeyFrame1Code;  // Key frame code
  uint8_t ubKeyFrame2;       // Key frame 2
  uint32_t uiKeyFrame2Code;  // Key frame code

  uint32_t uiUserData;
  uint8_t ubUserData2;
  uint32_t uiUserData3;

} ANITILE_PARAMS;

typedef enum {
  ANI_KEYFRAME_NO_CODE,
  ANI_KEYFRAME_BEGIN_TRANSLUCENCY,
  ANI_KEYFRAME_BEGIN_DAMAGE,
  ANI_KEYFRAME_CHAIN_WATER_EXPLOSION,
  ANI_KEYFRAME_DO_SOUND,

} KeyFrameEnums;

// ANimation tile data
extern ANITILE *pAniTileHead;

ANITILE *CreateAnimationTile(ANITILE_PARAMS *pAniParams);

void DeleteAniTile(ANITILE *pAniTile);
void UpdateAniTiles();
void SetAniTileFrame(ANITILE *pAniTile, int16_t sFrame);
void DeleteAniTiles();

void HideAniTile(ANITILE *pAniTile, BOOLEAN fHide);
void PauseAniTile(ANITILE *pAniTile, BOOLEAN fPause);

ANITILE *GetCachedAniTileOfType(int16_t sGridNo, uint8_t ubLevelID, uint32_t uiFlags);

void PauseAllAniTilesOfType(uint32_t uiType, BOOLEAN fPause);

#endif
