#include "Laptop/IMPAttributeSelection.h"

#include "Laptop/CharProfile.h"
#include "Laptop/IMPCompileCharacter.h"
#include "Laptop/IMPHomePage.h"
#include "Laptop/IMPMainPage.h"
#include "Laptop/IMPTextSystem.h"
#include "Laptop/IMPVideoObjects.h"
#include "Laptop/Laptop.h"
#include "SGP/ButtonSystem.h"
#include "SGP/Debug.h"
#include "SGP/Input.h"
#include "SGP/Video.h"
#include "SGP/WCheck.h"
#include "ScreenIDs.h"
#include "TileEngine/IsometricUtils.h"
#include "Utils/Cursors.h"
#include "Utils/EncryptedFile.h"
#include "Utils/Text.h"
#include "Utils/Utilities.h"
#include "Utils/WordWrap.h"

// width of the slider bar region
#define BAR_WIDTH 423 - 197

// width of the slider bar itself
#define SLIDER_BAR_WIDTH 37

// the sizeof one skill unit on the sliding bar in pixels
#define BASE_SKILL_PIXEL_UNIT_SIZE ((423 - 230))

enum {
  HEALTH_ATTRIBUTE,
  DEXTERITY_ATTRIBUTE,
  AGILITY_ATTRIBUTE,
  STRENGTH_ATTRIBUTE,
  WISDOM_ATTRIBUTE,
  LEADERSHIP_ATTRIBUTE,
  MARKSMANSHIP_SKILL,
  EXPLOSIVE_SKILL,
  MEDICAL_SKILL,
  MECHANICAL_SKILL,
};

enum {
  SLIDER_ERROR,
  SLIDER_OK,
  SLIDER_OUT_OF_RANGE,
};

// the skills as they stand
int32_t iCurrentStrength = 55;
int32_t iCurrentAgility = 55;
int32_t iCurrentDexterity = 55;
int32_t iCurrentHealth = 55;
int32_t iCurrentLeaderShip = 55;
int32_t iCurrentWisdom = 55;
int32_t iCurrentMarkmanship = 55;
int32_t iCurrentMechanical = 55;
int32_t iCurrentMedical = 55;
int32_t iCurrentExplosives = 55;

// which stat is message about stat at zero about
int32_t iCurrentStatAtZero = 0;

// total number of bonus points
int32_t iCurrentBonusPoints = 40;

// diplsay the 0 skill point warning..if skill set to 0, warn character
BOOLEAN fSkillAtZeroWarning = FALSE;

// is the sliding of the sliding bar active right now?
BOOLEAN fSlideIsActive = TRUE;

// first time in game
BOOLEAN fFirstIMPAttribTime = TRUE;

// review mode
BOOLEAN fReviewStats = FALSE;

// buttons
uint32_t giIMPAttributeSelectionButton[1];
uint32_t giIMPAttributeSelectionButtonImage[1];

// slider buttons
uint32_t giIMPAttributeSelectionSliderButton[20];
uint32_t giIMPAttributeSelectionSliderButtonImage[20];

// mouse regions
struct MOUSE_REGION pSliderRegions[10];
struct MOUSE_REGION pSliderBarRegions[10];

// The currently "anchored scroll bar"
struct MOUSE_REGION *gpCurrentScrollBox = NULL;
int32_t giCurrentlySelectedStat = -1;

// has any of the sliding bars moved?...for re-rendering puposes
BOOLEAN fHasAnySlidingBarMoved = FALSE;

int32_t uiBarToReRender = -1;

// are we actually coming back to edit, or are we restarting?
BOOLEAN fReturnStatus = FALSE;

// function definition
void ProcessAttributes(void);
void DestroyIMPAttributeSelectionButtons(void);
void CreateIMPAttributeSelectionButtons(void);
uint8_t IncrementStat(int32_t iStatToIncrement);
uint8_t DecrementStat(int32_t iStatToDecrement);
BOOLEAN DoWeHaveThisManyBonusPoints(int32_t iBonusPoints);
void CreateAttributeSliderButtons(void);
void DestroyAttributeSliderButtons(void);
void CreateSlideRegionMouseRegions(void);
void DestroySlideRegionMouseRegions(void);
int32_t GetCurrentAttributeValue(int32_t iAttribute);
void CreateSliderBarMouseRegions(void);
void DestroySlideBarMouseRegions(void);
void SetAttributes(void);
void DrawBonusPointsRemaining(void);
void SetGeneratedCharacterAttributes(void);

// callbacks
void BtnIMPAttributeFinishCallback(GUI_BUTTON *btn, int32_t reason);
void BtnIMPAttributeSliderLeftCallback(GUI_BUTTON *btn, int32_t reason);
void BtnIMPAttributeSliderRightCallback(GUI_BUTTON *btn, int32_t reason);
void SliderRegionButtonCallback(struct MOUSE_REGION *pRegion, int32_t iReason);
void SliderBarRegionButtonCallback(struct MOUSE_REGION *pRegion, int32_t iReason);
void StatAtZeroBoxCallBack(uint8_t bExitValue);

void EnterIMPAttributeSelection(void) {
  // set attributes and skills
  if ((fReturnStatus == FALSE) && (fFirstIMPAttribTime == TRUE)) {
    // re starting
    SetAttributes();

    gpCurrentScrollBox = NULL;
    giCurrentlySelectedStat = -1;

    // does character have PROBLEMS!!?!?!
    /*
if( DoesCharacterHaveAnAttitude() )
    {
iCurrentBonusPoints+= 10;
    }
if( DoesCharacterHaveAPersoanlity( ) )
    {
      iCurrentBonusPoints += 10;
    }
    */
  }
  fReturnStatus = TRUE;
  fFirstIMPAttribTime = FALSE;

  // create done button
  CreateIMPAttributeSelectionButtons();
  // create clider buttons
  CreateAttributeSliderButtons();

  // the mouse regions
  CreateSlideRegionMouseRegions();
  // CreateSliderBarMouseRegions( );

  // render background
  RenderIMPAttributeSelection();

  return;
}

void RenderIMPAlteredAttribute(void) {}
void RenderIMPAttributeSelection(void) {
  // the background
  RenderProfileBackGround();

  // attribute frame
  RenderAttributeFrame(51, 87);

  // render attribute boxes
  RenderAttributeBoxes();

  RenderAttrib1IndentFrame(51, 30);

  if (fReviewStats != TRUE) {
    RenderAttrib2IndentFrame(350, 42);
  }

  // reset rerender flag
  fHasAnySlidingBarMoved = FALSE;

  // print text for screen
  PrintImpText();

  // amt of bonus pts
  DrawBonusPointsRemaining();

  return;
}

