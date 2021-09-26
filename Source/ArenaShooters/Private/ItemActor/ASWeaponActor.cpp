// Fill out your copyright notice in the Description page of Project Settings.


#include "ItemActor/ASWeaponActor.h"
#include "Net/UnrealNetwork.h"

const FName AASWeaponActor::MuzzleSocketName = TEXT("MuzzleFlash");

AASWeaponActor::AASWeaponActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetCanBeDamaged(false);

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	WeaponMesh->SetCollisionProfileName(TEXT("NoCollision"));

	ScopeCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("ScopeCamera"));

	RootComponent = WeaponMesh;
	ScopeCamera->SetupAttachment(RootComponent);
}

FVector AASWeaponActor::GetMuzzleLocation() const
{
	if (WeaponMesh != nullptr)
	{
		return WeaponMesh->GetSocketLocation(MuzzleSocketName);
	}
	else
	{
		AS_LOG_S(Error);
		return GetActorLocation();
	}	
}

void AASWeaponActor::GetMuzzleLocationAndRotation(FVector& OutLocation, FRotator& OutRotation) const
{
	if (WeaponMesh != nullptr)
	{	
		OutLocation = WeaponMesh->GetSocketLocation(MuzzleSocketName);
		OutRotation = WeaponMesh->GetSocketRotation(MuzzleSocketName);
	}
	else
	{
		AS_LOG_S(Error);
		OutLocation = GetActorLocation();
		OutRotation = GetActorRotation();
	}	
}

void AASWeaponActor::PlayFireAnim()
{	
	if (WeaponMesh != nullptr)
	{
		WeaponMesh->SetPosition(0.0f, false);
		WeaponMesh->Play(false);
	}
}
