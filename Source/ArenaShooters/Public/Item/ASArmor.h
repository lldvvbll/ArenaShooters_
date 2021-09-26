// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ArenaShooters.h"
#include "Item/ASItem.h"
#include "Common/ASEnums.h"
#include "ASArmor.generated.h"

class UASArmorDataAsset;
class AASArmorActor;

UCLASS()
class ARENASHOOTERS_API UASArmor : public UASItem
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void SetDataAsset(UASItemDataAsset* NewDataAsset) override;

	const EArmorType GetArmorType() const;

	TWeakObjectPtr<AASArmorActor>& GetActor();
	const TWeakObjectPtr<AASArmorActor>& GetActor() const;

	float GetCurrentDurability() const;
	void SetCurrentDurability(float NewDurability);
	void ModifyDurability(float Value);
	float TakeDamage(float InDamage);
	bool IsCoveringBone(const FName& BoneName) const;

protected:
	UPROPERTY(Replicated, VisibleAnywhere)
	TWeakObjectPtr<AASArmorActor> ASArmorActor;

	UPROPERTY(Replicated)
	float CurrentDurability;
};
