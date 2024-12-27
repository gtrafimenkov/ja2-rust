#include "Tactical/SkillCheck.h"

#include "SGP/Random.h"
#include "Soldier.h"
#include "Strategic/StrategicMap.h"
#include "Tactical/DialogueControl.h"
#include "Tactical/DrugsAndAlcohol.h"
#include "Tactical/Items.h"
#include "Tactical/Morale.h"
#include "Tactical/Overhead.h"
#include "Tactical/SoldierControl.h"
#include "Tactical/SoldierMacros.h"
#include "Tactical/SoldierProfile.h"
#include "TileEngine/IsometricUtils.h"

int8_t EffectiveStrength(struct SOLDIERTYPE *pSoldier) {
  int8_t bBandaged;
  int32_t iEffStrength;

  // Effective strength is:
  // 1/2 full strength
  // plus 1/2 strength scaled according to how hurt we are
  bBandaged = pSoldier->bLifeMax - pSoldier->bLife - pSoldier->bBleeding;
  iEffStrength = pSoldier->bStrength / 2;
  iEffStrength +=
      (pSoldier->bStrength / 2) * (pSoldier->bLife + bBandaged / 2) / (pSoldier->bLifeMax);

  // ATE: Make sure at least 2...
  iEffStrength = max(iEffStrength, 2);

  return ((int8_t)iEffStrength);
}

int8_t EffectiveWisdom(struct SOLDIERTYPE *pSoldier) {
  int32_t iEffWisdom;

  iEffWisdom = pSoldier->bWisdom;

  iEffWisdom = EffectStatForBeingDrunk(pSoldier, iEffWisdom);

  return ((int8_t)iEffWisdom);
}

int8_t EffectiveAgility(struct SOLDIERTYPE *pSoldier) {
  int32_t iEffAgility;

  iEffAgility = pSoldier->bAgility;

  iEffAgility = EffectStatForBeingDrunk(pSoldier, iEffAgility);

  if (pSoldier->sWeightCarriedAtTurnStart > 100) {
    iEffAgility = (iEffAgility * 100) / pSoldier->sWeightCarriedAtTurnStart;
  }

  return ((int8_t)iEffAgility);
}

int8_t EffectiveMechanical(struct SOLDIERTYPE *pSoldier) {
  int32_t iEffMechanical;

  iEffMechanical = pSoldier->bMechanical;

  iEffMechanical = EffectStatForBeingDrunk(pSoldier, iEffMechanical);

  return ((int8_t)iEffMechanical);
}

int8_t EffectiveExplosive(struct SOLDIERTYPE *pSoldier) {
  int32_t iEffExplosive;

  iEffExplosive = pSoldier->bExplosive;

  iEffExplosive = EffectStatForBeingDrunk(pSoldier, iEffExplosive);

  return ((int8_t)iEffExplosive);
}

int8_t EffectiveMedical(struct SOLDIERTYPE *pSoldier) {
  int32_t iEffMedical;

  iEffMedical = pSoldier->bMedical;

  iEffMedical = EffectStatForBeingDrunk(pSoldier, iEffMedical);

  return ((int8_t)iEffMedical);
}

int8_t EffectiveLeadership(struct SOLDIERTYPE *pSoldier) {
  int32_t iEffLeadership;
  int8_t bDrunkLevel;

  iEffLeadership = pSoldier->bLeadership;

  // if we are drunk, effect leader ship in a +ve way...
  bDrunkLevel = GetDrunkLevel(pSoldier);

  if (bDrunkLevel == FEELING_GOOD) {
    iEffLeadership = (iEffLeadership * 120 / 100);
  }

  return ((int8_t)iEffLeadership);
}

int8_t EffectiveExpLevel(struct SOLDIERTYPE *pSoldier) {
  int32_t iEffExpLevel;
  int8_t bDrunkLevel;
  int32_t iExpModifier[] = {
      0,   // SOBER
      0,   // Feeling good
      -1,  // Borderline
      -2,  // Drunk
      0,   // Hung
  };

  iEffExpLevel = pSoldier->bExpLevel;

  bDrunkLevel = GetDrunkLevel(pSoldier);

  iEffExpLevel = iEffExpLevel + iExpModifier[bDrunkLevel];

  if (GetSolProfile(pSoldier) != NO_PROFILE) {
    if ((gMercProfiles[GetSolProfile(pSoldier)].bPersonalityTrait == CLAUSTROPHOBIC) &&
        IsSolActive(pSoldier) && pSoldier->bInSector && gbWorldSectorZ > 0) {
      // claustrophobic!
      iEffExpLevel--;
    }
  }

  if (iEffExpLevel < 1) {
    // can't go below 1
    return (1);
  } else {
    return ((int8_t)iEffExpLevel);
  }
}

