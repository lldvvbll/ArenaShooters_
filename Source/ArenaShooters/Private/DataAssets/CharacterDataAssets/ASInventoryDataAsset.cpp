// Fill out your copyright notice in the Description page of Project Settings.


#include "DataAssets/CharacterDataAssets/ASInventoryDataAsset.h"
#include "ASAssetManager.h"

UASInventoryDataAsset::UASInventoryDataAsset()
{
	AssetType = UASAssetManager::InventoryAssetType;
}

FPrimaryAssetId UASInventoryDataAsset::GetPrimaryAssetId() const
{
	return FPrimaryAssetId(AssetType, GetFName());
}
