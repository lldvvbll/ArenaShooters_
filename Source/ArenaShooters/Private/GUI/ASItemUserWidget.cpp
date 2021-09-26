// Fill out your copyright notice in the Description page of Project Settings.


#include "GUI/ASItemUserWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "GUI/ASDragItemUserWidget.h"
#include "GUI/ASItemDragDropOperation.h"
#include "GUI/ASItemScrollBoxWrapperUserWidget.h"
#include "Item/ASItem.h"
#include "Item/ASArmor.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/PanelWidget.h"
#include "Common/ASEnums.h"

void UASItemUserWidget::SetItem(const TWeakObjectPtr<UASItem>& NewItem)
{
	Item = NewItem;

	if (!Item.IsValid())
	{
		AS_LOG_SCREEN_S(5.0f, FColor::Red);
		return;
	}

	Item->OnChangeCount.AddUObject(this, &UASItemUserWidget::OnChangeItemCount);

	if (ItemImage != nullptr)
	{
		ItemImage->SetBrushFromTexture(Item->GetItemImage());
	}

	if (NameTextBlock != nullptr)
	{
		const FText& ItemName = Item->GetItemName();

		NameTextBlock->SetText(ItemName);
		if (ItemName.ToString().Len() > 15)
		{
			NameTextBlock->Font.Size = 15;
		}
	}

	if (CountTextBlock != nullptr)
	{
		switch (Item->GetItemType())
		{
		case EItemType::Weapon:
			{
				CountTextBlock->SetVisibility(ESlateVisibility::Hidden);
			}
			break;
		case EItemType::Armor:
			{
				const TWeakObjectPtr<UASArmor>& Armor = Cast<UASArmor>(Item);
				if (Armor.IsValid())
				{
					CountTextBlock->SetText(FText::FromString(FString::FromInt(Armor->GetCurrentDurability())));
				}
			}
			break;
		case EItemType::Ammo:	// fallthough
		case EItemType::HealingKit:
			{
				CountTextBlock->SetText(FText::FromString(FString::FromInt(Item->GetCount())));
			}
			break;
		default:
			checkNoEntry();
			break;
		}
	}
}

bool UASItemUserWidget::HasItem(const TWeakObjectPtr<UASItem>& InItem) const
{
	return Item == InItem;
}

void UASItemUserWidget::NativeConstruct()
{
	Super::NativeConstruct();

	ItemImage = Cast<UImage>(GetWidgetFromName(TEXT("ItemImage")));
	NameTextBlock = Cast<UTextBlock>(GetWidgetFromName(TEXT("NameTextBlock")));
	CountTextBlock = Cast<UTextBlock>(GetWidgetFromName(TEXT("CountTextBlock")));
}

FReply UASItemUserWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	FReply Reply = Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
	if (Reply.IsEventHandled())
		return Reply;

	return UWidgetBlueprintLibrary::DetectDragIfPressed(InMouseEvent, this, EKeys::LeftMouseButton).NativeReply;
}

void UASItemUserWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);

	DraggedItemWidget = CreateWidget<UASDragItemUserWidget>(this, (DragItemWidgetClass != nullptr ? DragItemWidgetClass : UASDragItemUserWidget::StaticClass()));
	if (DraggedItemWidget != nullptr && Item.IsValid())
	{
		DraggedItemWidget->SetItemImage(Item->GetItemImage());
	}

	if (auto ItemDragDropOp = NewObject<UASItemDragDropOperation>(GetTransientPackage(), UASItemDragDropOperation::StaticClass()))
	{
		ItemDragDropOp->SetItemData(Item, GetParent(), DraggedItemWidget);
		OutOperation = ItemDragDropOp;
	}
}

void UASItemUserWidget::OnChangeItemCount(int32 NewCount)
{
	if (CountTextBlock != nullptr)
	{
		CountTextBlock->SetText(FText::FromString(FString::FromInt(Item->GetCount())));
	}
}
