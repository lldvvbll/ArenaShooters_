// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/ASAmmo.h"
#include "DataAssets/ItemDataAssets/ASAmmoDataAsset.h"
#include "Net/UnrealNetwork.h"

void UASAmmo::SetCount(int32 NewCount)
{
	auto AmmoDA = Cast<UASAmmoDataAsset>(DataAsset);
	check(AmmoDA);

	Super::SetCount(FMath::Clamp<int32>(NewCount, 0, (AmmoDA != nullptr ? AmmoDA->MaxBundleCount : 100)));
}

EAmmoType UASAmmo::GetAmmoType() const
{
	auto AmmoDA = Cast<UASAmmoDataAsset>(DataAsset);
	check(AmmoDA);

	return (AmmoDA != nullptr ? AmmoDA->AmmoType : EAmmoType::None);
}
