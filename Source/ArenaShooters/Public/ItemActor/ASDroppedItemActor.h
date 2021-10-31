// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ArenaShooters.h"
#include "GameFramework/Actor.h"
#include "ASDroppedItemActor.generated.h"

class UASItem;

UCLASS(Abstract)
class ARENASHOOTERS_API AASDroppedItemActor : public AActor
{
	GENERATED_BODY()
	
public:	
	AASDroppedItemActor();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void SetSkeletalMesh(USkeletalMesh* InSkelMesh);
	void SetStaticMesh(UStaticMesh* InStaticMesh);

	TArray<TWeakObjectPtr<UASItem>> GetItems() const;
	void AddItem(UASItem* InItem);
	void AddItems(const TArray<UASItem*>& InItems);
	bool RemoveItem(UASItem* InItem);

	void SetSelfDestroy(float InLifeSpan);

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnRep_ASItems(TArray<UASItem*>& OldItems);

public:
	DECLARE_EVENT_OneParam(AASDroppedItemActor, FOnRemoveItemEvent, const TWeakObjectPtr<UASItem>&)
	FOnRemoveItemEvent OnRemoveItemEvent;

protected:
	UPROPERTY(EditDefaultsOnly)
	USphereComponent* Collision;

	UPROPERTY(EditDefaultsOnly)
	USkeletalMeshComponent* SkeletalMeshComp;

	UPROPERTY(EditDefaultsOnly)
	UStaticMeshComponent* StaticMeshComp;

	UPROPERTY(ReplicatedUsing = OnRep_ASItems)
	TArray<UASItem*> ASItems;

	UPROPERTY(EditInstanceOnly)
	TMap<FPrimaryAssetId, int32> DropItemDataAssetMap;
};
