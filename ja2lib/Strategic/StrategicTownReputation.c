#include "Strategic/StrategicTownReputation.h"

#include "SGP/Debug.h"

// #include "SGP/Debug.h"
#include "SGP/Types.h"
#include "Soldier.h"
#include "Strategic/GameClock.h"
#include "Strategic/GameEventHook.h"
#include "Strategic/StrategicTownLoyalty.h"
#include "Tactical/Overhead.h"
#include "Tactical/SoldierControl.h"
#include "Tactical/SoldierProfile.h"
#include "Tactical/SoldierProfileType.h"
#include "Utils/Message.h"

// init for town reputation at game start
#define INITIAL_TOWN_REPUTATION 0

// the max and min town opinion of an individual merc can be
#define MAX_TOWN_OPINION 50
#define MIN_TOWN_OPINION -50

// town reputation is currently updated 4x per day: at 9am, noon, 3pm, and 6pm

// the number of events per day
#define TOWN_OPINION_NUMBER_OF_PERIODS 4

// the start time after midnight that first town reputation event takes place .. in minutes
#define TOWN_OPINION_START_TIME (9 * 60)

// how often the town opinion events occur...right now every 3 hours
#define TOWN_OPINION_PERIOD (3 * 60)

void InitializeProfilesForTownReputation(void) {
  UINT32 uiProfileId = 0;

  // initialize the town opinion values in each recruitable merc's profile structure
  for (uiProfileId = 0; uiProfileId < FIRST_NPC; uiProfileId++) {
    memset(&(gMercProfiles[uiProfileId].bMercTownReputation), INITIAL_TOWN_REPUTATION,
           sizeof(gMercProfiles[uiProfileId].bMercTownReputation));
  }
}

void PostEventsForSpreadOfTownOpinion(void) {
  /* ARM - Do nothing, this system has been scrapped because it is so marginal and it's too late to
     bother with it now

          INT32 iCounter = 0;
          // called every day at 3am to set up daily events to handle spreading of town opinion
     about mercs for( iCounter = 0; iCounter < TOWN_OPINION_NUMBER_OF_PERIODS; iCounter++ )
          {
                  AddStrategicEvent( EVENT_HANDLE_TOWN_OPINION, GetGameTimeInDays()*24*60 +
     TOWN_OPINION_START_TIME + iCounter * TOWN_OPINION_PERIOD, 0 );
          }
  */
}

UINT8 GetTownOpinionOfMerc(UINT8 ubProfileId, UINT8 ubTownId) {
  Assert(ubProfileId < FIRST_NPC);
  Assert(ubTownId < NUM_TOWNS);

  // return amount
  return (gMercProfiles[ubProfileId].bMercTownReputation[ubTownId]);
}

UINT8 GetTownOpinionOfMercForSoldier(struct SOLDIERTYPE *pSoldier, UINT8 ubTownId) {
  // error check
  if (pSoldier == NULL) {
    return (0);
  }

  Assert(ubTownId < NUM_TOWNS);

  // pass on to
  return (GetTownOpinionOfMerc(GetSolProfile(pSoldier), ubTownId));
}

void UpdateTownOpinionOfThisMerc(UINT8 ubProfileId, UINT8 ubTownId, INT8 bAmount) {
  Assert(ubProfileId < FIRST_NPC);
  Assert(ubTownId < NUM_TOWNS);

  // check if opinion would be affected too greatly
  if (gMercProfiles[ubProfileId].bMercTownReputation[ubTownId] + bAmount > MAX_TOWN_OPINION) {
    // maximize
    gMercProfiles[ubProfileId].bMercTownReputation[ubTownId] = MAX_TOWN_OPINION;
  } else if (gMercProfiles[ubProfileId].bMercTownReputation[ubTownId] + bAmount <
             MIN_TOWN_OPINION) {
    // minimize
    gMercProfiles[ubProfileId].bMercTownReputation[ubTownId] = MIN_TOWN_OPINION;
  } else {
    // update amount
    gMercProfiles[ubProfileId].bMercTownReputation[ubTownId] += bAmount;
  }
}

void UpdateTownOpinionOfThisMercForSoldier(struct SOLDIERTYPE *pSoldier, UINT8 ubTownId,
                                           INT8 bAmount) {
  // error check
  if (pSoldier == NULL) {
    return;
  }

  Assert(ubTownId < NUM_TOWNS);

  // pass this on to the profile based function
  UpdateTownOpinionOfThisMerc(GetSolProfile(pSoldier), ubTownId, bAmount);
}

void HandleSpreadOfAllTownsOpinion(void) {
  UINT8 ubProfileId;

  // debug message
  ScreenMsg(MSG_FONT_RED, MSG_DEBUG, L"%s - Spreading town opinions about mercs", gswzWorldTimeStr);

  // run though all player-recruitable profiles and update towns opinion of mercs
  for (ubProfileId = 0; ubProfileId < FIRST_NPC; ubProfileId++) {
    HandleSpreadOfTownOpinionForMerc(ubProfileId);
  }
}

void HandleSpreadOfTownOpinionForMerc(UINT8 ubProfileId) {
  // handle opinion spread for this grunt
  INT32 iDistanceBetweenTowns;
  INT8 iCounterA, iCounterB;
  INT8 bOpinionOfTownA, bOpinionOfTownB;

  Assert(ubProfileId < FIRST_NPC);

  for (iCounterA = FIRST_TOWN; iCounterA < NUM_TOWNS; iCounterA++) {
    for (iCounterB = iCounterA; iCounterB < NUM_TOWNS; iCounterB++) {
      if (iCounterA != iCounterB) {
        bOpinionOfTownA = GetTownOpinionOfMerc(ubProfileId, iCounterA);
        bOpinionOfTownB = GetTownOpinionOfMerc(ubProfileId, iCounterB);

        iDistanceBetweenTowns = GetTownDistances(iCounterA, iCounterB);

        // spread opinion between towns
        HandleOpinionOfTownsAboutSoldier(bOpinionOfTownA, bOpinionOfTownB, iDistanceBetweenTowns,
                                         ubProfileId);
      }
    }
  }
}

void HandleOpinionOfTownsAboutSoldier(INT8 bTownA, INT8 bTownB, INT32 iDistanceBetweenThem,
                                      UINT8 ubProfile) {
  // ARM: System has been scrapped
}

/*
void HandleSpreadOfTownOpinionForMercForSoldier( struct SOLDIERTYPE *pSoldier )
{
        // error check
        if( pSoldier == NULL )
        {
                return;
        }

        // let the profile based one do the handling
        HandleSpreadOfTownOpinionForMerc( GetSolProfile(pSoldier) );
}


void HandleSpreadOfTownsOpinionForCurrentMercs( void )
{
        INT32 iCounter = 0, iNumberOnPlayersTeam = 0;

        // get the number on players team
        iNumberOnPlayersTeam = gTacticalStatus.Team[ OUR_TEAM ].bLastID;

        // run through all mercs on players current team
        for(iCounter = 0; iCounter < iNumberOnPlayersTeam; iCounter++ )
        {
                HandleSpreadOfTownOpinionForMercForSoldier( MercPtrs[ iCounter ] );
        }

        return;
}
*/
