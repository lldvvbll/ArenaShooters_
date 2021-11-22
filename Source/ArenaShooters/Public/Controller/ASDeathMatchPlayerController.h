// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ArenaShooters.h"
#include "Controller/ASPlayerController.h"
#include "ASDeathMatchPlayerController.generated.h"

class UASDeathMatchRankingUserWidget;
class UASDmLeaderBoardUserWidget;
class AASPlayerState;

UCLASS()
class ARENASHOOTERS_API AASDeathMatchPlayerController : public AASPlayerController
{
	GENERATED_BODY()

public:
	AASDeathMatchPlayerController();

	virtual void PostInitializeComponents() override;
	virtual void OnRep_PlayerState() override;

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

	virtual void OnChangedInnerMatchState(EInnerMatchState State) override;

	void ShowLeaderBoardWidget();
	void CreateRankingWidget();

	void OnChangedDeathCount(int32 NewDeathCount);
	void OnSetGameStateToWorld(AGameStateBase* NewGameState);

protected:
	UPROPERTY(EditDefaultsOnly, Category = UI)
	TSubclassOf<UASDeathMatchRankingUserWidget> DeathMatchRankingWidgetClass;

	UPROPERTY()
	UASDeathMatchRankingUserWidget* DeathMatchRankingWidget;

	bool bCreateRankingWidget;

	UPROPERTY(EditDefaultsOnly, Category = UI)
	TSubclassOf<UASDmLeaderBoardUserWidget> DmLeaderBoardWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = UI, Meta = (MultiLine = "true"))
	FText WelcomeMessage;

	UPROPERTY(EditDefaultsOnly, Category = UI)
	FText ItemSetChangeButtonNotification;
};
