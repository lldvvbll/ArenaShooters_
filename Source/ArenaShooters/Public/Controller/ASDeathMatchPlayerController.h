// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ArenaShooters.h"
#include "Controller/ASPlayerController.h"
#include "ASDeathMatchPlayerController.generated.h"

class UASDeathMatchRankingUserWidget;

UCLASS()
class ARENASHOOTERS_API AASDeathMatchPlayerController : public AASPlayerController
{
	GENERATED_BODY()

public:
	AASDeathMatchPlayerController();

	virtual void OnRep_PlayerState() override;

protected:
	virtual void BeginPlay() override;

protected:
	UPROPERTY(EditDefaultsOnly, Category = UI)
	TSubclassOf<UASDeathMatchRankingUserWidget> DeathMatchRankingWidgetClass;

	UPROPERTY()
	UASDeathMatchRankingUserWidget* DeathMatchRankingWidget;

	bool bCreateRankingWidget;
};
