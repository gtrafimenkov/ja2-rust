#include <stdarg.h>
#include <stdio.h>
#include <time.h>
#include <wchar.h>

#include "GameLoop.h"
#include "MessageBoxScreen.h"
#include "SGP/Container.h"
#include "SGP/English.h"
#include "SGP/Font.h"
#include "SGP/HImage.h"
#include "SGP/Types.h"
#include "SGP/VObject.h"
#include "SGP/VObjectBlitters.h"
#include "SGP/Video.h"
#include "SGP/WCheck.h"
#include "ScreenIDs.h"
#include "SysGlobals.h"
#include "Tactical/AnimationControl.h"
#include "Tactical/AnimationData.h"
#include "Tactical/Interface.h"
#include "Tactical/Overhead.h"
#include "Tactical/SoldierAni.h"
#include "Tactical/SoldierControl.h"
#include "TileEngine/IsometricUtils.h"
#include "TileEngine/RadarScreen.h"
#include "TileEngine/RenderDirty.h"
#include "TileEngine/RenderWorld.h"
#include "Utils/EventPump.h"
#include "Utils/FontControl.h"
#include "Utils/TimerControl.h"
#include "platform_strings.h"

void BuildListFile();

BOOLEAN gfAniEditMode = FALSE;
static uint16_t usStartAnim = 0;
static uint8_t ubStartHeight = 0;
static struct SOLDIERTYPE *pSoldier;

static BOOLEAN fOKFiles = FALSE;
static uint8_t ubNumStates = 0;
static uint16_t *pusStates = NULL;
static int8_t ubCurLoadedState = 0;

void CycleAnimations() {
  int32_t cnt;

  // FInd the next animation with start height the same...
  for (cnt = usStartAnim + 1; cnt < NUMANIMATIONSTATES; cnt++) {
    if (gAnimControl[cnt].ubHeight == ubStartHeight) {
      usStartAnim = (uint8_t)cnt;
      EVENT_InitNewSoldierAnim(pSoldier, usStartAnim, 0, TRUE);
      return;
    }
  }

  usStartAnim = 0;
  EVENT_InitNewSoldierAnim(pSoldier, usStartAnim, 0, TRUE);
}

uint32_t AniEditScreenInit(void) { return TRUE; }

// The ShutdownGame function will free up/undo all things that were started in InitializeGame()
// It will also be responsible to making sure that all Gaming Engine tasks exit properly

uint32_t AniEditScreenShutdown(void) { return TRUE; }

