// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ArenaShooters.h"
#include "GameMode/ASMatchGameStateBase.h"
#include "ASDeathmatchGameState.generated.h"

class UASItemFactoryComponent;

UCLASS()
class ARENASHOOTERS_API AASDeathmatchGameState : public AASMatchGameStateBase
{
	GENERATED_BODY()
	
public:
	AASDeathmatchGameState();
};
