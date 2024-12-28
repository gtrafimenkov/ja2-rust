// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#ifndef __CAMPAIGN_H
#define __CAMPAIGN_H

#include "Tactical/SoldierProfile.h"

#define SALARYAMT 0
#define HEALTHAMT 1
#define AGILAMT 2
#define DEXTAMT 3
#define WISDOMAMT 4
#define MEDICALAMT 5
#define EXPLODEAMT 6
#define MECHANAMT 7
#define MARKAMT 8
#define EXPERAMT 9
#define STRAMT 10
#define LDRAMT 11
#define ASSIGNAMT 12
#define NAMEAMT 13

#define FIRST_CHANGEABLE_STAT HEALTHAMT
#define LAST_CHANGEABLE_STAT LDRAMT
#define CHANGEABLE_STAT_COUNT (LDRAMT - HEALTHAMT + 1)

#define MAX_STAT_VALUE 100  // for stats and skills
#define MAXEXPLEVEL 10      // maximum merc experience level

#define SKILLS_SUBPOINTS_TO_IMPROVE 25
#define ATTRIBS_SUBPOINTS_TO_IMPROVE 50
#define LEVEL_SUBPOINTS_TO_IMPROVE \
  350  // per current level!	(Can't go over 6500, 10x must fit in USHORT!)

#define WORKIMPROVERATE 2   // increase to make working  mercs improve more
#define TRAINIMPROVERATE 2  // increase to make training mercs improve more

#define SALARY_CHANGE_PER_LEVEL 1.25  // Mercs salary is multiplied by this
#define MAX_DAILY_SALARY 30000        // must fit into an int16_t (32k)
#define MAX_LARGE_SALARY 500000       // no limit, really

// training cap: you can't train any stat/skill beyond this value
#define TRAINING_RATING_CAP 85

// stat change causes
#define FROM_SUCCESS 0
#define FROM_TRAINING 1
#define FROM_FAILURE 2

// types of experience bonus awards
enum {
  EXP_BONUS_MINIMUM,
  EXP_BONUS_SMALL,
  EXP_BONUS_AVERAGE,
  EXP_BONUS_LARGE,
  EXP_BONUS_MAXIMUM,
  NUM_EXP_BONUS_TYPES,
};

void StatChange(struct SOLDIERTYPE *pSoldier, uint8_t ubStat, uint16_t usNumChances,
                uint8_t ubReason);
void ProfileStatChange(MERCPROFILESTRUCT *pProfile, uint8_t ubStat, uint16_t usNumChances,
                       uint8_t ubReason);
void ProcessStatChange(MERCPROFILESTRUCT *pProfile, uint8_t ubStat, uint16_t usNumChances,
                       uint8_t ubReason);

void UpdateStats(struct SOLDIERTYPE *pSoldier);
void ProfileUpdateStats(MERCPROFILESTRUCT *pProfile);
void ProcessUpdateStats(MERCPROFILESTRUCT *pProfile, struct SOLDIERTYPE *pSoldier);

uint32_t CalcNewSalary(uint32_t uiOldSalary, BOOLEAN fIncrease, uint32_t uiMaxLimit);
uint32_t RoundOffSalary(uint32_t uiSalary);
uint16_t SubpointsPerPoint(uint8_t ubStat, int8_t bExpLevel);

void HandleUnhiredMercImprovement(MERCPROFILESTRUCT *pProfile);
void HandleUnhiredMercDeaths(int32_t iProfileID);

uint8_t CurrentPlayerProgressPercentage(void);
uint8_t HighestPlayerProgressPercentage(void);

void HourlyProgressUpdate(void);

void HandleAnyStatChangesAfterAttack(void);

void AwardExperienceBonusToActiveSquad(uint8_t ubExpBonusType);

void BuildStatChangeString(wchar_t *wString, size_t bufSize, wchar_t *wName, BOOLEAN fIncrease,
                           int16_t sPtsChanged, uint8_t ubStat);

void MERCMercWentUpALevelSendEmail(uint8_t ubMercMercIdValue);

#endif
