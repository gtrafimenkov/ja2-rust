#ifndef __SOLDER_CONTROL_H
#define __SOLDER_CONTROL_H

#include "SGP/Types.h"
#include "Tactical/AnimationCache.h"
#include "Tactical/ItemTypes.h"
#include "Tactical/OverheadTypes.h"
#include "Utils/TimerControl.h"

struct VObject;

// Kris:  November 10, 1997
// Please don't change this value from 10.  It will invalidate all of the maps and soldiers.
#define MAXPATROLGRIDS 10  // *** THIS IS A DUPLICATION - MUST BE MOVED !

// TEMP VALUES FOR NAMES
#define MAXCIVLASTNAMES 30
extern uint16_t CivLastNames[MAXCIVLASTNAMES][10];

// ANDREW: these are defines for OKDestanation usage - please move to approprite file
#define IGNOREPEOPLE 0
#define PEOPLETOO 1
#define ALLPEOPLE 2
#define FALLINGTEST 3

#define LOCKED_NO_NEWGRIDNO 2

#define NO_PROFILE 200

#define BATTLE_SND_LOWER_VOLUME 1

#define TAKE_DAMAGE_GUNFIRE 1
#define TAKE_DAMAGE_BLADE 2
#define TAKE_DAMAGE_HANDTOHAND 3
#define TAKE_DAMAGE_FALLROOF 4
#define TAKE_DAMAGE_BLOODLOSS 5
#define TAKE_DAMAGE_EXPLOSION 6
#define TAKE_DAMAGE_ELECTRICITY 7
#define TAKE_DAMAGE_GAS 8
#define TAKE_DAMAGE_TENTACLES 9
#define TAKE_DAMAGE_STRUCTURE_EXPLOSION 10
#define TAKE_DAMAGE_OBJECT 11

#define SOLDIER_UNBLIT_SIZE (75 * 75 * 2)

#define SOLDIER_IS_TACTICALLY_VALID 0x00000001
#define SOLDIER_SHOULD_BE_TACTICALLY_VALID 0x00000002
#define SOLDIER_MULTI_SELECTED 0x00000004
#define SOLDIER_PC 0x00000008
#define SOLDIER_ATTACK_NOTICED 0x00000010
#define SOLDIER_PCUNDERAICONTROL 0x00000020
#define SOLDIER_UNDERAICONTROL 0x00000040
#define SOLDIER_DEAD 0x00000080
#define SOLDIER_GREEN_RAY 0x00000100
#define SOLDIER_LOOKFOR_ITEMS 0x00000200
#define SOLDIER_ENEMY 0x00000400
#define SOLDIER_ENGAGEDINACTION 0x00000800
#define SOLDIER_ROBOT 0x00001000
#define SOLDIER_MONSTER 0x00002000
#define SOLDIER_ANIMAL 0x00004000
#define SOLDIER_VEHICLE 0x00008000
#define SOLDIER_MULTITILE_NZ 0x00010000
#define SOLDIER_MULTITILE_Z 0x00020000
#define SOLDIER_MULTITILE (SOLDIER_MULTITILE_Z | SOLDIER_MULTITILE_NZ)
#define SOLDIER_RECHECKLIGHT 0x00040000
#define SOLDIER_TURNINGFROMHIT 0x00080000
#define SOLDIER_BOXER 0x00100000
#define SOLDIER_LOCKPENDINGACTIONCOUNTER 0x00200000
#define SOLDIER_COWERING 0x00400000
#define SOLDIER_MUTE 0x00800000
#define SOLDIER_GASSED 0x01000000
#define SOLDIER_OFF_MAP 0x02000000
#define SOLDIER_PAUSEANIMOVE 0x04000000
#define SOLDIER_DRIVER 0x08000000
#define SOLDIER_PASSENGER 0x10000000
#define SOLDIER_NPC_DOING_PUNCH 0x20000000
#define SOLDIER_NPC_SHOOTING 0x40000000
#define SOLDIER_LOOK_NEXT_TURNSOLDIER 0x80000000

/*
#define	SOLDIER_TRAIT_LOCKPICKING		0x0001
#define	SOLDIER_TRAIT_HANDTOHAND		0x0002
#define	SOLDIER_TRAIT_ELECTRONICS		0x0004
#define	SOLDIER_TRAIT_NIGHTOPS			0x0008
#define	SOLDIER_TRAIT_THROWING			0x0010
#define	SOLDIER_TRAIT_TEACHING			0x0020
#define	SOLDIER_TRAIT_HEAVY_WEAPS		0x0040
#define	SOLDIER_TRAIT_AUTO_WEAPS		0x0080
#define	SOLDIER_TRAIT_STEALTHY			0x0100
#define	SOLDIER_TRAIT_AMBIDEXT			0x0200
#define	SOLDIER_TRAIT_THIEF					0x0400
#define	SOLDIER_TRAIT_MARTIALARTS		0x0800
#define	SOLDIER_TRAIT_KNIFING				0x1000
*/
#define HAS_SKILL_TRAIT(s, t) (s->ubSkillTrait1 == t || s->ubSkillTrait2 == t)
#define NUM_SKILL_TRAITS(s, t) \
  ((s->ubSkillTrait1 == t) ? ((s->ubSkillTrait2 == t) ? 2 : 1) : ((s->ubSkillTrait2 == t) ? 1 : 0))

#define SOLDIER_QUOTE_SAID_IN_SHIT 0x0001
#define SOLDIER_QUOTE_SAID_LOW_BREATH 0x0002
#define SOLDIER_QUOTE_SAID_BEING_PUMMELED 0x0004
#define SOLDIER_QUOTE_SAID_NEED_SLEEP 0x0008
#define SOLDIER_QUOTE_SAID_LOW_MORAL 0x0010
#define SOLDIER_QUOTE_SAID_MULTIPLE_CREATURES 0x0020
#define SOLDIER_QUOTE_SAID_ANNOYING_MERC 0x0040
#define SOLDIER_QUOTE_SAID_LIKESGUN 0x0080
#define SOLDIER_QUOTE_SAID_DROWNING 0x0100
#define SOLDIER_QUOTE_SAID_ROTTINGCORPSE 0x0200
#define SOLDIER_QUOTE_SAID_SPOTTING_CREATURE_ATTACK 0x0400
#define SOLDIER_QUOTE_SAID_SMELLED_CREATURE 0x0800
#define SOLDIER_QUOTE_SAID_ANTICIPATING_DANGER 0x1000
#define SOLDIER_QUOTE_SAID_WORRIED_ABOUT_CREATURES 0x2000
#define SOLDIER_QUOTE_SAID_PERSONALITY 0x4000
#define SOLDIER_QUOTE_SAID_FOUND_SOMETHING_NICE 0x8000

