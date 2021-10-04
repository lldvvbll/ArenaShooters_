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

	void OnChangedSelectedWeapon(const TWeakObjectPtr<UASWeapon>& OldWeapon, const TWeakObjectPtr<UASWeapon>& NewWeapon);
	void OnInsertArmor(EArmorSlotType SlotType, UASArmor* Armor);

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
	UTextBlock* TotalAmmoTextBlock;

	UPROPERTY()
	UASInventoryComponent* InventoryComp;
};
