// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/ASAmmo.h"
#include "DataAssets/ItemDataAssets/ASAmmoDataAsset.h"
#include "Net/UnrealNetwork.h"

void UASAmmo::SetCount(int32 NewCount)
{
	auto AmmoDA = Cast<UASAmmoDataAsset>(DataAsset);
	if (ensure(AmmoDA != nullptr))
	{
		Super::SetCount(FMath::Clamp<int32>(NewCount, 0, AmmoDA->MaxBundleCount));
	}
	else
	{
		Super::SetCount(FMath::Clamp<int32>(NewCount, 0, 100));
	}	
}

EAmmoType UASAmmo::GetAmmoType() const
{
	auto AmmoDA = Cast<UASAmmoDataAsset>(DataAsset);
	if (ensure(AmmoDA != nullptr))
		return AmmoDA->AmmoType;

	return EAmmoType::None;
}