#define SOLDIER_QUOTE_SAID_EXT_HEARD_SOMETHING 0x0001
#define SOLDIER_QUOTE_SAID_EXT_SEEN_CREATURE_ATTACK 0x0002
#define SOLDIER_QUOTE_SAID_EXT_USED_BATTLESOUND_HIT 0x0004
#define SOLDIER_QUOTE_SAID_EXT_CLOSE_CALL 0x0008
#define SOLDIER_QUOTE_SAID_EXT_MIKE 0x0010
#define SOLDIER_QUOTE_SAID_DONE_ASSIGNMENT 0x0020
#define SOLDIER_QUOTE_SAID_BUDDY_1_WITNESSED 0x0040
#define SOLDIER_QUOTE_SAID_BUDDY_2_WITNESSED 0x0080
#define SOLDIER_QUOTE_SAID_BUDDY_3_WITNESSED 0x0100

#define SOLDIER_CONTRACT_RENEW_QUOTE_NOT_USED 0
#define SOLDIER_CONTRACT_RENEW_QUOTE_89_USED 1
#define SOLDIER_CONTRACT_RENEW_QUOTE_115_USED 2

#define SOLDIER_MISC_HEARD_GUNSHOT 0x01
// make sure soldiers (esp tanks) are not hurt multiple times by explosions
#define SOLDIER_MISC_HURT_BY_EXPLOSION 0x02
// should be revealed due to xrays
#define SOLDIER_MISC_XRAYED 0x04

#define MAXBLOOD 40
#define NOBLOOD MAXBLOOD
#define BLOODTIME 5
#define FOOTPRINTTIME 2
#define MIN_BLEEDING_THRESHOLD 12  // you're OK while <4 Yellow life bars

#define BANDAGED(s) (s->bLifeMax - s->bLife - s->bBleeding)

// amount of time a stats is to be displayed differently, due to change
#define CHANGE_STAT_RECENTLY_DURATION 60000

// MACROS
// #######################################################

#define NO_PENDING_ACTION 255
#define NO_PENDING_ANIMATION 32001
#define NO_PENDING_DIRECTION 253
#define NO_PENDING_STANCE 254
#define NO_DESIRED_HEIGHT 255

#define MAX_FULLTILE_DIRECTIONS 3

// ENUMERATIONS FOR ACTIONS
enum {
  MERC_OPENDOOR,
  MERC_OPENSTRUCT,
  MERC_PICKUPITEM,
  MERC_PUNCH,
  MERC_KNIFEATTACK,
  MERC_GIVEAID,
  MERC_GIVEITEM,
  MERC_WAITFOROTHERSTOTRIGGER,
  MERC_CUTFFENCE,
  MERC_DROPBOMB,
  MERC_STEAL,
  MERC_TALK,
  MERC_ENTER_VEHICLE,
  MERC_REPAIR,
  MERC_RELOADROBOT,
  MERC_TAKEBLOOD,
  MERC_ATTACH_CAN,
  MERC_FUEL_VEHICLE,
};

// ENUMERATIONS FOR THROW ACTIONS
enum {
  NO_THROW_ACTION,
  THROW_ARM_ITEM,
  THROW_TARGET_MERC_CATCH,
};

// An enumeration for playing battle sounds
enum {
  BATTLE_SOUND_OK1,
  BATTLE_SOUND_OK2,
  BATTLE_SOUND_COOL1,
  BATTLE_SOUND_CURSE1,
  BATTLE_SOUND_HIT1,
  BATTLE_SOUND_HIT2,
  BATTLE_SOUND_LAUGH1,
  BATTLE_SOUND_ATTN1,
  BATTLE_SOUND_DIE1,
  BATTLE_SOUND_HUMM,
  BATTLE_SOUND_NOTHING,
  BATTLE_SOUND_GOTIT,
  BATTLE_SOUND_LOWMARALE_OK1,
  BATTLE_SOUND_LOWMARALE_OK2,
  BATTLE_SOUND_LOWMARALE_ATTN1,
  BATTLE_SOUND_LOCKED,
  BATTLE_SOUND_ENEMY,
  NUM_MERC_BATTLE_SOUNDS
};

// different kinds of merc
enum {
  MERC_TYPE__PLAYER_CHARACTER,
  MERC_TYPE__AIM_MERC,
  MERC_TYPE__MERC,
  MERC_TYPE__NPC,
  MERC_TYPE__EPC,
  MERC_TYPE__NPC_WITH_UNEXTENDABLE_CONTRACT,
  MERC_TYPE__VEHICLE,
};

// I don't care if this isn't intuitive!  The hand positions go right
// before the big pockets so we can loop through them that way. --CJC
#define NO_SLOT -1

// vehicle/human path structure
struct path {
  uint32_t uiSectorId;
  uint32_t uiEta;
  BOOLEAN fSpeed;
  struct path *pNext;
  struct path *pPrev;
};

enum {
  HELMETPOS = 0,
  VESTPOS,
  LEGPOS,
  HEAD1POS,
  HEAD2POS,
  HANDPOS,
  SECONDHANDPOS,
  BIGPOCK1POS,
  BIGPOCK2POS,
  BIGPOCK3POS,
  BIGPOCK4POS,
  SMALLPOCK1POS,
  SMALLPOCK2POS,
  SMALLPOCK3POS,
  SMALLPOCK4POS,
  SMALLPOCK5POS,
  SMALLPOCK6POS,
  SMALLPOCK7POS,
  SMALLPOCK8POS,  // = 18, so 19 pockets needed

  NUM_INV_SLOTS,
};

#define SOLDIER_CLASS_ENEMY(bSoldierClass) \
  ((bSoldierClass >= SOLDIER_CLASS_ADMINISTRATOR) && (bSoldierClass <= SOLDIER_CLASS_ARMY))
#define SOLDIER_CLASS_MILITIA(bSoldierClass) \
  ((bSoldierClass >= SOLDIER_CLASS_GREEN_MILITIA) && (bSoldierClass <= SOLDIER_CLASS_ELITE_MILITIA))

// This macro should be used whenever we want to see if someone is neutral
// IF WE ARE CONSIDERING ATTACKING THEM.  Creatures & bloodcats will attack neutrals
// but they can't attack empty vehicles!!
#define CONSIDERED_NEUTRAL(me, them) \
  ((them->bNeutral) && (me->bTeam != CREATURE_TEAM || (them->uiStatusFlags & SOLDIER_VEHICLE)))

typedef struct {
  uint8_t ubKeyID;
  uint8_t ubNumber;
} KEY_ON_RING;

typedef struct {
  float dX;
  float dY;
  float dZ;
  float dForceX;
  float dForceY;
  float dForceZ;
  float dLifeSpan;
  uint8_t ubActionCode;
  uint32_t uiActionData;

} THROW_PARAMS;

#define DELAYED_MOVEMENT_FLAG_PATH_THROUGH_PEOPLE 0x01

// reasons for being unable to continue movement
enum {
  REASON_STOPPED_NO_APS,
  REASON_STOPPED_SIGHT,
};

