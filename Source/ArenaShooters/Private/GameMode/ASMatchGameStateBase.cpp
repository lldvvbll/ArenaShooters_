// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/ASMatchGameStateBase.h"
#include "GameMode/ASItemFactoryComponent.h"
#include "Net/UnrealNetwork.h"
#include "ASGameInstance.h"
#include "Controller/ASPlayerState.h"
#include "Controller/ASPlayerController.h"
#include "ASAssetManager.h"
#include "DataAssets/ItemDataAssets/ASItemSetDataAsset.h"

AASMatchGameStateBase::AASMatchGameStateBase()
{
	ItemFactory = CreateDefaultSubobject<UASItemFactoryComponent>(TEXT("ItemFactory"));
}

void AASMatchGameStateBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AASMatchGameStateBase, ItemFactory);
	DOREPLIFETIME(AASMatchGameStateBase, MaxNumPlayers);
	DOREPLIFETIME(AASMatchGameStateBase, MinNumPlayers);
	DOREPLIFETIME(AASMatchGameStateBase, StartTimeForProcess);
	DOREPLIFETIME(AASMatchGameStateBase, GoalNumOfKills);
	DOREPLIFETIME(AASMatchGameStateBase, InnerMatchState);
	DOREPLIFETIME(AASMatchGameStateBase, MatchFinishTime);
	DOREPLIFETIME(AASMatchGameStateBase, RestartTime);	
}

void AASMatchGameStateBase::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	// �̸� �ε��� �α� ���� ȣ��
	TArray<UASItemSetDataAsset*> DummyArrayForAssetLoading = GetItemSetDataAssets();
}

void AASMatchGameStateBase::AddPlayerState(APlayerState* PlayerState)
{
	Super::AddPlayerState(PlayerState);

	if (!PlayerState->IsInactive())
	{
		auto ASPlayerState = Cast<AASPlayerState>(PlayerState);
		if (ensure(IsValid(ASPlayerState)))
		{
			if (GetNetMode() == NM_DedicatedServer)
			{
				TArray<UASItemSetDataAsset*> DataAssets = GetItemSetDataAssets();
				if (ensure(DataAssets.Num() > 0))
				{
					ASPlayerState->ServerSetItemSetDataAsset(DataAssets[0]);
				}
			}

			OnAddedPlayerState.Broadcast(ASPlayerState);

			ASPlayerState->OnChangedPlayerName.AddUObject(this, &AASMatchGameStateBase::OnChangedPlayerName);
			ASPlayerState->OnChangedPlayerId.AddUObject(this, &AASMatchGameStateBase::OnChangedPlayerId);
			ASPlayerState->OnChangedKillCount.AddUObject(this, &AASMatchGameStateBase::OnChangedPlayerKillCount);
			ASPlayerState->OnChangedDeathCount.AddUObject(this, &AASMatchGameStateBase::OnChangedPlayerDeathCount);	
		}
	}
}

void AASMatchGameStateBase::RemovePlayerState(APlayerState* PlayerState)
{
	Super::RemovePlayerState(PlayerState);

	if (!PlayerState->IsInactive())
	{
		auto ASPlayerState = Cast<AASPlayerState>(PlayerState);
		if (ensure(IsValid(ASPlayerState)))
		{
			OnRemovedPlayerState.Broadcast(ASPlayerState);

			ASPlayerState->OnChangedPlayerName.RemoveAll(this);
			ASPlayerState->OnChangedPlayerId.RemoveAll(this);
			ASPlayerState->OnChangedKillCount.RemoveAll(this);
			ASPlayerState->OnChangedDeathCount.RemoveAll(this);
		}
	}
}

UASItemFactoryComponent* AASMatchGameStateBase::GetItemFactory()
{
	return ItemFactory;
}

int32 AASMatchGameStateBase::GetNumPlayers() const
{
	return PlayerArray.Num();
}

int32 AASMatchGameStateBase::GetMaxNumPlayer() const
{
	return MaxNumPlayers;
}

void AASMatchGameStateBase::SetMaxNumPlayers(int32 Num)
{
	MaxNumPlayers = Num;
}

int32 AASMatchGameStateBase::GetMinNumPlayer() const
{
	return MinNumPlayers;
}

void AASMatchGameStateBase::SetMinNumPlayers(int32 Num)
{
	MinNumPlayers = Num;
}

int32 AASMatchGameStateBase::GetGoalNumOfKills() const
{
	return GoalNumOfKills;
}

void AASMatchGameStateBase::SetGoalNumOfKills(int32 Num)
{
	GoalNumOfKills = Num;
}

void AASMatchGameStateBase::SetStartTimeForProcess(float StartTime)
{
	StartTimeForProcess = StartTime;

	OnStartTimeForProcess.Broadcast(StartTimeForProcess);
}

EInnerMatchState AASMatchGameStateBase::GetInnerMatchState() const
{
	return InnerMatchState;
}