uint32_t AniEditScreenHandle(void) {
  InputAtom InputEvent;
  static BOOLEAN fFirstTime = TRUE;
  static uint16_t usOldState;
  static BOOLEAN fToggle = FALSE;
  static BOOLEAN fToggle2 = FALSE;

  //	EV_S_SETPOSITION SSetPosition;

  // Make backups
  if (fFirstTime) {
    gfAniEditMode = TRUE;

    usStartAnim = 0;
    ubStartHeight = ANIM_STAND;

    fFirstTime = FALSE;
    fToggle = FALSE;
    fToggle2 = FALSE;
    ubCurLoadedState = 0;

    pSoldier = MercPtrs[gusSelectedSoldier];

    gTacticalStatus.uiFlags |= LOADING_SAVED_GAME;

    EVENT_InitNewSoldierAnim(pSoldier, usStartAnim, 0, TRUE);

    BuildListFile();
  }

  /////////////////////////////////////////////////////
  StartFrameBufferRender();

  RenderWorld();

  ExecuteBaseDirtyRectQueue();

  /////////////////////////////////////////////////////
  EndFrameBufferRender();

  SetFont(LARGEFONT1);
  mprintf(0, 0, L"SOLDIER ANIMATION VIEWER");
  gprintfdirty(0, 0, L"SOLDIER ANIMATION VIEWER");

  mprintf(0, 20, L"Current Animation: %S %S", gAnimControl[usStartAnim].zAnimStr,
          gAnimSurfaceDatabase[pSoldier->usAnimSurface].Filename);
  gprintfdirty(0, 20, L"Current Animation: %S %S", gAnimControl[usStartAnim].zAnimStr,
               gAnimSurfaceDatabase[pSoldier->usAnimSurface].Filename);

  switch (ubStartHeight) {
    case ANIM_STAND:

      mprintf(0, 40, L"Current Stance: STAND");
      break;

    case ANIM_CROUCH:

      mprintf(0, 40, L"Current Stance: CROUCH");
      break;

    case ANIM_PRONE:

      mprintf(0, 40, L"Current Stance: PRONE");
      break;
  }
  gprintfdirty(0, 40, L"Current Animation: %S", gAnimControl[usStartAnim].zAnimStr);

  if (fToggle) {
    mprintf(0, 60, L"FORCE ON");
    gprintfdirty(0, 60, L"FORCE OFF");
  }

  if (fToggle2) {
    mprintf(0, 70, L"LOADED ORDER ON");
    gprintfdirty(0, 70, L"LOADED ORDER ON");

    mprintf(0, 90, L"LOADED ORDER : %S", gAnimControl[pusStates[ubCurLoadedState]].zAnimStr);
    gprintfdirty(0, 90, L"LOADED ORDER : %S", gAnimControl[pusStates[ubCurLoadedState]].zAnimStr);
  }

  if (DequeueEvent(&InputEvent) == TRUE) {
    if ((InputEvent.usEvent == KEY_DOWN) && (InputEvent.usParam == ESC)) {
      fFirstTime = TRUE;

      gfAniEditMode = FALSE;

      fFirstTimeInGameScreen = TRUE;

      gTacticalStatus.uiFlags &= (~LOADING_SAVED_GAME);

      if (fOKFiles) {
        MemFree(pusStates);
      }

      fOKFiles = FALSE;

      return (GAME_SCREEN);
    }

    if ((InputEvent.usEvent == KEY_UP) && (InputEvent.usParam == SPACE)) {
      if (!fToggle && !fToggle2) {
        CycleAnimations();
      }
    }

    if ((InputEvent.usEvent == KEY_UP) && (InputEvent.usParam == 's')) {
      if (!fToggle) {
        uint16_t usAnim = 0;
        usOldState = usStartAnim;

        switch (ubStartHeight) {
          case ANIM_STAND:

            usAnim = STANDING;
            break;

          case ANIM_CROUCH:

            usAnim = CROUCHING;
            break;

          case ANIM_PRONE:

            usAnim = PRONE;
            break;
        }

        EVENT_InitNewSoldierAnim(pSoldier, usAnim, 0, TRUE);
      } else {
        EVENT_InitNewSoldierAnim(pSoldier, usOldState, 0, TRUE);
      }

      fToggle = !fToggle;
    }

    if ((InputEvent.usEvent == KEY_UP) && (InputEvent.usParam == 'l')) {
      if (!fToggle2) {
        usOldState = usStartAnim;

        EVENT_InitNewSoldierAnim(pSoldier, pusStates[ubCurLoadedState], 0, TRUE);
      } else {
        EVENT_InitNewSoldierAnim(pSoldier, usOldState, 0, TRUE);
      }

      fToggle2 = !fToggle2;
    }

    if ((InputEvent.usEvent == KEY_UP) && (InputEvent.usParam == PGUP)) {
      if (fOKFiles && fToggle2) {
        ubCurLoadedState++;

        if (ubCurLoadedState == ubNumStates) {
          ubCurLoadedState = 0;
        }

        EVENT_InitNewSoldierAnim(pSoldier, pusStates[ubCurLoadedState], 0, TRUE);
      }
    }

    if ((InputEvent.usEvent == KEY_UP) && (InputEvent.usParam == PGDN)) {
      if (fOKFiles && fToggle2) {
        ubCurLoadedState--;

        if (ubCurLoadedState == 0) {
          ubCurLoadedState = ubNumStates;
        }

        EVENT_InitNewSoldierAnim(pSoldier, pusStates[ubCurLoadedState], 0, TRUE);
      }
    }

    if ((InputEvent.usEvent == KEY_UP) && (InputEvent.usParam == 'c')) {
      // CLEAR!
      usStartAnim = 0;
      EVENT_InitNewSoldierAnim(pSoldier, usStartAnim, 0, TRUE);
    }

    if ((InputEvent.usEvent == KEY_UP) && (InputEvent.usParam == ENTER)) {
      if (ubStartHeight == ANIM_STAND) {
        ubStartHeight = ANIM_CROUCH;
      } else if (ubStartHeight == ANIM_CROUCH) {
        ubStartHeight = ANIM_PRONE;
      } else {
        ubStartHeight = ANIM_STAND;
      }
    }
  }

  return (ANIEDIT_SCREEN);
}

uint16_t GetAnimStateFromName(char *zName) {
  int32_t cnt;

  // FInd the next animation with start height the same...
  for (cnt = 0; cnt < NUMANIMATIONSTATES; cnt++) {
    if (strcasecmp(gAnimControl[cnt].zAnimStr, zName) == 0) {
      return ((uint16_t)cnt);
    }
  }

  return (5555);
}

void BuildListFile() {
  FILE *infoFile;
  char currFilename[128];
  int numEntries = 0;
  int cnt;
  uint16_t usState;
  wchar_t zError[128];

  // Verify the existance of the header text file.
  infoFile = fopen("ANITEST.DAT", "rb");
  if (!infoFile) {
    return;
  }
  // count STIs inside header and verify each one's existance.
  while (!feof(infoFile)) {
    fgets(currFilename, 128, infoFile);
    // valid entry in header, continue on...

    numEntries++;
  }
  fseek(infoFile, 0, SEEK_SET);  // reset header file

  // Allocate array
  pusStates = (uint16_t *)MemAlloc(sizeof(uint16_t) * numEntries);

  fOKFiles = TRUE;

  cnt = 0;
  while (!feof(infoFile)) {
    fgets(currFilename, 128, infoFile);

    // Remove newline
    currFilename[strlen(currFilename) - 1] = '\0';
    currFilename[strlen(currFilename) - 1] = '\0';

    usState = GetAnimStateFromName(currFilename);

    if (usState != 5555) {
      cnt++;
      ubNumStates = (uint8_t)cnt;
      pusStates[cnt] = usState;
    } else {
      swprintf(zError, ARR_SIZE(zError), L"Animation str %S is not known: ", currFilename);
      DoMessageBox(MSG_BOX_BASIC_STYLE, zError, ANIEDIT_SCREEN, (uint8_t)MSG_BOX_FLAG_YESNO, NULL,
                   NULL);
      fclose(infoFile);
      return;
    }
  }
}
