#include "Tactical/Points.h"

#include "SGP/Debug.h"
#include "SGP/Types.h"
#include "SGP/WCheck.h"
#include "Tactical/AnimationControl.h"
#include "Tactical/DialogueControl.h"
#include "Tactical/DrugsAndAlcohol.h"
#include "Tactical/HandleUI.h"
#include "Tactical/Interface.h"
#include "Tactical/InterfaceItems.h"
#include "Tactical/Items.h"
#include "Tactical/Overhead.h"
#include "Tactical/PathAI.h"
#include "Tactical/RTTimeDefines.h"
#include "Tactical/SkillCheck.h"
#include "Tactical/SoldierControl.h"
#include "Tactical/SoldierMacros.h"
#include "Tactical/SoldierProfile.h"
#include "Tactical/StructureWrap.h"
#include "Tactical/Weapons.h"
#include "TacticalAI/AI.h"
#include "TileEngine/IsometricUtils.h"
#include "TileEngine/TileDef.h"
#include "TileEngine/WorldMan.h"
#include "Utils/FontControl.h"
#include "Utils/Message.h"
#include "Utils/Text.h"

extern BOOLEAN IsValidSecondHandShot(struct SOLDIERTYPE *pSoldier);

int16_t GetBreathPerAP(struct SOLDIERTYPE *pSoldier, uint16_t usAnimState);

int16_t TerrainActionPoints(struct SOLDIERTYPE *pSoldier, int16_t sGridno, int8_t bDir,
                            int8_t bLevel) {
  int16_t sAPCost = 0;
  int16_t sSwitchValue;
  BOOLEAN fHiddenStructVisible;  // Used for hidden struct visiblity

  if (pSoldier->bStealthMode) sAPCost += AP_STEALTH_MODIFIER;

  if (pSoldier->bReverse || gUIUseReverse) sAPCost += AP_REVERSE_MODIFIER;

  // if (GridCost[gridno] == NPCMINECOST)
  //   switchValue = BackupGridCost[gridno];
  // else

  sSwitchValue = gubWorldMovementCosts[sGridno][bDir][bLevel];

  // Check reality vs what the player knows....
  if (pSoldier->bTeam == gbPlayerNum) {
    // Is this obstcale a hidden tile that has not been revealed yet?
    if (DoesGridnoContainHiddenStruct((uint16_t)sGridno, &fHiddenStructVisible)) {
      // Are we not visible, if so use terrain costs!
      if (!fHiddenStructVisible) {
        // Set cost of terrain!
        sSwitchValue = gTileTypeMovementCost[gpWorldLevelData[sGridno].ubTerrainID];
      }
    }
  }
  if (sSwitchValue == TRAVELCOST_NOT_STANDING) {
    // use the cost of the terrain!
    sSwitchValue = gTileTypeMovementCost[gpWorldLevelData[sGridno].ubTerrainID];
  } else if (IS_TRAVELCOST_DOOR(sSwitchValue)) {
    sSwitchValue = DoorTravelCost(pSoldier, sGridno, (uint8_t)sSwitchValue,
                                  (BOOLEAN)(pSoldier->bTeam == gbPlayerNum), NULL);
  }

  if (sSwitchValue >= TRAVELCOST_BLOCKED && sSwitchValue != TRAVELCOST_DOOR) {
    return (100);  // Cost too much to be considered!
  }

  switch (sSwitchValue) {
    case TRAVELCOST_DIRTROAD:
    case TRAVELCOST_FLAT:
      sAPCost += AP_MOVEMENT_FLAT;
      break;
      // case TRAVELCOST_BUMPY		:
    case TRAVELCOST_GRASS:
      sAPCost += AP_MOVEMENT_GRASS;
      break;
    case TRAVELCOST_THICK:
      sAPCost += AP_MOVEMENT_BUSH;
      break;
    case TRAVELCOST_DEBRIS:
      sAPCost += AP_MOVEMENT_RUBBLE;
      break;
    case TRAVELCOST_SHORE:
      sAPCost += AP_MOVEMENT_SHORE;  // wading shallow water
      break;
    case TRAVELCOST_KNEEDEEP:
      sAPCost += AP_MOVEMENT_LAKE;  // wading waist/chest deep - very slow
      break;

    case TRAVELCOST_DEEPWATER:
      sAPCost += AP_MOVEMENT_OCEAN;  // can swim, so it's faster than wading
      break;
      /*
         case TRAVELCOST_VEINEND	:
         case TRAVELCOST_VEINMID	: sAPCost += AP_MOVEMENT_FLAT;
                                                                                                                              break;
      */
    case TRAVELCOST_DOOR:
      sAPCost += AP_MOVEMENT_FLAT;
      break;

      // cost for jumping a fence REPLACES all other AP costs!
    case TRAVELCOST_FENCE:
      return (AP_JUMPFENCE);

    case TRAVELCOST_NONE:
      return (0);

    default:

      DebugMsg(TOPIC_JA2, DBG_LEVEL_3,
               String("Calc AP: Unrecongnized MP type %d in %d, direction %d", sSwitchValue,
                      sGridno, bDir));
      break;
  }

  if (bDir & 1) {
    sAPCost = (sAPCost * 14) / 10;
  }

  return (sAPCost);
}

int16_t BreathPointAdjustmentForCarriedWeight(struct SOLDIERTYPE *pSoldier) {
  uint32_t uiCarriedPercent;
  uint32_t uiPercentCost;

  uiCarriedPercent = CalculateCarriedWeight(pSoldier);
  if (uiCarriedPercent < 101) {
    // normal BP costs
    uiPercentCost = 100;
  } else {
    if (uiCarriedPercent < 151) {
      // between 101 and 150% of max carried weight, extra BP cost
      // of 1% per % above 100... so at 150%, we pay 150%
      uiPercentCost = 100 + (uiCarriedPercent - 100) * 3;
    } else if (uiCarriedPercent < 201) {
      // between 151 and 200% of max carried weight, extra BP cost
      // of 2% per % above 150... so at 200%, we pay 250%
      uiPercentCost = 100 + (uiCarriedPercent - 100) * 3 + (uiCarriedPercent - 150);
    } else {
      // over 200%, extra BP cost of 3% per % above 200
      uiPercentCost =
          100 + (uiCarriedPercent - 100) * 3 + (uiCarriedPercent - 150) + (uiCarriedPercent - 200);
      // so at 250% weight, we pay 400% breath!
    }
  }
  return ((int16_t)uiPercentCost);
}

int16_t TerrainBreathPoints(struct SOLDIERTYPE *pSoldier, int16_t sGridno, int8_t bDir,
                            uint16_t usMovementMode) {
  int32_t iPoints = 0;
  uint8_t ubMovementCost;

  ubMovementCost = gubWorldMovementCosts[sGridno][bDir][0];

  switch (ubMovementCost) {
    case TRAVELCOST_DIRTROAD:
    case TRAVELCOST_FLAT:
      iPoints = BP_MOVEMENT_FLAT;
      break;
      // case TRAVELCOST_BUMPY			:
    case TRAVELCOST_GRASS:
      iPoints = BP_MOVEMENT_GRASS;
      break;
    case TRAVELCOST_THICK:
      iPoints = BP_MOVEMENT_BUSH;
      break;
    case TRAVELCOST_DEBRIS:
      iPoints = BP_MOVEMENT_RUBBLE;
      break;
    case TRAVELCOST_SHORE:
      iPoints = BP_MOVEMENT_SHORE;
      break;  // wading shallow water
    case TRAVELCOST_KNEEDEEP:
      iPoints = BP_MOVEMENT_LAKE;
      break;  // wading waist/chest deep - very slow
    case TRAVELCOST_DEEPWATER:
      iPoints = BP_MOVEMENT_OCEAN;
      break;  // can swim, so it's faster than wading
              //  case TRAVELCOST_VEINEND		:
              //  case TRAVELCOST_VEINMID		: iPoints = BP_MOVEMENT_FLAT;
              //  break;
    default:
      if (IS_TRAVELCOST_DOOR(ubMovementCost)) {
        iPoints = BP_MOVEMENT_FLAT;
        break;
      }
      /*
      #ifdef TESTVERSION
           NumMessage("ERROR: TerrainBreathPoints: Unrecognized grid cost = ",
                                                              GridCost[gridno]);
      #endif
      */
      return (0);
  }

  iPoints = iPoints * BreathPointAdjustmentForCarriedWeight(pSoldier) / 100;

  // ATE - MAKE MOVEMENT ALWAYS WALK IF IN WATER
  if (gpWorldLevelData[sGridno].ubTerrainID == DEEP_WATER ||
      gpWorldLevelData[sGridno].ubTerrainID == MED_WATER ||
      gpWorldLevelData[sGridno].ubTerrainID == LOW_WATER) {
    usMovementMode = WALKING;
  }

  // so, then we must modify it for other movement styles and accumulate
  switch (usMovementMode) {
    case RUNNING:
    case ADULTMONSTER_WALKING:
    case BLOODCAT_RUN:

      iPoints *= BP_RUN_ENERGYCOSTFACTOR;
      break;

    case SIDE_STEP:
    case WALK_BACKWARDS:
    case BLOODCAT_WALK_BACKWARDS:
    case MONSTER_WALK_BACKWARDS:
    case WALKING:
      iPoints *= BP_WALK_ENERGYCOSTFACTOR;
      break;

    case START_SWAT:
    case SWATTING:
    case SWAT_BACKWARDS:
      iPoints *= BP_SWAT_ENERGYCOSTFACTOR;
      break;
    case CRAWLING:
      iPoints *= BP_CRAWL_ENERGYCOSTFACTOR;
      break;
  }

  // ATE: Adjust these by realtime movement
  if (!(gTacticalStatus.uiFlags & TURNBASED) || !(gTacticalStatus.uiFlags & INCOMBAT)) {
    // ATE: ADJUST FOR RT - MAKE BREATH GO A LITTLE FASTER!
    iPoints = (int32_t)(iPoints * TB_BREATH_DEDUCT_MODIFIER);
  }

  return ((int16_t)iPoints);
}

