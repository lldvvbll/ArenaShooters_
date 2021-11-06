// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ArenaShooters.h"
#include "Blueprint/UserWidget.h"
#include "ASItemUserWidget.generated.h"

class UASItem;
class UASItemScrollBoxWrapperUserWidget;
class UASDragItemUserWidget;
class UASItem;
class UImage;
class UTextBlock;

UCLASS()
class ARENASHOOTERS_API UASItemUserWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	const TWeakObjectPtr<UASItem>& GetItem() const;
	void SetItem(const TWeakObjectPtr<UASItem>& NewItem);
	bool HasItem(const TWeakObjectPtr<UASItem>& InItem) const;

protected:
	virtual void NativeConstruct() override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;

	void OnChangeItemCount(UASItem* InItem);

private:
	UPROPERTY()
	UASItemScrollBoxWrapperUserWidget* ItemScrollBoxWrapperWidget;

	UPROPERTY(EditAnywhere, Meta = (AllowPrivateAccess = true))
	TSubclassOf<UASDragItemUserWidget> DragItemWidgetClass;

	UPROPERTY()
	UASDragItemUserWidget* DraggedItemWidget;

	UPROPERTY()
	UImage* ItemImage;

	UPROPERTY()
	UTextBlock* NameTextBlock;

	UPROPERTY()
	UTextBlock* CountTextBlock;

	UPROPERTY()
	TWeakObjectPtr<UASItem> Item;
};
