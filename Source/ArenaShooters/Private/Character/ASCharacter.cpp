// Copyright Epic Games, Inc. All Rights Reserved.

#include "Character/ASCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Character/ASActionComponent.h"
#include "Character/ASInventoryComponent.h"
#include "Character/ASStatusComponent.h"
#include "Character/ASDamageComponent.h"
#include "Net/UnrealNetwork.h"
#include "ASAssetManager.h"
#include "DataAssets/ItemDataAssets/ASWeaponDataAsset.h"
#include "DataAssets/ItemDataAssets/ASArmorDataAsset.h"
#include "DataAssets/ItemDataAssets/ASItemSetDataAsset.h"
#include "Item/ASItem.h"
#include "Item/ASWeapon.h"
#include "Item/ASArmor.h"
#include "Item/ASAmmo.h"
#include "Item/ASHealingKit.h"
#include "ItemActor/ASWeaponActor.h"
#include "ItemActor/ASArmorActor.h"
#include "ItemActor/ASBullet.h"
#include "ItemActor/ASDroppedItemActor.h"
#include "GameFramework/PlayerInput.h"
#include "Character/ASAnimInstance.h"
#include "GameMode/ASItemFactoryComponent.h"
#include "GameMode/ASMatchGameStateBase.h"
#include "Controller/ASPlayerState.h"

AASCharacter::AASCharacter()
{
	bReplicates = true;

	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;
	NormalCamOffset = FVector(0.0f, 30.0f, 100.0f);
	NormalCamArmLength = 200.0f;
	AimingCamOffset = FVector(0.0f, 45.0f, 75.0f);
	AimingCamArmLength = 80.0f;
	AimingSpeedRate = 0.5f;
	SprintSpeedRate = 1.4f;
	UseHealingKitSpeedRate = 0.5f;
	bPressedAimButton = false;
	AimKeyHoldTime = 0.0f;
	MaxAimKeyHoldTime = 0.3f;
	ShootingStance = EShootingStanceType::None;
	bPressedShootButton = false;
	bShownFullScreenWidget = false;
	CurrentBulletSpread = TNumericLimits<float>::Max();
	MinBulletSpread = TNumericLimits<float>::Max();
	MaxBulletSpread = TNumericLimits<float>::Max();
	BulletSpreadAmountPerShot = TNumericLimits<float>::Max();
	BulletSpreadRecoverySpeed = 0.0f;
	bTracePickingUp = false;
	PickingUpTraceElapseTime = 0.0f;
	PickingUpTraceInterval = 0.1f;
	AimingCamRightOffset = FVector(0.0f, 60.0f, 70.0f);
	AimingCamLeftOffset = FVector(0.0f, -30.0f, 70.0f);
	InclineSpeed = 500.0f;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = NormalCamArmLength;
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->SocketOffset = NormalCamOffset;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	ASAction = CreateDefaultSubobject<UASActionComponent>(TEXT("ASAction"));
	ASInventory = CreateDefaultSubobject<UASInventoryComponent>(TEXT("ASInventory"));
	ASStatus = CreateDefaultSubobject<UASStatusComponent>(TEXT("ASStatus"));
	ASStatus->OnHealthZero.AddUObject(this, &AASCharacter::Die);

	InteractionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionBox"));
	InteractionBox->SetCollisionProfileName(TEXT("Interaction"));
	InteractionBox->SetBoxExtent(FVector(130.0f, 130.0f, 95.0f), false);
	InteractionBox->SetupAttachment(RootComponent);

	ASDamageComp = CreateDefaultSubobject<UASDamageComponent>(TEXT("DamageComp"));

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	UCapsuleComponent* Capsule = GetCapsuleComponent();
	Capsule->InitCapsuleSize(42.f, 96.0f);
	Capsule->SetCollisionProfileName(TEXT("ASCharacter"));

	UCharacterMovementComponent* CharMoveComp = GetCharacterMovement();
	CharMoveComp->JumpZVelocity = 500.0f;
	CharMoveComp->AirControl = 0.0f;
	CharMoveComp->GetNavAgentPropertiesRef().bCanCrouch = true;
	CharMoveComp->bCanWalkOffLedgesWhenCrouching = true;
	//CharMoveComp->bUseControllerDesiredRotation = true;
	//CharMoveComp->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
}

void AASCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (USkeletalMeshComponent* SkMesh = GetMesh())
	{
		ASAnimInstance = Cast<UASAnimInstance>(SkMesh->GetAnimInstance());
	}
	else
	{
		AS_LOG_S(Error);
	}

	if (ASInventory != nullptr)
	{
		ASInventory->OnChangedSelectedWeapon.AddUObject(this, &AASCharacter::OnChangeSelectedWeapon);
	}
	else
	{
		AS_LOG_S(Error);
	}
}

void AASCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	if (ASAnimInstance != nullptr)
	{
		if (GetLocalRole() == ROLE_Authority)
		{
			ASAnimInstance->OnReloadComplete.AddUObject(this, &AASCharacter::CompleteReload);
			ASAnimInstance->OnUseHealingKitComplete.AddUObject(this, &AASCharacter::CompleteUseHealingKit);
		}

		ASAnimInstance->OnUseHealingKitEnd.AddUObject(this, &AASCharacter::OnEndUseHealingKitMontage);
	}
	else
	{
		AS_LOG_S(Error);
	}

	auto GameState = GetWorld()->GetGameState<AASMatchGameStateBase>();
	if (IsValid(GameState))
	{
		GameState->OnChangedInnerMatchState.AddUObject(this, &AASCharacter::OnChangedInnerMatchState);

		OnChangedInnerMatchState(GameState->GetInnerMatchState());
	}
	else
	{
		AS_LOG_S(Error);
	}
}

void AASCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (ASInventory != nullptr)
	{
		ASInventory->ClearAllItems();
	}

	TArray<AActor*> AttachedActors;
	GetAttachedActors(AttachedActors, false);

	for (auto& AttachedActor : AttachedActors)
	{
		if (!IsValid(AttachedActor))
			continue;

		AttachedActor->Destroy();
	}

	Super::EndPlay(EndPlayReason);
}

void AASCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (IsLocallyControlled())
	{
		if (bPressedAimButton && CanAimOrScope())
		{
			AimKeyHoldTime += DeltaSeconds;
			if (AimKeyHoldTime >= MaxAimKeyHoldTime)
			{
				ResetAimKeyState();
				ServerChangeShootingStance(EShootingStanceType::Aiming);
			}
		}

		if (bPressedShootButton)
		{
			Shoot();
		}

		if (bTracePickingUp)
		{
			if (PickingUpTraceElapseTime < PickingUpTraceInterval)
			{
				PickingUpTraceElapseTime += DeltaSeconds;
			}
			else
			{
				HighlightingPickableActor();

				PickingUpTraceElapseTime = 0.0f;
			}
		}

		if (ShootingStance != EShootingStanceType::None)
		{
			AimOffsetRotator = (GetControlRotation() - GetActorRotation()).GetNormalized();
		}

		if (ShootingStance == EShootingStanceType::Aiming)
		{
			FVector TargetSocketOffset;
			if (InclineValue > 0.0f)
			{
				TargetSocketOffset = AimingCamRightOffset;
			}
			else if (InclineValue < 0.0f)
			{
				TargetSocketOffset = AimingCamLeftOffset;
			}
			else
			{
				TargetSocketOffset = AimingCamOffset;
			}

			CameraBoom->SocketOffset = FMath::VInterpConstantTo(CameraBoom->SocketOffset, TargetSocketOffset, DeltaSeconds, InclineSpeed);
		}
	}

	if (GetLocalRole() == ROLE_Authority)
	{
		if (ShootingStance != EShootingStanceType::None)
		{
			AimOffsetRotator = (GetControlRotation() - GetActorRotation()).GetNormalized();
		}

		CurrentBulletSpread = FMath::FInterpConstantTo(CurrentBulletSpread, MinBulletSpread, DeltaSeconds, BulletSpreadRecoverySpeed);
	}
}

void AASCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AASCharacter, bSprinted);
	DOREPLIFETIME_CONDITION(AASCharacter, TurnValue, COND_SimulatedOnly);
	DOREPLIFETIME_CONDITION(AASCharacter, TurnRateValue, COND_SimulatedOnly);
	DOREPLIFETIME_CONDITION(AASCharacter, AimOffsetRotator, COND_SimulatedOnly);
	DOREPLIFETIME(AASCharacter, ShootingStance);
	DOREPLIFETIME(AASCharacter, bDead);
	DOREPLIFETIME(AASCharacter, bInvincible);
	DOREPLIFETIME_CONDITION(AASCharacter, InclineValue, COND_SimulatedOnly);
}

