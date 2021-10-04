// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/ASItem.h"
#include "DataAssets/ItemDataAssets/ASItemDataAsset.h"
#include "Net/UnrealNetwork.h"
#include "GameMode/ASItemFactoryComponent.h"

void UASItem::BeginDestroy()
{
	Super::BeginDestroy();

	AS_LOG(Error, TEXT("ASItem Name: %s"), *GetName());
}

void UASItem::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UASItem, DataAsset);
	DOREPLIFETIME(UASItem, Count);
	DOREPLIFETIME(UASItem, Owner);
}

bool UASItem::IsSupportedForNetworking() const
{
	return true;
}

void UASItem::SetDataAsset(UASItemDataAsset* NewDataAsset)
{
	DataAsset = NewDataAsset;
}

const UASItemDataAsset* UASItem::GetDataAsset() const
{
	return DataAsset;
}

EItemType UASItem::GetItemType() const
{
	return (DataAsset != nullptr) ? DataAsset->ItemType : EItemType::None;
}

UTexture2D* UASItem::GetItemImage() const
{
	return (DataAsset != nullptr) ? DataAsset->ItemImage : nullptr;
}

UTexture2D* UASItem::GetEquipmentSlotImage() const
{
	return (DataAsset != nullptr) ? DataAsset->EquipmentSlotImage : nullptr;
}

const FText& UASItem::GetItemName() const
{
	return (DataAsset != nullptr) ? DataAsset->ItemName : FText::GetEmpty();
}

TSubclassOf<AASDroppedItemActor> UASItem::GetDroppedItemActorClass() const
{
	return (DataAsset != nullptr) ? DataAsset->DroppedItemActorClass : nullptr;
}

int32 UASItem::GetCount() const
{
	return Count;
}

void UASItem::SetCount(int32 NewCount)
{
	Count = NewCount;

	if (Owner.IsValid() && Owner->GetLocalRole() == ROLE_Authority)
	{
		OnChangeCount.Broadcast(this);
	}

	if (Count <= 0 && IsBundleItem())
	{
		UASItemFactoryComponent::DeleteItem(GetWorld(), this);
	}
}

void UASItem::ModifyCount(int32 Value)
{
	SetCount(Count + Value);
}

int32 UASItem::GetMaxCount() const
{
	return (DataAsset != nullptr) ? DataAsset->MaxBundleCount : -1;
}

void UASItem::SetOwner(AActor* NewOwner)
{
	Owner = MakeWeakObjectPtr(NewOwner);
}

TWeakObjectPtr<AActor>& UASItem::GetOwner()
{
	return Owner;
}

bool UASItem::IsBundleItem() const
{
	return (DataAsset != nullptr) ? DataAsset->bBundle : false;
}

void UASItem::OnRep_Count()
{
	OnChangeCount.Broadcast(this);
}
