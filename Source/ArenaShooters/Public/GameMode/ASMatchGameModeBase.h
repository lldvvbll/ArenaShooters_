// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ArenaShooters.h"
#include "GameFramework/GameMode.h"
#include "ASMatchGameModeBase.generated.h"

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

	int32 GetMaxPlayerCount() const;
	void SetMaxPlayerCount(int32 Count);

	int32 GetMinPlayerCount() const;
	void SetMinPlayerCount(int32 Count);

protected:
	UPROPERTY()
	AASMatchGameStateBase* ASMatchGameState;

	int32 MaxPlayerCount;
	int32 MinPlayerCount;
};
