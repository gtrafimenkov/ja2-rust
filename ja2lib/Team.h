// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#ifndef __TEAM_H
#define __TEAM_H

#include "Soldier.h"

// DEFINE TEAMS
typedef enum {
  OUR_TEAM = 0,
  ENEMY_TEAM = 1,
  CREATURE_TEAM = 2,
  MILITIA_TEAM = 3,
  CIV_TEAM = 4,
  LAST_TEAM = CIV_TEAM,
  PLAYER_PLAN = 5,
} TeamID;

uint8_t GetTeamSide(uint8_t teamID);
void SetTeamSide(uint8_t teamID, uint8_t side);

#define MAX_SOLDIER_LIST_SIZE 200

struct SoldierList {
  int num;  // number of soldiers in the array
  struct SOLDIERTYPE* soldiers[MAX_SOLDIER_LIST_SIZE];
};

void GetTeamSoldiers(TeamID teamID, struct SoldierList* list);
void GetTeamSoldiers_Active(TeamID teamID, struct SoldierList* list);

#endif
