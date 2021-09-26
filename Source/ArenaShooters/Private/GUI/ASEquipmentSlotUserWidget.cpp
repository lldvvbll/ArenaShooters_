// Fill out your copyright notice in the Description page of Project Settings.


#include "GUI/ASEquipmentSlotUserWidget.h"
#include "GUI/ASItemDragDropOperation.h"
#include "GUI/ASDragItemUserWidget.h"
#include "Components/Border.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Item/ASItem.h"
#include "Blueprint/WidgetBlueprintLibrary.h"

void UASEquipmentSlotUserWidget::SetASItem(TWeakObjectPtr<UASItem>& NewItem)
{
	Item = NewItem;

	if (EquipmentImage != nullptr)
	{
		if (Item.IsValid())
		{
			EquipmentImage->SetBrushFromTexture(Item->GetEquipmentSlotImage());
			EquipmentImage->SetOpacity(1.0f);
		}
		else
		{
			EquipmentImage->SetBrushFromTexture(nullptr);
			EquipmentImage->SetOpacity(0.0f);
		}		
	}

	if (NameTextBlock != nullptr)
	{
		NameTextBlock->SetText((Item.IsValid() ? Item->GetItemName() : FText::GetEmpty()));
	}
}

void UASEquipmentSlotUserWidget::NativeConstruct()
{
	Super::NativeConstruct();

	HighlightBorder = Cast<UBorder>(GetWidgetFromName(TEXT("HighlightBorder")));
	EquipmentImage = Cast<UImage>(GetWidgetFromName(TEXT("EquipmentImage")));
	NameTextBlock = Cast<UTextBlock>(GetWidgetFromName(TEXT("NameTextBlock")));

	Highlight(false);
}

FReply UASEquipmentSlotUserWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	FReply Reply = Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
	if (Reply.IsEventHandled())
		return Reply;

	return UWidgetBlueprintLibrary::DetectDragIfPressed(InMouseEvent, this, EKeys::LeftMouseButton).NativeReply;
}

void UASEquipmentSlotUserWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);

	if (!Item.IsValid())
		return;

	DraggedItemWidget = CreateWidget<UASDragItemUserWidget>(this, (DragItemWidgetClass != nullptr ? DragItemWidgetClass : UASDragItemUserWidget::StaticClass()));
	if (DraggedItemWidget != nullptr && Item.IsValid())
	{
		DraggedItemWidget->SetItemImage(Item->GetItemImage());
	}

	if (auto ItemDragDropOp = NewObject<UASItemDragDropOperation>(GetTransientPackage(), UASItemDragDropOperation::StaticClass()))
	{
		ItemDragDropOp->SetItemData(Item, this, DraggedItemWidget);
		OutOperation = ItemDragDropOp;
	}
}

void UASEquipmentSlotUserWidget::NativeOnDragEnter(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDragEnter(InGeometry, InDragDropEvent, InOperation);
	
	if (GetOperationParentWidget(InOperation) == this)
		return;

	if (IsSuitableSlot(GetASItemFromDragDropOperation(InOperation)))
	{
		Highlight(true);
	}
	else
	{
		if (auto NewDraggedItemWidget = Cast<UASDragItemUserWidget>(InOperation->DefaultDragVisual))
		{
			NewDraggedItemWidget->SetSuitableBrush(false);
		}
	}
}

void UASEquipmentSlotUserWidget::NativeOnDragLeave(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDragLeave(InDragDropEvent, InOperation);

	if (GetOperationParentWidget(InOperation) == this)
		return;

	Highlight(false);

	if (auto NewDraggedItemWidget = Cast<UASDragItemUserWidget>(InOperation->DefaultDragVisual))
	{
		NewDraggedItemWidget->SetSuitableBrush(true);
	}
}

void UASEquipmentSlotUserWidget::Highlight(bool bOn)
{
	if (HighlightBorder != nullptr)
	{
		ESlateVisibility NewVisibility = (bOn ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden);
		if (HighlightBorder->GetVisibility() != NewVisibility)
		{
			HighlightBorder->SetVisibility(NewVisibility);

		}
	}
}

bool UASEquipmentSlotUserWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	if (Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation))
		return true;

	Highlight(false);

	if (GetOperationParentWidget(InOperation) == this)
		return true;

	return false;
}

bool UASEquipmentSlotUserWidget::IsSuitableSlot(const TWeakObjectPtr<UASItem>& InItem)
{
	if (!InItem.IsValid())
	{
		AS_LOG_SCREEN_S(5.0f, FColor::Red);
		return false;
	}

	if (InItem->GetItemType() != ItemType)
		return false;

	return true;
}

TWeakObjectPtr<UASItem> UASEquipmentSlotUserWidget::GetASItemFromDragDropOperation(UDragDropOperation* InOperation)
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

UWidget* UASEquipmentSlotUserWidget::GetOperationParentWidget(UDragDropOperation* InOperation)
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
