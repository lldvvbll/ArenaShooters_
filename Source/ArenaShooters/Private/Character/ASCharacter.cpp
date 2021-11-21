// Copyright Epic Games, Inc. All Rights Reserved.

#include "Character/ASCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
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
	bHoldingShootButton = false;
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

	USkeletalMeshComponent* SkMesh = GetMesh();
	if (ensure(SkMesh != nullptr))
	{
		ASAnimInstance = Cast<UASAnimInstance>(SkMesh->GetAnimInstance());
	}

	if (ensure(ASInventory != nullptr))
	{
		ASInventory->OnChangedSelectedWeapon.AddUObject(this, &AASCharacter::OnChangeSelectedWeapon);
	}
}

void AASCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	if (ensure(ASAnimInstance != nullptr))
	{
		if (GetLocalRole() == ROLE_Authority)
		{
			ASAnimInstance->OnReloadComplete.AddUObject(this, &AASCharacter::CompleteReload);
			ASAnimInstance->OnUseHealingKitComplete.AddUObject(this, &AASCharacter::CompleteUseHealingKit);
		}

		ASAnimInstance->OnUseHealingKitEnd.AddUObject(this, &AASCharacter::OnEndUseHealingKitMontage);
	}

	auto GameState = GetWorld()->GetGameState<AASMatchGameStateBase>();
	if (ensure(IsValid(GameState)))
	{
		GameState->OnChangedInnerMatchState.AddUObject(this, &AASCharacter::OnChangedInnerMatchState);

		OnChangedInnerMatchState(GameState->GetInnerMatchState());
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

		if (bHoldingShootButton)
		{
			if (!Shoot())
			{
				ReleasedShootButton();
			}
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

	if (ensure(ASStatus != nullptr))
	{
		ASStatus->SetStatusDefaults();
	}

	auto GameState = GetWorld()->GetGameState<AASMatchGameStateBase>();
	if (ensure(IsValid(GameState)))
	{
		if (GameState->IsMatchProcess())
		{
			auto ASPlayerState = GetPlayerState<AASPlayerState>();
			if (ensure(IsValid(ASPlayerState)))
			{
				if (ensure(ASInventory != nullptr))
				{
					ASInventory->ClearAllItems();

					UASItemSetDataAsset* DataAsset = ASPlayerState->GetItemSetDataAsset();
					ASInventory->EquipItemsByItemSetDataAsset(DataAsset);
				}
			}
		}
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
		if (ensure(ASAnimInstance != nullptr))
		{
			if (ASAnimInstance->IsPlayingUseHealingKitMontage())
			{
				MulticastStopUseHealingKitMontage();
			}
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
			if (ensure(ASDamageComp != nullptr))
			{
				ASDamageComp->TakeBulletDamage(Bullet, Hit);
			}
		}
	}
	else
	{
		if (bHitByBullet)
		{
			UGameplayStatics::SpawnEmitterAttached(BloodParticle, MyComp, NAME_None, HitLocation, HitNormal.ToOrientationRotator(),
				EAttachLocation::KeepWorldPosition);

			if (!IsLocallyControlled() || ShootingStance != EShootingStanceType::Scoping)
			{
				if (ensure(ASAnimInstance != nullptr))
				{
					ASAnimInstance->PlayHitReactMontage();
				}
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
	if (!ensure(ASInventory != nullptr))
		return EWeaponType::None;

	return ASInventory->GetSelectedWeaponType();
}

TWeakObjectPtr<UASWeapon> AASCharacter::GetUsingWeapon() const
{
	if (!ensure(ASInventory != nullptr))
		return TWeakObjectPtr<UASWeapon>();

	return ASInventory->GetSelectedWeapon();
}

TWeakObjectPtr<AASWeaponActor> AASCharacter::GetUsingWeaponActor() const
{
	if (!ensure(ASInventory != nullptr))
		return TWeakObjectPtr<AASWeaponActor>();

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
	if (!ensure(CanPickUpItem()))
		return;

	if (!ensure(ASInventory != nullptr))
		return;

	if (!ensure(ASInventory->IsSuitableWeaponSlot(SlotType, NewWeapon)))
		return;

	auto DroppedItemActor = Cast<AASDroppedItemActor>(NewWeapon->GetOwner());
	if (!ensure(IsValid(DroppedItemActor)))
		return;

	if (!ensure(IsInteractableActor(DroppedItemActor)))
		return;

	if (GetShootingStance() != EShootingStanceType::None)
	{
		ServerChangeShootingStance(EShootingStanceType::None);
	}

	MulticastPlayPickUpItemMontage();

	if (!ensure(DroppedItemActor->RemoveItem(NewWeapon)))
		return;

	UASItem* OldWeapon = nullptr;
	if (ensure(ASInventory->InsertWeapon(SlotType, NewWeapon, OldWeapon)))
	{
		if (OldWeapon != nullptr)
		{
			SpawnDroppedItemActor(OldWeapon);
		}		
	}
	else
	{
		DroppedItemActor->AddItem(NewWeapon);
	}
}

void AASCharacter::ServerPickUpArmor_Implementation(EArmorSlotType SlotType, UASArmor* NewArmor)
{
	if (!ensure(CanPickUpItem()))
		return;

	if (!ensure(ASInventory != nullptr))
		return;

	if (!ensure(ASInventory->IsSuitableArmorSlot(SlotType, NewArmor)))
		return;
		
	auto DroppedItemActor = Cast<AASDroppedItemActor>(NewArmor->GetOwner());
	if (!ensure(IsValid(DroppedItemActor)))
		return;

	if (!ensure(IsInteractableActor(DroppedItemActor)))
		return;

	if (GetShootingStance() != EShootingStanceType::None)
	{
		ServerChangeShootingStance(EShootingStanceType::None);
	}

	MulticastPlayPickUpItemMontage();

	if (!ensure(DroppedItemActor->RemoveItem(NewArmor)))
		return;

	UASItem* OldArmor = nullptr;
	if (ensure(ASInventory->InsertArmor(SlotType, NewArmor, OldArmor)))
	{
		if (OldArmor != nullptr)
		{
			SpawnDroppedItemActor(OldArmor);
		}
	}
	else
	{
		DroppedItemActor->AddItem(NewArmor);
	}
}

void AASCharacter::ServerPickUpInventoryItem_Implementation(UASItem* NewItem)
{
	if (!ensure(CanPickUpItem()))
		return;

	if (!ensure(ASInventory != nullptr))
		return;

	if (!ASInventory->IsEnableToAddItemToInventory(NewItem))
		return;

	auto DroppedItemActor = Cast<AASDroppedItemActor>(NewItem->GetOwner());
	if (!ensure(IsValid(DroppedItemActor)))
		return;

	if (!ensure(IsInteractableActor(DroppedItemActor)))
		return;

	if (GetShootingStance() != EShootingStanceType::None)
	{
		ServerChangeShootingStance(EShootingStanceType::None);
	}

	MulticastPlayPickUpItemMontage();

	if (!ensure(DroppedItemActor->RemoveItem(NewItem)))
		return;

	if (!ensure(ASInventory->AddItemToInventory(NewItem)))
	{
		DroppedItemActor->AddItem(NewItem);
	}
}

void AASCharacter::ServerDropItem_Implementation(UASItem* InItem)
{
	if (!ensure(IsValid(InItem)))
		return;

	if (!ensure(CanDropItem()))
		return;

	if (!ensure(ASInventory != nullptr))
		return;

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
	if (!ensure(ResultPair.Value))
		return;

	SpawnDroppedItemActor(ResultPair.Key);
}

bool AASCharacter::RemoveItem(UASItem* InItem)
{
	if (!ensure(ASInventory != nullptr))
		return false;

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
	if (!ensure(ASAnimInstance != nullptr))
		return false;

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

	if (!ensure(IsValid(InItem)))
		return;

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

	if (!ensure(ASInventory != nullptr))
		return;

	if (!ensure(ASInventory->IsSuitableWeaponSlot(SlotType, NewWeapon)))
		return;

	ServerPickUpWeapon(SlotType, NewWeapon);
}

void AASCharacter::PickUpArmor(EArmorSlotType SlotType, UASArmor* NewArmor)
{
	if (!CanPickUpItem())
		return;

	if (!ensure(ASInventory != nullptr))
		return;

	if (!ensure(ASInventory->IsSuitableArmorSlot(SlotType, NewArmor)))
		return;

	ServerPickUpArmor(SlotType, NewArmor);
}

void AASCharacter::PickUpInventoryItem(UASItem* NewItem)
{
	if (!CanPickUpItem())
		return;

	if (!ensure(ASInventory != nullptr))
		return;

	if (!ensure(ASInventory->IsEnableToAddItemToInventory(NewItem)))
		return;

	ServerPickUpInventoryItem(NewItem);
}

bool AASCharacter::CanDropItem() const
{
	if (!ensure(ASAnimInstance != nullptr))
		return false;

	if (ASAnimInstance->IsPlayingUseHealingKitMontage())
		return false;

	return true;
}

void AASCharacter::DropItem(UASItem* InItem)
{
	if (!ensure(IsValid(InItem)))
		return;

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
		UCharacterMovementComponent* CharMoveComp = GetCharacterMovement();
		if (ensure(CharMoveComp != nullptr))
		{
			CharMoveComp->SetMovementMode(EMovementMode::MOVE_None);
		}
	}
	else if (NetMode == NM_Client)
	{
		USkeletalMeshComponent* SkeletalMeshComp = GetMesh();
		if (ensure(SkeletalMeshComp != nullptr))
		{
			SkeletalMeshComp->SetCollisionObjectType(ECollisionChannel::ECC_PhysicsBody);
			SkeletalMeshComp->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
			SkeletalMeshComp->SetCollisionProfileName(TEXT("Ragdoll"));
			SkeletalMeshComp->SetAllBodiesBelowSimulatePhysics(FName(TEXT("pelvis")), true, true);

			auto AnimInstance = Cast<UASAnimInstance>(SkeletalMeshComp->GetAnimInstance());
			if (ensure(AnimInstance != nullptr))
			{
				AnimInstance->StopAllMontages(0.2f);
			}
		}
	}

	UCapsuleComponent* CapsuleComp = GetCapsuleComponent();
	if (ensure(CapsuleComp != nullptr))
	{
		CapsuleComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void AASCharacter::EndRagdoll()
{
	ENetMode NetMode = GetNetMode();
	if (NetMode == NM_DedicatedServer)
	{
		// nothing
	}
	else if (NetMode == NM_Client)
	{
		USkeletalMeshComponent* SkeletalMeshComp = GetMesh();
		if (ensure(SkeletalMeshComp != nullptr))
		{
			SkeletalMeshComp->SetCollisionObjectType(ECollisionChannel::ECC_Pawn);
			SkeletalMeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			SkeletalMeshComp->SetCollisionProfileName(TEXT("CharacterMesh"));
			SkeletalMeshComp->ResetAllBodiesSimulatePhysics();
		}
	}

	UCapsuleComponent* CapsuleComp = GetCapsuleComponent();
	if (ensure(CapsuleComp != nullptr))
	{
		CapsuleComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
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
	ResetAimKeyState();
	ReleasedShootButton();

	if (bSprinted)
	{
		ServerSprintEnd();
	}

	if (ShootingStance != EShootingStanceType::None)
	{
		ServerChangeShootingStance(EShootingStanceType::None);
	}

	if (ensure(ASAnimInstance != nullptr))
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
}

void AASCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if (!ensure(PlayerInputComponent != nullptr))
		return;

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

	EMovementMode CurMovementMode = GetCharacterMovement()->MovementMode.GetValue();

	if (ensure(ASAnimInstance != nullptr))
	{
		ASAnimInstance->OnMovementChanged(PrevMovementMode, CurMovementMode);
	}
}

void AASCharacter::MoveForward(float Value)
{
	if (ensure(Controller != nullptr))
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
}

void AASCharacter::MoveRight(float Value)
{
	if (ensure(Controller != nullptr))
	{
		if (Value != 0.0f)
		{
			const FRotator Rotation = Controller->GetControlRotation();
			const FRotator YawRotation(0, Rotation.Yaw, 0);
			const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

			AddMovementInput(Direction, Value);
		}
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
	if (!ensure(ASAnimInstance != nullptr))
		return false;

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
	if (!ensure(ASAnimInstance != nullptr))
		return false;

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

	if (!ensure(ASInventory != nullptr))
		return;

	if (ASInventory->GetSelectedWeaponSlotType() == EWeaponSlotType::Main)
		return;

	ServerSelectWeapon(EWeaponSlotType::Main);
}

void AASCharacter::SelectSubWeapon()
{
	if (!CanSelectWeapon())
		return;

	if (!ensure(ASInventory != nullptr))
		return;

	if (ASInventory->GetSelectedWeaponSlotType() == EWeaponSlotType::Sub)
		return;

	ServerSelectWeapon(EWeaponSlotType::Sub);
}

void AASCharacter::PressedShootButton()
{
	if (!ensure(ASAnimInstance != nullptr))
		return;

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
			bHoldingShootButton = true;
		}
		break;
	default:
		checkNoEntry();
		break;
	}
}

void AASCharacter::ReleasedShootButton()
{
	bHoldingShootButton = false;
}

bool AASCharacter::CanChangeFireMode() const
{
	if (!ensure(ASAnimInstance != nullptr))
		return false;

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

	if (bHoldingShootButton)
		return;

	TWeakObjectPtr<UASWeapon> Weapon = ASInventory->GetSelectedWeapon();
	if (!Weapon.IsValid())
		return;

	ServerChangeFireMode();
}

bool AASCharacter::CanReload() const
{
	if (!ensure(ASAnimInstance != nullptr))
		return false;

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

	if (!ensure(ASInventory != nullptr))
		return;

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
	if (!ensure(ASAnimInstance != nullptr))
		return false;

	if (!ensure(ASStatus != nullptr))
		return false;

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

	if (!ensure(ASInventory != nullptr))
		return;

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
	if (!ensure(ASAnimInstance != nullptr))
		return false;

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

bool AASCharacter::Shoot()
{
	if (!CanShoot())
		return false;

	if (!ensure(ASInventory != nullptr))
		return false;

	TWeakObjectPtr<UASWeapon> Weapon = ASInventory->GetSelectedWeapon();
	if (!Weapon.IsValid())
		return false;

	if (Weapon->GetCurrentAmmoCount() <= 0)
	{
		Weapon->PlayEmptyBulletSound();
		return false;
	}

	const TWeakObjectPtr<AASWeaponActor>& WeaponActor = Weapon->GetActor();
	if (!WeaponActor.IsValid())
		return false;

	if (Weapon->IsPassedFireInterval())
	{
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

	return true;
}

void AASCharacter::ResetAimKeyState()
{
	bPressedAimButton = false;
	AimKeyHoldTime = 0.0f;
}

void AASCharacter::ServerSprint_Implementation()
{
	if (!ensure(CanSprint()))
		return;

	if (ensure(ASAnimInstance != nullptr))
	{
		if (ASAnimInstance->IsPlayingUseHealingKitMontage())
		{
			MulticastStopUseHealingKitMontage();
		}
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
	if (ensure(CharMoveComp != nullptr && DefaultCharMoveComp != nullptr))
	{
		CharMoveComp->MaxWalkSpeed = DefaultCharMoveComp->MaxWalkSpeed * Rate;
		CharMoveComp->MaxWalkSpeedCrouched = DefaultCharMoveComp->MaxWalkSpeedCrouched * Rate;
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
	if (!ensure(CanSelectWeapon()))
		return;

	if (!ensure(ASInventory != nullptr))
		return;

	if (!ensure(ASInventory->GetSelectedWeaponSlotType() != WeaponSlotType))
		return;

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
	if (!ensure(ASAnimInstance != nullptr))
		return;

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
	if (!ensure(ASAnimInstance != nullptr))
		return false;

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

	if (ensure(CameraBoom != nullptr))
	{
		CameraBoom->TargetArmLength = AimingCamArmLength;
		CameraBoom->SocketOffset = AimingCamOffset;
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
	//if (!ensure(SelectedWeaponActor.IsValid()))
	//	return false;

	//FVector LocDiff = SelectedWeaponActor->GetMuzzleLocation() - MuzzleLocation;
	//if (!ensure(FMath::Abs(LocDiff.X) > ValidLocDiff || FMath::Abs(LocDiff.Y) > ValidLocDiff || FMath::Abs(LocDiff.Z) > ValidLocDiff))
	//	return false;

	//FRotator RotDiff = (GetActorRotation() - ShootRotation).GetNormalized();
	//if (!ensure(FMath::Abs(RotDiff.Pitch) > ValidRotDiff || FMath::Abs(RotDiff.Yaw) > ValidRotDiff))
	//	return false;

	TWeakObjectPtr<UASWeapon> SelectedWeapon = ASInventory->GetSelectedWeapon();
	if (!SelectedWeapon.IsValid())
		return false;

	if (!SelectedWeapon->IsPassedFireInterval())
		return false;

	return true;
}

void AASCharacter::ServerShoot_Implementation(const FVector& MuzzleLocation, const FRotator& ShootRotation)
{
	if (!ensure(CanShoot()))
		return;

	if (!ensure(ASInventory != nullptr))
		return;

	TWeakObjectPtr<UASWeapon> SelectedWeapon = ASInventory->GetSelectedWeapon();
	if (!ensure(SelectedWeapon.IsValid()))
		return;

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
	if (ensure(ASAnimInstance != nullptr))
	{
		ASAnimInstance->PlayShootMontage(GetUsingWeaponType());

		OnPlayShootMontage.Broadcast();
	}

	if (ensure(ASInventory != nullptr))
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
}

void AASCharacter::ServerChangeFireMode_Implementation()
{
	if (!ensure(CanChangeFireMode()))
		return;

	TWeakObjectPtr<UASWeapon> Weapon = ASInventory->GetSelectedWeapon();
	if (ensure(Weapon.IsValid()))
	{
		Weapon->ChangeToNextFireMode();
	}
}

void AASCharacter::SpawnDroppedItemActor(UASItem* DroppingItem)
{
	if (!ensure(IsValid(DroppingItem)))
		return;

	USkeletalMeshComponent* SkeletalMeshComp = GetMesh();
	FVector Location = (SkeletalMeshComp != nullptr ? SkeletalMeshComp->GetComponentLocation() : GetActorLocation());

	auto DroppedItemActor = GetWorld()->SpawnActor<AASDroppedItemActor>(DroppingItem->GetDroppedItemActorClass(), Location, GetActorRotation());
	if (ensure(IsValid(DroppedItemActor)))
	{
		DroppedItemActor->AddItem(DroppingItem);
	}
}

void AASCharacter::SpawnDroppedItemsActor(const TArray<UASItem*>& DroppingItems)
{
	USkeletalMeshComponent* SkeletalMeshComp = GetMesh();
	FVector Location = (SkeletalMeshComp != nullptr ? SkeletalMeshComp->GetComponentLocation() : GetActorLocation());

	auto DroppedItemActor = GetWorld()->SpawnActor<AASDroppedItemActor>(DroppedItemBoxActorClass, Location, GetActorRotation());
	if (ensure(IsValid(DroppedItemActor)))
	{
		DroppedItemActor->AddItems(DroppingItems);
	}
}

void AASCharacter::OnRemoveGroundItem(const TWeakObjectPtr<UASItem>& Item)
{
	OnGroundItemRemove.Broadcast(TArray<TWeakObjectPtr<UASItem>>{ Item });
}

void AASCharacter::ServerBeginReload_Implementation()
{
	if (!ensure(CanReload()))
		return;

	if (!ensure(ASInventory != nullptr))
		return;

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
	if (ensure(SelectedWeapon.IsValid()))
	{
		TArray<UASAmmo*> Ammos = ASInventory->GetAmmos(SelectedWeapon->GetAmmoType());
		ensure(SelectedWeapon->Reload(Ammos));
	}
}

void AASCharacter::MulticastPlayReloadMontage_Implementation()
{
	if (ensure(ASAnimInstance != nullptr))
	{
		ASAnimInstance->PlayReloadMontage(GetUsingWeaponType());
	}
}

void AASCharacter::MulticastStopReloadMontage_Implementation()
{
	if (ensure(ASAnimInstance != nullptr))
	{
		ASAnimInstance->StopReloadMontage();
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
	if (!ensure(IsValid(InHealingKit)))
		return;

	if (!ensure(InHealingKit->GetCount() > 0))
		return;

	if (!ensure(CanUseHealingKit()))
		return;

	if (!ensure(ASInventory != nullptr))
		return;

	if (!ensure(ASInventory->Contains(InHealingKit)))
		return;

	if (!ensure(ASAnimInstance != nullptr))
		return;

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
	if (!ensure(ASInventory != nullptr))
		return;

	if (!ensure(IsValid(UsingHealingKit)))
		return;

	if (!ensure(UsingHealingKit->GetCount() > 0))
		return;

	if (!ensure(ASInventory->Contains(UsingHealingKit)))
		return;

	UsingHealingKit->ModifyCount(-1);

	if (ensure(ASStatus != nullptr))
	{
		ASStatus->ModifyCurrentHealth(UsingHealingKit->GetRecoveryPoint());
	}
}

void AASCharacter::OnEndUseHealingKitMontage()
{
	if (GetLocalRole() == ROLE_Authority)
	{
		UsingHealingKit = nullptr;

		if (ensure(ASInventory != nullptr))
		{
			TWeakObjectPtr<UASWeapon> CurWeapon = ASInventory->GetSelectedWeapon();
			if (CurWeapon.IsValid())
			{
				ASInventory->ReattachWeaponActor(CurWeapon.Get(), ASInventory->GetProperWeaponSocketName(CurWeapon->GetWeaponType(), true));
			}
		}
	}

	SetMaxWalkSpeedRate(1.0f);
}

void AASCharacter::MulticastPlayUseHealingKitMontage_Implementation()
{
	SetMaxWalkSpeedRate(UseHealingKitSpeedRate);

	if (ensure(ASAnimInstance != nullptr))
	{
		ASAnimInstance->PlayUseHealingKitMontage();
	}
}

void AASCharacter::MulticastStopUseHealingKitMontage_Implementation()
{
	if (ensure(ASAnimInstance != nullptr))
	{
		ASAnimInstance->StopUseHealingKitMontage();
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
	if (ensure(ASAnimInstance != nullptr))
	{
		ASAnimInstance->PlayPickUpItemMontage();
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
		USkeletalMeshComponent* SkeletalMeshComp = GetMesh();
		if (ensure(SkeletalMeshComp != nullptr))
		{
			SkeletalMeshComp->SetScalarParameterValueOnMaterials(TEXT("bShowEffect"), 1.0f);
			SkeletalMeshComp->SetVectorParameterValueOnMaterials(TEXT("EffectColor"), FVector(FLinearColor::Yellow));
		}
	}
	else
	{
		USkeletalMeshComponent* SkeletalMeshComp = GetMesh();
		if (ensure(SkeletalMeshComp != nullptr))
		{
			SkeletalMeshComp->SetScalarParameterValueOnMaterials(TEXT("bShowEffect"), 0.0f);
		}
	}
}

void AASCharacter::ServerInclineValue_Implementation(float Value)
{
	InclineValue = Value;
}
