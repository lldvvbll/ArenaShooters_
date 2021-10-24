// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ArenaShooters.h"
#include "Blueprint/UserWidget.h"
#include "ASKillLogSlotUserWidget.generated.h"

class UTextBlock;
class UASKillLogUserWidget;

UCLASS()
class ARENASHOOTERS_API UASKillLogSlotUserWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void SetInfo(UASKillLogUserWidget* LogWidget, const FString& KillerName, const FString& DeadName, int32 MaxNameLen, float LifeTimeSec, FLinearColor LogColor);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	void RemoveLog();

protected:
	UPROPERTY()
	UTextBlock* KillerNameTextBlock;

	UPROPERTY()
	UTextBlock* DeadNameTextBlock;

	UPROPERTY()
	UTextBlock* ArrowTextBlock;

	FTimerHandle LifeEndTimerHandle;

	UPROPERTY()
	TWeakObjectPtr<UASKillLogUserWidget> LogWidgetPtr;
};
