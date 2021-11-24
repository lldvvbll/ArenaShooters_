// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/ASMatchGameModeBase.h"
#include "GameMode/ASMatchGameStateBase.h"
#include "ASGameInstance.h"
#include "Controller/ASPlayerController.h"
#include "Controller/ASPlayerState.h"
#include "Common/ASEnums.h"
#include "DataAssets/ItemDataAssets/ASItemSetDataAsset.h"
#include "Character/ASCharacter.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "OnlineSessionSettings.h"
#include "GameFramework/GameSession.h"

AASMatchGameModeBase::AASMatchGameModeBase()
{
	PrepareTime = 10.0f;
	MatchProcessTime = FTimespan::FromMinutes(15.0);
	bSetPrepareTimer = false;
	MinPlayerCount = 4;
	GoalNumOfKills = 10;
}

void AASMatchGameModeBase::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	int32 MaxPlayerCnt = GetMaxPlayerCount();
	if (!ensure(MinPlayerCount <= MaxPlayerCnt))
	{
		MinPlayerCount = MaxPlayerCnt;
	}
}

void AASMatchGameModeBase::InitGameState()
{
	Super::InitGameState();

	ASMatchGameState = Cast<AASMatchGameStateBase>(GameState);
	if (ensure(IsValid(ASMatchGameState)))
	{
		ASMatchGameState->SetMaxNumPlayers(GetMaxPlayerCount());
		ASMatchGameState->SetMinNumPlayers(MinPlayerCount);
		ASMatchGameState->SetGoalNumOfKills(GoalNumOfKills);
		ASMatchGameState->SetInnerMatchState(EInnerMatchState::Prepare);
	}

	if (auto GameInstance = GetGameInstance<UASGameInstance>())
	{
		GameInstance->SetPreparedMatchToSession(false);
	}
}

void AASMatchGameModeBase::PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
	Super::PreLogin(Options, Address, UniqueId, ErrorMessage);

	if (!ErrorMessage.IsEmpty())
		return;

	if (NumPlayers >= GetMaxPlayerCount())
	{
		ErrorMessage = TEXT("Server is Full");
		return;
	}

	if (IsValid(ASMatchGameState))
	{
		if (bSetPrepareTimer || ASMatchGameState->GetInnerMatchState() != EInnerMatchState::Prepare)
		{
			ErrorMessage = TEXT("Match is Already Started");
			return;
		}
	}
	else
	{
		ErrorMessage = TEXT("Invalid GameState");
		return;
	}
}

void AASMatchGameModeBase::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (ensure(IsValid(ASMatchGameState)))
	{
		if (ASMatchGameState->GetInnerMatchState() == EInnerMatchState::Prepare)
		{
			if (NumPlayers >= MinPlayerCount)
			{
				if (GetWorldTimerManager().IsTimerActive(PlayerWaitingTimerHandle))
				{
					GetWorldTimerManager().ClearTimer(PlayerWaitingTimerHandle);
				}

				GetWorldTimerManager().SetTimer(PlayerWaitingTimerHandle, this, &AASMatchGameModeBase::SetPrepareTimer, 10.0f);
			}
		}
	}
}

void AASMatchGameModeBase::InitStartSpot_Implementation(AActor* StartSpot, AController* NewPlayer)
{
	Super::InitStartSpot_Implementation(StartSpot, NewPlayer);

	if (ensure(IsValid(NewPlayer)))
	{
		if (ensure(IsValid(StartSpot)))
		{
			APawn* Pawn = NewPlayer->GetPawn();
			if (ensure(IsValid(Pawn)))
			{
				FRotator InitialControllerRot = StartSpot->GetActorRotation();
				InitialControllerRot.Roll = 0.f;

				Pawn->TeleportTo(StartSpot->GetActorLocation(), InitialControllerRot);

				NewPlayer->StartSpot = StartSpot;
			}
		}
	}	
}

void AASMatchGameModeBase::SetPlayerDefaults(APawn* PlayerPawn)
{
	Super::SetPlayerDefaults(PlayerPawn);

	if (ensure(IsValid(ASMatchGameState)))
	{
		if (ASMatchGameState->GetInnerMatchState() != EInnerMatchState::Prepare)
		{
			auto Character = Cast<AASCharacter>(PlayerPawn);
			if (IsValid(Character))
			{
				Character->TurnOnInvincible(RespawnInvicibleTimeSec);
			}
		}
	}
}

int32 AASMatchGameModeBase::GetMaxPlayerCount() const
{
	if (ensure(GameSession != nullptr))
	{
		return GameSession->MaxPlayers;
	}

	return 16;
}

