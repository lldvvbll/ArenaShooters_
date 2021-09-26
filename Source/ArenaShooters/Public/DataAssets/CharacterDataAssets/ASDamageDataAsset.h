// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ArenaShooters.h"
#include "Engine/DataAsset.h"
#include "ASDamageDataAsset.generated.h"

UCLASS()
class ARENASHOOTERS_API UASDamageDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UASDamageDataAsset();

	virtual FPrimaryAssetId GetPrimaryAssetId() const override;

	float GetDamageRateByBone(const USkinnedMeshComponent* MeshComp, const FName& BoneName) const;

public:
	UPROPERTY(VisibleAnywhere)
	FPrimaryAssetType AssetType;

	UPROPERTY(EditDefaultsOnly, Category = Damage)
	TMap<FName, float> DamageRateByBoneMap;
};