void AASCharacter::SetPlayerDefaults()
{
	Super::SetPlayerDefaults();

	bDead = false;
	SetCanBeDamaged(true);

	StopAllActions();

	if (ASStatus != nullptr)
	{
		ASStatus->SetStatusDefaults();
	}
	else
	{
		AS_LOG_S(Error);
	}

	auto GameState = GetWorld()->GetGameState<AASMatchGameStateBase>();
	if (IsValid(GameState))
	{
		if (GameState->IsMatchProcess())
		{
			auto ASPlayerState = GetPlayerState<AASPlayerState>();
			if (IsValid(ASPlayerState))
			{
				if (ASInventory != nullptr)
				{
					ASInventory->ClearAllItems();

					UASItemSetDataAsset* DataAsset = ASPlayerState->GetItemSetDataAsset();
					ASInventory->EquipItemsByItemSetDataAsset(DataAsset);
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
	else
	{
		AS_LOG_S(Error);
	}

	EndRagdoll();
}

void AASCharacter::Jump()
{
	if (IsLocallyControlled())
	{
		if (bIsCrouched)
		{
			UnCrouch(true);
			return;
		}
	}

	Super::Jump();
}

void AASCharacter::Falling()
{
	Super::Falling();

	if (IsLocallyControlled())
	{
		if (bIsCrouched)
		{
			UnCrouch(true);
		}
	}	

	if (GetLocalRole() == ROLE_Authority)
	{
		if (ASAnimInstance != nullptr)
		{
			if (ASAnimInstance->IsPlayingUseHealingKitMontage())
			{
				MulticastStopUseHealingKitMontage();
			}
		}
		else
		{
			AS_LOG_S(Error);
		}
		
		if (ShootingStance != EShootingStanceType::None)
		{
			ServerChangeShootingStance(EShootingStanceType::None);
		}
	}
}

bool AASCharacter::CanCrouch() const
{
	if (GetCharacterMovement()->IsFalling())
		return false;

	return Super::CanCrouch();
}

void AASCharacter::NotifyHit(UPrimitiveComponent* MyComp, AActor* Other, UPrimitiveComponent* OtherComp, bool bSelfMoved,
	FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit)
{
	Super::NotifyHit(MyComp, Other, OtherComp, bSelfMoved, HitLocation, HitNormal, NormalImpulse, Hit);

	auto Bullet = Cast<AASBullet>(Other);
	bool bHitByBullet = (MyComp == GetMesh() && Bullet != nullptr);

	if (GetLocalRole() == ROLE_Authority)
	{
		if (bHitByBullet)
		{
			if (ASDamageComp != nullptr)
			{
				ASDamageComp->TakeBulletDamage(Bullet, Hit);
			}
			else
			{
				AS_LOG_S(Error);
			}
		}
	}
	else
	{
		if (bHitByBullet)
		{
			UGameplayStatics::SpawnEmitterAttached(BloodParticle, MyComp, NAME_None, HitLocation, HitNormal.ToOrientationRotator(),
				EAttachLocation::KeepWorldPosition);

			if (ASAnimInstance != nullptr)
			{
				ASAnimInstance->PlayHitReactMontage();
			}
			else
			{
				AS_LOG_S(Error);
			}
		}
	}
}

void AASCharacter::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	if (OtherActor == nullptr)
		return;

	if (IsLocallyControlled())
	{
		if (auto DroppedItemActor = Cast<AASDroppedItemActor>(OtherActor))
		{
			DroppedItemActor->OnRemoveItemEvent.AddUObject(this, &AASCharacter::OnRemoveGroundItem);
			GroundItemActorSet.Emplace(TWeakObjectPtr<AASDroppedItemActor>(MakeWeakObjectPtr(DroppedItemActor)));

			OnGroundItemAdd.Broadcast(DroppedItemActor->GetItems());

			if (!bTracePickingUp && GroundItemActorSet.Num() > 0)
			{
				bTracePickingUp = true;
				PickingUpTraceElapseTime = 0.0f;
			}
		}
	}	
}

void AASCharacter::NotifyActorEndOverlap(AActor* OtherActor)
{
	Super::NotifyActorEndOverlap(OtherActor);

	if (OtherActor == nullptr)
		return;

	if (IsLocallyControlled())
	{
		auto DroppedItemActor = Cast<AASDroppedItemActor>(OtherActor);
		if (IsValid(DroppedItemActor))
		{
			DroppedItemActor->ShowOutline(false);

			for (auto Itr = GroundItemActorSet.CreateIterator(); Itr; ++Itr)
			{
				if (Itr->IsValid() && Itr->Get() == DroppedItemActor)
				{
					DroppedItemActor->OnRemoveItemEvent.RemoveAll(this);
					Itr.RemoveCurrent();

					OnGroundItemRemove.Broadcast(DroppedItemActor->GetItems());
					break;
				}
			}

			if (bTracePickingUp && GroundItemActorSet.Num() <= 0)
			{
				bTracePickingUp = false;
			}
		}
	}
}

bool AASCharacter::IsSprinted() const
{
	return bSprinted;
}

float AASCharacter::GetTotalTurnValue() const
{
	return TurnValue + TurnRateValue;
}

float AASCharacter::GetInclineValue() const
{
	return InclineValue;
}

EWeaponType AASCharacter::GetUsingWeaponType() const
{
	if (ASInventory == nullptr)
	{
		AS_LOG_S(Error);
		return EWeaponType::None;
	}

	return ASInventory->GetSelectedWeaponType();
}

TWeakObjectPtr<UASWeapon> AASCharacter::GetUsingWeapon() const
{
	if (ASInventory == nullptr)
	{
		AS_LOG_S(Error);
		return TWeakObjectPtr<UASWeapon>();
	}

	return ASInventory->GetSelectedWeapon();
}

TWeakObjectPtr<AASWeaponActor> AASCharacter::GetUsingWeaponActor() const
{
	if (ASInventory == nullptr)
	{
		AS_LOG_S(Error);
		return TWeakObjectPtr<AASWeaponActor>();
	}

	return ASInventory->GetSelectedWeaponActor();
}

FRotator AASCharacter::GetAimOffsetRotator() const
{
	return AimOffsetRotator;
}

EShootingStanceType AASCharacter::GetShootingStance() const
{
	return ShootingStance;
}

UASInventoryComponent* AASCharacter::GetInventoryComponent()
{
	return ASInventory;
}

TArray<TWeakObjectPtr<UASItem>> AASCharacter::GetGroundItems() const
{
	TArray<TWeakObjectPtr<UASItem>> GroundItems;

	for (auto& Item : GroundItemActorSet)
	{
		if (!Item.IsValid())
			continue;

		GroundItems += Item->GetItems();
	}

	return GroundItems;
}

void AASCharacter::ServerPickUpWeapon_Implementation(EWeaponSlotType SlotType, UASWeapon* NewWeapon)
{
	if (!CanPickUpItem())
	{
		AS_LOG_S(Error);
		return;
	}

	if (ASInventory == nullptr)
	{
		AS_LOG_S(Error);
		return;
	}

	if (!ASInventory->IsSuitableWeaponSlot(SlotType, NewWeapon))
	{
		AS_LOG_S(Error);
		return;
	}

	auto DroppedItemActor = Cast<AASDroppedItemActor>(NewWeapon->GetOwner());
	if (!IsValid(DroppedItemActor))
	{
		AS_LOG_S(Error);
		return;
	}

	if (!IsInteractableActor(DroppedItemActor))
	{
		AS_LOG_S(Error);
		return;
	}

	if (GetShootingStance() != EShootingStanceType::None)
	{
		ServerChangeShootingStance(EShootingStanceType::None);
	}

	MulticastPlayPickUpItemMontage();

	if (!DroppedItemActor->RemoveItem(NewWeapon))
	{
		AS_LOG_S(Error);
		return;
	}

	UASItem* OldWeapon = nullptr;
	if (ASInventory->InsertWeapon(SlotType, NewWeapon, OldWeapon))
	{
		if (OldWeapon != nullptr)
		{
			SpawnDroppedItemActor(OldWeapon);
		}		
	}
	else
	{
		DroppedItemActor->AddItem(NewWeapon);
		AS_LOG_S(Error);
	}
}

void AASCharacter::ServerPickUpArmor_Implementation(EArmorSlotType SlotType, UASArmor* NewArmor)
{
	if (!CanPickUpItem())
	{
		AS_LOG_S(Error);
		return;
	}

	if (ASInventory == nullptr)
	{
		AS_LOG_S(Error);
		return;
	}

	if (!ASInventory->IsSuitableArmorSlot(SlotType, NewArmor))
	{
		AS_LOG_S(Error);
		return;
	}
		
	auto DroppedItemActor = Cast<AASDroppedItemActor>(NewArmor->GetOwner());
	if (!IsValid(DroppedItemActor))
	{
		AS_LOG_S(Error);
		return;
	}

	if (!IsInteractableActor(DroppedItemActor))
	{
		AS_LOG_S(Error);
		return;
	}

	if (GetShootingStance() != EShootingStanceType::None)
	{
		ServerChangeShootingStance(EShootingStanceType::None);
	}

	MulticastPlayPickUpItemMontage();

	if (!DroppedItemActor->RemoveItem(NewArmor))
	{
		AS_LOG_S(Error);
		return;
	}

	UASItem* OldArmor = nullptr;
	if (ASInventory->InsertArmor(SlotType, NewArmor, OldArmor))
	{
		if (OldArmor != nullptr)
		{
			SpawnDroppedItemActor(OldArmor);
		}
	}
	else
	{
		DroppedItemActor->AddItem(NewArmor);
		AS_LOG_S(Error);
	}
}

void AASCharacter::ServerPickUpInventoryItem_Implementation(UASItem* NewItem)
{
	if (!CanPickUpItem())
	{
		AS_LOG_S(Error);
		return;
	}

	if (ASInventory == nullptr)
	{
		AS_LOG_S(Error);
		return;
	}

	if (!ASInventory->IsEnableToAddItemToInventory(NewItem))
		return;

	auto DroppedItemActor = Cast<AASDroppedItemActor>(NewItem->GetOwner());
	if (!IsValid(DroppedItemActor))
	{
		AS_LOG_S(Error);
		return;
	}

	if (!IsInteractableActor(DroppedItemActor))
	{
		AS_LOG_S(Error);
		return;
	}

	if (GetShootingStance() != EShootingStanceType::None)
	{
		ServerChangeShootingStance(EShootingStanceType::None);
	}

	MulticastPlayPickUpItemMontage();

	if (!DroppedItemActor->RemoveItem(NewItem))
	{
		AS_LOG_S(Error);
		return;
	}

	if (!ASInventory->AddItemToInventory(NewItem))
	{
		DroppedItemActor->AddItem(NewItem);
		AS_LOG_S(Error);
	}
}

void AASCharacter::ServerDropItem_Implementation(UASItem* InItem)
{
	if (!IsValid(InItem))
	{
		AS_LOG_S(Error);
		return;
	}

	if (!CanDropItem())
	{
		AS_LOG_S(Error);
		return;
	}

	if (ASInventory == nullptr)
	{
		AS_LOG_S(Error);
		return;
	}

	if (ASAnimInstance->IsPlayingReloadMontage())
	{
		TWeakObjectPtr<UASWeapon> SelectedWeapon = ASInventory->GetSelectedWeapon();
		if (SelectedWeapon.IsValid() && SelectedWeapon.Get() == InItem)
		{
			MulticastStopReloadMontage();
		}
		else if (auto Ammo = Cast<UASAmmo>(InItem))
		{
			if (Ammo->GetAmmoType() == SelectedWeapon->GetAmmoType())
			{
				MulticastStopReloadMontage();
			}
		}
	}

	ItemBoolPair ResultPair = ASInventory->RemoveItem(InItem);
	if (!ResultPair.Value)
	{
		AS_LOG_S(Error);
		return;
	}

	SpawnDroppedItemActor(ResultPair.Key);
}

bool AASCharacter::RemoveItem(UASItem* InItem)
{
	if (ASInventory == nullptr)
	{
		AS_LOG_S(Error);
		return false;
	}

	return ASInventory->RemoveItem(InItem).Value;
}

void AASCharacter::OnConstructedFullScreenWidget(UUserWidget* ConstructedWidget)
{
	if (ShootingStance != EShootingStanceType::None)
	{
		ServerChangeShootingStance(EShootingStanceType::None);
	}

	bShownFullScreenWidget = true;
}

void AASCharacter::OnDestructedFullScreenWidget(UUserWidget* DestructedWidget)
{
	bShownFullScreenWidget = false;
}

bool AASCharacter::IsShownFullScreenWidget() const
{
	return bShownFullScreenWidget;
}

bool AASCharacter::CanPickUpItem() const
{
	if (ASAnimInstance == nullptr)
	{
		AS_LOG_S(Error);
		return false;
	}

	if (bDead)
		return false;

	if (ASAnimInstance->IsPlayingUseHealingKitMontage())
		return false;

	if (ASAnimInstance->IsPlayingReloadMontage())
		return false;

	if (ASAnimInstance->IsPlayingEquipWeaponMontage())
		return false;

	return true;
}

void AASCharacter::PickUpItem(UASItem* InItem)
{
	if (!CanPickUpItem())
		return;

	if (!IsValid(InItem))
	{
		AS_LOG_S(Error);
		return;
	}

	switch (InItem->GetItemType())
	{
	case EItemType::Weapon:
		if (auto Weapon = Cast<UASWeapon>(InItem))
		{
			EWeaponSlotType SlotType = UASInventoryComponent::GetSuitableWeaponSlotType(Weapon->GetWeaponType());
			PickUpWeapon(SlotType, Weapon);
		}
		break;
	case EItemType::Armor:
		if (auto Armor = Cast<UASArmor>(InItem))
		{
			EArmorSlotType SlotType = UASInventoryComponent::GetSuitableArmorSlotType(Armor->GetArmorType());
			PickUpArmor(SlotType, Armor);
		}
		break;
	case EItemType::Ammo:			// fallthough
	case EItemType::HealingKit:
		{
			PickUpInventoryItem(InItem);
		}
		break;
	default:
		checkNoEntry();
		break;
	}
}

void AASCharacter::PickUpWeapon(EWeaponSlotType SlotType, UASWeapon* NewWeapon)
{
	if (!CanPickUpItem())
		return;

	if (ASInventory == nullptr)
	{
		AS_LOG_S(Error);
		return;
	}

	if (!ASInventory->IsSuitableWeaponSlot(SlotType, NewWeapon))
	{
		AS_LOG_S(Error);
		return;
	}

	ServerPickUpWeapon(SlotType, NewWeapon);
}

void AASCharacter::PickUpArmor(EArmorSlotType SlotType, UASArmor* NewArmor)
{
	if (!CanPickUpItem())
		return;

	if (ASInventory == nullptr)
	{
		AS_LOG_S(Error);
		return;
	}

	if (!ASInventory->IsSuitableArmorSlot(SlotType, NewArmor))
	{
		AS_LOG_S(Error);
		return;
	}

	ServerPickUpArmor(SlotType, NewArmor);
}

void AASCharacter::PickUpInventoryItem(UASItem* NewItem)
{
	if (!CanPickUpItem())
		return;

	if (ASInventory == nullptr)
	{
		AS_LOG_S(Error);
		return;
	}

	if (!ASInventory->IsEnableToAddItemToInventory(NewItem))
	{
		AS_LOG_S(Error);
		return;
	}

	ServerPickUpInventoryItem(NewItem);
}

bool AASCharacter::CanDropItem() const
{
	if (ASAnimInstance == nullptr)
	{
		AS_LOG_S(Error);
		return false;
	}

	if (ASAnimInstance->IsPlayingUseHealingKitMontage())
		return false;

	return true;
}

void AASCharacter::DropItem(UASItem* InItem)
{
	if (!IsValid(InItem))
	{
		AS_LOG_S(Error);
		return;
	}

	if (!CanDropItem())
		return;

	ServerDropItem(InItem);
}

UASStatusComponent* AASCharacter::GetStatusComponent()
{
	return ASStatus;
}

bool AASCharacter::IsDead() const
{
	return bDead;
}

void AASCharacter::StartRagdoll()
{
	ENetMode NetMode = GetNetMode();
	if (NetMode == NM_DedicatedServer)
	{
		if (UCharacterMovementComponent* CharMoveComp = GetCharacterMovement())
		{
			CharMoveComp->SetMovementMode(EMovementMode::MOVE_None);
		}
		else
		{
			AS_LOG_S(Error);
		}
	}
	else if (NetMode == NM_Client)
	{
		if (USkeletalMeshComponent* SkeletalMeshComp = GetMesh())
		{
			SkeletalMeshComp->SetCollisionObjectType(ECollisionChannel::ECC_PhysicsBody);
			SkeletalMeshComp->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
			SkeletalMeshComp->SetCollisionProfileName(TEXT("Ragdoll"));
			SkeletalMeshComp->SetAllBodiesBelowSimulatePhysics(FName(TEXT("pelvis")), true, true);

			if (auto AnimInstance = Cast<UASAnimInstance>(SkeletalMeshComp->GetAnimInstance()))
			{
				AnimInstance->StopAllMontages(0.2f);
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

	if (UCapsuleComponent* CapsuleComp = GetCapsuleComponent())
	{
		CapsuleComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	else
	{
		AS_LOG_S(Error);
	}
}

void AASCharacter::EndRagdoll()
{
	ENetMode NetMode = GetNetMode();
	if (NetMode == NM_DedicatedServer)
	{

	}
	else if (NetMode == NM_Client)
	{
		if (USkeletalMeshComponent* SkeletalMeshComp = GetMesh())
		{
			SkeletalMeshComp->SetCollisionObjectType(ECollisionChannel::ECC_Pawn);
			SkeletalMeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			SkeletalMeshComp->SetCollisionProfileName(TEXT("CharacterMesh"));
			SkeletalMeshComp->ResetAllBodiesSimulatePhysics();
		}
		else
		{
			AS_LOG_S(Error);
		}
	}

	if (UCapsuleComponent* CapsuleComp = GetCapsuleComponent())
	{
		CapsuleComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	}
	else
	{
		AS_LOG_S(Error);
	}
}

void AASCharacter::TurnOnInvincible(float Duration)
{
	if (Duration <= 0.0f)
		return;

	if (GetWorldTimerManager().IsTimerActive(InvincibleTimerHandle))
	{
		GetWorldTimerManager().ClearTimer(InvincibleTimerHandle);
	}

	GetWorldTimerManager().SetTimer(InvincibleTimerHandle, this, &AASCharacter::TurnOffInvincible, Duration);

	SetCanBeDamaged(false);
	bInvincible = true;
}

void AASCharacter::TurnOffInvincible()
{
	if (GetWorldTimerManager().IsTimerActive(InvincibleTimerHandle))
	{
		GetWorldTimerManager().ClearTimer(InvincibleTimerHandle);
	}

	SetCanBeDamaged(true);
	bInvincible = false;
}

void AASCharacter::StopAllActions()
{
	if (bSprinted)
	{
		ServerSprintEnd();
	}

	if (ShootingStance != EShootingStanceType::None)
	{
		ServerChangeShootingStance(EShootingStanceType::None);
	}

	if (ASAnimInstance != nullptr)
	{
		if (ASAnimInstance->IsPlayingReloadMontage())
		{
			MulticastStopReloadMontage();
		}

		if (ASAnimInstance->IsPlayingUseHealingKitMontage())
		{
			MulticastStopUseHealingKitMontage();
		}
	}
	else
	{
		AS_LOG_S(Error);
	}
}

void AASCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AASCharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &AASCharacter::Sprint);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &AASCharacter::SprintEnd);
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &AASCharacter::ToggleCrouch);
	PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &AASCharacter::PressedAimButton);
	PlayerInputComponent->BindAction("Aim", IE_Released, this, &AASCharacter::ReleasedAimButton);
	PlayerInputComponent->BindAction("SelectMainWeapon", IE_Pressed, this, &AASCharacter::SelectMainWeapon);
	PlayerInputComponent->BindAction("SelectSubWeapon", IE_Pressed, this, &AASCharacter::SelectSubWeapon);
	PlayerInputComponent->BindAction("Shoot", IE_Pressed, this, &AASCharacter::PressedShootButton);
	PlayerInputComponent->BindAction("Shoot", IE_Released, this, &AASCharacter::ReleasedShootButton);
	PlayerInputComponent->BindAction("ChangeFireMode", IE_Pressed, this, &AASCharacter::ChangeFireMode);
	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &AASCharacter::Reload);
	PlayerInputComponent->BindAction("HealingKit", IE_Pressed, this, &AASCharacter::UseHealingKit);
	PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &AASCharacter::Interact);

	PlayerInputComponent->BindAxis("MoveForward", this, &AASCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AASCharacter::MoveRight);
	PlayerInputComponent->BindAxis("Turn", this, &AASCharacter::Turn);
	PlayerInputComponent->BindAxis("TurnRate", this, &AASCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AASCharacter::LookUpAtRate);
	PlayerInputComponent->BindAxis("Incline", this, &AASCharacter::Incline);
}

