// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ArenaShooters.h"
#include "Blueprint/UserWidget.h"
#include "ASMatchItemSetSelectUserWidget.generated.h"

class UASMatchItemSetSlotUserWidget;
class UScrollBox;
class UASItemSetDataAsset;

UCLASS()
class ARENASHOOTERS_API UASMatchItemSetSelectUserWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	void OnClickedSlot(UASMatchItemSetSlotUserWidget* ClickedSlot);

protected:
	UPROPERTY()
	UScrollBox* SlotsScrollBox;

	UPROPERTY(EditDefaultsOnly, Category = Slot)
	TSubclassOf<UASMatchItemSetSlotUserWidget> MatchItemSetSlotWidgetClass;
};
