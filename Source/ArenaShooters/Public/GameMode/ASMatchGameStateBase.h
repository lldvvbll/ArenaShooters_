// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ArenaShooters.h"
#include "GameFramework/GameState.h"
#include "Common/ASEnums.h"
#include "ASMatchGameStateBase.generated.h"

class UASItemFactoryComponent;
class AASPlayerState;

UCLASS()
class ARENASHOOTERS_API AASMatchGameStateBase : public AGameState
{
	GENERATED_BODY()
	
public:
	AASMatchGameStateBase();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostInitializeComponents() override;

	virtual void AddPlayerState(APlayerState* PlayerState) override;
	virtual void RemovePlayerState(APlayerState* PlayerState) override;

	UASItemFactoryComponent* GetItemFactory();

	int32 GetMaxNumPlayer() const;
	void SetMaxNumPlayers(int32 Num);

	int32 GetGoalNumOfKills() const;
	void SetGoalNumOfKills(int32 Num);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastOnSetPrepareTimer(float PrepareTime);
	void MulticastOnSetPrepareTimer_Implementation(float PrepareTime);

	EInnerMatchState GetInnerMatchState() const;
	void SetInnerMatchState(EInnerMatchState State);
	bool IsMatchProcess() const;

	UFUNCTION(NetMulticast, Reliable)
	void MulticastOnKill(AASPlayerState* KillerPlayerState, AASPlayerState* DeadPlayerState);
	void MulticastOnKill_Implementation(AASPlayerState* KillerPlayerState, AASPlayerState* DeadPlayerState);

	virtual void OnFinishMatch();

protected:
	UFUNCTION()
	void OnRep_NumPlayers(int32 OldNum);

	UFUNCTION()
	void OnRep_InnerMatchState();

public:
	DECLARE_EVENT_OneParam(AASMatchGameStateBase, FOnChangedNumPlayersEvent, int32);
	FOnChangedNumPlayersEvent OnChangedNumPlayers;

	DECLARE_EVENT_OneParam(AASMatchGameStateBase, FOnSetPrepareTimeEvent, FDateTime);
	FOnSetPrepareTimeEvent OnSetPrepareTime;

	DECLARE_EVENT_OneParam(AASMatchGameStateBase, FOnChangedInnerMatchStateEvent, EInnerMatchState)
	FOnChangedInnerMatchStateEvent OnChangedInnerMatchState;

protected:
	UPROPERTY(Replicated, EditDefaultsOnly)
	UASItemFactoryComponent* ItemFactory;

	UPROPERTY(Replicated)
	int32 MaxNumPlayers;

	UPROPERTY(ReplicatedUsing = OnRep_NumPlayers)
	int32 NumPlayers;

	UPROPERTY(Replicated)
	int32 GoalNumOfKills;

	FDateTime StartTimeForProcess;

	UPROPERTY(ReplicatedUsing = OnRep_InnerMatchState)
	EInnerMatchState InnerMatchState;
};
