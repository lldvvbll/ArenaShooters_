// Fill out your copyright notice in the Description page of Project Settings.


#include "Controller/ASPlayerState.h"
#include "Net/UnrealNetwork.h"
#include "DataAssets/ItemDataAssets/ASItemSetDataAsset.h"
#include "GameMode/ASMatchGameStateBase.h"

void AASPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AASPlayerState, KillCount);
	DOREPLIFETIME(AASPlayerState, DeathCount);
	DOREPLIFETIME(AASPlayerState, ItemSetDataAsset);
}

void AASPlayerState::SetPlayerName(const FString& S)
{
	Super::SetPlayerName(S);

	if (GetNetMode() == NM_DedicatedServer)
	{
		OnChangedPlayerName.Broadcast(GetPlayerName());
	}
}

void AASPlayerState::OnRep_PlayerName()
{
	Super::OnRep_PlayerName();

	OnChangedPlayerName.Broadcast(GetPlayerName());
}

void AASPlayerState::OnRep_PlayerId()
{
	Super::OnRep_PlayerId();

	OnChangedPlayerId.Broadcast(GetPlayerId());
}

int32 AASPlayerState::GetKillCount() const
{
	return KillCount;
}

void AASPlayerState::SetKillCount(int32 Count)
{
	KillCount = Count;

	OnChangedKillCount.Broadcast(KillCount);
}

void AASPlayerState::ModifyKillCount(int32 Count)
{
	SetKillCount(KillCount + Count);
}

int32 AASPlayerState::GetDeathCount() const
{
	return DeathCount;
}

void AASPlayerState::SetDeathCount(int32 Count)
{
	DeathCount = Count;

	OnChangedDeathCount.Broadcast(DeathCount);
}

void AASPlayerState::ModifyDeathCount(int32 Count)
{
	SetDeathCount(DeathCount + Count);
}

void AASPlayerState::OnKill()
{
	ModifyKillCount(1);
}

void AASPlayerState::OnDie()
{
	ModifyDeathCount(1);
}

bool AASPlayerState::ServerSetItemSetDataAsset_Validate(UASItemSetDataAsset* DataAsset)
{
	auto GameState = GetWorld()->GetGameState<AASMatchGameStateBase>();
	if (!ensure(GameState))
		return false;

	return GameState->IsValidItemSetDataAsset(DataAsset);
}

void AASPlayerState::ServerSetItemSetDataAsset_Implementation(UASItemSetDataAsset* DataAsset)
{
	ItemSetDataAsset = DataAsset;

	ClientSetItemSetDataAsset(ItemSetDataAsset);
}

void AASPlayerState::ClientSetItemSetDataAsset_Implementation(UASItemSetDataAsset* DataAsset)
{
	ItemSetDataAsset = DataAsset;

	OnSetItemSetDataAsset.Broadcast(ItemSetDataAsset);
}

UASItemSetDataAsset* AASPlayerState::GetItemSetDataAsset() const
{
	return ItemSetDataAsset;
}

FPrimaryAssetId AASPlayerState::GetItemSetDataAssetId() const
{
	return (ItemSetDataAsset != nullptr ? ItemSetDataAsset->GetPrimaryAssetId() : FPrimaryAssetId());
}

void AASPlayerState::OnRep_KillCount()
{
	OnChangedKillCount.Broadcast(KillCount);
}

void AASPlayerState::OnRep_DeathCount()
{
	OnChangedDeathCount.Broadcast(DeathCount);
}

void AASPlayerState::OnRep_ItemSetDataAsset()
{
	OnSetItemSetDataAsset.Broadcast(ItemSetDataAsset);
}
