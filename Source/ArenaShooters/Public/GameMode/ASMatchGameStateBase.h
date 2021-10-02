// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ArenaShooters.h"
#include "GameFramework/GameState.h"
#include "ASMatchGameStateBase.generated.h"

class UASItemFactoryComponent;

UCLASS()
class ARENASHOOTERS_API AASMatchGameStateBase : public AGameState
{
	GENERATED_BODY()
	
public:
	AASMatchGameStateBase();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UASItemFactoryComponent* GetItemFactory();

	void SetNumPlayers(int32 NewNum);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastOnSetPrepareTimer(float PrepareTime);
	void MulticastOnSetPrepareTimer_Implementation(float PrepareTime);

	void SetMatchProcess(bool bIsMatchProcess);
	bool IsMatchProcess() const;

protected:
	UFUNCTION()
	void OnRep_NumPlayers(int32 OldNum);

	UFUNCTION()
	void OnRep_bMatchProcess();

public:
	DECLARE_EVENT_OneParam(AASMatchGameStateBase, FOnChangedNumPlayersEvent, int32);
	FOnChangedNumPlayersEvent OnChangedNumPlayers;

	DECLARE_EVENT_OneParam(AASMatchGameStateBase, FOnSetPrepareTimeEvent, FDateTime);
	FOnSetPrepareTimeEvent OnSetPrepareTime;

	DECLARE_EVENT_OneParam(AASMatchGameStateBase, FOnChangedMatchProcessEvent, bool)
	FOnChangedMatchProcessEvent OnChangedMatchProcess;

protected:
	UPROPERTY(Replicated, EditDefaultsOnly)
	UASItemFactoryComponent* ItemFactory;

	UPROPERTY(ReplicatedUsing = OnRep_NumPlayers)
	int32 NumPlayers;

	FDateTime StartTimeForProcess;

	UPROPERTY(ReplicatedUsing = OnRep_bMatchProcess)
	bool bMatchProcess;
};
