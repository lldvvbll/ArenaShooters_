// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ArenaShooters.h"
#include "Item/ASItem.h"
#include "Common/ASEnums.h"
#include "ASHealingKit.generated.h"

UCLASS()
class ARENASHOOTERS_API UASHealingKit : public UASItem
{
	GENERATED_BODY()

public:
	virtual void SetCount(int32 NewCount) override;

	EHealingKitType GetHealingKitType() const;

	float GetRecoveryPoint() const;
	FTimespan GetUsingTime() const;
};
