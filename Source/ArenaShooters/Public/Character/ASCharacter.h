// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "ArenaShooters.h"
#include "GameFramework/Character.h"
#include "Common/ASEnums.h"
#include "ASCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UASActionComponent;
class UASInventoryComponent;
class UASStatusComponent;
class UASDamageComponent;
class UASAnimInstance;
class UASItem;
class UASWeapon;
class UASArmor;
class UASAmmo;
class UASHealingKit;
class AASWeaponActor;
class AASArmorActor;
class AASDroppedItemActor;

UCLASS()
class ARENASHOOTERS_API AASCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AASCharacter();

	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void Restart() override;

	virtual void Jump() override;
	virtual void Falling() override;
	virtual bool CanCrouch() const override;

	virtual void NotifyHit(class UPrimitiveComponent* MyComp, AActor* Other, class UPrimitiveComponent* OtherComp, bool bSelfMoved,
		FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit) override;
	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;
	virtual void NotifyActorEndOverlap(AActor* OtherActor) override;

	bool IsSprinted() const;
	float GetTotalTurnValue() const;
	EWeaponType GetUsingWeaponType() const;
	FRotator GetAimOffsetRotator() const;
	EShootingStanceType GetShootingStance() const;

	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlayShootMontage();
	void MulticastPlayShootMontage_Implementation();

	UASInventoryComponent* GetInventoryComponent();
	TArray<TWeakObjectPtr<UASItem>> GetGroundItems() const;

	UFUNCTION(Server, Reliable)
	void ServerPickUpWeapon(EWeaponSlotType SlotType, UASWeapon* NewWeapon);
	void ServerPickUpWeapon_Implementation(EWeaponSlotType SlotType, UASWeapon* NewWeapon);

	UFUNCTION(Server, Reliable)
	void ServerPickUpArmor(EArmorSlotType SlotType, UASArmor* NewArmor);
	void ServerPickUpArmor_Implementation(EArmorSlotType SlotType, UASArmor* NewArmor);

	UFUNCTION(Server, Reliable)
	void ServerDropItem(UASItem* InItem);
	void ServerDropItem_Implementation(UASItem* InItem);

	UFUNCTION(Server, Reliable)
	void ServerPickUpInventoryItem(UASItem* NewItem);
	void ServerPickUpInventoryItem_Implementation(UASItem* NewItem);

	bool RemoveItem(UASItem* InItem);

	void OnConstructedFullScreenWidget(UUserWidget* ConstructedWidget);
	void OnDestructedFullScreenWidget(UUserWidget* DestructedWidget);
	bool IsShownFullScreenWidget() const;

	void PickUpWeapon(EWeaponSlotType SlotType, UASWeapon* NewWeapon);
	void PickUpArmor(EArmorSlotType SlotType, UASArmor* NewArmor);
	void PickUpInventoryItem(UASItem* NewItem);
	void DropItem(UASItem* InItem);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlayPickUpItemMontage();
	void MulticastPlayPickUpItemMontage_Implementation();

	UASStatusComponent* GetStatusComponent();
	bool IsDead() const;

	void StartRagdoll();
	void EndRagdoll();

