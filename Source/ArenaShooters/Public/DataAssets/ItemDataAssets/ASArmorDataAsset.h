// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ArenaShooters.h"
#include "DataAssets/ItemDataAssets/ASItemDataAsset.h"
#include "Common/ASEnums.h"
#include "ASArmorDataAsset.generated.h"

class AASArmorActor;
class UASArmor;

UCLASS()
class ARENASHOOTERS_API UASArmorDataAsset : public UASItemDataAsset
{
	GENERATED_BODY()

public:
	using ItemClassBaseT = UASArmor;

public:
	UASArmorDataAsset();

public:
	UPROPERTY(EditDefaultsOnly)
	EArmorType ArmorType;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AASArmorActor> ASArmorActorClass;

	UPROPERTY(EditDefaultsOnly)
	float MaxDurability;

	UPROPERTY(EditDefaultsOnly)
	float DamageReduceRate;

	UPROPERTY(EditDefaultsOnly)
	TSet<FName> CoveringBoneNames;
};
