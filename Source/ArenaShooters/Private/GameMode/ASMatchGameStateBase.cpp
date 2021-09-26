// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/ASMatchGameStateBase.h"
#include "GameMode/ASItemFactoryComponent.h"
#include "Net/UnrealNetwork.h"
#include "..\..\Public\GameMode\ASMatchGameStateBase.h"

AASMatchGameStateBase::AASMatchGameStateBase()
{
	ItemFactory = CreateDefaultSubobject<UASItemFactoryComponent>(TEXT("ItemFactory"));
}

void AASMatchGameStateBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AASMatchGameStateBase, ItemFactory);
	DOREPLIFETIME(AASMatchGameStateBase, NumPlayers);
}

UASItemFactoryComponent* AASMatchGameStateBase::GetItemFactory()
{
	return ItemFactory;
}

void AASMatchGameStateBase::SetNumPlayers(int32 NewNum)
{
	NumPlayers = NewNum;
}

void AASMatchGameStateBase::OnRep_NumPlayers(int32 OldNum)
{
	AS_LOG(Warning, TEXT("NumPlayer: %d"), NumPlayers);

	OnChangedNumPlayers.Broadcast(NumPlayers);
}
