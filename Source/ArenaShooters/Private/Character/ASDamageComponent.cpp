// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/ASDamageComponent.h"
#include "Character/ASCharacter.h"
#include "Character/ASStatusComponent.h"
#include "Character/ASInventoryComponent.h"
#include "ItemActor/ASBullet.h"
#include "ASAssetManager.h"
#include "DataAssets/CharacterDataAssets/ASDamageDataAsset.h"
#include "Item/ASArmor.h"
#include "Controller/ASPlayerController.h"
#include "GameMode/ASMatchGameStateBase.h"
#include "GameMode/ASMatchGameModeBase.h"

UASDamageComponent::UASDamageComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	bWantsInitializeComponent = true;
	SetIsReplicatedByDefault(true);
}

void UASDamageComponent::InitializeComponent()
{
	Super::InitializeComponent();

	ASChar = GetOwner<AASCharacter>();
	verify(ASChar != nullptr);
	ASChar->OnTakeAnyDamage.AddDynamic(this, &UASDamageComponent::OnTakeDamage);

	ASStatus = ASChar->GetStatusComponent();
	verify(ASStatus != nullptr);

	ASInventory = ASChar->GetInventoryComponent();
	verify(ASInventory != nullptr);
	
	DamageDataAsset = UASAssetManager::Get().GetDataAsset<UASDamageDataAsset>(DamageAssetId);
	check(DamageDataAsset != nullptr);
}

void UASDamageComponent::TakeBulletDamage(AASBullet* InBullet, const FHitResult& InHit)
{
	if (!ensure(IsValid(InBullet)))
		return;

	auto InstigatorChar = InBullet->GetOwner<AASCharacter>();
	if (!ensure(IsValid(InstigatorChar)))
		return;

	float Damage = InBullet->GetDamage();
	float TakenDamage = Damage;

	TArray<TWeakObjectPtr<UASArmor>> CoveringArmors = ASInventory->GetCoveringArmors(InHit.BoneName);
	for (auto& Armor : CoveringArmors)
	{
		TakenDamage = Armor->TakeDamage(TakenDamage);
	}

	if (ensure(DamageDataAsset != nullptr))
	{
		TakenDamage *= DamageDataAsset->GetDamageRateByBone(ASChar->GetMesh(), InHit.BoneName);
	}

	FPointDamageEvent DamageEvent(TakenDamage, InHit, InBullet->GetActorForwardVector(), nullptr);
	ASChar->TakeDamage(TakenDamage, DamageEvent, InstigatorChar->GetController(), InBullet);
}

void UASDamageComponent::OnTakeDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
	bool bBeforeDead = ASChar->IsDead();

	ASStatus->ModifyCurrentHealth(-Damage);

	if (!bBeforeDead && ASChar->IsDead())
	{
		auto GameMode = GetWorld()->GetAuthGameMode<AASMatchGameModeBase>();
		if (ensure(GameMode))
		{
			GameMode->OnKillCharacter(Cast<AASPlayerController>(InstigatedBy), ASChar->GetController<AASPlayerController>());
		}
	}
}
