// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ArenaShooters.h"
#include "GameMode/ASMatchGameModeBase.h"
#include "ASDeathmatchGameMode.generated.h"

UCLASS()
class ARENASHOOTERS_API AASDeathmatchGameMode : public AASMatchGameModeBase
{
	GENERATED_BODY()

public:
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;

};
