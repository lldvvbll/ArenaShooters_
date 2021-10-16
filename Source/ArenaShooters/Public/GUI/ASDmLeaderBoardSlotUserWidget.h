// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ArenaShooters.h"
#include "Blueprint/UserWidget.h"
#include "ASDmLeaderBoardSlotUserWidget.generated.h"

class UTextBlock;
class AASPlayerState;

UCLASS()
class ARENASHOOTERS_API UASDmLeaderBoardSlotUserWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetInfo(int32 Ranking, AASPlayerState* PlayerState);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	void OnChangedPlayerName(FString NewName);
	void OnChangedPlayerKillCount(int32 NewCount);
	void OnChangedPlayerDeathCount(int32 NewCount);

protected:
	UPROPERTY()
	UTextBlock* RankingTextBlock;

	UPROPERTY()
	UTextBlock* NameTextBlock;

	UPROPERTY()
	UTextBlock* KillCountTextBlock;

	UPROPERTY()
	UTextBlock* DeathCountTextBlock;

	UPROPERTY(EditDefaultsOnly, Category = TextColor)
	FLinearColor MyPlayerStateTextColor;

	UPROPERTY(EditDefaultsOnly, Category = TextColor)
	FLinearColor OtherPlayerStateTextColor;

	UPROPERTY()
	TWeakObjectPtr<AASPlayerState> PlayerStatePtr;
};
