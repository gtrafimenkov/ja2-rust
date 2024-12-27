#include "Tactical/HandleUIPlan.h"

#include "Soldier.h"
#include "Tactical/AnimationControl.h"
#include "Tactical/HandleUI.h"
#include "Tactical/Interface.h"
#include "Tactical/Overhead.h"
#include "Tactical/PathAI.h"
#include "Tactical/Points.h"
#include "Tactical/SoldierCreate.h"
#include "Tactical/Weapons.h"
#include "TileEngine/IsometricUtils.h"
#include "TileEngine/RenderWorld.h"
#include "Utils/Message.h"

uint8_t gubNumUIPlannedMoves = 0;
struct SOLDIERTYPE *gpUIPlannedSoldier = NULL;
struct SOLDIERTYPE *gpUIStartPlannedSoldier = NULL;
BOOLEAN gfInUIPlanMode = FALSE;

void SelectPausedFireAnimation(struct SOLDIERTYPE *pSoldier);

BOOLEAN BeginUIPlan(struct SOLDIERTYPE *pSoldier) {
  gubNumUIPlannedMoves = 0;
  gpUIPlannedSoldier = pSoldier;
  gpUIStartPlannedSoldier = pSoldier;
  gfInUIPlanMode = TRUE;

  gfPlotNewMovement = TRUE;

  ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, L"Entering Planning Mode");

  return (TRUE);
}

