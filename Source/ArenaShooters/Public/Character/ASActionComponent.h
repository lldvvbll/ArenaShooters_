// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ArenaShooters.h"
#include "Components/ActorComponent.h"
#include "Common/ASEnums.h"
#include "ASActionComponent.generated.h"


UCLASS()
class ARENASHOOTERS_API UASActionComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UASActionComponent();
		
	void SetMovementState(EMovementState NewState);

private:
	UPROPERTY(VisibleAnywhere, Meta = (AllowPrivateAccess = true))
	EMovementState CurrentMovementState;
};
