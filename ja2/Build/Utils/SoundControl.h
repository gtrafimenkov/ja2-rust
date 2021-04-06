#ifndef SOUND_CONTROL_H
#define SOUND_CONTROL_H

#define FARLEFT 0
#define LEFTSIDE 48
#define MIDDLE 64
#define MIDDLEPAN 64
#define RIGHTSIDE 80
#define FARRIGHT 127

#define LOWVOLUME 25
#define BTNVOLUME 40
#define MIDVOLUME 65
#define HIGHVOLUME 127

#define RATE_11025 0xffffffff

#define LOOPING 0

// sound group priorities (higher = more important)
#define GROUP_PLAYER 1000
#define GROUP_AMBIENT 0

// SOUNDS ENUMERATION
enum SoundDefines {
  MISS_1 = 0,
  MISS_2,
  MISS_3,
  MISS_4,
  MISS_5,
  MISS_6,
  MISS_7,
  MISS_8,
  MISS_G1,
  MISS_G2,
  MISS_KNIFE,
  FALL_1,
  FALL_2,
  FALL_TO_GROUND_1,
  FALL_TO_GROUND_2,
  FALL_TO_GROUND_3,
  HEAVY_FALL_1,
  BODY_SPLAT_1,
  GLASS_SHATTER1,
  GLASS_SHATTER2,
  DROPEN_1,
  DROPEN_2,
  DROPEN_3,
  DRCLOSE_1,
  DRCLOSE_2,
  UNLOCK_DOOR_1,
  KICKIN_DOOR,
  BREAK_LOCK,
  PICKING_LOCK,

  GARAGE_DOOR_OPEN,
  GARAGE_DOOR_CLOSE,
  ELEVATOR_DOOR_OPEN,
  ELEVATOR_DOOR_CLOSE,
  HITECH_DOOR_OPEN,
  HITECH_DOOR_CLOSE,
  CURTAINS_OPEN,
  CURTAINS_CLOSE,
  METAL_DOOR_OPEN,
  METAL_DOOR_CLOSE,

  WALK_LEFT_OUT,
  WALK_RIGHT_OUT,
  WALK_LEFT_OUT2,
  WALK_RIGHT_OUT2,
  WALK_LEFT_IN,
  WALK_RIGHT_IN,
  WALK_LEFT_IN2,
  WALK_RIGHT_IN2,
  WALK_LEFT_ROAD,
  WALK_RIGHT_ROAD,
  WALK_LEFT_ROAD2,
  WALK_RIGHT_ROAD2,
  CRAWL_1,
  CRAWL_2,
  CRAWL_3,
  CRAWL_4,
  TARG_REFINE_BEEP,
  ENDTURN_1,
  HEADCR_1,
  DOORCR_1,
  HEADSPLAT_1,
  BODY_EXPLODE_1,
  EXPLOSION_1,
  CROW_EXPLODE_1,
  SMALL_EXPLODE_1,
  HELI_1,
  BULLET_IMPACT_1,
  BULLET_IMPACT_2,
  BULLET_IMPACT_3,
  CREATURE_BATTLECRY_1,
  ENTER_WATER_1,
  ENTER_DEEP_WATER_1,

  COW_HIT_SND,
  COW_DIE_SND,

  // ROCKET GUN COMPUTER VOICE...
  RG_ID_IMPRINTED,
  RG_ID_INVALID,
  RG_TARGET_SELECTED,

  // CAVE COLLAPSE
  CAVE_COLLAPSE,

  // AIR RAID SOUNDS...
  S_RAID_WHISTLE,
  S_RAID_AMBIENT,
  S_RAID_DIVE,
  S_RAID_TB_DIVE,
  S_RAID_TB_BOMB,

  // VEHICLE SOUNDS
  S_VECH1_MOVE,
  S_VECH1_ON,
  S_VECH1_OFF,
  S_VECH1_INTO,

  S_DRYFIRE1,

  // IMPACT SOUNDS
  S_WOOD_IMPACT1,
  S_WOOD_IMPACT2,
  S_WOOD_IMPACT3,
  S_PORCELAIN_IMPACT1,
  S_RUBBER_IMPACT1,
  S_STONE_IMPACT1,
  S_WATER_IMPACT1,
  S_VEG_IMPACT1,
  S_METAL_IMPACT1,
  S_METAL_IMPACT2,
  S_METAL_IMPACT3,

  S_SLAP_IMPACT,

  // WEAPON RELOAD
  S_RELOAD_REVOLVER,
  S_RELOAD_PISTOL,
  S_RELOAD_SMG,
  S_RELOAD_RIFLE,
  S_RELOAD_SHOTGUN,
  S_RELOAD_LMG,

  // WEAPON LOCKNLOAD
  S_LNL_REVOLVER,
  S_LNL_PISTOL,
  S_LNL_SMG,
  S_LNL_RIFLE,
  S_LNL_SHOTGUN,
  S_LNL_LMG,

