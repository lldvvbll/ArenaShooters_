// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ArenaShooters.h"
#include "Engine/DataAsset.h"
#include "Common/ASEnums.h"
#include "ASItemDataAsset.generated.h"

class UASItem;
class AASDroppedItemActor;

UCLASS(Abstract)
class ARENASHOOTERS_API UASItemDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	UASItemDataAsset();

	virtual FPrimaryAssetId GetPrimaryAssetId() const override;

public:
	UPROPERTY(VisibleAnywhere)
	FPrimaryAssetType AssetType;

	UPROPERTY(VisibleAnywhere)
	EItemType ItemType;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UASItem> ItemClass;

	UPROPERTY(EditDefaultSOnly)
	FText ItemName;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AASDroppedItemActor> DroppedItemActorClass;

	UPROPERTY(EditDefaultsOnly)
	UTexture2D* ItemImage;

	UPROPERTY(EditDefaultsOnly)
	UTexture2D* EquipmentSlotImage;

	UPROPERTY(EditDefaultsOnly)
	int32 MaxBundleCount;

	UPROPERTY(EditDefaultsOnly)
	bool bBundle;
};
