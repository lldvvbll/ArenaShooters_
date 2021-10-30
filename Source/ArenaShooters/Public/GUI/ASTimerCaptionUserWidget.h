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
	void SetInfo(const FText& Caption, const FDateTime& Time);
	void SetInfo(const FText& Caption, float TimeSec);

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
