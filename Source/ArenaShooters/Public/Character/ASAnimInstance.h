// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ArenaShooters.h"
#include "Animation/AnimInstance.h"
#include "Common/ASEnums.h"
#include "ASAnimInstance.generated.h"

class AASCharacter;

UCLASS()
class ARENASHOOTERS_API UASAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	UASAnimInstance();

	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
	virtual void NativeBeginPlay() override;

	UFUNCTION(BlueprintPure, Meta = (BlueprintThreadSafe))
	bool IsActualSprinted() const;

	void PlayShootMontage(EWeaponType WeaponType);
	void PlayReloadMontage(EWeaponType WeaponType);
	void PlayEquipMontage(EWeaponType WeaponType);
	void PlayPickUpItemMontage();
	void PlayUseHealingKitMontage();
	void PlayHitReactMontage();

protected:
	UFUNCTION()
	void OnMontageEnd(UAnimMontage* Montage, bool bInterrupted);

	UFUNCTION()
	void AnimNotify_ReloadComplete();

	UFUNCTION()
	void AnimNotify_UseHealingKitComplete();

	UFUNCTION()
	void AnimNotify_HitReact();

public:
	DECLARE_EVENT(UASAnimInstance, FOnReloadCompleteEvent);
	FOnReloadCompleteEvent OnReloadComplete;

	DECLARE_EVENT(UASAnimInstance, FOnReloadEndEvent);
	FOnReloadEndEvent OnReloadEnd;

	DECLARE_EVENT(UASAnimInstance, FOnChangeWeaponEndEvent);
	FOnChangeWeaponEndEvent OnChangeWeaponEnd;

	DECLARE_EVENT(UASAnimInstance, FOnUseHealingKitCompleteEvent);
	FOnUseHealingKitCompleteEvent OnUseHealingKitComplete;

	DECLARE_EVENT(UASAnimInstance, FOnUseHealingKitEndEvent);
	FOnUseHealingKitEndEvent OnUseHealingKitEnd;

private:
	UPROPERTY()
	AASCharacter* ASChar;

	UPROPERTY(EditDefaultsOnly, Category = Montage, Meta = (AllowPrivateAccess = true))
	UAnimMontage* PistolShootMontage;

	UPROPERTY(EditDefaultsOnly, Category = Montage, Meta = (AllowPrivateAccess = true))
	UAnimMontage* ARShootMontage;

	UPROPERTY(EditDefaultsOnly, Category = Montage, Meta = (AllowPrivateAccess = true))
	UAnimMontage* PistolReloadMontage;

	UPROPERTY(EditDefaultsOnly, Category = Montage, Meta = (AllowPrivateAccess = true))
	UAnimMontage* ARReloadMontage;

	UPROPERTY(EditDefaultsOnly, Category = Montage, Meta = (AllowPrivateAccess = true))
	UAnimMontage* PistolEquipMontage;

	UPROPERTY(EditDefaultsOnly, Category = Montage, Meta = (AllowPrivateAccess = true))
	UAnimMontage* AREquipMontage;

	UPROPERTY(EditDefaultsOnly, Category = Montage, Meta = (AllowPrivateAccess = true))
	UAnimMontage* PickUpItemMontage;

	UPROPERTY(EditDefaultsOnly, Category = Montage, Meta = (AllowPrivateAccess = true))
	UAnimMontage* UseHealingKitMontage;

	UPROPERTY(EditDefaultsOnly, Category = Montage, Meta = (AllowPrivateAccess = true))
	UAnimMontage* HitReactMontage;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Meta = (AllowPrivateAccess = true))
	bool bInAir;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Meta = (AllowPrivateAccess = true))
	float CurrentSpeed;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Meta = (AllowPrivateAccess = true))
	float Direction;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Meta = (AllowPrivateAccess = true))
	bool bCrouched;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Meta = (AllowPrivateAccess = true))
	bool bSprinted;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Meta = (AllowPrivateAccess = true))
	float MaxWalkSpeedCrouched;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Meta = (AllowPrivateAccess = true))
	float CurrentRotationSpeed;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Meta = (AllowPrivateAccess = true))
	float TurnValue;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Meta = (AllowPrivateAccess = true))
	EWeaponType CurrentWeaponType;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Meta = (AllowPrivateAccess = true))
	EShootingStanceType ShootingStance;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Meta = (AllowPrivateAccess = true))
	float AimYaw;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Meta = (AllowPrivateAccess = true))
	float AimPitch;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Meta = (AllowPrivateAccess = true))
	bool bLocallyControlled;

	UPROPERTY(EditDefaultsOnly, Meta = (AllowPrivateAccess = true))
	USoundWave* BodyHitSound;
};