int16_t ActionPointCost(struct SOLDIERTYPE *pSoldier, int16_t sGridNo, int8_t bDir,
                        uint16_t usMovementMode) {
  int16_t sTileCost, sPoints, sSwitchValue;

  sPoints = 0;

  // get the tile cost for that tile based on WALKING
  sTileCost = TerrainActionPoints(pSoldier, sGridNo, bDir, pSoldier->bLevel);

  // Get switch value...
  sSwitchValue = gubWorldMovementCosts[sGridNo][bDir][pSoldier->bLevel];

  // Tile cost should not be reduced based on movement mode...
  if (sSwitchValue == TRAVELCOST_FENCE) {
    return (sTileCost);
  }

  // ATE - MAKE MOVEMENT ALWAYS WALK IF IN WATER
  if (gpWorldLevelData[sGridNo].ubTerrainID == DEEP_WATER ||
      gpWorldLevelData[sGridNo].ubTerrainID == MED_WATER ||
      gpWorldLevelData[sGridNo].ubTerrainID == LOW_WATER) {
    usMovementMode = WALKING;
  }

  // so, then we must modify it for other movement styles and accumulate
  if (sTileCost > 0) {
    switch (usMovementMode) {
      case RUNNING:
      case ADULTMONSTER_WALKING:
      case BLOODCAT_RUN:
        sPoints = (int16_t)(double)((sTileCost / RUNDIVISOR));
        break;

      case CROW_FLY:
      case SIDE_STEP:
      case WALK_BACKWARDS:
      case ROBOT_WALK:
      case BLOODCAT_WALK_BACKWARDS:
      case MONSTER_WALK_BACKWARDS:
      case LARVAE_WALK:
      case WALKING:
        sPoints = (sTileCost + WALKCOST);
        break;

      case START_SWAT:
      case SWAT_BACKWARDS:
      case SWATTING:
        sPoints = (sTileCost + SWATCOST);
        break;
      case CRAWLING:
        sPoints = (sTileCost + CRAWLCOST);
        break;

      default:

        // Invalid movement mode
        DebugMsg(TOPIC_JA2, DBG_LEVEL_3,
                 String("Invalid movement mode %d used in ActionPointCost", usMovementMode));
        sPoints = 1;
    }
  }

  if (sSwitchValue == TRAVELCOST_NOT_STANDING) {
    switch (usMovementMode) {
      case RUNNING:
      case WALKING:
      case LARVAE_WALK:
      case SIDE_STEP:
      case WALK_BACKWARDS:
        // charge crouch APs for ducking head!
        sPoints += AP_CROUCH;
        break;

      default:
        break;
    }
  }

  return (sPoints);
}

int16_t EstimateActionPointCost(struct SOLDIERTYPE *pSoldier, int16_t sGridNo, int8_t bDir,
                                uint16_t usMovementMode, int8_t bPathIndex, int8_t bPathLength) {
  // This action point cost code includes the penalty for having to change
  // stance after jumping a fence IF our path continues...
  int16_t sTileCost, sPoints, sSwitchValue;
  sPoints = 0;

  // get the tile cost for that tile based on WALKING
  sTileCost = TerrainActionPoints(pSoldier, sGridNo, bDir, pSoldier->bLevel);

  // so, then we must modify it for other movement styles and accumulate
  if (sTileCost > 0) {
    switch (usMovementMode) {
      case RUNNING:
      case ADULTMONSTER_WALKING:
      case BLOODCAT_RUN:
        sPoints = (int16_t)(double)((sTileCost / RUNDIVISOR));
        break;

      case CROW_FLY:
      case SIDE_STEP:
      case ROBOT_WALK:
      case WALK_BACKWARDS:
      case BLOODCAT_WALK_BACKWARDS:
      case MONSTER_WALK_BACKWARDS:
      case LARVAE_WALK:
      case WALKING:
        sPoints = (sTileCost + WALKCOST);
        break;

      case START_SWAT:
      case SWAT_BACKWARDS:
      case SWATTING:
        sPoints = (sTileCost + SWATCOST);
        break;
      case CRAWLING:
        sPoints = (sTileCost + CRAWLCOST);
        break;

      default:

        // Invalid movement mode
        DebugMsg(TOPIC_JA2, DBG_LEVEL_3,
                 String("Invalid movement mode %d used in ActionPointCost", usMovementMode));
        sPoints = 1;
    }
  }

  // Get switch value...
  sSwitchValue = gubWorldMovementCosts[sGridNo][bDir][pSoldier->bLevel];

  // ATE: If we have a 'special cost, like jump fence...
  if (sSwitchValue == TRAVELCOST_FENCE) {
    // If we are changeing stance ( either before or after getting there....
    // We need to reflect that...
    switch (usMovementMode) {
      case SIDE_STEP:
      case WALK_BACKWARDS:
      case RUNNING:
      case WALKING:

        // Add here cost to go from crouch to stand AFTER fence hop....
        // Since it's AFTER.. make sure we will be moving after jump...
        if ((bPathIndex + 2) < bPathLength) {
          sPoints += AP_CROUCH;
        }
        break;

      case SWATTING:
      case START_SWAT:
      case SWAT_BACKWARDS:

        // Add cost to stand once there BEFORE....
        sPoints += AP_CROUCH;
        break;

      case CRAWLING:

        // Can't do it here.....
        break;
    }
  } else if (sSwitchValue == TRAVELCOST_NOT_STANDING) {
    switch (usMovementMode) {
      case RUNNING:
      case WALKING:
      case SIDE_STEP:
      case WALK_BACKWARDS:
        // charge crouch APs for ducking head!
        sPoints += AP_CROUCH;
        break;

      default:
        break;
    }
  }

  return (sPoints);
}

BOOLEAN EnoughPoints(struct SOLDIERTYPE *pSoldier, int16_t sAPCost, int16_t sBPCost,
                     BOOLEAN fDisplayMsg) {
  int16_t sNewAP = 0;

  // If this guy is on a special move... don't care about APS, OR BPSs!
  if (pSoldier->ubWaitActionToDo) {
    return (TRUE);
  }

  if (pSoldier->ubQuoteActionID >= QUOTE_ACTION_ID_TRAVERSE_EAST &&
      pSoldier->ubQuoteActionID <= QUOTE_ACTION_ID_TRAVERSE_NORTH) {
    // AI guy on special move off map
    return (TRUE);
  }

  // IN realtime.. only care about BPs
  if ((gTacticalStatus.uiFlags & REALTIME) || !(gTacticalStatus.uiFlags & INCOMBAT)) {
    sAPCost = 0;
  }

#ifdef NETWORKED
  if (!IsTheSolderUnderMyControl(pSoldier->ubID)) {
    return (TRUE);
  }
#endif

  // Get New points
  sNewAP = pSoldier->bActionPoints - sAPCost;

  // If we cannot deduct points, return FALSE
  if (sNewAP < 0) {
    // Display message if it's our own guy
    if (pSoldier->bTeam == gbPlayerNum && fDisplayMsg) {
      ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, TacticalStr[NOT_ENOUGH_APS_STR]);
    }
    return (FALSE);
  }

  return (TRUE);
}

