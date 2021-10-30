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
		if (ASAnimInstance != nullptr)
		{
			ASAnimInstance->OnReloadComplete.AddUObject(this, &AASCharacter::ServerCompleteReload);
			ASAnimInstance->OnReloadEnd.AddUObject(this, &AASCharacter::EndReload);
			ASAnimInstance->OnChangeWeaponEnd.AddUObject(this, &AASCharacter::ServerEndSelectWeapon);
			ASAnimInstance->OnUseHealingKitComplete.AddUObject(this, &AASCharacter::ServerCompleteHealingKit);
			ASAnimInstance->OnUseHealingKitEnd.AddUObject(this, &AASCharacter::EndHealingKit);
		}
	}

	if (ASInventory != nullptr)
	{
		ASInventory->OnChangedSelectedWeapon.AddUObject(this, &AASCharacter::OnChangeSelectedWeapon);
	}
}

void AASCharacter::BeginPlay()
{
	Super::BeginPlay();

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

void AASCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bPressedAimButton && CanAimOrScope() && IsLocallyControlled())
	{
		AimKeyHoldTime += DeltaSeconds;
		if (AimKeyHoldTime >= MaxAimKeyHoldTime)
		{
			ResetAimKeyState();
			ServerChangeShootingStance(EShootingStanceType::Aiming);
		}
	}

	if (ShootingStance != EShootingStanceType::None)
	{
		if (IsLocallyControlled() || GetLocalRole() == ROLE_Authority)
		{
			AimOffsetRotator = (GetControlRotation() - GetActorRotation()).GetNormalized();
		}
	}

	if (bPressedShootButton && IsLocallyControlled())
	{
		Shoot();
	}

	if (GetLocalRole() == ROLE_Authority)
	{
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
	DOREPLIFETIME(AASCharacter, bReloading);
	DOREPLIFETIME(AASCharacter, bDead);
	DOREPLIFETIME(AASCharacter, bChangeWeapon);
	DOREPLIFETIME(AASCharacter, bUseHealingKit);
}

