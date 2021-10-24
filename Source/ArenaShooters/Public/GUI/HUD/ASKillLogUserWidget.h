// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ArenaShooters.h"
#include "Blueprint/UserWidget.h"
#include "Common/ASEnums.h"
#include "ASKillLogUserWidget.generated.h"

class UScrollBox;
class UASKillLogSlotUserWidget;

UCLASS()
class ARENASHOOTERS_API UASKillLogUserWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void AddLog(const FString& KillerName, const FString& DeadName, EKillLogType KillLogType);
	void RemoveLog(UASKillLogSlotUserWidget* LogWidget);

protected:
	virtual void NativeConstruct() override;

protected:
	UPROPERTY()
	UScrollBox* KillLogScrollBox;

	UPROPERTY(EditDefaultsOnly, Category = "Log Line")
	TSubclassOf<UASKillLogSlotUserWidget> KillLogSlotUserWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "Log Line")
	int32 MaxLineNum;

	UPROPERTY(EditDefaultsOnly, Category = "Log Line")
	int32 MaxNameLen;

	UPROPERTY(EditDefaultsOnly, Category = "Log Line")
	float LogLifeTimeSec;

	UPROPERTY(EditDefaultsOnly, Category = "Log Line")
	FLinearColor OtherKillLogColor;

	UPROPERTY(EditDefaultsOnly, Category = "Log Line")
	FLinearColor MyKillLogColor;

	UPROPERTY(EditDefaultsOnly, Category = "Log Line")
	FLinearColor MyDeathLogColor;

	UPROPERTY()
	TArray<UASKillLogSlotUserWidget*> CachedLogWidgets;
};
