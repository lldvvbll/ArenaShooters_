// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ArenaShooters.h"
#include "GameFramework/PlayerState.h"
#include "ASPlayerState.generated.h"

UCLASS()
class ARENASHOOTERS_API AASPlayerState : public APlayerState
{
	GENERATED_BODY()
	
public:
	AASPlayerState();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	int32 GetKillCount() const;
	void SetKillCount(int32 Count);
	void ModifyKillCount(int32 Count);

	int32 GetDeathCount() const;
	void SetDeathCount(int32 Count);
	void ModifyDeathCount(int32 Count);

	void OnKill();
	void OnDie();

protected:
	UFUNCTION()
	void OnRep_KillCount();

	UFUNCTION()
	void OnRep_DeathCount();

public:
	DECLARE_EVENT_OneParam(AASMatchGameStateBase, FOnChangedKillCountEvent, int32);
	FOnChangedKillCountEvent OnChangedKillCount;

	DECLARE_EVENT_OneParam(AASMatchGameStateBase, FOnChangedDeathCountEvent, int32);
	FOnChangedDeathCountEvent OnChangedDeathCount;

protected:
	UPROPERTY(ReplicatedUsing = OnRep_KillCount)
	int32 KillCount;

	UPROPERTY(ReplicatedUsing = OnRep_DeathCount)
	int32 DeathCount;
};
