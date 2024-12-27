#ifndef ITEMS_H
#define ITEMS_H

#include "Tactical/ItemTypes.h"

struct SOLDIERTYPE;
struct OBJECTTYPE;

extern uint8_t SlotToPocket[7];

extern BOOLEAN WeaponInHand(struct SOLDIERTYPE *pSoldier);

extern INT8 FindObj(struct SOLDIERTYPE *pSoldier, uint16_t usItem);
INT8 FindAmmo(struct SOLDIERTYPE *pSoldier, uint8_t ubCalibre, uint8_t ubMagSize,
              INT8 bExcludeSlot);

extern INT8 FindAttachment(struct OBJECTTYPE *pObj, uint16_t usItem);
extern INT8 FindObjClass(struct SOLDIERTYPE *pSoldier, uint32_t usItemClass);
extern INT8 FindObjClassAfterSlot(struct SOLDIERTYPE *pSoldier, INT8 bStart, uint32_t usItemClass);
extern INT8 FindAIUsableObjClass(struct SOLDIERTYPE *pSoldier, uint32_t usItemClass);
extern INT8 FindAIUsableObjClassWithin(struct SOLDIERTYPE *pSoldier, uint32_t usItemClass,
                                       INT8 bLower, INT8 bUpper);
extern INT8 FindEmptySlotWithin(struct SOLDIERTYPE *pSoldier, INT8 bLower, INT8 bUpper);
extern INT8 FindExactObj(struct SOLDIERTYPE *pSoldier, struct OBJECTTYPE *pObj);
extern INT8 FindObjInObjRange(struct SOLDIERTYPE *pSoldier, uint16_t usItem1, uint16_t usItem2);
extern INT8 FindLaunchable(struct SOLDIERTYPE *pSoldier, uint16_t usWeapon);
extern INT8 FindGLGrenade(struct SOLDIERTYPE *pSoldier);
extern INT8 FindThrowableGrenade(struct SOLDIERTYPE *pSoldier);
extern INT8 FindUsableObj(struct SOLDIERTYPE *pSoldier, uint16_t usItem);

extern void DeleteObj(struct OBJECTTYPE *pObj);
extern void CopyObj(struct OBJECTTYPE *pSourceObj, struct OBJECTTYPE *pTargetObj);
extern void SwapObjs(struct OBJECTTYPE *pObj1, struct OBJECTTYPE *pObj2);

extern void SwapWithinObj(struct OBJECTTYPE *pObj, uint8_t ubIndex1, uint8_t ubIndex2);
extern void RemoveObjFrom(struct OBJECTTYPE *pObj, uint8_t ubRemoveIndex);
// Returns true if swapped, false if added to end of stack
extern BOOLEAN PlaceObjectAtObjectIndex(struct OBJECTTYPE *pSourceObj,
                                        struct OBJECTTYPE *pTargetObj, uint8_t ubIndex);
extern void GetObjFrom(struct OBJECTTYPE *pObj, uint8_t ubGetIndex, struct OBJECTTYPE *pDest);

extern BOOLEAN AttachObject(struct SOLDIERTYPE *pSoldier, struct OBJECTTYPE *pTargetObj,
                            struct OBJECTTYPE *pAttachment);
extern BOOLEAN RemoveAttachment(struct OBJECTTYPE *pObj, INT8 bAttachPos,
                                struct OBJECTTYPE *pNewObj);

extern uint8_t CalculateObjectWeight(struct OBJECTTYPE *pObject);
extern uint32_t CalculateCarriedWeight(struct SOLDIERTYPE *pSoldier);

extern uint16_t TotalPoints(struct OBJECTTYPE *pObj);
extern uint16_t UseKitPoints(struct OBJECTTYPE *pObj, uint16_t usPoints,
                             struct SOLDIERTYPE *pSoldier);

extern BOOLEAN EmptyWeaponMagazine(struct OBJECTTYPE *pWeapon, struct OBJECTTYPE *pAmmo);
extern BOOLEAN CreateItem(uint16_t usItem, INT8 bStatus, struct OBJECTTYPE *pObj);
extern BOOLEAN CreateItems(uint16_t usItem, INT8 bStatus, uint8_t ubNumber,
                           struct OBJECTTYPE *pObj);
