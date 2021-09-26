// Fill out your copyright notice in the Description page of Project Settings.


#include "DataAssets/ItemDataAssets/ASHealingKitDataAsset.h"
#include "ASAssetManager.h"

UASHealingKitDataAsset::UASHealingKitDataAsset()
{
	AssetType = UASAssetManager::HealingKitAssetType;
	ItemType = EItemType::HealingKit;
}
