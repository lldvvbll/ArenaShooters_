// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/ASMatchGameStateBase.h"
#include "GameMode/ASItemFactoryComponent.h"
#include "Net/UnrealNetwork.h"
#include "ASGameInstance.h"

AASMatchGameStateBase::AASMatchGameStateBase()
{
	ItemFactory = CreateDefaultSubobject<UASItemFactoryComponent>(TEXT("ItemFactory"));

	StartTimeForProcess = FDateTime::MaxValue();
}

void AASMatchGameStateBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AASMatchGameStateBase, ItemFactory);
	DOREPLIFETIME(AASMatchGameStateBase, MaxNumPlayers);
	DOREPLIFETIME(AASMatchGameStateBase, NumPlayers);
	DOREPLIFETIME(AASMatchGameStateBase, GoalNumOfKills);	
	DOREPLIFETIME(AASMatchGameStateBase, InnerMatchState);
}

void AASMatchGameStateBase::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	auto GameInst = GetGameInstance<UASGameInstance>();
	if (IsValid(GameInst))
	{
		SetInnerMatchState(GameInst->GetInnerMatchState());
	}
	else
	{
		AS_LOG_S(Error);
	}
}

void AASMatchGameStateBase::AddPlayerState(APlayerState* PlayerState)
{
	Super::AddPlayerState(PlayerState);

	OnChangedNumPlayers.Broadcast(PlayerArray.Num());
}

void AASMatchGameStateBase::RemovePlayerState(APlayerState* PlayerState)
{
	Super::RemovePlayerState(PlayerState);

	OnChangedNumPlayers.Broadcast(PlayerArray.Num());
}

UASItemFactoryComponent* AASMatchGameStateBase::GetItemFactory()
{
	return ItemFactory;
}

int32 AASMatchGameStateBase::GetMaxNumPlayer() const
{
	return MaxNumPlayers;
}

void AASMatchGameStateBase::SetMaxNumPlayers(int32 Num)
{
	MaxNumPlayers = Num;
}

int32 AASMatchGameStateBase::GetGoalNumOfKills() const
{
	return GoalNumOfKills;
}

void AASMatchGameStateBase::SetGoalNumOfKills(int32 Num)
{
	GoalNumOfKills = Num;
}

void AASMatchGameStateBase::MulticastOnSetPrepareTimer_Implementation(float PrepareTime)
{
	StartTimeForProcess = FDateTime::Now() + FTimespan::FromSeconds(PrepareTime + 1.0f);

	AS_LOG(Warning, TEXT("StartTimeForProcess: %s"), *StartTimeForProcess.ToString());

	OnSetPrepareTime.Broadcast(StartTimeForProcess);
}

EInnerMatchState AASMatchGameStateBase::GetInnerMatchState() const
{
	return InnerMatchState;
}

void AASMatchGameStateBase::SetInnerMatchState(EInnerMatchState State)
{
	InnerMatchState = State;

	OnChangedInnerMatchState.Broadcast(InnerMatchState);
}

bool AASMatchGameStateBase::IsMatchProcess() const
{
	return InnerMatchState == EInnerMatchState::Process;
}

void AASMatchGameStateBase::MulticastOnKill_Implementation(AASPlayerState* KillerPlayerState, AASPlayerState* DeadPlayerState)
{
	if (GetLocalRole() == ROLE_Authority)
	{
		// server

	}
	else
	{
		// client

		// todo: Kill log
	}
}

void AASMatchGameStateBase::OnFinishMatch()
{
	auto GameInst = GetGameInstance<UASGameInstance>();
	if (IsValid(GameInst))
	{
		GameInst->SetInnerMatchState(EInnerMatchState::Finish);
	}
	else
	{
		AS_LOG_S(Error);
	}

	SetInnerMatchState(EInnerMatchState::Finish);
}

void AASMatchGameStateBase::OnRep_NumPlayers(int32 OldNum)
{
	AS_LOG(Warning, TEXT("NumPlayer: %d"), NumPlayers);

	OnChangedNumPlayers.Broadcast(NumPlayers);
}

void AASMatchGameStateBase::OnRep_InnerMatchState()
{
	OnChangedInnerMatchState.Broadcast(InnerMatchState);
}