void ExitIMPAttributeSelection(void) {
  // get rid of slider buttons
  DestroyAttributeSliderButtons();

  // the mouse regions
  DestroySlideRegionMouseRegions();
  // DestroySlideBarMouseRegions( );

  // get rid of done buttons
  DestroyIMPAttributeSelectionButtons();

  fReturnStatus = FALSE;

  return;
}

void HandleIMPAttributeSelection(void) {
  // review mode, do not allow changes
  if (fReviewStats) {
    return;
  }

  // set the currently selectd slider bar
  if (gfLeftButtonState && gpCurrentScrollBox != NULL) {
    // if theuser is holding down the mouse cursor to left of the start of the slider bars
    if (gusMouseXPos < (SKILL_SLIDE_START_X + LAPTOP_SCREEN_UL_X)) {
      DecrementStat(giCurrentlySelectedStat);
    }

    // else if the user is holding down the mouse button to the right of the scroll bars
    else if (gusMouseXPos > (LAPTOP_SCREEN_UL_X + SKILL_SLIDE_START_X + BAR_WIDTH)) {
      IncrementStat(giCurrentlySelectedStat);
    } else {
      int32_t iCurrentAttributeValue;
      int32_t sNewX = gusMouseXPos;
      int32_t iNewValue;

      // get old stat value
      iCurrentAttributeValue = GetCurrentAttributeValue(giCurrentlySelectedStat);
      sNewX = sNewX - (SKILL_SLIDE_START_X + LAPTOP_SCREEN_UL_X);
      iNewValue = (sNewX * 50) / BASE_SKILL_PIXEL_UNIT_SIZE + 35;

      // chenged, move mouse region if change large enough
      if (iCurrentAttributeValue != iNewValue) {
        // update screen
        fHasAnySlidingBarMoved = TRUE;
      }

      // change is enough
      if (iNewValue - iCurrentAttributeValue > 0) {
        // positive, increment stat
        for (int cnt = iNewValue - iCurrentAttributeValue; cnt > 0; cnt--) {
          IncrementStat(giCurrentlySelectedStat);
        }
      } else {
        // negative, decrement stat
        for (int cnt = iCurrentAttributeValue - iNewValue; cnt > 0; cnt--) {
          DecrementStat(giCurrentlySelectedStat);
        }
      }
    }

    RenderIMPAttributeSelection();
  } else {
    gpCurrentScrollBox = NULL;
    giCurrentlySelectedStat = -1;
  }

  // prcoess current state of attributes
  ProcessAttributes();

  // has any bar moved?
  if (fHasAnySlidingBarMoved) {
    // render
    if (uiBarToReRender == -1) {
      RenderIMPAttributeSelection();
    } else {
      RenderAttributeFrameForIndex(51, 87, uiBarToReRender);
      /*
      // print text for screen
      PrintImpText( );

      // amt of bonus pts
      DrawBonusPointsRemaining( );

      RenderAttributeFrame( 51, 87 );

      // render attribute boxes
      RenderAttributeBoxes( );

      PrintImpText( );

      InvalidateRegion( LAPTOP_SCREEN_UL_X + 51, LAPTOP_SCREEN_WEB_UL_Y + 87, LAPTOP_SCREEN_UL_X +
      51 + 400, LAPTOP_SCREEN_WEB_UL_Y + 87 + 220 );
*/
      uiBarToReRender = -1;
      MarkButtonsDirty();
    }

    fHasAnySlidingBarMoved = FALSE;
  }
  if (fSkillAtZeroWarning == TRUE) {
    DoLapTopMessageBox(MSG_BOX_IMP_STYLE, pSkillAtZeroWarning[0], LAPTOP_SCREEN, MSG_BOX_FLAG_YESNO,
                       StatAtZeroBoxCallBack);
    fSkillAtZeroWarning = FALSE;
  }
  return;
}

void ProcessAttributes(void) {
  // this function goes through and confirms thet state of attributes, ie not allowing attributes to
  // drop below 35 or skills to go below 0...and if skill is 34 set to 0

  // check any attribute below 35

  // strength
  if (iCurrentStrength <= 35) {
    iCurrentStrength = 35;
    // disable button too
  }

  // dex
  if (iCurrentDexterity <= 35) {
    iCurrentDexterity = 35;
    // disable button too
  }

  // agility
  if (iCurrentAgility <= 35) {
    iCurrentAgility = 35;
    // disable button too
  }

  // wisdom
  if (iCurrentWisdom <= 35) {
    iCurrentWisdom = 35;
    // disable button too
  }

  // leadership
  if (iCurrentLeaderShip <= 35) {
    iCurrentLeaderShip = 35;
    // disable button too
  }

  // health
  if (iCurrentHealth <= 35) {
    iCurrentHealth = 35;
    // disable button too
  }

  // now check for above 85
  // strength
  if (iCurrentStrength >= 85) {
    iCurrentStrength = 85;
    // disable button too
  }

  // dex
  if (iCurrentDexterity >= 85) {
    iCurrentDexterity = 85;
    // disable button too
  }

  // agility
  if (iCurrentAgility >= 85) {
    iCurrentAgility = 85;
    // disable button too
  }

  // wisdom
  if (iCurrentWisdom >= 85) {
    iCurrentWisdom = 85;
    // disable button too
  }

  // leadership
  if (iCurrentLeaderShip >= 85) {
    iCurrentLeaderShip = 85;
    // disable button too
  }

  // health
  if (iCurrentHealth >= 85) {
    iCurrentHealth = 85;
    // disable button too
  }

  return;
}

