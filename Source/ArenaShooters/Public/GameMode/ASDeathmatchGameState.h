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

	AASPlayerState* GetTopRankPlayerState() const;

protected:
	void UpdateRanking();

	virtual void OnChangedPlayerName(FString Name) override;
	virtual void OnChangedPlayerId(int32 Id) override;
	virtual void OnChangedPlayerKillCount(int32 Count) override;
	virtual void OnChangedPlayerDeathCount(int32 Count)override;

	UFUNCTION()
	void OnRep_RankedPlayerStates();

public:
	DECLARE_EVENT_OneParam(AASDeathmatchGameState, FOnUpdatedRankingEvent, const TArray<FRankedPlayerState>&);
	FOnUpdatedRankingEvent OnUpdatedRanking;

protected:
	UPROPERTY(ReplicatedUsing = OnRep_RankedPlayerStates)
	TArray<FRankedPlayerState> RankedPlayerStates;
};
