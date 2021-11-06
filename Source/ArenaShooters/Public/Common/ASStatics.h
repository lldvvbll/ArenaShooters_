// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ArenaShooters.h"
#include "ASStatics.generated.h"

class UASItem;
class UASWeapon;
class UASArmor;
class UASAmmo;
class UASHealingKit;

UCLASS()
class ARENASHOOTERS_API UASStatics : public UObject
{
	GENERATED_BODY()
		
public:
	static bool CompareItemForSorting(UASItem* Left, UASItem* Right);
	static bool CompareWeaponForSorting(UASWeapon* Left, UASWeapon* Right);
	static bool CompareArmorForSorting(UASArmor* Left, UASArmor* Right);
	static bool CompareAmmoForSorting(UASAmmo* Left, UASAmmo* Right);
	static bool CompareHealingKitForSorting(UASHealingKit* Left, UASHealingKit* Right);

};