uint8_t IncrementStat(int32_t iStatToIncrement) {
  // this function is responsable for incrementing a stat

  // review mode, do not allow changes
  if (fReviewStats) {
    return (SLIDER_ERROR);
  }

  // make sure we have enough bonus points
  if (iCurrentBonusPoints < 1) {
    // nope...GO GET SOME BONUS POINTS, IDIOT!
    return (SLIDER_ERROR);
  }

  // check to make sure stat isn't maxed out already
  switch (iStatToIncrement) {
    case (STRENGTH_ATTRIBUTE):
      if (iCurrentStrength > 84) {
        // too high, leave
        return (SLIDER_OUT_OF_RANGE);
      } else {
        iCurrentStrength++;
        iCurrentBonusPoints--;
      }
      break;
    case (DEXTERITY_ATTRIBUTE):
      if (iCurrentDexterity > 84) {
        // too high, leave
        return (SLIDER_OUT_OF_RANGE);
      } else {
        iCurrentDexterity++;
        iCurrentBonusPoints--;
      }
      break;
    case (AGILITY_ATTRIBUTE):
      if (iCurrentAgility > 84) {
        // too high, leave
        return (SLIDER_OUT_OF_RANGE);
      } else {
        iCurrentAgility++;
        iCurrentBonusPoints--;
      }
      break;
    case (LEADERSHIP_ATTRIBUTE):
      if (iCurrentLeaderShip > 84) {
        // too high, leave
        return (SLIDER_OUT_OF_RANGE);
      } else {
        iCurrentLeaderShip++;
        iCurrentBonusPoints--;
      }
      break;
    case (WISDOM_ATTRIBUTE):
      if (iCurrentWisdom > 84) {
        // too high, leave
        return (SLIDER_OUT_OF_RANGE);
      } else {
        iCurrentWisdom++;
        iCurrentBonusPoints--;
      }
      break;
    case (HEALTH_ATTRIBUTE):
      if (iCurrentHealth > 84) {
        // too high, leave
        return (SLIDER_OUT_OF_RANGE);
      } else {
        iCurrentHealth++;
        iCurrentBonusPoints--;
      }
      break;
    case (MARKSMANSHIP_SKILL):
      if (iCurrentMarkmanship > 84) {
        // too high, leave
        return (SLIDER_OUT_OF_RANGE);
      } else {
        if (iCurrentMarkmanship == 0) {
          if (DoWeHaveThisManyBonusPoints(15) == TRUE) {
            iCurrentMarkmanship += 35;
            iCurrentBonusPoints -= 15;
            fSkillAtZeroWarning = FALSE;
          } else {
            return (SLIDER_OK);
          }
        } else {
          iCurrentMarkmanship++;
          iCurrentBonusPoints--;
        }
      }
      break;
    case (MECHANICAL_SKILL):
      if (iCurrentMechanical > 84) {
        // too high, leave
        return (SLIDER_OUT_OF_RANGE);
      } else {
        if (iCurrentMechanical == 0) {
          if (DoWeHaveThisManyBonusPoints(15) == TRUE) {
            iCurrentMechanical += 35;
            iCurrentBonusPoints -= 15;
            fSkillAtZeroWarning = FALSE;
          } else {
            return (SLIDER_OK);
          }
        } else {
          iCurrentMechanical++;
          iCurrentBonusPoints--;
        }
      }
      break;
    case (MEDICAL_SKILL):
      if (iCurrentMedical > 84) {
        // too high, leave
        return (SLIDER_OUT_OF_RANGE);
      } else {
        if (iCurrentMedical == 0) {
          if (DoWeHaveThisManyBonusPoints(15) == TRUE) {
            iCurrentMedical += 35;
            iCurrentBonusPoints -= 15;
            fSkillAtZeroWarning = FALSE;
          } else {
            return (SLIDER_OK);
          }
        } else {
          iCurrentMedical++;
          iCurrentBonusPoints--;
        }
      }
      break;
    case (EXPLOSIVE_SKILL):
      if (iCurrentExplosives > 84) {
        // too high, leave
        return (SLIDER_OUT_OF_RANGE);
      } else {
        if (iCurrentExplosives == 0) {
          if (DoWeHaveThisManyBonusPoints(15) == TRUE) {
            iCurrentExplosives += 35;
            iCurrentBonusPoints -= 15;
            fSkillAtZeroWarning = FALSE;
          } else {
            return (SLIDER_OK);
          }
        } else {
          iCurrentExplosives++;
          iCurrentBonusPoints--;
        }
      }
      break;
  }

  return (SLIDER_OK);
}

uint8_t DecrementStat(int32_t iStatToDecrement) {
  // review mode, do not allow changes
  if (fReviewStats) {
    return (SLIDER_ERROR);
  }

  // decrement a stat
  // check to make sure stat isn't maxed out already
  switch (iStatToDecrement) {
    case (STRENGTH_ATTRIBUTE):
      if (iCurrentStrength > 35) {
        // ok to decrement
        iCurrentStrength--;
        iCurrentBonusPoints++;
      } else {
        return (SLIDER_OUT_OF_RANGE);
      }
      break;
    case (DEXTERITY_ATTRIBUTE):
      if (iCurrentDexterity > 35) {
        // ok to decrement
        iCurrentDexterity--;
        iCurrentBonusPoints++;
      } else {
        return (SLIDER_OUT_OF_RANGE);
      }
      break;
    case (AGILITY_ATTRIBUTE):
      if (iCurrentAgility > 35) {
        // ok to decrement
        iCurrentAgility--;
        iCurrentBonusPoints++;
      } else {
        return (SLIDER_OUT_OF_RANGE);
      }
      break;
    case (WISDOM_ATTRIBUTE):
      if (iCurrentWisdom > 35) {
        // ok to decrement
        iCurrentWisdom--;
        iCurrentBonusPoints++;
      } else {
        return (SLIDER_OUT_OF_RANGE);
      }
      break;
    case (LEADERSHIP_ATTRIBUTE):
      if (iCurrentLeaderShip > 35) {
        // ok to decrement
        iCurrentLeaderShip--;
        iCurrentBonusPoints++;
      } else {
        return (SLIDER_OUT_OF_RANGE);
      }
      break;
    case (HEALTH_ATTRIBUTE):
      if (iCurrentHealth > 35) {
        // ok to decrement
        iCurrentHealth--;
        iCurrentBonusPoints++;
      } else {
        return (SLIDER_OUT_OF_RANGE);
      }
      break;
    case (MARKSMANSHIP_SKILL):
      if (iCurrentMarkmanship > 35) {
        // ok to decrement
        iCurrentMarkmanship--;
        iCurrentBonusPoints++;
      } else if (iCurrentMarkmanship == 35) {
        // ok to decrement
        iCurrentMarkmanship -= 35;
        iCurrentBonusPoints += 15;
        fSkillAtZeroWarning = TRUE;
      }
      break;
    case (MEDICAL_SKILL):
      if (iCurrentMedical > 35) {
        // ok to decrement
        iCurrentMedical--;
        iCurrentBonusPoints++;
      } else if (iCurrentMedical == 35) {
        // ok to decrement
        iCurrentMedical -= 35;
        iCurrentBonusPoints += 15;
        fSkillAtZeroWarning = TRUE;
      }
      break;
    case (MECHANICAL_SKILL):
      if (iCurrentMechanical > 35) {
        // ok to decrement
        iCurrentMechanical--;
        iCurrentBonusPoints++;
      } else if (iCurrentMechanical == 35) {
        // ok to decrement
        iCurrentMechanical -= 35;
        iCurrentBonusPoints += 15;
        fSkillAtZeroWarning = TRUE;
      }
      break;
    case (EXPLOSIVE_SKILL):
      if (iCurrentExplosives > 35) {
        // ok to decrement
        iCurrentExplosives--;
        iCurrentBonusPoints++;
      } else if (iCurrentExplosives == 35) {
        // ok to decrement
        iCurrentExplosives -= 35;
        iCurrentBonusPoints += 15;
        fSkillAtZeroWarning = TRUE;
      }
      break;
  }

  if (fSkillAtZeroWarning == TRUE) {
    // current stat at zero
    iCurrentStatAtZero = iStatToDecrement;
  }

  return (SLIDER_OK);
}

