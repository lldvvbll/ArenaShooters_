// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ArenaShooters.h"
#include "Blueprint/UserWidget.h"
#include "ASHudUserWidget.generated.h"

class UASInventoryStatusUserWidget;
class UProgressBar;
class UTextBlock;
class UBorder;
class UASKillDeathCaptionUserWidget;
class AASPlayerState;

UCLASS()
class ARENASHOOTERS_API UASHudUserWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	void OnSetMatchFinishTime(float Time);
	void OnKill(AASPlayerState* KillerPlayerState, AASPlayerState* DeadPlayerState, int32 KillCount);
	void OnChangedCharacterHealth(float NewHealth) const;

protected:
	UPROPERTY()
	UASInventoryStatusUserWidget* InventoryStatusWidget;

	UPROPERTY()
	UBorder* FinishCountDownBorder;

	UPROPERTY()
	UTextBlock* FinishCountDownTextBlock;

	UPROPERTY()
	UASKillDeathCaptionUserWidget* KillDeathCaptionWidget;

	UPROPERTY()
	UProgressBar* HealthProgressBar;

	float MaxCharHealth;

	bool bSetMatchFinishTime;
	FDateTime MatchFinishTime;
};
