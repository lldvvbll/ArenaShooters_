// Fill out your copyright notice in the Description page of Project Settings.


#include "ASAssetManager.h"

const FPrimaryAssetType	UASAssetManager::WeaponAssetType = TEXT("Weapon");
const FPrimaryAssetType	UASAssetManager::ArmorAssetType = TEXT("Armor");
const FPrimaryAssetType UASAssetManager::AmmoAssetType = TEXT("Ammo");
const FPrimaryAssetType UASAssetManager::HealingKitAssetType = TEXT("HealingKit");
const FPrimaryAssetType UASAssetManager::DamageAssetType = TEXT("Damage");
const FPrimaryAssetType UASAssetManager::InventoryAssetType = TEXT("Inventory");
const FPrimaryAssetType UASAssetManager::ItemSetAssetType = TEXT("ItemSet");

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

EItemType UASAssetManager::ConvertAssetIdToItemType(const FPrimaryAssetId& PrimaryAssetId)
{
	EItemType ItemType = EItemType::None;

	if (PrimaryAssetId.PrimaryAssetType == UASAssetManager::WeaponAssetType)
	{
		ItemType = EItemType::Weapon;
	}
	else if (PrimaryAssetId.PrimaryAssetType == UASAssetManager::ArmorAssetType)
	{
		ItemType = EItemType::Armor;
	}
	else if (PrimaryAssetId.PrimaryAssetType == UASAssetManager::AmmoAssetType)
	{
		ItemType = EItemType::Ammo;
	}
	else if (PrimaryAssetId.PrimaryAssetType == UASAssetManager::HealingKitAssetType)
	{
		ItemType = EItemType::HealingKit;
	}

	return ItemType;
}
