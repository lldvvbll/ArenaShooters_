// Fill out your copyright notice in the Description page of Project Settings.


#include "GUI/Inventory/ASArmorSlotUserWidget.h"
#include "Components/Border.h"
#include "Components/TextBlock.h"
#include "Item/ASArmor.h"
#include "Character/ASInventoryComponent.h"
#include "Character/ASCharacter.h"

void UASArmorSlotUserWidget::SetASItem(TWeakObjectPtr<UASItem>& NewItem)
{
	Super::SetASItem(NewItem);

	if (DurabilityTextBlock != nullptr)
	{
		UASArmor* Armor = (Item.IsValid() ? Cast<UASArmor>(Item) : nullptr);

		if (Armor != nullptr)
		{
			DurabilityTextBlock->SetText(FText::AsNumber(Armor->GetCurrentDurability()));
		}
		else
		{
			DurabilityTextBlock->SetText(FText::GetEmpty());
		}		
	}
}

void UASArmorSlotUserWidget::NativeConstruct()
{
	Super::NativeConstruct();

	DurabilityTextBlock = Cast<UTextBlock>(GetWidgetFromName(TEXT("DurabilityTextBlock")));
}

bool UASArmorSlotUserWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	if (Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation))
		return true;

	if (GetOperationParentWidget(InOperation) != nullptr)
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
