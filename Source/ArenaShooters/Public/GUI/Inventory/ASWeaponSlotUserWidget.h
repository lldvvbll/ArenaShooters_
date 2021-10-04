// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ArenaShooters.h"
#include "GUI/Inventory/ASEquipmentSlotUserWidget.h"
#include "Common/ASEnums.h"
#include "ASWeaponSlotUserWidget.generated.h"

class UTextBlock;

UCLASS()
class ARENASHOOTERS_API UASWeaponSlotUserWidget : public UASEquipmentSlotUserWidget
{
	GENERATED_BODY()

public:
	virtual void SetASItem(TWeakObjectPtr<UASItem>& NewItem) override;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

	virtual bool IsSuitableSlot(const TWeakObjectPtr<UASItem>& InItem) override;

	void OnCurrentAmmoCountChanged(int32 NewAmmoCount);

protected:
	UPROPERTY(EditAnywhere)
	EWeaponSlotType WeaponSlotType;

	UPROPERTY()
	UTextBlock* AmmoCountTextBlock;

	int32 MaxAmmoCount;

	FDelegateHandle OnCurrentAmmoCountChangedEventHandle;
};
