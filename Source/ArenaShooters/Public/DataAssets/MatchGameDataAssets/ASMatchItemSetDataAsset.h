// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ArenaShooters.h"
#include "Engine/DataAsset.h"
#include "ASMatchItemSetDataAsset.generated.h"

UCLASS()
class ARENASHOOTERS_API UASMatchItemSetDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UASMatchItemSetDataAsset();

	virtual FPrimaryAssetId GetPrimaryAssetId() const override;

public:
	UPROPERTY(VisibleAnywhere)
	FPrimaryAssetType AssetType;

	UPROPERTY(EditDefaultsOnly)
	FText SetName;

	UPROPERTY(EditDefaultsOnly)
	UTexture2D* ButtonImage;

	UPROPERTY(EditDefaultsOnly)
	TMap<FPrimaryAssetId, int32> ItemMap;
};