void AASCharacter::OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PrevMovementMode, PreviousCustomMode);

	if (ASAction != nullptr)
	{
		EMovementMode CurMode = GetCharacterMovement()->MovementMode.GetValue();
		switch (CurMode)
		{
		case EMovementMode::MOVE_Walking:		// fallthough
		case EMovementMode::MOVE_NavWalking:
			{
				ASAction->SetMovementState(EMovementState::Grounded);
			}
			break;
		case EMovementMode::MOVE_Falling:
			{
				ASAction->SetMovementState(EMovementState::InAir);
			}
			break;
		default:
			break;
		}
	}
	else
	{
		AS_LOG_S(Error);
	}
}

void AASCharacter::MoveForward(float Value)
{
	if (Controller != nullptr)
	{
		if (Value != 0.0f)
		{
			const FRotator Rotation = Controller->GetControlRotation();
			const FRotator YawRotation(0, Rotation.Yaw, 0);
			const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

			if (ShootingStance != EShootingStanceType::None && Value > 0.0f)
			{
				Value /= 2.0f;
			}

			AddMovementInput(Direction, Value);
		}
	}
	else
	{
		AS_LOG_S(Error);
	}
}

void AASCharacter::MoveRight(float Value)
{
	if (Controller != nullptr)
	{
		if (Value != 0.0f)
		{
			const FRotator Rotation = Controller->GetControlRotation();
			const FRotator YawRotation(0, Rotation.Yaw, 0);
			const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

			AddMovementInput(Direction, Value);
		}
	}
	else
	{
		AS_LOG_S(Error);
	}
}

