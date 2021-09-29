// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/ASMatchGameModeBase.h"
#include "GameMode/ASMatchGameStateBase.h"

AASMatchGameModeBase::AASMatchGameModeBase()
{
	MaxPlayerCount = 16;
	MinPlayerCount = 2;
}

void AASMatchGameModeBase::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	ASMatchGameState->SetNumPlayers(NumPlayers);
}

void AASMatchGameModeBase::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	ASMatchGameState->SetNumPlayers(NumPlayers);
}

void AASMatchGameModeBase::PreInitializeComponents()
{
	Super::PreInitializeComponents();

	ASMatchGameState = Cast<AASMatchGameStateBase>(GameState);
	if (ASMatchGameState == nullptr)
	{
		AS_LOG_S(Warning);
	}
}

int32 AASMatchGameModeBase::GetMaxPlayerCount() const
{
	return MaxPlayerCount;
}

void AASMatchGameModeBase::SetMaxPlayerCount(int32 Count)
{
	MaxPlayerCount = Count;
}

int32 AASMatchGameModeBase::GetMinPlayerCount() const
{
	return MinPlayerCount;
}

void AASMatchGameModeBase::SetMinPlayerCount(int32 Count)
{
	MinPlayerCount = Count;
}