BOOLEAN AddUIPlan(uint16_t sGridNo, uint8_t ubPlanID) {
  struct SOLDIERTYPE *pPlanSoldier;
  int16_t sXPos, sYPos;
  int16_t sAPCost = 0;
  int8_t bDirection;
  INT32 iLoop;
  SOLDIERCREATE_STRUCT MercCreateStruct;
  uint8_t ubNewIndex;

  // Depeding on stance and direction facing, add guy!

  // If we have a planned action here, ignore!

  // If not OK Dest, ignore!
  if (!NewOKDestination(gpUIPlannedSoldier, sGridNo, FALSE, (int8_t)gsInterfaceLevel)) {
    return (FALSE);
  }

  if (ubPlanID == UIPLAN_ACTION_MOVETO) {
    // Calculate cost to move here
    sAPCost = PlotPath(gpUIPlannedSoldier, sGridNo, COPYROUTE, NO_PLOT, TEMPORARY,
                       (uint16_t)gpUIPlannedSoldier->usUIMovementMode, NOT_STEALTH, FORWARD,
                       gpUIPlannedSoldier->bActionPoints);
    // Adjust for running if we are not already running
    if (gpUIPlannedSoldier->usUIMovementMode == RUNNING) {
      sAPCost += AP_START_RUN_COST;
    }

    if (EnoughPoints(gpUIPlannedSoldier, sAPCost, 0, FALSE)) {
      memset(&MercCreateStruct, 0, sizeof(MercCreateStruct));
      MercCreateStruct.bTeam = SOLDIER_CREATE_AUTO_TEAM;
      MercCreateStruct.ubProfile = NO_PROFILE;
      MercCreateStruct.fPlayerPlan = TRUE;
      MercCreateStruct.bBodyType = gpUIPlannedSoldier->ubBodyType;
      MercCreateStruct.sInsertionGridNo = sGridNo;

      // Get Grid Corrdinates of mouse
      if (TacticalCreateSoldier(&MercCreateStruct, &ubNewIndex)) {
        // Get pointer to soldier
        GetSoldier(&pPlanSoldier, (uint16_t)ubNewIndex);

        pPlanSoldier->sPlannedTargetX = -1;
        pPlanSoldier->sPlannedTargetY = -1;

        // Compare OPPLISTS!
        // Set ones we don't know about but do now back to old ( ie no new guys )
        for (iLoop = 0; iLoop < MAX_NUM_SOLDIERS; iLoop++) {
          if (gpUIPlannedSoldier->bOppList[iLoop] < 0) {
            pPlanSoldier->bOppList[iLoop] = gpUIPlannedSoldier->bOppList[iLoop];
          }
        }

        // Get XY from Gridno
        ConvertGridNoToCenterCellXY(sGridNo, &sXPos, &sYPos);

        EVENT_SetSoldierPosition(pPlanSoldier, sXPos, sYPos);
        EVENT_SetSoldierDestination(pPlanSoldier, sGridNo);
        pPlanSoldier->bVisible = 1;
        pPlanSoldier->usUIMovementMode = gpUIPlannedSoldier->usUIMovementMode;

        pPlanSoldier->bActionPoints = gpUIPlannedSoldier->bActionPoints - sAPCost;

        pPlanSoldier->ubPlannedUIAPCost = (uint8_t)pPlanSoldier->bActionPoints;

        // Get direction
        bDirection =
            (int8_t)gpUIPlannedSoldier->usPathingData[gpUIPlannedSoldier->usPathDataSize - 1];

        // Set direction
        pPlanSoldier->bDirection = bDirection;
        pPlanSoldier->bDesiredDirection = bDirection;

        // Set walking animation
        ChangeSoldierState(pPlanSoldier, pPlanSoldier->usUIMovementMode, 0, FALSE);

        // Change selected soldier
        gusSelectedSoldier = (uint16_t)pPlanSoldier->ubID;

        // Change global planned mode to this guy!
        gpUIPlannedSoldier = pPlanSoldier;

        gubNumUIPlannedMoves++;

        gfPlotNewMovement = TRUE;

        ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, L"Adding Merc Move to Plan");
      }
    } else {
      ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, L"Merc will not have enough action points");
    }
  } else if (ubPlanID == UIPLAN_ACTION_FIRE) {
    sAPCost = CalcTotalAPsToAttack(gpUIPlannedSoldier, sGridNo, TRUE,
                                   (int8_t)(gpUIPlannedSoldier->bShownAimTime / 2));

    // Get XY from Gridno
    ConvertGridNoToCenterCellXY(sGridNo, &sXPos, &sYPos);

    // If this is a player guy, show message about no APS
    if (EnoughPoints(gpUIPlannedSoldier, sAPCost, 0, FALSE)) {
      // CHECK IF WE ARE A PLANNED SOLDIER OR NOT< IF SO< CREATE!
      if (gpUIPlannedSoldier->ubID < MAX_NUM_SOLDIERS) {
        memset(&MercCreateStruct, 0, sizeof(MercCreateStruct));
        MercCreateStruct.bTeam = SOLDIER_CREATE_AUTO_TEAM;
        MercCreateStruct.ubProfile = NO_PROFILE;
        MercCreateStruct.fPlayerPlan = TRUE;
        MercCreateStruct.bBodyType = gpUIPlannedSoldier->ubBodyType;
        MercCreateStruct.sInsertionGridNo = sGridNo;

        // Get Grid Corrdinates of mouse
        if (TacticalCreateSoldier(&MercCreateStruct, &ubNewIndex)) {
          // Get pointer to soldier
          GetSoldier(&pPlanSoldier, (uint16_t)ubNewIndex);

          pPlanSoldier->sPlannedTargetX = -1;
          pPlanSoldier->sPlannedTargetY = -1;

          // Compare OPPLISTS!
          // Set ones we don't know about but do now back to old ( ie no new guys )
          for (iLoop = 0; iLoop < MAX_NUM_SOLDIERS; iLoop++) {
            if (gpUIPlannedSoldier->bOppList[iLoop] < 0) {
              pPlanSoldier->bOppList[iLoop] = gpUIPlannedSoldier->bOppList[iLoop];
            }
          }

          EVENT_SetSoldierPosition(pPlanSoldier, gpUIPlannedSoldier->dXPos,
                                   gpUIPlannedSoldier->dYPos);
          EVENT_SetSoldierDestination(pPlanSoldier, gpUIPlannedSoldier->sGridNo);
          pPlanSoldier->bVisible = 1;
          pPlanSoldier->usUIMovementMode = gpUIPlannedSoldier->usUIMovementMode;

          pPlanSoldier->bActionPoints = gpUIPlannedSoldier->bActionPoints - sAPCost;

          pPlanSoldier->ubPlannedUIAPCost = (uint8_t)pPlanSoldier->bActionPoints;

          // Get direction
          bDirection =
              (int8_t)gpUIPlannedSoldier->usPathingData[gpUIPlannedSoldier->usPathDataSize - 1];

          // Set direction
          pPlanSoldier->bDirection = bDirection;
          pPlanSoldier->bDesiredDirection = bDirection;

          // Set walking animation
          ChangeSoldierState(pPlanSoldier, pPlanSoldier->usUIMovementMode, 0, FALSE);

          // Change selected soldier
          gusSelectedSoldier = (uint16_t)pPlanSoldier->ubID;

          // Change global planned mode to this guy!
          gpUIPlannedSoldier = pPlanSoldier;

          gubNumUIPlannedMoves++;
        }
      }

      gpUIPlannedSoldier->bActionPoints = gpUIPlannedSoldier->bActionPoints - sAPCost;

      gpUIPlannedSoldier->ubPlannedUIAPCost = (uint8_t)gpUIPlannedSoldier->bActionPoints;

      // Get direction from gridno
      bDirection = (int8_t)GetDirectionFromGridNo(sGridNo, gpUIPlannedSoldier);

      // Set direction
      gpUIPlannedSoldier->bDirection = bDirection;
      gpUIPlannedSoldier->bDesiredDirection = bDirection;

      // Set to shooting animation
      SelectPausedFireAnimation(gpUIPlannedSoldier);

      gpUIPlannedSoldier->sPlannedTargetX = sXPos;
      gpUIPlannedSoldier->sPlannedTargetY = sYPos;

      ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, L"Adding Merc Shoot to Plan");

    } else {
      ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, L"Merc will not have enough action points");
    }
  }
  return (TRUE);
}