enum {
  HIT_BY_TEARGAS = 0x01,
  HIT_BY_MUSTARDGAS = 0x02,
  HIT_BY_CREATUREGAS = 0x04,
};

struct SOLDIERTYPE {
  // ID
  uint8_t ubID;
  uint8_t bReserved1;

  // DESCRIPTION / STATS, ETC
  uint8_t ubBodyType;
  INT8 bActionPoints;
  INT8 bInitialActionPoints;

  uint32_t uiStatusFlags;

  struct OBJECTTYPE inv[NUM_INV_SLOTS];
  struct OBJECTTYPE *pTempObject;
  KEY_ON_RING *pKeyRing;

  INT8 bOldLife;  // life at end of last turn, recorded for monster AI
  // attributes
  uint8_t bInSector;
  INT8 bFlashPortraitFrame;
  INT16 sFractLife;  // fraction of life pts (in hundreths)
  INT8 bBleeding;    // blood loss control variable
  INT8 bBreath;      // current breath value
  INT8 bBreathMax;   // max breath, affected by fatigue/sleep
  INT8 bStealthMode;

  INT16 sBreathRed;  // current breath value
  BOOLEAN fDelayedMovement;

  BOOLEAN fReloading;
  uint8_t ubWaitActionToDo;
  BOOLEAN fPauseAim;
  INT8 ubInsertionDirection;
  INT8 bGunType;
  // skills
  uint8_t ubOppNum;
  INT8 bLastRenderVisibleValue;
  BOOLEAN fInMissionExitNode;
  uint8_t ubAttackingHand;
  INT8 bScientific;
  // traits
  INT16 sWeightCarriedAtTurnStart;
  CHAR16 name[10];

  INT8 bVisible;  // to render or not to render...

  INT8 bActive;

  INT8 bTeam;  // Team identifier

  // NEW MOVEMENT INFORMATION for Strategic Movement
  uint8_t ubGroupID;        // the movement group the merc is currently part of.
  BOOLEAN fBetweenSectors;  // set when the group isn't actually in a sector.
                            // sSectorX and sSectorY will reflect the sector the
                            // merc was at last.

  uint8_t ubMovementNoiseHeard;  // 8 flags by direction

  // 23 bytes so far

  // WORLD POSITION STUFF
  FLOAT dXPos;
  FLOAT dYPos;
  FLOAT dOldXPos;
  FLOAT dOldYPos;
  INT16 sInitialGridNo;
  INT16 sGridNo;
  INT8 bDirection;
  INT16 sHeightAdjustment;
  INT16 sDesiredHeight;
  INT16 sTempNewGridNo;  // New grid no for advanced animations
  INT16 sRoomNo;
  INT8 bOverTerrainType;
  INT8 bOldOverTerrainType;

  INT8 bCollapsed;        // collapsed due to being out of APs
  INT8 bBreathCollapsed;  // collapsed due to being out of APs
  // 50 bytes so far

  uint8_t ubDesiredHeight;
  uint16_t usPendingAnimation;
  uint8_t ubPendingStanceChange;
  uint16_t usAnimState;
  BOOLEAN fNoAPToFinishMove;
  BOOLEAN fPausedMove;
  BOOLEAN fUIdeadMerc;        // UI Flags for removing a newly dead merc
  BOOLEAN fUInewMerc;         // UI Flags for adding newly created merc ( panels, etc )
  BOOLEAN fUICloseMerc;       // UI Flags for closing panels
  BOOLEAN fUIFirstTimeNOAP;   // UI Flag for diming guys when no APs ( dirty flags )
  BOOLEAN fUIFirstTimeUNCON;  // UI FLAG For unconscious dirty

  TIMECOUNTER UpdateCounter;
  TIMECOUNTER DamageCounter;
  TIMECOUNTER ReloadCounter;
  TIMECOUNTER FlashSelCounter;
  TIMECOUNTER AICounter;
  TIMECOUNTER FadeCounter;

  uint8_t ubSkillTrait1;
  uint8_t ubSkillTrait2;

  uint32_t uiAIDelay;
  INT8 bDexterity;  // dexterity (hand coord) value
  INT8 bWisdom;
  INT16 sReloadDelay;
  uint8_t ubAttackerID;
  uint8_t ubPreviousAttackerID;
  BOOLEAN fTurnInProgress;

  BOOLEAN fIntendedTarget;  // intentionally shot?
  BOOLEAN fPauseAllAnimation;

  INT8 bExpLevel;  // general experience level
  INT16 sInsertionGridNo;

  BOOLEAN fContinueMoveAfterStanceChange;

  // 60
  struct AnimationSurfaceCache AnimCache;  // will be 9 bytes once changed to pointers

  INT8 bLife;  // current life (hit points or health)
  uint8_t bSide;
  uint8_t bViewRange;
  INT8 bNewOppCnt;
  INT8 bService;  // first aid, or other time consuming process

  uint16_t usAniCode;
  uint16_t usAniFrame;
  INT16 sAniDelay;

  // MOVEMENT TO NEXT TILE HANDLING STUFF
  INT8 bAgility;  // agility (speed) value
  uint8_t ubDelayedMovementCauseMerc;
  INT16 sDelayedMovementCauseGridNo;
  INT16 sReservedMovementGridNo;

  INT8 bStrength;

  // Weapon Stuff
  BOOLEAN fHoldAttackerUntilDone;
  INT16 sTargetGridNo;
  INT8 bTargetLevel;
  INT8 bTargetCubeLevel;
  INT16 sLastTarget;
  INT8 bTilesMoved;
  INT8 bLeadership;
  FLOAT dNextBleed;
  BOOLEAN fWarnedAboutBleeding;
  BOOLEAN fDyingComment;

  uint8_t ubTilesMovedPerRTBreathUpdate;
  uint16_t usLastMovementAnimPerRTBreathUpdate;

  BOOLEAN fTurningToShoot;
  BOOLEAN fTurningToFall;
  BOOLEAN fTurningUntilDone;
  BOOLEAN fGettingHit;
  BOOLEAN fInNonintAnim;
  BOOLEAN fFlashLocator;
  INT16 sLocatorFrame;
  BOOLEAN fShowLocator;
  BOOLEAN fFlashPortrait;
  INT8 bMechanical;
  INT8 bLifeMax;  // maximum life for this merc

  INT32 iFaceIndex;

  // PALETTE MANAGEMENT STUFF
  PaletteRepID HeadPal;   // 30
  PaletteRepID PantsPal;  // 30
  PaletteRepID VestPal;   // 30
  PaletteRepID SkinPal;   // 30
  PaletteRepID MiscPal;   // 30

  // FULL 3-d TILE STUFF ( keep records of three tiles infront )
  uint16_t usFrontArcFullTileList[MAX_FULLTILE_DIRECTIONS];
  INT16 usFrontArcFullTileGridNos[MAX_FULLTILE_DIRECTIONS];

