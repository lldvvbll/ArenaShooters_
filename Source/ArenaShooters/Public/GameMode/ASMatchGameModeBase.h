// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ArenaShooters.h"
#include "GameFramework/GameMode.h"
#include "ASMatchGameModeBase.generated.h"

class UASGameInstance;
class AASMatchGameStateBase;
class AASPlayerController;

UCLASS()
class ARENASHOOTERS_API AASMatchGameModeBase : public AGameMode
{
	GENERATED_BODY()
	
public:
	AASMatchGameModeBase();

	virtual void InitGameState() override;
	virtual void PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void PreInitializeComponents() override;
	virtual void Tick(float DeltaSeconds) override;

	virtual void InitStartSpot_Implementation(AActor* StartSpot, AController* NewPlayer) override;
	virtual void SetPlayerDefaults(APawn* PlayerPawn) override;

	int32 GetMaxPlayerCount() const;
	int32 GetMinPlayerCount() const;
	int32 GetGoalNumOfKills() const;

	void ProcessMatch();
	void FinishMatch();
	virtual void OnKillCharacter(AASPlayerController* KillerController, AASPlayerController* DeadController);

protected:
	void SetPrepareTimer();
	void SetProcessTimer();
	void SetRestartTimer();
	void OnCalledRestartTimer();

	virtual void PrepareAllPlayerStart();

	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);

protected:
	UPROPERTY()
	UASGameInstance* ASGameInstance;

	UPROPERTY()
	AASMatchGameStateBase* ASMatchGameState;

	FTimerHandle PlayerWaitingTimerHandle;
	FTimerHandle PrepareTimerHandle;

	UPROPERTY(EditDefaultsOnly, Category = Setting)
	FTimespan PrepareTime;

	UPROPERTY(EditDefaultsOnly, Category = Setting)
	FTimespan MatchProcessTime;

	bool bSetPrepareTimer;

	UPROPERTY(EditDefaultsOnly, Category = Setting)
	int32 MaxPlayerCount;

	UPROPERTY(EditDefaultsOnly, Category = Setting)
	int32 MinPlayerCount;

	UPROPERTY(EditDefaultsOnly, Category = Setting)
	int32 GoalNumOfKills;

	FTimerHandle MatchFinishTimeHandle;

	UPROPERTY(EditDefaultsOnly, Category = Setting)
	FTimespan PostFinishTime;

	UPROPERTY(EditDefaultsOnly, Category = Setting)
	float RespawnInvicibleTimeSec;
};
