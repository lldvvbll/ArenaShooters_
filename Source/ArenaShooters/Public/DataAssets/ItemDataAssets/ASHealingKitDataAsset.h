// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ArenaShooters.h"
#include "DataAssets/ItemDataAssets/ASItemDataAsset.h"
#include "Common/ASEnums.h"
#include "ASHealingKitDataAsset.generated.h"

class UASHealingKit;

UCLASS()
class ARENASHOOTERS_API UASHealingKitDataAsset : public UASItemDataAsset
{
	GENERATED_BODY()

public:
	using ItemClassBaseT = UASHealingKit;

public:
	UASHealingKitDataAsset();

public:
	UPROPERTY(EditDefaultsOnly)
	EHealingKitType HealingKitType;

	UPROPERTY(EditDefaultsOnly)
	float RecoveryPoint;

	UPROPERTY(EditDefaultsOnly)
	FTimespan UsingTime;
};
