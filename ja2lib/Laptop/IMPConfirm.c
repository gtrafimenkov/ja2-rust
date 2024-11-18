#include "Laptop/IMPConfirm.h"

#include "Laptop/CharProfile.h"
#include "Laptop/Email.h"
#include "Laptop/Finances.h"
#include "Laptop/History.h"
#include "Laptop/IMPCompileCharacter.h"
#include "Laptop/IMPHomePage.h"
#include "Laptop/IMPMainPage.h"
#include "Laptop/IMPPortraits.h"
#include "Laptop/IMPTextSystem.h"
#include "Laptop/IMPVideoObjects.h"
#include "Laptop/IMPVoices.h"
#include "Laptop/Laptop.h"
#include "Laptop/LaptopSave.h"
#include "SGP/ButtonSystem.h"
#include "SGP/Random.h"
#include "SGP/WCheck.h"
#include "Strategic/GameClock.h"
#include "Strategic/GameEventHook.h"
#include "Strategic/Strategic.h"
#include "Tactical/Overhead.h"
#include "Tactical/SoldierControl.h"
#include "Tactical/SoldierProfile.h"
#include "Tactical/SoldierProfileType.h"
#include "Utils/Cursors.h"
#include "Utils/EncryptedFile.h"
#include "Utils/Utilities.h"
#include "Utils/WordWrap.h"
#include "rust_fileman.h"
#include "rust_laptop.h"

#define IMP_MERC_FILE "IMP.dat"

uint32_t giIMPConfirmButton[2];
uint32_t giIMPConfirmButtonImage[2];
BOOLEAN fNoAlreadySelected = FALSE;
uint16_t uiEyeXPositions[] = {
    8, 9,  8, 6, 13, 11, 8, 8,
    4,  // 208
    5,  // 209
    7,
    5,  // 211
    7, 11,
    8,  // 214
    5,
};

uint16_t uiEyeYPositions[] = {
    5, 4, 5, 6, 5, 5, 4, 4,
    4,  // 208
    5,
    5,  // 210
    7,
    6,  // 212
    5,
    5,  // 214
    6,
};

uint16_t uiMouthXPositions[] = {
    8, 9, 7, 7, 11, 10, 8, 8,
    5,  // 208
    6,
    7,  // 210
    6,
    7,  // 212
    9,
    7,  // 214
    5,
};

uint16_t uiMouthYPositions[] = {
    21, 23, 24, 25, 23, 24, 24, 24,
    25,  // 208
    24,
    24,  // 210
    26,
    24,  // 212
    23,
    24,  // 214
    26,
};

BOOLEAN fLoadingCharacterForPreviousImpProfile = FALSE;

void CreateConfirmButtons(void);
void DestroyConfirmButtons(void);
void GiveItemsToPC(uint8_t ubProfileId);
void MakeProfileInvItemAnySlot(MERCPROFILESTRUCT *pProfile, uint16_t usItem, uint8_t ubStatus,
                               uint8_t ubHowMany);
void MakeProfileInvItemThisSlot(MERCPROFILESTRUCT *pProfile, uint32_t uiPos, uint16_t usItem,
                                uint8_t ubStatus, uint8_t ubHowMany);
int32_t FirstFreeBigEnoughPocket(MERCPROFILESTRUCT *pProfile, uint16_t usItem);

// callbacks
void BtnIMPConfirmNo(GUI_BUTTON *btn, int32_t reason);
void BtnIMPConfirmYes(GUI_BUTTON *btn, int32_t reason);

void EnterIMPConfirm(void) {
  // create buttons
  CreateConfirmButtons();
  return;
}

void RenderIMPConfirm(void) {
  // the background
  RenderProfileBackGround();

  // indent
  RenderAvgMercIndentFrame(90, 40);

  // highlight answer
  PrintImpText();

  return;
}

void ExitIMPConfirm(void) {
  // destroy buttons
  DestroyConfirmButtons();
  return;
}

void HandleIMPConfirm(void) { return; }