  struct SGPPaletteEntry *p8BPPPalette;  // 4
  uint16_t *p16BPPPalette;
  uint16_t *pShades[NUM_SOLDIER_SHADES];  // Shading tables
  uint16_t *pGlowShades[20];              //
  uint16_t *pCurrentShade;
  INT8 bMedical;
  BOOLEAN fBeginFade;
  uint8_t ubFadeLevel;
  uint8_t ubServiceCount;
  uint8_t ubServicePartner;
  INT8 bMarksmanship;
  INT8 bExplosive;
  THROW_PARAMS *pThrowParams;
  BOOLEAN fTurningFromPronePosition;
  INT8 bReverse;
  struct LEVELNODE *pLevelNode;
  struct LEVELNODE *pExternShadowLevelNode;
  struct LEVELNODE *pRoofUILevelNode;

  // WALKING STUFF
  INT8 bDesiredDirection;
  INT16 sDestXPos;
  INT16 sDestYPos;
  INT16 sDesiredDest;
  INT16 sDestination;
  INT16 sFinalDestination;
  INT8 bLevel;
  INT8 bStopped;
  INT8 bNeedToLook;

  // PATH STUFF
  uint16_t usPathingData[MAX_PATH_LIST_SIZE];
  uint16_t usPathDataSize;
  uint16_t usPathIndex;
  INT16 sBlackList;
  INT8 bAimTime;
  INT8 bShownAimTime;
  INT8 bPathStored;  // good for AI to reduct redundancy
  INT8 bHasKeys;     // allows AI controlled dudes to open locked doors

  // UNBLIT BACKGROUND
  uint16_t *pBackGround;
  uint16_t *pZBackground;
  uint16_t usUnblitX, usUnblitY;
  uint16_t usUnblitWidth, usUnblitHeight;

  uint8_t ubStrategicInsertionCode;
  uint16_t usStrategicInsertionData;

  INT32 iLight;
  INT32 iMuzFlash;
  INT8 bMuzFlashCount;

  INT16 sX;
  INT16 sY;

  uint16_t usOldAniState;
  INT16 sOldAniCode;

  INT8 bBulletsLeft;
  uint8_t ubSuppressionPoints;

  // STUFF FOR RANDOM ANIMATIONS
  uint32_t uiTimeOfLastRandomAction;
  INT16 usLastRandomAnim;

  // AI STUFF
  INT8 bOppList[MAX_NUM_SOLDIERS];  // AI knowledge database
  INT8 bLastAction;
  INT8 bAction;
  uint16_t usActionData;
  INT8 bNextAction;
  uint16_t usNextActionData;
  INT8 bActionInProgress;
  INT8 bAlertStatus;
  INT8 bOppCnt;
  INT8 bNeutral;
  INT8 bNewSituation;
  INT8 bNextTargetLevel;
  INT8 bOrders;
  INT8 bAttitude;
  INT8 bUnderFire;
  INT8 bShock;
  INT8 bUnderEscort;
  INT8 bBypassToGreen;
  uint8_t ubLastMercToRadio;
  INT8 bDominantDir;                   // AI main direction to face...
  INT8 bPatrolCnt;                     // number of patrol gridnos
  INT8 bNextPatrolPnt;                 // index to next patrol gridno
  INT16 usPatrolGrid[MAXPATROLGRIDS];  // AI list for ptr->orders==PATROL
  INT16 sNoiseGridno;
  uint8_t ubNoiseVolume;
  INT8 bLastAttackHit;
  uint8_t ubXRayedBy;
  FLOAT dHeightAdjustment;
  INT8 bMorale;
  INT8 bTeamMoraleMod;
  INT8 bTacticalMoraleMod;
  INT8 bStrategicMoraleMod;
  INT8 bAIMorale;
  uint8_t ubPendingAction;
  uint8_t ubPendingActionAnimCount;
  uint32_t uiPendingActionData1;
  INT16 sPendingActionData2;
  INT8 bPendingActionData3;
  INT8 ubDoorHandleCode;
  uint32_t uiPendingActionData4;
  INT8 bInterruptDuelPts;
  INT8 bPassedLastInterrupt;
  INT8 bIntStartAPs;
  INT8 bMoved;
  INT8 bHunting;
  uint8_t ubLastCall;
  uint8_t ubCaller;
  INT16 sCallerGridNo;
  uint8_t bCallPriority;
  INT8 bCallActedUpon;
  INT8 bFrenzied;
  INT8 bNormalSmell;
  INT8 bMonsterSmell;
  INT8 bMobility;
  INT8 bRTPCombat;
  INT8 fAIFlags;

  BOOLEAN fDontChargeReadyAPs;
  uint16_t usAnimSurface;
  uint16_t sZLevel;
  BOOLEAN fPrevInWater;
  BOOLEAN fGoBackToAimAfterHit;

  INT16 sWalkToAttackGridNo;
  INT16 sWalkToAttackWalkToCost;

  BOOLEAN fForceRenderColor;
  BOOLEAN fForceNoRenderPaletteCycle;

  INT16 sLocatorOffX;
  INT16 sLocatorOffY;
  BOOLEAN fStopPendingNextTile;

  BOOLEAN fForceShade;
  uint16_t *pForcedShade;

  INT8 bDisplayDamageCount;
  INT8 fDisplayDamage;
  INT16 sDamage;
  INT16 sDamageX;
  INT16 sDamageY;
  INT8 bDamageDir;
  INT8 bDoBurst;
  INT16 usUIMovementMode;
  INT8 bUIInterfaceLevel;
  BOOLEAN fUIMovementFast;

  TIMECOUNTER BlinkSelCounter;
  TIMECOUNTER PortraitFlashCounter;
  BOOLEAN fDeadSoundPlayed;
  uint8_t ubProfile;
  uint8_t ubQuoteRecord;
  uint8_t ubQuoteActionID;
  uint8_t ubBattleSoundID;

  BOOLEAN fClosePanel;
  BOOLEAN fClosePanelToDie;
  uint8_t ubClosePanelFrame;
  BOOLEAN fDeadPanel;
  uint8_t ubDeadPanelFrame;
  BOOLEAN fOpenPanel;
  INT8 bOpenPanelFrame;

  INT16 sPanelFaceX;
  INT16 sPanelFaceY;

  // QUOTE STUFF
  INT8 bNumHitsThisTurn;
  uint16_t usQuoteSaidFlags;
  INT8 fCloseCall;
  INT8 bLastSkillCheck;
  INT8 ubSkillCheckAttempts;

  INT8 bVocalVolume;  // verbal sounds need to differ in volume

  INT8 bStartFallDir;
  INT8 fTryingToFall;

  uint8_t ubPendingDirection;
  uint32_t uiAnimSubFlags;

  uint8_t bAimShotLocation;
  uint8_t ubHitLocation;

  uint16_t *pEffectShades[NUM_SOLDIER_EFFECTSHADES];  // Shading tables for effects

