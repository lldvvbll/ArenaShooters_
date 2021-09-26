// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ArenaShooters.h"
#include "Engine/DataAsset.h"
#include "ASInventoryDataAsset.generated.h"

UCLASS()
class ARENASHOOTERS_API UASInventoryDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UASInventoryDataAsset();

	virtual FPrimaryAssetId GetPrimaryAssetId() const override;

public:
	UPROPERTY(VisibleAnywhere)
	FPrimaryAssetType AssetType;

	UPROPERTY(EditDefaultsOnly, Category = ActorAttach)
	FName UsingWeaponSocketName;

	UPROPERTY(EditDefaultsOnly, Category = ActorAttach)
	FName UsingWeaponPistolSocketName;

	UPROPERTY(EditDefaultsOnly, Category = ActorAttach)
	FName BackSocketName;

	UPROPERTY(EditDefaultsOnly, Category = ActorAttach)
	FName SideSocketName;

	UPROPERTY(EditDefaultsOnly, Category = ActorAttach)
	FName HelmetSocketName;

	UPROPERTY(EditDefaultsOnly, Category = ActorAttach)
	FName JacketSocketName;
};