int8_t EffectiveMarksmanship(struct SOLDIERTYPE *pSoldier) {
  int32_t iEffMarksmanship;

  iEffMarksmanship = pSoldier->bMarksmanship;

  iEffMarksmanship = EffectStatForBeingDrunk(pSoldier, iEffMarksmanship);

  return ((int8_t)iEffMarksmanship);
}

int8_t EffectiveDexterity(struct SOLDIERTYPE *pSoldier) {
  int32_t iEffDexterity;

  iEffDexterity = pSoldier->bDexterity;

  iEffDexterity = EffectStatForBeingDrunk(pSoldier, iEffDexterity);

  return ((int8_t)iEffDexterity);
}

uint8_t GetPenaltyForFatigue(struct SOLDIERTYPE *pSoldier) {
  uint8_t ubPercentPenalty;

  if (pSoldier->bBreathMax >= 85)
    ubPercentPenalty = 0;
  else if (pSoldier->bBreathMax >= 70)
    ubPercentPenalty = 10;
  else if (pSoldier->bBreathMax >= 50)
    ubPercentPenalty = 25;
  else if (pSoldier->bBreathMax >= 30)
    ubPercentPenalty = 50;
  else if (pSoldier->bBreathMax >= 15)
    ubPercentPenalty = 75;
  else if (pSoldier->bBreathMax > 0)
    ubPercentPenalty = 90;
  else
    ubPercentPenalty = 100;

  return (ubPercentPenalty);
}

void ReducePointsForFatigue(struct SOLDIERTYPE *pSoldier, uint16_t *pusPoints) {
  *pusPoints -= (*pusPoints * GetPenaltyForFatigue(pSoldier)) / 100;
}

int32_t GetSkillCheckPenaltyForFatigue(struct SOLDIERTYPE *pSoldier, int32_t iSkill) {
  // use only half the full effect of fatigue for skill checks
  return (((iSkill * GetPenaltyForFatigue(pSoldier)) / 100) / 2);
}

