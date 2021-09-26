// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/ASHealingKit.h"
#include "DataAssets/ItemDataAssets/ASHealingKitDataAsset.h"

void UASHealingKit::SetCount(int32 NewCount)
{
	auto HealingKitDA = Cast<UASHealingKitDataAsset>(DataAsset);
	check(HealingKitDA);

	Super::SetCount(FMath::Clamp<int32>(NewCount, 0, (HealingKitDA != nullptr ? HealingKitDA->MaxBundleCount : 100)));
}

EHealingKitType UASHealingKit::GetHealingKitType() const
{
	auto HealingKitDA = Cast<UASHealingKitDataAsset>(DataAsset);
	check(HealingKitDA);

	return (HealingKitDA != nullptr ? HealingKitDA->HealingKitType : EHealingKitType::None);
}

float UASHealingKit::GetRecoveryPoint() const
{
	auto HealingKitDA = Cast<UASHealingKitDataAsset>(DataAsset);
	check(HealingKitDA);

	return (HealingKitDA != nullptr ? HealingKitDA->RecoveryPoint : 0.0f);
}

FTimespan UASHealingKit::GetUsingTime() const
{
	auto HealingKitDA = Cast<UASHealingKitDataAsset>(DataAsset);
	check(HealingKitDA);

	return (HealingKitDA != nullptr ? HealingKitDA->UsingTime : FTimespan::MaxValue());
}
