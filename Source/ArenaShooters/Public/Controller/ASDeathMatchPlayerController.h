// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ArenaShooters.h"
#include "Controller/ASPlayerController.h"
#include "ASDeathMatchPlayerController.generated.h"

class UASDeathMatchRankingUserWidget;
class UASDmLeaderBoardUserWidget;

UCLASS()
class ARENASHOOTERS_API AASDeathMatchPlayerController : public AASPlayerController
{
	GENERATED_BODY()

public:
	AASDeathMatchPlayerController();

	virtual void OnRep_PlayerState() override;

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

	virtual void OnChangedInnerMatchState(EInnerMatchState State) override;
	
	void ShowLeaderBoardWidget();
	void ShowMatchResultWidget();

protected:
	UPROPERTY(EditDefaultsOnly, Category = UI)
	TSubclassOf<UASDeathMatchRankingUserWidget> DeathMatchRankingWidgetClass;

	UPROPERTY()
	UASDeathMatchRankingUserWidget* DeathMatchRankingWidget;

	bool bCreateRankingWidget;

	UPROPERTY(EditDefaultsOnly, Category = UI)
	TSubclassOf<UASDmLeaderBoardUserWidget> DmLeaderBoardWidgetClass;
};