void CreateConfirmButtons(void) {
  // create buttons for confirm screen

  giIMPConfirmButtonImage[0] = LoadButtonImage("LAPTOP\\button_2.sti", -1, 0, -1, 1, -1);
  giIMPConfirmButton[0] = CreateIconAndTextButton(
      giIMPConfirmButtonImage[0], pImpButtonText[16], FONT12ARIAL, FONT_WHITE, DEFAULT_SHADOW,
      FONT_WHITE, DEFAULT_SHADOW, TEXT_CJUSTIFIED, LAPTOP_SCREEN_UL_X + (136),
      LAPTOP_SCREEN_WEB_UL_Y + (254), BUTTON_TOGGLE, MSYS_PRIORITY_HIGH,
      BtnGenericMouseMoveButtonCallback, (GUI_CALLBACK)BtnIMPConfirmYes);

  giIMPConfirmButtonImage[1] = LoadButtonImage("LAPTOP\\button_2.sti", -1, 0, -1, 1, -1);
  giIMPConfirmButton[1] = CreateIconAndTextButton(
      giIMPConfirmButtonImage[1], pImpButtonText[17], FONT12ARIAL, FONT_WHITE, DEFAULT_SHADOW,
      FONT_WHITE, DEFAULT_SHADOW, TEXT_CJUSTIFIED, LAPTOP_SCREEN_UL_X + (136),
      LAPTOP_SCREEN_WEB_UL_Y + (314), BUTTON_TOGGLE, MSYS_PRIORITY_HIGH,
      BtnGenericMouseMoveButtonCallback, (GUI_CALLBACK)BtnIMPConfirmNo);

  SetButtonCursor(giIMPConfirmButton[0], CURSOR_WWW);
  SetButtonCursor(giIMPConfirmButton[1], CURSOR_WWW);

  return;
}

void DestroyConfirmButtons(void) {
  // destroy buttons for confirm screen

  RemoveButton(giIMPConfirmButton[0]);
  UnloadButtonImage(giIMPConfirmButtonImage[0]);

  RemoveButton(giIMPConfirmButton[1]);
  UnloadButtonImage(giIMPConfirmButtonImage[1]);
  return;
}

BOOLEAN AddCharacterToPlayersTeam(void) {
  MERC_HIRE_STRUCT HireMercStruct;

  // last minute chage to make sure merc with right facehas not only the right body but body
  // specific skills... ie..small mercs have martial arts..but big guys and women don't don't

  HandleMercStatsForChangesInFace();

  memset(&HireMercStruct, 0, sizeof(MERC_HIRE_STRUCT));

  HireMercStruct.ubProfileID = (uint8_t)(PLAYER_GENERATED_CHARACTER_ID + LaptopSaveInfo.iVoiceId);

  if (fLoadingCharacterForPreviousImpProfile == FALSE) {
    // give them items
    GiveItemsToPC(HireMercStruct.ubProfileID);
  }

  HireMercStruct.sSectorX = gsMercArriveSectorX;
  HireMercStruct.sSectorY = gsMercArriveSectorY;
  HireMercStruct.fUseLandingZoneForArrival = TRUE;

  HireMercStruct.fCopyProfileItemsOver = TRUE;

  // indefinite contract length
  HireMercStruct.iTotalContractLength = -1;

  HireMercStruct.ubInsertionCode = INSERTION_CODE_ARRIVING_GAME;
  HireMercStruct.uiTimeTillMercArrives = GetMercArrivalTimeOfDay();

  SetProfileFaceData(HireMercStruct.ubProfileID, (uint8_t)(200 + iPortraitNumber),
                     uiEyeXPositions[iPortraitNumber], uiEyeYPositions[iPortraitNumber],
                     uiMouthXPositions[iPortraitNumber], uiMouthYPositions[iPortraitNumber]);

  // if we succesfully hired the merc
  if (!HireMerc(&HireMercStruct)) {
    return (FALSE);
  } else {
    return (TRUE);
  }
}

