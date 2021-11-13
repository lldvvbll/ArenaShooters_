// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ArenaShooters.h"
#include "Blueprint/UserWidget.h"
#include "ASTimerCaptionUserWidget.generated.h"

class UTextBlock;

UCLASS()
class ARENASHOOTERS_API UASTimerCaptionUserWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void SetInfo(const FText& Caption, const FDateTime& InEndTime, bool bShowTime = true);
	void SetInfoWithEndTime(const FText& Caption, float EndTimeSec, bool bShowTime = true);
	void SetInfoWithDuration(const FText& Caption, float Duration, bool bShowTime = true);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

protected:
	UPROPERTY()
	UTextBlock* CaptionTextBlock;

	UPROPERTY()
	UTextBlock* CountDownTextBlock;

	FDateTime EndTime;
	bool bSetEndTime = false;
};
