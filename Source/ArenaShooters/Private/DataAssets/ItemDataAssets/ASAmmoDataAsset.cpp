// Fill out your copyright notice in the Description page of Project Settings.


#include "DataAssets/ItemDataAssets/ASAmmoDataAsset.h"
#include "ASAssetManager.h"

UASAmmoDataAsset::UASAmmoDataAsset()
{
	AssetType = UASAssetManager::AmmoAssetType;
	ItemType = EItemType::Ammo;
}
