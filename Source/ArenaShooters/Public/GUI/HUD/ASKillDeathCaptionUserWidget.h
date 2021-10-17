// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ArenaShooters.h"
#include "Blueprint/UserWidget.h"
#include "ASKillDeathCaptionUserWidget.generated.h"

class UTextBlock;

UCLASS()
class ARENASHOOTERS_API UASKillDeathCaptionUserWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void ShowKillCaption(const FString& DeadPlayerName, int32 KillCount);
	void ShowDeadCaption(const FString& KillerPlayerName);

protected:
	virtual void NativeConstruct() override;

	void SetTimer();
	void Hide();

protected:
	UPROPERTY()
	UTextBlock* KillDeathCaptionTextBlock;

	UPROPERTY()
	UTextBlock* KillCountTextBlock;

	UPROPERTY(EditDefaultsOnly, Category = Timer)
	float DurationSec;
};
