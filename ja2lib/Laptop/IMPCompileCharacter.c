#include "Laptop/IMPCompileCharacter.h"

#include "Laptop/CharProfile.h"
#include "Laptop/IMPAboutUs.h"
#include "Laptop/IMPAttributeEntrance.h"
#include "Laptop/IMPAttributeFinish.h"
#include "Laptop/IMPAttributeSelection.h"
#include "Laptop/IMPBeginScreen.h"
#include "Laptop/IMPFinish.h"
#include "Laptop/IMPHomePage.h"
#include "Laptop/IMPMainPage.h"
#include "Laptop/IMPPersonalityEntrance.h"
#include "Laptop/IMPPersonalityFinish.h"
#include "Laptop/IMPPersonalityQuiz.h"
#include "Laptop/IMPPortraits.h"
#include "Laptop/IMPVideoObjects.h"
#include "Laptop/IMPVoices.h"
#include "Laptop/Laptop.h"
#include "Laptop/LaptopSave.h"
#include "SGP/Random.h"
#include "SGP/WCheck.h"
#include "Tactical/AnimationData.h"
#include "Tactical/SoldierProfile.h"
#include "Tactical/SoldierProfileType.h"
#include "Utils/Cursors.h"
#include "Utils/EncryptedFile.h"
#include "Utils/Utilities.h"
#include "Utils/WordWrap.h"
// how many times should a 'die' be rolled for skills of the same type?

#define HOW_MANY_ROLLS_FOR_SAME_SKILL_CHECK 20

int32_t AttitudeList[ATTITUDE_LIST_SIZE];
int32_t iLastElementInAttitudeList = 0;

int32_t SkillsList[ATTITUDE_LIST_SIZE];
int32_t BackupSkillsList[ATTITUDE_LIST_SIZE];
int32_t iLastElementInSkillsList = 0;

int32_t PersonalityList[ATTITUDE_LIST_SIZE];
int32_t iLastElementInPersonalityList = 0;

extern BOOLEAN fLoadingCharacterForPreviousImpProfile;

// positions of the face x and y for eyes and mouth for the 10 portraits
int16_t sFacePositions[NUMBER_OF_PLAYER_PORTRAITS][4] = {
    {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0},
    {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0},
    {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0},
};

char* pPlayerSelectedFaceFileNames[NUMBER_OF_PLAYER_PORTRAITS] = {
    "Faces\\200.sti", "Faces\\201.sti", "Faces\\202.sti", "Faces\\203.sti",
    "Faces\\204.sti", "Faces\\205.sti", "Faces\\206.sti", "Faces\\207.sti",
    "Faces\\208.sti", "Faces\\209.sti", "Faces\\210.sti", "Faces\\211.sti",
    "Faces\\212.sti", "Faces\\213.sti", "Faces\\214.sti", "Faces\\215.sti",
};

char* pPlayerSelectedBigFaceFileNames[NUMBER_OF_PLAYER_PORTRAITS] = {
    "Faces\\BigFaces\\200.sti", "Faces\\BigFaces\\201.sti", "Faces\\BigFaces\\202.sti",
    "Faces\\BigFaces\\203.sti", "Faces\\BigFaces\\204.sti", "Faces\\BigFaces\\205.sti",
    "Faces\\BigFaces\\206.sti", "Faces\\BigFaces\\207.sti", "Faces\\BigFaces\\208.sti",
    "Faces\\BigFaces\\209.sti", "Faces\\BigFaces\\210.sti", "Faces\\BigFaces\\211.sti",
    "Faces\\BigFaces\\212.sti", "Faces\\BigFaces\\213.sti", "Faces\\BigFaces\\214.sti",
    "Faces\\BigFaces\\215.sti",
};

// function declarations
void SelectMercFace(void);
void SetMercSkinAndHairColors(void);
BOOLEAN ShouldThisMercHaveABigBody(void);

