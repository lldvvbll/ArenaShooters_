// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/ASDeathmatchGameMode.h"
#include "Controller/ASPlayerController.h"
#include "Controller/ASPlayerState.h"

void AASDeathmatchGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

}

void AASDeathmatchGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

}

void AASDeathmatchGameMode::OnKillCharacter(AASPlayerController* KillerController, AASPlayerController* DeadController)
{
	Super::OnKillCharacter(KillerController, DeadController);

	if (IsValid(KillerController))
	{
		auto KillerPlayerState = KillerController->GetPlayerState<AASPlayerState>();
		if (IsValid(KillerPlayerState))
		{
			if (KillerPlayerState->GetKillCount() >= GoalNumOfKills)
			{
				OnAchieveGoal(KillerPlayerState);
			}
		}
		else
		{
			AS_LOG_S(Error);
		}
	}
	else
	{
		AS_LOG_S(Error);
	}
}

void AASDeathmatchGameMode::OnAchieveGoal(AASPlayerState* WonPlayerState)
{
	FinishMatch();
}
