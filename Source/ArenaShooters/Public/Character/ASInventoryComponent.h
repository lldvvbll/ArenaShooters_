// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ArenaShooters.h"
#include "Components/ActorComponent.h"
#include "Common/ASEnums.h"
#include "ASInventoryComponent.generated.h"

class UASItem;
class UASWeapon;
class UASArmor;
class UASAmmo;
class UASHealingKit;
class UASWeaponDataAsset;
class UASArmorDataAsset;
class AASWeaponActor;
class AASArmorActor;
class UASInventoryDataAsset;

using ItemBoolPair = TPair<UASItem*, bool>;
using ItemPtrBoolPair = TPair<TWeakObjectPtr<UASItem>, bool>;

UCLASS()
class ARENASHOOTERS_API UASInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UASInventoryComponent();

	virtual void InitializeComponent() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	TWeakObjectPtr<UASWeapon> GetSelectedWeapon() const;
	TWeakObjectPtr<AASWeaponActor> GetSelectedWeaponActor();
	const EWeaponType GetSelectedWeaponType() const;
	const EWeaponSlotType GetSelectedWeaponSlotType() const;

	static EWeaponSlotType GetSuitableWeaponSlotType(EWeaponType WeaponType);
	static EArmorSlotType GetSuitableArmorSlotType(EArmorType ArmorType);
	static bool IsSuitableWeaponSlot(EWeaponSlotType SlotType, const UASWeapon* Weapon);
	static bool IsSuitableArmorSlot(EArmorSlotType SlotType, const UASArmor* Armor);
	
	FName GetProperWeaponSocketName(EWeaponType WeaponType, bool bUsing);
	FName GetProperArmorSocketName(EArmorType ArmorType);

	bool InsertWeapon(EWeaponSlotType SlotType, UASWeapon* NewWeapon, UASItem*& Out_OldItem);
	bool InsertArmor(EArmorSlotType SlotType, UASArmor* NewArmor, UASItem*& Out_OldItem);

	bool SelectWeapon(EWeaponSlotType SlotType);

	ItemBoolPair RemoveItem(UASItem* InItem);

	ItemPtrBoolPair FindItemFromWeaponSlot(EWeaponSlotType SlotType) const;
	ItemPtrBoolPair SetItemToWeaponSlot(EWeaponSlotType SlotType, UASItem* NewItem);
	ItemBoolPair RemoveItemFromWeaponSlot(EWeaponSlotType SlotType);

	ItemPtrBoolPair FindItemFromArmorSlot(EArmorSlotType SlotType) const;
	ItemPtrBoolPair SetItemToArmorSlot(EArmorSlotType SlotType, UASItem* NewItem);
	ItemBoolPair RemoveItemFromArmorSlot(EArmorSlotType SlotType);

	bool IsEnableToAddItemToInventory(UASItem* NewItem) const;
	bool AddItemToInventory(UASItem* NewItem);
	TArray<TWeakObjectPtr<UASItem>> GetInventoryItems() const;
	bool Contains(UASItem* InItem) const;

	int32 GetAmmoCountInInventory(EAmmoType AmmoType) const;
	TArray<UASAmmo*> GetAmmos(EAmmoType AmmoType) const;
	TArray<UASHealingKit*> GetHealingKits() const;

	void ReattachWeaponActor(UASWeapon* InWeapon, const FName& SocketName) const;

	TArray<TWeakObjectPtr<UASArmor>> GetCoveringArmors(const FName& BoneName);

private:
	ItemBoolPair GetItemFromWeaponSlot(EWeaponSlotType SlotType);
	ItemBoolPair GetItemFromArmorSlot(EArmorSlotType SlotType);

	void OnWeaponInserted(EWeaponSlotType SlotType, UASWeapon* InsertedWeapon);
	void OnArmorInserted(EArmorSlotType SlotType, UASArmor* InsertedArmor);
	void OnWeaponRemoved(EWeaponSlotType SlotType, UASWeapon* RemovedWeapon);
	void OnArmorRemoved(UASArmor* RemovedArmor);
	void OnSelectedWeaponChanged(UASWeapon* OldWeapon, UASWeapon* NewWeapon);
	void OnAddedItemToInventory(UASItem* AddedItem);
	void OnRemovedItemFromInventory(UASItem* RemovedItem);
	void OnChangedInventoryItemCount(UASItem* Item);
	void OnChangedAmmoCountInInventory(EAmmoType AmmoType);

	void SpawnWeaponActor(UASWeapon& Weapon, const FName& AttachSocket);
	void SpawnArmorActor(UASArmor& Armor, const FName& AttachSocket);

	EWeaponSlotType GetWeaponSlotTypeFromWeapon(UASWeapon* InWeapon);

	UFUNCTION()
	void OnRep_WeaponSlots(TArray<UASItem*>& OldWeaponSlots);

	UFUNCTION()
	void OnRep_ArmorSlots(TArray<UASItem*>& OldArmorSlots);

	UFUNCTION()
	void OnRep_SelectedWeapon(UASWeapon* OldWeapon);

	UASItem* FindItemFromInventory(UASItem* ComparingItem) const;

	UFUNCTION()
	void OnRep_InventoryItems(TArray<UASItem*>& OldInventoryItems);

public:
	DECLARE_EVENT_TwoParams(UASInventoryComponent, FOnInsertWeaponEvent, EWeaponSlotType, UASWeapon*);
	FOnInsertWeaponEvent OnInsertWeapon;

	DECLARE_EVENT_TwoParams(UASInventoryComponent, FOnInsertArmorEvent, EArmorSlotType, UASArmor*);
	FOnInsertArmorEvent OnInsertArmor;

	DECLARE_EVENT_OneParam(UASInventoryComponent, FOnAddInventoryItemEvent, const TWeakObjectPtr<UASItem>&);
	FOnAddInventoryItemEvent OnAddInventoryItem;

	DECLARE_EVENT_OneParam(UASInventoryComponent, FOnRemoveInventoryItemEvent, const TWeakObjectPtr<UASItem>&);
	FOnRemoveInventoryItemEvent OnRemoveInventoryItem;

	DECLARE_EVENT_TwoParams(UASInventoryComponent, FOnChangedSelectedWeaponEvent, const TWeakObjectPtr<UASWeapon>&, const TWeakObjectPtr<UASWeapon>&);
	FOnChangedSelectedWeaponEvent OnChangedSelectedWeapon;

	DECLARE_EVENT_OneParam(UASInventoryComponent, FOnChangedInventoryAmmoCountEvent, int32);
	FOnChangedInventoryAmmoCountEvent OnChangedInventoryAmmoCount;

private:
	UPROPERTY(EditDefaultsOnly, Meta = (AllowPrivateAccess = true))
	FPrimaryAssetId InventoryAssetId;

	UPROPERTY()
	UASInventoryDataAsset* InventoryDataAsset;

	UPROPERTY(ReplicatedUsing = OnRep_WeaponSlots)
	TArray<UASItem*> WeaponSlots;

	UPROPERTY(ReplicatedUsing = OnRep_ArmorSlots)
	TArray<UASItem*> ArmorSlots;

	UPROPERTY(ReplicatedUsing = OnRep_InventoryItems)
	TArray<UASItem*> InventoryItems;

	UPROPERTY(ReplicatedUsing = OnRep_SelectedWeapon)
	UASWeapon* SelectedWeapon;

	UPROPERTY(Replicated)
	EWeaponSlotType SelectedWeaponSlotType;

	UPROPERTY(EditDefaultsOnly)
	int32 MaxInventoryItemCount;

	UPROPERTY()
	UASAmmo* ReloadingAmmo;
};
