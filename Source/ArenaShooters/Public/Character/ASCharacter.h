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

	virtual void SetPlayerDefaults() override;

	virtual void Jump() override;
	virtual void Falling() override;
	virtual bool CanCrouch() const override;

	virtual void NotifyHit(class UPrimitiveComponent* MyComp, AActor* Other, class UPrimitiveComponent* OtherComp, bool bSelfMoved,
		FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit) override;
	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;
	virtual void NotifyActorEndOverlap(AActor* OtherActor) override;

	bool IsSprinted() const;
	float GetTotalTurnValue() const;
	float GetInclineValue() const;

	EWeaponType GetUsingWeaponType() const;
	TWeakObjectPtr<UASWeapon> GetUsingWeapon() const;
	TWeakObjectPtr<AASWeaponActor> GetUsingWeaponActor() const;

	FRotator GetAimOffsetRotator() const;
	EShootingStanceType GetShootingStance() const;
	UASInventoryComponent* GetInventoryComponent();
	TArray<TWeakObjectPtr<UASItem>> GetGroundItems() const;

	UFUNCTION(Server, Reliable)
	void ServerPickUpWeapon(EWeaponSlotType SlotType, UASWeapon* NewWeapon);
	void ServerPickUpWeapon_Implementation(EWeaponSlotType SlotType, UASWeapon* NewWeapon);

	UFUNCTION(Server, Reliable)
	void ServerPickUpArmor(EArmorSlotType SlotType, UASArmor* NewArmor);
	void ServerPickUpArmor_Implementation(EArmorSlotType SlotType, UASArmor* NewArmor);

	UFUNCTION(Server, Reliable)
	void ServerPickUpInventoryItem(UASItem* NewItem);
	void ServerPickUpInventoryItem_Implementation(UASItem* NewItem);

	UFUNCTION(Server, Reliable)
	void ServerDropItem(UASItem* InItem);
	void ServerDropItem_Implementation(UASItem* InItem);

	bool RemoveItem(UASItem* InItem);

	void OnConstructedFullScreenWidget(UUserWidget* ConstructedWidget);
	void OnDestructedFullScreenWidget(UUserWidget* DestructedWidget);
	bool IsShownFullScreenWidget() const;

	bool CanPickUpItem() const;
	void PickUpItem(UASItem* InItem);
	void PickUpWeapon(EWeaponSlotType SlotType, UASWeapon* NewWeapon);
	void PickUpArmor(EArmorSlotType SlotType, UASArmor* NewArmor);
	void PickUpInventoryItem(UASItem* NewItem);

	bool CanDropItem() const;
	void DropItem(UASItem* InItem);

	UASStatusComponent* GetStatusComponent();
	bool IsDead() const;

	void StartRagdoll();
	void EndRagdoll();

	void TurnOnInvincible(float Duration);
	void TurnOffInvincible();

	void StopAllActions();

