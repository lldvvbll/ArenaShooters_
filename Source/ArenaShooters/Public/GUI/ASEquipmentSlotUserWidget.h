// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ArenaShooters.h"
#include "Blueprint/UserWidget.h"
#include "Common/ASEnums.h"
#include "ASEquipmentSlotUserWidget.generated.h"

class UBorder;
class UImage;
class UASItem;
class UTextBlock;
class UASDragItemUserWidget;

UCLASS(Abstract)
class ARENASHOOTERS_API UASEquipmentSlotUserWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void SetASItem(TWeakObjectPtr<UASItem>& Item);

protected:
	virtual void NativeConstruct() override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
	virtual void NativeOnDragEnter(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual void NativeOnDragLeave(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

	void Highlight(bool bOn);

	virtual bool IsSuitableSlot(const TWeakObjectPtr<UASItem>& InItem);

	TWeakObjectPtr<UASItem> GetASItemFromDragDropOperation(UDragDropOperation* InOperation);
	UWidget* GetOperationParentWidget(UDragDropOperation* InOperation);

protected:
	UPROPERTY()
	UBorder* HighlightBorder;

	UPROPERTY()
	UImage* EquipmentImage;

	UPROPERTY()
	UTextBlock* NameTextBlock;

	UPROPERTY(EditAnywhere)
	EItemType ItemType;

	UPROPERTY(EditAnywhere, Meta = (AllowPrivateAccess = true))
	TSubclassOf<UASDragItemUserWidget> DragItemWidgetClass;

	UPROPERTY()
	UASDragItemUserWidget* DraggedItemWidget;

	UPROPERTY()
	TWeakObjectPtr<UASItem> Item;
};