void BtnIMPConfirmYes(GUI_BUTTON *btn, int32_t reason) {
  // btn callback for IMP Homepage About US button
  if (!(btn->uiFlags & BUTTON_ENABLED)) return;

  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    btn->uiFlags |= (BUTTON_CLICKED_ON);
  } else if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    if (btn->uiFlags & BUTTON_CLICKED_ON) {
      // reset button
      btn->uiFlags &= ~(BUTTON_CLICKED_ON);

      if (LaptopSaveInfo.fIMPCompletedFlag) {
        // already here, leave
        return;
      }

      if (LaptopMoneyGetBalance() < COST_OF_PROFILE) {
        // not enough
        return;
      }

      // line moved by CJC Nov 28 2002 to AFTER the check for money
      LaptopSaveInfo.fIMPCompletedFlag = TRUE;

      // charge the player
      AddTransactionToPlayersBook(IMP_PROFILE,
                                  (uint8_t)(PLAYER_GENERATED_CHARACTER_ID + LaptopSaveInfo.iVoiceId),
                                  -(COST_OF_PROFILE));
      AddHistoryToPlayersLog(HISTORY_CHARACTER_GENERATED, 0, GetGameTimeInMin(), -1, -1);
      AddCharacterToPlayersTeam();

      // write the created imp merc
      WriteOutCurrentImpCharacter((uint8_t)(PLAYER_GENERATED_CHARACTER_ID + LaptopSaveInfo.iVoiceId));

      fButtonPendingFlag = TRUE;
      iCurrentImpPage = IMP_HOME_PAGE;

      // send email notice
      AddFutureDayStrategicEvent(EVENT_DAY2_ADD_EMAIL_FROM_IMP, 60 * 7, 0, 2);

      ResetCharacterStats();

      // Display a popup msg box telling the user when and where the merc will arrive
      // DisplayPopUpBoxExplainingMercArrivalLocationAndTime( PLAYER_GENERATED_CHARACTER_ID +
      // LaptopSaveInfo.iVoiceId );

      // reset the id of the last merc so we dont get the
      // DisplayPopUpBoxExplainingMercArrivalLocationAndTime() pop up box in another screen by
      // accident
      LaptopSaveInfo.sLastHiredMerc.iIdOfMerc = -1;
    }
  }
}

// fixed? by CJC Nov 28 2002
void BtnIMPConfirmNo(GUI_BUTTON *btn, int32_t reason) {
  // btn callback for IMP Homepage About US button
  if (!(btn->uiFlags & BUTTON_ENABLED)) return;

  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    btn->uiFlags |= (BUTTON_CLICKED_ON);
  } else if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    if (btn->uiFlags & BUTTON_CLICKED_ON) {
      iCurrentImpPage = IMP_FINISH;

      /*

      LaptopSaveInfo.fIMPCompletedFlag = FALSE;
      ResetCharacterStats();

      fButtonPendingFlag = TRUE;
      iCurrentImpPage = IMP_HOME_PAGE;
      */
      /*
      if( fNoAlreadySelected == TRUE )
      {
              // already selected no
              fButtonPendingFlag = TRUE;
              iCurrentImpPage = IMP_HOME_PAGE;
      }
fNoAlreadySelected = TRUE;
      */
      btn->uiFlags &= ~(BUTTON_CLICKED_ON);
    }
  }
}

/*
void BtnIMPConfirmNo( GUI_BUTTON *btn,int32_t reason )
{


        // btn callback for IMP Homepage About US button
        if (!(btn->uiFlags & BUTTON_ENABLED))
                return;

        if(reason & MSYS_CALLBACK_REASON_LBUTTON_DWN )
        {
                 btn->uiFlags|=(BUTTON_CLICKED_ON);
        }
        else if(reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
        {
                if( btn->uiFlags & BUTTON_CLICKED_ON )
                {
                        LaptopSaveInfo.fIMPCompletedFlag = TRUE;
                        if( fNoAlreadySelected == TRUE )
                        {
                                // already selected no
                                fButtonPendingFlag = TRUE;
                                iCurrentImpPage = IMP_HOME_PAGE;
                        }
      fNoAlreadySelected = TRUE;
      btn->uiFlags&=~(BUTTON_CLICKED_ON);
                }
        }
}
*/

#define PROFILE_HAS_SKILL_TRAIT(p, t) ((p->bSkillTrait == t) || (p->bSkillTrait2 == t))

