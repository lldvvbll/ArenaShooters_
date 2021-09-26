// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/ASMatchGameModeBase.h"
#include "GameMode/ASMatchGameStateBase.h"

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
