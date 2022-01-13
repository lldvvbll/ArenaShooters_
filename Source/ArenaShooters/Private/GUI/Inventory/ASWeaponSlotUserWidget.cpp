// Fill out your copyright notice in the Description page of Project Settings.


#include "GUI/Inventory/ASWeaponSlotUserWidget.h"
#include "Components/Border.h"
#include "Components/TextBlock.h"
#include "Item/ASWeapon.h"
#include "Character/ASCharacter.h"
#include "Character/ASInventoryComponent.h"

void UASWeaponSlotUserWidget::SetASItem(TWeakObjectPtr<UASItem>& NewItem)
{
	TWeakObjectPtr<UASItem> OldItem = Item;

	Super::SetASItem(NewItem);

	if (UASWeapon* OldWeapon = (OldItem.IsValid() ? Cast<UASWeapon>(OldItem) : nullptr))
	{
		OldWeapon->OnCurrentAmmoCountChanged.RemoveAll(this);
		MaxAmmoCount = 0;
	}

	if (UASWeapon* NewWeapon = (Item.IsValid() ? Cast<UASWeapon>(Item) : nullptr))
	{
		NewWeapon->OnCurrentAmmoCountChanged.AddUObject(this, &UASWeaponSlotUserWidget::OnCurrentAmmoCountChanged);

		MaxAmmoCount = NewWeapon->GetMaxAmmoCount();
		OnCurrentAmmoCountChanged(NewWeapon->GetCurrentAmmoCount());
	}
	else
	{
		OnCurrentAmmoCountChanged(0);
	}
}

void UASWeaponSlotUserWidget::NativeConstruct()
{
	Super::NativeConstruct();

	AmmoCountTextBlock = Cast<UTextBlock>(GetWidgetFromName(TEXT("AmmoCountTextBlock")));

	MaxAmmoCount = 0;
}

void UASWeaponSlotUserWidget::NativeDestruct()
{
	Super::NativeDestruct();

	if (UASWeapon* Weapon = (Item.IsValid() ? Cast<UASWeapon>(Item) : nullptr))
	{
		Weapon->OnCurrentAmmoCountChanged.RemoveAll(this);
		MaxAmmoCount = 0;
	}
}

bool UASWeaponSlotUserWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	if (Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation))
		return true;

	if (GetOperationParentWidget(InOperation) != this)
	{
		TWeakObjectPtr<UASItem> WeaponItem = GetASItemFromDragDropOperation(InOperation);
		if (IsSuitableSlot(WeaponItem))
		{
			auto ASChar = Cast<AASCharacter>(GetOwningPlayerPawn());
			if (ASChar != nullptr)
			{
				ASChar->PickUpWeapon(WeaponSlotType, Cast<UASWeapon>(WeaponItem));
				return true;
			}
			else
			{
				AS_LOG_SCREEN_S(5.0f, FColor::Red);
			}
		}
	}

	return false;
}

bool UASWeaponSlotUserWidget::IsSuitableSlot(const TWeakObjectPtr<UASItem>& InItem)
{
	if (!Super::IsSuitableSlot(InItem))
		return false;

	auto Weapon = Cast<UASWeapon>(InItem);
	if (Weapon == nullptr)
		return false;
	
	if (!UASInventoryComponent::IsSuitableWeaponSlot(WeaponSlotType, Weapon))
		return false;

	return true;
}

void UASWeaponSlotUserWidget::OnCurrentAmmoCountChanged(int32 NewAmmoCount)
{
	if (AmmoCountTextBlock != nullptr)
	{
		AmmoCountTextBlock->SetText(FText::FromString(FString::Printf(TEXT("%d/%d"), NewAmmoCount, MaxAmmoCount)));
	}
}