void DeductPoints(struct SOLDIERTYPE *pSoldier, int16_t sAPCost, int16_t sBPCost) {
  int16_t sNewAP = 0;
  int8_t bNewBreath;

  // in real time, there IS no AP cost, (only breath cost)
  if (!(gTacticalStatus.uiFlags & TURNBASED) || !(gTacticalStatus.uiFlags & INCOMBAT)) {
    sAPCost = 0;
  }

  // Get New points
  sNewAP = pSoldier->bActionPoints - sAPCost;

  // If this is the first time with no action points, set UI flag
  if (sNewAP <= 0 && pSoldier->bActionPoints > 0) {
    pSoldier->fUIFirstTimeNOAP = TRUE;
    fInterfacePanelDirty = TRUE;
  }

  // If we cannot deduct points, return FALSE
  if (sNewAP < 0) {
    sNewAP = 0;
  }

  pSoldier->bActionPoints = (int8_t)sNewAP;

  DebugMsg(TOPIC_JA2, DBG_LEVEL_3,
           String("Deduct Points (%d at %d) %d %d", GetSolID(pSoldier), pSoldier->sGridNo, sAPCost,
                  sBPCost));

  if (AM_A_ROBOT(pSoldier)) {
    // zap all breath costs for robot
    sBPCost = 0;
  }

  // is there a BREATH deduction/transaction to be made?  (REMEMBER: could be a GAIN!)
  if (sBPCost) {
    // Adjust breath changes due to spending or regaining of energy
    sBPCost = AdjustBreathPts(pSoldier, sBPCost);
    sBPCost *= -1;

    pSoldier->sBreathRed -= sBPCost;

    // CJC: moved check for high breathred to below so that negative breath can be detected

    // cap breathred
    if (pSoldier->sBreathRed < 0) {
      pSoldier->sBreathRed = 0;
    }
    if (pSoldier->sBreathRed > 10000) {
      pSoldier->sBreathRed = 10000;
    }

    // Get new breath
    bNewBreath = (uint8_t)(pSoldier->bBreathMax - ((float)pSoldier->sBreathRed / (float)100));

    if (bNewBreath > 100) {
      bNewBreath = 100;
    }
    if (bNewBreath < 00) {
      // Take off 1 AP per 5 breath... rem adding a negative subtracts
      pSoldier->bActionPoints += (bNewBreath / 5);
      if (pSoldier->bActionPoints < 0) {
        pSoldier->bActionPoints = 0;
      }

      bNewBreath = 0;
    }

    if (bNewBreath > pSoldier->bBreathMax) {
      bNewBreath = pSoldier->bBreathMax;
    }
    pSoldier->bBreath = bNewBreath;
  }

  // UPDATE BAR
  DirtyMercPanelInterface(pSoldier, DIRTYLEVEL1);
}

int16_t AdjustBreathPts(struct SOLDIERTYPE *pSold, int16_t sBPCost) {
  int16_t sBreathFactor = 100;
  uint8_t ubBandaged;

  // NumMessage("BEFORE adjustments, BREATH PTS = ",breathPts);

  // in real time, there IS no AP cost, (only breath cost)
  /*
  if (!(gTacticalStatus.uiFlags & TURNBASED) || !(gTacticalStatus.uiFlags & INCOMBAT ) )
  {
          // ATE: ADJUST FOR RT - MAKE BREATH GO A LITTLE FASTER!
          sBPCost	*= TB_BREATH_DEDUCT_MODIFIER;
  }
  */

  // adjust breath factor for current breath deficiency
  sBreathFactor += (100 - pSold->bBreath);

  // adjust breath factor for current life deficiency (but add 1/2 bandaging)
  ubBandaged = pSold->bLifeMax - pSold->bLife - pSold->bBleeding;
  // sBreathFactor += (pSold->bLifeMax - (pSold->bLife + (ubBandaged / 2)));
  sBreathFactor += 100 * (pSold->bLifeMax - (pSold->bLife + (ubBandaged / 2))) / pSold->bLifeMax;

  if (pSold->bStrength > 80) {
    // give % reduction to breath costs for high strength mercs
    sBreathFactor -= (pSold->bStrength - 80) / 2;
  }

  /*	THIS IS OLD JAGGED ALLIANCE STUFF (left for possible future reference)

   // apply penalty due to high temperature, heat, and hot Metaviran sun
   // if INDOORS, in DEEP WATER, or possessing HEAT TOLERANCE trait
   if ((ptr->terrtype == FLOORTYPE) || (ptr->terr >= OCEAN21) ||
                                         (ptr->trait == HEAT_TOLERANT))
     breathFactor += (Status.heatFactor / 5);	// 20% of normal heat penalty
   else
     breathFactor += Status.heatFactor;		// not used to this!
  */

  // if a non-swimmer type is thrashing around in deep water
  if ((pSold->ubProfile != NO_PROFILE) &&
      (gMercProfiles[pSold->ubProfile].bPersonalityTrait == NONSWIMMER)) {
    if (pSold->usAnimState == DEEP_WATER_TRED || pSold->usAnimState == DEEP_WATER_SWIM) {
      sBreathFactor *= 5;  // lose breath 5 times faster in deep water!
    }
  }

  if (sBreathFactor == 0) {
    sBPCost = 0;
  } else if (sBPCost > 0)  // breath DECREASE
    // increase breath COST by breathFactor
    sBPCost = ((sBPCost * sBreathFactor) / 100);
  else  // breath INCREASE
    // decrease breath GAIN by breathFactor
    sBPCost = ((sBPCost * 100) / sBreathFactor);

  return (sBPCost);
}

void UnusedAPsToBreath(struct SOLDIERTYPE *pSold) {
  int16_t sUnusedAPs, sBreathPerAP = 0, sBreathChange, sRTBreathMod;

  // Note to Andrew (or whomever else it may concern):

  // This function deals with BETWEEN TURN breath/energy gains. The basic concept is:
  //
  //	- look at LAST (current) animation of merc to see what he's now doing
  //	- look at how many AP remain unspent (indicating duration of time doing that anim)
  //
  //  figure out how much breath/energy (if any) he should recover. Obviously if a merc
  //	is STANDING BREATHING and hasn't spent any AP then it means he *stood around* for
  //  the entire duration of one turn (which, instead of spending energy, REGAINS energy)

  // COMMENTED OUT FOR NOW SINCE MOST OF THE ANIMATION DEFINES DO NOT MATCH

  // If we are not in turn-based combat...

  if (pSold->uiStatusFlags & SOLDIER_VEHICLE) {
    return;
  }

  if (!(gTacticalStatus.uiFlags & TURNBASED) || !(gTacticalStatus.uiFlags & INCOMBAT)) {
    // ALRIGHT, GIVE A FULL AMOUNT BACK, UNLES MODIFIED BY WHAT ACTIONS WE WERE DOING
    sBreathPerAP = GetBreathPerAP(pSold, pSold->usAnimState);

    // adjust for carried weight
    sBreathPerAP = sBreathPerAP * 100 / BreathPointAdjustmentForCarriedWeight(pSold);

    // If this value is -ve, we have a gain, else we have a loos which we should not really do
    // We just want to limit this to no gain if we were doing stuff...
    sBreathChange = 3 * sBreathPerAP;

    // Adjust for on drugs
    HandleBPEffectDueToDrugs(pSold, &sBreathChange);

    if (sBreathChange > 0) {
      sBreathChange = 0;
    } else {
      // We have a gain, now limit this depending on what we were doing...
      // OK for RT, look at how many tiles we have moved, our last move anim
      if (pSold->ubTilesMovedPerRTBreathUpdate > 0) {
        // How long have we done this for?
        // And what anim were we doing?
        sBreathPerAP = GetBreathPerAP(pSold, pSold->usLastMovementAnimPerRTBreathUpdate);

        sRTBreathMod = sBreathPerAP * pSold->ubTilesMovedPerRTBreathUpdate;

        // Deduct some if we were exerting ourselves
        // We add here because to gain breath, sBreathChange needs to be -ve
        if (sRTBreathMod > 0) {
          sBreathChange += sRTBreathMod;
        }

        if (sBreathChange < 0) {
          sBreathChange = 0;
        }
      }
    }

    // Divide by a number to adjust that in realtimer we do not want to recover as
    // as fast as the TB values do
    sBreathChange *= TB_BREATH_RECOVER_MODIFIER;

    // adjust breath only, don't touch action points!
    DeductPoints(pSold, 0, (int16_t)sBreathChange);

    // Reset value for RT breath update
    pSold->ubTilesMovedPerRTBreathUpdate = 0;

  } else {
    // if merc has any APs left unused this turn (that aren't carrying over)
    if (pSold->bActionPoints > MAX_AP_CARRIED) {
      sUnusedAPs = pSold->bActionPoints - MAX_AP_CARRIED;

      sBreathPerAP = GetBreathPerAP(pSold, pSold->usAnimState);

      if (sBreathPerAP < 0) {
        // can't gain any breath when we've just been gassed, OR
        // if standing in tear gas without a gas mask on
        if (pSold->uiStatusFlags & SOLDIER_GASSED) {
          return;  // can't breathe here, so get no breath back!
        }
      }

      // adjust for carried weight
      sBreathPerAP = sBreathPerAP * 100 / BreathPointAdjustmentForCarriedWeight(pSold);

      sBreathChange = (AP_MAXIMUM - sUnusedAPs) * sBreathPerAP;
    } else {
      sBreathChange = 0;
    }
    // Adjust for on drugs
    HandleBPEffectDueToDrugs(pSold, &sBreathChange);

    // adjust breath only, don't touch action points!
    DeductPoints(pSold, 0, (int16_t)sBreathChange);
  }
}

