// Fill out your copyright notice in the Description page of Project Settings.


#include "GUI/Inventory/ASItemScrollBoxWrapperUserWidget.h"
#include "GUI/Inventory/ASItemDragDropOperation.h"
#include "GUI/Inventory/ASItemUserWidget.h"
#include "GUI/Inventory/ASDragItemUserWidget.h"
#include "Components/ScrollBox.h"
#include "Item/ASItem.h"
#include "Character/ASCharacter.h"
#include "Common/ASStatics.h"

void UASItemScrollBoxWrapperUserWidget::AddItemsToScrollBox(const TArray<TWeakObjectPtr<UASItem>>& Items)
{
	if (ItemScrollBox == nullptr)
	{
		AS_LOG_SCREEN_S(5.0f, FColor::Red);
		return;
	}

	for (auto& Item : Items)
	{
		if (auto ItemWidget = CreateWidget<UASItemUserWidget>(ItemScrollBox, ItemWidgetClass))
		{
			ItemWidget->SetItem(Item);
			CachedItemWidgets.Emplace(ItemWidget);
		}
	}

	CachedItemWidgets.Sort(
		[](const UASItemUserWidget& Left, const UASItemUserWidget& Right)
		{
			const TWeakObjectPtr<UASItem>& LeftItemPtr = Left.GetItem();
			const TWeakObjectPtr<UASItem>& RightItemPtr = Right.GetItem();

			if (LeftItemPtr.IsValid() && RightItemPtr.IsValid())
			{
				return UASStatics::CompareItemForSorting(LeftItemPtr.Get(), RightItemPtr.Get());
			}
			else if (RightItemPtr.IsValid())
			{
				return false;
			}

			return true;
		});

	ItemScrollBox->ClearChildren();
	for (auto& ItemWidget : CachedItemWidgets)
	{
		ItemScrollBox->AddChild(ItemWidget);
	}	
}

void UASItemScrollBoxWrapperUserWidget::RemoveItemsFromScrollBox(const TArray<TWeakObjectPtr<UASItem>>& Items)
{
	if (ItemScrollBox == nullptr)
	{
		AS_LOG_SCREEN_S(5.0f, FColor::Red);
		return;
	}

	int32 ItemWidgetNum = ItemScrollBox->GetChildrenCount();

	TArray<int32> RemoveItemIndices;
	RemoveItemIndices.Reserve(ItemWidgetNum);
	
	for (auto& Item : Items)
	{
		for (int32 Idx = 0; Idx < ItemWidgetNum; ++Idx)
		{
			auto ItemWidget = Cast<UASItemUserWidget>(ItemScrollBox->GetChildAt(Idx));
			if (ItemWidget == nullptr)
				continue;

			if (ItemWidget->HasItem(Item))
			{
				CachedItemWidgets.Remove(ItemWidget);
				RemoveItemIndices.Emplace(Idx);
			}
		}
	}

	for (auto& Idx : RemoveItemIndices)
	{
		ItemScrollBox->RemoveChildAt(Idx);
	}
}

bool UASItemScrollBoxWrapperUserWidget::IsInventoryScrollBoxWrapperWidget() const
{
	return bInventoryScrollBoxWrapper;
}

void UASItemScrollBoxWrapperUserWidget::NativeConstruct()
{
	Super::NativeConstruct();

	ItemScrollBox = Cast<UScrollBox>(GetWidgetFromName(TEXT("ItemScrollBox")));
}

void UASItemScrollBoxWrapperUserWidget::NativeOnDragEnter(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDragEnter(InGeometry, InDragDropEvent, InOperation);

	if (GetOperationParentWidget(InOperation) == ItemScrollBox)
		return;

	if (bInventoryScrollBoxWrapper)
	{
		TWeakObjectPtr<UASItem> Item = GetASItemFromDragDropOperation(InOperation);
		if (Item.IsValid())
		{
			EItemType ItemType = Item->GetItemType();
			if (ItemType == EItemType::Weapon || ItemType == EItemType::Armor)
			{
				if (auto DraggedItemWidget = Cast<UASDragItemUserWidget>(InOperation->DefaultDragVisual))
				{
					DraggedItemWidget->SetSuitableBrush(false);
				}
			}
		}
	}
}

void UASItemScrollBoxWrapperUserWidget::NativeOnDragLeave(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDragLeave(InDragDropEvent, InOperation);

	if (GetOperationParentWidget(InOperation) == ItemScrollBox)
		return;

	if (bInventoryScrollBoxWrapper)
	{
		TWeakObjectPtr<UASItem> Item = GetASItemFromDragDropOperation(InOperation);
		if (Item.IsValid())
		{
			EItemType ItemType = Item->GetItemType();
			if (ItemType == EItemType::Weapon || ItemType == EItemType::Armor)
			{
				if (auto DraggedItemWidget = Cast<UASDragItemUserWidget>(InOperation->DefaultDragVisual))
				{
					DraggedItemWidget->SetSuitableBrush(true);
				}
			}
		}
	}
}

bool UASItemScrollBoxWrapperUserWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	if (Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation))
		return true;

	TWeakObjectPtr<UASItem> Item = GetASItemFromDragDropOperation(InOperation);
	if (Item.IsValid())
	{
		if (bInventoryScrollBoxWrapper)
		{
			if (GetOperationParentWidget(InOperation) != ItemScrollBox)
			{
				EItemType ItemType = Item->GetItemType();
				if (ItemType != EItemType::Weapon && ItemType != EItemType::Armor)
				{
					if (auto ASChar = Cast<AASCharacter>(GetOwningPlayerPawn()))
					{
						ASChar->PickUpInventoryItem(Item.Get());
						return true;
					}
				}
			}
		}
		else
		{
			if (GetOperationParentWidget(InOperation) != ItemScrollBox)
			{
				if (auto ASChar = Cast<AASCharacter>(GetOwningPlayerPawn()))
				{
					ASChar->DropItem(Item.Get());
				}
			}

			return true;
		}
	}
	else
	{
		AS_LOG_S(Error);
	}

	return false;
}

TWeakObjectPtr<UASItem> UASItemScrollBoxWrapperUserWidget::GetASItemFromDragDropOperation(UDragDropOperation* InOperation)
{
	if (InOperation == nullptr)
	{
		AS_LOG_SCREEN_S(5.0f, FColor::Red);
		return TWeakObjectPtr<UASItem>();
	}

	auto DragDropOp = Cast<UASItemDragDropOperation>(InOperation);
	if (DragDropOp == nullptr)
	{
		AS_LOG_SCREEN_S(5.0f, FColor::Red);
		return TWeakObjectPtr<UASItem>();
	}

	return DragDropOp->GetItem();
}

UWidget* UASItemScrollBoxWrapperUserWidget::GetOperationParentWidget(UDragDropOperation* InOperation)
{
	if (InOperation == nullptr)
	{
		AS_LOG_SCREEN_S(5.0f, FColor::Red);
		return nullptr;
	}

	auto DragDropOp = Cast<UASItemDragDropOperation>(InOperation);
	if (DragDropOp == nullptr)
	{
		AS_LOG_SCREEN_S(5.0f, FColor::Red);
		return nullptr;
	}

	return DragDropOp->GetParentWidget();
}
