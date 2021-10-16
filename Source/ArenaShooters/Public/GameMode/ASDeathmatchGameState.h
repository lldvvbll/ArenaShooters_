// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ArenaShooters.h"
#include "GameMode/ASMatchGameStateBase.h"
#include "Common/ASStructs.h"
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

	virtual void AddPlayerState(APlayerState* PlayerState) override;
	virtual void RemovePlayerState(APlayerState* PlayerState) override;

	virtual void OnFinishMatch() override;

	const TArray<FRankedPlayerState>& GetRankedPlayerStates() const;

protected:
	void UpdateRanking();

	void OnChangedPlayerName(FString Name);
	void OnChangedPlayerId(int32 Id);
	void OnChangedPlayerKillCount(int32 Count);
	void OnChangedPlayerDeathCount(int32 Count);

	UFUNCTION()
	void OnRep_RankedPlayerStates();

public:
	DECLARE_EVENT_OneParam(AASDeathmatchGameState, FOnUpdatedRankingEvent, const TArray<FRankedPlayerState>&);
	FOnUpdatedRankingEvent OnUpdatedRanking;

protected:
	UPROPERTY(ReplicatedUsing = OnRep_RankedPlayerStates)
	TArray<FRankedPlayerState> RankedPlayerStates;
};
