// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/ASDeathmatchGameMode.h"

void AASDeathmatchGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (MinPlayerCount <= NumPlayers)
	{
		StartPlay();
	}
}

void AASDeathmatchGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

}