int16_t GetBreathPerAP(struct SOLDIERTYPE *pSoldier, uint16_t usAnimState) {
  int16_t sBreathPerAP = 0;
  BOOLEAN fAnimTypeFound = FALSE;

  if (gAnimControl[usAnimState].uiFlags & ANIM_VARIABLE_EFFORT) {
    // Default effort
    sBreathPerAP = BP_PER_AP_MIN_EFFORT;

    // OK, check if we are in water and are waling/standing
    if (MercInWater(pSoldier)) {
      switch (usAnimState) {
        case STANDING:

          sBreathPerAP = BP_PER_AP_LT_EFFORT;
          break;

        case WALKING:

          sBreathPerAP = BP_PER_AP_MOD_EFFORT;
          break;
      }
    } else {
      switch (usAnimState) {
        case STANDING:

          sBreathPerAP = BP_PER_AP_NO_EFFORT;
          break;

        case WALKING:

          sBreathPerAP = BP_PER_AP_LT_EFFORT;
          break;
      }
    }
    fAnimTypeFound = TRUE;
  }

  if (gAnimControl[usAnimState].uiFlags & ANIM_NO_EFFORT) {
    sBreathPerAP = BP_PER_AP_NO_EFFORT;
    fAnimTypeFound = TRUE;
  }

  if (gAnimControl[usAnimState].uiFlags & ANIM_MIN_EFFORT) {
    sBreathPerAP = BP_PER_AP_MIN_EFFORT;
    fAnimTypeFound = TRUE;
  }

  if (gAnimControl[usAnimState].uiFlags & ANIM_LIGHT_EFFORT) {
    sBreathPerAP = BP_PER_AP_LT_EFFORT;
    fAnimTypeFound = TRUE;
  }

  if (gAnimControl[usAnimState].uiFlags & ANIM_MODERATE_EFFORT) {
    sBreathPerAP = BP_PER_AP_MOD_EFFORT;
    fAnimTypeFound = TRUE;
  }

  if (!fAnimTypeFound) {
    DebugMsg(TOPIC_JA2, DBG_LEVEL_3,
             String("Unknown end-of-turn breath anim: %s", gAnimControl[usAnimState].zAnimStr));
  }

  return (sBreathPerAP);
}

// uint8_t CalcAPsToBurst( int8_t bBaseActionPoints, uint16_t usItem )
uint8_t CalcAPsToBurst(int8_t bBaseActionPoints, struct OBJECTTYPE *pObj) {
  // base APs is what you'd get from CalcActionPoints();
  if (pObj->usItem == G11) {
    return (1);
  } else {
    // NB round UP, so 21-25 APs pay full

    int8_t bAttachPos;

    bAttachPos = FindAttachment(pObj, SPRING_AND_BOLT_UPGRADE);
    if (bAttachPos != -1) {
      return ((max(3, (AP_BURST * bBaseActionPoints + (AP_MAXIMUM - 1)) / AP_MAXIMUM) * 100) /
              (100 + pObj->bAttachStatus[bAttachPos] / 5));
    } else {
      return (max(3, (AP_BURST * bBaseActionPoints + (AP_MAXIMUM - 1)) / AP_MAXIMUM));
    }
  }
}

uint8_t CalcTotalAPsToAttack(struct SOLDIERTYPE *pSoldier, int16_t sGridNo,
                             uint8_t ubAddTurningCost, int8_t bAimTime) {
  uint16_t sAPCost = 0;
  uint16_t usItemNum;
  int16_t sActionGridNo;
  uint8_t ubDirection;
  int16_t sAdjustedGridNo;
  uint32_t uiItemClass;

  // LOOK IN BUDDY'S HAND TO DETERMINE WHAT TO DO HERE
  usItemNum = pSoldier->inv[HANDPOS].usItem;
  uiItemClass = Item[usItemNum].usItemClass;

  if (uiItemClass == IC_GUN || uiItemClass == IC_LAUNCHER || uiItemClass == IC_TENTACLES ||
      uiItemClass == IC_THROWING_KNIFE) {
    sAPCost = MinAPsToAttack(pSoldier, sGridNo, ubAddTurningCost);

    if (pSoldier->bDoBurst) {
      sAPCost += CalcAPsToBurst(CalcActionPoints(pSoldier), &(pSoldier->inv[HANDPOS]));
    } else {
      sAPCost += bAimTime;
    }
  }

  // ATE: HERE, need to calculate APs!
  if (uiItemClass & IC_EXPLOSV) {
    sAPCost = MinAPsToAttack(pSoldier, sGridNo, ubAddTurningCost);

    sAPCost = 5;
  }

  if (uiItemClass == IC_PUNCH || (uiItemClass == IC_BLADE && uiItemClass != IC_THROWING_KNIFE)) {
    // IF we are at this gridno, calc min APs but if not, calc cost to goto this lication
    if (pSoldier->sGridNo != sGridNo) {
      // OK, in order to avoid path calculations here all the time... save and check if it's
      // changed!
      if (pSoldier->sWalkToAttackGridNo == sGridNo) {
        sAdjustedGridNo = sGridNo;
        sAPCost += (uint8_t)(pSoldier->sWalkToAttackWalkToCost);
      } else {
        // int32_t		cnt;
        // int16_t		sSpot;
        uint8_t ubGuyThere;
        int16_t sGotLocation = NOWHERE;
        BOOLEAN fGotAdjacent = FALSE;
        struct SOLDIERTYPE *pTarget;

        ubGuyThere = WhoIsThere2(sGridNo, pSoldier->bLevel);

        if (ubGuyThere != NOBODY) {
          pTarget = MercPtrs[ubGuyThere];

          if (pSoldier->ubBodyType == BLOODCAT) {
            sGotLocation = FindNextToAdjacentGridEx(pSoldier, sGridNo, &ubDirection,
                                                    &sAdjustedGridNo, TRUE, FALSE);
            if (sGotLocation == -1) {
              sGotLocation = NOWHERE;
            }
          } else {
            sGotLocation =
                FindAdjacentPunchTarget(pSoldier, pTarget, &sAdjustedGridNo, &ubDirection);
          }
        }

        if (sGotLocation == NOWHERE && pSoldier->ubBodyType != BLOODCAT) {
          sActionGridNo =
              FindAdjacentGridEx(pSoldier, sGridNo, &ubDirection, &sAdjustedGridNo, TRUE, FALSE);

          if (sActionGridNo == -1) {
            sGotLocation = NOWHERE;
          } else {
            sGotLocation = sActionGridNo;
          }
          fGotAdjacent = TRUE;
        }

        if (sGotLocation != NOWHERE) {
          if (pSoldier->sGridNo == sGotLocation || !fGotAdjacent) {
            pSoldier->sWalkToAttackWalkToCost = 0;
          } else {
            // Save for next time...
            pSoldier->sWalkToAttackWalkToCost =
                PlotPath(pSoldier, sGotLocation, NO_COPYROUTE, NO_PLOT, TEMPORARY,
                         (uint16_t)pSoldier->usUIMovementMode, NOT_STEALTH, FORWARD,
                         pSoldier->bActionPoints);

            if (pSoldier->sWalkToAttackWalkToCost == 0) {
              return (99);
            }
          }
        } else {
          return (0);
        }
        sAPCost += pSoldier->sWalkToAttackWalkToCost;
      }

      // Save old location!
      pSoldier->sWalkToAttackGridNo = sGridNo;

      // Add points to attack
      sAPCost += MinAPsToAttack(pSoldier, sAdjustedGridNo, ubAddTurningCost);
    } else {
      // Add points to attack
      // Use our gridno
      sAPCost += MinAPsToAttack(pSoldier, sGridNo, ubAddTurningCost);
    }

    // Add aim time...
    sAPCost += bAimTime;
  }

  return ((int8_t)sAPCost);
}

uint8_t MinAPsToAttack(struct SOLDIERTYPE *pSoldier, int16_t sGridno, uint8_t ubAddTurningCost) {
  uint16_t sAPCost = 0;
  uint32_t uiItemClass;

  if (pSoldier->bWeaponMode == WM_ATTACHED) {
    int8_t bAttachSlot;
    // look for an attached grenade launcher

    bAttachSlot = FindAttachment(&(pSoldier->inv[HANDPOS]), UNDER_GLAUNCHER);
    if (bAttachSlot == NO_SLOT) {
      // default to hand
      // LOOK IN BUDDY'S HAND TO DETERMINE WHAT TO DO HERE
      uiItemClass = Item[pSoldier->inv[HANDPOS].usItem].usItemClass;
    } else {
      uiItemClass = Item[UNDER_GLAUNCHER].usItemClass;
    }
  } else {
    // LOOK IN BUDDY'S HAND TO DETERMINE WHAT TO DO HERE
    uiItemClass = Item[pSoldier->inv[HANDPOS].usItem].usItemClass;
  }

  if (uiItemClass == IC_BLADE || uiItemClass == IC_GUN || uiItemClass == IC_LAUNCHER ||
      uiItemClass == IC_TENTACLES || uiItemClass == IC_THROWING_KNIFE) {
    sAPCost = MinAPsToShootOrStab(pSoldier, sGridno, ubAddTurningCost);
  } else if (uiItemClass & (IC_GRENADE | IC_THROWN)) {
    sAPCost = MinAPsToThrow(pSoldier, sGridno, ubAddTurningCost);
  } else if (uiItemClass == IC_PUNCH) {
    sAPCost = MinAPsToPunch(pSoldier, sGridno, ubAddTurningCost);
  }

  return ((uint8_t)sAPCost);
}