BOOLEAN DoWeHaveThisManyBonusPoints(int32_t iBonusPoints) {
  // returns if player has at least this many bonus points
  if (iCurrentBonusPoints >= iBonusPoints) {
    // yep, return true
    return (TRUE);
  } else {
    // nope, return false
    return (FALSE);
  }
}

void CreateIMPAttributeSelectionButtons(void) {
  // the finished button
  giIMPAttributeSelectionButtonImage[0] = LoadButtonImage("LAPTOP\\button_2.sti", -1, 0, -1, 1, -1);
  /*	giIMPAttributeSelectionButton[0] = QuickCreateButton( giIMPAttributeSelectionButtonImage[0],
     LAPTOP_SCREEN_UL_X +  ( 136 ), LAPTOP_SCREEN_WEB_UL_Y + ( 314 ), BUTTON_TOGGLE,
     MSYS_PRIORITY_HIGHEST - 1, BtnGenericMouseMoveButtonCallback,
     (GUI_CALLBACK)BtnIMPAttributeFinishCallback );
    */
  giIMPAttributeSelectionButton[0] = CreateIconAndTextButton(
      giIMPAttributeSelectionButtonImage[0], pImpButtonText[11], FONT12ARIAL, FONT_WHITE,
      DEFAULT_SHADOW, FONT_WHITE, DEFAULT_SHADOW, TEXT_CJUSTIFIED, LAPTOP_SCREEN_UL_X + (136),
      LAPTOP_SCREEN_WEB_UL_Y + (314), BUTTON_TOGGLE, MSYS_PRIORITY_HIGH,
      BtnGenericMouseMoveButtonCallback, (GUI_CALLBACK)BtnIMPAttributeFinishCallback);

  SetButtonCursor(giIMPAttributeSelectionButton[0], CURSOR_WWW);
  return;
}

void DestroyIMPAttributeSelectionButtons(void) {
  // this function will destroy the buttons needed for the IMP attrib enter page

  // the begin  button
  RemoveButton(giIMPAttributeSelectionButton[0]);
  UnloadButtonImage(giIMPAttributeSelectionButtonImage[0]);

  return;
}

void BtnIMPAttributeFinishCallback(GUI_BUTTON *btn, int32_t reason) {
  // btn callback for IMP attrbite begin button
  if (!(btn->uiFlags & BUTTON_ENABLED)) return;

  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    btn->uiFlags |= (BUTTON_CLICKED_ON);
  } else if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    if (btn->uiFlags & BUTTON_CLICKED_ON) {
      btn->uiFlags &= ~(BUTTON_CLICKED_ON);

      // are we done diting, or just reviewing the stats?
      if (fReviewStats == TRUE) {
        iCurrentImpPage = IMP_FINISH;
      } else {
        iCurrentImpPage = IMP_ATTRIBUTE_FINISH;
      }
      fButtonPendingFlag = TRUE;
    }
  }
}

