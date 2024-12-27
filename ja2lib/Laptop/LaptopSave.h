#ifndef _LAPTOP_SAVE_H_
#define _LAPTOP_SAVE_H_

#include "Laptop/StoreInventory.h"
#include "SGP/Types.h"
#include "Tactical/ItemTypes.h"

#define MAX_BOOKMARKS 20

#define MAX_PURCHASE_AMOUNT 10

#define SPECK_QUOTE__ALREADY_TOLD_PLAYER_THAT_LARRY_RELAPSED 0x00000001
#define SPECK_QUOTE__SENT_EMAIL_ABOUT_LACK_OF_PAYMENT 0x00000002

typedef struct {
  BOOLEAN fActive;
  UINT8 ubSoldierID;
  UINT8 ubMercID;
  INT32 iPayOutPrice;
} LIFE_INSURANCE_PAYOUT;

typedef struct {
  BOOLEAN fHaveDisplayedPopUpInLaptop;  // Is set when the popup gets displayed, reset when entering
                                        // laptop again.
  INT32 iIdOfMerc;
  uint32_t uiArrivalTime;
} LAST_HIRED_MERC_STRUCT;

typedef struct {
  UINT16 usItemIndex;
  UINT8 ubNumberPurchased;
  INT8 bItemQuality;
  UINT16 usBobbyItemIndex;  // Item number in the BobbyRayInventory structure
  BOOLEAN fUsed;  // Indicates wether or not the item is from the used inventory or the regular
                  // inventory
} BobbyRayPurchaseStruct;

typedef struct {
  BOOLEAN fActive;
  BobbyRayPurchaseStruct BobbyRayPurchase[MAX_PURCHASE_AMOUNT];
  UINT8 ubNumberPurchases;
} BobbyRayOrderStruct;

// used when the player goes to bobby rays when it is still down
enum {
  BOBBYR_NEVER_BEEN_TO_SITE,
  BOBBYR_BEEN_TO_SITE_ONCE,
  BOBBYR_ALREADY_SENT_EMAIL,
};

typedef struct {
  // General Laptop Info
  BOOLEAN gfNewGameLaptop;              // Is it the firs time in Laptop
  BOOLEAN fVisitedBookmarkAlready[20];  // have we visitied this site already?
  INT32 iBookMarkList[MAX_BOOKMARKS];

  INT32 iCurrentBalance;  // current players balance

  // IMP Information
  BOOLEAN fIMPCompletedFlag;       // Has the player Completed the IMP process
  BOOLEAN fSentImpWarningAlready;  // Has the Imp email warning already been sent

  // Personnel Info
  INT16 ubDeadCharactersList[256];
  INT16 ubLeftCharactersList[256];
  INT16 ubOtherCharactersList[256];

  // MERC site info
  UINT8 gubPlayersMercAccountStatus;
  uint32_t guiPlayersMercAccountNumber;
  UINT8 gubLastMercIndex;

  // Aim Site

  // BobbyRay Site
  STORE_INVENTORY BobbyRayInventory[MAXITEMS];
  STORE_INVENTORY BobbyRayUsedInventory[MAXITEMS];

  BobbyRayOrderStruct *BobbyRayOrdersOnDeliveryArray;
  UINT8 usNumberOfBobbyRayOrderItems;  // The number of elements in the array
  UINT8 usNumberOfBobbyRayOrderUsed;   // The number of items in the array that are used

  // Flower Site
  // NONE

  // Insurance Site
  LIFE_INSURANCE_PAYOUT *pLifeInsurancePayouts;
  UINT8 ubNumberLifeInsurancePayouts;     // The number of elements in the array
  UINT8 ubNumberLifeInsurancePayoutUsed;  // The number of items in the array that are used

  BOOLEAN fBobbyRSiteCanBeAccessed;

  UINT8 ubPlayerBeenToMercSiteStatus;
  BOOLEAN fFirstVisitSinceServerWentDown;
  BOOLEAN fNewMercsAvailableAtMercSite;
  BOOLEAN fSaidGenericOpeningInMercSite;
  BOOLEAN fSpeckSaidFloMarriedCousinQuote;
  BOOLEAN fHasAMercDiedAtMercSite;

#ifdef CRIPPLED_VERSION
  UINT8 ubCrippleFiller[20];
#endif

  INT8 gbNumDaysTillFirstMercArrives;
  INT8 gbNumDaysTillSecondMercArrives;
  INT8 gbNumDaysTillThirdMercArrives;
  INT8 gbNumDaysTillFourthMercArrives;

  uint32_t guiNumberOfMercPaymentsInDays;  // Keeps track of each day of payment the MERC site gets

  UINT16 usInventoryListLength[BOBBY_RAY_LISTS];

  INT32 iVoiceId;

  UINT8 ubHaveBeenToBobbyRaysAtLeastOnceWhileUnderConstruction;

  BOOLEAN fMercSiteHasGoneDownYet;

  UINT8 ubSpeckCanSayPlayersLostQuote;

  LAST_HIRED_MERC_STRUCT sLastHiredMerc;

  INT32 iCurrentHistoryPage;
  INT32 iCurrentFinancesPage;
  INT32 iCurrentEmailPage;

  uint32_t uiSpeckQuoteFlags;

  uint32_t uiFlowerOrderNumber;

  uint32_t uiTotalMoneyPaidToSpeck;

  UINT8 ubLastMercAvailableId;
  UINT8 bPadding[86];

} LaptopSaveInfoStruct;

extern LaptopSaveInfoStruct LaptopSaveInfo;

extern BobbyRayPurchaseStruct BobbyRayPurchases[MAX_PURCHASE_AMOUNT];

void LaptopSaveVariablesInit();
BOOLEAN LoadLaptopInfoFromSavedGame(HWFILE hFile);
BOOLEAN SaveLaptopInfoToSavedGame(HWFILE hFile);

#endif