int8_t CalcAimSkill(struct SOLDIERTYPE *pSoldier, uint16_t usWeapon) {
  int8_t bAimSkill;

  if (Item[usWeapon].usItemClass == IC_GUN || Item[usWeapon].usItemClass == IC_LAUNCHER) {
    // GUNS: modify aiming cost by shooter's MARKSMANSHIP
    bAimSkill = EffectiveMarksmanship(pSoldier);
  } else
  // for now use this for all other weapons
  // if ( Item[ usInHand ].usItemClass == IC_BLADE )
  {
    // KNIVES: modify aiming cost by avg of attacker's DEXTERITY & AGILITY
    bAimSkill = (EffectiveDexterity(pSoldier) + EffectiveAgility(pSoldier)) / 2;
    // return( 4 );
  }
  return (bAimSkill);
}

uint8_t BaseAPsToShootOrStab(int8_t bAPs, int8_t bAimSkill, struct OBJECTTYPE *pObj) {
  int16_t sTop, sBottom;
  int8_t bAttachPos;

  // Calculate default top & bottom of the magic "aiming" formula!

  // get this man's maximum possible action points (ignoring carryovers)
  // the 2 times is here only to allow rounding off using integer math later
  sTop = 2 * bAPs;  // CalcActionPoints( pSoldier );

  // Shots per turn rating is for max. aimSkill(100), drops down to 1/2 at = 0
  // DIVIDE BY 4 AT THE END HERE BECAUSE THE SHOTS PER TURN IS NOW QUADRUPLED!
  // NB need to define shots per turn for ALL Weapons then.
  sBottom = ((50 + (bAimSkill / 2)) * Weapon[pObj->usItem].ubShotsPer4Turns) / 4;

  bAttachPos = FindAttachment(pObj, SPRING_AND_BOLT_UPGRADE);
  if (bAttachPos != -1) {
    sBottom = (sBottom * (100 + pObj->bAttachStatus[bAttachPos] / 5)) / 100;
  }

  // add minimum aiming time to the overall minimum AP_cost
  //     This here ROUNDS UP fractions of 0.5 or higher using integer math
  //     This works because 'top' is 2x what it really should be throughout
  return ((((100 * sTop) / sBottom) + 1) / 2);
}

void GetAPChargeForShootOrStabWRTGunRaises(struct SOLDIERTYPE *pSoldier, int16_t sGridNo,
                                           uint8_t ubAddTurningCost, BOOLEAN *pfChargeTurning,
                                           BOOLEAN *pfChargeRaise) {
  uint8_t ubDirection;
  uint32_t uiMercFlags;
  uint16_t usTargID;
  BOOLEAN fAddingTurningCost = FALSE;
  BOOLEAN fAddingRaiseGunCost = FALSE;

  if (sGridNo != NOWHERE) {
    // OK, get a direction and see if we need to turn...
    if (ubAddTurningCost) {
      // Given a gridno here, check if we are on a guy - if so - get his gridno
      if (FindSoldier(sGridNo, &usTargID, &uiMercFlags, FIND_SOLDIER_GRIDNO)) {
        sGridNo = MercPtrs[usTargID]->sGridNo;
      }

      ubDirection = (uint8_t)GetDirectionFromGridNo(sGridNo, pSoldier);

      // Is it the same as he's facing?
      if (ubDirection != pSoldier->bDirection) {
        fAddingTurningCost = TRUE;
      }
    }
  } else {
    if (ubAddTurningCost) {
      // Assume we need to add cost!
      fAddingTurningCost = TRUE;
    }
  }

  if (Item[pSoldier->inv[HANDPOS].usItem].usItemClass == IC_THROWING_KNIFE) {
  } else {
    // Do we need to ready weapon?
    if (!(gAnimControl[pSoldier->usAnimState].uiFlags & (ANIM_FIREREADY | ANIM_FIRE))) {
      fAddingRaiseGunCost = TRUE;
    }
  }

  (*pfChargeTurning) = fAddingTurningCost;
  (*pfChargeRaise) = fAddingRaiseGunCost;
}

uint8_t MinAPsToShootOrStab(struct SOLDIERTYPE *pSoldier, int16_t sGridNo,
                            uint8_t ubAddTurningCost) {
  uint32_t uiMercFlags;
  uint16_t usTargID;
  int8_t bFullAPs;
  int8_t bAimSkill;
  uint8_t bAPCost = AP_MIN_AIM_ATTACK;
  BOOLEAN fAddingTurningCost = FALSE;
  BOOLEAN fAddingRaiseGunCost = FALSE;
  uint16_t usItem;

  if (pSoldier->bWeaponMode == WM_ATTACHED) {
    usItem = UNDER_GLAUNCHER;
  } else {
    usItem = pSoldier->inv[HANDPOS].usItem;
  }

  GetAPChargeForShootOrStabWRTGunRaises(pSoldier, sGridNo, ubAddTurningCost, &fAddingTurningCost,
                                        &fAddingRaiseGunCost);

  if (Item[usItem].usItemClass == IC_THROWING_KNIFE) {
    // Do we need to stand up?
    bAPCost += GetAPsToChangeStance(pSoldier, ANIM_STAND);
  }

  // ATE: Look at stance...
  if (gAnimControl[pSoldier->usAnimState].ubHeight == ANIM_STAND) {
    // Don't charge turning if gun-ready...
    if (fAddingRaiseGunCost) {
      fAddingTurningCost = FALSE;
    }
  } else {
    // Just charge turning costs...
    if (fAddingTurningCost) {
      fAddingRaiseGunCost = FALSE;
    }
  }

  if (AM_A_ROBOT(pSoldier)) {
    fAddingRaiseGunCost = FALSE;
  }

  if (fAddingTurningCost) {
    if (Item[usItem].usItemClass == IC_THROWING_KNIFE) {
      bAPCost += 1;
    } else {
      bAPCost += GetAPsToLook(pSoldier);
    }
  }

  if (fAddingRaiseGunCost) {
    bAPCost += GetAPsToReadyWeapon(pSoldier, pSoldier->usAnimState);
    pSoldier->fDontChargeReadyAPs = FALSE;
  }

  if (sGridNo != NOWHERE) {
    // Given a gridno here, check if we are on a guy - if so - get his gridno
    if (FindSoldier(sGridNo, &usTargID, &uiMercFlags, FIND_SOLDIER_GRIDNO)) {
      sGridNo = MercPtrs[usTargID]->sGridNo;
    }
  }

  // if attacking a new target (or if the specific target is uncertain)
  if ((sGridNo != pSoldier->sLastTarget) && usItem != ROCKET_LAUNCHER) {
    bAPCost += AP_CHANGE_TARGET;
  }

  bFullAPs = CalcActionPoints(pSoldier);
  // aim skill is the same whether we are using 1 or 2 guns
  bAimSkill = CalcAimSkill(pSoldier, usItem);

  if (pSoldier->bWeaponMode == WM_ATTACHED) {
    int8_t bAttachSlot;
    struct OBJECTTYPE GrenadeLauncher;

    // look for an attached grenade launcher
    bAttachSlot = FindAttachment(&(pSoldier->inv[HANDPOS]), UNDER_GLAUNCHER);

    // create temporary grenade launcher and use that
    if (bAttachSlot != NO_SLOT) {
      CreateItem(UNDER_GLAUNCHER, pSoldier->inv[HANDPOS].bAttachStatus[bAttachSlot],
                 &GrenadeLauncher);
    } else {
      // fake it, use a 100 status...
      CreateItem(UNDER_GLAUNCHER, 100, &GrenadeLauncher);
    }

    bAPCost += BaseAPsToShootOrStab(bFullAPs, bAimSkill, &GrenadeLauncher);

  } else if (IsValidSecondHandShot(pSoldier)) {
    // charge the maximum of the two
    bAPCost += max(BaseAPsToShootOrStab(bFullAPs, bAimSkill, &(pSoldier->inv[HANDPOS])),
                   BaseAPsToShootOrStab(bFullAPs, bAimSkill, &(pSoldier->inv[SECONDHANDPOS])));
  } else {
    bAPCost += BaseAPsToShootOrStab(bFullAPs, bAimSkill, &(pSoldier->inv[HANDPOS]));
  }

  // the minimum AP cost of ANY shot can NEVER be more than merc's maximum APs!
  if (bAPCost > bFullAPs) bAPCost = bFullAPs;

  // this SHOULD be impossible, but nevertheless...
  if (bAPCost < 1) bAPCost = 1;

  if (pSoldier->inv[HANDPOS].usItem == ROCKET_LAUNCHER) {
    bAPCost += GetAPsToChangeStance(pSoldier, ANIM_STAND);
  }

  return (bAPCost);
}

