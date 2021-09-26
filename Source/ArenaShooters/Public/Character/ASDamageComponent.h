// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ArenaShooters.h"
#include "Components/ActorComponent.h"
#include "ASDamageComponent.generated.h"

class UASStatusComponent;
class AASBullet;
class UASDamageDataAsset;

UCLASS()
class ARENASHOOTERS_API UASDamageComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UASDamageComponent();

	virtual void InitializeComponent() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void TakeBulletDamage(AASBullet* InBullet, const FHitResult& InHit);

private:
	UFUNCTION()
	void OnTakeDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser);

private:
	UPROPERTY()
	AASCharacter* ASChar;

	UPROPERTY()
	UASStatusComponent* ASStatus;

	UPROPERTY()
	UASInventoryComponent* ASInventory;

	UPROPERTY(EditDefaultsOnly, Meta = (AllowPrivateAccess = true))
	FPrimaryAssetId DamageAssetId;

	UPROPERTY()
	UASDamageDataAsset* DamageDataAsset;
};
