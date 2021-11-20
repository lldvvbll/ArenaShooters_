// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/ASWeapon.h"
#include "DataAssets/ItemDataAssets/ASWeaponDataAsset.h"
#include "Net/UnrealNetwork.h"
#include "ItemActor/ASBullet.h"
#include "ItemActor/ASWeaponActor.h"
#include "Character/ASCharacter.h"
#include "Item/ASAmmo.h"

void UASWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UASWeapon, ASWeaponActor);
	DOREPLIFETIME(UASWeapon, CurrentFireMode);
	DOREPLIFETIME(UASWeapon, CurrentAmmoCount);
}

void UASWeapon::SetDataAsset(UASItemDataAsset* NewDataAsset)
{
	Super::SetDataAsset(NewDataAsset);

	SetLastFireTick();
}

const EWeaponType UASWeapon::GetWeaponType() const
{
	auto WeaponDA = Cast<UASWeaponDataAsset>(GetDataAsset());
	check(WeaponDA);

	return WeaponDA->WeaponType;
}

TWeakObjectPtr<AASWeaponActor>& UASWeapon::GetActor()
{
	return ASWeaponActor;
}

const TWeakObjectPtr<AASWeaponActor>& UASWeapon::GetActor() const
{
	return ASWeaponActor;
}

AASBullet* UASWeapon::Fire(EShootingStanceType InShootingStance, const FVector& InMuzzleLocation, const FRotator& InMuzzleRotation)
{
	auto WeaponDA = Cast<UASWeaponDataAsset>(GetDataAsset());
	check(WeaponDA);
	if (WeaponDA == nullptr)
		return nullptr;

	if (CurrentAmmoCount <= 0)
		return nullptr;

	FActorSpawnParameters Param;
	Param.Owner = GetOwner().Get();
	Param.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

	auto Bullet = GetWorld()->SpawnActor<AASBullet>(WeaponDA->ASBulletClass, InMuzzleLocation, InMuzzleRotation, Param);
	if (Bullet != nullptr)
	{
		Bullet->SetDamage(WeaponDA->Damage);

		CurrentAmmoCount--;

		SetLastFireTick();
	}

	return Bullet;
}

EFireMode UASWeapon::GetFireMode() const
{
	return CurrentFireMode;
}

void UASWeapon::ChangeToNextFireMode()
{
	auto WeaponDA = Cast<UASWeaponDataAsset>(GetDataAsset());
	check(WeaponDA);

	if (WeaponDA == nullptr || WeaponDA->FireModes.Num() == 0)
	{
		AS_LOG_S(Error);
		return;
	}

	int32 ModeNum = WeaponDA->FireModes.Num();
	for (int32 Idx = 0; Idx < ModeNum; ++Idx)
	{
		if (CurrentFireMode == WeaponDA->FireModes[Idx])
		{
			int32 NextIdx = Idx + 1;
			if (NextIdx == ModeNum)
			{
				NextIdx = 0;
			}

			CurrentFireMode = WeaponDA->FireModes[NextIdx];
			break;
		}
	}
}

FTimespan UASWeapon::GetFireInterval() const
{
	auto WeaponDA = Cast<UASWeaponDataAsset>(GetDataAsset());
	check(WeaponDA);

	return (WeaponDA != nullptr ? WeaponDA->FireInterval : FTimespan::MaxValue());
}

bool UASWeapon::IsPassedFireInterval() const
{
	FTimespan DeltaTime = FDateTime::Now() - LastFireTime;
	if (DeltaTime < GetFireInterval())
		return false;

	return true;
}

void UASWeapon::SetLastFireTick()
{
	LastFireTime = FDateTime::Now();
}

void UASWeapon::OnRep_CurrentFireMode()
{
	OnFireModeChanged.Broadcast(CurrentFireMode);
}

int32 UASWeapon::GetMaxAmmoCount() const
{
	auto WeaponDA = Cast<UASWeaponDataAsset>(GetDataAsset());
	check(WeaponDA);

	return (WeaponDA != nullptr ? WeaponDA->MaxAmmoCount : 0);
}

int32 UASWeapon::GetCurrentAmmoCount() const
{
	return CurrentAmmoCount;
}