void RenderAttributeBoxes(void) {
  // this function will render the boxes in the sliding attribute bar, based on position
  int32_t iCnt = STRENGTH_ATTRIBUTE;
  int16_t sX = 0;
  int16_t sY = 0;
  CHAR16 sString[3];

  // set last char to null
  sString[2] = 0;

  // font stuff
  SetFont(FONT10ARIAL);
  SetFontShadow(NO_SHADOW);
  SetFontForeground(FONT_WHITE);
  SetFontBackground(FONT_BLACK);

  // run through and render each slider bar
  for (iCnt = HEALTH_ATTRIBUTE; iCnt <= MECHANICAL_SKILL; iCnt++) {
    // position is  ( width *  ( stat - 35 ) ) /50]
    // unless 0, then it is 0 - for skills

    // get y position
    sY = SKILL_SLIDE_START_Y + SKILL_SLIDE_HEIGHT * ((int16_t)iCnt);

    switch (iCnt) {
      case (STRENGTH_ATTRIBUTE):
        // blt in strength slider
        sX = ((iCurrentStrength - 35) * BASE_SKILL_PIXEL_UNIT_SIZE) / 50;
        sX += SKILL_SLIDE_START_X;
        RenderSliderBar(sX, sY);

        // set sliderbar mouse region
        MSYS_MoveMouseRegionTo(&pSliderBarRegions[iCnt], (int16_t)(sX + LAPTOP_SCREEN_UL_X),
                               (int16_t)(sY + LAPTOP_SCREEN_WEB_UL_Y));

        // the text
        swprintf(sString, ARR_SIZE(sString), L"%d", iCurrentStrength);
        sX += LAPTOP_SCREEN_UL_X;
        sY += LAPTOP_SCREEN_WEB_UL_Y;
        mprintf(sX + 13, sY + 3, sString);
        break;
      case (DEXTERITY_ATTRIBUTE):
        // blt in strength slider
        sX = ((iCurrentDexterity - 35) * BASE_SKILL_PIXEL_UNIT_SIZE) / 50;
        sX += SKILL_SLIDE_START_X;
        RenderSliderBar(sX, sY);

        // set sliderbar mouse region
        MSYS_MoveMouseRegionTo(&pSliderBarRegions[iCnt], (int16_t)(sX + LAPTOP_SCREEN_UL_X),
                               (int16_t)(sY + LAPTOP_SCREEN_WEB_UL_Y));

        // the text
        swprintf(sString, ARR_SIZE(sString), L"%d", iCurrentDexterity);
        sX += LAPTOP_SCREEN_UL_X;
        sY += LAPTOP_SCREEN_WEB_UL_Y;
        mprintf(sX + 13, sY + 3, sString);

        break;
      case (AGILITY_ATTRIBUTE):
        // blt in strength slider
        sX = ((iCurrentAgility - 35) * BASE_SKILL_PIXEL_UNIT_SIZE) / 50;
        sX += SKILL_SLIDE_START_X;
        RenderSliderBar(sX, sY);

        // set sliderbar mouse region
        MSYS_MoveMouseRegionTo(&pSliderBarRegions[iCnt], (int16_t)(sX + LAPTOP_SCREEN_UL_X),
                               (int16_t)(sY + LAPTOP_SCREEN_WEB_UL_Y));

        // the text
        swprintf(sString, ARR_SIZE(sString), L"%d", iCurrentAgility);
        sX += LAPTOP_SCREEN_UL_X;
        sY += LAPTOP_SCREEN_WEB_UL_Y;
        mprintf(sX + 13, sY + 3, sString);

        break;
      case (WISDOM_ATTRIBUTE):
        // blt in strength slider
        sX = ((iCurrentWisdom - 35) * BASE_SKILL_PIXEL_UNIT_SIZE) / 50;
        sX += SKILL_SLIDE_START_X;
        RenderSliderBar(sX, sY);

        // set sliderbar mouse region
        MSYS_MoveMouseRegionTo(&pSliderBarRegions[iCnt], (int16_t)(sX + LAPTOP_SCREEN_UL_X),
                               (int16_t)(sY + LAPTOP_SCREEN_WEB_UL_Y));

        // the text
        swprintf(sString, ARR_SIZE(sString), L"%d", iCurrentWisdom);
        sX += LAPTOP_SCREEN_UL_X;
        sY += LAPTOP_SCREEN_WEB_UL_Y;
        mprintf(sX + 13, sY + 3, sString);
        break;
      case (LEADERSHIP_ATTRIBUTE):
        // blt in strength slider
        sX = ((iCurrentLeaderShip - 35) * BASE_SKILL_PIXEL_UNIT_SIZE) / 50;
        sX += SKILL_SLIDE_START_X;
        RenderSliderBar(sX, sY);
        // set sliderbar mouse region
        MSYS_MoveMouseRegionTo(&pSliderBarRegions[iCnt], (int16_t)(sX + LAPTOP_SCREEN_UL_X),
                               (int16_t)(sY + LAPTOP_SCREEN_WEB_UL_Y));

        // the text
        swprintf(sString, ARR_SIZE(sString), L"%d", iCurrentLeaderShip);
        sX += LAPTOP_SCREEN_UL_X;
        sY += LAPTOP_SCREEN_WEB_UL_Y;
        mprintf(sX + 13, sY + 3, sString);
        break;
      case (HEALTH_ATTRIBUTE):
        // blt in health slider
        sX = ((iCurrentHealth - 35) * BASE_SKILL_PIXEL_UNIT_SIZE) / 50;
        sX += SKILL_SLIDE_START_X;
        RenderSliderBar(sX, sY);

        // set sliderbar mouse region
        MSYS_MoveMouseRegionTo(&pSliderBarRegions[iCnt], (int16_t)(sX + LAPTOP_SCREEN_UL_X),
                               (int16_t)(sY + LAPTOP_SCREEN_WEB_UL_Y));

        // the text
        swprintf(sString, ARR_SIZE(sString), L"%d", iCurrentHealth);
        sY += LAPTOP_SCREEN_WEB_UL_Y;
        sX += LAPTOP_SCREEN_UL_X;
        mprintf(sX + 13, sY + 3, sString);
        break;
      case (MARKSMANSHIP_SKILL):
        // blt in marksmanship slider

        sX = ((iCurrentMarkmanship - 35) * BASE_SKILL_PIXEL_UNIT_SIZE) / 50;
        // if less than zero..a zero'ed skill...reset to zero
        if (sX < 0) {
          sX = 0;
        }

        sX += SKILL_SLIDE_START_X;
        RenderSliderBar(sX, sY);
        // set sliderbar mouse region
        MSYS_MoveMouseRegionTo(&pSliderBarRegions[iCnt], (int16_t)(sX + LAPTOP_SCREEN_UL_X),
                               (int16_t)(sY + LAPTOP_SCREEN_WEB_UL_Y));

        // the text
        swprintf(sString, ARR_SIZE(sString), L"%d", iCurrentMarkmanship);
        sY += LAPTOP_SCREEN_WEB_UL_Y;
        sX += LAPTOP_SCREEN_UL_X;
        mprintf(sX + 13, sY + 3, sString);
        break;
      case (MEDICAL_SKILL):
        // blt in medical slider

        sX = ((iCurrentMedical - 35) * BASE_SKILL_PIXEL_UNIT_SIZE) / 50;
        // if less than zero..a zero'ed skill...reset to zero
        if (sX < 0) {
          sX = 0;
        }

        sX += SKILL_SLIDE_START_X;
        RenderSliderBar(sX, sY);

        // set sliderbar mouse region
        MSYS_MoveMouseRegionTo(&pSliderBarRegions[iCnt], (int16_t)(sX + LAPTOP_SCREEN_UL_X),
                               (int16_t)(sY + LAPTOP_SCREEN_WEB_UL_Y));

        // the text
        swprintf(sString, ARR_SIZE(sString), L"%d", iCurrentMedical);
        sY += LAPTOP_SCREEN_WEB_UL_Y;
        sX += LAPTOP_SCREEN_UL_X;
        mprintf(sX + 13, sY + 3, sString);
        break;
      case (MECHANICAL_SKILL):
        // blt in mech slider

        sX = ((iCurrentMechanical - 35) * BASE_SKILL_PIXEL_UNIT_SIZE) / 50;
        // if less than zero..a zero'ed skill...reset to zero
        if (sX < 0) {
          sX = 0;
        }

        sX += SKILL_SLIDE_START_X;
        RenderSliderBar(sX, sY);

        // set sliderbar mouse region
        MSYS_MoveMouseRegionTo(&pSliderBarRegions[iCnt], (int16_t)(sX + LAPTOP_SCREEN_UL_X),
                               (int16_t)(sY + LAPTOP_SCREEN_WEB_UL_Y));

        // the text
        swprintf(sString, ARR_SIZE(sString), L"%d", iCurrentMechanical);
        sY += LAPTOP_SCREEN_WEB_UL_Y;
        sX += LAPTOP_SCREEN_UL_X;
        mprintf(sX + 13, sY + 3, sString);
        break;
      case (EXPLOSIVE_SKILL):
        // blt in explosive slider

        sX = ((iCurrentExplosives - 35) * BASE_SKILL_PIXEL_UNIT_SIZE) / 50;
        // if less than zero..a zero'ed skill...reset to zero
        if (sX < 0) {
          sX = 0;
        }

        sX += SKILL_SLIDE_START_X;
        RenderSliderBar(sX, sY);

        // set sliderbar mouse region
        MSYS_MoveMouseRegionTo(&pSliderBarRegions[iCnt], (int16_t)(sX + LAPTOP_SCREEN_UL_X),
                               (int16_t)(sY + LAPTOP_SCREEN_WEB_UL_Y));

        // the text
        swprintf(sString, ARR_SIZE(sString), L"%d", iCurrentExplosives);
        sY += LAPTOP_SCREEN_WEB_UL_Y;
        sX += LAPTOP_SCREEN_UL_X;
        mprintf(sX + 13, sY + 3, sString);
        break;
    }
  }

  // reset shadow
  SetFontShadow(DEFAULT_SHADOW);

  return;
}

