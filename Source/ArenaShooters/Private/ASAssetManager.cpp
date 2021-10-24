// Fill out your copyright notice in the Description page of Project Settings.


#include "ASAssetManager.h"
#include "DataAssets/ItemDataAssets/ASItemDataAsset.h"

const FPrimaryAssetType	UASAssetManager::WeaponAssetType = TEXT("Weapon");
const FPrimaryAssetType	UASAssetManager::ArmorAssetType = TEXT("Armor");
const FPrimaryAssetType UASAssetManager::AmmoAssetType = TEXT("Ammo");
const FPrimaryAssetType UASAssetManager::HealingKitAssetType = TEXT("HealingKit");
const FPrimaryAssetType UASAssetManager::DamageAssetType = TEXT("Damage");
const FPrimaryAssetType UASAssetManager::InventoryAssetType = TEXT("Inventory");
const FPrimaryAssetType UASAssetManager::MatchItemSetAssetType = TEXT("MatchItemSet");

UASAssetManager& UASAssetManager::Get()
{
	UASAssetManager* This = Cast<UASAssetManager>(GEngine->AssetManager);
	if (This == nullptr)
	{
		AS_LOG(Fatal, TEXT("Invalid AssetManager in DefaultEngine.ini, must be UASAssetManager!"));
		return *NewObject<UASAssetManager>();
	}

	return *This;
}
