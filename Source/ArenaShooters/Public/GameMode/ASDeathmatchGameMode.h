// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ArenaShooters.h"
#include "GameMode/ASMatchGameModeBase.h"
#include "ASDeathmatchGameMode.generated.h"

class AASPlayerController;
class AASPlayerState;

UCLASS()
class ARENASHOOTERS_API AASDeathmatchGameMode : public AASMatchGameModeBase
{
	GENERATED_BODY()

public:
	virtual void OnKillCharacter(AASPlayerController* KillerController, AASPlayerController* DeadController) override;

protected:
	UPROPERTY(EditDefaultsOnly)
	FTimespan RespawnDelay;
};
