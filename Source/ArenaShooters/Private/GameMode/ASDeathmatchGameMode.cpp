// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/ASDeathmatchGameMode.h"
#include "Controller/ASPlayerController.h"
#include "Controller/ASPlayerState.h"

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
				FinishMatch();
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
