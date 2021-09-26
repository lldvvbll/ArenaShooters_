// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ArenaShooters.h"
#include "GameFramework/Actor.h"
#include "ASBullet.generated.h"

class UProjectileMovementComponent;

UCLASS()
class ARENASHOOTERS_API AASBullet : public AActor
{
	GENERATED_BODY()
	
public:	
	AASBullet();

	virtual void NotifyHit(class UPrimitiveComponent* MyComp, AActor* Other, class UPrimitiveComponent* OtherComp, bool bSelfMoved,
		FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit) override;

	float GetDamage() const;
	void SetDamage(float NewDamage);

protected:
	UPROPERTY(EditDefaultsOnly)
	USphereComponent* Collision;

	UPROPERTY(EditDefaultsOnly)
	UProjectileMovementComponent* Projectile;

	UPROPERTY(EditDefaultsOnly)
	UParticleSystemComponent* TraceParticle;

	float Damage;

	UPROPERTY(EditDefaultsOnly)
	UParticleSystem* DefaultSparkParticle;

	UPROPERTY(EditDefaultsOnly)
	UMaterial* DefaultBulletHoleDecal;
};