  uint8_t ubPlannedUIAPCost;
  INT16 sPlannedTargetX;
  INT16 sPlannedTargetY;

  INT16 sSpreadLocations[6];
  BOOLEAN fDoSpread;
  INT16 sStartGridNo;
  INT16 sEndGridNo;
  INT16 sForcastGridno;
  INT16 sZLevelOverride;
  INT8 bMovedPriorToInterrupt;
  INT32 iEndofContractTime;  // time, in global time(resolution, minutes) that merc will leave, or
                             // if its a M.E.R.C. merc it will be set to -1.  -2 for NPC and player
                             // generated
  INT32 iStartContractTime;
  INT32 iTotalContractLength;    // total time of AIM mercs contract	or the time since last paid
                                 // for a M.E.R.C. merc
  INT32 iNextActionSpecialData;  // AI special action data record for the next action
  uint8_t ubWhatKindOfMercAmI;   // Set to the type of character it is
  INT8 bAssignment;              // soldiers current assignment
  INT8 bOldAssignment;           // old assignment, for autosleep purposes
  BOOLEAN fForcedToStayAwake;    // forced by player to stay awake, reset to false, the moment they
                                 // are set to rest or sleep
  INT8 bTrainStat;               // current stat soldier is training
  INT16 sSectorX;                // X position on the Stategic Map
  INT16 sSectorY;                // Y position on the Stategic Map
  INT8 bSectorZ;                 // Z sector location
  INT32 iVehicleId;              // the id of the vehicle the char is in
  struct path *pMercPath;        // Path Structure
  uint8_t fHitByGasFlags;        // flags
  uint16_t usMedicalDeposit;     // is there a medical deposit on merc
  uint16_t usLifeInsurance;      // is there life insurance taken out on merc

  // DEF:  Used for the communications
  uint32_t uiStartMovementTime;    // the time since the merc first started moving
  uint32_t uiOptimumMovementTime;  // everytime in ececute overhead the time for the current ani
                                   // will be added to this total
  uint32_t usLastUpdateTime;       // The last time the soldier was in ExecuteOverhead

  BOOLEAN fIsSoldierMoving;   // ie.  Record time is on
  BOOLEAN fIsSoldierDelayed;  // Is the soldier delayed Soldier
  BOOLEAN fSoldierUpdatedFromNetwork;
  uint32_t uiSoldierUpdateNumber;
  BYTE ubSoldierUpdateType;
  // END

  INT32 iStartOfInsuranceContract;
  uint32_t uiLastAssignmentChangeMin;  // timestamp of last assignment change in minutes
  INT32 iTotalLengthOfInsuranceContract;

  uint8_t ubSoldierClass;  // admin, elite, troop (creature types?)
  uint8_t ubAPsLostToSuppression;
  BOOLEAN fChangingStanceDueToSuppression;
  uint8_t ubSuppressorID;

  // Squad merging vars
  uint8_t ubDesiredSquadAssignment;
  uint8_t ubNumTraversalsAllowedToMerge;

  uint16_t usPendingAnimation2;
  uint8_t ubCivilianGroup;

  // time changes...when a stat was changed according to GetJA2Clock();
  uint32_t uiChangeLevelTime;
  uint32_t uiChangeHealthTime;
  uint32_t uiChangeStrengthTime;
  uint32_t uiChangeDexterityTime;
  uint32_t uiChangeAgilityTime;
  uint32_t uiChangeWisdomTime;
  uint32_t uiChangeLeadershipTime;
  uint32_t uiChangeMarksmanshipTime;
  uint32_t uiChangeExplosivesTime;
  uint32_t uiChangeMedicalTime;
  uint32_t uiChangeMechanicalTime;

  uint32_t uiUniqueSoldierIdValue;  // the unique value every instance of a soldier gets - 1 is the
                                    // first valid value
  INT8 bBeingAttackedCount;         // Being attacked counter

  INT8 bNewItemCount[NUM_INV_SLOTS];
  INT8 bNewItemCycleCount[NUM_INV_SLOTS];
  BOOLEAN fCheckForNewlyAddedItems;
  INT8 bEndDoorOpenCode;

  uint8_t ubScheduleID;
  INT16 sEndDoorOpenCodeData;
  TIMECOUNTER NextTileCounter;
  BOOLEAN fBlockedByAnotherMerc;
  INT8 bBlockedByAnotherMercDirection;
  uint16_t usAttackingWeapon;
  INT8 bWeaponMode;
  uint8_t ubTargetID;
  INT8 bAIScheduleProgress;
  INT16 sOffWorldGridNo;
  struct TAG_anitile *pAniTile;
  INT8 bCamo;
  INT16 sAbsoluteFinalDestination;
  uint8_t ubHiResDirection;
  uint8_t ubHiResDesiredDirection;
  uint8_t ubLastFootPrintSound;
  INT8 bVehicleID;
  INT8 fPastXDest;
  INT8 fPastYDest;
  INT8 bMovementDirection;
  INT16 sOldGridNo;
  uint16_t usDontUpdateNewGridNoOnMoveAnimChange;
  INT16 sBoundingBoxWidth;
  INT16 sBoundingBoxHeight;
  INT16 sBoundingBoxOffsetX;
  INT16 sBoundingBoxOffsetY;
  uint32_t uiTimeSameBattleSndDone;
  INT8 bOldBattleSnd;
  BOOLEAN fReactingFromBeingShot;
  BOOLEAN fContractPriceHasIncreased;
  INT32 iBurstSoundID;
  BOOLEAN fFixingSAMSite;
  BOOLEAN fFixingRobot;
  INT8 bSlotItemTakenFrom;
  BOOLEAN fSignedAnotherContract;
  uint8_t ubAutoBandagingMedic;
  BOOLEAN fDontChargeTurningAPs;
  uint8_t ubRobotRemoteHolderID;
  uint32_t uiTimeOfLastContractUpdate;
  INT8 bTypeOfLastContract;
  INT8 bTurnsCollapsed;
  INT8 bSleepDrugCounter;
  uint8_t ubMilitiaKills;

  INT8 bFutureDrugEffect[2];    // value to represent effect of a needle
  INT8 bDrugEffectRate[2];      // represents rate of increase and decrease of effect
  INT8 bDrugEffect[2];          // value that affects AP & morale calc ( -ve is poorly )
  INT8 bDrugSideEffectRate[2];  // duration of negative AP and morale effect
  INT8 bDrugSideEffect[2];      // duration of negative AP and morale effect
  INT8 bTimesDrugUsedSinceSleep[2];

  INT8 bBlindedCounter;
  BOOLEAN fMercCollapsedFlag;
  BOOLEAN fDoneAssignmentAndNothingToDoFlag;
  BOOLEAN fMercAsleep;
  BOOLEAN fDontChargeAPsForStanceChange;

  uint8_t ubHoursOnAssignment;  // used for assignments handled only every X hours

