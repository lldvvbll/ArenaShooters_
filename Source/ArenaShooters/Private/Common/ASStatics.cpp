// Fill out your copyright notice in the Description page of Project Settings.


#include "Common/ASStatics.h"
#include "Item/ASItem.h"
#include "Item/ASWeapon.h"
#include "Item/ASArmor.h"
#include "Item/ASAmmo.h"
#include "Item/ASHealingKit.h"

bool UASStatics::CompareItemForSorting(UASItem* Left, UASItem* Right)
{
	if (Left != nullptr && Right != nullptr)
	{
		EItemType LeftItemType = Left->GetItemType();
		EItemType RightItemType = Right->GetItemType();
		if (LeftItemType == RightItemType)
		{
			switch (LeftItemType)
			{
			case EItemType::Weapon:
				return CompareWeaponForSorting(Cast<UASWeapon>(Left), Cast<UASWeapon>(Right));
			case EItemType::Armor:
				return CompareArmorForSorting(Cast<UASArmor>(Left), Cast<UASArmor>(Right));
			case EItemType::Ammo:
				return CompareAmmoForSorting(Cast<UASAmmo>(Left), Cast<UASAmmo>(Right));
			case EItemType::HealingKit:
				return CompareHealingKitForSorting(Cast<UASHealingKit>(Left), Cast<UASHealingKit>(Right));
			default:
				checkNoEntry();
				break;
			}

			return true;
		}

		return LeftItemType < RightItemType;
	}
	else if (Right != nullptr)
	{
		return false;
	}

	return true;
}

bool UASStatics::CompareWeaponForSorting(UASWeapon* Left, UASWeapon* Right)
{
	if (Left != nullptr && Right != nullptr)
	{
		EWeaponType LeftType = Left->GetWeaponType();
		EWeaponType RightType = Right->GetWeaponType();
		if (LeftType < RightType)
		{
			return true;
		}
		else if (LeftType > RightType)
		{
			return false;
		}
		
		return GetTypeHash(Left->GetPrimaryAssetId()) <= GetTypeHash(Right->GetPrimaryAssetId());
	}
	else if (Right != nullptr)
	{
		return false;
	}

	return true;
}

bool UASStatics::CompareArmorForSorting(UASArmor* Left, UASArmor* Right)
{
	if (Left != nullptr && Right != nullptr)
	{
		EArmorType LeftType = Left->GetArmorType();
		EArmorType RightType = Right->GetArmorType();
		if (LeftType == RightType)
		{
			return GetTypeHash(Left->GetPrimaryAssetId()) <= GetTypeHash(Right->GetPrimaryAssetId());
		}

		return LeftType < RightType;
	}
	else if (Right != nullptr)
	{
		return false;
	}

	return true;
}

bool UASStatics::CompareAmmoForSorting(UASAmmo* Left, UASAmmo* Right)
{
	if (Left != nullptr && Right != nullptr)
	{
		EAmmoType LeftType = Left->GetAmmoType();
		EAmmoType RightType = Right->GetAmmoType();
		if (LeftType == RightType)
		{
			return GetTypeHash(Left->GetPrimaryAssetId()) <= GetTypeHash(Right->GetPrimaryAssetId());
		}

		return LeftType < RightType;
	}
	else if (Right != nullptr)
	{
		return false;
	}

	return true;
}

bool UASStatics::CompareHealingKitForSorting(UASHealingKit* Left, UASHealingKit* Right)
{
	if (Left != nullptr && Right != nullptr)
	{
		EHealingKitType LeftType = Left->GetHealingKitType();
		EHealingKitType RightType = Right->GetHealingKitType();
		if (LeftType == RightType)
		{
			return GetTypeHash(Left->GetPrimaryAssetId()) <= GetTypeHash(Right->GetPrimaryAssetId());
		}

		return LeftType < RightType;
	}
	else if (Right != nullptr)
	{
		return false;
	}

	return true;
}
