// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ArenaShooters.h"
#include "Blueprint/UserWidget.h"
#include "ASHudUserWidget.generated.h"

class UASInventoryStatusUserWidget;
class UProgressBar;
class UTextBlock;
class UBorder;

UCLASS()
class ARENASHOOTERS_API UASHudUserWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	void OnSetMatchFinishTime(float Time);
	void OnChangedCharacterHealth(float NewHealth) const;

protected:
	UPROPERTY()
	UASInventoryStatusUserWidget* InventoryStatusWidget;

	UPROPERTY()
	UBorder* FinishCountDownBorder;

	UPROPERTY()
	UTextBlock* FinishCountDownTextBlock;

	UPROPERTY()
	UProgressBar* HealthProgressBar;

	float MaxCharHealth;

	bool bSetMatchFinishTime;
	FDateTime MatchFinishTime;
};