uint8_t MinAPsToPunch(struct SOLDIERTYPE *pSoldier, int16_t sGridNo, uint8_t ubAddTurningCost) {
  uint8_t bAPCost = 0;
  uint16_t usTargID;
  uint8_t ubDirection;

  //  bAimSkill = ( pSoldier->bDexterity + pSoldier->bAgility) / 2;
  if (sGridNo != NOWHERE) {
    usTargID = WhoIsThere2(sGridNo, pSoldier->bTargetLevel);

    // Given a gridno here, check if we are on a guy - if so - get his gridno
    if (usTargID != NOBODY) {
      sGridNo = MercPtrs[usTargID]->sGridNo;

      // Check if target is prone, if so, calc cost...
      if (gAnimControl[MercPtrs[usTargID]->usAnimState].ubEndHeight == ANIM_PRONE) {
        bAPCost += GetAPsToChangeStance(pSoldier, ANIM_CROUCH);
      } else {
        if (pSoldier->sGridNo == sGridNo) {
          bAPCost += GetAPsToChangeStance(pSoldier, ANIM_STAND);
        }
      }
    }

    if (ubAddTurningCost) {
      if (pSoldier->sGridNo == sGridNo) {
        // ATE: Use standing turn cost....
        ubDirection = (uint8_t)GetDirectionFromGridNo(sGridNo, pSoldier);

        // Is it the same as he's facing?
        if (ubDirection != pSoldier->bDirection) {
          bAPCost += AP_LOOK_STANDING;
        }
      }
    }
  }

  bAPCost += 4;

  return (bAPCost);
}

int8_t MinPtsToMove(struct SOLDIERTYPE *pSoldier) {
  // look around all 8 directions and return lowest terrain cost
  int32_t cnt;
  int16_t sLowest = 127;
  int16_t sGridno, sCost;

  if (TANK(pSoldier)) {
    return ((int8_t)sLowest);
  }

  for (cnt = 0; cnt <= 7; cnt++) {
    sGridno = NewGridNo(pSoldier->sGridNo, DirectionInc((int16_t)cnt));
    if (sGridno != pSoldier->sGridNo) {
      if ((sCost = ActionPointCost(pSoldier, sGridno, (uint8_t)cnt, pSoldier->usUIMovementMode)) <
          sLowest) {
        sLowest = sCost;
      }
    }
  }
  return ((int8_t)sLowest);
}

int8_t PtsToMoveDirection(struct SOLDIERTYPE *pSoldier, int8_t bDirection) {
  int16_t sGridno, sCost;
  int8_t bOverTerrainType;
  uint16_t usMoveModeToUse;

  sGridno = NewGridNo(pSoldier->sGridNo, DirectionInc((int16_t)bDirection));

  usMoveModeToUse = pSoldier->usUIMovementMode;

  // ATE: Check if the new place is watter and we were tying to run....
  bOverTerrainType = GetTerrainType(sGridno);

  if (bOverTerrainType == MED_WATER || bOverTerrainType == DEEP_WATER ||
      bOverTerrainType == LOW_WATER) {
    usMoveModeToUse = WALKING;
  }

  sCost = ActionPointCost(pSoldier, sGridno, bDirection, usMoveModeToUse);

  if (gubWorldMovementCosts[sGridno][bDirection][pSoldier->bLevel] != TRAVELCOST_FENCE) {
    if (usMoveModeToUse == RUNNING && pSoldier->usAnimState != RUNNING) {
      sCost += AP_START_RUN_COST;
    }
  }

  return ((int8_t)sCost);
}

int8_t MinAPsToStartMovement(struct SOLDIERTYPE *pSoldier, uint16_t usMovementMode) {
  int8_t bAPs = 0;

  switch (usMovementMode) {
    case RUNNING:
    case WALKING:
      if (gAnimControl[pSoldier->usAnimState].ubEndHeight == ANIM_PRONE) {
        bAPs += AP_CROUCH + AP_PRONE;
      } else if (gAnimControl[pSoldier->usAnimState].ubEndHeight == ANIM_CROUCH) {
        bAPs += AP_CROUCH;
      }
      break;
    case SWATTING:
      if (gAnimControl[pSoldier->usAnimState].ubEndHeight == ANIM_PRONE) {
        bAPs += AP_PRONE;
      } else if (gAnimControl[pSoldier->usAnimState].ubEndHeight == ANIM_STAND) {
        bAPs += AP_CROUCH;
      }
      break;
    case CRAWLING:
      if (gAnimControl[pSoldier->usAnimState].ubEndHeight == ANIM_STAND) {
        bAPs += AP_CROUCH + AP_PRONE;
      } else if (gAnimControl[pSoldier->usAnimState].ubEndHeight == ANIM_CROUCH) {
        bAPs += AP_CROUCH;
      }
      break;
    default:
      break;
  }

  if (usMovementMode == RUNNING && pSoldier->usAnimState != RUNNING) {
    bAPs += AP_START_RUN_COST;
  }
  return (bAPs);
}

BOOLEAN EnoughAmmo(struct SOLDIERTYPE *pSoldier, BOOLEAN fDisplay, int8_t bInvPos) {
  if (pSoldier->inv[bInvPos].usItem != NOTHING) {
    if (pSoldier->bWeaponMode == WM_ATTACHED) {
      return (TRUE);
    } else {
      if (pSoldier->inv[bInvPos].usItem == ROCKET_LAUNCHER) {
        // hack... they turn empty afterwards anyways
        return (TRUE);
      }

      if (Item[pSoldier->inv[bInvPos].usItem].usItemClass == IC_LAUNCHER ||
          pSoldier->inv[bInvPos].usItem == TANK_CANNON) {
        if (FindAttachmentByClass(&(pSoldier->inv[bInvPos]), IC_GRENADE) != ITEM_NOT_FOUND) {
          return (TRUE);
        }

        // ATE: Did an else if here...
        if (FindAttachmentByClass(&(pSoldier->inv[bInvPos]), IC_BOMB) != ITEM_NOT_FOUND) {
          return (TRUE);
        }

        if (fDisplay) {
          TacticalCharacterDialogue(pSoldier, QUOTE_OUT_OF_AMMO);
        }

        return (FALSE);
      } else if (Item[pSoldier->inv[bInvPos].usItem].usItemClass == IC_GUN) {
        if (pSoldier->inv[bInvPos].ubGunShotsLeft == 0) {
          if (fDisplay) {
            TacticalCharacterDialogue(pSoldier, QUOTE_OUT_OF_AMMO);
          }
          return (FALSE);
        }
      }
    }

    return (TRUE);
  }

  return (FALSE);
}

void DeductAmmo(struct SOLDIERTYPE *pSoldier, int8_t bInvPos) {
  struct OBJECTTYPE *pObj;

  // tanks never run out of MG ammo!
  // unlimited cannon ammo is handled in AI
  if (TANK(pSoldier) && pSoldier->inv[bInvPos].usItem != TANK_CANNON) {
    return;
  }

  pObj = &(pSoldier->inv[bInvPos]);
  if (pObj->usItem != NOTHING) {
    if (pObj->usItem == TANK_CANNON) {
    } else if (Item[pObj->usItem].usItemClass == IC_GUN && pObj->usItem != TANK_CANNON) {
      if (pSoldier->usAttackingWeapon == pObj->usItem) {
        // OK, let's see, don't overrun...
        if (pObj->ubGunShotsLeft != 0) {
          pObj->ubGunShotsLeft--;
        }
      } else {
        // firing an attachment?
      }
    } else if (Item[pObj->usItem].usItemClass == IC_LAUNCHER || pObj->usItem == TANK_CANNON) {
      int8_t bAttachPos;

      bAttachPos = FindAttachmentByClass(pObj, IC_GRENADE);
      if (bAttachPos == ITEM_NOT_FOUND) {
        bAttachPos = FindAttachmentByClass(pObj, IC_BOMB);
      }

      if (bAttachPos != ITEM_NOT_FOUND) {
        RemoveAttachment(pObj, bAttachPos, NULL);
      }
    }

    // Dirty Bars
    DirtyMercPanelInterface(pSoldier, DIRTYLEVEL1);
  }
}

uint16_t GetAPsToPickupItem(struct SOLDIERTYPE *pSoldier, uint16_t usMapPos) {
  struct ITEM_POOL *pItemPool;
  uint16_t sAPCost = 0;
  int16_t sActionGridNo;

  // Check if we are over an item pool
  if (GetItemPool(usMapPos, &pItemPool, pSoldier->bLevel)) {
    // If we are in the same tile, just return pickup cost
    sActionGridNo = AdjustGridNoForItemPlacement(pSoldier, usMapPos);

    if (pSoldier->sGridNo != sActionGridNo) {
      sAPCost = PlotPath(pSoldier, sActionGridNo, NO_COPYROUTE, NO_PLOT, TEMPORARY,
                         (uint16_t)pSoldier->usUIMovementMode, NOT_STEALTH, FORWARD,
                         pSoldier->bActionPoints);

      // If point cost is zero, return 0
      if (sAPCost != 0) {
        // ADD APS TO PICKUP
        sAPCost += AP_PICKUP_ITEM;
      }
    } else {
      sAPCost += AP_PICKUP_ITEM;
    }
  }

  return (sAPCost);
}

uint16_t GetAPsToGiveItem(struct SOLDIERTYPE *pSoldier, uint16_t usMapPos) {
  uint16_t sAPCost = 0;

  sAPCost =
      PlotPath(pSoldier, usMapPos, NO_COPYROUTE, NO_PLOT, TEMPORARY,
               (uint16_t)pSoldier->usUIMovementMode, NOT_STEALTH, FORWARD, pSoldier->bActionPoints);

  // If point cost is zero, return 0
  if (sAPCost != 0 || pSoldier->sGridNo == usMapPos) {
    // ADD APS TO PICKUP
    sAPCost += AP_GIVE_ITEM;
  }

  return (sAPCost);
}

