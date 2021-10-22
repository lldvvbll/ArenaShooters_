// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ArenaShooters.h"
#include "Blueprint/UserWidget.h"
#include "Common/ASEnums.h"
#include "ASInventoryUserWidget.generated.h"

class AASCharacter;
class UASWeaponSlotUserWidget;
class UASArmorSlotUserWidget;
class UASItemScrollBoxWrapperUserWidget;
class UASWeapon;
class UASArmor;
class UASInventoryComponent;
class UASItem;

UCLASS()
class ARENASHOOTERS_API UASInventoryUserWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	AASCharacter* GetASCharacter() const;
	
	void AddItemsToGroundScrollBox(const TArray<TWeakObjectPtr<UASItem>>& Items);
	void RemoveItemsFromGroundScrollBox(const TArray<TWeakObjectPtr<UASItem>>& Items);
	
	void AddItemsToInventoryScrollBox(const TArray<TWeakObjectPtr<UASItem>>& Items);

	void OnChangedWeapon(EWeaponSlotType SlotType, UASWeapon* RemovedWeapon);
	void OnChangedArmor(EArmorSlotType SlotType, UASArmor* RemovedArmor);

	void OnAddInventoryItem(const TWeakObjectPtr<UASItem>& NewItem);
	void OnRemoveInventoryItem(const TWeakObjectPtr<UASItem>& InItem);

	UFUNCTION()
	void BackToGame();

public:
	DECLARE_EVENT_OneParam(UASInventoryUserWidget, FOnConstructedEvent, UUserWidget*);
	FOnConstructedEvent OnConstructed;

	DECLARE_EVENT_OneParam(UASInventoryUserWidget, FOnDestructedEvent, UUserWidget*);
	FOnDestructedEvent OnDestructed;

private:
	UPROPERTY()
	UASWeaponSlotUserWidget* MainWeaponSlotWidget;

	UPROPERTY()
	UASWeaponSlotUserWidget* SubWeaponSlotWidget;

	UPROPERTY()
	UASArmorSlotUserWidget* HelmetSlotWidget;

	UPROPERTY()
	UASArmorSlotUserWidget* JacketSlotWidget;

	UPROPERTY()
	UASItemScrollBoxWrapperUserWidget* InventoryItemScrollBoxWrapperWidget;

	UPROPERTY()
	UASItemScrollBoxWrapperUserWidget* GroundItemScrollBoxWrapperWidget;

	UPROPERTY()
	UASInventoryComponent* ASInventoryComp;
};
