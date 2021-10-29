// Fill out your copyright notice in the Description page of Project Settings.


#include "DataAssets/ItemDataAssets/ASItemSetDataAsset.h"
#include "ASAssetManager.h"

UASItemSetDataAsset::UASItemSetDataAsset()
{
	AssetType = UASAssetManager::ItemSetAssetType;
}

FPrimaryAssetId UASItemSetDataAsset::GetPrimaryAssetId() const
{
	return FPrimaryAssetId(AssetType, GetFName());
}
