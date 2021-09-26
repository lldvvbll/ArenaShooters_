// Fill out your copyright notice in the Description page of Project Settings.


#include "ASItemFactory.h"
#include "Net/UnrealNetwork.h"
#include "Engine/ActorChannel.h"
#include "ASGameState.h"
#include "Item/ASItem.h"
#include "DataAssets/ItemDataAssets/ASItemDataAsset.h"
#include "Character/ASCharacter.h"

UASItem* AASItemFactory::NewASItem(UWorld* World, AActor* NewOwner, UASItemDataAsset* DataAsset, int32 Count/* = 0*/)
{
	if (DataAsset == nullptr)
	{
		AS_LOG_S(Error);
		return nullptr;
	}
	if (World == nullptr)
	{
		AS_LOG_S(Error);
		return nullptr;
	}

	auto GameState = World->GetGameState<AASGameState>();
	if (GameState == nullptr)
	{
		AS_LOG_S(Error);
		return nullptr;
	}

	AASItemFactory* ItemFactory = GameState->GetASItemFactory();
	if (ItemFactory == nullptr)
	{
		AS_LOG_S(Error);
		return nullptr;
	}

	auto NewItem = NewObject<UASItem>(World->GetCurrentLevel(), DataAsset->ItemClass);
	if (NewItem == nullptr)
	{
		AS_LOG_S(Error);
		return nullptr;
	}

	NewItem->SetDataAsset(DataAsset);
	NewItem->SetOwner(NewOwner);
	NewItem->SetCount(Count);

	ItemFactory->ASItems.Emplace(NewItem);

	return NewItem;
}

bool AASItemFactory::DeleteItem(UWorld* World, UASItem* InItem)
{
	if (!IsValid(InItem))
	{
		AS_LOG_S(Error);
		return false;
	}

	if (World == nullptr)
	{
		AS_LOG_S(Error);
		return false;
	}

	auto GameState = World->GetGameState<AASGameState>();
	if (GameState == nullptr)
	{
		AS_LOG_S(Error);
		return false;
	}

	AASItemFactory* ItemFactory = GameState->GetASItemFactory();
	if (ItemFactory == nullptr)
	{
		AS_LOG_S(Error);
		return false;
	}

	int32 Idx = ItemFactory->ASItems.Find(InItem);
	if (Idx == INDEX_NONE)
		return false;

	auto Owner = Cast<AASCharacter>(InItem->GetOwner());
	if (IsValid(Owner))
	{
		if (!Owner->RemoveItem(InItem))
			return false;
	}

	ItemFactory->ASItems[Idx] = nullptr;
	ItemFactory->ASItems.RemoveAtSwap(Idx);

	InItem->MarkPendingKill();

	UObject* Obj = InItem;
	FReferencerInformationList Refs;
	if (IsReferenced(Obj, RF_Public, EInternalObjectFlags::None, true, &Refs))
	{
		for (auto& Info : Refs.InternalReferences)
		{
			if (Info.Referencer != nullptr)
			{
				AS_LOG(Warning, TEXT("Int Ref: %s"), *Info.Referencer->GetName());
			}
		}

		for (auto& Info : Refs.ExternalReferences)
		{
			if (Info.Referencer != nullptr)
			{
				AS_LOG(Warning, TEXT("Ext Ref: %s"), *Info.Referencer->GetName());
			}
		}
	}

	return true;
}

AASItemFactory::AASItemFactory()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	bAlwaysRelevant = true;
	SetReplicatingMovement(false);
}

bool AASItemFactory::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	bool WroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

	for (auto& ASItem : ASItems)
	{
		if (IsValid(ASItem))
		{
			WroteSomething |= Channel->ReplicateSubobject(ASItem, *Bunch, *RepFlags);
		}
	}

	return WroteSomething;
}

void AASItemFactory::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AASItemFactory, ASItems);
}
