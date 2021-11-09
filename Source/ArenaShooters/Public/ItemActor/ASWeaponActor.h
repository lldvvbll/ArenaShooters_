// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ArenaShooters.h"
#include "GameFramework/Actor.h"
#include "ASWeaponActor.generated.h"

UCLASS()
class ARENASHOOTERS_API AASWeaponActor : public AActor
{
	GENERATED_BODY()
	
public:	
	AASWeaponActor();

	virtual void Tick(float DeltaSeconds) override;
	virtual void BecomeViewTarget(APlayerController* PC) override;
	virtual void EndViewTarget(APlayerController* PC) override;

	FVector GetMuzzleLocation() const;
	void GetMuzzleLocationAndRotation(FVector& OutLocation, FRotator& OutRotation) const;

	void PlayFireAnim();

protected:
	static const FName MuzzleSocketName;
	static const FName CameraSocketName;

protected:
	UPROPERTY(EditDefaultsOnly)
	USkeletalMeshComponent* WeaponMesh;

	UPROPERTY(EditDefaultsOnly)
	UCameraComponent* ScopeCamera;

	bool bBecomeViewTarget;
};
