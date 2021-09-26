// Fill out your copyright notice in the Description page of Project Settings.


#include "ASGameState.h"
#include "Net/UnrealNetwork.h"
#include "ASItemFactory.h"

void AASGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AASGameState, ItemFactory);
}

void AASGameState::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = this;
	ItemFactory = GetWorld()->SpawnActor<AASItemFactory>(SpawnParameters);

	check(ItemFactory);
}

AASItemFactory* AASGameState::GetASItemFactory() const
{
	return ItemFactory;
}
