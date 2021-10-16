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

	DOREPLIFETIME(AASDeathmatchGameState, RankedPlayerStates);
}

void AASDeathmatchGameState::AddPlayerState(APlayerState* PlayerState)
{
	Super::AddPlayerState(PlayerState);

	if (!PlayerState->IsInactive())
	{
		auto ASPlayerState = Cast<AASPlayerState>(PlayerState);
		if (IsValid(ASPlayerState))
		{
			ASPlayerState->OnChangedPlayerName.AddUObject(this, &AASDeathmatchGameState::OnChangedPlayerName);
			ASPlayerState->OnChangedPlayerId.AddUObject(this, &AASDeathmatchGameState::OnChangedPlayerId);
			ASPlayerState->OnChangedKillCount.AddUObject(this, &AASDeathmatchGameState::OnChangedPlayerKillCount);
			ASPlayerState->OnChangedDeathCount.AddUObject(this, &AASDeathmatchGameState::OnChangedPlayerDeathCount);

			UpdateRanking();
		}
		else
		{
			AS_LOG_S(Error);
		}
	}
}

void AASDeathmatchGameState::RemovePlayerState(APlayerState* PlayerState)
{
	Super::RemovePlayerState(PlayerState);

	if (!PlayerState->IsInactive())
	{
		auto ASPlayerState = Cast<AASPlayerState>(PlayerState);
		if (IsValid(ASPlayerState))
		{
			ASPlayerState->OnChangedPlayerName.RemoveAll(this);
			ASPlayerState->OnChangedPlayerId.RemoveAll(this);
			ASPlayerState->OnChangedKillCount.RemoveAll(this);
			ASPlayerState->OnChangedDeathCount.RemoveAll(this);

			UpdateRanking();
		}
		else
		{
			AS_LOG_S(Error);
		}
	}
}

void AASDeathmatchGameState::OnFinishMatch()
{
	Super::OnFinishMatch();

}

const TArray<FRankedPlayerState>& AASDeathmatchGameState::GetRankedPlayerStates() const
{
	return RankedPlayerStates;
}

void AASDeathmatchGameState::UpdateRanking()
{
	if (GetLocalRole() != ROLE_Authority)
		return;

	RankedPlayerStates.Empty();
	RankedPlayerStates.Reserve(PlayerArray.Num());

	for (auto& Player : PlayerArray)
	{
		auto ASPlayerState = Cast<AASPlayerState>(Player);
		if (IsValid(ASPlayerState))
		{
			RankedPlayerStates.Emplace(0, ASPlayerState);
		}
	}

	RankedPlayerStates.Sort(
		[](const FRankedPlayerState& Left, const FRankedPlayerState& Right)
		{
			int32 LeftKillCount = Left.PlayerState->GetKillCount();
			int32 RightKillCount = Right.PlayerState->GetKillCount();
			if (LeftKillCount > RightKillCount)
			{
				return true;
			}
			else if (LeftKillCount < RightKillCount)
			{
				return false;
			}

			int32 LeftDeathCount = Left.PlayerState->GetDeathCount();
			int32 RightDeathCount = Right.PlayerState->GetDeathCount();
			if (LeftDeathCount < RightDeathCount)
			{
				return true;
			}
			else if (LeftDeathCount > RightDeathCount)
			{
				return false;
			}

			return Left.PlayerState->GetPlayerId() < Right.PlayerState->GetPlayerId();
		});

	for (int32 Idx = 0; Idx < RankedPlayerStates.Num(); ++Idx)
	{
		FRankedPlayerState& RankedPlayerState = RankedPlayerStates[Idx];

		if (Idx == 0)
		{
			RankedPlayerState.Ranking = 1;
		}
		else
		{
			FRankedPlayerState& BeforeRankedPlayerState = RankedPlayerStates[Idx - 1];

			int32 CurKillCount = RankedPlayerState.PlayerState->GetKillCount();
			int32 CurDeathCount = RankedPlayerState.PlayerState->GetDeathCount();
			int32 BeforeKillCount = BeforeRankedPlayerState.PlayerState->GetKillCount();
			int32 BeforeDeathCount = BeforeRankedPlayerState.PlayerState->GetDeathCount();

			if (CurKillCount == BeforeKillCount && CurDeathCount == BeforeDeathCount)
			{
				RankedPlayerState.Ranking = BeforeRankedPlayerState.Ranking;
			}
			else
			{
				RankedPlayerState.Ranking = BeforeRankedPlayerState.Ranking + 1;
			}
		}
	}

	OnUpdatedRanking.Broadcast(RankedPlayerStates);
}

void AASDeathmatchGameState::OnChangedPlayerName(FString Name)
{
	UpdateRanking();
}

void AASDeathmatchGameState::OnChangedPlayerId(int32 Id)
{
	UpdateRanking();
}

void AASDeathmatchGameState::OnChangedPlayerKillCount(int32 Count)
{
	UpdateRanking();
}

void AASDeathmatchGameState::OnChangedPlayerDeathCount(int32 Count)
{
}

void AASDeathmatchGameState::OnRep_RankedPlayerStates()
{
	OnUpdatedRanking.Broadcast(RankedPlayerStates);
}