void CreateAttributeSliderButtons(void) {
  // this function will create the buttons for the attribute slider
  // the finished button
  int32_t iCounter = 0;

  giIMPAttributeSelectionSliderButtonImage[0] =
      LoadButtonImage("LAPTOP\\AttributeArrows.sti", -1, 0, -1, 1, -1);
  giIMPAttributeSelectionSliderButtonImage[1] =
      LoadButtonImage("LAPTOP\\AttributeArrows.sti", -1, 3, -1, 4, -1);

  for (iCounter = 0; iCounter < 20; iCounter += 2) {
    // left button - decrement stat
    giIMPAttributeSelectionSliderButton[iCounter] = QuickCreateButton(
        giIMPAttributeSelectionSliderButtonImage[0], LAPTOP_SCREEN_UL_X + (163),
        (int16_t)(LAPTOP_SCREEN_WEB_UL_Y + (99 + iCounter / 2 * 20)), BUTTON_TOGGLE,
        MSYS_PRIORITY_HIGHEST - 1, BtnGenericMouseMoveButtonCallback,
        (GUI_CALLBACK)BtnIMPAttributeSliderLeftCallback);

    // right button - increment stat
    giIMPAttributeSelectionSliderButton[iCounter + 1] = QuickCreateButton(
        giIMPAttributeSelectionSliderButtonImage[1], LAPTOP_SCREEN_UL_X + (419),
        (int16_t)(LAPTOP_SCREEN_WEB_UL_Y + (99 + iCounter / 2 * 20)), BUTTON_TOGGLE,
        MSYS_PRIORITY_HIGHEST - 1, BtnGenericMouseMoveButtonCallback,
        (GUI_CALLBACK)BtnIMPAttributeSliderRightCallback);

    SetButtonCursor(giIMPAttributeSelectionSliderButton[iCounter], CURSOR_WWW);
    SetButtonCursor(giIMPAttributeSelectionSliderButton[iCounter + 1], CURSOR_WWW);
    // set user data
    MSYS_SetBtnUserData(giIMPAttributeSelectionSliderButton[iCounter], 0, iCounter / 2);
    MSYS_SetBtnUserData(giIMPAttributeSelectionSliderButton[iCounter + 1], 0, iCounter / 2);
  }

  MarkButtonsDirty();
}

void DestroyAttributeSliderButtons(void) {
  // this function will destroy the buttons used for attribute manipulation
  int32_t iCounter = 0;

  // get rid of image
  UnloadButtonImage(giIMPAttributeSelectionSliderButtonImage[0]);
  UnloadButtonImage(giIMPAttributeSelectionSliderButtonImage[1]);

  for (iCounter = 0; iCounter < 20; iCounter++) {
    // get rid of button
    RemoveButton(giIMPAttributeSelectionSliderButton[iCounter]);
  }

  return;
}

void BtnIMPAttributeSliderLeftCallback(GUI_BUTTON *btn, int32_t reason) {
  int32_t iValue = -1;

  // btn callback for IMP personality quiz answer button
  if (!(btn->uiFlags & BUTTON_ENABLED)) return;

  iValue = (int32_t)MSYS_GetBtnUserData(btn, 0);

  if (reason & MSYS_CALLBACK_REASON_LBUTTON_REPEAT) {
    DecrementStat(iValue);
    // stat has changed, rerender
    fHasAnySlidingBarMoved = TRUE;
    uiBarToReRender = iValue;
  } else if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    DecrementStat(iValue);
    fHasAnySlidingBarMoved = TRUE;
    btn->uiFlags |= (BUTTON_CLICKED_ON);
    uiBarToReRender = iValue;
  }

  else if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    if (btn->uiFlags & BUTTON_CLICKED_ON) {
      btn->uiFlags &= ~(BUTTON_CLICKED_ON);
    }
  }
}

void BtnIMPAttributeSliderRightCallback(GUI_BUTTON *btn, int32_t reason) {
  int32_t iValue = -1;

  // btn callback for IMP personality quiz answer button
  if (!(btn->uiFlags & BUTTON_ENABLED)) return;

  iValue = (int32_t)MSYS_GetBtnUserData(btn, 0);

  if (reason & MSYS_CALLBACK_REASON_LBUTTON_REPEAT) {
    IncrementStat(iValue);
    // stat has changed, rerender
    fHasAnySlidingBarMoved = TRUE;
    uiBarToReRender = iValue;
  } else if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    IncrementStat(iValue);
    fHasAnySlidingBarMoved = TRUE;
    uiBarToReRender = iValue;
    btn->uiFlags |= (BUTTON_CLICKED_ON);

  }

  else if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    if (btn->uiFlags & BUTTON_CLICKED_ON) {
      btn->uiFlags &= ~(BUTTON_CLICKED_ON);
    }
  }
}

