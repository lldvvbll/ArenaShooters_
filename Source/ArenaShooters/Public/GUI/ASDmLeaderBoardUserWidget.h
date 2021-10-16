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

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	
	UFUNCTION()
	void BackToGame();

	void UpdateRanking(const TArray<FRankedPlayerState>& RankedPlayerStates);	

public:
	DECLARE_EVENT_OneParam(UASGameMenuUserWidget, FOnConstructedEvent, UUserWidget*);
	FOnConstructedEvent OnConstructed;

	DECLARE_EVENT_OneParam(UASGameMenuUserWidget, FOnDestructedEvent, UUserWidget*);
	FOnDestructedEvent OnDestructed;

protected:
	UPROPERTY()
	UTextBlock* GoalNumOfKillsTextBlock;

	UPROPERTY()
	UScrollBox* RankingScrollBox;

	UPROPERTY(EditDefaultsOnly, Category = Slot)
	TSubclassOf<UASDmLeaderBoardSlotUserWidget> DmLeaderBoardSlotWidgetClass;
};
