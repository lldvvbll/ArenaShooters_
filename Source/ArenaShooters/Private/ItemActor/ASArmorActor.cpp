// Fill out your copyright notice in the Description page of Project Settings.


#include "ItemActor/ASArmorActor.h"

AASArmorActor::AASArmorActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetCanBeDamaged(false);

	EmptyRootComp = CreateDefaultSubobject<USceneComponent>(TEXT("EmptyRootComp"));

	ArmorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ArmorMesh"));
	ArmorMesh->SetCollisionProfileName(TEXT("NoCollision"));
	ArmorMesh->SetupAttachment(EmptyRootComp);

	RootComponent = EmptyRootComp;
}