void CreateSlideRegionMouseRegions(void) {
  // this function will create that mouse regions on the sliding area, that, if the player clicks
  // on, the bar will automatically jump to
  int32_t iCounter = 0;

  for (iCounter = 0; iCounter < 10; iCounter++) {
    // define the region
    MSYS_DefineRegion(
        &pSliderRegions[iCounter], (int16_t)(SKILL_SLIDE_START_X + LAPTOP_SCREEN_UL_X),
        (int16_t)(LAPTOP_SCREEN_WEB_UL_Y + SKILL_SLIDE_START_Y + iCounter * SKILL_SLIDE_HEIGHT),
        (int16_t)(LAPTOP_SCREEN_UL_X + SKILL_SLIDE_START_X + BAR_WIDTH),
        (int16_t)(LAPTOP_SCREEN_WEB_UL_Y + SKILL_SLIDE_START_Y + iCounter * SKILL_SLIDE_HEIGHT +
                  15),
        MSYS_PRIORITY_HIGH + 2, CURSOR_WWW, MSYS_NO_CALLBACK, SliderRegionButtonCallback);

    // define user data
    MSYS_SetRegionUserData(&pSliderRegions[iCounter], 0, iCounter);
    // now add it
    MSYS_AddRegion(&pSliderRegions[iCounter]);
  }

  return;
}

void CreateSliderBarMouseRegions(void) {
  // this function will create that mouse regions on the sliding bars, that, if the player clicks
  // on, the bar will automatically jump to
  int32_t iCounter = 0;
  int16_t sX = 0;

  // set the starting X
  sX = (((55 - 35) * BASE_SKILL_PIXEL_UNIT_SIZE) / 50) + SKILL_SLIDE_START_X + LAPTOP_SCREEN_UL_X;

  for (iCounter = 0; iCounter < 10; iCounter++) {
    // define the region
    MSYS_DefineRegion(
        &pSliderBarRegions[iCounter], (int16_t)(sX),
        (int16_t)(LAPTOP_SCREEN_WEB_UL_Y + SKILL_SLIDE_START_Y + iCounter * SKILL_SLIDE_HEIGHT),
        (int16_t)(sX + SLIDER_BAR_WIDTH),
        (int16_t)(LAPTOP_SCREEN_WEB_UL_Y + SKILL_SLIDE_START_Y + iCounter * SKILL_SLIDE_HEIGHT +
                  15),
        MSYS_PRIORITY_HIGH + 2, CURSOR_WWW, MSYS_NO_CALLBACK, SliderBarRegionButtonCallback);

    // define user data
    MSYS_SetRegionUserData(&pSliderBarRegions[iCounter], 0, iCounter);
    // now add it
    MSYS_AddRegion(&pSliderBarRegions[iCounter]);
  }

  return;
}

void DestroySlideRegionMouseRegions(void) {
  // this function will destroy the regions user for the slider ' jumping'
  int32_t iCounter = 0;

  // delete the regions
  for (iCounter = 0; iCounter < 10; iCounter++) {
    MSYS_RemoveRegion(&pSliderRegions[iCounter]);
  }

  return;
}

void DestroySlideBarMouseRegions(void) {
  // this function will destroy the regions user for the slider ' jumping'
  int32_t iCounter = 0;

  // delete the regions
  for (iCounter = 0; iCounter < 10; iCounter++) {
    MSYS_RemoveRegion(&pSliderBarRegions[iCounter]);
  }

  return;
}

void SliderRegionButtonCallback(struct MOUSE_REGION *pRegion, int32_t iReason) {
  int32_t iCurrentAttributeValue = 0;
  int32_t iNewAttributeValue = 0;
  int32_t iAttributeDelta = 0;
  int32_t iCounter = 0;
  int16_t sX = 0;
  static int16_t sOldX = -1;
  static int32_t iAttribute = -1;
  int32_t iNewValue = 0;
  int16_t sNewX = -1;

  // if we already have an anchored slider bar
  if (gpCurrentScrollBox != pRegion && gpCurrentScrollBox != NULL) return;

  if (iReason & MSYS_CALLBACK_REASON_INIT) {
    return;
  }
  if (iReason & MSYS_CALLBACK_REASON_LBUTTON_REPEAT) {
    if (fSlideIsActive == FALSE) {
      // not active leave
      return;
    }

    // check to see if we have moved
    if (MSYS_GetRegionUserData(pRegion, 0) != iAttribute) {
      // different regions
      iAttribute = MSYS_GetRegionUserData(pRegion, 0);
      sOldX = -1;
      return;
    }

    uiBarToReRender = iAttribute;

    giCurrentlySelectedStat = iAttribute;
    gpCurrentScrollBox = pRegion;

    // get new attribute value x
    sNewX = pRegion->MouseXPos;

    // sOldX has been reset, set to sNewX
    if (sOldX == -1) {
      sOldX = sNewX;
      return;
    }
    // check against old x
    if (sNewX != sOldX) {
      // get old stat value
      iCurrentAttributeValue = GetCurrentAttributeValue(iAttribute);
      sNewX = sNewX - (SKILL_SLIDE_START_X + LAPTOP_SCREEN_UL_X);
      iNewValue = (sNewX * 50) / BASE_SKILL_PIXEL_UNIT_SIZE + 35;

      // chenged, move mouse region if change large enough
      if (iCurrentAttributeValue != iNewValue) {
        // update screen
        fHasAnySlidingBarMoved = TRUE;
      }

      // change is enough
      if (iNewValue - iCurrentAttributeValue > 0) {
        // positive, increment stat
        for (int cnt = iNewValue - iCurrentAttributeValue; cnt > 0; cnt--) {
          IncrementStat(iAttribute);
        }
      } else {
        // negative, decrement stat
        for (int cnt = iCurrentAttributeValue - iNewValue; cnt > 0; cnt--) {
          DecrementStat(iAttribute);
        }
      }

      sOldX = sNewX;
    }
  } else if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    if (fSlideIsActive) {
      // reset slide is active flag
      fSlideIsActive = FALSE;
      return;
    }

    // get mouse XY

    sX = pRegion->MouseXPos;

    // which region are we in?

    // get attribute
    iAttribute = MSYS_GetRegionUserData(pRegion, 0);
    uiBarToReRender = iAttribute;

    // get value of attribute
    iCurrentAttributeValue = GetCurrentAttributeValue(iAttribute);

    // set the new attribute value based on position of mouse click
    iNewAttributeValue = ((sX - SKILL_SLIDE_START_X) * 50) / BASE_SKILL_PIXEL_UNIT_SIZE;

    // too high, reset to 85
    if (iNewAttributeValue > 85) {
      iNewAttributeValue = 85;
    }

    // get the delta
    iAttributeDelta = iCurrentAttributeValue - iNewAttributeValue;

    // set Counter
    iCounter = iAttributeDelta;

    // check if increment or decrement
    if (iAttributeDelta > 0) {
      // decrement
      for (iCounter = 0; iCounter < iAttributeDelta; iCounter++) {
        DecrementStat(iAttribute);
      }
    } else {
      // increment attribute
      for (iCounter = iAttributeDelta; iCounter < 0; iCounter++) {
        if (iCurrentAttributeValue == 0) {
          iCounter = 0;
        }
        IncrementStat(iAttribute);
      }
    }

    // update screen
    fHasAnySlidingBarMoved = TRUE;

  } else if (iReason & MSYS_CALLBACK_REASON_RBUTTON_UP) {
  } else if (iReason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    // get mouse positions
    sX = pRegion->MouseXPos;

    // get attribute
    iAttribute = MSYS_GetRegionUserData(pRegion, 0);
    uiBarToReRender = iAttribute;

    // get value of attribute
    iCurrentAttributeValue = GetCurrentAttributeValue(iAttribute);

    // get the boxes bounding x
    sNewX = ((iCurrentAttributeValue - 35) * BASE_SKILL_PIXEL_UNIT_SIZE) / 50 +
            SKILL_SLIDE_START_X + LAPTOP_SCREEN_UL_X;

    // the sNewX is below 0, reset to zero
    if (sNewX < 0) {
      sNewX = 0;
    }

    if ((sX > sNewX) && (sX < sNewX + SLIDER_BAR_WIDTH)) {
      // we are within the slide bar, set fact we want to drag and draw
      fSlideIsActive = TRUE;
    } else {
      // otherwise want to jump to position
      fSlideIsActive = FALSE;
    }
  }
}

