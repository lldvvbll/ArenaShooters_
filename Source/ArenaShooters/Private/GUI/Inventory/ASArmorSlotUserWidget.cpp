// Fill out your copyright notice in the Description page of Project Settings.


#include "GUI/Inventory/ASArmorSlotUserWidget.h"
#include "Components/Border.h"
#include "Components/TextBlock.h"
#include "Item/ASArmor.h"
#include "Character/ASInventoryComponent.h"
#include "Character/ASCharacter.h"

void UASArmorSlotUserWidget::SetASItem(TWeakObjectPtr<UASItem>& NewItem)
{
	TWeakObjectPtr<UASItem> OldItem = Item;

	Super::SetASItem(NewItem);

	if (UASArmor* OldArmor = (OldItem.IsValid() ? Cast<UASArmor>(OldItem) : nullptr))
	{
		OldArmor->OnChangedDurability.RemoveAll(this);
	}

	if (UASArmor* NewArmor = (Item.IsValid() ? Cast<UASArmor>(Item) : nullptr))
	{
		NewArmor->OnChangedDurability.AddUObject(this, &UASArmorSlotUserWidget::OnChangedArmorDurability);

		OnChangedArmorDurability(NewArmor->GetCurrentDurability(), NewArmor->GetMaxDurability());
	}
	else
	{
		OnChangedArmorDurability(0.0f, 0.0f);
	}
}

void UASArmorSlotUserWidget::NativeConstruct()
{
	Super::NativeConstruct();

	DurabilityTextBlock = Cast<UTextBlock>(GetWidgetFromName(TEXT("DurabilityTextBlock")));
}

void UASArmorSlotUserWidget::NativeDestruct()
{
	Super::NativeDestruct();

	if (UASArmor* Armor = (Item.IsValid() ? Cast<UASArmor>(Item) : nullptr))
	{
		Armor->OnChangedDurability.RemoveAll(this);
	}
}

bool UASArmorSlotUserWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	if (Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation))
		return true;

	if (GetOperationParentWidget(InOperation) != this)
	{
		TWeakObjectPtr<UASItem> ArmorItem = GetASItemFromDragDropOperation(InOperation);
		if (IsSuitableSlot(ArmorItem))
		{
			auto ASChar = Cast<AASCharacter>(GetOwningPlayerPawn());
			if (ASChar != nullptr)
			{
				ASChar->PickUpArmor(ArmorSlotType, Cast<UASArmor>(ArmorItem));
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

bool UASArmorSlotUserWidget::IsSuitableSlot(const TWeakObjectPtr<UASItem>& InItem)
{
	if (!Super::IsSuitableSlot(InItem))
		return false;

	auto Armor = Cast<UASArmor>(InItem);
	if (Armor == nullptr)
		return false;

	if (!UASInventoryComponent::IsSuitableArmorSlot(ArmorSlotType, Armor))
		return false;

	return true;
}

void UASArmorSlotUserWidget::OnChangedArmorDurability(float Durability, float MaxDurability)
{
	if (DurabilityTextBlock != nullptr)
	{
		DurabilityTextBlock->SetText(FText::FromString(FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Durability), FMath::CeilToInt(MaxDurability))));
	}
}
