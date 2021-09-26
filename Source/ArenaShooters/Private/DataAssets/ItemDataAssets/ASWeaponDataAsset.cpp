// Fill out your copyright notice in the Description page of Project Settings.


#include "DataAssets/ItemDataAssets/ASWeaponDataAsset.h"
#include "ASAssetManager.h"

UASWeaponDataAsset::UASWeaponDataAsset()
{
	AssetType = UASAssetManager::WeaponAssetType;
	ItemType = EItemType::Weapon;
}