  uint8_t
      ubMercJustFired;  // the merc was just fired..there may be dialogue events occuring, this flag
                        // will prevent any interaction with contracts until after the merc leaves
  uint8_t ubTurnsUntilCanSayHeardNoise;
  uint16_t usQuoteSaidExtFlags;

  uint16_t sContPathLocation;
  INT8 bGoodContPath;
  uint8_t ubPendingActionInterrupted;
  INT8 bNoiseLevel;
  INT8 bRegenerationCounter;
  INT8 bRegenBoostersUsedToday;
  INT8 bNumPelletsHitBy;
  INT16 sSkillCheckGridNo;
  uint8_t ubLastEnemyCycledID;

  uint8_t ubPrevSectorID;
  uint8_t ubNumTilesMovesSinceLastForget;
  INT8 bTurningIncrement;
  uint32_t uiBattleSoundID;

  BOOLEAN fSoldierWasMoving;
  BOOLEAN fSayAmmoQuotePending;
  uint16_t usValueGoneUp;

  uint8_t ubNumLocateCycles;
  uint8_t ubDelayedMovementFlags;
  BOOLEAN fMuzzleFlash;
  uint8_t ubCTGTTargetID;

  TIMECOUNTER PanelAnimateCounter;
  uint32_t uiMercChecksum;

  INT8 bCurrentCivQuote;
  INT8 bCurrentCivQuoteDelta;
  uint8_t ubMiscSoldierFlags;
  uint8_t ubReasonCantFinishMove;

  INT16 sLocationOfFadeStart;
  uint8_t bUseExitGridForReentryDirection;

  uint32_t uiTimeSinceLastSpoke;
  uint8_t ubContractRenewalQuoteCode;
  INT16 sPreTraversalGridNo;
  uint32_t uiXRayActivatedTime;
  INT8 bTurningFromUI;
  INT8 bPendingActionData5;

  INT8 bDelayedStrategicMoraleMod;
  uint8_t ubDoorOpeningNoise;

  struct GROUP *pGroup;
  uint8_t ubLeaveHistoryCode;
  BOOLEAN fDontUnsetLastTargetFromTurn;
  INT8 bOverrideMoveSpeed;
  BOOLEAN fUseMoverrideMoveSpeed;

  uint32_t uiTimeSoldierWillArrive;
  BOOLEAN fDieSoundUsed;
  BOOLEAN fUseLandingZoneForArrival;
  BOOLEAN fFallClockwise;
  INT8 bVehicleUnderRepairID;
  INT32 iTimeCanSignElsewhere;
  INT8 bHospitalPriceModifier;
  INT8 bFillerBytes[3];
  uint32_t uiStartTimeOfInsuranceContract;
  BOOLEAN fRTInNonintAnim;
  BOOLEAN fDoingExternalDeath;
  INT8 bCorpseQuoteTolerance;
  INT8 bYetAnotherPaddingSpace;
  INT32 iPositionSndID;
  INT32 iTuringSoundID;
  uint8_t ubLastDamageReason;
  BOOLEAN fComplainedThatTired;
  INT16 sLastTwoLocations[2];
  INT16 bFillerDude;
  INT32 uiTimeSinceLastBleedGrunt;
  uint8_t ubNextToPreviousAttackerID;

  uint8_t bFiller[39];
};

#define HEALTH_INCREASE 0x0001
#define STRENGTH_INCREASE 0x0002
#define DEX_INCREASE 0x0004
#define AGIL_INCREASE 0x0008
#define WIS_INCREASE 0x0010
#define LDR_INCREASE 0x0020

#define MRK_INCREASE 0x0040
#define MED_INCREASE 0x0080
#define EXP_INCREASE 0x0100
#define MECH_INCREASE 0x0200

#define LVL_INCREASE 0x0400

typedef enum { WM_NORMAL = 0, WM_BURST, WM_ATTACHED, NUM_WEAPON_MODES } WeaponModes;

// TYPEDEFS FOR ANIMATION PROFILES
struct ANIM_PROF_TILE {
  uint16_t usTileFlags;
  INT8 bTileX;
  INT8 bTileY;
};

struct ANIM_PROF_DIR {
  uint8_t ubNumTiles;
  struct ANIM_PROF_TILE *pTiles;
};

struct ANIM_PROF {
  struct ANIM_PROF_DIR Dirs[8];
};

// Globals
//////////

// VARIABLES FOR PALETTE REPLACEMENTS FOR HAIR, ETC
extern uint32_t guiNumPaletteSubRanges;
extern uint8_t *gubpNumReplacementsPerRange;
extern PaletteSubRangeType *gpPaletteSubRanges;
extern uint32_t guiNumReplacements;
extern PaletteReplacementType *gpPalRep;

extern uint8_t bHealthStrRanges[];

// Functions
////////////

// CREATION FUNCTIONS
BOOLEAN DeleteSoldier(struct SOLDIERTYPE *pSoldier);
BOOLEAN CreateSoldierLight(struct SOLDIERTYPE *pSoldier);
BOOLEAN DeleteSoldierLight(struct SOLDIERTYPE *pSoldier);

BOOLEAN CreateSoldierCommon(uint8_t ubBodyType, struct SOLDIERTYPE *pSoldier, uint16_t usSoldierID,
                            uint16_t usState);

// Soldier Management functions, called by Event Pump.c
BOOLEAN EVENT_InitNewSoldierAnim(struct SOLDIERTYPE *pSoldier, uint16_t usNewState,
                                 uint16_t usStartingAniCode, BOOLEAN fForce);

BOOLEAN ChangeSoldierState(struct SOLDIERTYPE *pSoldier, uint16_t usNewState,
                           uint16_t usStartingAniCode, BOOLEAN fForce);
void EVENT_SetSoldierPosition(struct SOLDIERTYPE *pSoldier, FLOAT dNewXPos, FLOAT dNewYPos);
void EVENT_SetSoldierDestination(struct SOLDIERTYPE *pSoldier, uint16_t usNewDirection);
void EVENT_GetNewSoldierPath(struct SOLDIERTYPE *pSoldier, uint16_t sDestGridNo,
                             uint16_t usMovementAnim);
BOOLEAN EVENT_InternalGetNewSoldierPath(struct SOLDIERTYPE *pSoldier, uint16_t sDestGridNo,
                                        uint16_t usMovementAnim, BOOLEAN fFromUI,
                                        BOOLEAN fForceRestart);

void EVENT_SetSoldierDirection(struct SOLDIERTYPE *pSoldier, uint16_t usNewDirection);
void EVENT_SetSoldierDesiredDirection(struct SOLDIERTYPE *pSoldier, uint16_t usNewDirection);
void EVENT_FireSoldierWeapon(struct SOLDIERTYPE *pSoldier, INT16 sTargetGridNo);
void EVENT_SoldierGotHit(struct SOLDIERTYPE *pSoldier, uint16_t usWeaponIndex, INT16 ubDamage,
                         INT16 sBreathLoss, uint16_t bDirection, uint16_t sRange,
                         uint8_t ubAttackerID, uint8_t ubSpecial, uint8_t ubHitLocation,
                         INT16 sSubsequent, INT16 sLocationGridNo);
