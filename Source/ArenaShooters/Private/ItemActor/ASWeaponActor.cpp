// Fill out your copyright notice in the Description page of Project Settings.


#include "ItemActor/ASWeaponActor.h"
#include "Net/UnrealNetwork.h"

const FName AASWeaponActor::MuzzleSocketName = TEXT("MuzzleFlash");
const FName AASWeaponActor::CameraSocketName = TEXT("CameraSocket");

AASWeaponActor::AASWeaponActor()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetCanBeDamaged(false);

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	WeaponMesh->SetCollisionProfileName(TEXT("NoCollision"));

	ScopeCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("ScopeCamera"));

	RootComponent = WeaponMesh;
	ScopeCamera->SetupAttachment(RootComponent, CameraSocketName);
}

void AASWeaponActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bBecomeViewTarget)
	{
		if (WeaponMesh != nullptr)
		{
			if (ScopeCamera != nullptr)
			{
				FRotator SocketRotator = WeaponMesh->GetSocketRotation(CameraSocketName);
				ScopeCamera->SetRelativeRotation(FRotator(0.0f, 0.0f, -SocketRotator.Roll));
			}
			else
			{
				AS_LOG_S(Error);
			}
		}
		else
		{
			AS_LOG_S(Error);
		}
	}	
}

void AASWeaponActor::BecomeViewTarget(APlayerController* PC)
{
	Super::BecomeViewTarget(PC);

	bBecomeViewTarget = true;
}

void AASWeaponActor::EndViewTarget(APlayerController* PC)
{
	Super::EndViewTarget(PC);

	bBecomeViewTarget = false;

	if (ScopeCamera != nullptr)
	{
		ScopeCamera->SetRelativeRotation(FRotator::ZeroRotator);
	}
	else
	{
		AS_LOG_S(Error);
	}
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
