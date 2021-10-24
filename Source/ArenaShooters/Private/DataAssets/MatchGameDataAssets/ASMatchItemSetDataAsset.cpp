// Fill out your copyright notice in the Description page of Project Settings.


#include "DataAssets/MatchGameDataAssets/ASMatchItemSetDataAsset.h"
#include "ASAssetManager.h"

UASMatchItemSetDataAsset::UASMatchItemSetDataAsset()
{
	AssetType = UASAssetManager::MatchItemSetAssetType;
}

FPrimaryAssetId UASMatchItemSetDataAsset::GetPrimaryAssetId() const
{
	return FPrimaryAssetId(AssetType, GetFName());
}