void EVENT_SoldierBeginBladeAttack(struct SOLDIERTYPE *pSoldier, INT16 sGridNo,
                                   uint8_t ubDirection);
void EVENT_SoldierBeginPunchAttack(struct SOLDIERTYPE *pSoldier, INT16 sGridNo,
                                   uint8_t ubDirection);
void EVENT_SoldierBeginFirstAid(struct SOLDIERTYPE *pSoldier, INT16 sGridNo, uint8_t ubDirection);
void EVENT_StopMerc(struct SOLDIERTYPE *pSoldier, INT16 sGridNo, INT8 bDirection);
void EVENT_SoldierBeginCutFence(struct SOLDIERTYPE *pSoldier, INT16 sGridNo, uint8_t ubDirection);
void EVENT_SoldierBeginRepair(struct SOLDIERTYPE *pSoldier, INT16 sGridNo, uint8_t ubDirection);
void EVENT_SoldierBeginRefuel(struct SOLDIERTYPE *pSoldier, INT16 sGridNo, uint8_t ubDirection);

BOOLEAN SoldierReadyWeapon(struct SOLDIERTYPE *pSoldier, INT16 sTargetXPos, INT16 sTargetYPos,
                           BOOLEAN fEndReady);
void SetSoldierHeight(struct SOLDIERTYPE *pSoldier, FLOAT dNewHeight);
void BeginSoldierClimbUpRoof(struct SOLDIERTYPE *pSoldier);
void BeginSoldierClimbDownRoof(struct SOLDIERTYPE *pSoldier);
void BeginSoldierClimbFence(struct SOLDIERTYPE *pSoldier);
void SetSoldierGridNo(struct SOLDIERTYPE *pSoldier, INT16 sNewGridNo, BOOLEAN fForceRemove);

BOOLEAN CheckSoldierHitRoof(struct SOLDIERTYPE *pSoldier);
void BeginSoldierGetup(struct SOLDIERTYPE *pSoldier);
BOOLEAN ReCreateSelectedSoldierLight();

// Soldier Management functions called by Overhead.c
BOOLEAN ConvertAniCodeToAniFrame(struct SOLDIERTYPE *pSoldier, uint16_t usAniFrame);
void TurnSoldier(struct SOLDIERTYPE *pSold);
void EVENT_BeginMercTurn(struct SOLDIERTYPE *pSoldier, BOOLEAN fFromRealTime,
                         INT32 iRealTimeCounter);
void ChangeSoldierStance(struct SOLDIERTYPE *pSoldier, uint8_t ubDesiredStance);
void ModifySoldierAniSpeed(struct SOLDIERTYPE *pSoldier);
void StopSoldier(struct SOLDIERTYPE *pSoldier);
uint8_t SoldierTakeDamage(struct SOLDIERTYPE *pSoldier, INT8 bHeight, INT16 sLifeDeduct,
                          INT16 sBreathDeduct, uint8_t ubReason, uint8_t ubAttacker,
                          INT16 sSourceGrid, INT16 sSubsequent, BOOLEAN fShowDamage);
void RevivePlayerTeam();
void ReviveSoldier(struct SOLDIERTYPE *pSoldier);

// Palette functions for soldiers
BOOLEAN CreateSoldierPalettes(struct SOLDIERTYPE *pSoldier);
BOOLEAN GetPaletteRepIndexFromID(PaletteRepID aPalRep, uint8_t *pubPalIndex);
BOOLEAN SetPaletteReplacement(struct SGPPaletteEntry *p8BPPPalette, PaletteRepID aPalRep);
BOOLEAN LoadPaletteData();
BOOLEAN DeletePaletteData();

// UTILITY FUNCTUIONS
void MoveMerc(struct SOLDIERTYPE *pSoldier, FLOAT dMovementChange, FLOAT dAngle,
              BOOLEAN fCheckRange);
void MoveMercFacingDirection(struct SOLDIERTYPE *pSoldier, BOOLEAN fReverse, FLOAT dMovementDist);
INT16 GetDirectionFromXY(INT16 sXPos, INT16 sYPos, struct SOLDIERTYPE *pSoldier);
INT16 GetDirectionFromGridNo(INT16 sGridNo, struct SOLDIERTYPE *pSoldier);
uint8_t atan8(INT16 sXPos, INT16 sYPos, INT16 sXPos2, INT16 sYPos2);
uint8_t atan8FromAngle(DOUBLE dAngle);
INT8 CalcActionPoints(struct SOLDIERTYPE *pSold);
BOOLEAN IsActionInterruptable(struct SOLDIERTYPE *pSoldier);
INT16 GetDirectionToGridNoFromGridNo(INT16 sGridNoDest, INT16 sGridNoSrc);
void ReleaseSoldiersAttacker(struct SOLDIERTYPE *pSoldier);
BOOLEAN MercInWater(struct SOLDIERTYPE *pSoldier);
uint16_t GetNewSoldierStateFromNewStance(struct SOLDIERTYPE *pSoldier, uint8_t ubDesiredStance);
uint16_t GetMoveStateBasedOnStance(struct SOLDIERTYPE *pSoldier, uint8_t ubStanceHeight);
void SoldierGotoStationaryStance(struct SOLDIERTYPE *pSoldier);
BOOLEAN ReCreateSoldierLight(struct SOLDIERTYPE *pSoldier);

BOOLEAN DoMercBattleSound(struct SOLDIERTYPE *pSoldier, uint8_t ubBattleSoundID);
BOOLEAN InternalDoMercBattleSound(struct SOLDIERTYPE *pSoldier, uint8_t ubBattleSoundID,
                                  INT8 bSpecialCode);

uint32_t SoldierDressWound(struct SOLDIERTYPE *pSoldier, struct SOLDIERTYPE *pVictim, INT16 sKitPts,
                           INT16 sStatus);
void ReceivingSoldierCancelServices(struct SOLDIERTYPE *pSoldier);
void GivingSoldierCancelServices(struct SOLDIERTYPE *pSoldier);
void InternalReceivingSoldierCancelServices(struct SOLDIERTYPE *pSoldier, BOOLEAN fPlayEndAnim);
void InternalGivingSoldierCancelServices(struct SOLDIERTYPE *pSoldier, BOOLEAN fPlayEndAnim);

// WRAPPERS FOR SOLDIER EVENTS
void SendSoldierPositionEvent(struct SOLDIERTYPE *pSoldier, FLOAT dNewXPos, FLOAT dNewYPos);
void SendSoldierDestinationEvent(struct SOLDIERTYPE *pSoldier, uint16_t usNewDestination);
void SendGetNewSoldierPathEvent(struct SOLDIERTYPE *pSoldier, uint16_t sDestGridNo,
                                uint16_t usMovementAnim);