void GiveItemsToPC(uint8_t ubProfileId) {
  MERCPROFILESTRUCT *pProfile;

  // gives starting items to merc
  // NOTE: Any guns should probably be from those available in regular gun set

  pProfile = &(gMercProfiles[ubProfileId]);

  // STANDARD EQUIPMENT

  // kevlar vest, leggings, & helmet
  MakeProfileInvItemThisSlot(pProfile, VESTPOS, FLAK_JACKET, 100, 1);
  if (PreRandom(100) < (uint32_t)pProfile->bWisdom) {
    MakeProfileInvItemThisSlot(pProfile, HELMETPOS, STEEL_HELMET, 100, 1);
  }

  // canteen
  MakeProfileInvItemThisSlot(pProfile, SMALLPOCK4POS, CANTEEN, 100, 1);

  if (pProfile->bMarksmanship >= 80) {
    // good shooters get a better & matching ammo
    MakeProfileInvItemThisSlot(pProfile, HANDPOS, MP5K, 100, 1);
    MakeProfileInvItemThisSlot(pProfile, SMALLPOCK1POS, CLIP9_30, 100, 2);
  } else {
    // Automatic pistol, with matching ammo
    MakeProfileInvItemThisSlot(pProfile, HANDPOS, BERETTA_93R, 100, 1);
    MakeProfileInvItemThisSlot(pProfile, SMALLPOCK1POS, CLIP9_15, 100, 3);
  }

  // OPTIONAL EQUIPMENT: depends on skills & special skills

  if (pProfile->bMedical >= 60) {
    // strong medics get full medic kit
    MakeProfileInvItemAnySlot(pProfile, MEDICKIT, 100, 1);
  } else if (pProfile->bMedical >= 30) {
    // passable medics get first aid kit
    MakeProfileInvItemAnySlot(pProfile, FIRSTAIDKIT, 100, 1);
  }

  if (pProfile->bMechanical >= 50) {
    // mechanics get toolkit
    MakeProfileInvItemAnySlot(pProfile, TOOLKIT, 100, 1);
  }

  if (pProfile->bExplosive >= 50) {
    // loonies get TNT & Detonator
    MakeProfileInvItemAnySlot(pProfile, TNT, 100, 1);
    MakeProfileInvItemAnySlot(pProfile, DETONATOR, 100, 1);
  }

  // check for special skills
  if (PROFILE_HAS_SKILL_TRAIT(pProfile, LOCKPICKING) && (iMechanical)) {
    MakeProfileInvItemAnySlot(pProfile, LOCKSMITHKIT, 100, 1);
  }

  if (PROFILE_HAS_SKILL_TRAIT(pProfile, HANDTOHAND)) {
    MakeProfileInvItemAnySlot(pProfile, BRASS_KNUCKLES, 100, 1);
  }

  if (PROFILE_HAS_SKILL_TRAIT(pProfile, ELECTRONICS) && (iMechanical)) {
    MakeProfileInvItemAnySlot(pProfile, METALDETECTOR, 100, 1);
  }

  if (PROFILE_HAS_SKILL_TRAIT(pProfile, NIGHTOPS)) {
    MakeProfileInvItemAnySlot(pProfile, BREAK_LIGHT, 100, 2);
  }

  if (PROFILE_HAS_SKILL_TRAIT(pProfile, THROWING)) {
    MakeProfileInvItemAnySlot(pProfile, THROWING_KNIFE, 100, 1);
  }

  if (PROFILE_HAS_SKILL_TRAIT(pProfile, STEALTHY)) {
    MakeProfileInvItemAnySlot(pProfile, SILENCER, 100, 1);
  }

  if (PROFILE_HAS_SKILL_TRAIT(pProfile, KNIFING)) {
    MakeProfileInvItemAnySlot(pProfile, COMBAT_KNIFE, 100, 1);
  }

  if (PROFILE_HAS_SKILL_TRAIT(pProfile, CAMOUFLAGED)) {
    MakeProfileInvItemAnySlot(pProfile, CAMOUFLAGEKIT, 100, 1);
  }
}

void MakeProfileInvItemAnySlot(MERCPROFILESTRUCT *pProfile, uint16_t usItem, uint8_t ubStatus,
                               uint8_t ubHowMany) {
  int32_t iSlot;

  iSlot = FirstFreeBigEnoughPocket(pProfile, usItem);

  if (iSlot == -1) {
    // no room, item not received
    return;
  }

  // put the item into that slot
  MakeProfileInvItemThisSlot(pProfile, iSlot, usItem, ubStatus, ubHowMany);
}

void MakeProfileInvItemThisSlot(MERCPROFILESTRUCT *pProfile, uint32_t uiPos, uint16_t usItem,
                                uint8_t ubStatus, uint8_t ubHowMany) {
  pProfile->inv[uiPos] = usItem;
  pProfile->bInvStatus[uiPos] = ubStatus;
  pProfile->bInvNumber[uiPos] = ubHowMany;
}