void CreateACharacterFromPlayerEnteredStats(void) {
  // copy over full name
  wcscpy(gMercProfiles[PLAYER_GENERATED_CHARACTER_ID + LaptopSaveInfo.iVoiceId].zName, pFullName);

  // the nickname
  wcscpy(gMercProfiles[PLAYER_GENERATED_CHARACTER_ID + LaptopSaveInfo.iVoiceId].zNickname,
         pNickName);

  // gender
  if (fCharacterIsMale == TRUE) {
    // male
    gMercProfiles[PLAYER_GENERATED_CHARACTER_ID + LaptopSaveInfo.iVoiceId].bSex = MALE;
  } else {
    // female
    gMercProfiles[PLAYER_GENERATED_CHARACTER_ID + LaptopSaveInfo.iVoiceId].bSex = FEMALE;
  }

  // attributes
  gMercProfiles[PLAYER_GENERATED_CHARACTER_ID + LaptopSaveInfo.iVoiceId].bLifeMax = (int8_t)iHealth;
  gMercProfiles[PLAYER_GENERATED_CHARACTER_ID + LaptopSaveInfo.iVoiceId].bLife = (int8_t)iHealth;
  gMercProfiles[PLAYER_GENERATED_CHARACTER_ID + LaptopSaveInfo.iVoiceId].bAgility =
      (int8_t)iAgility;
  gMercProfiles[PLAYER_GENERATED_CHARACTER_ID + LaptopSaveInfo.iVoiceId].bStrength =
      (int8_t)iStrength;
  gMercProfiles[PLAYER_GENERATED_CHARACTER_ID + LaptopSaveInfo.iVoiceId].bDexterity =
      (int8_t)iDexterity;
  gMercProfiles[PLAYER_GENERATED_CHARACTER_ID + LaptopSaveInfo.iVoiceId].bWisdom = (int8_t)iWisdom;
  gMercProfiles[PLAYER_GENERATED_CHARACTER_ID + LaptopSaveInfo.iVoiceId].bLeadership =
      (int8_t)iLeadership;

  // skills
  gMercProfiles[PLAYER_GENERATED_CHARACTER_ID + LaptopSaveInfo.iVoiceId].bMarksmanship =
      (int8_t)iMarksmanship;
  gMercProfiles[PLAYER_GENERATED_CHARACTER_ID + LaptopSaveInfo.iVoiceId].bMedical =
      (int8_t)iMedical;
  gMercProfiles[PLAYER_GENERATED_CHARACTER_ID + LaptopSaveInfo.iVoiceId].bMechanical =
      (int8_t)iMechanical;
  gMercProfiles[PLAYER_GENERATED_CHARACTER_ID + LaptopSaveInfo.iVoiceId].bExplosive =
      (int8_t)iExplosives;

  // personality
  gMercProfiles[PLAYER_GENERATED_CHARACTER_ID + LaptopSaveInfo.iVoiceId].bPersonalityTrait =
      (int8_t)iPersonality;

  // attitude
  gMercProfiles[PLAYER_GENERATED_CHARACTER_ID + LaptopSaveInfo.iVoiceId].bAttitude =
      (int8_t)iAttitude;

  gMercProfiles[PLAYER_GENERATED_CHARACTER_ID + LaptopSaveInfo.iVoiceId].bExpLevel = 1;

  // set time away
  gMercProfiles[PLAYER_GENERATED_CHARACTER_ID + LaptopSaveInfo.iVoiceId].bMercStatus = 0;

  // face
  SelectMercFace();

  return;
}

BOOLEAN DoesCharacterHaveAnAttitude(void) {
  // simply checks if caracter has an attitude other than normal
  switch (iAttitude) {
    case ATT_LONER:
    case ATT_PESSIMIST:
    case ATT_ARROGANT:
    case ATT_BIG_SHOT:
    case ATT_ASSHOLE:
    case ATT_COWARD:
      return (TRUE);
    default:
      return (FALSE);
  }
}

BOOLEAN DoesCharacterHaveAPersoanlity(void) {
  // only one we can get is PSYCHO, and that is not much of a penalty
  return (FALSE);
  /*
  // simply checks if caracter has a personality other than normal
if( iPersonality != NO_PERSONALITYTRAIT )
  {
          // yep
    return ( TRUE );
  }
  else
  {
          // nope
          return ( FALSE );
  }
  */
}

