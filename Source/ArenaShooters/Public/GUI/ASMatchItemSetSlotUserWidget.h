// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ArenaShooters.h"
#include "Blueprint/UserWidget.h"
#include "ASMatchItemSetSlotUserWidget.generated.h"

class UASMatchItemSetSelectUserWidget;
class UButton;
class UImage;
class UTextBlock;
class UASItemSetDataAsset;

UCLASS()
class ARENASHOOTERS_API UASMatchItemSetSlotUserWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetDataAsset(UASItemSetDataAsset* DataAsset);
	FPrimaryAssetId GetItemSetDataAssetId() const;

	void ChangeButtonState(bool bIsSelected);

protected:
	virtual void NativeConstruct() override;

	UFUNCTION()
	void OnClickedButton();

public:
	DECLARE_EVENT_OneParam(UASMatchItemSetSlotUserWidget, FOnClickedSlotEvent, UASMatchItemSetSlotUserWidget*);
	FOnClickedSlotEvent OnClickedSlot;

protected:
	UPROPERTY()
	UButton* SelectButton;

	UPROPERTY()
	UImage* ThumbnailImage;

	UPROPERTY()
	UTextBlock* NameTextBlock;

	UPROPERTY()
	UASItemSetDataAsset* ItemSetDataAsset;

	bool bSelected = false;

	UPROPERTY(EditDefaultsOnly)
	FLinearColor SelectedButtonColor;

	UPROPERTY(EditDefaultsOnly)
	FLinearColor UnselectedButtonColor;
};