protected:
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode = 0) override;

	void MoveForward(float Value);
	void MoveRight(float Value);
	void Turn(float Value);
	void TurnAtRate(float Rate);
	void LookUpAtRate(float Rate);

	void Sprint();
	void SprintEnd();
	void ToggleCrouch();
	void PressedAimButton();
	void ReleasedAimButton();
	void SelectMainWeapon();
	void SelectSubWeapon();
	void PressedShootButton();
	void ReleasedShootButton();
	void ChangeFireMode();
	void Reload();
	void HealingKit();
	void DoFunction();

	void Shoot();
	void ResetAimKeyState();

	UFUNCTION(Server, Reliable)
	void ServerSprint();
	void ServerSprint_Implementation();

	UFUNCTION(Server, Reliable)
	void ServerSprintEnd();
	void ServerSprintEnd_Implementation();

	UFUNCTION()
	void OnRep_bSprinted();

	void SetMaxWalkSpeedRate(float Rate);

	UFUNCTION(Server, Reliable)
	void ServerSetTurnValue(float NewTurnValue);
	void ServerSetTurnValue_Implementation(float NewTurnValue);

	UFUNCTION(Server, Reliable)
	void ServerSetTurnRateValue(float NewTurnRateValue);
	void ServerSetTurnRateValue_Implementation(float NewTurnRateValue);

	UFUNCTION(Server, Reliable)
	void ServerSelectWeapon(EWeaponSlotType WeaponSlotType);
	void ServerSelectWeapon_Implementation(EWeaponSlotType WeaponSlotType);

	UFUNCTION(Server, Reliable)
	void ServerEndSelectWeapon();
	void ServerEndSelectWeapon_Implementation();

	UFUNCTION()
	void OnRep_bChangeWeapon();

	UFUNCTION(Server, Reliable)
	void ServerChangeShootingStance(EShootingStanceType NewShootingStance);
	void ServerChangeShootingStance_Implementation(EShootingStanceType NewShootingStance);

	UFUNCTION()
	void OnRep_ShootingStance(EShootingStanceType OldShootingStance);

	bool CanAimOrScope() const;
	void StartAiming();
	void EndAiming();
	void StartScoping();
	void EndScoping();

	UFUNCTION(Server, Reliable)
	void ServerShoot(const FVector& MuzzleLocation, const FRotator& ShootRotation);
	bool ServerShoot_Validate(const FVector& MuzzleLocation, const FRotator& ShootRotation);
	void ServerShoot_Implementation(const FVector& MuzzleLocation, const FRotator& ShootRotation);

	UFUNCTION(Server, Reliable)
	void ServerChangeFireMode();
	void ServerChangeFireMode_Implementation();

	void SpawnDroppedItemActor(UASItem* DroppingItem);
	void OnRemoveGroundItem(const TWeakObjectPtr<UASItem>& Item);

	UFUNCTION(Server, Reliable)
	void ServerBeginReload();
	void ServerBeginReload_Implementation();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerCompleteReload();
	bool ServerCompleteReload_Validate();
	void ServerCompleteReload_Implementation();

	UFUNCTION(Server, Reliable)
	void ServerEndReload();
	void ServerEndReload_Implementation();

	void EndReload();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastCancelReload();
	void MulticastCancelReload_Implementation();

	UFUNCTION()
	void OnRep_bReloading(bool OldbReloading);

	void Die();

	UFUNCTION()
	void OnRep_bDead();

	UFUNCTION(Server, Reliable)
	void ServerBeginHealingKit(UASHealingKit* InHealingKit);
	void ServerBeginHealingKit_Implementation(UASHealingKit* InHealingKit);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerCompleteHealingKit();
	bool ServerCompleteHealingKit_Validate();
	void ServerCompleteHealingKit_Implementation();

	UFUNCTION(Server, Reliable)
	void ServerEndHealingKit();
	void ServerEndHealingKit_Implementation();

	void EndHealingKit();

	UFUNCTION()
	void OnRep_bUseHealingKit(bool OldbUseHealingKit);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastCancelUseHealingKit();
	void MulticastCancelUseHealingKit_Implementation();

	void OnChangeSelectedWeapon(const TWeakObjectPtr<UASWeapon>& InOldWeapon, const TWeakObjectPtr<UASWeapon>& InNewWeapon);

	void OnChangedInnerMatchState(EInnerMatchState State);

public:
	DECLARE_EVENT_OneParam(AASCharacter, FOnScopeEvent, const TWeakObjectPtr<UASWeapon>&)
	FOnScopeEvent OnScope;

	DECLARE_EVENT(AASCharacter, FOnUnscopeEvent)
	FOnUnscopeEvent OnUnscope;

	DECLARE_EVENT_OneParam(AASCharacter, FOnGroundItemAddEvent, const TArray<TWeakObjectPtr<UASItem>>&)
	FOnGroundItemAddEvent OnGroundItemAdd;

	DECLARE_EVENT_OneParam(AASCharacter, FOnGroundItemRemoveEvent, const TArray<TWeakObjectPtr<UASItem>>&)
	FOnGroundItemRemoveEvent OnGroundItemRemove;

	DECLARE_EVENT(AASCharacter, FOnPlayShootMontageEvent)
	FOnPlayShootMontageEvent OnPlayShootMontage;

