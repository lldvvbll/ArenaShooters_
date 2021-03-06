// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ArenaShooters.h"
#include "Blueprint/UserWidget.h"
#include "Common/ASEnums.h"
#include "ASInventoryStatusUserWidget.generated.h"

class UProgressBar;
class UTextBlock;
class UASWeapon;
class UASArmor;
class UASInventoryComponent;

UCLASS()
class ARENASHOOTERS_API UASInventoryStatusUserWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	void OnChangedSelectedWeapon(const TWeakObjectPtr<UASWeapon>& OldWeapon, const TWeakObjectPtr<UASWeapon>& NewWeapon);
	void OnInsertArmor(EArmorSlotType SlotType, UASArmor* Armor);

	void BindProgressBarToArmor(UProgressBar* ProgressBar, EArmorSlotType SlotType);

	void OnChangedArmorDurability(float Durability, float MaxDurability, int32 SlotTypeInt);
	void OnChangedFireMode(EFireMode NewFireMode);
	void OnChangedCurrentAmmoCount(int32 NewCount);
	void OnChangedInventoryAmmoCount(int32 NewCount);

protected:
	UPROPERTY()
	UProgressBar* HelmetProgressBar;
		
	UPROPERTY()
	UProgressBar* JacketProgressBar;

	UPROPERTY()
	UTextBlock* FireModeTextBlock;

	UPROPERTY()
	UTextBlock* CurrentAmmoTextBlock;

	UPROPERTY()
	UTextBlock* InventoryAmmoTextBlock;

	UPROPERTY()
	UASInventoryComponent* InventoryComp;
};
