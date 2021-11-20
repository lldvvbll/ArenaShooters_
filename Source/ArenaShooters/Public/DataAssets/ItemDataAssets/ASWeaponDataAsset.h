// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ArenaShooters.h"
#include "DataAssets/ItemDataAssets/ASItemDataAsset.h"
#include "Common/ASEnums.h"
#include "ASWeaponDataAsset.generated.h"

class AASWeaponActor;
class AASBullet;
class UASWeapon;

UCLASS()
class ARENASHOOTERS_API UASWeaponDataAsset : public UASItemDataAsset
{
	GENERATED_BODY()
	
public:
	using ItemClassBaseT = UASWeapon;

public:
	UASWeaponDataAsset();

public:
	UPROPERTY(EditDefaultsOnly)
	EWeaponType WeaponType;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AASWeaponActor> ASWeaponActorClass;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AASBullet> ASBulletClass;

	UPROPERTY(EditDefaultsOnly)
	TArray<EFireMode> FireModes;

	UPROPERTY(EditDefaultsOnly)
	FTimespan FireInterval;

	UPROPERTY(EditDefaultsOnly)
	float Damage;

	UPROPERTY(EditDefaultsOnly)
	int32 MaxAmmoCount;

	UPROPERTY(EditDefaultsOnly)
	EAmmoType AmmoType;

	UPROPERTY(EditDefaultsOnly)
	FTimespan ReloadTime;

	UPROPERTY(EditDefaultsOnly)
	FVector2D RecoilPitch;

	UPROPERTY(EditDefaultsOnly)
	FVector2D RecoilYaw;

	UPROPERTY(EditDefaultsOnly)
	float MinBulletSpread;

	UPROPERTY(EditDefaultsOnly)
	float MaxBulletSpread;

	UPROPERTY(EditDefaultsOnly)
	float BulletSpreadAmountPerShot;

	UPROPERTY(EditDefaultsOnly)
	float BulletSpreadRecoverySpeed;

	UPROPERTY(EditDefaultsOnly)
	USoundWave* EmptyBulletSound;
};