void CreatePlayerAttitude(void) {
  // this function will 'roll a die' and decide if any attitude does exists
  int32_t iDiceValue = 0;
  int32_t iCounter = 0, iCounter2 = 0;

  int32_t iAttitudeHits[NUM_ATTITUDES] = {0};
  int32_t iHighestHits = 0;
  int32_t iNumAttitudesWithHighestHits = 0;

  iAttitude = ATT_NORMAL;

  if (iLastElementInAttitudeList == 0) {
    return;
  }

  // count # of hits for each attitude
  for (iCounter = 0; iCounter < iLastElementInAttitudeList; iCounter++) {
    iAttitudeHits[AttitudeList[iCounter]]++;
  }

  // find highest # of hits for any attitude
  for (iCounter = 0; iCounter < NUM_ATTITUDES; iCounter++) {
    if (iAttitudeHits[iCounter]) {
      if (iAttitudeHits[iCounter] > iHighestHits) {
        iHighestHits = max(iHighestHits, iAttitudeHits[iCounter]);
        iNumAttitudesWithHighestHits = 1;
      } else if (iAttitudeHits[iCounter] == iHighestHits) {
        iNumAttitudesWithHighestHits++;
      }
    }
  }

  // roll dice
  iDiceValue = Random(iNumAttitudesWithHighestHits + 1);

  // find attitude
  for (iCounter = 0; iCounter < NUM_ATTITUDES; iCounter++) {
    if (iAttitudeHits[iCounter] == iHighestHits) {
      if (iCounter2 == iDiceValue) {
        // this is it!
        iAttitude = iCounter2;
        break;
      } else {
        // one of the next attitudes...
        iCounter2++;
      }
    }
  }

  /*
          iAttitude = ATT_NORMAL;
          // set attitude
          if ( ( AttitudeList[ iDiceValue ] == ATT_LONER )||( AttitudeList[ iDiceValue ] >
     ATT_OPTIMIST ) )
          {
                  for ( iCounter = 0; iCounter < iLastElementInAttitudeList; iCounter++ )
                  {
                          if ( iCounter != iDiceValue )
                          {
                                  if ( AttitudeList[ iCounter ] == AttitudeList[ iDiceValue ] )
                                  {
            iAttitude = AttitudeList[ iDiceValue ];
                                  }
                          }
                  }
          }
          else
          {
            iAttitude = AttitudeList[ iDiceValue ];
          }
          */
}

void AddAnAttitudeToAttitudeList(int8_t bAttitude) {
  // adds an attitude to attitude list

  if (iLastElementInAttitudeList < ATTITUDE_LIST_SIZE) {
    // add element
    AttitudeList[iLastElementInAttitudeList] = (int32_t)bAttitude;

    // increment attitude list counter
    iLastElementInAttitudeList++;
  }

  return;
}

void AddSkillToSkillList(int8_t bSkill) {
  // adds a skill to skills list

  if (iLastElementInSkillsList < ATTITUDE_LIST_SIZE) {
    // add element
    SkillsList[iLastElementInSkillsList] = (int32_t)bSkill;

    // increment attitude list counter
    iLastElementInSkillsList++;
  }

  return;
}

void RemoveSkillFromSkillsList(int32_t iIndex) {
  int32_t iLoop;

  // remove a skill from the index given and shorten the list
  if (iIndex < iLastElementInSkillsList) {
    memset(BackupSkillsList, 0, ATTITUDE_LIST_SIZE * sizeof(int32_t));

    // use the backup array to create a version of the array without
    // this index
    for (iLoop = 0; iLoop < iIndex; iLoop++) {
      BackupSkillsList[iLoop] = SkillsList[iLoop];
    }
    for (iLoop = iIndex + 1; iLoop < iLastElementInSkillsList; iLoop++) {
      BackupSkillsList[iLoop - 1] = SkillsList[iLoop];
    }
    // now copy this over to the skills list
    memcpy(SkillsList, BackupSkillsList, ATTITUDE_LIST_SIZE * sizeof(int32_t));

    // reduce recorded size by 1
    iLastElementInSkillsList--;
  }
}

int32_t FindSkillInSkillsList(int32_t iSkill) {
  int32_t iLoop;

  for (iLoop = 0; iLoop < iLastElementInSkillsList; iLoop++) {
    if (SkillsList[iLoop] == iSkill) {
      return (iLoop);
    }
  }

  return (-1);
}

