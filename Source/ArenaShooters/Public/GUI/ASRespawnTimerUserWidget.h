// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ArenaShooters.h"
#include "Blueprint/UserWidget.h"
#include "ASRespawnTimerUserWidget.generated.h"

class UTextBlock;

UCLASS()
class ARENASHOOTERS_API UASRespawnTimerUserWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void SetEndTime(const FDateTime& Time);
	void SetEndTimeByServerWorldTime(float TimeSec);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

protected:
	UPROPERTY()
	UTextBlock* CountDownTextBlock;

	FDateTime EndTime;
	bool bSetEndTime = false;
};
