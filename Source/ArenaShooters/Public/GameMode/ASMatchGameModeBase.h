// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ArenaShooters.h"
#include "GameFramework/GameMode.h"
#include "ASMatchGameModeBase.generated.h"

class UASGameInstance;
class AASMatchGameStateBase;

UCLASS()
class ARENASHOOTERS_API AASMatchGameModeBase : public AGameMode
{
	GENERATED_BODY()
	
public:
	AASMatchGameModeBase();

	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;
	virtual void PreInitializeComponents() override;
	virtual void Tick(float DeltaSeconds) override;

	int32 GetMaxPlayerCount() const;
	void SetMaxPlayerCount(int32 Count);

	int32 GetMinPlayerCount() const;
	void SetMinPlayerCount(int32 Count);

protected:
	void SetPrepareTimer();
	void OnCalledPrepareTimer();

protected:
	UPROPERTY()
	UASGameInstance* ASGameInstance;

	UPROPERTY()
	AASMatchGameStateBase* ASMatchGameState;

	FTimerHandle PrepareTimerHandle;

	UPROPERTY(EditDefaultsOnly, Category = Setting)
	float PrepareTime;

	bool bSetPrepareTimer;

	UPROPERTY(EditDefaultsOnly, Category = Setting)
	int32 MaxPlayerCount;

	UPROPERTY(EditDefaultsOnly, Category = Setting)
	int32 MinPlayerCount;
};