void SendSoldierSetDirectionEvent(struct SOLDIERTYPE *pSoldier, uint16_t usNewDirection);
void SendSoldierSetDesiredDirectionEvent(struct SOLDIERTYPE *pSoldier, uint16_t usDesiredDirection);
void SendChangeSoldierStanceEvent(struct SOLDIERTYPE *pSoldier, uint8_t ubNewStance);
void SendBeginFireWeaponEvent(struct SOLDIERTYPE *pSoldier, INT16 sTargetGridNo);

void HandleAnimationProfile(struct SOLDIERTYPE *pSoldier, uint16_t usAnimState, BOOLEAN fRemove);
BOOLEAN GetProfileFlagsFromGridno(struct SOLDIERTYPE *pSoldier, uint16_t usAnimState,
                                  uint16_t sTestGridNo, uint16_t *usFlags);

void HaultSoldierFromSighting(struct SOLDIERTYPE *pSoldier, BOOLEAN fFromSightingEnemy);
void ReLoadSoldierAnimationDueToHandItemChange(struct SOLDIERTYPE *pSoldier, uint16_t usOldItem,
                                               uint16_t usNewItem);

BOOLEAN CheckForBreathCollapse(struct SOLDIERTYPE *pSoldier);

BOOLEAN PreloadSoldierBattleSounds(struct SOLDIERTYPE *pSoldier, BOOLEAN fRemove);

#define PTR_CIVILIAN (pSoldier->bTeam == CIV_TEAM)
#define PTR_CROUCHED (gAnimControl[pSoldier->usAnimState].ubHeight == ANIM_CROUCH)
#define PTR_STANDING (gAnimControl[pSoldier->usAnimState].ubHeight == ANIM_STAND)
#define PTR_PRONE (gAnimControl[pSoldier->usAnimState].ubHeight == ANIM_PRONE)

void EVENT_SoldierBeginGiveItem(struct SOLDIERTYPE *pSoldier);

void DoNinjaAttack(struct SOLDIERTYPE *pSoldier);

BOOLEAN SoldierCarriesTwoHandedWeapon(struct SOLDIERTYPE *pSoldier);
BOOLEAN InternalSoldierReadyWeapon(struct SOLDIERTYPE *pSoldier, uint8_t sFacingDir,
                                   BOOLEAN fEndReady);

void RemoveSoldierFromGridNo(struct SOLDIERTYPE *pSoldier);

void PositionSoldierLight(struct SOLDIERTYPE *pSoldier);

void SetCheckSoldierLightFlag(struct SOLDIERTYPE *pSoldier);

void EVENT_InternalSetSoldierDestination(struct SOLDIERTYPE *pSoldier, uint16_t usNewDirection,
                                         BOOLEAN fFromMove, uint16_t usAnimState);

void ChangeToFlybackAnimation(struct SOLDIERTYPE *pSoldier, INT8 bDirection);
void ChangeToFallbackAnimation(struct SOLDIERTYPE *pSoldier, INT8 bDirection);

// reset soldier timers
void ResetSoldierChangeStatTimer(struct SOLDIERTYPE *pSoldier);

void EVENT_SoldierBeginKnifeThrowAttack(struct SOLDIERTYPE *pSoldier, INT16 sGridNo,
                                        uint8_t ubDirection);
void EVENT_SoldierBeginUseDetonator(struct SOLDIERTYPE *pSoldier);
void EVENT_SoldierBeginDropBomb(struct SOLDIERTYPE *pSoldier);
void EVENT_SoldierEnterVehicle(struct SOLDIERTYPE *pSoldier, INT16 sGridNo, uint8_t ubDirection);

void SetSoldierCowerState(struct SOLDIERTYPE *pSoldier, BOOLEAN fOn);

BOOLEAN PlayerSoldierStartTalking(struct SOLDIERTYPE *pSoldier, uint8_t ubTargetID,
                                  BOOLEAN fValidate);

void EVENT_InternalSetSoldierPosition(struct SOLDIERTYPE *pSoldier, FLOAT dNewXPos, FLOAT dNewYPos,
                                      BOOLEAN fUpdateDest, BOOLEAN fUpdateFinalDest,
                                      BOOLEAN fForceDelete);

void InternalRemoveSoldierFromGridNo(struct SOLDIERTYPE *pSoldier, BOOLEAN fForce);

void EVENT_SetSoldierPositionAndMaybeFinalDest(struct SOLDIERTYPE *pSoldier, FLOAT dNewXPos,
                                               FLOAT dNewYPos, BOOLEAN fUpdateFinalDest);

void EVENT_SetSoldierPositionForceDelete(struct SOLDIERTYPE *pSoldier, FLOAT dNewXPos,
                                         FLOAT dNewYPos);

void CalcNewActionPoints(struct SOLDIERTYPE *pSoldier);

BOOLEAN InternalIsValidStance(struct SOLDIERTYPE *pSoldier, INT8 bDirection, INT8 bNewStance);

void AdjustNoAPToFinishMove(struct SOLDIERTYPE *pSoldier, BOOLEAN fSet);

void UpdateRobotControllerGivenController(struct SOLDIERTYPE *pSoldier);
void UpdateRobotControllerGivenRobot(struct SOLDIERTYPE *pSoldier);
struct SOLDIERTYPE *GetRobotController(struct SOLDIERTYPE *pSoldier);
BOOLEAN CanRobotBeControlled(struct SOLDIERTYPE *pSoldier);
BOOLEAN ControllingRobot(struct SOLDIERTYPE *pSoldier);

void EVENT_SoldierBeginReloadRobot(struct SOLDIERTYPE *pSoldier, INT16 sGridNo, uint8_t ubDirection,
                                   uint8_t ubMercSlot);

void EVENT_SoldierBeginTakeBlood(struct SOLDIERTYPE *pSoldier, INT16 sGridNo, uint8_t ubDirection);

void EVENT_SoldierBeginAttachCan(struct SOLDIERTYPE *pSoldier, INT16 sGridNo, uint8_t ubDirection);

void HandleSoldierTakeDamageFeedback(struct SOLDIERTYPE *pSoldier);

void PickDropItemAnimation(struct SOLDIERTYPE *pSoldier);

BOOLEAN IsValidSecondHandShot(struct SOLDIERTYPE *pSoldier);
BOOLEAN IsValidSecondHandShotForReloadingPurposes(struct SOLDIERTYPE *pSoldier);

void CrowsFlyAway(uint8_t ubTeam);

void DebugValidateSoldierData();

void BeginTyingToFall(struct SOLDIERTYPE *pSoldier);

void SetSoldierAsUnderAiControl(struct SOLDIERTYPE *pSoldier);
void HandlePlayerTogglingLightEffects(BOOLEAN fToggleValue);

#endif