int32_t SkillCheck(struct SOLDIERTYPE *pSoldier, int8_t bReason, int8_t bChanceMod) {
  int32_t iSkill;
  int32_t iChance;
  int32_t iRoll, iMadeItBy;
  int8_t bSlot;
  int32_t iLoop;
  struct SOLDIERTYPE *pTeamSoldier;
  int8_t bBuddyIndex;
  BOOLEAN fForceDamnSound = FALSE;

  switch (bReason) {
    case LOCKPICKING_CHECK:
    case ELECTRONIC_LOCKPICKING_CHECK:

      fForceDamnSound = TRUE;

      iSkill = EffectiveMechanical(pSoldier);
      if (iSkill == 0) {
        break;
      }
      // adjust skill based on wisdom (knowledge)
      iSkill = iSkill * (EffectiveWisdom(pSoldier) + 100) / 200;
      // and dexterity (clumsy?)
      iSkill = iSkill * (EffectiveDexterity(pSoldier) + 100) / 200;
      // factor in experience
      iSkill = iSkill + EffectiveExpLevel(pSoldier) * 3;
      if (HAS_SKILL_TRAIT(pSoldier, LOCKPICKING)) {
        // if we specialize in picking locks...
        iSkill += gbSkillTraitBonus[LOCKPICKING] * NUM_SKILL_TRAITS(pSoldier, LOCKPICKING);
      }
      if (bReason == ELECTRONIC_LOCKPICKING_CHECK && !(HAS_SKILL_TRAIT(pSoldier, ELECTRONICS))) {
        // if we are unfamiliar with electronics...
        iSkill /= 2;
      }
      // adjust chance based on status of kit
      bSlot = FindObj(pSoldier, LOCKSMITHKIT);
      if (bSlot == NO_SLOT) {
        // this should never happen, but might as well check...
        iSkill = 0;
      }
      iSkill = iSkill * pSoldier->inv[bSlot].bStatus[0] / 100;
      break;
    case ATTACHING_DETONATOR_CHECK:
    case ATTACHING_REMOTE_DETONATOR_CHECK:
      iSkill = EffectiveExplosive(pSoldier);
      if (iSkill == 0) {
        break;
      }
      iSkill = (iSkill * 3 + EffectiveDexterity(pSoldier)) / 4;
      if (bReason == ATTACHING_REMOTE_DETONATOR_CHECK &&
          !(HAS_SKILL_TRAIT(pSoldier, ELECTRONICS))) {
        iSkill /= 2;
      }
      break;
    case PLANTING_BOMB_CHECK:
    case PLANTING_REMOTE_BOMB_CHECK:
      iSkill = EffectiveExplosive(pSoldier) * 7;
      iSkill += EffectiveWisdom(pSoldier) * 2;
      iSkill += EffectiveExpLevel(pSoldier) * 10;
      iSkill = iSkill / 10;  // bring the value down to a percentage

      if (bReason == PLANTING_REMOTE_BOMB_CHECK && !(HAS_SKILL_TRAIT(pSoldier, ELECTRONICS))) {
        // deduct only a bit...
        iSkill = (iSkill * 3) / 4;
      }

      // Ok, this is really damn easy, so skew the values...
      // e.g. if calculated skill is 84, skewed up to 96
      // 51 to 84
      // 22 stays as is
      iSkill = (iSkill + 100 * (iSkill / 25)) / (iSkill / 25 + 1);
      break;

    case DISARM_TRAP_CHECK:

      fForceDamnSound = TRUE;

      iSkill = EffectiveExplosive(pSoldier) * 7;
      if (iSkill == 0) {
        break;
      }
      iSkill += EffectiveDexterity(pSoldier) * 2;
      iSkill += EffectiveExpLevel(pSoldier) * 10;
      iSkill = iSkill / 10;  // bring the value down to a percentage
      // penalty based on poor wisdom
      iSkill -= (100 - EffectiveWisdom(pSoldier)) / 5;
      break;

    case DISARM_ELECTRONIC_TRAP_CHECK:

      fForceDamnSound = TRUE;

      iSkill = max(EffectiveMechanical(pSoldier), EffectiveExplosive(pSoldier)) * 7;
      if (iSkill == 0) {
        break;
      }
      iSkill += EffectiveDexterity(pSoldier) * 2;
      iSkill += EffectiveExpLevel(pSoldier) * 10;
      iSkill = iSkill / 10;  // bring the value down to a percentage
      // penalty based on poor wisdom
      iSkill -= (100 - EffectiveWisdom(pSoldier)) / 5;

      if (!(HAS_SKILL_TRAIT(pSoldier, ELECTRONICS))) {
        iSkill = (iSkill * 3) / 4;
      }
      break;

    case OPEN_WITH_CROWBAR:
      // Add for crowbar...
      iSkill = EffectiveStrength(pSoldier) + 20;
      fForceDamnSound = TRUE;
      break;

    case SMASH_DOOR_CHECK:
      iSkill = EffectiveStrength(pSoldier);
      break;
    case UNJAM_GUN_CHECK:
      iSkill = 30 + EffectiveMechanical(pSoldier) / 2;
      break;
    case NOTICE_DART_CHECK:
      // only a max of ~20% chance
      iSkill = EffectiveWisdom(pSoldier) / 10 + EffectiveExpLevel(pSoldier);
      break;
    case LIE_TO_QUEEN_CHECK:
      // competitive check vs the queen's wisdom and leadership... poor guy!
      iSkill = 50 * (EffectiveWisdom(pSoldier) + EffectiveLeadership(pSoldier)) /
               (gMercProfiles[QUEEN].bWisdom + gMercProfiles[QUEEN].bLeadership);
      break;
    case ATTACHING_SPECIAL_ITEM_CHECK:
    case ATTACHING_SPECIAL_ELECTRONIC_ITEM_CHECK:
      iSkill = EffectiveMechanical(pSoldier);
      if (iSkill == 0) {
        break;
      }
      // adjust skill based on wisdom (knowledge)
      iSkill = iSkill * (EffectiveWisdom(pSoldier) + 100) / 200;
      // and dexterity (clumsy?)
      iSkill = iSkill * (EffectiveDexterity(pSoldier) + 100) / 200;
      // factor in experience
      iSkill = iSkill + EffectiveExpLevel(pSoldier) * 3;
      if (bReason == ATTACHING_SPECIAL_ELECTRONIC_ITEM_CHECK &&
          !(HAS_SKILL_TRAIT(pSoldier, ELECTRONICS))) {
        // if we are unfamiliar with electronics...
        iSkill /= 2;
      }
      break;
    default:
      iSkill = 0;
      break;
  }

  iSkill -= GetSkillCheckPenaltyForFatigue(pSoldier, iSkill);

  iChance = iSkill + bChanceMod;

  switch (bReason) {
    case LOCKPICKING_CHECK:
    case ELECTRONIC_LOCKPICKING_CHECK:
    case DISARM_TRAP_CHECK:
    case DISARM_ELECTRONIC_TRAP_CHECK:
    case OPEN_WITH_CROWBAR:
    case SMASH_DOOR_CHECK:
    case ATTACHING_SPECIAL_ITEM_CHECK:
    case ATTACHING_SPECIAL_ELECTRONIC_ITEM_CHECK:
      // for lockpicking and smashing locks, if the chance isn't reasonable
      // we set it to 0 so they can never get through the door if they aren't
      // good enough
      if (iChance < 30) {
        iChance = 0;
        break;
      }
      // else fall through
    default:
      iChance += GetMoraleModifier(pSoldier);
      break;
  }

  if (iChance > 99) {
    iChance = 99;
  } else if (iChance < 0) {
    iChance = 0;
  }

  iRoll = PreRandom(100);
  iMadeItBy = iChance - iRoll;
  if (iMadeItBy < 0) {
    if ((pSoldier->bLastSkillCheck == bReason) &&
        (pSoldier->sGridNo == pSoldier->sSkillCheckGridNo)) {
      pSoldier->ubSkillCheckAttempts++;
      if (pSoldier->ubSkillCheckAttempts > 2) {
        if (iChance == 0) {
          // do we realize that we just can't do this?
          if ((100 - (pSoldier->ubSkillCheckAttempts - 2) * 20) < EffectiveWisdom(pSoldier)) {
            // say "I can't do this" quote
            TacticalCharacterDialogue(pSoldier, QUOTE_DEFINITE_CANT_DO);
            return (iMadeItBy);
          }
        }
      }
    } else {
      pSoldier->bLastSkillCheck = bReason;
      pSoldier->ubSkillCheckAttempts = 1;
      pSoldier->sSkillCheckGridNo = pSoldier->sGridNo;
    }

    if (fForceDamnSound || Random(100) < 40) {
      switch (bReason) {
        case UNJAM_GUN_CHECK:
        case NOTICE_DART_CHECK:
        case LIE_TO_QUEEN_CHECK:
          // silent check
          break;
        default:
          DoMercBattleSound(pSoldier, BATTLE_SOUND_CURSE1);
          break;
      }
    }

  } else {
    // A buddy might make a positive comment based on our success;
    // Increase the chance for people with higher skill and for more difficult tasks
    iChance = 15 + iSkill / 20 + (-bChanceMod) / 20;
    if (iRoll < iChance) {
      // If a buddy of this merc is standing around nearby, they'll make a positive comment.
      iLoop = gTacticalStatus.Team[gbPlayerNum].bFirstID;
      for (pTeamSoldier = MercPtrs[iLoop]; iLoop <= gTacticalStatus.Team[gbPlayerNum].bLastID;
           iLoop++, pTeamSoldier++) {
        if (OK_INSECTOR_MERC(pTeamSoldier)) {
          bBuddyIndex = WhichBuddy(pTeamSoldier->ubProfile, GetSolProfile(pSoldier));
          if (bBuddyIndex >= 0 && SpacesAway(pSoldier->sGridNo, pTeamSoldier->sGridNo) < 15) {
            switch (bBuddyIndex) {
              case 0:
                // buddy #1 did something good!
                TacticalCharacterDialogue(pTeamSoldier, QUOTE_BUDDY_1_GOOD);
                break;
              case 1:
                // buddy #2 did something good!
                TacticalCharacterDialogue(pTeamSoldier, QUOTE_BUDDY_2_GOOD);
                break;
              case 2:
                // learn to like buddy did something good!
                TacticalCharacterDialogue(pTeamSoldier, QUOTE_LEARNED_TO_LIKE_WITNESSED);
                break;
              default:
                break;
            }
          }
        }
      }
    }
  }
  return (iMadeItBy);
}

int8_t CalcTrapDetectLevel(struct SOLDIERTYPE *pSoldier, BOOLEAN fExamining) {
  // return the level of trap which the guy is able to detect

  int8_t bDetectLevel;

  // formula: 1 pt for every exp_level
  //     plus 1 pt for every 40 explosives
  //     less 1 pt for every 20 wisdom MISSING

  bDetectLevel = EffectiveExpLevel(pSoldier);
  bDetectLevel += (EffectiveExplosive(pSoldier) / 40);
  bDetectLevel -= ((100 - EffectiveWisdom(pSoldier)) / 20);

  // if the examining flag is true, this isn't just a casual glance
  // and the merc should have a higher chance
  if (fExamining) {
    bDetectLevel += (int8_t)PreRandom(bDetectLevel / 3 + 2);
  }

  // if substantially bleeding, or still in serious shock, randomly lower value
  if ((pSoldier->bBleeding > 20) || (pSoldier->bShock > 1)) {
    bDetectLevel -= (int8_t)PreRandom(3);
  }

  if (bDetectLevel < 1) {
    bDetectLevel = 1;
  }

  return (bDetectLevel);
}
