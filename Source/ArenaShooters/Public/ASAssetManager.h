// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ArenaShooters.h"
#include "Engine/AssetManager.h"
#include "ASAssetManager.generated.h"

class UASItemDataAsset;

UCLASS()
class ARENASHOOTERS_API UASAssetManager : public UAssetManager
{
	GENERATED_BODY()
	
public:
	static const FPrimaryAssetType WeaponAssetType;
	static const FPrimaryAssetType ArmorAssetType;
	static const FPrimaryAssetType AmmoAssetType;
	static const FPrimaryAssetType HealingKitAssetType;
	static const FPrimaryAssetType DamageAssetType;
	static const FPrimaryAssetType InventoryAssetType;

public:
	static UASAssetManager& Get();

	UASItemDataAsset* GetDataAsset(const FPrimaryAssetId& PrimaryAssetId, bool bLogWarning = true);

	template <typename T>
	T* GetDataAsset(const FPrimaryAssetId& PrimaryAssetId, bool bLogWarning = true)
	{
		T* DataAsset = GetPrimaryAssetObject<T>(PrimaryAssetId);
		if (DataAsset == nullptr)
		{
			DataAsset = ForceLoadDataAsset<T>(PrimaryAssetId, bLogWarning);
		}

		return DataAsset;
	}

	template <typename T>
	T* ForceLoadDataAsset(const FPrimaryAssetId& PrimaryAssetId, bool bLogWarning = true)
	{
		T* LoadedDataAsset = Cast<T>(GetPrimaryAssetPath(PrimaryAssetId).TryLoad());

		if (bLogWarning && LoadedDataAsset == nullptr)
		{
			AS_LOG(Warning, TEXT("Failed to load Asset for identifier %s!"), *PrimaryAssetId.ToString());
		}

		return LoadedDataAsset;
	}
};