protected:
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode = 0) override;

	void MoveForward(float Value);
	void MoveRight(float Value);
	void Turn(float Value);
	void TurnAtRate(float Rate);
	void LookUpAtRate(float Rate);
	void Incline(float Value);

	bool CanSprint() const;
	void Sprint();
	void SprintEnd();

	void ToggleCrouch();
	void PressedAimButton();
	void ReleasedAimButton();

	bool CanSelectWeapon() const;
	void SelectMainWeapon();
	void SelectSubWeapon();

	void PressedShootButton();
	void ReleasedShootButton();

	bool CanChangeFireMode() const;
	void ChangeFireMode();
	
	bool CanReload() const;
	void Reload();
	
	bool CanUseHealingKit() const;
	void UseHealingKit();

	void Interact();

	bool CanShoot() const;
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

	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlayChangeWeaponMontage(EWeaponType WeaponType);
	void MulticastPlayChangeWeaponMontage_Implementation(EWeaponType WeaponType);

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

	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlayShootMontage();
	void MulticastPlayShootMontage_Implementation();

	UFUNCTION(Server, Reliable)
	void ServerChangeFireMode();
	void ServerChangeFireMode_Implementation();

	void SpawnDroppedItemActor(UASItem* DroppingItem);
	void SpawnDroppedItemsActor(const TArray<UASItem*>& DroppingItems);
	void OnRemoveGroundItem(const TWeakObjectPtr<UASItem>& Item);

	UFUNCTION(Server, Reliable)
	void ServerBeginReload();
	void ServerBeginReload_Implementation();

	void CompleteReload();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlayReloadMontage();
	void MulticastPlayReloadMontage_Implementation();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastStopReloadMontage();
	void MulticastStopReloadMontage_Implementation();

	void Die();

	UFUNCTION()
	void OnRep_bDead();

	UFUNCTION(Server, Reliable)
	void ServerBeginHealingKit(UASHealingKit* InHealingKit);
	void ServerBeginHealingKit_Implementation(UASHealingKit* InHealingKit);

	void CompleteUseHealingKit();
	void OnEndUseHealingKitMontage();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlayUseHealingKitMontage();
	void MulticastPlayUseHealingKitMontage_Implementation();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastStopUseHealingKitMontage();
	void MulticastStopUseHealingKitMontage_Implementation();

	void OnChangeSelectedWeapon(const TWeakObjectPtr<UASWeapon>& InOldWeapon, const TWeakObjectPtr<UASWeapon>& InNewWeapon);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlayPickUpItemMontage();
	void MulticastPlayPickUpItemMontage_Implementation();

	void OnChangedInnerMatchState(EInnerMatchState State);

	void HighlightingPickableActor();
	AActor* FindPickableActor() const;
	bool IsInteractableActor(AActor* InActor) const;

	UFUNCTION()
	void OnRep_bInvincible();

	UFUNCTION(Server, Reliable)
	void ServerInclineValue(float Value);
	void ServerInclineValue_Implementation(float Value);

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

	DECLARE_EVENT_OneParam(AASCharacter, FOnTracePickingUpEvent, AActor*)
	FOnTracePickingUpEvent OnTracePickingUp;

	DECLARE_EVENT_OneParam(AASCharacter, FOnChangedShootingStanceEvent, EShootingStanceType)
	FOnChangedShootingStanceEvent OnChangedShootingStance;

protected:
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

	TSet<TWeakObjectPtr<AASDroppedItemActor>> GroundItemActorSet;

	UPROPERTY(ReplicatedUsing = OnRep_bDead)
	bool bDead;

	bool bShownFullScreenWidget;

	UPROPERTY()
	UASHealingKit* UsingHealingKit;

	FDateTime HealingKitStartTime;

	UPROPERTY(EditDefaultsOnly)
	UParticleSystem* BloodParticle;
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AASDroppedItemActor> DroppedItemBoxActorClass;

	float CurrentBulletSpread;
	float MinBulletSpread;
	float MaxBulletSpread;
	float BulletSpreadAmountPerShot;
	float BulletSpreadRecoverySpeed;

	bool bTracePickingUp;
	float PickingUpTraceElapseTime;
	float PickingUpTraceInterval;

	FTimerHandle InvincibleTimerHandle;

	UPROPERTY(ReplicatedUsing = OnRep_bInvincible)
	bool bInvincible;

	UPROPERTY(Replicated)
	float InclineValue;

	UPROPERTY(EditDefaultsOnly, Category = Camera, Meta = (AllowPrivateAccess = true))
	FVector AimingCamRightOffset;

	UPROPERTY(EditDefaultsOnly, Category = Camera, Meta = (AllowPrivateAccess = true))
	FVector AimingCamLeftOffset;

	UPROPERTY(EditDefaultsOnly, Category = Camera, Meta = (AllowPrivateAccess = true))
	float InclineSpeed;

	FVector CurrentAimingCamInclineOffset;
};

