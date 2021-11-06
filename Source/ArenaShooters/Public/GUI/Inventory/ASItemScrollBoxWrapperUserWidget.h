// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ArenaShooters.h"
#include "Blueprint/UserWidget.h"
#include "ASItemScrollBoxWrapperUserWidget.generated.h"

class UScrollBox;
class UASItem;
class UASItemUserWidget;

UCLASS()
class ARENASHOOTERS_API UASItemScrollBoxWrapperUserWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void AddItemsToScrollBox(const TArray<TWeakObjectPtr<UASItem>>& Items);
	void RemoveItemsFromScrollBox(const TArray<TWeakObjectPtr<UASItem>>& Items);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeOnDragEnter(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual void NativeOnDragLeave(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

	TWeakObjectPtr<UASItem> GetASItemFromDragDropOperation(UDragDropOperation* InOperation);
	UWidget* GetOperationParentWidget(UDragDropOperation* InOperation);

private:
	UPROPERTY(EditInstanceOnly, Meta = (AllowPrivateAccess = true))
	bool bInventoryScrollBoxWrapper;

	UPROPERTY()
	UScrollBox* ItemScrollBox;

	UPROPERTY(EditDefaultsOnly, Meta = (AllowPrivateAccess = true))
	TSubclassOf<UASItemUserWidget> ItemWidgetClass;

	UPROPERTY()
	TArray<UASItemUserWidget*> CachedItemWidgets;
};
