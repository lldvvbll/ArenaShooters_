// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ArenaShooters.h"
#include "GameFramework/Actor.h"
#include "ASArmorActor.generated.h"

UCLASS()
class ARENASHOOTERS_API AASArmorActor : public AActor
{
	GENERATED_BODY()
	
public:
	AASArmorActor();

protected:
	UPROPERTY(EditDefaultsOnly)
	USceneComponent* EmptyRootComp;

	UPROPERTY(EditDefaultsOnly)
	UStaticMeshComponent* ArmorMesh;
};
