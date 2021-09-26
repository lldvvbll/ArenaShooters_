// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ArenaShooters.h"
#include "Blueprint/DragDropOperation.h"
#include "ASItemDragDropOperation.generated.h"

class UASItem;
class UASDragItemUserWidget;

UCLASS()
class ARENASHOOTERS_API UASItemDragDropOperation : public UDragDropOperation
{
	GENERATED_BODY()

public:
	void SetItemData(const TWeakObjectPtr<UASItem>& InItem, UWidget* InParentWidget, UASDragItemUserWidget* InDragItemUserWidget);

	const TWeakObjectPtr<UASItem>& GetItem() const;
	UWidget* GetParentWidget() const;

private:
	UPROPERTY()
	TWeakObjectPtr<UASItem> Item;

	UPROPERTY()
	UWidget* ParentWidget;
};
