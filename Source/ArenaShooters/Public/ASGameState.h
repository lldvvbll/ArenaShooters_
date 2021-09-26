// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ArenaShooters.h"
#include "GameFramework/GameStateBase.h"
#include "ASGameState.generated.h"

class AASItemFactory;

UCLASS()
class ARENASHOOTERS_API AASGameState : public AGameStateBase
{
	GENERATED_BODY()
	
public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostInitializeComponents() override;

	AASItemFactory* GetASItemFactory() const;

private:
	UPROPERTY(Replicated)
	AASItemFactory* ItemFactory;
};