int8_t GetAPsToReloadGunWithAmmo(struct OBJECTTYPE *pGun, struct OBJECTTYPE *pAmmo) {
  if (Item[pGun->usItem].usItemClass == IC_LAUNCHER) {
    // always standard AP cost
    return (AP_RELOAD_GUN);
  }
  if (Weapon[pGun->usItem].ubMagSize == Magazine[Item[pAmmo->usItem].ubClassIndex].ubMagSize) {
    // normal situation
    return (AP_RELOAD_GUN);
  } else {
    // trying to reload with wrong size of magazine
    return (AP_RELOAD_GUN + AP_RELOAD_GUN);
  }
}

int8_t GetAPsToAutoReload(struct SOLDIERTYPE *pSoldier) {
  struct OBJECTTYPE *pObj;
  int8_t bSlot, bSlot2, bExcludeSlot;
  int8_t bAPCost = 0, bAPCost2 = 0;
  ;

  CHECKF(pSoldier);
  pObj = &(pSoldier->inv[HANDPOS]);

  if (Item[pObj->usItem].usItemClass == IC_GUN || Item[pObj->usItem].usItemClass == IC_LAUNCHER) {
    bSlot = FindAmmoToReload(pSoldier, HANDPOS, NO_SLOT);
    if (bSlot != NO_SLOT) {
      // we would reload using this ammo!
      bAPCost += GetAPsToReloadGunWithAmmo(pObj, &(pSoldier->inv[bSlot]));
    }

    if (IsValidSecondHandShotForReloadingPurposes(pSoldier)) {
      pObj = &(pSoldier->inv[SECONDHANDPOS]);
      bExcludeSlot = NO_SLOT;
      bSlot2 = NO_SLOT;

      // if the ammo for the first gun is the same we have to do special checks
      if (ValidAmmoType(pObj->usItem, pSoldier->inv[bSlot].usItem)) {
        if (pSoldier->inv[bSlot].ubNumberOfObjects == 1) {
          // we must not consider this slot for reloading!
          bExcludeSlot = bSlot;
        } else {
          // we can reload the 2nd gun from the same pocket!
          bSlot2 = bSlot;
        }
      }

      if (bSlot2 == NO_SLOT) {
        bSlot2 = FindAmmoToReload(pSoldier, SECONDHANDPOS, bExcludeSlot);
      }

      if (bSlot2 != NO_SLOT) {
        // we would reload using this ammo!
        bAPCost2 = GetAPsToReloadGunWithAmmo(pObj, &(pSoldier->inv[bSlot2]));
        if (EnoughPoints(pSoldier, (int16_t)(bAPCost + bAPCost2), 0, FALSE)) {
          // we can afford to reload both guns; otherwise display just for 1 gun
          bAPCost += bAPCost2;
        }
      }
    }
  }

  return (bAPCost);
}

uint16_t GetAPsToReloadRobot(struct SOLDIERTYPE *pSoldier, struct SOLDIERTYPE *pRobot) {
  uint16_t sAPCost = 0;
  int16_t sActionGridNo;
  uint8_t ubDirection;
  int16_t sAdjustedGridNo;

  sActionGridNo =
      FindAdjacentGridEx(pSoldier, pRobot->sGridNo, &ubDirection, &sAdjustedGridNo, TRUE, FALSE);

  sAPCost =
      PlotPath(pSoldier, sActionGridNo, NO_COPYROUTE, NO_PLOT, TEMPORARY,
               (uint16_t)pSoldier->usUIMovementMode, NOT_STEALTH, FORWARD, pSoldier->bActionPoints);

  // If point cost is zero, return 0
  if (sAPCost != 0 || sActionGridNo == pSoldier->sGridNo) {
    // ADD APS TO RELOAD
    sAPCost += 4;
  }

  return (sAPCost);
}

uint16_t GetAPsToChangeStance(struct SOLDIERTYPE *pSoldier, int8_t bDesiredHeight) {
  uint16_t sAPCost = 0;
  int8_t bCurrentHeight;

  bCurrentHeight = gAnimControl[pSoldier->usAnimState].ubEndHeight;

  if (bCurrentHeight == bDesiredHeight) {
    sAPCost = 0;
  }

  if (bCurrentHeight == ANIM_STAND && bDesiredHeight == ANIM_PRONE) {
    sAPCost = AP_CROUCH + AP_PRONE;
  }
  if (bCurrentHeight == ANIM_STAND && bDesiredHeight == ANIM_CROUCH) {
    sAPCost = AP_CROUCH;
  }
  if (bCurrentHeight == ANIM_CROUCH && bDesiredHeight == ANIM_PRONE) {
    sAPCost = AP_PRONE;
  }
  if (bCurrentHeight == ANIM_CROUCH && bDesiredHeight == ANIM_STAND) {
    sAPCost = AP_CROUCH;
  }
  if (bCurrentHeight == ANIM_PRONE && bDesiredHeight == ANIM_STAND) {
    sAPCost = AP_PRONE + AP_CROUCH;
  }
  if (bCurrentHeight == ANIM_PRONE && bDesiredHeight == ANIM_CROUCH) {
    sAPCost = AP_PRONE;
  }

  return (sAPCost);
}

uint16_t GetBPsToChangeStance(struct SOLDIERTYPE *pSoldier, int8_t bDesiredHeight) {
  uint16_t sBPCost = 0;
  int8_t bCurrentHeight;

  bCurrentHeight = gAnimControl[pSoldier->usAnimState].ubEndHeight;

  if (bCurrentHeight == bDesiredHeight) {
    sBPCost = 0;
  }

  if (bCurrentHeight == ANIM_STAND && bDesiredHeight == ANIM_PRONE) {
    sBPCost = BP_CROUCH + BP_PRONE;
  }
  if (bCurrentHeight == ANIM_STAND && bDesiredHeight == ANIM_CROUCH) {
    sBPCost = BP_CROUCH;
  }
  if (bCurrentHeight == ANIM_CROUCH && bDesiredHeight == ANIM_PRONE) {
    sBPCost = BP_PRONE;
  }
  if (bCurrentHeight == ANIM_CROUCH && bDesiredHeight == ANIM_STAND) {
    sBPCost = BP_CROUCH;
  }
  if (bCurrentHeight == ANIM_PRONE && bDesiredHeight == ANIM_STAND) {
    sBPCost = BP_PRONE + BP_CROUCH;
  }
  if (bCurrentHeight == ANIM_PRONE && bDesiredHeight == ANIM_CROUCH) {
    sBPCost = BP_PRONE;
  }

  return (sBPCost);
}

uint16_t GetAPsToLook(struct SOLDIERTYPE *pSoldier) {
  // Set # of APs
  switch (gAnimControl[pSoldier->usAnimState].ubEndHeight) {
    // Now change to appropriate animation
    case ANIM_STAND:
      return (AP_LOOK_STANDING);
      break;

    case ANIM_CROUCH:
      return (AP_LOOK_CROUCHED);
      break;

    case ANIM_PRONE:
      // AP_PRONE is the AP cost to go to or from the prone stance.  To turn while prone, your merc
      // has to get up to crouched, turn, and then go back down.  Hence you go up (AP_PRONE), turn
      // (AP_LOOK_PRONE) and down (AP_PRONE).
      return (AP_LOOK_PRONE + AP_PRONE + AP_PRONE);
      break;

    // no other values should be possible
    default:
      Assert(FALSE);
      return (0);
      break;
  }
}

BOOLEAN CheckForMercContMove(struct SOLDIERTYPE *pSoldier) {
  int16_t sAPCost;
  int16_t sGridNo;

  if (!(gTacticalStatus.uiFlags & INCOMBAT)) {
    return (FALSE);
  }

  if (gpItemPointer != NULL) {
    return (FALSE);
  }

  if (pSoldier->bLife >= OKLIFE) {
    if (pSoldier->sGridNo != pSoldier->sFinalDestination || pSoldier->bGoodContPath) {
      // OK< check if we are the selected guy!
      if (pSoldier->ubID == gusSelectedSoldier) {
        if (SoldierOnScreen(pSoldier->ubID)) {
          sGridNo = pSoldier->sFinalDestination;

          if (pSoldier->bGoodContPath) {
            sGridNo = pSoldier->sContPathLocation;
          }

          // Do a check if we can afford move here!

          // get a path to dest...
          if (FindBestPath(pSoldier, sGridNo, pSoldier->bLevel, pSoldier->usUIMovementMode,
                           NO_COPYROUTE, 0)) {
            sAPCost = PtsToMoveDirection(pSoldier, (uint8_t)guiPathingData[0]);

            if (EnoughPoints(pSoldier, sAPCost, 0, FALSE)) {
              return (TRUE);
            }
          } else {
            return (FALSE);
          }
        }
      }
    }
  }
  return (FALSE);
}

