// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ArenaShooters.h"
#include "UObject/NoExportTypes.h"
#include "Common/ASEnums.h"
#include "ASItem.generated.h"

class UASItemDataAsset;
class AASItemActor;
class AASDroppedItemActor;

UCLASS(Abstract)
class ARENASHOOTERS_API UASItem : public UObject
{
	GENERATED_BODY()
	
public:
	virtual void BeginDestroy() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool IsSupportedForNetworking() const override;

	FPrimaryAssetId GetPrimaryAssetId() const;

	virtual void SetDataAsset(UASItemDataAsset* NewDataAsset);
	const UASItemDataAsset* GetDataAsset() const;

	EItemType GetItemType() const;
	UTexture2D* GetItemImage() const;
	UTexture2D* GetEquipmentSlotImage() const;
	const FText& GetItemName() const;
	TSubclassOf<AASDroppedItemActor> GetDroppedItemActorClass() const;

	int32 GetCount() const;
	virtual void SetCount(int32 NewCount);
	void ModifyCount(int32 Value);
	int32 GetMaxCount() const;

	void SetOwner(AActor* NewOwner);
	TWeakObjectPtr<AActor>& GetOwner();

	bool IsBundleItem() const;

	UFUNCTION()
	void OnRep_Count();

public:
	DECLARE_EVENT_OneParam(UASItem, FOnChangeCountEvent, UASItem*);
	FOnChangeCountEvent OnChangeCount;

protected:
	UPROPERTY(Replicated, VisibleAnywhere)
	UASItemDataAsset* DataAsset;

	UPROPERTY(ReplicatedUsing = OnRep_Count, VisibleAnywhere)
	int32 Count;

	UPROPERTY(Replicated)
	TWeakObjectPtr<AActor> Owner;
};
