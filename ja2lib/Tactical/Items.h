#ifndef ITEMS_H
#define ITEMS_H

#include "Tactical/ItemTypes.h"

struct SOLDIERTYPE;
struct OBJECTTYPE;

extern UINT8 SlotToPocket[7];

extern BOOLEAN WeaponInHand(struct SOLDIERTYPE *pSoldier);

extern INT8 FindObj(struct SOLDIERTYPE *pSoldier, UINT16 usItem);
INT8 FindAmmo(struct SOLDIERTYPE *pSoldier, UINT8 ubCalibre, UINT8 ubMagSize, INT8 bExcludeSlot);

extern INT8 FindAttachment(struct OBJECTTYPE *pObj, UINT16 usItem);
extern INT8 FindObjClass(struct SOLDIERTYPE *pSoldier, uint32_t usItemClass);
extern INT8 FindObjClassAfterSlot(struct SOLDIERTYPE *pSoldier, INT8 bStart, uint32_t usItemClass);
extern INT8 FindAIUsableObjClass(struct SOLDIERTYPE *pSoldier, uint32_t usItemClass);
extern INT8 FindAIUsableObjClassWithin(struct SOLDIERTYPE *pSoldier, uint32_t usItemClass,
                                       INT8 bLower, INT8 bUpper);
extern INT8 FindEmptySlotWithin(struct SOLDIERTYPE *pSoldier, INT8 bLower, INT8 bUpper);
extern INT8 FindExactObj(struct SOLDIERTYPE *pSoldier, struct OBJECTTYPE *pObj);
extern INT8 FindObjInObjRange(struct SOLDIERTYPE *pSoldier, UINT16 usItem1, UINT16 usItem2);
extern INT8 FindLaunchable(struct SOLDIERTYPE *pSoldier, UINT16 usWeapon);
extern INT8 FindGLGrenade(struct SOLDIERTYPE *pSoldier);
extern INT8 FindThrowableGrenade(struct SOLDIERTYPE *pSoldier);
extern INT8 FindUsableObj(struct SOLDIERTYPE *pSoldier, UINT16 usItem);

extern void DeleteObj(struct OBJECTTYPE *pObj);
extern void CopyObj(struct OBJECTTYPE *pSourceObj, struct OBJECTTYPE *pTargetObj);
extern void SwapObjs(struct OBJECTTYPE *pObj1, struct OBJECTTYPE *pObj2);

extern void SwapWithinObj(struct OBJECTTYPE *pObj, UINT8 ubIndex1, UINT8 ubIndex2);
extern void RemoveObjFrom(struct OBJECTTYPE *pObj, UINT8 ubRemoveIndex);
// Returns true if swapped, false if added to end of stack
extern BOOLEAN PlaceObjectAtObjectIndex(struct OBJECTTYPE *pSourceObj,
                                        struct OBJECTTYPE *pTargetObj, UINT8 ubIndex);
extern void GetObjFrom(struct OBJECTTYPE *pObj, UINT8 ubGetIndex, struct OBJECTTYPE *pDest);

extern BOOLEAN AttachObject(struct SOLDIERTYPE *pSoldier, struct OBJECTTYPE *pTargetObj,
                            struct OBJECTTYPE *pAttachment);
extern BOOLEAN RemoveAttachment(struct OBJECTTYPE *pObj, INT8 bAttachPos,
                                struct OBJECTTYPE *pNewObj);

extern UINT8 CalculateObjectWeight(struct OBJECTTYPE *pObject);
extern uint32_t CalculateCarriedWeight(struct SOLDIERTYPE *pSoldier);

extern UINT16 TotalPoints(struct OBJECTTYPE *pObj);
extern UINT16 UseKitPoints(struct OBJECTTYPE *pObj, UINT16 usPoints, struct SOLDIERTYPE *pSoldier);

