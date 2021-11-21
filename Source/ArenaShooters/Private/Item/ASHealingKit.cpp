// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/ASHealingKit.h"
#include "DataAssets/ItemDataAssets/ASHealingKitDataAsset.h"

void UASHealingKit::SetCount(int32 NewCount)
{
	auto HealingKitDA = Cast<UASHealingKitDataAsset>(DataAsset);
	if (ensure(HealingKitDA != nullptr))
	{
		Super::SetCount(FMath::Clamp<int32>(NewCount, 0, HealingKitDA->MaxBundleCount));
	}
	else
	{
		Super::SetCount(FMath::Clamp<int32>(NewCount, 0, 100));
	}	
}

EHealingKitType UASHealingKit::GetHealingKitType() const
{
	auto HealingKitDA = Cast<UASHealingKitDataAsset>(DataAsset);
	if (ensure(HealingKitDA != nullptr))
		return HealingKitDA->HealingKitType;

	return EHealingKitType::None;
}

float UASHealingKit::GetRecoveryPoint() const
{
	auto HealingKitDA = Cast<UASHealingKitDataAsset>(DataAsset);
	if (ensure(HealingKitDA != nullptr))
		return HealingKitDA->RecoveryPoint;
	
	return 0.0f;
}

FTimespan UASHealingKit::GetUsingTime() const
{
	auto HealingKitDA = Cast<UASHealingKitDataAsset>(DataAsset);
	if (ensure(HealingKitDA != nullptr))
		return HealingKitDA->UsingTime;
	
	return FTimespan::MaxValue();
}
