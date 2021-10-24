// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/ASDeathmatchGameMode.h"
#include "GameMode/ASDeathmatchGameState.h"
#include "Controller/ASPlayerController.h"
#include "Controller/ASPlayerState.h"

void AASDeathmatchGameMode::OnKillCharacter(AASPlayerController* KillerController, AASPlayerController* DeadController)
{
	Super::OnKillCharacter(KillerController, DeadController);

	auto DmGameState = GetGameState<AASDeathmatchGameState>();
	if (IsValid(DmGameState))
	{
		AASPlayerState* TopRankPlayerState = DmGameState->GetTopRankPlayerState();
		if (IsValid(TopRankPlayerState))
		{
			if (TopRankPlayerState->GetKillCount() >= GoalNumOfKills)
			{
				FinishMatch();
			}
		}
		else
		{
			AS_LOG_S(Error);
		}


		if (DmGameState->IsMatchProcess())
		{
			if (IsValid(DeadController))
			{
				DeadController->SetRespawnTimer(RespawnDelay);
			}
		}
	}
	else
	{
		AS_LOG_S(Error);
	}
}
