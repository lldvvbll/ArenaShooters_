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

}

void AASDeathmatchGameState::OnFinishMatch()
{
	Super::OnFinishMatch();

}

TArray<AASPlayerState*> AASDeathmatchGameState::GetPlayersSortedByKillCount() const
{
	TArray<AASPlayerState*> SortedPlayers;
	SortedPlayers.Reserve(PlayerArray.Num());

	for (auto& Player : PlayerArray)
	{
		auto ASPlayerState = Cast<AASPlayerState>(Player);
		if (IsValid(ASPlayerState))
		{
			SortedPlayers.Emplace(ASPlayerState);
		}		
	}

	SortedPlayers.Sort(
		[](const AASPlayerState& Left, const AASPlayerState& Right)
		{
			int32 LeftKillCount = Left.GetKillCount();
			int32 RightKillCount = Right.GetKillCount();
			if (LeftKillCount > RightKillCount)
			{
				return true;
			}
			else if (LeftKillCount < RightKillCount)
			{
				return false;
			}

			return Left.GetDeathCount() < Right.GetDeathCount();
		});

	return SortedPlayers;
}