  // WEAPON SHOT SOUNDS
  S_SMALL_ROCKET_LAUNCHER,
  S_GLAUNCHER,
  S_UNDER_GLAUNCHER,
  S_ROCKET_LAUNCHER,
  S_MORTAR_SHOT,
  S_GLOCK17,
  S_GLOCK18,
  S_BERETTA92,
  S_BERETTA93,
  S_SWSPECIAL,
  S_BARRACUDA,
  S_DESERTEAGLE,
  S_M1911,
  S_MP5K,
  S_MAC10,
  S_THOMPSON,
  S_COMMANDO,
  S_MP53,
  S_AKSU74,
  S_P90,
  S_TYPE85,
  S_SKS,
  S_DRAGUNOV,
  S_M24,
  S_AUG,
  S_G41,
  S_RUGERMINI,
  S_C7,
  S_FAMAS,
  S_AK74,
  S_AKM,
  S_M14,
  S_FNFAL,
  S_G3A3,
  S_G11,
  S_M870,
  S_SPAS,
  S_CAWS,
  S_FNMINI,
  S_RPK74,
  S_21E,
  S_THROWKNIFE,
  S_TANK_CANNON,
  S_BURSTTYPE1,
  S_AUTOMAG,

  S_SILENCER_1,
  S_SILENCER_2,

  // SWOOSHES.
  SWOOSH_1,
  SWOOSH_2,
  SWOOSH_3,
  SWOOSH_4,
  SWOOSH_5,
  SWOOSH_6,

  // CREATURE SOUNDS....
  ACR_FALL_1,
  ACR_STEP_1,
  ACR_STEP_2,
  ACR_SWIPE,
  ACR_EATFLESH,
  ACR_CRIPPLED,
  ACR_DIE_PART1,
  ACR_DIE_PART2,
  ACR_LUNGE,
  ACR_SMELL_THREAT,
  ACR_SMEEL_PREY,
  ACR_SPIT,
  // BABY
  BCR_DYING,
  BCR_DRAGGING,
  BCR_SHRIEK,
  BCR_SPITTING,
  // LARVAE
  LCR_MOVEMENT,
  LCR_RUPTURE,
  // QUEEN
  LQ_SHRIEK,
  LQ_DYING,
  LQ_ENRAGED_ATTACK,
  LQ_RUPTURING,
  LQ_CRIPPLED,
  LQ_SMELLS_THREAT,
  LQ_WHIP_ATTACK,

  THROW_IMPACT_1,
  THROW_IMPACT_2,

  IDLE_SCRATCH,
  IDLE_ARMPIT,
  IDLE_BACKCRACK,

  AUTORESOLVE_FINISHFX,

  // Interface buttons, misc.
  EMAIL_ALERT,
  ENTERING_TEXT,
  REMOVING_TEXT,
  COMPUTER_BEEP2_IN,
  COMPUTER_BEEP2_OUT,
  COMPUTER_SWITCH1_IN,
  COMPUTER_SWITCH1_OUT,
  VSM_SWITCH1_IN,
  VSM_SWITCH1_OUT,
  VSM_SWITCH2_IN,
  VSM_SWITCH2_OUT,
  SM_SWITCH1_IN,
  SM_SWITCH1_OUT,
  SM_SWITCH2_IN,
  SM_SWITCH2_OUT,
  SM_SWITCH3_IN,
  SM_SWITCH3_OUT,
  BIG_SWITCH3_IN,
  BIG_SWITCH3_OUT,
  KLAXON_ALARM,
  BOXING_BELL,
  HELI_CRASH,
  ATTACH_TO_GUN,
  ATTACH_CERAMIC_PLATES,
  ATTACH_DETONATOR,

  GRAB_ROOF,
  LAND_ON_ROOF,

  UNSTEALTHY_OUTSIDE_1,
  UNSTEALTHY_OUTSIDE_2,
  UNSTEALTHY_INSIDE_1,

  OPEN_DEFAULT_OPENABLE,
  CLOSE_DEFAULT_OPENABLE,

  FIRE_ON_MERC,
  GLASS_CRACK,
  SPIT_RICOCHET,

  BLOODCAT_HIT_1,
  BLOODCAT_DIE_1,
  SLAP_1,

  ROBOT_BEEP,
  DOOR_ELECTRICITY,
  SWIM_1,
  SWIM_2,
  KEY_FAILURE,
  TARGET_OUT_OF_RANGE,
  OPEN_STATUE,
  USE_STATUE_REMOTE,
  USE_WIRE_CUTTERS,
  DRINK_CANTEEN_FEMALE,
  BLOODCAT_ATTACK,
  BLOODCAT_ROAR,
  ROBOT_GREETING,
  ROBOT_DEATH,
  GAS_EXPLODE_1,
  AIR_ESCAPING_1,

  OPEN_DRAWER,
  CLOSE_DRAWER,
  OPEN_LOCKER,
  CLOSE_LOCKER,
  OPEN_WOODEN_BOX,
  CLOSE_WOODEN_BOX,
  ROBOT_STOP,

