// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ArenaShooters.h"
#include "Components/ActorComponent.h"
#include "ASItemFactoryComponent.generated.h"

class UASItem;
class UASItemDataAsset;

UCLASS()
class ARENASHOOTERS_API UASItemFactoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	static UASItem* NewASItem(UWorld* World, AActor* NewOwner, UASItemDataAsset* DataAsset, int32 Count = 0);

	template <typename ItemT, typename ItemDataAssetT>
	static ItemT* NewASItem(UWorld* World, AActor* NewOwner, ItemDataAssetT* DataAsset, int32 Count = 0)
	{
		static_assert(TIsDerivedFrom<ItemDataAssetT, UASItemDataAsset>::IsDerived, "Invalid ItemDataAssetT!");
		static_assert(TIsDerivedFrom<ItemT, ItemDataAssetT::ItemClassBaseT>::IsDerived, "Invalid ItemT!");

		return Cast<ItemT>(NewASItem(World, NewOwner, DataAsset, Count));
	}

	static bool DeleteItem(UWorld* World, UASItem* InItem);

public:	
	UASItemFactoryComponent();

	virtual bool ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	UPROPERTY(Replicated)
	TArray<UASItem*> ASItems;
};
