// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ArenaShooters.h"
#include "Blueprint/UserWidget.h"
#include "ASMatchItemSetSlotUserWidget.generated.h"

class UASMatchItemSetSelectUserWidget;
class UButton;
class UImage;
class UTextBlock;
class UASMatchItemSetDataAsset;

UCLASS()
class ARENASHOOTERS_API UASMatchItemSetSlotUserWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetDataAsset(UASMatchItemSetDataAsset* DataAsset);
	FPrimaryAssetId GetDataAssetId() const;

	void ChangeButtonState(bool bIsSelected);

protected:
	virtual void NativeConstruct() override;

	UFUNCTION()
	void OnClickedButton();

public:
	DECLARE_EVENT_OneParam(UASMatchItemSetSlotUserWidget, FOnSelectedEvent, UASMatchItemSetSlotUserWidget*);
	FOnSelectedEvent OnSelected;

protected:
	UPROPERTY()
	UButton* SelectButton;

	UPROPERTY()
	UImage* ThumbnailImage;

	UPROPERTY()
	UTextBlock* NameTextBlock;

	FPrimaryAssetId DataAssetId;

	bool bSelected = false;

	UPROPERTY(EditDefaultsOnly)
	FLinearColor SelectedButtonColor;

	UPROPERTY(EditDefaultsOnly)
	FLinearColor UnselectedButtonColor;
};