void ValidateSkillsList(void) {
  int32_t iIndex, iValue;
  MERCPROFILESTRUCT* pProfile;

  // remove from the generated traits list any traits that don't match
  // the character's skills
  pProfile = &(gMercProfiles[PLAYER_GENERATED_CHARACTER_ID + LaptopSaveInfo.iVoiceId]);
  if (pProfile->bMechanical == 0) {
    // without mechanical, electronics is useless
    iIndex = FindSkillInSkillsList(ELECTRONICS);
    while (iIndex != -1) {
      RemoveSkillFromSkillsList(iIndex);
      iIndex = FindSkillInSkillsList(ELECTRONICS);
    }
  }

  // special check for lockpicking
  iValue = pProfile->bMechanical;
  iValue = (iValue * pProfile->bWisdom) / 100;
  iValue = (iValue * pProfile->bDexterity) / 100;
  if (iValue + gbSkillTraitBonus[LOCKPICKING] < 50) {
    // not good enough for lockpicking!

    // so is lockpicking
    iIndex = FindSkillInSkillsList(LOCKPICKING);
    while (iIndex != -1) {
      RemoveSkillFromSkillsList(iIndex);
      iIndex = FindSkillInSkillsList(LOCKPICKING);
    }
  }

  if (pProfile->bMarksmanship == 0) {
    // without marksmanship, the following traits are useless:
    // auto weapons, heavy weapons
    iIndex = FindSkillInSkillsList(AUTO_WEAPS);
    while (iIndex != -1) {
      RemoveSkillFromSkillsList(iIndex);
      iIndex = FindSkillInSkillsList(AUTO_WEAPS);
    }
    // so is lockpicking
    iIndex = FindSkillInSkillsList(HEAVY_WEAPS);
    while (iIndex != -1) {
      RemoveSkillFromSkillsList(iIndex);
      iIndex = FindSkillInSkillsList(HEAVY_WEAPS);
    }
  }
}

void CreatePlayerSkills(void) {
  // this function will 'roll a die' and decide if any attitude does exists
  int32_t iDiceValue = 0;

  ValidateSkillsList();

  // roll dice
  iDiceValue = Random(iLastElementInSkillsList);

  // set attitude
  iSkillA = SkillsList[iDiceValue];

  // second dice value
  iDiceValue = Random(iLastElementInSkillsList);

  iSkillB = SkillsList[iDiceValue];

  // allow expert level for generated merc so you CAN have two of the same
  // but there is no such thing as expert level for electronics

  while (iSkillA == iSkillB && (iSkillB == ELECTRONICS || iSkillB == AMBIDEXT)) {
    // remove electronics as an option and roll again
    RemoveSkillFromSkillsList(iDiceValue);
    if (iLastElementInSkillsList == 0) {
      // ok, only one trait!
      iSkillB = NO_SKILLTRAIT;
      break;
    } else {
      iDiceValue = Random(iLastElementInSkillsList);
      iSkillB = SkillsList[iDiceValue];
    }
  }

  /*
  // are the same,
// reroll until different, or until ATTITUDE_LIST_SIZE times
  iSkillB = Random( iLastElementInSkillsList + 1 );

  while( (iSkillA == iSkillB ) && ( iCounter < HOW_MANY_ROLLS_FOR_SAME_SKILL_CHECK ) )
  {
          // increment counter
          iCounter++;

          // next random
iSkillB = Random( iLastElementInSkillsList + 1 );

  }
if( iCounter == ATTITUDE_LIST_SIZE )
  {
iSkillB = NO_SKILLTRAIT;
  }
  return;


  */
}

void AddAPersonalityToPersonalityList(int8_t bPersonlity) {
  // CJC, Oct 26 98: prevent personality list from being generated
  // because no dialogue was written to support PC personality quotes

  // BUT we can manage this for PSYCHO okay

  if (bPersonlity != PSYCHO) {
    return;
  }

  // will add a persoanlity to persoanlity list
  if (iLastElementInPersonalityList < ATTITUDE_LIST_SIZE) {
    // add element
    PersonalityList[iLastElementInPersonalityList] = (int32_t)bPersonlity;

    // increment attitude list counter
    iLastElementInPersonalityList++;
  }

  return;
}

