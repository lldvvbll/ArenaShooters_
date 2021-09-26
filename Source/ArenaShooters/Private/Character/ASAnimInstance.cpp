// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/ASAnimInstance.h"
#include "Character/ASCharacter.h"

UASAnimInstance::UASAnimInstance()
{
	MaxWalkSpeedCrouched = 300.0f;
}

void UASAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!::IsValid(ASChar))
		return;
	
	bInAir = ASChar->GetCharacterMovement()->IsFalling();

	FVector CharVelocity = ASChar->GetVelocity();
	FRotator CharRotation = ASChar->GetActorRotation();

	CurrentSpeed = CharVelocity.Size();
	Direction = CalculateDirection(CharVelocity, CharRotation);
	bCrouched = ASChar->bIsCrouched;
	bSprinted = ASChar->IsSprinted();
	TurnValue = ASChar->GetTotalTurnValue();
	CurrentWeaponType = ASChar->GetUsingWeaponType();
	ShootingStance = ASChar->GetShootingStance();
	if (ShootingStance != EShootingStanceType::None)
	{
		FRotator AimRot = ASChar->GetAimOffsetRotator();
		AimYaw = FMath::ClampAngle(AimRot.Yaw, -90.0f, 90.0f);
		AimPitch = FMath::ClampAngle(AimRot.Pitch, -90.0f, 90.0f);
	}
}

void UASAnimInstance::NativeBeginPlay()
{
	Super::NativeBeginPlay();

	ASChar = Cast<AASCharacter>(TryGetPawnOwner());
	if (!::IsValid(ASChar))
		return;

	if (UCharacterMovementComponent* MoveComp = ASChar->GetCharacterMovement())
	{
		MaxWalkSpeedCrouched = MoveComp->MaxWalkSpeedCrouched;
	}

	bLocallyControlled = ASChar->IsLocallyControlled();

	OnMontageEnded.AddDynamic(this, &UASAnimInstance::OnMontageEnd);
}

bool UASAnimInstance::IsActualSprinted() const
{
	return  bSprinted && !bInAir && (CurrentSpeed > 800.0f);
}

void UASAnimInstance::PlayShootMontage()
{
	switch (CurrentWeaponType)
	{
	case EWeaponType::Pistol:
		{
			Montage_Play(PistolShootMontage);
		}
		break;
	case EWeaponType::AssaultRifle:
		{
			Montage_Play(ARShootMontage);
		}
		break;
	default:
		checkNoEntry();
		break;
	}
}

void UASAnimInstance::PlayReloadMontage()
{
	switch (CurrentWeaponType)
	{
	case EWeaponType::Pistol:
		{
			Montage_Play(PistolReloadMontage);
		}
		break;
	case EWeaponType::AssaultRifle:
		{
			Montage_Play(ARReloadMontage);
		}
		break;
	default:
		checkNoEntry();
		break;
	}
}

void UASAnimInstance::PlayEquipMontage()
{
	switch (CurrentWeaponType)
	{
	case EWeaponType::Pistol:
		{
			Montage_Play(PistolEquipMontage);
		}
		break;
	case EWeaponType::AssaultRifle:
		{
			Montage_Play(AREquipMontage);
		}
		break;
	default:
		checkNoEntry();
		break;
	}
}

void UASAnimInstance::PlayPickUpItemMontage()
{
	Montage_Play(PickUpItemMontage);
}

void UASAnimInstance::PlayUseHealingKitMontage()
{
	Montage_Play(UseHealingKitMontage);
}

void UASAnimInstance::PlayHitReactMontage()
{
	Montage_Play(HitReactMontage);
}

void UASAnimInstance::OnMontageEnd(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage == nullptr)
		return;

	if (Montage == PistolReloadMontage || Montage == ARReloadMontage)
	{
		OnReloadEnd.Broadcast();
	}
	else if (Montage == PistolEquipMontage || Montage == AREquipMontage)
	{
		OnChangeWeaponEnd.Broadcast();
	}
	else if (Montage == UseHealingKitMontage)
	{
		OnUseHealingKitEnd.Broadcast();
	}
}

void UASAnimInstance::AnimNotify_ReloadComplete()
{
	OnReloadComplete.Broadcast();
}

void UASAnimInstance::AnimNotify_UseHealingKitComplete()
{
	OnUseHealingKitComplete.Broadcast();
}

void UASAnimInstance::AnimNotify_HitReact()
{
	ASChar = Cast<AASCharacter>(TryGetPawnOwner());
	if (!::IsValid(ASChar))
		return;
	
	if (ASChar->IsLocallyControlled())
	{
		UGameplayStatics::SpawnSoundAtLocation(GetWorld(), BodyHitSound, ASChar->GetActorLocation());
	}	
}