extern BOOLEAN CreateMoney(uint32_t uiMoney, struct OBJECTTYPE *pObj);
extern uint16_t DefaultMagazine(uint16_t usItem);
uint16_t RandomMagazine(uint16_t usItem, uint8_t ubPercentStandard);
extern BOOLEAN ReloadGun(struct SOLDIERTYPE *pSoldier, struct OBJECTTYPE *pGun,
                         struct OBJECTTYPE *pAmmo);

uint8_t ItemSlotLimit(uint16_t usItem, INT8 bSlot);

// Function to put an item in a soldier profile
// It's very primitive, just finds an empty place!
BOOLEAN PlaceObjectInSoldierProfile(uint8_t ubProfile, struct OBJECTTYPE *pObject);
BOOLEAN RemoveObjectFromSoldierProfile(uint8_t ubProfile, uint16_t usItem);
INT8 FindObjectInSoldierProfile(uint8_t ubProfile, uint16_t usItem);

BOOLEAN ObjectExistsInSoldierProfile(uint8_t ubProfile, uint16_t usItem);
void SetMoneyInSoldierProfile(uint8_t ubProfile, uint32_t uiMoney);

void DamageObj(struct OBJECTTYPE *pObj, INT8 bAmount);

void CheckEquipmentForDamage(struct SOLDIERTYPE *pSoldier, INT32 iDamage);
BOOLEAN ArmBomb(struct OBJECTTYPE *pObj, INT8 bSetting);

// NOTE TO ANDREW:
//
// The following functions expect that pObj points to the object
// "in the cursor", which should have memory allocated for it already
BOOLEAN PlaceObject(struct SOLDIERTYPE *pSoldier, INT8 bPos, struct OBJECTTYPE *pObj);

// Send fNewItem to true to set off new item glow in inv panel
BOOLEAN AutoPlaceObject(struct SOLDIERTYPE *pSoldier, struct OBJECTTYPE *pObj, BOOLEAN fNewItem);
BOOLEAN RemoveObjectFromSlot(struct SOLDIERTYPE *pSoldier, INT8 bPos, struct OBJECTTYPE *pObj);

void RemoveInvObject(struct SOLDIERTYPE *pSoldier, uint16_t usItem);

uint8_t SwapKeysToSlot(struct SOLDIERTYPE *pSoldier, INT8 bKeyRingPosition,
                       struct OBJECTTYPE *pObj);

// create a keyobject
BOOLEAN CreateKeyObject(struct OBJECTTYPE *pObj, uint8_t ubNumberOfKeys, uint8_t ubKeyIdValue);
BOOLEAN DeleteKeyObject(struct OBJECTTYPE *pObj);
BOOLEAN AllocateObject(struct OBJECTTYPE **pObj);

// removes a key from a *KEYRING* slot
BOOLEAN RemoveKeyFromSlot(struct SOLDIERTYPE *pSoldier, INT8 bKeyRingPosition,
                          struct OBJECTTYPE *pObj);

// take several
BOOLEAN RemoveKeysFromSlot(struct SOLDIERTYPE *pSoldier, INT8 bKeyRingPosition,
                           uint8_t ubNumberOfKeys, struct OBJECTTYPE *pObj);

// add the keys to an inventory slot
uint8_t AddKeysToSlot(struct SOLDIERTYPE *pSoldier, INT8 bKeyRingPosition, struct OBJECTTYPE *pObj);

// Kris:  December 9, 1997
// I need a bunch of validation functions for ammo, attachments, etc., so I'll be adding them here.
// Chris, maybe you might find these useful, or add your own.  I don't really know what I'm doing
// yet, so feel free to correct me...

// Simple check to see if the item has any attachments
BOOLEAN ItemHasAttachments(struct OBJECTTYPE *pItem);

// Determine if this item can receive this attachment.  This is different, in that it may
// be possible to have this attachment on this item, but may already have an attachment on
// it which doesn't work simultaneously with the new attachment (like a silencer and duckbill).
BOOLEAN ValidItemAttachment(struct OBJECTTYPE *pObj, uint16_t usAttachment,
                            BOOLEAN fAttemptingAttachment);

