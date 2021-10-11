// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ArenaShooters.h"
#include "Blueprint/UserWidget.h"
#include "ASDeathMatchRankingUserWidget.generated.h"

class UASDmRankingSlotUserWidget;
class AASPlayerState;
class UTextBlock;
class UScrollBox;

UCLASS()
class ARENASHOOTERS_API UASDeathMatchRankingUserWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;

	void OnKill(AASPlayerState* KillerPlayerState, AASPlayerState* DeadPlayerState);
	void OnChangedNumPlayers(int32 PlayerNum);

	void UpdatePlayerRanking();

protected:
	UPROPERTY(EditDefaultsOnly, Category = Slot)
	TSubclassOf<UASDmRankingSlotUserWidget> DmRankingSlotWidgetClass;

	UPROPERTY()
	UASDmRankingSlotUserWidget* DmRankingSlotWidget;

	UPROPERTY()
	UTextBlock* GoalNumOfKillsTextBlock;

	UPROPERTY()
	UScrollBox* RankScrollBox;
};
