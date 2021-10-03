// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/ASMatchGameModeBase.h"
#include "GameMode/ASMatchGameStateBase.h"
#include "ASGameInstance.h"

AASMatchGameModeBase::AASMatchGameModeBase()
{
	PrepareTime = 10.0f;
	bSetPrepareTimer = false;
	MaxPlayerCount = 16;
	MinPlayerCount = 1;
}

void AASMatchGameModeBase::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	ASMatchGameState->SetNumPlayers(NumPlayers);

	if (ASGameInstance->IsMatchProcess())
	{
		if (ASMatchGameState != nullptr)
		{
			ASMatchGameState->SetMatchProcess(true);
		}
		else
		{
			AS_LOG_S(Warning);
		}
	}
	else
	{
		if (!bSetPrepareTimer && NumPlayers >= MinPlayerCount)
		{
			bSetPrepareTimer = true;

			FTimerHandle TimerHandle;
			GetWorldTimerManager().SetTimer(TimerHandle, this, &AASMatchGameModeBase::SetPrepareTimer, 3.0f);
		}
	}
}

void AASMatchGameModeBase::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	ASMatchGameState->SetNumPlayers(NumPlayers);
}

void AASMatchGameModeBase::PreInitializeComponents()
{
	Super::PreInitializeComponents();

	ASGameInstance = GetGameInstance<UASGameInstance>();
	if (ASGameInstance == nullptr)
	{
		AS_LOG_S(Warning);
	}

	ASMatchGameState = Cast<AASMatchGameStateBase>(GameState);
	if (ASMatchGameState != nullptr)
	{
		ASMatchGameState->SetMaxNumPlayers(MaxPlayerCount);
	}
	else
	{
		AS_LOG_S(Warning);
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

void AASMatchGameModeBase::SetMaxPlayerCount(int32 Count)
{
	MaxPlayerCount = Count;
}

int32 AASMatchGameModeBase::GetMinPlayerCount() const
{
	return MinPlayerCount;
}

void AASMatchGameModeBase::SetMinPlayerCount(int32 Count)
{
	MinPlayerCount = Count;
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
		AS_LOG_S(Warning);
	}
}

void AASMatchGameModeBase::OnCalledPrepareTimer()
{
	if (ASGameInstance != nullptr)
	{
		ASGameInstance->SetIsMatchProcess(true);
	}
	else
	{
		AS_LOG_S(Warning);
	}
	
	RestartGame();
}