// Determines if it is possible to merge an item with any item whose class
// is the same as the indicated item
BOOLEAN ValidAttachmentClass(uint16_t usAttachment, uint16_t usItem);

// Determines if it is possible to equip this weapon with this ammo.
BOOLEAN ValidAmmoType(uint16_t usItem, uint16_t usAmmoType);

// Determines if this item is a two handed item.
BOOLEAN TwoHandedItem(uint16_t usItem);

// Existing functions without header def's, added them here, just incase I'll need to call
// them from the editor.
BOOLEAN ValidAttachment(uint16_t usAttachment, uint16_t usItem);

BOOLEAN ValidLaunchable(uint16_t usLaunchable, uint16_t usItem);
uint16_t GetLauncherFromLaunchable(uint16_t usLaunchable);

BOOLEAN ValidMerge(uint16_t usMerge, uint16_t usItem);
BOOLEAN EvaluateValidMerge(uint16_t usMerge, uint16_t usItem, uint16_t *pusResult,
                           uint8_t *pubType);

// is the item passed a medical/ first aid kit? and what type?
INT8 IsMedicalKitItem(struct OBJECTTYPE *pObject);

BOOLEAN AutoReload(struct SOLDIERTYPE *pSoldier);
INT8 FindAmmoToReload(struct SOLDIERTYPE *pSoldier, INT8 bWeaponIn, INT8 bExcludeSlot);

void SwapOutHandItem(struct SOLDIERTYPE *pSoldier);
void SwapHandItems(struct SOLDIERTYPE *pSoldier);

INT8 FindAttachmentByClass(struct OBJECTTYPE *pObj, uint32_t uiItemClass);
void RemoveObjs(struct OBJECTTYPE *pObj, uint8_t ubNumberToRemove);

void WaterDamage(struct SOLDIERTYPE *pSoldier);

INT8 FindObjWithin(struct SOLDIERTYPE *pSoldier, uint16_t usItem, INT8 bLower, INT8 bUpper);

BOOLEAN ApplyCammo(struct SOLDIERTYPE *pSoldier, struct OBJECTTYPE *pObj, BOOLEAN *pfGoodAPs);

uint32_t ConvertProfileMoneyValueToObjectTypeMoneyValue(uint8_t ubStatus);
uint8_t ConvertObjectTypeMoneyValueToProfileMoneyValue(uint32_t uiMoneyAmount);

BOOLEAN CheckForChainReaction(uint16_t usItem, INT8 bStatus, INT8 bDamage, BOOLEAN fOnGround);

BOOLEAN ItemIsLegal(uint16_t usItemIndex);
BOOLEAN ExtendedGunListGun(uint16_t usGun);
uint16_t StandardGunListReplacement(uint16_t usGun);
uint16_t FindReplacementMagazine(uint8_t ubCalibre, uint8_t ubMagSize, uint8_t ubAmmoType);
uint16_t FindReplacementMagazineIfNecessary(uint16_t usOldGun, uint16_t usOldAmmo,
                                            uint16_t usNewGun);

BOOLEAN DamageItemOnGround(struct OBJECTTYPE *pObject, INT16 sGridNo, INT8 bLevel, INT32 iDamage,
                           uint8_t ubOwner);

BOOLEAN ApplyCanteen(struct SOLDIERTYPE *pSoldier, struct OBJECTTYPE *pObj, BOOLEAN *pfGoodAPs);
BOOLEAN ApplyElixir(struct SOLDIERTYPE *pSoldier, struct OBJECTTYPE *pObj, BOOLEAN *pfGoodAPs);

BOOLEAN CompatibleFaceItem(uint16_t usItem1, uint16_t usItem2);

uint32_t MoneySlotLimit(INT8 bSlot);

void CheckEquipmentForFragileItemDamage(struct SOLDIERTYPE *pSoldier, INT32 iDamage);

// Range of Xray device
#define XRAY_RANGE 40
// Seconds that Xray lasts
#define XRAY_TIME 5

extern void ActivateXRayDevice(struct SOLDIERTYPE *pSoldier);
extern void TurnOffXRayEffects(struct SOLDIERTYPE *pSoldier);
extern INT8 FindLaunchableAttachment(struct OBJECTTYPE *pObj, uint16_t usWeapon);

#endif
