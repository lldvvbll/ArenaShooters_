// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ArenaShooters.h"
#include "GameFramework/PlayerState.h"
#include "ASPlayerState.generated.h"

class UASItemSetDataAsset;

UCLASS()
class ARENASHOOTERS_API AASPlayerState : public APlayerState
{
	GENERATED_BODY()
	
public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void SetPlayerName(const FString& S) override;
	virtual void OnRep_PlayerName() override;
	virtual void OnRep_PlayerId() override;

	int32 GetKillCount() const;
	void SetKillCount(int32 Count);
	void ModifyKillCount(int32 Count);

	int32 GetDeathCount() const;
	void SetDeathCount(int32 Count);
	void ModifyDeathCount(int32 Count);

	void OnKill();
	void OnDie();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerSetItemSetDataAsset(UASItemSetDataAsset* DataAsset);
	bool ServerSetItemSetDataAsset_Validate(UASItemSetDataAsset* DataAsset);
	void ServerSetItemSetDataAsset_Implementation(UASItemSetDataAsset* DataAsset);

	UFUNCTION(Client, Reliable)
	void ClientSetItemSetDataAsset(UASItemSetDataAsset* DataAsset);
	void ClientSetItemSetDataAsset_Implementation(UASItemSetDataAsset* DataAsset);

	UASItemSetDataAsset* GetItemSetDataAsset() const;
	FPrimaryAssetId GetItemSetDataAssetId() const;

protected:
	UFUNCTION()
	void OnRep_KillCount();

	UFUNCTION()
	void OnRep_DeathCount();

	UFUNCTION()
	void OnRep_ItemSetDataAsset();

public:
	DECLARE_EVENT_OneParam(AASPlayerState, FOnChangedPlayerNameEvent, FString);
	FOnChangedPlayerNameEvent OnChangedPlayerName;

	DECLARE_EVENT_OneParam(AASPlayerState, FOnChangedPlayerIdEvent, int32);
	FOnChangedPlayerIdEvent OnChangedPlayerId;

	DECLARE_EVENT_OneParam(AASPlayerState, FOnChangedKillCountEvent, int32);
	FOnChangedKillCountEvent OnChangedKillCount;

	DECLARE_EVENT_OneParam(AASPlayerState, FOnChangedDeathCountEvent, int32);
	FOnChangedDeathCountEvent OnChangedDeathCount;

	DECLARE_EVENT_OneParam(AASPlayerState, FOnSetItemSetDataAssetEvent, UASItemSetDataAsset*);
	FOnSetItemSetDataAssetEvent OnSetItemSetDataAsset;

protected:
	UPROPERTY(ReplicatedUsing = OnRep_KillCount)
	int32 KillCount;

	UPROPERTY(ReplicatedUsing = OnRep_DeathCount)
	int32 DeathCount;

	UPROPERTY(ReplicatedUsing = OnRep_ItemSetDataAsset)
	UASItemSetDataAsset* ItemSetDataAsset;
};