void EndUIPlan() {
  int cnt;
  struct SOLDIERTYPE *pSoldier;

  // Zero out any planned soldiers
  for (cnt = MAX_NUM_SOLDIERS; cnt < TOTAL_SOLDIERS; cnt++) {
    pSoldier = MercPtrs[cnt];

    if (IsSolActive(pSoldier)) {
      if (pSoldier->sPlannedTargetX != -1) {
        SetRenderFlags(RENDER_FLAG_FULL);
      }
      TacticalRemoveSoldier(pSoldier->ubID);
    }
  }
  gfInUIPlanMode = FALSE;
  gusSelectedSoldier = gpUIStartPlannedSoldier->ubID;

  gfPlotNewMovement = TRUE;

  ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, L"Leaving Planning Mode");
}

BOOLEAN InUIPlanMode() { return (gfInUIPlanMode); }

void SelectPausedFireAnimation(struct SOLDIERTYPE *pSoldier) {
  // Determine which animation to do...depending on stance and gun in hand...

  switch (gAnimControl[pSoldier->usAnimState].ubEndHeight) {
    case ANIM_STAND:

      if (pSoldier->bDoBurst > 0) {
        ChangeSoldierState(pSoldier, STANDING_BURST, 2, FALSE);
      } else {
        ChangeSoldierState(pSoldier, SHOOT_RIFLE_STAND, 2, FALSE);
      }
      break;

    case ANIM_PRONE:
      ChangeSoldierState(pSoldier, SHOOT_RIFLE_PRONE, 2, FALSE);
      break;

    case ANIM_CROUCH:
      ChangeSoldierState(pSoldier, SHOOT_RIFLE_CROUCH, 2, FALSE);
      break;
  }
}
