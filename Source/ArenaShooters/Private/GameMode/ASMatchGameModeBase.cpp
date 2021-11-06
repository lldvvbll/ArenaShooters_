// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/ASMatchGameModeBase.h"
#include "GameMode/ASMatchGameStateBase.h"
#include "ASGameInstance.h"
#include "Controller/ASPlayerController.h"
#include "Controller/ASPlayerState.h"
#include "Common/ASEnums.h"
#include "DataAssets/ItemDataAssets/ASItemSetDataAsset.h"
#include "Character/ASCharacter.h"

AASMatchGameModeBase::AASMatchGameModeBase()
{
	PrepareTime = 10.0f;
	MatchProcessTime = FTimespan::FromMinutes(15.0);
	bSetPrepareTimer = false;
	MaxPlayerCount = 16;
	MinPlayerCount = 1;
	GoalNumOfKills = 1;
}

void AASMatchGameModeBase::PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
	Super::PreLogin(Options, Address, UniqueId, ErrorMessage);

	if (ErrorMessage.Len() > 0)
		return;


}

void AASMatchGameModeBase::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (IsValid(ASMatchGameState))
	{
		if (ASMatchGameState->GetInnerMatchState() == EInnerMatchState::Prepare)
		{
			if (!bSetPrepareTimer && NumPlayers >= MinPlayerCount)
			{
				bSetPrepareTimer = true;

				FTimerHandle TimerHandle;
				GetWorldTimerManager().SetTimer(TimerHandle, this, &AASMatchGameModeBase::SetPrepareTimer, 3.0f);
			}
		}
	}
	else
	{
		AS_LOG_S(Error);
	}
}

void AASMatchGameModeBase::PreInitializeComponents()
{
	Super::PreInitializeComponents();

	ASGameInstance = GetGameInstance<UASGameInstance>();
	if (!IsValid(ASGameInstance))
	{
		AS_LOG_S(Error);
	}
}

void AASMatchGameModeBase::InitGameState()
{
	Super::InitGameState();

	ASMatchGameState = Cast<AASMatchGameStateBase>(GameState);
	if (IsValid(ASMatchGameState))
	{
		ASMatchGameState->SetMaxNumPlayers(MaxPlayerCount);
		ASMatchGameState->SetGoalNumOfKills(GoalNumOfKills);
	}
	else
	{
		AS_LOG_S(Error);
	}
}

void AASMatchGameModeBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

}

void AASMatchGameModeBase::InitStartSpot_Implementation(AActor* StartSpot, AController* NewPlayer)
{
	Super::InitStartSpot_Implementation(StartSpot, NewPlayer);

	if (IsValid(NewPlayer))
	{
		if (IsValid(StartSpot))
		{
			APawn* Pawn = NewPlayer->GetPawn();
			if (IsValid(Pawn))
			{
				FRotator InitialControllerRot = StartSpot->GetActorRotation();
				InitialControllerRot.Roll = 0.f;

				Pawn->TeleportTo(StartSpot->GetActorLocation(), InitialControllerRot);

				NewPlayer->StartSpot = StartSpot;
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
	else
	{
		AS_LOG_S(Error);
	}	
}

void AASMatchGameModeBase::SetPlayerDefaults(APawn* PlayerPawn)
{
	Super::SetPlayerDefaults(PlayerPawn);

	if (IsValid(ASMatchGameState))
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
	else
	{
		AS_LOG_S(Error);
	}
}

int32 AASMatchGameModeBase::GetMaxPlayerCount() const
{
	return MaxPlayerCount;
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

	if (IsValid(ASMatchGameState))
	{
		ASMatchGameState->SetInnerMatchState(EInnerMatchState::Process);
	}
	else
	{
		AS_LOG_S(Error);
	}

	SetProcessTimer();
}

void AASMatchGameModeBase::FinishMatch()
{
	if (GetWorldTimerManager().IsTimerActive(MatchFinishTimeHandle))
	{
		GetWorldTimerManager().ClearTimer(MatchFinishTimeHandle);
	}

	if (IsValid(ASMatchGameState))
	{
		ASMatchGameState->SetInnerMatchState(EInnerMatchState::Finish);
	}
	else
	{
		AS_LOG_S(Error);
	}

	if (NumPlayers >= MinPlayerCount)
	{
		SetRestartTimer();
	}	
}

void AASMatchGameModeBase::OnKillCharacter(AASPlayerController* KillerController, AASPlayerController* DeadController)
{
	AASPlayerState* DeadPlayerState = nullptr;
	if (IsValid(DeadController))
	{
		DeadPlayerState = DeadController->GetPlayerState<AASPlayerState>();
		if (IsValid(DeadPlayerState))
		{
			DeadPlayerState->OnDie();
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

	AASPlayerState* KillerPlayerState = nullptr;
	int32 KillCount = -1;
	if (IsValid(KillerController))
	{
		KillerPlayerState = KillerController->GetPlayerState<AASPlayerState>();
		if (IsValid(KillerPlayerState))
		{
			KillerPlayerState->OnKill();
			KillCount = KillerPlayerState->GetKillCount();
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

	if (IsValid(ASMatchGameState))
	{
		ASMatchGameState->MulticastOnKill(KillerPlayerState, DeadPlayerState, KillCount);
	}
	else
	{
		AS_LOG_S(Error);
	}
}

void AASMatchGameModeBase::SetPrepareTimer()
{
	float PrepareTimeSec = PrepareTime.GetTotalSeconds();
	GetWorldTimerManager().SetTimer(PrepareTimerHandle, this, &AASMatchGameModeBase::ProcessMatch, PrepareTimeSec);

	if (IsValid(ASMatchGameState))
	{
		float StartTime = ASMatchGameState->GetServerWorldTimeSeconds() + PrepareTimeSec;
		ASMatchGameState->SetStartTimeForProcess(StartTime);
	}
	else
	{
		AS_LOG_S(Error);
	}
}

void AASMatchGameModeBase::SetProcessTimer()
{
	float MatchProcessTimeSec = MatchProcessTime.GetTotalSeconds();
	GetWorldTimerManager().SetTimer(MatchFinishTimeHandle, this, &AASMatchGameModeBase::FinishMatch, MatchProcessTimeSec);

	if (IsValid(ASMatchGameState))
	{
		float FinishTime = ASMatchGameState->GetServerWorldTimeSeconds() + MatchProcessTimeSec;
		ASMatchGameState->SetMatchFinishTime(FinishTime);
	}
	else
	{
		AS_LOG_S(Error);
	}
}

void AASMatchGameModeBase::SetRestartTimer()
{
	float RestartTime = PostFinishTime.GetTotalSeconds();

	FTimerHandle TimerHandle;
	GetWorldTimerManager().SetTimer(TimerHandle, this, &AASMatchGameModeBase::OnCalledRestartTimer, RestartTime);

	if (IsValid(ASMatchGameState))
	{
		float FinishTime = ASMatchGameState->GetServerWorldTimeSeconds() + RestartTime;
		ASMatchGameState->SetRestartTime(FinishTime);
	}
	else
	{
		AS_LOG_S(Error);
	}
}

void AASMatchGameModeBase::OnCalledRestartTimer()
{
	RestartGame();
}

void AASMatchGameModeBase::PrepareAllPlayerStart()
{
}
