// Fill out your copyright notice in the Description page of Project Settings.


#include "DataAssets/CharacterDataAssets/ASDamageDataAsset.h"
#include "ASAssetManager.h"

UASDamageDataAsset::UASDamageDataAsset()
{
	AssetType = UASAssetManager::DamageAssetType;
}

FPrimaryAssetId UASDamageDataAsset::GetPrimaryAssetId() const
{
	return FPrimaryAssetId(AssetType, GetFName());
}

float UASDamageDataAsset::GetDamageRateByBone(const USkinnedMeshComponent* MeshComp, const FName& BoneName) const
{
	if (MeshComp != nullptr)
	{
		for (auto& Pair : DamageRateByBoneMap)
		{
			if (BoneName == Pair.Key || MeshComp->BoneIsChildOf(BoneName, Pair.Key))
			{
				return Pair.Value;
			}
		}
	}

	return 1.0f;
}