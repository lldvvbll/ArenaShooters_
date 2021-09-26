// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/ASArmor.h"
#include "DataAssets/ItemDataAssets/ASArmorDataAsset.h"
#include "Net/UnrealNetwork.h"

void UASArmor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UASArmor, ASArmorActor);
	DOREPLIFETIME(UASArmor, CurrentDurability);
}

void UASArmor::SetDataAsset(UASItemDataAsset* NewDataAsset)
{
	Super::SetDataAsset(NewDataAsset);

	auto ArmorDA = Cast<UASArmorDataAsset>(NewDataAsset);
	check(ArmorDA);

	SetCurrentDurability(ArmorDA->MaxDurability);
}

const EArmorType UASArmor::GetArmorType() const
{
	auto ArmorDA = Cast<UASArmorDataAsset>(DataAsset);
	check(ArmorDA);

	return ArmorDA->ArmorType;
}

TWeakObjectPtr<AASArmorActor>& UASArmor::GetActor()
{
	return ASArmorActor;
}

const TWeakObjectPtr<AASArmorActor>& UASArmor::GetActor() const
{
	return ASArmorActor;
}

float UASArmor::GetCurrentDurability() const
{
	return CurrentDurability;
}

void UASArmor::SetCurrentDurability(float NewDurability)
{
	auto ArmorDA = Cast<UASArmorDataAsset>(DataAsset);
	check(ArmorDA);

	CurrentDurability = FMath::Clamp<float>(NewDurability, 0.0f, (ArmorDA != nullptr ? ArmorDA->MaxDurability : 100.0f));

	if (CurrentDurability < KINDA_SMALL_NUMBER)
	{
		AS_LOG(Warning, TEXT("Armor is broken!"));
	}
}

void UASArmor::ModifyDurability(float Value)
{
	SetCurrentDurability(CurrentDurability + Value);
}

float UASArmor::TakeDamage(float InDamage)
{
	float TakenDamage = InDamage;

	if (CurrentDurability > KINDA_SMALL_NUMBER)
	{
		auto ArmorDA = Cast<UASArmorDataAsset>(DataAsset);
		check(ArmorDA);

		if (ArmorDA != nullptr)
		{
			float ReducedDamage = TakenDamage * ArmorDA->DamageReduceRate;
			ModifyDurability(-ReducedDamage);

			TakenDamage -= ReducedDamage;

			AS_LOG(Warning, TEXT("%s was taken Damage. %f -> %f (-%f), durability: %f"), 
				*(ArmorDA->ItemName.ToString()), InDamage, TakenDamage, ReducedDamage, CurrentDurability);
		}
	}

	return TakenDamage;
}

bool UASArmor::IsCoveringBone(const FName& BoneName) const
{
	auto ArmorDA = Cast<UASArmorDataAsset>(DataAsset);
	check(ArmorDA);

	if (ArmorDA == nullptr)
		return false;

	return ArmorDA->CoveringBoneNames.Contains(BoneName);
}