void AASCharacter::SetPlayerDefaults()
{
	Super::SetPlayerDefaults();

	bDead = false;
	SetCanBeDamaged(true);

	if (IsValid(ASStatus))
	{
		ASStatus->SetStatusDefaults();
	}

	auto GameState = GetWorld()->GetGameState<AASMatchGameStateBase>();
	if (IsValid(GameState))
	{
		if (GameState->IsMatchProcess())
		{
			auto ASPlayerState = GetPlayerState<AASPlayerState>();
			if (IsValid(ASPlayerState))
			{
				if (IsValid(ASInventory))
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
	if (IsLocallyControlled())
	{
		if (bIsCrouched)
		{
			UnCrouch(true);
		}
	}	

	if (GetLocalRole() == ROLE_Authority)
	{
		if (ShootingStance != EShootingStanceType::None)
		{
			ServerChangeShootingStance(EShootingStanceType::None);
		}

		if (bUseHealingKit)
		{
			MulticastCancelUseHealingKit();
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
			FDelegateHandle Handle = DroppedItemActor->OnRemoveItemEvent.AddUObject(this, &AASCharacter::OnRemoveGroundItem);
			GroundItemActorSet.Emplace(TPair<TWeakObjectPtr<AASDroppedItemActor>, FDelegateHandle>(MakeWeakObjectPtr(DroppedItemActor), Handle));

			OnGroundItemAdd.Broadcast(DroppedItemActor->GetItems());
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
		if (auto DroppedItemActor = Cast<AASDroppedItemActor>(OtherActor))
		{
			for (auto Itr = GroundItemActorSet.CreateIterator(); Itr; ++Itr)
			{
				if ((Itr->Key).IsValid() && (Itr->Key).Get() == DroppedItemActor)
				{
					DroppedItemActor->OnRemoveItemEvent.Remove(Itr->Value);
					Itr.RemoveCurrent();

					OnGroundItemRemove.Broadcast(DroppedItemActor->GetItems());
					break;
				}
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

EWeaponType AASCharacter::GetUsingWeaponType() const
{
	return (ASInventory != nullptr) ? ASInventory->GetSelectedWeaponType() : EWeaponType::None;
}

FRotator AASCharacter::GetAimOffsetRotator() const
{
	return AimOffsetRotator;
}

EShootingStanceType AASCharacter::GetShootingStance() const
{
	return ShootingStance;
}

void AASCharacter::MulticastPlayShootMontage_Implementation()
{
	if (GetLocalRole() == ROLE_Authority)
		return;

	if (ASAnimInstance != nullptr)
	{
		ASAnimInstance->PlayShootMontage(GetUsingWeaponType());

		OnPlayShootMontage.Broadcast();
	}

	if (ASInventory != nullptr)
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

		TWeakObjectPtr<AASWeaponActor> WeaponActor = ASInventory->GetSelectedWeaponActor();
		if (WeaponActor.IsValid())
		{
			WeaponActor->PlayFireAnim();
		}
	}
}

UASInventoryComponent* AASCharacter::GetInventoryComponent()
{
	return ASInventory;
}

TArray<TWeakObjectPtr<UASItem>> AASCharacter::GetGroundItems() const
{
	TArray<TWeakObjectPtr<UASItem>> GroundItems;

	for (auto& Pair : GroundItemActorSet)
	{
		if (!(Pair.Key).IsValid())
			continue;

		GroundItems += (Pair.Key)->GetItems();
	}

	return GroundItems;
}

void AASCharacter::ServerPickUpWeapon_Implementation(EWeaponSlotType SlotType, UASWeapon* NewWeapon)
{
	if (ASInventory == nullptr)
	{
		AS_LOG_S(Error);
		return;
	}

	if (bUseHealingKit)
	{
		AS_LOG_S(Error);
		return;
	}

	if (!ASInventory->IsSuitableWeaponSlot(SlotType, NewWeapon))
		return;

	MulticastPlayPickUpItemMontage();

	auto DroppedItemActor = Cast<AASDroppedItemActor>(NewWeapon->GetOwner());
	if (!IsValid(DroppedItemActor))
	{
		AS_LOG_S(Error);
		return;
	}

	if (!DroppedItemActor->RemoveItem(NewWeapon))
	{
		AS_LOG_S(Error);
		return;
	}

	UASItem* OldWeapon = nullptr;
	if (ASInventory->InsertWeapon(SlotType, NewWeapon, OldWeapon))
	{
		SpawnDroppedItemActor(OldWeapon);
	}
	else
	{
		DroppedItemActor->AddItem(NewWeapon);
		AS_LOG_S(Error);
	}
}

void AASCharacter::ServerPickUpArmor_Implementation(EArmorSlotType SlotType, UASArmor* NewArmor)
{
	if (ASInventory == nullptr)
	{
		AS_LOG_S(Error);
		return;
	}

	if (bUseHealingKit)
	{
		AS_LOG_S(Error);
		return;
	}

	if (!ASInventory->IsSuitableArmorSlot(SlotType, NewArmor))
		return;

	MulticastPlayPickUpItemMontage();

	auto DroppedItemActor = Cast<AASDroppedItemActor>(NewArmor->GetOwner());
	if (!IsValid(DroppedItemActor))
	{
		AS_LOG_S(Error);
		return;
	}

	if (!DroppedItemActor->RemoveItem(NewArmor))
	{
		AS_LOG_S(Error);
		return;
	}

	UASItem* OldArmor = nullptr;
	if (ASInventory->InsertArmor(SlotType, NewArmor, OldArmor))
	{
		SpawnDroppedItemActor(OldArmor);
	}
	else
	{
		DroppedItemActor->AddItem(NewArmor);
		AS_LOG_S(Error);
	}
}

void AASCharacter::ServerDropItem_Implementation(UASItem* InItem)
{
	if (ASInventory == nullptr)
	{
		AS_LOG_S(Error);
		return;
	}

	if (InItem == nullptr)
	{
		AS_LOG_S(Error);
		return;
	}

	if (bUseHealingKit)
	{
		AS_LOG_S(Error);
		return;
	}

	if (bReloading)
	{
		TWeakObjectPtr<UASWeapon> SelectedWeapon = ASInventory->GetSelectedWeapon();
		if (SelectedWeapon.IsValid() && SelectedWeapon.Get() == InItem)
		{
			MulticastCancelReload();
		}
		else if (auto Ammo = Cast<UASAmmo>(InItem))
		{
			if (Ammo->GetAmmoType() == SelectedWeapon->GetAmmoType())
			{
				MulticastCancelReload();
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

void AASCharacter::ServerPickUpInventoryItem_Implementation(UASItem* NewItem)
{
	if (ASInventory == nullptr)
	{
		AS_LOG_S(Error);
		return;
	}

	if (bUseHealingKit)
	{
		AS_LOG_S(Error);
		return;
	}

	if (!ASInventory->IsEnableToAddItemToInventory(NewItem))
		return;

	MulticastPlayPickUpItemMontage();

	auto DroppedItemActor = Cast<AASDroppedItemActor>(NewItem->GetOwner());
	if (!IsValid(DroppedItemActor))
	{
		AS_LOG_S(Error);
		return;
	}

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

bool AASCharacter::RemoveItem(UASItem* InItem)
{
	if (!IsValid(ASInventory))
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

void AASCharacter::PickUpWeapon(EWeaponSlotType SlotType, UASWeapon* NewWeapon)
{
	if (ASInventory == nullptr)
	{
		AS_LOG_S(Error);
		return;
	}

	if (bUseHealingKit)
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
	if (ASInventory == nullptr)
	{
		AS_LOG_S(Error);
		return;
	}

	if (bUseHealingKit)
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
	if (ASInventory == nullptr)
	{
		AS_LOG_S(Error);
		return;
	}

	if (bUseHealingKit)
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

void AASCharacter::DropItem(UASItem* InItem)
{
	if (ASInventory == nullptr)
	{
		AS_LOG_S(Error);
		return;
	}

	if (InItem == nullptr)
	{
		AS_LOG_S(Error);
		return;
	}

	if (bUseHealingKit)
	{
		AS_LOG_S(Error);
		return;
	}

	ServerDropItem(InItem);
}

void AASCharacter::MulticastPlayPickUpItemMontage_Implementation()
{
	if (GetLocalRole() == ROLE_Authority)
		return;

	if (ASAnimInstance != nullptr)
	{
		ASAnimInstance->PlayPickUpItemMontage();
	}
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
		}
	}

	if (UCapsuleComponent* CapsuleComp = GetCapsuleComponent())
	{
		CapsuleComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
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
	}

	if (UCapsuleComponent* CapsuleComp = GetCapsuleComponent())
	{
		CapsuleComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
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
	PlayerInputComponent->BindAction("HealingKit", IE_Pressed, this, &AASCharacter::HealingKit);
	PlayerInputComponent->BindAction("Function", IE_Pressed, this, &AASCharacter::DoFunction);

	PlayerInputComponent->BindAxis("MoveForward", this, &AASCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AASCharacter::MoveRight);
	PlayerInputComponent->BindAxis("Turn", this, &AASCharacter::Turn);
	PlayerInputComponent->BindAxis("TurnRate", this, &AASCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AASCharacter::LookUpAtRate);
}

void AASCharacter::OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PrevMovementMode, PreviousCustomMode);

	if (ASAction == nullptr)
		return;

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

void AASCharacter::MoveForward(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
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

void AASCharacter::MoveRight(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(Direction, Value);
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

void AASCharacter::Sprint()
{
	if (bSprinted || (ShootingStance != EShootingStanceType::None) || bReloading || bDead)
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

void AASCharacter::SelectMainWeapon()
{
	if (ASInventory == nullptr)
		return;
	if (ASInventory->GetSelectedWeaponSlotType() == EWeaponSlotType::Main || bDead || bReloading || bChangeWeapon || bUseHealingKit)
		return;

	ServerSelectWeapon(EWeaponSlotType::Main);
}

void AASCharacter::SelectSubWeapon()
{
	if (ASInventory == nullptr)
		return;
	if (ASInventory->GetSelectedWeaponSlotType() == EWeaponSlotType::Sub || bDead || bReloading || bChangeWeapon || bUseHealingKit)
		return;

	ServerSelectWeapon(EWeaponSlotType::Sub);
}

void AASCharacter::PressedShootButton()
{
	if (bReloading || bDead || bChangeWeapon || bUseHealingKit)
		return;

	TWeakObjectPtr<UASWeapon> Weapon = ASInventory->GetSelectedWeapon();
	if (!Weapon.IsValid())
	{
		ItemPtrBoolPair Pair = ASInventory->FindItemFromWeaponSlot(EWeaponSlotType::Main);
		return;
	}		

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

void AASCharacter::ChangeFireMode()
{
	if (bDead || bReloading || bChangeWeapon || bUseHealingKit)
		return;

	ServerChangeFireMode();
}

void AASCharacter::Reload()
{
	if (ASInventory == nullptr)
	{
		AS_LOG_S(Error);
		return;
	}

	if (bReloading)
	{
		AS_LOG_S(Error);
		return;
	}

	if (GetCharacterMovement()->IsFalling() || bDead || bUseHealingKit)
		return;

	TWeakObjectPtr<UASWeapon> CurWeapon = ASInventory->GetSelectedWeapon();
	if (!CurWeapon.IsValid() || !CurWeapon->CanReload())
		return;

	int32 InventoryAmmoCount = ASInventory->GetAmmoCountInInventory(CurWeapon->GetAmmoType());
	if (InventoryAmmoCount <= 0)
		return;

	ServerBeginReload();
}

void AASCharacter::HealingKit()
{
	if (ASInventory == nullptr)
	{
		AS_LOG_S(Error);
		return;
	}

	if (bUseHealingKit)
	{
		AS_LOG_S(Error);
		return;
	}

	if (GetCharacterMovement()->IsFalling() || bDead)
		return;

	// todo: remove comment
	//if (ASStatus != nullptr && ASStatus->GetCurrentHealth() >= ASStatus->GetMaxHealth())
	//	return;

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

void AASCharacter::DoFunction()
{
}

void AASCharacter::Shoot()
{
	if (ShootingStance == EShootingStanceType::None || bReloading || bDead || bChangeWeapon || bUseHealingKit)
		return;
	if (ASInventory == nullptr)
		return;

	TWeakObjectPtr<UASWeapon> Weapon = ASInventory->GetSelectedWeapon();
	if (!Weapon.IsValid())
		return;
	if (!Weapon->IsPassedFireInterval())
		return;

	if (Weapon->GetCurrentAmmoCount() <= 0)
	{
		// todo: 탄약 없음 알림
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
	if (bSprinted || ShootingStance != EShootingStanceType::None || bReloading || bDead)
		return;

	if (bUseHealingKit)
	{
		MulticastCancelUseHealingKit();
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
	if (ASInventory == nullptr)
		return;
	if (ASInventory->GetSelectedWeaponSlotType() == WeaponSlotType || bDead || bReloading || bChangeWeapon || bUseHealingKit)
		return;

	ServerChangeShootingStance(EShootingStanceType::None);

	ItemPtrBoolPair ResultPair = ASInventory->FindItemFromWeaponSlot(WeaponSlotType);
	if (!ResultPair.Value)
		return;

	if (ResultPair.Key != nullptr)
	{
		if (bReloading)
		{
			MulticastCancelReload();
		}

		if (ASInventory->SelectWeapon(WeaponSlotType))
		{
			bChangeWeapon = true;
		}
	}
	else
	{
		// todo: delete
		FPrimaryAssetId& WeaponAssetId = (WeaponSlotType == EWeaponSlotType::Main) ? TestARAssetId : TestPistolAssetId;

		if (auto WeaponDataAsset = UASAssetManager::Get().GetDataAsset<UASWeaponDataAsset>(WeaponAssetId))
		{
			UASItem* OldWeapon = nullptr;
			if (ASInventory->InsertWeapon(WeaponSlotType, UASItemFactoryComponent::NewASItem<UASWeapon>(GetWorld(), this, WeaponDataAsset), OldWeapon))
			{
				if (OldWeapon != nullptr)
				{
					AS_LOG_S(Error);
				}
			}
		}
	}
}

void AASCharacter::ServerEndSelectWeapon_Implementation()
{
	bChangeWeapon = false;
}

void AASCharacter::OnRep_bChangeWeapon()
{
	if (bChangeWeapon)
	{
		if (ASAnimInstance != nullptr)
		{
			ASAnimInstance->PlayEquipMontage(GetUsingWeaponType());
		}
	}
}

void AASCharacter::ServerChangeShootingStance_Implementation(EShootingStanceType NewShootingStance)
{
	if (ShootingStance == NewShootingStance || bUseHealingKit)
		return;

	if (NewShootingStance != EShootingStanceType::None && bChangeWeapon)
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
}

bool AASCharacter::CanAimOrScope() const
{
	return (ShootingStance == EShootingStanceType::None) && (GetUsingWeaponType() != EWeaponType::None) && 
		!GetCharacterMovement()->IsFalling() && !bReloading && !bDead && !bChangeWeapon && !bShownFullScreenWidget &&
		!bUseHealingKit;
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
	if (ShootingStance == EShootingStanceType::None || bReloading || bDead || bChangeWeapon || bUseHealingKit)
		return;
	if (ASInventory == nullptr)
		return;

	TWeakObjectPtr<UASWeapon> SelectedWeapon = ASInventory->GetSelectedWeapon();
	if (!SelectedWeapon.IsValid())
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

void AASCharacter::ServerChangeFireMode_Implementation()
{
	if (bDead || bReloading || bChangeWeapon || bUseHealingKit)
		return;

	TWeakObjectPtr<UASWeapon> Weapon = ASInventory->GetSelectedWeapon();
	if (!Weapon.IsValid())
		return;

	Weapon->ChangeToNextFireMode();
}

void AASCharacter::SpawnDroppedItemActor(UASItem* DroppingItem)
{
	if (DroppingItem == nullptr)
		return;

	USkeletalMeshComponent* SkeletalMeshComp = GetMesh();
	FVector Location = (SkeletalMeshComp != nullptr ? SkeletalMeshComp->GetComponentLocation() : GetActorLocation());

	if (auto DroppedItemActor = GetWorld()->SpawnActor<AASDroppedItemActor>(DroppingItem->GetDroppedItemActorClass(), Location, GetActorRotation()))
	{
		DroppedItemActor->AddItem(DroppingItem);
	}
}

void AASCharacter::OnRemoveGroundItem(const TWeakObjectPtr<UASItem>& Item)
{
	OnGroundItemRemove.Broadcast(TArray<TWeakObjectPtr<UASItem>>{ Item });
}

void AASCharacter::ServerBeginReload_Implementation()
{
	if (ASInventory == nullptr)
	{
		AS_LOG_S(Error);
		return;
	}

	if (GetCharacterMovement()->IsFalling() || bReloading || bChangeWeapon || bUseHealingKit)
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

	bReloading = true;
	ReloadStartTime = FDateTime::Now();
}

bool AASCharacter::ServerCompleteReload_Validate()
{
	if (ASInventory == nullptr)
	{
		AS_LOG_S(Error);
		return false;
	}

	if (!bReloading)
	{
		AS_LOG_S(Error);
		return false;
	}

	TWeakObjectPtr<UASWeapon> SelectedWeapon = ASInventory->GetSelectedWeapon();
	if (!SelectedWeapon.IsValid() || !SelectedWeapon->CanReload())
	{
		AS_LOG_S(Error);
		return false;
	}

	return true;
}

void AASCharacter::ServerCompleteReload_Implementation()
{
	TWeakObjectPtr<UASWeapon> SelectedWeapon = ASInventory->GetSelectedWeapon();

	if (FDateTime::Now() - ReloadStartTime >= SelectedWeapon->GetReloadTime())
	{
		TArray<UASAmmo*> Ammos = ASInventory->GetAmmos(SelectedWeapon->GetAmmoType());
		if (!SelectedWeapon->Reload(Ammos))
		{
			AS_LOG_S(Error);
		}
	}
}

void AASCharacter::ServerEndReload_Implementation()
{
	bReloading = false;
	ReloadStartTime = FDateTime::MaxValue();
}

void AASCharacter::EndReload()
{
	if (!bReloading)
		return;

	ServerEndReload();
}

void AASCharacter::MulticastCancelReload_Implementation()
{
	if (GetLocalRole() == ROLE_Authority)
	{
		ServerEndReload();
	}
	else
	{
		if (ASAnimInstance != nullptr)
		{
			ASAnimInstance->Montage_Stop(0.1f);
		}
	}
}

void AASCharacter::OnRep_bReloading(bool OldbReloading)
{
	if (bReloading && !OldbReloading)
	{
		if (ASAnimInstance != nullptr)
		{
			ASAnimInstance->PlayReloadMontage(GetUsingWeaponType());
		}
	}
}

void AASCharacter::Die()
{
	if (bDead)
		return;

	if (GetLocalRole() == ROLE_Authority)
	{
		if (bSprinted)
		{
			ServerSprintEnd();
		}

		if (ShootingStance != EShootingStanceType::None)
		{
			ServerChangeShootingStance(EShootingStanceType::None);
		}

		if (bReloading)
		{
			MulticastCancelReload();
		}

		if (bUseHealingKit)
		{
			MulticastCancelUseHealingKit();
		}

		bDead = true;
		SetCanBeDamaged(false);

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
	if (ASInventory == nullptr)
	{
		AS_LOG_S(Error);
		return;
	}

	if (bUseHealingKit)
	{
		AS_LOG_S(Error);
		return;
	}

	if (InHealingKit == nullptr)
	{
		AS_LOG_S(Error);
		return;
	}

	if (InHealingKit->GetCount() <= 0)
	{
		AS_LOG_S(Error);
		return;
	}

	if (!ASInventory->Contains(InHealingKit))
	{
		AS_LOG_S(Error);
		return;
	}

	if (GetCharacterMovement()->IsFalling() || bDead)
		return;

	// todo: check currrent health
	//if (ASStatus != nullptr && ASStatus->GetCurrentHealth() >= ASStatus->GetMaxHealth())
	//	return;

	if (bSprinted)
	{
		ServerSprintEnd();
	}

	if (GetShootingStance() != EShootingStanceType::None)
	{
		ServerChangeShootingStance(EShootingStanceType::None);
	}

	if (bReloading)
	{
		MulticastCancelReload();
	}

	TWeakObjectPtr<UASWeapon> CurWeapon = ASInventory->GetSelectedWeapon();
	if (CurWeapon.IsValid())
	{
		ASInventory->ReattachWeaponActor(CurWeapon.Get(), ASInventory->GetProperWeaponSocketName(CurWeapon->GetWeaponType(), false));
	}

	SetMaxWalkSpeedRate(UseHealingKitSpeedRate);

	bUseHealingKit = true;
	UsingHealingKit = InHealingKit;
	HealingKitStartTime = FDateTime::Now();
}

bool AASCharacter::ServerCompleteHealingKit_Validate()
{
	if (ASInventory == nullptr)
	{
		AS_LOG_S(Error);
		return false;
	}

	if (!bUseHealingKit)
	{
		AS_LOG_S(Error);
		return false;
	}

	if (!IsValid(UsingHealingKit))
	{
		AS_LOG_S(Error);
		return false;
	}

	if (UsingHealingKit->GetCount() <= 0)
	{
		AS_LOG_S(Error);
		return false;
	}

	if (!ASInventory->Contains(UsingHealingKit))
	{
		AS_LOG_S(Error);
		return false;
	}

	return true;
}

void AASCharacter::ServerCompleteHealingKit_Implementation()
{
	if (ASInventory == nullptr)
	{
		AS_LOG_S(Error);
		return;
	}

	if (FDateTime::Now() - HealingKitStartTime >= UsingHealingKit->GetUsingTime())
	{
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
}

void AASCharacter::ServerEndHealingKit_Implementation()
{
	TWeakObjectPtr<UASWeapon> CurWeapon = ASInventory->GetSelectedWeapon();
	if (CurWeapon.IsValid())
	{
		ASInventory->ReattachWeaponActor(CurWeapon.Get(), ASInventory->GetProperWeaponSocketName(CurWeapon->GetWeaponType(), true));
	}

	SetMaxWalkSpeedRate(1.0f);

	bUseHealingKit = false;
	UsingHealingKit = nullptr;
	HealingKitStartTime = FDateTime::MaxValue();
}

void AASCharacter::EndHealingKit()
{
	if (!bUseHealingKit)
		return;

	ServerEndHealingKit();
}

void AASCharacter::OnRep_bUseHealingKit(bool OldbUseHealingKit)
{
	if (bUseHealingKit && !OldbUseHealingKit)
	{
		if (ASAnimInstance != nullptr)
		{
			ASAnimInstance->PlayUseHealingKitMontage();
		}

		SetMaxWalkSpeedRate(UseHealingKitSpeedRate);
	}
	else if (!bUseHealingKit && OldbUseHealingKit)
	{
		SetMaxWalkSpeedRate(1.0f);
	}
}

void AASCharacter::MulticastCancelUseHealingKit_Implementation()
{
	if (GetLocalRole() == ROLE_Authority)
	{
		ServerEndHealingKit();
	}
	else
	{
		if (ASAnimInstance != nullptr)
		{
			ASAnimInstance->Montage_Stop(0.1f);
		}
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
