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
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;
	virtual void PreInitializeComponents() override;

protected:
	UPROPERTY()
	AASMatchGameStateBase* ASMatchGameState;
};