void AASMatchGameStateBase::SetInnerMatchState(EInnerMatchState State)
{
	InnerMatchState = State;

	AS_LOG(Warning, TEXT("InnerMatchState: %d"), static_cast<int32>(InnerMatchState));

	OnChangedInnerMatchState.Broadcast(InnerMatchState);

	for (auto Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		auto PlayerController = Cast<AASPlayerController>(Iterator->Get());
		if (ensure(IsValid(PlayerController)))
		{
			PlayerController->OnChangedInnerMatchState(InnerMatchState);
		}
	}
}

bool AASMatchGameStateBase::IsMatchProcess() const
{
	return InnerMatchState == EInnerMatchState::Process;
}

void AASMatchGameStateBase::MulticastOnKill_Implementation(AASPlayerState* KillerPlayerState, AASPlayerState* DeadPlayerState, int32 KillCount)
{
	OnKill.Broadcast(KillerPlayerState, DeadPlayerState, KillCount);
}

FDateTime AASMatchGameStateBase::GetMatchFinishTime() const
{
	return MatchFinishTime;
}

void AASMatchGameStateBase::SetMatchFinishTime(float FinishTime)
{
	MatchFinishTime = FinishTime;

	OnSetMatchFinishTime.Broadcast(MatchFinishTime);
}

FDateTime AASMatchGameStateBase::GetRestartTime() const
{
	return RestartTime;
}

void AASMatchGameStateBase::SetRestartTime(float Time)
{
	RestartTime = Time;

	OnSetRestartTime.Broadcast(RestartTime);
}

TArray<UASItemSetDataAsset*> AASMatchGameStateBase::GetItemSetDataAssets() const
{
	TArray<UASItemSetDataAsset*> DataAssets;

	UASAssetManager& AssetManager = UASAssetManager::Get();
	for (auto& AssetId : ItemSetAssetIds)
	{
		auto ItemSetAsset = AssetManager.GetDataAsset<UASItemSetDataAsset>(AssetId);
		if (ensure(ItemSetAsset))
		{
			DataAssets.Emplace(ItemSetAsset);
		}
	}

	return DataAssets;
}

bool AASMatchGameStateBase::IsValidItemSetDataAsset(UASItemSetDataAsset* DataAsset) const
{
	if (!ensure(DataAsset))
		return false;

	return ItemSetAssetIds.Contains(DataAsset->GetPrimaryAssetId());
}

void AASMatchGameStateBase::OnChangedPlayerName(FString Name)
{
}

void AASMatchGameStateBase::OnChangedPlayerId(int32 Id)
{
}

void AASMatchGameStateBase::OnChangedPlayerKillCount(int32 Count)
{
}

void AASMatchGameStateBase::OnChangedPlayerDeathCount(int32 Count)
{
}

void AASMatchGameStateBase::OnRep_GoalNumOfKills()
{
	OnSetGoalNumOfKills.Broadcast(GoalNumOfKills);
}

void AASMatchGameStateBase::OnRep_StartTimeForProcess()
{
	OnStartTimeForProcess.Broadcast(StartTimeForProcess);

	for (auto Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		auto PlayerController = Cast<AASPlayerController>(Iterator->Get());
		if (ensure(IsValid(PlayerController)))
		{
			APawn* Pawn = PlayerController->GetPawn();
			if (ensure(IsValid(Pawn)))
			{
				Pawn->DisableInput(nullptr);
			}
		}
	}
}

void AASMatchGameStateBase::OnRep_InnerMatchState()
{
	for (auto Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		auto PlayerController = Cast<AASPlayerController>(Iterator->Get());
		if (ensure(IsValid(PlayerController)))
		{
			PlayerController->OnChangedInnerMatchState(InnerMatchState);
		}
	}

	OnChangedInnerMatchState.Broadcast(InnerMatchState);
}

void AASMatchGameStateBase::OnRep_MatchFinishTime()
{
	OnSetMatchFinishTime.Broadcast(MatchFinishTime);
}

void AASMatchGameStateBase::OnRep_RestartTime()
{
	OnSetRestartTime.Broadcast(RestartTime);

	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		auto PlayerController = Cast<AASPlayerController>(Iterator->Get());
		if (IsValid(PlayerController))
		{
			PlayerController->ShowRestartTimerWidget(RestartTime);
		}
	}
}

AASPlayerState* AASMatchGameStateBase::GetPlayerStateOfTopKillCount() const
{
	AASPlayerState* TopPlayer = nullptr;
	int32 TopNumOfKills = 0;
	for (auto& PlayerState : PlayerArray)
	{
		auto ASPlayerState = Cast<AASPlayerState>(PlayerState);
		if (!IsValid(ASPlayerState))
			continue;

		int32 KillCount = ASPlayerState->GetKillCount();
		if (KillCount > TopNumOfKills)
		{
			TopPlayer = ASPlayerState;
			TopNumOfKills = KillCount;
		}
	}

	return TopPlayer;
}