extern BOOLEAN EmptyWeaponMagazine(struct OBJECTTYPE *pWeapon, struct OBJECTTYPE *pAmmo);
extern BOOLEAN CreateItem(UINT16 usItem, INT8 bStatus, struct OBJECTTYPE *pObj);
extern BOOLEAN CreateItems(UINT16 usItem, INT8 bStatus, UINT8 ubNumber, struct OBJECTTYPE *pObj);
extern BOOLEAN CreateMoney(uint32_t uiMoney, struct OBJECTTYPE *pObj);
extern UINT16 DefaultMagazine(UINT16 usItem);
UINT16 RandomMagazine(UINT16 usItem, UINT8 ubPercentStandard);
extern BOOLEAN ReloadGun(struct SOLDIERTYPE *pSoldier, struct OBJECTTYPE *pGun,
                         struct OBJECTTYPE *pAmmo);

UINT8 ItemSlotLimit(UINT16 usItem, INT8 bSlot);

// Function to put an item in a soldier profile
// It's very primitive, just finds an empty place!
BOOLEAN PlaceObjectInSoldierProfile(UINT8 ubProfile, struct OBJECTTYPE *pObject);
BOOLEAN RemoveObjectFromSoldierProfile(UINT8 ubProfile, UINT16 usItem);
INT8 FindObjectInSoldierProfile(UINT8 ubProfile, UINT16 usItem);

BOOLEAN ObjectExistsInSoldierProfile(UINT8 ubProfile, UINT16 usItem);
void SetMoneyInSoldierProfile(UINT8 ubProfile, uint32_t uiMoney);

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

void RemoveInvObject(struct SOLDIERTYPE *pSoldier, UINT16 usItem);

UINT8 SwapKeysToSlot(struct SOLDIERTYPE *pSoldier, INT8 bKeyRingPosition, struct OBJECTTYPE *pObj);

// create a keyobject
BOOLEAN CreateKeyObject(struct OBJECTTYPE *pObj, UINT8 ubNumberOfKeys, UINT8 ubKeyIdValue);
BOOLEAN DeleteKeyObject(struct OBJECTTYPE *pObj);
BOOLEAN AllocateObject(struct OBJECTTYPE **pObj);

// removes a key from a *KEYRING* slot
BOOLEAN RemoveKeyFromSlot(struct SOLDIERTYPE *pSoldier, INT8 bKeyRingPosition,
                          struct OBJECTTYPE *pObj);

// take several
BOOLEAN RemoveKeysFromSlot(struct SOLDIERTYPE *pSoldier, INT8 bKeyRingPosition,
                           UINT8 ubNumberOfKeys, struct OBJECTTYPE *pObj);

// add the keys to an inventory slot
UINT8 AddKeysToSlot(struct SOLDIERTYPE *pSoldier, INT8 bKeyRingPosition, struct OBJECTTYPE *pObj);

// Kris:  December 9, 1997
// I need a bunch of validation functions for ammo, attachments, etc., so I'll be adding them here.
// Chris, maybe you might find these useful, or add your own.  I don't really know what I'm doing
// yet, so feel free to correct me...

// Simple check to see if the item has any attachments
BOOLEAN ItemHasAttachments(struct OBJECTTYPE *pItem);

// Determine if this item can receive this attachment.  This is different, in that it may
// be possible to have this attachment on this item, but may already have an attachment on
// it which doesn't work simultaneously with the new attachment (like a silencer and duckbill).
BOOLEAN ValidItemAttachment(struct OBJECTTYPE *pObj, UINT16 usAttachment,
                            BOOLEAN fAttemptingAttachment);

// Determines if it is possible to merge an item with any item whose class
// is the same as the indicated item
BOOLEAN ValidAttachmentClass(UINT16 usAttachment, UINT16 usItem);

// Determines if it is possible to equip this weapon with this ammo.
BOOLEAN ValidAmmoType(UINT16 usItem, UINT16 usAmmoType);

