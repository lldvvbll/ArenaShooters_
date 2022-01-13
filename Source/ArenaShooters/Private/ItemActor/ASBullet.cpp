// Fill out your copyright notice in the Description page of Project Settings.


#include "ItemActor/ASBullet.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Character/ASCharacter.h"

AASBullet::AASBullet()
{
	PrimaryActorTick.bCanEverTick = false;
	SetCanBeDamaged(false);
	bReplicates = true;
	Damage = 0.0f;

	Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	Collision->SetCollisionProfileName(TEXT("Bullet"));
	Collision->CanCharacterStepUpOn = ECB_No;

	Projectile = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Projectile"));
	Projectile->InitialSpeed = 12000.0f;
	Projectile->MaxSpeed = 15000.0f;

	TraceParticle = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("TraceParticle"));
	
	RootComponent = Collision;
	TraceParticle->SetupAttachment(RootComponent);
}

void AASBullet::NotifyHit(UPrimitiveComponent* MyComp, AActor* Other, UPrimitiveComponent* OtherComp, bool bSelfMoved,
	FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit)
{
	Super::NotifyHit(MyComp, Other, OtherComp, bSelfMoved, HitLocation, HitNormal, NormalImpulse, Hit);

	if (GetLocalRole() == ROLE_Authority)
	{
		SetLifeSpan(1.0f);
	}
	else
	{
		if (TraceParticle != nullptr)
		{
			TraceParticle->Deactivate();
		}

		if (Other != nullptr && !Other->IsA(AASCharacter::StaticClass()))
		{
			static const FName InvisibleWall_NAME(TEXT("InvisibleWall"));

			// 보이지 않는 벽에 충돌 했을 경우에는 이펙트와 사운드를 출력하지 않는다.
			if (OtherComp != nullptr && OtherComp->GetCollisionProfileName() != InvisibleWall_NAME)
			{
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), DefaultSparkParticle, Hit.ImpactPoint);

				UGameplayStatics::SpawnDecalAttached(DefaultBulletHoleDecal, FVector(15.0f, 3.0f, 3.0f), OtherComp, NAME_None, HitLocation,
					HitNormal.ToOrientationRotator(), EAttachLocation::KeepWorldPosition, 30.0f);
			}
		}		
	}
}

float AASBullet::GetDamage() const
{
	return Damage;
}

void AASBullet::SetDamage(float NewDamage)
{
	Damage = NewDamage;
}
