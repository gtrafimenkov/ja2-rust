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
  uint8_t ubSoldierID;
  uint8_t ubMercID;
  INT32 iPayOutPrice;
} LIFE_INSURANCE_PAYOUT;

typedef struct {
  BOOLEAN fHaveDisplayedPopUpInLaptop;  // Is set when the popup gets displayed, reset when entering
                                        // laptop again.
  INT32 iIdOfMerc;
  uint32_t uiArrivalTime;
} LAST_HIRED_MERC_STRUCT;

typedef struct {
  uint16_t usItemIndex;
  uint8_t ubNumberPurchased;
  int8_t bItemQuality;
  uint16_t usBobbyItemIndex;  // Item number in the BobbyRayInventory structure
  BOOLEAN fUsed;  // Indicates wether or not the item is from the used inventory or the regular
                  // inventory
} BobbyRayPurchaseStruct;

typedef struct {
  BOOLEAN fActive;
  BobbyRayPurchaseStruct BobbyRayPurchase[MAX_PURCHASE_AMOUNT];
  uint8_t ubNumberPurchases;
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
  uint8_t gubPlayersMercAccountStatus;
  uint32_t guiPlayersMercAccountNumber;
  uint8_t gubLastMercIndex;

  // Aim Site

  // BobbyRay Site
  STORE_INVENTORY BobbyRayInventory[MAXITEMS];
  STORE_INVENTORY BobbyRayUsedInventory[MAXITEMS];

  BobbyRayOrderStruct *BobbyRayOrdersOnDeliveryArray;
  uint8_t usNumberOfBobbyRayOrderItems;  // The number of elements in the array
  uint8_t usNumberOfBobbyRayOrderUsed;   // The number of items in the array that are used

  // Flower Site
  // NONE

  // Insurance Site
  LIFE_INSURANCE_PAYOUT *pLifeInsurancePayouts;
  uint8_t ubNumberLifeInsurancePayouts;     // The number of elements in the array
  uint8_t ubNumberLifeInsurancePayoutUsed;  // The number of items in the array that are used

  BOOLEAN fBobbyRSiteCanBeAccessed;

  uint8_t ubPlayerBeenToMercSiteStatus;
  BOOLEAN fFirstVisitSinceServerWentDown;
  BOOLEAN fNewMercsAvailableAtMercSite;
  BOOLEAN fSaidGenericOpeningInMercSite;
  BOOLEAN fSpeckSaidFloMarriedCousinQuote;
  BOOLEAN fHasAMercDiedAtMercSite;

#ifdef CRIPPLED_VERSION
  uint8_t ubCrippleFiller[20];
#endif

  int8_t gbNumDaysTillFirstMercArrives;
  int8_t gbNumDaysTillSecondMercArrives;
  int8_t gbNumDaysTillThirdMercArrives;
  int8_t gbNumDaysTillFourthMercArrives;

  uint32_t guiNumberOfMercPaymentsInDays;  // Keeps track of each day of payment the MERC site gets

  uint16_t usInventoryListLength[BOBBY_RAY_LISTS];

  INT32 iVoiceId;

  uint8_t ubHaveBeenToBobbyRaysAtLeastOnceWhileUnderConstruction;

  BOOLEAN fMercSiteHasGoneDownYet;

  uint8_t ubSpeckCanSayPlayersLostQuote;

  LAST_HIRED_MERC_STRUCT sLastHiredMerc;

  INT32 iCurrentHistoryPage;
  INT32 iCurrentFinancesPage;
  INT32 iCurrentEmailPage;

  uint32_t uiSpeckQuoteFlags;

  uint32_t uiFlowerOrderNumber;

  uint32_t uiTotalMoneyPaidToSpeck;

  uint8_t ubLastMercAvailableId;
  uint8_t bPadding[86];

} LaptopSaveInfoStruct;

extern LaptopSaveInfoStruct LaptopSaveInfo;

extern BobbyRayPurchaseStruct BobbyRayPurchases[MAX_PURCHASE_AMOUNT];

void LaptopSaveVariablesInit();
BOOLEAN LoadLaptopInfoFromSavedGame(HWFILE hFile);
BOOLEAN SaveLaptopInfoToSavedGame(HWFILE hFile);

#endif