int16_t GetAPsToReadyWeapon(struct SOLDIERTYPE *pSoldier, uint16_t usAnimState) {
  uint16_t usItem;

  // If this is a dwel pistol anim
  // ATE: What was I thinking, hooking into animations like this....
  // if ( usAnimState == READY_DUAL_STAND || usAnimState == READY_DUAL_CROUCH )
  //{
  // return( AP_READY_DUAL );
  //}
  if (IsValidSecondHandShot(pSoldier)) {
    return (AP_READY_DUAL);
  }

  // OK, now check type of weapon
  usItem = pSoldier->inv[HANDPOS].usItem;

  if (usItem == NOTHING) {
    return (0);
  } else {
    // CHECK FOR RIFLE
    if (Item[usItem].usItemClass == IC_GUN) {
      return (Weapon[usItem].ubReadyTime);
    }
  }

  return (0);
}

int8_t GetAPsToClimbRoof(struct SOLDIERTYPE *pSoldier, BOOLEAN fClimbDown) {
  if (!fClimbDown) {
    // OK, add aps to goto stand stance...
    return ((int8_t)(AP_CLIMBROOF + GetAPsToChangeStance(pSoldier, ANIM_STAND)));
  } else {
    // Add aps to goto crouch
    return ((int8_t)(AP_CLIMBOFFROOF + GetAPsToChangeStance(pSoldier, ANIM_CROUCH)));
  }
}

int16_t GetBPsToClimbRoof(struct SOLDIERTYPE *pSoldier, BOOLEAN fClimbDown) {
  if (!fClimbDown) {
    return (BP_CLIMBROOF);
  } else {
    return (BP_CLIMBOFFROOF);
  }
}

int8_t GetAPsToCutFence(struct SOLDIERTYPE *pSoldier) {
  // OK, it's normally just cost, but add some if different stance...
  return (GetAPsToChangeStance(pSoldier, ANIM_CROUCH) + AP_USEWIRECUTTERS);
}

int8_t GetAPsToBeginFirstAid(struct SOLDIERTYPE *pSoldier) {
  // OK, it's normally just cost, but add some if different stance...
  return (GetAPsToChangeStance(pSoldier, ANIM_CROUCH) + AP_START_FIRST_AID);
}

int8_t GetAPsToBeginRepair(struct SOLDIERTYPE *pSoldier) {
  // OK, it's normally just cost, but add some if different stance...
  return (GetAPsToChangeStance(pSoldier, ANIM_CROUCH) + AP_START_REPAIR);
}

int8_t GetAPsToRefuelVehicle(struct SOLDIERTYPE *pSoldier) {
  // OK, it's normally just cost, but add some if different stance...
  return (GetAPsToChangeStance(pSoldier, ANIM_CROUCH) + AP_REFUEL_VEHICLE);
}

#define TOSSES_PER_10TURNS 18  // max # of grenades tossable in 10 turns
#define AP_MIN_AIM_ATTACK 0    // minimum permitted extra aiming
#define AP_MAX_AIM_ATTACK 4    // maximum permitted extra aiming

int16_t MinAPsToThrow(struct SOLDIERTYPE *pSoldier, int16_t sGridNo, uint8_t ubAddTurningCost) {
  int32_t iTop, iBottom;
  int32_t iFullAPs;
  int32_t iAPCost = AP_MIN_AIM_ATTACK;
  uint16_t usInHand;
  uint16_t usTargID;
  uint32_t uiMercFlags;
  uint8_t ubDirection;

  // make sure the guy's actually got a throwable item in his hand!
  usInHand = pSoldier->inv[HANDPOS].usItem;

  if (!(Item[usInHand].usItemClass & IC_GRENADE)) {
#ifdef JA2TESTVERSION
    ScreenMsg(MSG_FONT_YELLOW, MSG_DEBUG, L"MinAPsToThrow - Called when in-hand item is %s",
              usInHand);
#endif
    return (0);
  }

  if (sGridNo != NOWHERE) {
    // Given a gridno here, check if we are on a guy - if so - get his gridno
    if (FindSoldier(sGridNo, &usTargID, &uiMercFlags, FIND_SOLDIER_GRIDNO)) {
      sGridNo = MercPtrs[usTargID]->sGridNo;
    }

    // OK, get a direction and see if we need to turn...
    if (ubAddTurningCost) {
      ubDirection = (uint8_t)GetDirectionFromGridNo(sGridNo, pSoldier);

      // Is it the same as he's facing?
      if (ubDirection != pSoldier->bDirection) {
        // iAPCost += GetAPsToLook( pSoldier );
      }
    }
  } else {
    // Assume we need to add cost!
    // iAPCost += GetAPsToLook( pSoldier );
  }

  // if attacking a new target (or if the specific target is uncertain)
  if ((sGridNo != pSoldier->sLastTarget)) {
    iAPCost += AP_CHANGE_TARGET;
  }

  iAPCost += GetAPsToChangeStance(pSoldier, ANIM_STAND);

  // Calculate default top & bottom of the magic "aiming" formula)

  // get this man's maximum possible action points (ignoring carryovers)
  iFullAPs = CalcActionPoints(pSoldier);

  // the 2 times is here only to around rounding off using integer math later
  iTop = 2 * iFullAPs;

  // if it's anything but a mortar
  //	if ( usInHand != MORTAR)
  // tosses per turn is for max dexterity, drops down to 1/2 at dexterity = 0
  // bottom = (TOSSES_PER_10TURNS * (50 + (ptr->dexterity / 2)) / 10);
  // else
  iBottom = (TOSSES_PER_10TURNS * (50 + (pSoldier->bDexterity / 2)) / 10);

  // add minimum aiming time to the overall minimum AP_cost
  //     This here ROUNDS UP fractions of 0.5 or higher using integer math
  //     This works because 'top' is 2x what it really should be throughout
  iAPCost += (((100 * iTop) / iBottom) + 1) / 2;

  // the minimum AP cost of ANY throw can NEVER be more than merc has APs!
  if (iAPCost > iFullAPs) iAPCost = iFullAPs;

  // this SHOULD be impossible, but nevertheless...
  if (iAPCost < 1) iAPCost = 1;

  return ((int16_t)iAPCost);
}

uint16_t GetAPsToDropBomb(struct SOLDIERTYPE *pSoldier) { return (AP_DROP_BOMB); }

uint16_t GetTotalAPsToDropBomb(struct SOLDIERTYPE *pSoldier, int16_t sGridNo) {
  int16_t sAPs = 0;

  sAPs =
      PlotPath(pSoldier, sGridNo, NO_COPYROUTE, NO_PLOT, TEMPORARY,
               (uint16_t)pSoldier->usUIMovementMode, NOT_STEALTH, FORWARD, pSoldier->bActionPoints);

  if (sAPs > 0) {
    sAPs += AP_DROP_BOMB;
  }

  return (sAPs);
}

uint16_t GetAPsToUseRemote(struct SOLDIERTYPE *pSoldier) { return (AP_USE_REMOTE); }

int8_t GetAPsToStealItem(struct SOLDIERTYPE *pSoldier, int16_t usMapPos) {
  uint16_t sAPCost = 0;

  sAPCost =
      PlotPath(pSoldier, usMapPos, NO_COPYROUTE, NO_PLOT, TEMPORARY,
               (uint16_t)pSoldier->usUIMovementMode, NOT_STEALTH, FORWARD, pSoldier->bActionPoints);

  // ADD APS TO PICKUP
  sAPCost += AP_STEAL_ITEM;

  // CJC August 13 2002: added cost to stand into equation
  if (!(PTR_STANDING)) {
    sAPCost += GetAPsToChangeStance(pSoldier, ANIM_STAND);
  }

  return ((int8_t)sAPCost);
}

int8_t GetBPsToStealItem(struct SOLDIERTYPE *pSoldier) { return (BP_STEAL_ITEM); }

int8_t GetAPsToUseJar(struct SOLDIERTYPE *pSoldier, int16_t usMapPos) {
  uint16_t sAPCost = 0;

  sAPCost =
      PlotPath(pSoldier, usMapPos, NO_COPYROUTE, NO_PLOT, TEMPORARY,
               (uint16_t)pSoldier->usUIMovementMode, NOT_STEALTH, FORWARD, pSoldier->bActionPoints);

  // If point cost is zero, return 0
  if (sAPCost != 0) {
    // ADD APS TO PICKUP
    sAPCost += AP_TAKE_BLOOD;
  }

  return ((int8_t)sAPCost);
}

int8_t GetAPsToUseCan(struct SOLDIERTYPE *pSoldier, int16_t usMapPos) {
  uint16_t sAPCost = 0;

  sAPCost =
      PlotPath(pSoldier, usMapPos, NO_COPYROUTE, NO_PLOT, TEMPORARY,
               (uint16_t)pSoldier->usUIMovementMode, NOT_STEALTH, FORWARD, pSoldier->bActionPoints);

  // If point cost is zero, return 0
  if (sAPCost != 0) {
    // ADD APS TO PICKUP
    sAPCost += AP_ATTACH_CAN;
  }

  return ((int8_t)sAPCost);
}

int8_t GetAPsToJumpOver(struct SOLDIERTYPE *pSoldier) {
  return (GetAPsToChangeStance(pSoldier, ANIM_STAND) + AP_JUMP_OVER);
}