void SliderBarRegionButtonCallback(struct MOUSE_REGION *pRegion, int32_t iReason) {
  if (iReason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    fSlideIsActive = TRUE;
    SliderRegionButtonCallback(&pSliderRegions[MSYS_GetRegionUserData(pRegion, 0)],
                               MSYS_CALLBACK_REASON_LBUTTON_REPEAT);
  }
  if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    fSlideIsActive = FALSE;
  }
}

int32_t GetCurrentAttributeValue(int32_t iAttribute) {
  // this function will get the value of the attribute that was passed to this fucntion via
  // iAttribute
  int32_t iValue = 0;

  switch (iAttribute) {
    case (STRENGTH_ATTRIBUTE):
      iValue = iCurrentStrength;
      break;
    case (DEXTERITY_ATTRIBUTE):
      iValue = iCurrentDexterity;
      break;
    case (AGILITY_ATTRIBUTE):
      iValue = iCurrentAgility;
      break;
    case (HEALTH_ATTRIBUTE):
      iValue = iCurrentHealth;
      break;
    case (WISDOM_ATTRIBUTE):
      iValue = iCurrentWisdom;
      break;
    case (LEADERSHIP_ATTRIBUTE):
      iValue = iCurrentLeaderShip;
      break;
    case (MARKSMANSHIP_SKILL):
      iValue = iCurrentMarkmanship;
      break;
    case (MEDICAL_SKILL):
      iValue = iCurrentMedical;
      break;
    case (MECHANICAL_SKILL):
      iValue = iCurrentMechanical;
      break;
    case (EXPLOSIVE_SKILL):
      iValue = iCurrentExplosives;
      break;
  }

  return iValue;
}

void SetAttributes(void) {
  /*
    // set attributes and skills based on what is in charprofile.c

          // attributes
            iCurrentStrength = iStrength + iAddStrength;
                  iCurrentDexterity = iDexterity + iAddDexterity;
                  iCurrentHealth = iHealth + iAddHealth;
                  iCurrentLeaderShip = iLeadership + iAddLeadership;
                  iCurrentWisdom = iWisdom + iAddWisdom;
                  iCurrentAgility = iAgility + iAddAgility;

          // skills
      iCurrentMarkmanship = iMarksmanship + iAddMarksmanship;
                  iCurrentMechanical = iMechanical + iAddMechanical;
                  iCurrentMedical = iMedical + iAddMedical;
                  iCurrentExplosives = iExplosives + iAddExplosives;

                  // reset bonus pts
      iCurrentBonusPoints = 40;
  */

  iCurrentStrength = 55;
  iCurrentDexterity = 55;
  iCurrentHealth = 55;
  iCurrentLeaderShip = 55;
  iCurrentWisdom = 55;
  iCurrentAgility = 55;

  // skills
  iCurrentMarkmanship = 55;
  iCurrentMechanical = 55;
  iCurrentMedical = 55;
  iCurrentExplosives = 55;

  // reset bonus pts
  iCurrentBonusPoints = 40;

  ResetIncrementCharacterAttributes();

  return;
}

void DrawBonusPointsRemaining(void) {
  // draws the amount of points remaining player has
  CHAR16 sString[64];

  // just reviewing, don't blit stats
  if (fReviewStats == TRUE) {
    return;
  }
  // parse amountof bns pts remaining
  swprintf(sString, ARR_SIZE(sString), L"%d", iCurrentBonusPoints);

  // set font color
  SetFontForeground(FONT_WHITE);
  SetFontBackground(FONT_BLACK);
  SetFont(FONT12ARIAL);
  // print string
  mprintf(LAPTOP_SCREEN_UL_X + 425, LAPTOP_SCREEN_WEB_UL_Y + 51, sString);

  InvalidateRegion(LAPTOP_SCREEN_UL_X + 425, LAPTOP_SCREEN_WEB_UL_Y + 51, LAPTOP_SCREEN_UL_X + 475,
                   LAPTOP_SCREEN_WEB_UL_Y + 71);
  return;
}

void SetGeneratedCharacterAttributes(void) {
  // copies over the attributes of the player generated character
  iStrength = iCurrentStrength;
  iDexterity = iCurrentDexterity;
  iHealth = iCurrentHealth;
  iLeadership = iCurrentLeaderShip;
  iWisdom = iCurrentWisdom;
  iAgility = iCurrentAgility;

  // skills
  iMarksmanship = iCurrentMarkmanship;
  iMechanical = iCurrentMechanical;
  iMedical = iCurrentMedical;
  iExplosives = iCurrentExplosives;

  return;
}

void StatAtZeroBoxCallBack(uint8_t bExitValue) {
  // yes, so start over, else stay here and do nothing for now
  if (bExitValue == MSG_BOX_RETURN_YES) {
    MarkButtonsDirty();
  } else if (bExitValue == MSG_BOX_RETURN_NO) {
    IncrementStat(iCurrentStatAtZero);
    fHasAnySlidingBarMoved = TRUE;
    MarkButtonsDirty();
  }

  return;
}