int32 AASMatchGameModeBase::GetMinPlayerCount() const
{
	return MinPlayerCount;
}

int32 AASMatchGameModeBase::GetGoalNumOfKills() const
{
	return GoalNumOfKills;
}

void AASMatchGameModeBase::ProcessMatch()
{
	PrepareAllPlayerStart();

	if (ensure(IsValid(ASMatchGameState)))
	{
		ASMatchGameState->SetInnerMatchState(EInnerMatchState::Process);
	}

	SetProcessTimer();
}

void AASMatchGameModeBase::FinishMatch()
{
	if (!ensure(IsValid(ASMatchGameState)))
		return;

	if (ASMatchGameState->GetInnerMatchState() != EInnerMatchState::Process)
		return;

	if (GetWorldTimerManager().IsTimerActive(MatchFinishTimeHandle))
	{
		GetWorldTimerManager().ClearTimer(MatchFinishTimeHandle);
	}

	ASMatchGameState->SetInnerMatchState(EInnerMatchState::Finish);

	SetRestartTimer();
}

void AASMatchGameModeBase::OnKillCharacter(AASPlayerController* KillerController, AASPlayerController* DeadController)
{
	AASPlayerState* DeadPlayerState = nullptr;
	if (ensure(IsValid(DeadController)))
	{
		DeadPlayerState = DeadController->GetPlayerState<AASPlayerState>();
		if (ensure(IsValid(DeadPlayerState)))
		{
			DeadPlayerState->OnDie();
		}
	}

	AASPlayerState* KillerPlayerState = nullptr;
	int32 KillCount = -1;
	if (ensure(IsValid(KillerController)))
	{
		KillerPlayerState = KillerController->GetPlayerState<AASPlayerState>();
		if (ensure(IsValid(KillerPlayerState)))
		{
			KillerPlayerState->OnKill();
			KillCount = KillerPlayerState->GetKillCount();
		}
	}

	if (ensure(IsValid(ASMatchGameState)))
	{
		ASMatchGameState->MulticastOnKill(KillerPlayerState, DeadPlayerState, KillCount);
	}
}

void AASMatchGameModeBase::SetPrepareTimer()
{
	if (!ensure(IsValid(ASMatchGameState)))
		return;

	if (ASMatchGameState->GetInnerMatchState() != EInnerMatchState::Prepare)
		return;

	if (GetWorldTimerManager().IsTimerActive(PlayerWaitingTimerHandle))
	{
		GetWorldTimerManager().ClearTimer(PlayerWaitingTimerHandle);
	}

	bSetPrepareTimer = true;

	if (auto GameInstance = GetGameInstance<UASGameInstance>())
	{
		GameInstance->SetPreparedMatchToSession(true);
	}

	float PrepareTimeSec = PrepareTime.GetTotalSeconds();
	GetWorldTimerManager().SetTimer(PrepareTimerHandle, this, &AASMatchGameModeBase::ProcessMatch, PrepareTimeSec);

	float StartTime = ASMatchGameState->GetServerWorldTimeSeconds() + PrepareTimeSec;
	ASMatchGameState->SetStartTimeForProcess(StartTime);
}

void AASMatchGameModeBase::SetProcessTimer()
{
	float MatchProcessTimeSec = MatchProcessTime.GetTotalSeconds();
	GetWorldTimerManager().SetTimer(MatchFinishTimeHandle, this, &AASMatchGameModeBase::FinishMatch, MatchProcessTimeSec);

	if (ensure(IsValid(ASMatchGameState)))
	{
		float FinishTime = ASMatchGameState->GetServerWorldTimeSeconds() + MatchProcessTimeSec;
		ASMatchGameState->SetMatchFinishTime(FinishTime);
	}
}

void AASMatchGameModeBase::SetRestartTimer()
{
	float RestartTime = PostFinishTime.GetTotalSeconds();

	FTimerHandle TimerHandle;
	GetWorldTimerManager().SetTimer(TimerHandle, this, &AASMatchGameModeBase::OnCalledRestartTimer, RestartTime);

	if (ensure(IsValid(ASMatchGameState)))
	{
		float FinishTime = ASMatchGameState->GetServerWorldTimeSeconds() + RestartTime;
		ASMatchGameState->SetRestartTime(FinishTime);
	}
}

void AASMatchGameModeBase::OnCalledRestartTimer()
{
	RestartGame();
}

void AASMatchGameModeBase::PrepareAllPlayerStart()
{
}
