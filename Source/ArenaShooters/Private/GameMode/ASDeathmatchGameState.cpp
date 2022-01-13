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

	if (GetLocalRole() == ROLE_Authority)
	{
		if (!PlayerState->IsInactive())
		{
			auto ASPlayerState = Cast<AASPlayerState>(PlayerState);
			if (ensure(IsValid(ASPlayerState)))
			{
				UpdateRanking();
			}
		}
	}
}

void AASDeathmatchGameState::RemovePlayerState(APlayerState* PlayerState)
{
	Super::RemovePlayerState(PlayerState);

	if (GetLocalRole() == ROLE_Authority)
	{
		if (!PlayerState->IsInactive())
		{
			auto ASPlayerState = Cast<AASPlayerState>(PlayerState);
			if (ensure(IsValid(ASPlayerState)))
			{
				UpdateRanking();
			}
		}
	}
}

const TArray<FRankedPlayerState>& AASDeathmatchGameState::GetRankedPlayerStates() const
{
	return RankedPlayerStates;
}

AASPlayerState* AASDeathmatchGameState::GetTopRankPlayerState() const
{
	if (RankedPlayerStates.Num() <= 0)
		return nullptr;

	return RankedPlayerStates[0].PlayerState;
}

void AASDeathmatchGameState::UpdateRanking()
{
	if (GetLocalRole() != ROLE_Authority)
		return;

	RankedPlayerStates.Empty(PlayerArray.Num());

	for (auto& Player : PlayerArray)
	{
		auto ASPlayerState = Cast<AASPlayerState>(Player);
		if (IsValid(ASPlayerState))
		{
			RankedPlayerStates.Emplace(0, ASPlayerState);
		}
	}

	// 킬 수가 높은 순, 데스가 적은 순, 둘 다 같다면 아이디 순
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

	// 기본적으로 정렬 된 순서대로 랭킹을 매기지만 킬 수, 데스 수가 같으면 동일 랭크이다
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
	Super::OnChangedPlayerName(Name);

	//UpdateRanking();
}

void AASDeathmatchGameState::OnChangedPlayerId(int32 Id)
{
	Super::OnChangedPlayerId(Id);

	UpdateRanking();
}

void AASDeathmatchGameState::OnChangedPlayerKillCount(int32 Count)
{
	Super::OnChangedPlayerKillCount(Count);

	UpdateRanking();
}

void AASDeathmatchGameState::OnChangedPlayerDeathCount(int32 Count)
{
	Super::OnChangedPlayerDeathCount(Count);

}

void AASDeathmatchGameState::OnRep_RankedPlayerStates()
{
	OnUpdatedRanking.Broadcast(RankedPlayerStates);
}
