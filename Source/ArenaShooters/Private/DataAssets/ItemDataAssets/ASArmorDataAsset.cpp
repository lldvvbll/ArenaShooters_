// Fill out your copyright notice in the Description page of Project Settings.


#include "DataAssets/ItemDataAssets/ASArmorDataAsset.h"
#include "ASAssetManager.h"

UASArmorDataAsset::UASArmorDataAsset()
{
	AssetType = UASAssetManager::ArmorAssetType;
	ItemType = EItemType::Armor;
}
