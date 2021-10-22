// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ArenaShooters.h"
#include "Blueprint/UserWidget.h"
#include "Common/ASStructs.h"
#include "ASDmLeaderBoardUserWidget.generated.h"

class UTextBlock;
class UScrollBox;
class UASDmLeaderBoardSlotUserWidget;
class AASPlayerState;

UCLASS()
class ARENASHOOTERS_API UASDmLeaderBoardUserWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void ChangeToMatchResultWidget();

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	
	UFUNCTION()
	void BackToGame();

	void UpdateRanking(const TArray<FRankedPlayerState>& RankedPlayerStates);

public:
	DECLARE_EVENT_OneParam(UASDmLeaderBoardUserWidget, FOnConstructedEvent, UUserWidget*);
	FOnConstructedEvent OnConstructed;

	DECLARE_EVENT_OneParam(UASDmLeaderBoardUserWidget, FOnDestructedEvent, UUserWidget*);
	FOnDestructedEvent OnDestructed;

protected:
	UPROPERTY()
	UTextBlock* TitleTextBlock;

	UPROPERTY()
	UTextBlock* GoalNumOfKillsTextBlock;

	UPROPERTY()
	UScrollBox* RankingScrollBox;

	UPROPERTY(EditDefaultsOnly, Category = Slot)
	TSubclassOf<UASDmLeaderBoardSlotUserWidget> DmLeaderBoardSlotWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "Match Result")
	FLinearColor MatchResultTitleColor;
};