void UASWeapon::OnRep_CurrentAmmoCount()
{
	OnCurrentAmmoCountChanged.Broadcast(CurrentAmmoCount);
}

EAmmoType UASWeapon::GetAmmoType() const
{
	auto WeaponDA = Cast<UASWeaponDataAsset>(GetDataAsset());
	check(WeaponDA);

	return (WeaponDA != nullptr ? WeaponDA->AmmoType : EAmmoType::None);
}

bool UASWeapon::CanReload() const
{
	return (GetMaxAmmoCount() > CurrentAmmoCount);
}

bool UASWeapon::Reload(TArray<UASAmmo*>& InAmmos)
{
	if (InAmmos.Num() <= 0)
	{
		AS_LOG_S(Error);
		return false;
	}

	EAmmoType WeaponAmmoType = GetAmmoType();

	int32 NeedfulAmmoCount = GetMaxAmmoCount() - CurrentAmmoCount;
	for (int32 Idx = InAmmos.Num() - 1; Idx >= 0; --Idx)
	{
		if (NeedfulAmmoCount <= 0)
			break;

		UASAmmo* Ammo = InAmmos[Idx];
		if (Ammo == nullptr)
			continue;

		if (Ammo->GetAmmoType() != WeaponAmmoType)
			continue;

		int32 AmmoCnt = Ammo->GetCount();
		if (NeedfulAmmoCount >= AmmoCnt)
		{
			CurrentAmmoCount += AmmoCnt;
			Ammo->ModifyCount(-AmmoCnt);

			NeedfulAmmoCount -= AmmoCnt;
		}
		else
		{
			CurrentAmmoCount += NeedfulAmmoCount;
			Ammo->ModifyCount(-NeedfulAmmoCount);

			NeedfulAmmoCount = 0;
		}
	}

	return true;
}

FTimespan UASWeapon::GetReloadTime() const
{
	auto WeaponDA = Cast<UASWeaponDataAsset>(GetDataAsset());
	check(WeaponDA);

	return (WeaponDA != nullptr ? WeaponDA->ReloadTime : FTimespan::MaxValue());
}

void UASWeapon::GetRecoil(FVector2D& OutPitch, FVector2D& OutYaw) const
{
	auto WeaponDA = Cast<UASWeaponDataAsset>(GetDataAsset());
	check(WeaponDA);

	if (WeaponDA != nullptr)
	{
		OutPitch = WeaponDA->RecoilPitch;
		OutYaw = WeaponDA->RecoilYaw;
	}
}

float UASWeapon::GetMinBulletSpread() const
{
	auto WeaponDA = Cast<UASWeaponDataAsset>(GetDataAsset());
	check(WeaponDA);

	return (WeaponDA != nullptr ? WeaponDA->MinBulletSpread : TNumericLimits<float>::Max());
}

float UASWeapon::GetMaxBulletSpread() const
{
	auto WeaponDA = Cast<UASWeaponDataAsset>(GetDataAsset());
	check(WeaponDA);

	return (WeaponDA != nullptr ? WeaponDA->MaxBulletSpread : TNumericLimits<float>::Max());
}

float UASWeapon::GetBulletSpreadAmountPerShot() const
{
	auto WeaponDA = Cast<UASWeaponDataAsset>(GetDataAsset());
	check(WeaponDA);

	return (WeaponDA != nullptr ? WeaponDA->BulletSpreadAmountPerShot : TNumericLimits<float>::Max());
}

float UASWeapon::GetBulletSpreadRecoverySpeed() const
{
	auto WeaponDA = Cast<UASWeaponDataAsset>(GetDataAsset());
	check(WeaponDA);

	return (WeaponDA != nullptr ? WeaponDA->BulletSpreadRecoverySpeed : 0.0f);
}

void UASWeapon::PlayEmptyBulletSound()
{
	auto WeaponDA = Cast<UASWeaponDataAsset>(GetDataAsset());
	check(WeaponDA);

	if (WeaponDA != nullptr)
	{
		if (ASWeaponActor.IsValid())
		{
			UGameplayStatics::SpawnSoundAtLocation(GetWorld(), WeaponDA->EmptyBulletSound, ASWeaponActor->GetActorLocation());
		}
		else
		{
			AS_LOG_S(Error);
		}
	}
}
