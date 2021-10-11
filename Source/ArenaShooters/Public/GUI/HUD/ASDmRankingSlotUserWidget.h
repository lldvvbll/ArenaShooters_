// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ArenaShooters.h"
#include "Blueprint/UserWidget.h"
#include "ASDmRankingSlotUserWidget.generated.h"

class AASPlayerState;
class UTextBlock;

UCLASS()
class ARENASHOOTERS_API UASDmRankingSlotUserWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetPlayerInfo(int32 Rank, AASPlayerState* PlayerState);

protected:
	virtual void NativeConstruct() override;

protected:
	UPROPERTY()
	UTextBlock* RankTextBlock;

	UPROPERTY()
	UTextBlock* NameTextBlock;

	UPROPERTY()
	UTextBlock* KillCountTextBlock;

	UPROPERTY(EditDefaultsOnly, Category = TextColor)
	FLinearColor MyPlayerStateTextColor;

	UPROPERTY(EditDefaultsOnly, Category = TextColor)
	FLinearColor OtherPlayerStateTextColor;
};