private:
	UPROPERTY(VisibleAnywhere, Category = Camera, Meta = (AllowPrivateAccess = true))
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, Category = Camera, Meta = (AllowPrivateAccess = true))
	UCameraComponent* FollowCamera;

	UPROPERTY(VisibleAnywhere, Category = Action, Meta = (AllowPrivateAccess = true))
	UASActionComponent* ASAction;

	UPROPERTY(EditDefaultsOnly, Category = Inventory, Meta = (AllowPrivateAccess = true))
	UASInventoryComponent* ASInventory;

	UPROPERTY(EditDefaultsOnly, Category = Inventory, Meta = (AllowPrivateAccess = true))
	UASStatusComponent* ASStatus;

	UPROPERTY(EditDefaultsOnly)
	UBoxComponent* InteractionBox;

	UPROPERTY(EditDefaultsOnly, Category = Damage, Meta = (AllowPrivateAccess = true))
	UASDamageComponent* ASDamageComp;

	UPROPERTY()
	UASAnimInstance* ASAnimInstance;

	UPROPERTY(VisibleAnywhere, Category = Camera, Meta = (AllowPrivateAccess = true))
	float BaseTurnRate;

	UPROPERTY(VisibleAnywhere, Category = Camera, Meta = (AllowPrivateAccess = true))
	float BaseLookUpRate;

	UPROPERTY(EditDefaultsOnly, Category = Camera, Meta = (AllowPrivateAccess = true))
	FVector NormalCamOffset;

	UPROPERTY(EditDefaultsOnly, Category = Camera, Meta = (AllowPrivateAccess = true))
	FVector AimingCamOffset;

	UPROPERTY(EditDefaultsOnly, Category = Camera, Meta = (AllowPrivateAccess = true))
	float NormalCamArmLength;

	UPROPERTY(EditDefaultsOnly, Category = Camera, Meta = (AllowPrivateAccess = true))
	float AimingCamArmLength;

	UPROPERTY(ReplicatedUsing = OnRep_bSprinted)
	bool bSprinted;

	UPROPERTY(EditDefaultsOnly, Category = Movement, Meta = (AllowPrivateAccess = true))
	float SprintSpeedRate;

	UPROPERTY(EditDefaultsOnly, Category = Movement, Meta = (AllowPrivateAccess = true))
	float UseHealingKitSpeedRate;

	UPROPERTY(Replicated)
	float TurnValue;

	UPROPERTY(Replicated)
	float TurnRateValue;

	bool bPressedAimButton;
	float AimKeyHoldTime;

	UPROPERTY(EditDefaultsOnly, Category = Aiming, Meta = (AllowPrivateAccess = true))
	float MaxAimKeyHoldTime;

	UPROPERTY(Replicated)
	FRotator AimOffsetRotator;

	UPROPERTY(EditDefaultsOnly, Category = Aiming, Meta = (AllowPrivateAccess = true))
	float AimingSpeedRate;

	bool bPressedShootButton;

	UPROPERTY(ReplicatedUsing = OnRep_ShootingStance)
	EShootingStanceType ShootingStance;

	TSet<TPair<TWeakObjectPtr<AASDroppedItemActor>, FDelegateHandle>> GroundItemActorSet;

	UPROPERTY(ReplicatedUsing = OnRep_bReloading)
	bool bReloading;

	FDateTime ReloadStartTime;

	UPROPERTY(ReplicatedUsing = OnRep_bDead)
	bool bDead;

	UPROPERTY(ReplicatedUsing = OnRep_bChangeWeapon)
	bool bChangeWeapon;

	bool bShownFullScreenWidget;

	UPROPERTY(ReplicatedUsing = OnRep_bUseHealingKit)
	bool bUseHealingKit;

	UPROPERTY()
	UASHealingKit* UsingHealingKit;

	FDateTime HealingKitStartTime;

	UPROPERTY(EditDefaultsOnly)
	UParticleSystem* BloodParticle;
	
	float CurrentBulletSpread;
	float MinBulletSpread;
	float MaxBulletSpread;
	float BulletSpreadAmountPerShot;
	float BulletSpreadRecoverySpeed;


	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Test, Meta = (AllowPrivateAccess = true))
	FPrimaryAssetId TestARAssetId;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Test, Meta = (AllowPrivateAccess = true))
	FPrimaryAssetId TestPistolAssetId;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Test, Meta = (AllowPrivateAccess = true))
	FPrimaryAssetId TestArmorAssetId;
};

