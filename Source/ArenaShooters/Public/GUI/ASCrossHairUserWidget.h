// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ArenaShooters.h"
#include "Blueprint/UserWidget.h"
#include "Common/ASEnums.h"
#include "ASCrossHairUserWidget.generated.h"

class UImage;
class UASWeapon;

UCLASS()
class ARENASHOOTERS_API UASCrossHairUserWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void OnChangeSelectedWeapon(const TWeakObjectPtr<UASWeapon>& OldWeapon, const TWeakObjectPtr<UASWeapon>& NewWeapon);
	void OnChangedShootingStance(EShootingStanceType NewShootingStance);

	void SpreadBar();
	void SetBarVisibility(bool bVisible);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime);

protected:
	UPROPERTY()
	UImage* TopBar;

	UPROPERTY()
	UImage* BottomBar;

	UPROPERTY()
	UImage* LeftBar;

	UPROPERTY()
	UImage* RightBar;

	float CurrentBarOffset;
	float MinBarOffset;
	float MaxBarOffset;
	float OffsetPerShot;
	float RecoverySpeed;
};
