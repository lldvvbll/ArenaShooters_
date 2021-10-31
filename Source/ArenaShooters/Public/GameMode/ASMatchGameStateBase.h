// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ArenaShooters.h"
#include "GameFramework/GameState.h"
#include "Common/ASEnums.h"
#include "ASMatchGameStateBase.generated.h"

class UASItemFactoryComponent;
class AASPlayerState;
class UASItemSetDataAsset;

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

	int32 GetNumPlayers() const;

	int32 GetMaxNumPlayer() const;
	void SetMaxNumPlayers(int32 Num);

	int32 GetGoalNumOfKills() const;
	void SetGoalNumOfKills(int32 Num);

	void SetStartTimeForProcess(float StartTime);

	EInnerMatchState GetInnerMatchState() const;
	void SetInnerMatchState(EInnerMatchState State);
	bool IsMatchProcess() const;

	UFUNCTION(NetMulticast, Reliable)
	void MulticastOnKill(AASPlayerState* KillerPlayerState, AASPlayerState* DeadPlayerState, int32 KillCount);
	void MulticastOnKill_Implementation(AASPlayerState* KillerPlayerState, AASPlayerState* DeadPlayerState, int32 KillCount);

	FDateTime GetMatchFinishTime() const;
	void SetMatchFinishTime(float FinishTime);

	FDateTime GetRestartTime() const;
	void SetRestartTime(float Time);

	TArray<UASItemSetDataAsset*> GetItemSetDataAssets() const;
	bool IsValidItemSetDataAsset(UASItemSetDataAsset* DataAsset) const;

protected:
	virtual void OnChangedPlayerName(FString Name);
	virtual void OnChangedPlayerId(int32 Id);
	virtual void OnChangedPlayerKillCount(int32 Count);
	virtual void OnChangedPlayerDeathCount(int32 Count);

	UFUNCTION()
	void OnRep_StartTimeForProcess();

	UFUNCTION()
	void OnRep_InnerMatchState();

	UFUNCTION()
	void OnRep_MatchFinishTime();

	UFUNCTION()
	void OnRep_RestartTime();

	AASPlayerState* GetPlayerStateOfTopKillCount() const;

public:
	DECLARE_EVENT_OneParam(AASMatchGameStateBase, FOnAddedPlayerStateEvent, APlayerState*);
	FOnAddedPlayerStateEvent OnAddedPlayerState;

	DECLARE_EVENT_OneParam(AASMatchGameStateBase, FOnRemovedPlayerStateEvent, APlayerState*);
	FOnRemovedPlayerStateEvent OnRemovedPlayerState;

	DECLARE_EVENT_OneParam(AASMatchGameStateBase, FOnStartTimeForProcessEvent, float);
	FOnStartTimeForProcessEvent OnStartTimeForProcess;

	DECLARE_EVENT_OneParam(AASMatchGameStateBase, FOnChangedInnerMatchStateEvent, EInnerMatchState)
	FOnChangedInnerMatchStateEvent OnChangedInnerMatchState;

	DECLARE_EVENT_OneParam(AASMatchGameStateBase, FOnSetMatchFinishTimeEvent, float)
	FOnSetMatchFinishTimeEvent OnSetMatchFinishTime;

	DECLARE_EVENT_OneParam(AASMatchGameStateBase, FOnSetRestartTimeEvent, float)
	FOnSetRestartTimeEvent OnSetRestartTime;

	DECLARE_EVENT_ThreeParams(AASMatchGameStateBase, FOnKillEvent, AASPlayerState*, AASPlayerState*, int32)
	FOnKillEvent OnKill;

protected:
	UPROPERTY(Replicated, EditDefaultsOnly)
	UASItemFactoryComponent* ItemFactory;

	UPROPERTY(Replicated)
	int32 MaxNumPlayers;

	UPROPERTY(Replicated)
	int32 GoalNumOfKills;

	UPROPERTY(ReplicatedUsing = OnRep_StartTimeForProcess)
	float StartTimeForProcess;

	UPROPERTY(ReplicatedUsing = OnRep_InnerMatchState)
	EInnerMatchState InnerMatchState;

	UPROPERTY(ReplicatedUsing = OnRep_MatchFinishTime)
	float MatchFinishTime;

	UPROPERTY(ReplicatedUsing = OnRep_RestartTime)
	float RestartTime;

	UPROPERTY(EditDefaultsOnly)
	TArray<FPrimaryAssetId> ItemSetAssetIds;
};
