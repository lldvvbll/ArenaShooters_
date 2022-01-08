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

	if (ASChar == nullptr)
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
	InclineValue = ASChar->GetInclineValue();
}

void UASAnimInstance::NativeBeginPlay()
{
	Super::NativeBeginPlay();

	ASChar = Cast<AASCharacter>(TryGetPawnOwner());
	if (!ensure(ASChar != nullptr))
		return;

	UCharacterMovementComponent* MoveComp = ASChar->GetCharacterMovement();
	if (ensure(MoveComp != nullptr))
	{
		MaxWalkSpeedCrouched = MoveComp->MaxWalkSpeedCrouched;
	}

	bLocallyControlled = ASChar->IsLocallyControlled();

	OnMontageEnded.AddDynamic(this, &UASAnimInstance::OnMontageEnd);

	ASChar->MovementModeChangedDelegate.AddDynamic(this, &UASAnimInstance::OnMovementChanged);
}

bool UASAnimInstance::IsActualSprinted() const
{
	return  bSprinted && !bInAir && (CurrentSpeed > 800.0f);
}

void UASAnimInstance::PlayShootMontage(EWeaponType WeaponType)
{
	switch (WeaponType)
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

void UASAnimInstance::PlayReloadMontage(EWeaponType WeaponType)
{
	switch (WeaponType)
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

void UASAnimInstance::StopReloadMontage()
{
	if (Montage_IsPlaying(ARReloadMontage))
	{
		Montage_Stop(0.2f, ARReloadMontage);
	}

	if (Montage_IsPlaying(PistolReloadMontage))
	{
		Montage_Stop(0.2f, PistolReloadMontage);
	}
}

bool UASAnimInstance::IsPlayingReloadMontage() const
{
	return Montage_IsPlaying(ARReloadMontage) || Montage_IsPlaying(PistolReloadMontage);
}

void UASAnimInstance::PlayEquipWeaponMontage(EWeaponType WeaponType)
{
	switch (WeaponType)
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

bool UASAnimInstance::IsPlayingEquipWeaponMontage() const
{
	return Montage_IsPlaying(PistolEquipMontage) || Montage_IsPlaying(AREquipMontage);
}

void UASAnimInstance::PlayPickUpItemMontage()
{
	Montage_Play(PickUpItemMontage);
}

void UASAnimInstance::PlayUseHealingKitMontage()
{
	Montage_Play(UseHealingKitMontage);
}

void UASAnimInstance::StopUseHealingKitMontage()
{
	if (Montage_IsPlaying(UseHealingKitMontage))
	{
		Montage_Stop(0.2f, UseHealingKitMontage);
	}	
}

bool UASAnimInstance::IsPlayingUseHealingKitMontage() const
{
	return Montage_IsPlaying(UseHealingKitMontage);
}

void UASAnimInstance::PlayHitReactMontage()
{
	Montage_Play(HitReactMontage);
}

void UASAnimInstance::OnMovementChanged(ACharacter* Character, EMovementMode PrevMovementMode, uint8 PreviousCustomMode)
{
	if (GetWorld()->IsClient())
	{
		if (ensure(IsValid(Character)))
		{
			EMovementMode CurMovementMode = Character->GetCharacterMovement()->MovementMode.GetValue();
			switch (CurMovementMode)
			{
			case EMovementMode::MOVE_Walking:
			case EMovementMode::MOVE_NavWalking:
				if (PrevMovementMode == EMovementMode::MOVE_Falling)
				{
					UGameplayStatics::SpawnSoundAtLocation(GetWorld(), FootstepSound, Character->GetActorLocation());
				}
				break;
			default:
				break;
			}
		}
	}
}

void UASAnimInstance::OnMontageEnd(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage == nullptr)
		return;

	if (Montage == UseHealingKitMontage)
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
	if (!ensure(ASChar))
		return;
	
	if (ASChar->IsLocallyControlled())
	{
		UGameplayStatics::SpawnSoundAtLocation(GetWorld(), BodyHitSound, ASChar->GetActorLocation());
	}
}
