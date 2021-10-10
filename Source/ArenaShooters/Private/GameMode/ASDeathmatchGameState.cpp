// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/ASDeathmatchGameState.h"
#include "GameMode/ASItemFactoryComponent.h"
#include "Net/UnrealNetwork.h"
#include "Controller/ASPlayerState.h"

AASDeathmatchGameState::AASDeathmatchGameState()
{
}

void AASDeathmatchGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AASDeathmatchGameState, WonPlayerState);
}

void AASDeathmatchGameState::OnFinishMatch()
{
	Super::OnFinishMatch();

	SetWinner(GetPlayerStateOfTopKillCount());
}

void AASDeathmatchGameState::SetWinner(AASPlayerState* InWonPlayerState)
{
	WonPlayerState = InWonPlayerState;

	OnSetWinner.Broadcast(WonPlayerState);
}

void AASDeathmatchGameState::OnRep_WonPlayerState()
{
	OnSetWinner.Broadcast(WonPlayerState);
}