int32_t FirstFreeBigEnoughPocket(MERCPROFILESTRUCT *pProfile, uint16_t usItem) {
  uint32_t uiPos;

  // if it fits into a small pocket
  if (Item[usItem].ubPerPocket != 0) {
    // check small pockets first
    for (uiPos = SMALLPOCK1POS; uiPos <= SMALLPOCK8POS; uiPos++) {
      if (pProfile->inv[uiPos] == NONE) {
        return (uiPos);
      }
    }
  }

  // check large pockets
  for (uiPos = BIGPOCK1POS; uiPos <= BIGPOCK4POS; uiPos++) {
    if (pProfile->inv[uiPos] == NONE) {
      return (uiPos);
    }
  }

  return (-1);
}

void WriteOutCurrentImpCharacter(int32_t iProfileId) {
  // grab the profile number and write out what is contained there in
  FileID hFile = FILE_ID_ERR;
  uint32_t uiBytesWritten = 0;

  // open the file for writing
  hFile = File_OpenForWriting(IMP_MERC_FILE);

  // write out the profile id
  if (!File_Write(hFile, &iProfileId, sizeof(int32_t), &uiBytesWritten)) {
    return;
  }

  // write out the portrait id
  if (!File_Write(hFile, &iPortraitNumber, sizeof(int32_t), &uiBytesWritten)) {
    return;
  }

  // write out the profile itself
  if (!File_Write(hFile, &gMercProfiles[iProfileId], sizeof(MERCPROFILESTRUCT), &uiBytesWritten)) {
    return;
  }

  // close file
  File_Close(hFile);

  return;
}

void LoadInCurrentImpCharacter(void) {
  int32_t iProfileId = 0;
  FileID hFile = FILE_ID_ERR;
  uint32_t uiBytesRead = 0;

  // open the file for writing
  hFile = File_OpenForReading(IMP_MERC_FILE);

  // valid file?
  if (hFile == -1) {
    return;
  }

  // read in the profile
  if (!File_Read(hFile, &iProfileId, sizeof(int32_t), &uiBytesRead)) {
    return;
  }

  // read in the portrait
  if (!File_Read(hFile, &iPortraitNumber, sizeof(int32_t), &uiBytesRead)) {
    return;
  }

  // read in the profile
  if (!File_Read(hFile, &gMercProfiles[iProfileId], sizeof(MERCPROFILESTRUCT), &uiBytesRead)) {
    return;
  }

  // close file
  File_Close(hFile);

  if (LaptopMoneyGetBalance() < COST_OF_PROFILE) {
    // not enough
    return;
  }

  // charge the player
  // is the character male?
  fCharacterIsMale = (gMercProfiles[iProfileId].bSex == MALE);
  fLoadingCharacterForPreviousImpProfile = TRUE;
  AddTransactionToPlayersBook(IMP_PROFILE, 0, -(COST_OF_PROFILE));
  AddHistoryToPlayersLog(HISTORY_CHARACTER_GENERATED, 0, GetGameTimeInMin(), -1, -1);
  LaptopSaveInfo.iVoiceId = iProfileId - PLAYER_GENERATED_CHARACTER_ID;
  AddCharacterToPlayersTeam();
  AddFutureDayStrategicEvent(EVENT_DAY2_ADD_EMAIL_FROM_IMP, 60 * 7, 0, 2);
  LaptopSaveInfo.fIMPCompletedFlag = TRUE;
  fPausedReDrawScreenFlag = TRUE;
  fLoadingCharacterForPreviousImpProfile = FALSE;

  return;
}

void ResetIMPCharactersEyesAndMouthOffsets(uint8_t ubMercProfileID) {
  // ATE: Check boundary conditions!
  if (((gMercProfiles[ubMercProfileID].ubFaceIndex - 200) > 16) ||
      (ubMercProfileID >= PROF_HUMMER)) {
    return;
  }

  gMercProfiles[ubMercProfileID].usEyesX =
      uiEyeXPositions[gMercProfiles[ubMercProfileID].ubFaceIndex - 200];
  gMercProfiles[ubMercProfileID].usEyesY =
      uiEyeYPositions[gMercProfiles[ubMercProfileID].ubFaceIndex - 200];

  gMercProfiles[ubMercProfileID].usMouthX =
      uiMouthXPositions[gMercProfiles[ubMercProfileID].ubFaceIndex - 200];
  gMercProfiles[ubMercProfileID].usMouthY =
      uiMouthYPositions[gMercProfiles[ubMercProfileID].ubFaceIndex - 200];
}
