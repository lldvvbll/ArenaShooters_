// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/ASDeathmatchGameMode.h"
#include "GameMode/ASDeathmatchGameState.h"
#include "Controller/ASPlayerController.h"
#include "Controller/ASPlayerState.h"
#include "GameFramework/PlayerStart.h"
#include "EngineUtils.h"

AActor* AASDeathmatchGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
	auto DmGameState = GetGameState<AASDeathmatchGameState>();
	if (ensure(DmGameState))
	{
		if (DmGameState->GetInnerMatchState() != EInnerMatchState::Prepare)
		{
			// 시작 위치 중에서, 가장 가까운 캐릭터와의 거리가 가장 먼 것을 찾는다.

			APlayerStart* FoundedStartSpot = nullptr;
			float FarthestLenSquared = 0.0f;

			UWorld* World = GetWorld();
			for (TActorIterator<APlayerStart> It(World); It; ++It)
			{
				APlayerStart* PlayerStart = *It;
				if (!ensure(IsValid(PlayerStart)))
					continue;

				FVector PlayerStartLocation = PlayerStart->GetActorLocation();

				bool bFound = false;
				float NearestLenSquared = TNumericLimits<float>::Max();
				for (auto Itr = World->GetPlayerControllerIterator(); Itr; ++Itr)
				{
					if (Itr->Get() == Player)
						continue;

					auto PlayerController = Cast<AASPlayerController>(Itr->Get());
					if (ensure(IsValid(PlayerController)))
					{
						auto Pawn = PlayerController->GetPawn();
						if (!ensure(IsValid(Pawn)))
							continue;

						float LenSquared = (PlayerStartLocation - Pawn->GetActorLocation()).SizeSquared();
						if (!bFound || NearestLenSquared > LenSquared)
						{
							NearestLenSquared = LenSquared;
							bFound = true;
						}
					}
				}

				if (bFound)
				{
					if (FoundedStartSpot == nullptr || FarthestLenSquared < NearestLenSquared)
					{
						FarthestLenSquared = NearestLenSquared;
						FoundedStartSpot = PlayerStart;
					}
				}
			}

			return FoundedStartSpot;
		}
	}

	return Super::ChoosePlayerStart_Implementation(Player);
}

bool AASDeathmatchGameMode::ShouldSpawnAtStartSpot(AController* Player)
{
	auto DmGameState = GetGameState<AASDeathmatchGameState>();
	if (ensure(DmGameState))
	{
		if (DmGameState->GetInnerMatchState() != EInnerMatchState::Prepare)
			return false;
	}

	return Super::ShouldSpawnAtStartSpot(Player);
}

void AASDeathmatchGameMode::OnKillCharacter(AASPlayerController* KillerController, AASPlayerController* DeadController)
{
	Super::OnKillCharacter(KillerController, DeadController);

	auto DmGameState = GetGameState<AASDeathmatchGameState>();
	if (ensure(DmGameState))
	{
		AASPlayerState* TopRankPlayerState = DmGameState->GetTopRankPlayerState();
		if (ensure(IsValid(TopRankPlayerState)))
		{
			if (TopRankPlayerState->GetKillCount() >= GoalNumOfKills)
			{
				FinishMatch();
			}
		}

		if (DmGameState->IsMatchProcess())
		{
			if (IsValid(DeadController))
			{
				DeadController->SetRespawnTimer(RespawnDelay);
			}
		}
	}
}

void AASDeathmatchGameMode::PrepareAllPlayerStart()
{
	TArray<APlayerStart*> PlayerStarts;

	UWorld* World = GetWorld();
	for (TActorIterator<APlayerStart> It(World); It; ++It)
	{
		APlayerStart* PlayerStart = *It;
		if (!ensure(IsValid(PlayerStart)))
			continue;

		PlayerStarts.Emplace(PlayerStart);
	}

	if (!ensure(PlayerStarts.Num() > 0))
		return;

	// Shuffle
	int32 LastIdx = PlayerStarts.Num() - 1;
	for (int32 Idx = 0; Idx <= LastIdx; ++Idx)
	{
		int32 TargetIdx = FMath::RandRange(0, LastIdx);
		PlayerStarts.Swap(Idx, TargetIdx);
	}

	int32 PlayerStartIdx = 0;
	for (auto Itr = GetWorld()->GetPlayerControllerIterator(); Itr; ++Itr)
	{
		auto PlayerController = Cast<AASPlayerController>(Itr->Get());
		if (ensure(IsValid(PlayerController)))
		{
			PlayerController->StartSpot = PlayerStarts[PlayerStartIdx++];

			if (PlayerStartIdx > LastIdx)
			{
				PlayerStartIdx = 0;
			}
		}
	}
}
