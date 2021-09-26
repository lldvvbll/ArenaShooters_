// Fill out your copyright notice in the Description page of Project Settings.


#include "GUI/ASItemDragDropOperation.h"
#include "GUI/ASDragItemUserWidget.h"

void UASItemDragDropOperation::SetItemData(const TWeakObjectPtr<UASItem>& InItem, UWidget* InParentWidget, UASDragItemUserWidget* InDragItemUserWidget)
{
	Item = InItem;
	ParentWidget = InParentWidget;
	DefaultDragVisual = InDragItemUserWidget;
	Pivot = EDragPivot::CenterCenter;
}

const TWeakObjectPtr<UASItem>& UASItemDragDropOperation::GetItem() const
{
	return Item;
}

UWidget* UASItemDragDropOperation::GetParentWidget() const
{
	return ParentWidget;
}
