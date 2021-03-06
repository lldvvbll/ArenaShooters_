// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ArenaShooters.h"
#include "Blueprint/UserWidget.h"
#include "ASPrepareInfoUserWidget.generated.h"

class UTextBlock;

UCLASS()
class ARENASHOOTERS_API UASPrepareInfoUserWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetMaxNumPlayers(int32 Num);
	void SetNumPlayers(int32 Num);
	void StartCountDown(float InMatchStartTime);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	void OnAddedPlayerState(APlayerState* AddedPlayerState);
	void OnRemovedPlayerState(APlayerState* AddedPlayerState);

protected:
	UPROPERTY()
	UTextBlock* MaxNumPlayersTextBlock;

	UPROPERTY()
	UTextBlock* NumPlayersTextBlock;

	UPROPERTY()
	UTextBlock*	MatchStartInTextBlock;

	UPROPERTY()
	UTextBlock* CountDownTextBlock;

	bool bCountDown;
	FDateTime MatchStartTime;
};