void AASCharacter::Turn(float Value)
{
	AddControllerYawInput(Value);

	TurnValue = Value;
	ServerSetTurnValue(Value);
}

void AASCharacter::TurnAtRate(float Rate)
{
	float Value = Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds();
	AddControllerYawInput(Value);

	TurnRateValue = Value;
	ServerSetTurnRateValue(Value);
}

void AASCharacter::LookUpAtRate(float Rate)
{
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AASCharacter::Incline(float Value)
{
	InclineValue = Value;
	ServerInclineValue(Value);
}

bool AASCharacter::CanSprint() const
{
	if (ASAnimInstance == nullptr)
	{
		AS_LOG_S(Error);
		return false;
	}

	if (bDead)
		return false;

	if (bSprinted)
		return false;

	if (ShootingStance != EShootingStanceType::None)
		return false;
	
	if (ASAnimInstance->IsPlayingReloadMontage())
		return false;;

	return true;
}

void AASCharacter::Sprint()
{
	if (!CanSprint())
		return;

	ServerSprint();
}

void AASCharacter::SprintEnd()
{
	if (!bSprinted)
		return;

	ServerSprintEnd();
}

void AASCharacter::ToggleCrouch()
{
	if (IsLocallyControlled())
	{
		if (bIsCrouched)
		{
			UnCrouch(true);
		}
		else
		{
			Crouch(true);
		}
	}
}

void AASCharacter::PressedAimButton()
{
	bPressedAimButton = true;
	AimKeyHoldTime = 0.0f;
}

void AASCharacter::ReleasedAimButton()
{
	switch (ShootingStance)
	{
	case EShootingStanceType::None:
		if (bPressedAimButton)
		{
			if (CanAimOrScope())
			{
				ServerChangeShootingStance(EShootingStanceType::Scoping);
			}
		}
		break;
	case EShootingStanceType::Scoping:
		if (bPressedAimButton)
		{
			ServerChangeShootingStance(EShootingStanceType::None);
		}
		break;
	case EShootingStanceType::Aiming:
		{
			ServerChangeShootingStance(EShootingStanceType::None);
		}
		break;
	default:
		checkNoEntry();
		break;
	}

	ResetAimKeyState();
}

bool AASCharacter::CanSelectWeapon() const
{
	if (ASAnimInstance == nullptr)
	{
		AS_LOG_S(Error);
		return false;
	}

	if (bDead)
		return false;

	if (ASAnimInstance->IsPlayingReloadMontage())
		return false;
	
	if (ASAnimInstance->IsPlayingEquipWeaponMontage())
		return false;

	if (ASAnimInstance->IsPlayingUseHealingKitMontage())
		return false;

	return true;
}

void AASCharacter::SelectMainWeapon()
{
	if (!CanSelectWeapon())
		return;

	if (ASInventory == nullptr)
	{
		AS_LOG_S(Error);
		return;
	}

	if (ASInventory->GetSelectedWeaponSlotType() == EWeaponSlotType::Main)
		return;

	ServerSelectWeapon(EWeaponSlotType::Main);
}

void AASCharacter::SelectSubWeapon()
{
	if (!CanSelectWeapon())
		return;

	if (ASInventory == nullptr)
	{
		AS_LOG_S(Error);
		return;
	}

	if (ASInventory->GetSelectedWeaponSlotType() == EWeaponSlotType::Sub)
		return;

	ServerSelectWeapon(EWeaponSlotType::Sub);
}

void AASCharacter::PressedShootButton()
{
	if (ASAnimInstance == nullptr)
	{
		AS_LOG_S(Error);
		return;
	}

	if (bDead)
		return;

	if (ASAnimInstance->IsPlayingEquipWeaponMontage())
		return;

	if (ASAnimInstance->IsPlayingUseHealingKitMontage())
		return;

	if (ASAnimInstance->IsPlayingReloadMontage())
		return;

	TWeakObjectPtr<UASWeapon> Weapon = ASInventory->GetSelectedWeapon();
	if (!Weapon.IsValid())
		return;

	switch (Weapon->GetFireMode())
	{
	case EFireMode::SemiAuto:
		{
			Shoot();
		}
		break;
	case EFireMode::FullAuto:
		{
			bPressedShootButton = true;
		}
		break;
	default:
		checkNoEntry();
		break;
	}
}

void AASCharacter::ReleasedShootButton()
{
	bPressedShootButton = false;
}

bool AASCharacter::CanChangeFireMode() const
{
	if (ASAnimInstance == nullptr)
	{
		AS_LOG_S(Error);
		return false;
	}

	if (bDead)
		return false;

	if (ASAnimInstance->IsPlayingReloadMontage())
		return false;

	if (ASAnimInstance->IsPlayingEquipWeaponMontage())
		return false;

	if (ASAnimInstance->IsPlayingUseHealingKitMontage())
		return false;

	return true;
}

void AASCharacter::ChangeFireMode()
{
	if (!CanChangeFireMode())
		return;

	if (bPressedShootButton)
		return;

	TWeakObjectPtr<UASWeapon> Weapon = ASInventory->GetSelectedWeapon();
	if (!Weapon.IsValid())
		return;

	ServerChangeFireMode();
}

bool AASCharacter::CanReload() const
{
	if (ASAnimInstance == nullptr)
	{
		AS_LOG_S(Error);
		return false;
	}

	if (bDead)
		return false;

	if (GetCharacterMovement()->IsFalling())
		return false;
	
	if (ASAnimInstance->IsPlayingUseHealingKitMontage())
		return false;
	
	if (ASAnimInstance->IsPlayingEquipWeaponMontage())
		return false;

	if (ASAnimInstance->IsPlayingReloadMontage())
		return false;
	
	return true;
}

void AASCharacter::Reload()
{
	if (!CanReload())
		return;

	if (ASInventory == nullptr)
	{
		AS_LOG_S(Error);
		return;
	}

	TWeakObjectPtr<UASWeapon> CurWeapon = ASInventory->GetSelectedWeapon();
	if (!CurWeapon.IsValid() || !CurWeapon->CanReload())
		return;

	int32 InventoryAmmoCount = ASInventory->GetAmmoCountInInventory(CurWeapon->GetAmmoType());
	if (InventoryAmmoCount <= 0)
		return;

	ServerBeginReload();
}

bool AASCharacter::CanUseHealingKit() const
{
	if (ASAnimInstance == nullptr)
	{
		AS_LOG_S(Error);
		return false;
	}

	if (ASStatus == nullptr)
	{
		AS_LOG_S(Error);
		return false;
	}

	if (bDead)
		return false;

	if (GetCharacterMovement()->IsFalling())
		return false;

	if (ASAnimInstance->IsPlayingUseHealingKitMontage())
		return false;

	if (ASStatus->GetCurrentHealth() >= ASStatus->GetMaxHealth())
		return false;

	return true;
}

void AASCharacter::UseHealingKit()
{
	if (!CanUseHealingKit())
		return;

	if (ASInventory == nullptr)
	{
		AS_LOG_S(Error);
		return;
	}

	TArray<UASHealingKit*> HealingKits = ASInventory->GetHealingKits();
	for (auto& HealingKit : HealingKits)
	{
		if (HealingKit == nullptr)
			continue;

		if (HealingKit->GetCount() <= 0)
			continue;

		ServerBeginHealingKit(HealingKit);
		break;
	}
}

void AASCharacter::Interact()
{
	if (GroundItemActorSet.Num() == 0)
		return;

	AActor* PickableActor = FindPickableActor();
	if (IsValid(PickableActor))
	{
		if (auto DroppedItemActor = Cast<AASDroppedItemActor>(PickableActor))
		{
			if (DroppedItemActor->GetItemNum() == 1 && GroundItemActorSet.Contains(DroppedItemActor))
			{
				PickUpItem(DroppedItemActor->GetItems()[0].Get());
			}
		}
	}
}

bool AASCharacter::CanShoot() const
{
	if (ASAnimInstance == nullptr)
	{
		AS_LOG_S(Error);
		return false;
	}

	if (bDead)
		return false;

	if (ShootingStance == EShootingStanceType::None)
		return false;

	if (ASAnimInstance->IsPlayingReloadMontage())
		return false;
	
	if (ASAnimInstance->IsPlayingEquipWeaponMontage())
		return false;

	if (ASAnimInstance->IsPlayingUseHealingKitMontage())
		return false;

	return true;
}

void AASCharacter::Shoot()
{
	if (!CanShoot())
		return;

	if (ASInventory == nullptr)
	{
		AS_LOG_S(Error);
		return;
	}

	TWeakObjectPtr<UASWeapon> Weapon = ASInventory->GetSelectedWeapon();
	if (!Weapon.IsValid())
		return;

	if (!Weapon->IsPassedFireInterval())
		return;

	if (Weapon->GetCurrentAmmoCount() <= 0)
	{
		// todo: ź�� ���� �˸�
		return;
	}

	const TWeakObjectPtr<AASWeaponActor>& WeaponActor = Weapon->GetActor();
	if (!WeaponActor.IsValid())
		return;
	
	switch (ShootingStance)
	{
	case EShootingStanceType::Aiming:
		{
			FVector CamLoc = FollowCamera->GetComponentLocation();
			FVector CamForward = FollowCamera->GetForwardVector().GetSafeNormal();
			FVector MuzzleLoc = WeaponActor->GetMuzzleLocation();

			float LengthToStartLoc = FMath::Abs((CamLoc - MuzzleLoc) | CamForward);
			FVector TraceStartLoc = CamLoc + (CamForward * LengthToStartLoc);
			FVector TraceEndLoc = CamLoc + (CamForward * 15000.0f);

			FVector TargetLoc;

			FHitResult HitResult;
			FCollisionQueryParams QueryParams;
			QueryParams.AddIgnoredActor(this);
			if (GetWorld()->LineTraceSingleByProfile(HitResult, TraceStartLoc, TraceEndLoc, TEXT("Bullet"), QueryParams))
			{
				TargetLoc = HitResult.Location;
			}
			else
			{
				TargetLoc = TraceEndLoc;
			}
			
			FVector FireDir = (TargetLoc - MuzzleLoc).GetSafeNormal();
			FRotator FireRot = FRotationMatrix::MakeFromX(FireDir).Rotator();
			ServerShoot(MuzzleLoc, FireRot);
			
			Weapon->SetLastFireTick();
		}		
		break;
	case EShootingStanceType::Scoping:
		{
			FVector MuzzleLocation;
			FRotator MuzzleRotation;
			WeaponActor->GetMuzzleLocationAndRotation(MuzzleLocation, MuzzleRotation);

			ServerShoot(MuzzleLocation, MuzzleRotation);

			Weapon->SetLastFireTick();
		}
		break;
	default:
		checkNoEntry();
		break;
	}
}

void AASCharacter::ResetAimKeyState()
{
	bPressedAimButton = false;
	AimKeyHoldTime = 0.0f;
}

void AASCharacter::ServerSprint_Implementation()
{
	if (!CanSprint())
	{
		AS_LOG_S(Error);
		return;
	}

	if (ASAnimInstance != nullptr)
	{
		if (ASAnimInstance->IsPlayingUseHealingKitMontage())
		{
			MulticastStopUseHealingKitMontage();
		}
	}
	else
	{
		AS_LOG_S(Error);
	}

	SetMaxWalkSpeedRate(SprintSpeedRate);
	bSprinted = true;
}

void AASCharacter::ServerSprintEnd_Implementation()
{
	if (!bSprinted)
		return;

	SetMaxWalkSpeedRate(1.0f);
	bSprinted = false;
}

void AASCharacter::OnRep_bSprinted()
{
	SetMaxWalkSpeedRate(bSprinted ? SprintSpeedRate : 1.0f);
}

void AASCharacter::SetMaxWalkSpeedRate(float Rate)
{
	UCharacterMovementComponent* CharMoveComp = GetCharacterMovement();
	auto DefaultCharMoveComp = GetDefault<UCharacterMovementComponent>();
	if (CharMoveComp != nullptr && DefaultCharMoveComp != nullptr)
	{
		CharMoveComp->MaxWalkSpeed = DefaultCharMoveComp->MaxWalkSpeed * Rate;
		CharMoveComp->MaxWalkSpeedCrouched = DefaultCharMoveComp->MaxWalkSpeedCrouched * Rate;
	}
	else
	{
		AS_LOG_S(Error);
	}
}

void AASCharacter::ServerSetTurnValue_Implementation(float NewTurnValue)
{
	TurnValue = NewTurnValue;
}

void AASCharacter::ServerSetTurnRateValue_Implementation(float NewTurnRateValue)
{
	TurnRateValue = NewTurnRateValue;
}

void AASCharacter::ServerSelectWeapon_Implementation(EWeaponSlotType WeaponSlotType)
{
	if (!CanSelectWeapon())
	{
		AS_LOG_S(Error);
		return;
	}

	if (ASInventory->GetSelectedWeaponSlotType() == WeaponSlotType)
	{
		AS_LOG_S(Error);
		return;
	}

	ServerChangeShootingStance(EShootingStanceType::None);

	ItemPtrBoolPair ResultPair = ASInventory->FindItemFromWeaponSlot(WeaponSlotType);
	if (!ResultPair.Value)
		return;

	UASWeapon* NewWeapon = Cast<UASWeapon>(ResultPair.Key);
	if (IsValid(NewWeapon))
	{
		if (ASAnimInstance->IsPlayingReloadMontage())
		{
			MulticastStopReloadMontage();
		}

		if (ASInventory->SelectWeapon(WeaponSlotType))
		{
			MulticastPlayChangeWeaponMontage(NewWeapon->GetWeaponType());
		}
	}
}

void AASCharacter::MulticastPlayChangeWeaponMontage_Implementation(EWeaponType WeaponType)
{
	if (ASAnimInstance != nullptr)
	{
		ASAnimInstance->PlayEquipWeaponMontage(WeaponType);
	}
}

void AASCharacter::ServerChangeShootingStance_Implementation(EShootingStanceType NewShootingStance)
{
	if (ASAnimInstance == nullptr)
	{
		AS_LOG_S(Error);
		return;
	}

	if (ASAnimInstance->IsPlayingUseHealingKitMontage())
		return;

	if (ShootingStance == NewShootingStance)
		return;

	if (NewShootingStance != EShootingStanceType::None && ASAnimInstance->IsPlayingEquipWeaponMontage())
		return;

	switch (ShootingStance)
	{
	case EShootingStanceType::None:
		break;
	case EShootingStanceType::Aiming:
		{
			EndAiming();
		}
		break;
	case EShootingStanceType::Scoping:
		{
			EndScoping();
		}
		break;
	default:
		checkNoEntry();
		break;
	}

	switch (NewShootingStance)
	{
	case EShootingStanceType::None:
		break;
	case EShootingStanceType::Aiming:
		if (CanAimOrScope())
		{
			StartAiming();
		}
		break;
	case EShootingStanceType::Scoping:
		if (CanAimOrScope())
		{
			StartScoping();
		}
		break;
	default:
		checkNoEntry();
		break;
	}

	OnChangedShootingStance.Broadcast(ShootingStance);
}

void AASCharacter::OnRep_ShootingStance(EShootingStanceType OldShootingStance)
{
	switch (OldShootingStance)
	{
	case EShootingStanceType::None:
		break;
	case EShootingStanceType::Aiming:
		{
			EndAiming();
		}		
		break;
	case EShootingStanceType::Scoping:
		{
			EndScoping();
		}
		break;
	default:
		checkNoEntry();
		break;
	}

	switch (ShootingStance)
	{
	case EShootingStanceType::None:
		break;
	case EShootingStanceType::Aiming:
		{
			StartAiming();
		}
		break;
	case EShootingStanceType::Scoping:
		{
			StartScoping();
		}
		break;
	default:
		checkNoEntry();
		break;
	}

	OnChangedShootingStance.Broadcast(ShootingStance);
}

bool AASCharacter::CanAimOrScope() const
{
	if (ASAnimInstance == nullptr)
	{
		AS_LOG_S(Error);
		return false;
	}

	if (bDead)
		return false;

	if (bShownFullScreenWidget)
		return false;

	if (ShootingStance != EShootingStanceType::None)
		return false;
	
	if (GetUsingWeaponType() == EWeaponType::None)
		return false;

	if (GetCharacterMovement()->IsFalling())
		return false;
	
	if (ASAnimInstance->IsPlayingReloadMontage())
		return false;
	
	if (ASAnimInstance->IsPlayingEquipWeaponMontage())
		return false;

	if (ASAnimInstance->IsPlayingUseHealingKitMontage())
		return false;
	
	return true;
}

void AASCharacter::StartAiming()
{
	if (GetLocalRole() == ROLE_Authority)
	{
		ShootingStance = EShootingStanceType::Aiming;
		ServerSprintEnd();
	}	

	SetMaxWalkSpeedRate(AimingSpeedRate);

	if (CameraBoom != nullptr)
	{
		CameraBoom->TargetArmLength = AimingCamArmLength;
		CameraBoom->SocketOffset = AimingCamOffset;
	}
	else
	{
		AS_LOG_S(Error);
	}
}

void AASCharacter::EndAiming()
{
	if (GetLocalRole() == ROLE_Authority)
	{
		ShootingStance = EShootingStanceType::None;
	}

	SetMaxWalkSpeedRate(1.0f);

	if (CameraBoom != nullptr)
	{
		CameraBoom->TargetArmLength = NormalCamArmLength;
		CameraBoom->SocketOffset = NormalCamOffset;
	}
}

void AASCharacter::StartScoping()
{
	if (GetLocalRole() == ROLE_Authority)
	{
		ShootingStance = EShootingStanceType::Scoping;
		ServerSprintEnd();
	}

	SetMaxWalkSpeedRate(AimingSpeedRate);

	if (ASInventory != nullptr)
	{
		OnScope.Broadcast(ASInventory->GetSelectedWeapon());
	}	
}

void AASCharacter::EndScoping()
{
	if (GetLocalRole() == ROLE_Authority)
	{
		ShootingStance = EShootingStanceType::None;
	}

	SetMaxWalkSpeedRate(1.0f);
	OnUnscope.Broadcast();
}

bool AASCharacter::ServerShoot_Validate(const FVector& MuzzleLocation, const FRotator& ShootRotation)
{
	//constexpr float ValidLocDiff = 200.0f;
	//constexpr float ValidRotDiff = 45.0f;

	//TWeakObjectPtr<AASWeaponActor> SelectedWeaponActor = ASInventory->GetSelectedWeaponActor();
	//if (!SelectedWeaponActor.IsValid())
	//{
	//	AS_LOG_S(Error);
	//	return false;
	//}

	//FVector LocDiff = SelectedWeaponActor->GetMuzzleLocation() - MuzzleLocation;
	//if (FMath::Abs(LocDiff.X) > ValidLocDiff || FMath::Abs(LocDiff.Y) > ValidLocDiff || FMath::Abs(LocDiff.Z) > ValidLocDiff)
	//{
	//	AS_LOG(Error, TEXT("%s"), *LocDiff.ToString());
	//	return false;
	//}
	//
	//FRotator RotDiff = (GetActorRotation() - ShootRotation).GetNormalized();
	//if (FMath::Abs(RotDiff.Pitch) > ValidRotDiff || FMath::Abs(RotDiff.Yaw) > ValidRotDiff)
	//{
	//	AS_LOG(Error, TEXT("%s"), *RotDiff.ToString());
	//	return false;
	//}

	TWeakObjectPtr<UASWeapon> SelectedWeapon = ASInventory->GetSelectedWeapon();
	if (!SelectedWeapon.IsValid())
		return false;

	if (!SelectedWeapon->IsPassedFireInterval())
		return false;

	return true;
}

void AASCharacter::ServerShoot_Implementation(const FVector& MuzzleLocation, const FRotator& ShootRotation)
{
	if (!CanShoot())
	{
		AS_LOG_S(Error);
		return;
	}

	if (ASInventory == nullptr)
	{
		AS_LOG_S(Error);
		return;
	}

	TWeakObjectPtr<UASWeapon> SelectedWeapon = ASInventory->GetSelectedWeapon();
	if (!SelectedWeapon.IsValid())
	{
		AS_LOG_S(Error);
		return;
	}

	FRotator MuzzleRot = ShootRotation;
	if (ShootingStance == EShootingStanceType::Aiming)
	{
		float RandPitch = FMath::RandRange(-CurrentBulletSpread, CurrentBulletSpread);
		float MaxYaw = FMath::Sqrt(CurrentBulletSpread * CurrentBulletSpread - RandPitch * RandPitch);
		float RandYaw = FMath::RandRange(-MaxYaw, MaxYaw);

		MuzzleRot.Pitch += RandPitch;
		MuzzleRot.Yaw += RandYaw;
	}

	AASBullet* SpawnedBullet = SelectedWeapon->Fire(ShootingStance, MuzzleLocation, MuzzleRot);
	if (SpawnedBullet != nullptr)
	{
		CurrentBulletSpread = FMath::Clamp(CurrentBulletSpread + BulletSpreadAmountPerShot, MinBulletSpread, MaxBulletSpread);

		MulticastPlayShootMontage();
	}
}

void AASCharacter::MulticastPlayShootMontage_Implementation()
{
	if (ASAnimInstance != nullptr)
	{
		ASAnimInstance->PlayShootMontage(GetUsingWeaponType());

		OnPlayShootMontage.Broadcast();
	}
	else
	{
		AS_LOG_S(Error);
	}

	if (ASInventory != nullptr)
	{
		if (GetLocalRole() == ROLE_AutonomousProxy)
		{
			TWeakObjectPtr<UASWeapon> Weapon = ASInventory->GetSelectedWeapon();
			if (Weapon.IsValid())
			{
				FVector2D RecoilPitch;
				FVector2D RecoilYaw;
				Weapon->GetRecoil(RecoilPitch, RecoilYaw);

				AddControllerPitchInput(-FMath::RandRange(RecoilPitch.X, RecoilPitch.Y));
				AddControllerYawInput(FMath::RandRange(RecoilYaw.X, RecoilYaw.Y));
			}
		}

		TWeakObjectPtr<AASWeaponActor> WeaponActor = ASInventory->GetSelectedWeaponActor();
		if (WeaponActor.IsValid())
		{
			WeaponActor->PlayFireAnim();
		}
	}
	else
	{
		AS_LOG_S(Error);
	}
}

void AASCharacter::ServerChangeFireMode_Implementation()
{
	if (!CanChangeFireMode())
	{
		AS_LOG_S(Error);
		return;
	}

	TWeakObjectPtr<UASWeapon> Weapon = ASInventory->GetSelectedWeapon();
	if (Weapon.IsValid())
	{
		Weapon->ChangeToNextFireMode();
	}
	else
	{
		AS_LOG_S(Error);
	}
}

void AASCharacter::SpawnDroppedItemActor(UASItem* DroppingItem)
{
	if (!IsValid(DroppingItem))
	{
		AS_LOG_S(Error);
		return;
	}		

	USkeletalMeshComponent* SkeletalMeshComp = GetMesh();
	FVector Location = (SkeletalMeshComp != nullptr ? SkeletalMeshComp->GetComponentLocation() : GetActorLocation());

	auto DroppedItemActor = GetWorld()->SpawnActor<AASDroppedItemActor>(DroppingItem->GetDroppedItemActorClass(), Location, GetActorRotation());
	if (IsValid(DroppedItemActor))
	{
		DroppedItemActor->AddItem(DroppingItem);
	}
	else
	{
		AS_LOG_S(Error);
	}
}

void AASCharacter::SpawnDroppedItemsActor(const TArray<UASItem*>& DroppingItems)
{
	USkeletalMeshComponent* SkeletalMeshComp = GetMesh();
	FVector Location = (SkeletalMeshComp != nullptr ? SkeletalMeshComp->GetComponentLocation() : GetActorLocation());

	auto DroppedItemActor = GetWorld()->SpawnActor<AASDroppedItemActor>(DroppedItemBoxActorClass, Location, GetActorRotation());
	if (IsValid(DroppedItemActor))
	{
		DroppedItemActor->AddItems(DroppingItems);
	}
	else
	{
		AS_LOG_S(Error);
	}
}

void AASCharacter::OnRemoveGroundItem(const TWeakObjectPtr<UASItem>& Item)
{
	OnGroundItemRemove.Broadcast(TArray<TWeakObjectPtr<UASItem>>{ Item });
}

void AASCharacter::ServerBeginReload_Implementation()
{
	if (!CanReload())
	{
		AS_LOG_S(Error);
		return;
	}

	if (ASInventory == nullptr)
	{
		AS_LOG_S(Error);
		return;
	}

	TWeakObjectPtr<UASWeapon> CurWeapon = ASInventory->GetSelectedWeapon();
	if (!CurWeapon.IsValid() || !CurWeapon->CanReload())
		return;

	int32 InventoryAmmoCount = ASInventory->GetAmmoCountInInventory(CurWeapon->GetAmmoType());
	if (InventoryAmmoCount <= 0)
		return;

	if (bSprinted)
	{
		ServerSprintEnd();
	}

	if (GetShootingStance() == EShootingStanceType::Scoping)
	{
		ServerChangeShootingStance(EShootingStanceType::None);
	}

	MulticastPlayReloadMontage();
}

void AASCharacter::CompleteReload()
{
	TWeakObjectPtr<UASWeapon> SelectedWeapon = ASInventory->GetSelectedWeapon();
	if (SelectedWeapon.IsValid())
	{
		TArray<UASAmmo*> Ammos = ASInventory->GetAmmos(SelectedWeapon->GetAmmoType());
		if (!SelectedWeapon->Reload(Ammos))
		{
			AS_LOG_S(Error);
		}
	}
	else
	{
		AS_LOG_S(Error);
	}
}

void AASCharacter::MulticastPlayReloadMontage_Implementation()
{
	if (ASAnimInstance != nullptr)
	{
		ASAnimInstance->PlayReloadMontage(GetUsingWeaponType());
	}
	else
	{
		AS_LOG_S(Error);
	}
}

void AASCharacter::MulticastStopReloadMontage_Implementation()
{
	if (ASAnimInstance != nullptr)
	{
		ASAnimInstance->StopReloadMontage();
	}
	else
	{
		AS_LOG_S(Error);
	}
}

void AASCharacter::Die()
{
	if (bDead)
		return;

	if (GetLocalRole() == ROLE_Authority)
	{
		StopAllActions();

		bDead = true;
		SetCanBeDamaged(false);

		SpawnDroppedItemsActor(ASInventory->RemoveAllItems());

		StartRagdoll();
	}
}

void AASCharacter::OnRep_bDead()
{
	if (bDead)
	{
		StartRagdoll();
	}
	else
	{
		EndRagdoll();
	}
}

void AASCharacter::ServerBeginHealingKit_Implementation(UASHealingKit* InHealingKit)
{
	if (!IsValid(InHealingKit))
	{
		AS_LOG_S(Error);
		return;
	}

	if (InHealingKit->GetCount() <= 0)
	{
		AS_LOG_S(Error);
		return;
	}

	if (!CanUseHealingKit())
	{
		AS_LOG_S(Error);
		return;
	}

	if (ASInventory == nullptr)
	{
		AS_LOG_S(Error);
		return;
	}

	if (!ASInventory->Contains(InHealingKit))
	{
		AS_LOG_S(Error);
		return;
	}

	if (ASAnimInstance == nullptr)
	{
		AS_LOG_S(Error);
		return;
	}

	if (bSprinted)
	{
		ServerSprintEnd();
	}

	if (GetShootingStance() != EShootingStanceType::None)
	{
		ServerChangeShootingStance(EShootingStanceType::None);
	}

	if (ASAnimInstance->IsPlayingReloadMontage())
	{
		MulticastStopReloadMontage();
	}

	TWeakObjectPtr<UASWeapon> CurWeapon = ASInventory->GetSelectedWeapon();
	if (CurWeapon.IsValid())
	{
		ASInventory->ReattachWeaponActor(CurWeapon.Get(), ASInventory->GetProperWeaponSocketName(CurWeapon->GetWeaponType(), false));
	}

	UsingHealingKit = InHealingKit;

	MulticastPlayUseHealingKitMontage();
}

void AASCharacter::CompleteUseHealingKit()
{
	if (ASInventory == nullptr)
	{
		AS_LOG_S(Error);
		return;
	}

	if (!IsValid(UsingHealingKit))
	{
		AS_LOG_S(Error);
		return;
	}

	if (UsingHealingKit->GetCount() <= 0)
	{
		AS_LOG_S(Error);
		return;
	}

	if (!ASInventory->Contains(UsingHealingKit))
	{
		AS_LOG_S(Error);
		return;
	}

	UsingHealingKit->ModifyCount(-1);

	if (ASStatus != nullptr)
	{
		ASStatus->ModifyCurrentHealth(UsingHealingKit->GetRecoveryPoint());
	}
	else
	{
		AS_LOG_S(Error);
	}
}

void AASCharacter::OnEndUseHealingKitMontage()
{
	if (GetLocalRole() == ROLE_Authority)
	{
		UsingHealingKit = nullptr;

		if (ASInventory != nullptr)
		{
			TWeakObjectPtr<UASWeapon> CurWeapon = ASInventory->GetSelectedWeapon();
			if (CurWeapon.IsValid())
			{
				ASInventory->ReattachWeaponActor(CurWeapon.Get(), ASInventory->GetProperWeaponSocketName(CurWeapon->GetWeaponType(), true));
			}
		}
		else
		{
			AS_LOG_S(Error);
		}
	}

	SetMaxWalkSpeedRate(1.0f);
}

void AASCharacter::MulticastPlayUseHealingKitMontage_Implementation()
{
	SetMaxWalkSpeedRate(UseHealingKitSpeedRate);

	if (ASAnimInstance != nullptr)
	{
		ASAnimInstance->PlayUseHealingKitMontage();
	}
	else
	{
		AS_LOG_S(Error);
	}
}

void AASCharacter::MulticastStopUseHealingKitMontage_Implementation()
{
	if (ASAnimInstance != nullptr)
	{
		ASAnimInstance->StopUseHealingKitMontage();
	}
	else
	{
		AS_LOG_S(Error);
	}
}

void AASCharacter::OnChangeSelectedWeapon(const TWeakObjectPtr<UASWeapon>& InOldWeapon, const TWeakObjectPtr<UASWeapon>& InNewWeapon)
{
	if (InNewWeapon.IsValid())
	{
		MinBulletSpread = InNewWeapon->GetMinBulletSpread();
		MaxBulletSpread = InNewWeapon->GetMaxBulletSpread();
		BulletSpreadAmountPerShot = InNewWeapon->GetBulletSpreadAmountPerShot();
		BulletSpreadRecoverySpeed = InNewWeapon->GetBulletSpreadRecoverySpeed();

		CurrentBulletSpread = MinBulletSpread;
	}
	else
	{
		MinBulletSpread = TNumericLimits<float>::Max();
		MaxBulletSpread = TNumericLimits<float>::Max();
		BulletSpreadAmountPerShot = TNumericLimits<float>::Max();
		BulletSpreadRecoverySpeed = 0.0f;

		CurrentBulletSpread = MinBulletSpread;
	}
}

void AASCharacter::MulticastPlayPickUpItemMontage_Implementation()
{
	if (ASAnimInstance != nullptr)
	{
		ASAnimInstance->PlayPickUpItemMontage();
	}
	else
	{
		AS_LOG_S(Error);
	}
}

void AASCharacter::OnChangedInnerMatchState(EInnerMatchState State)
{
	ENetMode NetMode = GetNetMode();
	if (NetMode == NM_DedicatedServer)
	{
		switch (State)
		{
		case EInnerMatchState::Prepare:
			{
				SetCanBeDamaged(false);
			}
			break;
		case EInnerMatchState::Process:
			{
				SetCanBeDamaged(true);
				StopAllActions();
			}
			break;
		case EInnerMatchState::Finish:
			{
				SetCanBeDamaged(false);
			}
			break;
		default:
			checkNoEntry();
			break;
		}
	}
	else if (NetMode == NM_Client)
	{
		switch (State)
		{
		case EInnerMatchState::Prepare:
			{
				EnableInput(nullptr);
			}
			break;
		case EInnerMatchState::Process:
			{
				EnableInput(nullptr);
			}
			break;
		case EInnerMatchState::Finish:
			{
				DisableInput(nullptr);
			}
			break;
		default:
			checkNoEntry();
			break;
		}
	}
}

void AASCharacter::HighlightingPickableActor()
{
	AActor* PickableActor = FindPickableActor();

	auto DroppedItemActor = Cast<AASDroppedItemActor>(PickableActor);
	bool bDroppedItemActorValid = IsValid(DroppedItemActor);

	for (auto& ItemPtr : GroundItemActorSet)
	{
		if (!ItemPtr.IsValid())
			continue;

		ItemPtr->ShowOutline(bDroppedItemActorValid && ItemPtr == DroppedItemActor && DroppedItemActor->GetItemNum() == 1);
	}
}

AActor* AASCharacter::FindPickableActor() const
{
	constexpr float TraceLen = 500.0f;

	FVector CamLoc = FollowCamera->GetComponentLocation();
	FVector CamForward = FollowCamera->GetForwardVector().GetSafeNormal();

	auto PC = GetController<APlayerController>();
	if (IsValid(PC) && PC->PlayerCameraManager != nullptr)
	{
		CamLoc = PC->PlayerCameraManager->GetCameraLocation();
		CamForward = PC->PlayerCameraManager->GetCameraRotation().Vector();
	}

	FVector EndLoc = CamLoc + (CamForward * TraceLen);

	TArray<AActor*> AttachedActors;
	GetAttachedActors(AttachedActors, false);

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	QueryParams.AddIgnoredActors(AttachedActors);
	if (GetWorld()->LineTraceSingleByChannel(HitResult, CamLoc, EndLoc, ECC_GameTraceChannel5, QueryParams))
	{
		return HitResult.GetActor();
	}

	return nullptr;
}

bool AASCharacter::IsInteractableActor(AActor* InActor) const
{
	constexpr float InteractableLenSquared = 200.0f * 200.0f;

	if ((InActor->GetActorLocation() - GetActorLocation()).SizeSquared() > InteractableLenSquared)
		return false;

	return true;
}

void AASCharacter::OnRep_bInvincible()
{
	if (bInvincible)
	{
		if (USkeletalMeshComponent* SkeletalMeshComp = GetMesh())
		{
			SkeletalMeshComp->SetScalarParameterValueOnMaterials(TEXT("bShowEffect"), 1.0f);
			SkeletalMeshComp->SetVectorParameterValueOnMaterials(TEXT("EffectColor"), FVector(FLinearColor::Yellow));
		}
		else
		{
			AS_LOG_S(Error);
		}
	}
	else
	{
		if (USkeletalMeshComponent* SkeletalMeshComp = GetMesh())
		{
			SkeletalMeshComp->SetScalarParameterValueOnMaterials(TEXT("bShowEffect"), 0.0f);
		}
		else
		{
			AS_LOG_S(Error);
		}
	}
}

void AASCharacter::ServerInclineValue_Implementation(float Value)
{
	InclineValue = Value;
}
