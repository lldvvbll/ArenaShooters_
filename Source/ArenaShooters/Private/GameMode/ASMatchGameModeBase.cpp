// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/ASMatchGameModeBase.h"
#include "GameMode/ASMatchGameStateBase.h"
#include "ASGameInstance.h"
#include "Controller/ASPlayerController.h"
#include "Controller/ASPlayerState.h"
#include "Common/ASEnums.h"

AASMatchGameModeBase::AASMatchGameModeBase()
{
	PrepareTime = 10.0f;
	bSetPrepareTimer = false;
	MaxPlayerCount = 16;
	MinPlayerCount = 1;
	GoalNumOfKills = 1;
}

void AASMatchGameModeBase::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (!ASGameInstance->IsMatchProcess())
	{
		if (!bSetPrepareTimer && NumPlayers >= MinPlayerCount)
		{
			bSetPrepareTimer = true;

			FTimerHandle TimerHandle;
			GetWorldTimerManager().SetTimer(TimerHandle, this, &AASMatchGameModeBase::SetPrepareTimer, 3.0f);
		}
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

void AASMatchGameModeBase::FinishMatch()
{
	if (IsValid(ASGameInstance))
	{
		ASGameInstance->SetInnerMatchState(EInnerMatchState::Finish);
	}
	else
	{
		AS_LOG_S(Error);
	}

	if (IsValid(ASMatchGameState))
	{
		ASMatchGameState->OnFinishMatch();
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
	if (IsValid(KillerController))
	{
		KillerPlayerState = KillerController->GetPlayerState<AASPlayerState>();
		if (IsValid(KillerPlayerState))
		{
			KillerPlayerState->OnKill();
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

	auto ASGameState = GetGameState<AASMatchGameStateBase>();
	if (IsValid(ASGameState))
	{
		ASGameState->MulticastOnKill(KillerPlayerState, DeadPlayerState);
	}
	else
	{
		AS_LOG_S(Error);
	}
}

void AASMatchGameModeBase::SetPrepareTimer()
{
	GetWorldTimerManager().SetTimer(PrepareTimerHandle, this, &AASMatchGameModeBase::OnCalledPrepareTimer, PrepareTime);

	if (ASMatchGameState != nullptr)
	{
		ASMatchGameState->MulticastOnSetPrepareTimer(PrepareTime);
	}
	else
	{
		AS_LOG_S(Error);
	}
}

void AASMatchGameModeBase::OnCalledPrepareTimer()
{
	if (IsValid(ASGameInstance))
	{
		ASGameInstance->SetInnerMatchState(EInnerMatchState::Process);
	}
	else
	{
		AS_LOG_S(Error);
	}
	
	RestartGame();
}
