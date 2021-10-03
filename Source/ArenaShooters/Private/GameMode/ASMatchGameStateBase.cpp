// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/ASMatchGameStateBase.h"
#include "GameMode/ASItemFactoryComponent.h"
#include "Net/UnrealNetwork.h"

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
	DOREPLIFETIME(AASMatchGameStateBase, bMatchProcess);
}

UASItemFactoryComponent* AASMatchGameStateBase::GetItemFactory()
{
	return ItemFactory;
}

int32 AASMatchGameStateBase::GetMaxNumPlayer() const
{
	return MaxNumPlayers;
}

void AASMatchGameStateBase::SetMaxNumPlayers(int32 NewNum)
{
	MaxNumPlayers = NewNum;
}

void AASMatchGameStateBase::SetNumPlayers(int32 NewNum)
{
	NumPlayers = NewNum;
}

void AASMatchGameStateBase::MulticastOnSetPrepareTimer_Implementation(float PrepareTime)
{
	StartTimeForProcess = FDateTime::Now() + FTimespan::FromSeconds(PrepareTime + 1.0f);

	AS_LOG(Warning, TEXT("StartTimeForProcess: %s"), *StartTimeForProcess.ToString());

	OnSetPrepareTime.Broadcast(StartTimeForProcess);
}

void AASMatchGameStateBase::SetMatchProcess(bool bIsMatchProcess)
{
	bMatchProcess = bIsMatchProcess;

	AS_LOG(Warning, TEXT("bMatchProcess: %d"), bMatchProcess);
}

bool AASMatchGameStateBase::IsMatchProcess() const
{
	return bMatchProcess;
}

void AASMatchGameStateBase::OnRep_NumPlayers(int32 OldNum)
{
	AS_LOG(Warning, TEXT("NumPlayer: %d"), NumPlayers);

	OnChangedNumPlayers.Broadcast(NumPlayers);
}

void AASMatchGameStateBase::OnRep_bMatchProcess()
{
	AS_LOG(Warning, TEXT("bMatchProcess: %d"), bMatchProcess);

	OnChangedMatchProcess.Broadcast(bMatchProcess);
}
