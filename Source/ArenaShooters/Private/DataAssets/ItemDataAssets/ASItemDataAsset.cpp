// Fill out your copyright notice in the Description page of Project Settings.


#include "DataAssets/ItemDataAssets/ASItemDataAsset.h"
#include "Common/ASEnums.h"

UASItemDataAsset::UASItemDataAsset()
{
	ItemType = EItemType::None;
}

FPrimaryAssetId UASItemDataAsset::GetPrimaryAssetId() const
{
	return FPrimaryAssetId(AssetType, GetFName());
}