// Determines if this item is a two handed item.
BOOLEAN TwoHandedItem(UINT16 usItem);

// Existing functions without header def's, added them here, just incase I'll need to call
// them from the editor.
BOOLEAN ValidAttachment(UINT16 usAttachment, UINT16 usItem);

BOOLEAN ValidLaunchable(UINT16 usLaunchable, UINT16 usItem);
UINT16 GetLauncherFromLaunchable(UINT16 usLaunchable);

BOOLEAN ValidMerge(UINT16 usMerge, UINT16 usItem);
BOOLEAN EvaluateValidMerge(UINT16 usMerge, UINT16 usItem, UINT16 *pusResult, UINT8 *pubType);

// is the item passed a medical/ first aid kit? and what type?
INT8 IsMedicalKitItem(struct OBJECTTYPE *pObject);

BOOLEAN AutoReload(struct SOLDIERTYPE *pSoldier);
INT8 FindAmmoToReload(struct SOLDIERTYPE *pSoldier, INT8 bWeaponIn, INT8 bExcludeSlot);

void SwapOutHandItem(struct SOLDIERTYPE *pSoldier);
void SwapHandItems(struct SOLDIERTYPE *pSoldier);

INT8 FindAttachmentByClass(struct OBJECTTYPE *pObj, uint32_t uiItemClass);
void RemoveObjs(struct OBJECTTYPE *pObj, UINT8 ubNumberToRemove);

void WaterDamage(struct SOLDIERTYPE *pSoldier);

INT8 FindObjWithin(struct SOLDIERTYPE *pSoldier, UINT16 usItem, INT8 bLower, INT8 bUpper);

BOOLEAN ApplyCammo(struct SOLDIERTYPE *pSoldier, struct OBJECTTYPE *pObj, BOOLEAN *pfGoodAPs);

uint32_t ConvertProfileMoneyValueToObjectTypeMoneyValue(UINT8 ubStatus);
UINT8 ConvertObjectTypeMoneyValueToProfileMoneyValue(uint32_t uiMoneyAmount);

BOOLEAN CheckForChainReaction(UINT16 usItem, INT8 bStatus, INT8 bDamage, BOOLEAN fOnGround);

BOOLEAN ItemIsLegal(UINT16 usItemIndex);
BOOLEAN ExtendedGunListGun(UINT16 usGun);
UINT16 StandardGunListReplacement(UINT16 usGun);
UINT16 FindReplacementMagazine(UINT8 ubCalibre, UINT8 ubMagSize, UINT8 ubAmmoType);
UINT16 FindReplacementMagazineIfNecessary(UINT16 usOldGun, UINT16 usOldAmmo, UINT16 usNewGun);

BOOLEAN DamageItemOnGround(struct OBJECTTYPE *pObject, INT16 sGridNo, INT8 bLevel, INT32 iDamage,
                           UINT8 ubOwner);

BOOLEAN ApplyCanteen(struct SOLDIERTYPE *pSoldier, struct OBJECTTYPE *pObj, BOOLEAN *pfGoodAPs);
BOOLEAN ApplyElixir(struct SOLDIERTYPE *pSoldier, struct OBJECTTYPE *pObj, BOOLEAN *pfGoodAPs);

BOOLEAN CompatibleFaceItem(UINT16 usItem1, UINT16 usItem2);

uint32_t MoneySlotLimit(INT8 bSlot);

void CheckEquipmentForFragileItemDamage(struct SOLDIERTYPE *pSoldier, INT32 iDamage);

// Range of Xray device
#define XRAY_RANGE 40
// Seconds that Xray lasts
#define XRAY_TIME 5

extern void ActivateXRayDevice(struct SOLDIERTYPE *pSoldier);
extern void TurnOffXRayEffects(struct SOLDIERTYPE *pSoldier);
extern INT8 FindLaunchableAttachment(struct OBJECTTYPE *pObj, UINT16 usWeapon);

#endif
