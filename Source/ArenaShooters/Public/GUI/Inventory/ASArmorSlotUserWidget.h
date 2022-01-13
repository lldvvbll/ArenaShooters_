// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ArenaShooters.h"
#include "GUI/Inventory/ASEquipmentSlotUserWidget.h"
#include "ASArmorSlotUserWidget.generated.h"

class UTextBlock;
class UASArmor;

UCLASS()
class ARENASHOOTERS_API UASArmorSlotUserWidget : public UASEquipmentSlotUserWidget
{
	GENERATED_BODY()

public:
	virtual void SetASItem(TWeakObjectPtr<UASItem>& NewItem) override;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

	virtual bool IsSuitableSlot(const TWeakObjectPtr<UASItem>& InItem) override;

	void OnChangedArmorDurability(float Durability, float MaxDurability);

protected:
	UPROPERTY()
	UTextBlock* DurabilityTextBlock;

	UPROPERTY(EditAnywhere)
	EArmorSlotType ArmorSlotType;
};
