// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ArenaShooters.h"
#include "GameMode/ASMatchGameStateBase.h"
#include "ASDeathmatchGameState.generated.h"

class UASItemFactoryComponent;
class AASPlayerState;

UCLASS()
class ARENASHOOTERS_API AASDeathmatchGameState : public AASMatchGameStateBase
{
	GENERATED_BODY()
	
public:
	AASDeathmatchGameState();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void OnFinishMatch() override;

	void SetWinner(AASPlayerState* WonPlayerState);

protected:
	UFUNCTION()
	void OnRep_WonPlayerState();

public:
	DECLARE_EVENT_OneParam(AASMatchGameStateBase, FOnSetWinnerEvent, AASPlayerState*);
	FOnSetWinnerEvent OnSetWinner;

protected:
	UPROPERTY(Replicated)
	AASPlayerState* WonPlayerState;
};