void CreatePlayerPersonality(void) {
  // only psycho is available since we have no quotes
  // SO if the array is not empty, give them psycho!
  if (iLastElementInPersonalityList == 0) {
    iPersonality = NO_PERSONALITYTRAIT;
  } else {
    iPersonality = PSYCHO;
  }

  /*
    // this function will 'roll a die' and decide if any Personality does exists
    int32_t iDiceValue = 0;
    int32_t iCounter = 0;
          int32_t iSecondAttempt = -1;

          // roll dice
          iDiceValue = Random( iLastElementInPersonalityList + 1 );

          iPersonality = NO_PERSONALITYTRAIT;
    if( PersonalityList[ iDiceValue ] !=  NO_PERSONALITYTRAIT )
          {
                  for( iCounter = 0; iCounter < iLastElementInPersonalityList; iCounter++ )
                  {
                          if( iCounter != iDiceValue )
                          {
                                  if( PersonalityList[ iCounter ] == PersonalityList[ iDiceValue ] )
                                  {
                                          if( PersonalityList[ iDiceValue ] != PSYCHO )
                                          {
              iPersonality = PersonalityList[ iDiceValue ];
                                          }
                                          else
                                          {
              iSecondAttempt = iCounter;
                                          }
                                          if( iSecondAttempt != iCounter )
                                          {
                                                  iPersonality = PersonalityList[ iDiceValue ];
                                          }

                                  }
                          }
                  }
          }

          return;
  */
}

void CreatePlayersPersonalitySkillsAndAttitude(void) {
  // creates personality, skills and attitudes from curretly built list

  // personality
  CreatePlayerPersonality();

  // skills are now created later after stats have been chosen
  // CreatePlayerSkills( );

  // attitude
  CreatePlayerAttitude();

  return;
}

void ResetSkillsAttributesAndPersonality(void) {
  // reset count of skills attributes and personality

  iLastElementInPersonalityList = 0;

  iLastElementInSkillsList = 0;

  iLastElementInAttitudeList = 0;
}

void ResetIncrementCharacterAttributes(void) {
  // this resets any increments due to character generation

  // attributes
  iAddStrength = 0;
  iAddDexterity = 0;
  iAddWisdom = 0;
  iAddAgility = 0;
  iAddHealth = 0;
  iAddLeadership = 0;

  // skills
  iAddMarksmanship = 0;
  iAddExplosives = 0;
  iAddMedical = 0;
  iAddMechanical = 0;
}

void SelectMercFace(void) {
  // this procedure will select the approriate face for the merc and save offsets

  // grab face filename
  //  strcpy( gMercProfiles[ PLAYER_GENERATED_CHARACTER_ID + LaptopSaveInfo.iVoiceId
  //  ].ubUnusedFaceFileName , pPlayerSelectedFaceFileNames[ iPortraitNumber ]);

  // now the offsets
  gMercProfiles[PLAYER_GENERATED_CHARACTER_ID + LaptopSaveInfo.iVoiceId].ubFaceIndex =
      200 + (uint8_t)iPortraitNumber;

  // eyes
  gMercProfiles[PLAYER_GENERATED_CHARACTER_ID + LaptopSaveInfo.iVoiceId].usEyesX =
      sFacePositions[iPortraitNumber][0];
  gMercProfiles[PLAYER_GENERATED_CHARACTER_ID + LaptopSaveInfo.iVoiceId].usEyesY =
      sFacePositions[iPortraitNumber][1];

  // mouth
  gMercProfiles[PLAYER_GENERATED_CHARACTER_ID + LaptopSaveInfo.iVoiceId].usMouthX =
      sFacePositions[iPortraitNumber][2];
  gMercProfiles[PLAYER_GENERATED_CHARACTER_ID + LaptopSaveInfo.iVoiceId].usMouthY =
      sFacePositions[iPortraitNumber][3];

  // set merc skins and hair color
  SetMercSkinAndHairColors();
  return;
}