  WATER_WALK1_IN,
  WATER_WALK1_OUT,
  WATER_WALK2_IN,
  WATER_WALK2_OUT,

  PRONE_UP_SOUND,
  PRONE_DOWN_SOUND,
  KNEEL_UP_SOUND,
  KNEEL_DOWN_SOUND,
  PICKING_SOMETHING_UP,

  COW_FALL,

  BLOODCAT_GROWL_1,
  BLOODCAT_GROWL_2,
  BLOODCAT_GROWL_3,
  BLOODCAT_GROWL_4,
  CREATURE_GAS_NOISE,
  CREATURE_FALL_PART_2,
  CREATURE_DISSOLVE_1,
  QUEEN_AMBIENT_NOISE,
  CREATURE_FALL,
  CROW_PECKING_AT_FLESH,
  CROW_FLYING_AWAY,
  SLAP_2,
  MORTAR_START,
  MORTAR_WHISTLE,
  MORTAR_LOAD,

  TURRET_MOVE,
  TURRET_STOP,
  COW_FALL_2,
  KNIFE_IMPACT,
  EXPLOSION_ALT_BLAST_1,
  EXPLOSION_BLAST_2,
  DRINK_CANTEEN_MALE,
  USE_X_RAY_MACHINE,
  CATCH_OBJECT,
  FENCE_OPEN,

  NUM_SAMPLES
};

enum AmbientDefines {
  LIGHTNING_1 = 0,
  LIGHTNING_2,
  RAIN_1,
  BIRD_1,
  BIRD_2,
  CRICKETS_1,
  CRICKETS_2,
  CRICKET_1,
  CRICKET_2,
  OWL_1,
  OWL_2,
  OWL_3,
  NIGHT_BIRD_1,
  NIGHT_BIRD_2,

  NUM_AMBIENTS
};

typedef void (*SOUND_STOP_CALLBACK)(void *pData);

extern UINT8 AmbientVols[NUM_AMBIENTS];

extern char szSoundEffects[NUM_SAMPLES][255];

BOOLEAN InitJA2Sound();
BOOLEAN ShutdownJA2Sound();
UINT32 PlayJA2Sample(UINT32 usNum, UINT32 usRate, UINT32 ubVolume, UINT32 ubLoops, UINT32 uiPan);
UINT32 PlayJA2StreamingSample(UINT32 usNum, UINT32 usRate, UINT32 ubVolume, UINT32 ubLoops,
                              UINT32 uiPan);

UINT32 PlayJA2SampleFromFile(STR8 szFileName, UINT32 usRate, UINT32 ubVolume, UINT32 ubLoops,
                             UINT32 uiPan);
UINT32 PlayJA2StreamingSampleFromFile(STR8 szFileName, UINT32 usRate, UINT32 ubVolume,
                                      UINT32 ubLoops, UINT32 uiPan,
                                      SOUND_STOP_CALLBACK EndsCallback);

UINT32 PlayJA2Ambient(UINT32 usNum, UINT32 ubVolume, UINT32 ubLoops);
UINT32 PlayJA2AmbientRandom(UINT32 usNum, UINT32 uiTimeMin, UINT32 uiTimeMax);

UINT32 PlaySoldierJA2Sample(UINT16 usID, UINT32 usNum, UINT32 usRate, UINT32 ubVolume,
                            UINT32 ubLoops, UINT32 uiPan, BOOLEAN fCheck);

UINT32 GetSoundEffectsVolume();
void SetSoundEffectsVolume(UINT32 uiNewVolume);

UINT32 GetSpeechVolume();
void SetSpeechVolume(UINT32 uiNewVolume);

// Calculates a volume based on the current Speech Volume level
UINT32 CalculateSpeechVolume(UINT32 uiVolume);

// Calculates a volume based on the current Sound Effects Volume level
UINT32 CalculateSoundEffectsVolume(UINT32 uiVolume);

INT8 SoundDir(INT16 sGridNo);
INT8 SoundVolume(INT8 bInitialVolume, INT16 sGridNo);

// ATE: Warning! Use this sparingly! NOT very robust - can
// have only 1 delayed sound at a time, and uses the global custom
// timer, again, of which can only be one
void PlayDelayedJA2Sample(UINT32 uiDelay, UINT32 usNum, UINT32 usRate, UINT32 ubVolume,
                          UINT32 ubLoops, UINT32 uiPan);

#define POSITION_SOUND_FROM_SOLDIER 0x00000001

INT32 NewPositionSnd(INT16 sGridNo, UINT32 uiFlags, UINT32 uiData, UINT32 iSoundToPlay);
void DeletePositionSnd(INT32 iPositionSndIndex);
void SetPositionSndsActive();
void SetPositionSndsInActive();
void SetPositionSndsVolumeAndPanning();
void SetPositionSndGridNo(INT32 iPositionSndIndex, INT16 sGridNo);

#endif