void SetMercSkinAndHairColors(void) {
  enum { PINKSKIN, TANSKIN, DARKSKIN, BLACKSKIN, NUMSKINS };
  enum {
    BROWNHEAD,
    BLACKHEAD,  // black skin (only this line )
    WHITEHEAD,  // dark skin (this line plus all above)
    BLONDHEAD,
    REDHEAD,  // pink/tan skin (this line plus all above )
    NUMHEADS
  };

  // skin strings
  char* sSkinStrings[] = {
      "PINKSKIN",
      "TANSKIN",
      "DARKSKIN",
      "BLACKSKIN",
  };

  // the hair colors
  char* sHairStrings[] = {
      "BROWNHEAD", "BLACKHEAD", "WHITEHEAD", "BLONDHEAD", "REDHEAD",
  };

  // given the portrait number, set the merc's skin and hair color
  int16_t sSkinColor = 0, sHairColor = 0;

  switch (iPortraitNumber) {
    case (0):
      sSkinColor = BLACKSKIN;
      sHairColor = BROWNHEAD;
      break;
    case (1):
      sSkinColor = TANSKIN;
      sHairColor = BROWNHEAD;
      break;
    case (2):
      sSkinColor = TANSKIN;
      sHairColor = BROWNHEAD;
      break;
    case (3):
      sSkinColor = DARKSKIN;
      sHairColor = BROWNHEAD;
      break;
    case (4):
      sSkinColor = TANSKIN;
      sHairColor = BROWNHEAD;
      break;
    case (5):
      sSkinColor = DARKSKIN;
      sHairColor = BLACKHEAD;
      break;
    case (6):
      sSkinColor = TANSKIN;
      sHairColor = BROWNHEAD;
      break;
    case (7):
      sSkinColor = TANSKIN;
      sHairColor = BROWNHEAD;
      break;
    case (8):
      sSkinColor = TANSKIN;
      sHairColor = BROWNHEAD;
      break;
    case (9):
      sSkinColor = PINKSKIN;
      sHairColor = BROWNHEAD;
      break;
    case (10):
      sSkinColor = TANSKIN;
      sHairColor = BLACKHEAD;
      break;
    case (11):
      sSkinColor = TANSKIN;
      sHairColor = BLACKHEAD;
      break;
    case (12):
      sSkinColor = PINKSKIN;
      sHairColor = BROWNHEAD;
      break;
    case (13):
      sSkinColor = BLACKSKIN;
      sHairColor = BROWNHEAD;
      break;
    case (14):
      sSkinColor = TANSKIN;
      sHairColor = REDHEAD;
      break;
    case (15):
      sSkinColor = TANSKIN;
      sHairColor = BLONDHEAD;
      break;
  }

  // now set them
  strcpy(gMercProfiles[PLAYER_GENERATED_CHARACTER_ID + LaptopSaveInfo.iVoiceId].HAIR,
         sHairStrings[sHairColor]);
  strcpy(gMercProfiles[PLAYER_GENERATED_CHARACTER_ID + LaptopSaveInfo.iVoiceId].SKIN,
         sSkinStrings[sSkinColor]);
}

void HandleMercStatsForChangesInFace() {
  if (fLoadingCharacterForPreviousImpProfile) {
    return;
  }

  // now figure out skills
  CreatePlayerSkills();

  // body type
  if (fCharacterIsMale) {
    // male
    // big or regular
    if (ShouldThisMercHaveABigBody()) {
      gMercProfiles[PLAYER_GENERATED_CHARACTER_ID + LaptopSaveInfo.iVoiceId].ubBodyType = BIGMALE;

      if (iSkillA == MARTIALARTS) {
        iSkillA = HANDTOHAND;
      }
      if (iSkillB == MARTIALARTS) {
        iSkillB = HANDTOHAND;
      }
    } else {
      gMercProfiles[PLAYER_GENERATED_CHARACTER_ID + LaptopSaveInfo.iVoiceId].ubBodyType = REGMALE;
    }
  } else {
    // female
    gMercProfiles[PLAYER_GENERATED_CHARACTER_ID + LaptopSaveInfo.iVoiceId].ubBodyType = REGFEMALE;

    if (iSkillA == MARTIALARTS) {
      iSkillA = HANDTOHAND;
    }
    if (iSkillB == MARTIALARTS) {
      iSkillB = HANDTOHAND;
    }
  }

  // skill trait
  gMercProfiles[PLAYER_GENERATED_CHARACTER_ID + LaptopSaveInfo.iVoiceId].bSkillTrait =
      (int8_t)iSkillA;
  gMercProfiles[PLAYER_GENERATED_CHARACTER_ID + LaptopSaveInfo.iVoiceId].bSkillTrait2 =
      (int8_t)iSkillB;
}

BOOLEAN ShouldThisMercHaveABigBody(void) {
  // should this merc be a big body typ
  if ((iPortraitNumber == 0) || (iPortraitNumber == 6) || (iPortraitNumber == 7)) {
    if (gMercProfiles[PLAYER_GENERATED_CHARACTER_ID + LaptopSaveInfo.iVoiceId].bStrength >= 75) {
      return (TRUE);
    }
  }

  return (FALSE);
}
